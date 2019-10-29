// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ContentSystem, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

void CreateAudioContent(ContentSystem* system);
void CreateGeometryContent(ContentSystem* system);
void CreateImageContent(ContentSystem* system);
void CreateDataContent(ContentSystem* system);
void CreateVectorContent(ContentSystem* system);
void CreateScriptContent(ContentSystem* system);
void CreateBinaryContent(ContentSystem* system);
void CreateZilchPluginContent(ContentSystem* system);
void CreateSupportContent(ContentSystem* system);

namespace Events
{
DefineEvent(PackageBuilt);
} // namespace Events

ZilchDefineType(ContentSystemEvent, builder, type)
{
}

struct SortByLoadOrder
{
  bool operator()(ResourceEntry& left, ResourceEntry& right)
  {
    // First sort by load order, then name for determinism.
    return left.LoadOrder < right.LoadOrder || (left.LoadOrder == right.LoadOrder && left.Name < right.Name);
  }
};

namespace Z
{
ContentSystem* gContentSystem;
} // namespace Z

ContentComponent* ContentComponentFactory::CreateFromName(StringRange name)
{
  ContentCreatorMapType::range r = Creators.Find(name);
  if (r.Empty())
    return nullptr;

  MakeContentComponent creator = r.Front().second;
  ContentComponent* component = (*creator)();
  return component;
}

void InitializeContentSystem()
{
  ContentSystem* system = ContentSystem::GetInstance();
  Z::gContentSystem = system;

  AddContentComponent<ContentTags>(system);

  CreateAudioContent(system);
  CreateGeometryContent(system);
  CreateImageContent(system);
  CreateDataContent(system);
  CreateVectorContent(system);
  CreateScriptContent(system);
  CreateBinaryContent(system);
  CreateZilchPluginContent(system);
  CreateSupportContent(system);
}

void ShutdownContentSystem()
{
}

ContentSystem::ContentSystem()
{
  mHistoryEnabled = true;
  mIdCount = 0;

  // some special load order dependencies
  LoadOrderMap["FontDefinition"] = 5;
  LoadOrderMap["Atlas"] = 5;
  LoadOrderMap["CollisionGroup"] = 5;
  LoadOrderMap["SampleCurve"] = 9;
  LoadOrderMap["SoundTag"] = 40;
  LoadOrderMap["SoundAttenuator"] = 41;
  LoadOrderMap["SoundCue"] = 55;
  LoadOrderMap["ResourceTable"] = 99;

  IgnoredExtensions.Insert("meta");
  IgnoredExtensions.Insert("library");
  IgnoredExtensions.Insert("libview");
  IgnoredExtensions.Insert("__pycache__");

  HistoryPath = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "History");
}

ContentSystem::~ContentSystem()
{
  DeleteObjectsInContainer(Libraries);
}

void ContentSystem::EnumerateLibraries()
{
  ProfileScopeFunction();

  // For every search path
  forRange (String path, LibrarySearchPaths.All())
  {
    path = FilePath::Normalize(path);
    EnumerateLibrariesInPath(path);
  }
}

ContentLibrary* ContentSystem::LibraryFromDirectory(Status& status, StringParam name, StringParam libraryDirectory)
{
  // If the library already exists then update the directory and return the
  // library
  if (ContentLibrary* loadedLibrary = Libraries.FindValue(name, nullptr))
    return loadedLibrary;

  // Normalize the path to prevent issues with bad paths in config files
  String directory = FilePath::Normalize(libraryDirectory);

  // Find the library file
  String libraryFile = FilePath::CombineWithExtension(directory, name, ".library");

  ContentLibrary* library = new ContentLibrary();
  library->LibraryFile = libraryFile;
  library->SourcePath = directory;

  // Does the library file listing file exist?
  if (FileExists(libraryFile))
  {
    // The library file exists, load it
    bool loaded = library->Load();

    if (!loaded)
    {
      status.SetFailed(String::Format("Can not load library file '%s'", libraryFile.c_str()));
      return nullptr;
    }

    // Generate an id for this library if necessary
    if (library->LibraryId == 0)
      library->LibraryId = GenerateUniqueId64();

    // Set name if necessary
    if (library->Name.Empty())
      library->Name = name;

    // Check to see if the library file is writable
    // if it is not prevent modifications to this library.
    library->mReadOnly = !FileWritable(libraryFile);
  }
  else
  {
    // Creating a new library
    library->Name = name;
    library->LibraryId = GenerateUniqueId64();
    library->mReadOnly = false;
    // String prepedDirectory = BuildString("\\\\?\\", directory);
    if (!DirectoryExists(directory))
    {
      // Library directory does not exist so create it
      // to prevent errors when saving.
      CreateDirectoryAndParents(directory);
    }

    library->Save();
  }

  // Map the library
  Libraries.InsertOrError(library->Name, library);

  // Scan the directory for the content items
  FileRange filesInDirectory(directory);

  for (; !filesInDirectory.Empty(); filesInDirectory.PopFront())
  {
    String filename = filesInDirectory.Front();

    Status addStatus;

    // Add each item to the content library
    AddContentItemInfo addContent;
    addContent.FileName = filename;
    addContent.Library = library;

    AddContentItemToLibrary(addStatus, addContent);
  }

  // Library has been created
  return library;
}

HandleOf<ResourcePackage> ContentSystem::BuildLibrary(Status& status, ContentLibrary* library, bool sendEvent)
{
  ProfileScopeFunctionArgs(library->Name);
  ZPrintFilter(Filter::EngineFilter, "Building Content Library '%s'\n", library->Name.c_str());

  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  String sourceDirectory = mainConfig->SourceDirectory;

  String outputPath = library->GetOutputPath();

  // If we don't already have the content output directory, then see if we have
  // local prebuilt content that can be copied into the output directory (this
  // is faster than building the content ourselves, if it exists).
  if (!DirectoryExists(outputPath))
  {
    String versionedContentName = FilePath::GetFileName(ContentOutputPath);
    String prebuiltContent = FilePath::Combine(PrebuiltContentPath, library->Name);
    if (DirectoryExists(prebuiltContent))
    {
      ZPrint("Copying prebuilt content from '%s' to '%s'\n", prebuiltContent.c_str(), outputPath.c_str());
      ProfileScope("PrebuiltContent");
      CopyFolderContents(outputPath, prebuiltContent);
    }
    else
    {
      ZPrint("Prebuilt content '%s' does not exist\n", prebuiltContent.c_str());
    }
  }
  else
  {
    ZPrint("Content output for '%s' exists\n", outputPath.c_str());
  }

  Array<ContentItem*> items;
  items.Reserve(library->ContentItems.Size());
  items.Append(library->ContentItems.Values());
  HandleOf<ResourcePackage> package = Z::gContentSystem->BuildContentItems(status, items, library, false);

  String libraryPackageFile = FilePath::CombineWithExtension(outputPath, library->Name, ".pack");
  package->Save(libraryPackageFile);

  if (sendEvent)
  {
    // Changing this to non-delayed could be a problem
    // If it is, we need to allocate the ResourcePackage.
    ContentSystemEvent toSend;
    toSend.mLibrary = library;
    toSend.mPackage = package;
    Z::gContentSystem->DispatchEvent(Events::PackageBuilt, &toSend);
  }

  return package;
}

class ContentAddCleanUp
{
public:
  String FileToDelete;

  void Success()
  {
    FileToDelete = String();
  }

  ~ContentAddCleanUp()
  {
    if (!FileToDelete.Empty())
      DeleteFile(FileToDelete);
  }
};

ContentItem* ContentSystem::AddContentItemToLibrary(Status& status, AddContentItemInfo& info)
{
  status = Status();

  if (Z::gEngine->IsReadOnly())
  {
    const String cMessage("Cannot add content items in read-only mode");
    DoNotifyWarning("Content", cMessage);
    status.SetFailed(cMessage);
    return nullptr;
  }

  ContentAddCleanUp cleanUp;

  ContentLibrary* library = info.Library;
  if (library == nullptr)
  {
    status.SetFailed("Need a content library to contain resource's ContentItem.");
    return nullptr;
  }

  if (info.FileName.Empty())
  {
    status.SetFailed("No filename");
    return nullptr;
  }

  // Generate final path of the file
  String fullPath = FilePath::Combine(library->SourcePath, info.FileName);
  String extension = FilePath::GetExtension(info.FileName);
  extension = extension.ToLower();

  // Add from external file (file not in content system already)
  bool fromExternalFile = !info.ExternalFile.Empty();
  if (fromExternalFile)
  {
    // Check to see if there is a content item with
    // the same file name already in the library
    if (FileExists(fullPath))
    {
      if (info.OnContentFileConflict == ContentFileConflict::FindNewName)
      {
        // Try to generate a new name
        info.OnContentFileConflict = ContentFileConflict::Fail;
        String fileToSplit = info.FileName;
        Pair<StringRange, StringRange> splitPath = SplitOnFirst(fileToSplit, '.');

        // Keep looping trying to find a valid file name
        for (uint i = 0; i < 100; ++i)
        {
          String numberSuffix = String::Format("_%d", i);
          info.FileName = BuildString(splitPath.first, numberSuffix, splitPath.second);

          ContentItem* item = AddContentItemToLibrary(status, info);

          if (status.Failed())
          {
            // Check if something else beside AlreadyExists has gone wrong
            if (status.Context != ExtenedAddErrors::AlreadyExists)
              return nullptr;
          }
          else
          {
            // It worked! stop looping
            return item;
          }
        }

        status.SetFailed(String::Format("Can not find valid name for file '%s'", info.FileName.c_str()));
        return nullptr;
      }
      else if (info.OnContentFileConflict == ContentFileConflict::Replace)
      {
        ContentItem* existingItem = library->FindContentItemByFileName(info.FileName);

        if (existingItem)
        {
          // Replacing content file
          CopyFile(fullPath, info.ExternalFile);

          // Fill out our content initializer, this has information for the new
          // content options
          ContentInitializer initializer;
          initializer.Filename = info.FileName;
          initializer.Library = library;
          initializer.Options = info.Options;
          initializer.AddResourceId = info.AddResourceId;

          // Update the .meta information for this item
          ContentTypeEntry entry = CreatorsByExtension.FindValue(extension, ContentTypeEntry());
          UpdateContentItem contentUpdater = entry.UpdateItem;
          if (contentUpdater)
          {
            // update the content items meta with our new settings
            existingItem = (*contentUpdater)(existingItem, initializer);
            existingItem->SaveMetaFile();

            // Rebuild the resources associated with this item ?
            status.SetSucceeded("Existing content item updated.");

            // Return updated content item
            return existingItem;
          }
          else
          {
            // Return existing content item, the newer file was already copied
            // in
            return existingItem;
          }
        }
        else
        {
          String metaFilePath = BuildString(fullPath, ".meta");
          // If we found the content file and no content item check to see if a
          // meta file is present
          if (FileExists(metaFilePath))
          {
            // If there is a meta file it is most likely corrupted or failed to
            // load somehow
            String metaFile = FilePath::GetFileName(metaFilePath);
            status.SetFailed(
                String::Format("Meta file %s is corrupt or failed to load.", info.FileName.c_str(), metaFile.c_str()));
          }
          else
          {
            status.SetFailed(String::Format("File %s exists but is not a content file.", info.FileName.c_str()));
          }
          return nullptr;
        }
      }
      else // if(info.OnContentFileConflict == ContentFileConflict::Fail)
      {
        String message = String::Format("The content file named %s has already been added.", info.FileName.c_str());
        status.SetFailed(message);
        status.Context = ExtenedAddErrors::AlreadyExists;
        return nullptr;
      }
    }

    // Copy into content library
    CopyFile(fullPath, info.ExternalFile);

    // Delete file if something fails
    cleanUp.FileToDelete = fullPath;
  }

  String uniqueFileId = UniqueFileId(fullPath);

  ContentItem* currentContentItem = library->ContentItems.FindValue(uniqueFileId, nullptr);

  if (currentContentItem)
  {
    if (info.OnContentFileConflict == ContentFileConflict::Replace)
    {
      library->ContentItems.Erase(currentContentItem->UniqueFileId);
      delete currentContentItem;
    }
    else
    {
      status.SetFailed(
          String::Format("The content file named '%s' is already in the content library", info.FileName.c_str()));
      return nullptr;
    }
  }

  bool contentFileExists = FileExists(fullPath);

  String metaFile = FilePath::CombineWithExtension(library->SourcePath, info.FileName, ".meta");

  if (FileExists(metaFile))
  {
    if (!contentFileExists)
    {
      // Meta file but no content item
      status.SetFailed(String::Format("File '%s' not found but meta file exists.", fullPath.c_str()));
      return nullptr;
    }

    ContentTypeEntry entry = CreatorsByExtension.FindValue(extension, ContentTypeEntry());
    if (entry.MakeItem == 0)
    {
      return nullptr;
    }

    // Meta file and Content file exist normal load path

    // Content item meta file already exists so load it.
    UniquePointer<Serializer> stream(GetLoaderStreamFile(status, metaFile));

    if (status.Failed())
    {
      // status set by GetLoaderStreamFile
      return nullptr;
    }

    // Determine what content type is in the meta file
    PolymorphicNode node;
    uint offset = stream->GetPolymorphic(node);

    entry = Z::gContentSystem->Creators.FindValue(node.TypeName, ContentTypeEntry());
    MakeContentItem contentCreator = entry.MakeItem;

    if (contentCreator == nullptr)
    {
      status.SetFailed(String::Format("Failed to create content type %s. Content type is not "
                                      "registered with content system.",
                                      node.TypeName.Data()));
      return nullptr;
    }

    // Create the content item
    ContentInitializer initializer;
    initializer.Filename = info.FileName;
    initializer.Library = library;
    initializer.Options = info.Options;
    initializer.AddResourceId = info.AddResourceId;

    ContentItem* newContentItem = (*contentCreator)(initializer);

    // Check for content item load errors
    if (newContentItem == nullptr)
    {
      status.SetFailed(initializer.Message);
      SafeDelete(newContentItem);
      return nullptr;
    }

    // Serialize content item data
    newContentItem->Serialize(*stream);

    // Set file name
    newContentItem->Filename = info.FileName;
    newContentItem->UniqueFileId = uniqueFileId;

    // Initialize into the library
    newContentItem->Initialize(library);

    // Adding new tags
    if (!info.Tags.Empty())
    {
      newContentItem->SetTags(info.Tags);
      newContentItem->SaveMetaFile();
    }

    info.AddedContentItem = newContentItem;

    cleanUp.Success();
    status.SetSucceeded("Successfully found content item");
    return newContentItem;
  }
  else
  {
    // Adding new content item
    if (!contentFileExists)
    {
      status.SetFailed(
          String::Format("File %s is missing from content library %s ", fullPath.c_str(), library->Name.c_str()));
      return nullptr;
    }

    // Is this file extension ignored?
    if (IgnoredExtensions.Count(extension))
    {
      status.SetFailed(String::Format("Extension '%s' is ignored", extension.c_str()));
      return nullptr;
    }

    ContentTypeEntry entry = CreatorsByExtension.FindValue(extension, ContentTypeEntry());
    if (entry.MakeItem == 0)
    {
      status.SetFailed(String::Format("Could not find content type for extension %s", extension.c_str()));
      return nullptr;
    }

    // Creating a new content item
    ContentInitializer initializer;
    initializer.Filename = info.FileName;
    initializer.Extension = extension;
    initializer.BuilderType = info.BuilderType;
    initializer.AddResourceId = info.AddResourceId;
    initializer.Success = true;
    initializer.Library = library;
    initializer.Options = info.Options;
    initializer.ResourceOwner = info.ResourceOwner;

    // If no name is provided generate one
    if (info.Name.Empty())
      initializer.Name = FilePath::GetFileNameWithoutExtension(info.FileName);
    else
      initializer.Name = info.Name;

    // Create the content item
    ContentItem* newContentItem = (*entry.MakeItem)(initializer);

    // Check for failure to create content item
    if (newContentItem == nullptr || initializer.Success == false)
    {
      status.SetFailed(initializer.Message);
      SafeDelete(newContentItem);
      return nullptr;
    }

    // Content item was created
    newContentItem->Filename = info.FileName;
    newContentItem->UniqueFileId = uniqueFileId;
    newContentItem->Initialize(library);
    newContentItem->SetTags(info.Tags);

    // Save new meta file
    newContentItem->SaveMetaFile();

    // Add to source control
    SourceControl* sourceControl = GetSourceControl(library->SourceControlType);

    Status sourceControlStatus;
    sourceControl->Add(sourceControlStatus, newContentItem->GetFullPath());
    DoNotifyStatus(sourceControlStatus);

    sourceControl->Add(sourceControlStatus, newContentItem->GetMetaFilePath());
    DoNotifyStatus(sourceControlStatus);

    info.AddedContentItem = newContentItem;

    cleanUp.Success();
    status.SetSucceeded("Successfully added new content item");
    return newContentItem;
  }
}

bool ContentSystem::RemoveContentItemFromLibray(ContentItem* contentItem)
{
  ContentLibrary* library = contentItem->mLibrary;
  SetWorkingDirectory(library->SourcePath);

  // Get the original full path to the content item and meta file
  String contentFile = contentItem->GetFullPath();
  String contentMetaFile = contentItem->GetMetaFilePath();

  if (Z::gContentSystem->mHistoryEnabled)
  {
    String backUpPath = GetHistoryPath(library);
    BackUpFile(backUpPath, contentFile);
    BackUpFile(backUpPath, contentMetaFile);
  }

  // Update source control
  SourceControl* sourceControl = GetSourceControl(library->SourceControlType);

  Status status;
  sourceControl->Remove(status, contentFile);
  DoNotifyStatus(status);

  sourceControl->Remove(status, contentMetaFile);
  DoNotifyStatus(status);

  // Remove from content library
  library->ContentItems.Erase(contentItem->UniqueFileId);

  // Delete the content item
  delete contentItem;

  return true;
}

bool ContentSystem::RenameContentItemFile(ContentItem* contentItem, StringParam newFileName)
{
  ContentLibrary* library = contentItem->mLibrary;
  SetWorkingDirectory(library->SourcePath);

  // Build Strings

  // Get the original full path to the content item and meta file
  String oldId = contentItem->UniqueFileId;
  String contentFile = contentItem->GetFullPath();
  String contentMetaFile = contentItem->GetMetaFilePath();

  String path = FilePath::GetDirectoryPath(contentFile);
  String newFullPath = FilePath::Combine(path, newFileName);
  String newMetaFile = BuildString(newFullPath, ".meta");

  // Does a file with this name already exist?
  // Is that file not the file we are renaming? If they are the same file allow
  // capitalization changes in existing name.
  if (FileExists(newFileName) && (contentFile.ToLower() != newFullPath.ToLower()))
  {
    // Do not rename the file
    return false;
  }

  SourceControl* sourceControl = GetSourceControl(library->SourceControlType);

  // Rename using source control this will also change the files
  Status status;
  sourceControl->Rename(status, contentFile, newFullPath);
  DoNotifyStatus(status);

  sourceControl->Rename(status, contentMetaFile, newMetaFile);
  DoNotifyStatus(status);

  // Update the content item's data
  contentItem->Filename = newFileName;
  contentItem->UniqueFileId = UniqueFileId(newFullPath);

  // Update the Content items on the Library
  library->ContentItems.Erase(oldId);
  library->ContentItems.Insert(contentItem->UniqueFileId, contentItem);

  return true;
}

String ContentSystem::GetHistoryPath(ContentLibrary* library)
{
  return FilePath::Combine(Z::gContentSystem->HistoryPath, library->Name);
}

ContentItem* ContentSystem::FindContentItemByFileName(StringParam filename)
{
  forRange (ContentLibrary* library, Libraries.Values())
  {
    ContentItem* item = library->FindContentItemByFileName(filename);
    if (item)
      return item;
  }

  return nullptr;
}

ContentItem* ContentSystem::CreateFromName(StringRange name)
{
  ContentCreatorMapType::range r = Creators.Find(name);
  if (!r.Empty())
  {
    MakeContentItem creator = r.Front().second.MakeItem;
    ContentInitializer initializer;
    ContentItem* contentItem = (*creator)(initializer);
    return contentItem;
  }
  return nullptr;
}

HandleOf<ResourcePackage>
ContentSystem::BuildContentItems(Status& status, ContentItemArray& toBuild, ContentLibrary* library, bool useJobs)
{
  ProfileScopeFunctionArgs(library->Name);
  Z::gEngine->LoadingStart();

  ResourcePackage* package = new ResourcePackage();
  package->Name = library->Name;

  package->Location = library->GetOutputPath();
  CreateDirectoryAndParents(package->Location);

  BuildOptions buildOptions(library);

  bool allBuilt = true;

  for (uint i = 0; i < toBuild.Size(); ++i)
  {
    // Process from this contentItem down.
    ContentItem* contentItem = toBuild[i];
    static const String cProcessing("Processing");
    Z::gEngine->LoadingUpdate(
        cProcessing, library->Name, contentItem->Filename, ProgressType::Normal, (float)(i + 1) / toBuild.Size());

    contentItem->BuildContentItem(useJobs);

    if (buildOptions.Failure)
    {
      ZPrint("Content Build Failed, %s\n", buildOptions.Message.c_str());
      buildOptions.Failure = false;
      buildOptions.Message = String();
      allBuilt = false;
    }

    contentItem->BuildListing(package->Resources);

    // Don't do this in the thread (do it after).
    if (contentItem->mNeedsEditorProcessing)
      package->EditorProcessing.PushBack(contentItem);
  }

  Sort(package->Resources.All(), SortByLoadOrder());

  if (!allBuilt)
    status.SetFailed(String::Format("Failed to build content library '%s'", library->Name.c_str()));

  Z::gEngine->LoadingFinish();
  return package;
}

HandleOf<ResourcePackage> ContentSystem::BuildSingleContentItem(Status& status, ContentItem* contentItem)
{
  ContentItemArray contentItems;
  contentItems.PushBack(contentItem);
  HandleOf<ResourcePackage> package = BuildContentItems(status, contentItems, contentItem->mLibrary, false);
  DoNotifyStatus(status);
  return package;
}

void ContentSystem::EnumerateLibrariesInPath(StringParam path)
{
  // For every item in the search path
  FileRange files(path);
  for (; !files.Empty(); files.PopFront())
  {
    // If the path is a directory, enumerate it
    String directoryPath = FilePath::Combine(path, files.Front());
    if (DirectoryExists(directoryPath))
    {
      String name = files.Front();
      Status status;
      LibraryFromDirectory(status, name, directoryPath);
    }
  }
}

} // namespace Zero
