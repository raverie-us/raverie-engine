// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Utility Functions
namespace ExportUtility
{

// Files or extensions that are NOT automatically copied (may be handled
// specially)
HashSet<String>& GetExcludedFiles()
{
  static HashSet<String> files;
  files.Insert("WelderEditor.exe");
  files.Insert("exp");
  files.Insert("ilk");
  files.Insert("lib");
  files.Insert("pdb");
  files.Insert("obj");
  files.Insert("iobj");
  files.Insert("ipdb");
  files.Insert("log");
  return files;
}

void AddFilesHelper(StringParam directory,
                    StringParam relativePathFromStart,
                    HashSet<String>& additionalFileExcludes,
                    FileCallback callback,
                    void* userData)
{
  HashSet<String>& excludedFiles = GetExcludedFiles();

  // Get the total number of files being added to calculate and show progress
  // for the exporting screen
  float i = 0.0f;
  float totalFiles = 0.0f;
  for (FileRange fileRange(directory); !fileRange.Empty(); fileRange.PopFront())
    totalFiles += 1.0f;

  for (FileRange fileRange(directory); !fileRange.Empty(); fileRange.PopFront())
  {
    String fileName = fileRange.Front();

    if (excludedFiles.Contains(fileName) || excludedFiles.Contains(FilePath::GetExtension(fileName)) ||
        additionalFileExcludes.Contains(fileName) || additionalFileExcludes.Contains(FilePath::GetExtension(fileName)))
      continue;

    String newRelativePath = FilePath::Combine(relativePathFromStart, fileName);

    String fullPath = FilePath::Combine(directory, fileName);
    if (DirectoryExists(fullPath))
    {
      AddFilesHelper(fullPath, newRelativePath, additionalFileExcludes, callback, userData);
    }
    else
    {
      callback(fullPath, newRelativePath, fileName, userData, i / totalFiles);
    }
  }
}

void AddFiles(StringParam directory, HashSet<String>& additionalFileExcludes, FileCallback callback, void* userData)
{
  AddFilesHelper(directory, String(), additionalFileExcludes, callback, userData);
}

void ArchiveLibraryOutput(Archive& archive, ContentLibrary* library)
{
  ResourceListing listing;
  library->BuildListing(listing);

  // Add every file in the package to the archive
  String outputPath = library->GetOutputPath();
  String archivePath = library->Name;

  int itemsDone = 0;
  float librarySize = (float)listing.Size();

  forRange (ResourceEntry resource, listing.All())
  {
    ++itemsDone;
    String fullPath = FilePath::Combine(outputPath, resource.Location);
    String relativePath = FilePath::Combine(archivePath, resource.Location);

    // Don't export resources that are marked as template files.
    if (resource.GetResourceTemplate() && resource.Type.Contains("Zilch"))
      continue;

    archive.AddFile(fullPath, relativePath);
    Z::gEngine->LoadingUpdate(
        "Archive Library", library->Name, resource.Name, ProgressType::Normal, float(itemsDone) / librarySize);
  }

  // Finally add the pack file
  String packFile = BuildString(library->Name, ".pack");
  String packFilePath = FilePath::Combine(outputPath, packFile);

  archive.AddFile(packFilePath, FilePath::Combine(archivePath, packFile));
}

void ArchiveLibraryOutput(Archive& archive, StringParam libraryName)
{
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(libraryName, nullptr);
  if (library)
    ArchiveLibraryOutput(archive, library);
}

void CopyLibraryOut(StringParam outputDirectory, ContentLibrary* library, bool skipTemplates)
{
  String libraryPath = library->GetOutputPath();
  if (!DirectoryExists(libraryPath))
  {
    ZPrint("Skipped copying library output because it was not built %s\n", library->Name.c_str());
    return;
  }

  String libraryOutputPath = FilePath::Combine(outputDirectory, library->Name);

  CreateDirectoryAndParents(libraryOutputPath);

  // Copy the .pack file
  String packFile = BuildString(library->Name, ".pack");
  String packFileSource = FilePath::Combine(libraryPath, packFile);
  if (FileExists(packFileSource))
  {
    String packFileDestination = FilePath::Combine(libraryOutputPath, packFile);
    CopyFile(packFileDestination, packFileSource);
  }

  BoundType* zilchDocumentType = ZilchTypeId(ZilchDocumentResource);
  BoundType* ZilchPluginSourceType = ZilchTypeId(ZilchPluginSource);
  BoundType* zilchPluginLibraryType = ZilchTypeId(ZilchPluginLibrary);

  int itemsDone = 0;
  float librarySize = (float)library->GetContentItems().Size();

  forRange (ContentItem* contentItem, library->GetContentItems())
  {
    ++itemsDone;
    bool isTemplate = contentItem->has(ResourceTemplate);

    // Copy each generated Resource
    ResourceListing listing;
    contentItem->BuildListing(listing);
    forRange (ResourceEntry& entry, listing.All())
    {
      // Skip zilch Resource Templates
      if (isTemplate && skipTemplates)
      {
        BoundType* resourceType = MetaDatabase::FindType(entry.Type);

        // Skip zilch resource types
        if (resourceType->IsA(zilchDocumentType) || resourceType->IsA(ZilchPluginSourceType) ||
            resourceType->IsA(zilchPluginLibraryType))
        {
          continue;
        }
      }

      String fileName = entry.Location;
      String source = FilePath::Combine(libraryPath, fileName);
      if (!FileExists(source))
        continue;

      String destination = FilePath::Combine(libraryOutputPath, fileName);
      CopyFile(destination, source);
      Z::gEngine->LoadingUpdate("Copying", fileName, "", ProgressType::Normal, float(itemsDone) / librarySize);
    }
  }
}

void CopyLibraryOut(StringParam outputDirectory, StringParam name, bool skipTemplates)
{
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(name, nullptr);
  if (library)
    CopyLibraryOut(outputDirectory, library, skipTemplates);
}

void RelativeCopyFile(StringParam dest, StringParam source, StringParam filename)
{
  CopyFile(FilePath::Combine(dest, filename), FilePath::Combine(source, filename));
}

void ArchiveFileCallback(
    StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData, float progressPercent)
{
  Archive& archive = *(Archive*)userData;
  archive.AddFile(fullPath, relativePath);
  Z::gEngine->LoadingUpdate("Archiving", fileName, "", ProgressType::Normal, progressPercent);
}

void CopyFileCallback(
    StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData, float progressPercent)
{
  String& outputDirectory = *(String*)userData;
  String destPath = FilePath::Combine(outputDirectory, relativePath);
  CreateDirectoryAndParents(FilePath::GetDirectoryPath(destPath));
  CopyFile(destPath, fullPath);
  Z::gEngine->LoadingUpdate("Copying", fileName, "", ProgressType::Normal, progressPercent);
}

} // namespace ExportUtility

// Content Source
struct ExportTargetSource : public DataSource
{
  ExportTargetList* mExportList;

  ExportTargetSource(ExportTargetList* exportList) : mExportList(exportList)
  {
  }

  DataEntry* GetRoot() override
  {
    return (DataEntry*)mExportList;
  }

  DataEntry* ToEntry(DataIndex index) override
  {
    return (DataEntry*)index.Id;
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    return DataIndex((u64)dataEntry);
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    return nullptr;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    if (dataEntry == mExportList)
      return mExportList->Entries.Size();
    else
      return 0;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    ExportTargetList* listing = (ExportTargetList*)dataEntry;
    return (DataEntry*)listing->SortedEntries[index];
  }

  bool IsExpandable(DataEntry* dataEntry) override
  {
    return dataEntry == mExportList;
  }

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    // Ignore the root
    if (dataEntry == mExportList)
      return;

    ExportTargetEntry* entry = (ExportTargetEntry*)dataEntry;
    if (!column.Empty())
    {
      if (column == CommonColumns::Name)
      {
        variant = entry->TargetName;
      }
      else if (column == "Export")
      {
        variant = entry->Export;
      }
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    ExportTargetEntry* entry = (ExportTargetEntry*)dataEntry;
    if (!column.Empty())
    {
      if (column == "Export")
      {
        entry->Export = variant.Get<bool>();
        return true;
      }
    }
    return false;
  }
};

// ExportTargetList
ExportTargetList::ExportTargetList()
{
}

ExportTargetList::~ExportTargetList()
{
  DeleteObjectsInContainer(SortedEntries);
}

void ExportTargetList::AddEntry(ExportTargetEntry* entry)
{
  Entries.Insert(entry->TargetName, entry);
  SortedEntries.PushBack(entry);
}

void ExportTargetList::SetActiveTargets(HashSet<String>& activeTargets)
{
  forRange (ExportTargetEntry* entry, SortedEntries)
  {
    if (activeTargets.Contains(entry->TargetName))
      entry->Export = true;
  }
}

HashSet<String> ExportTargetList::GetActiveTargets()
{
  HashSet<String> activeTargets;
  forRange (ExportTargetEntry* entry, SortedEntries)
  {
    if (entry->Export)
      activeTargets.Insert(entry->TargetName);
  }
  return activeTargets;
}

ExportUI::ExportUI(Composite* parent) : Composite(parent)
{
  this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 2), Thickness::cZero));

  new Label(this, cText, "Application Name:");
  mApplicationName = new TextBox(this);
  mApplicationName->SetEditable(true);

  Cog* projectCog = Z::gEditor->GetProjectCog();
  ProjectSettings* projectSettings = Z::gEngine->GetProjectSettings();
  ExportSettings* exportSettings = HasOrAdd<ExportSettings>(projectCog);
  if (projectSettings)
    mApplicationName->SetText(projectSettings->GetProjectName());

  new Label(this, cText, "Export Path:");

  Composite* pathRow = new Composite(this);
  pathRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));

  mExportPath = new TextBox(pathRow);
  mExportPath->SetEditable(true);
  mExportPath->SetText(
      FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Exports", projectSettings->GetProjectName()));
  mExportPath->SetSizing(SizeAxis::X, SizePolicy::Flex, Pixels(200));

  TextButton* pathSelectButton = new TextButton(pathRow);
  pathSelectButton->SetText("...");
  pathSelectButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  ConnectThisTo(pathSelectButton, Events::ButtonPressed, OnSelectPath);

  mTreeView = new TreeView(this);
  mTreeView->SetMinSize(Pixels(200, 200));
  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);

  SetTreeFormatting();

  mSource = new ExportTargetSource(&mTargetList);
  mTreeView->SetDataSource(mSource);

  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
  {
    TextButton* exportButton = new TextButton(buttonRow);
    exportButton->SetText("Export");
    exportButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
    ConnectThisTo(exportButton, Events::ButtonPressed, OnExportApplication);

    Composite* temp = new Composite(buttonRow);
    temp->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(10));

    TextButton* exportContentButton = new TextButton(buttonRow);
    exportContentButton->SetText("Export Content");
    exportContentButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(100));
    ConnectThisTo(exportContentButton, Events::ButtonPressed, OnExportContentFolder);

    temp = new Composite(buttonRow);
    temp->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

    TextButton* cancelButton = new TextButton(buttonRow);
    cancelButton->SetText("Cancel");
    cancelButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
    ConnectThisTo(cancelButton, Events::ButtonPressed, OnCancel);
  }
}

ExportUI::~ExportUI()
{
  SafeDelete(mSource);
}

void ExportUI::SetTreeFormatting()
{
  TreeFormatting formatting;
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders);

  // Name column
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Name;
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 3;
  format->HeaderName = "Target Platform";
  format->Editable = false;

  // Export column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "Export";
  format->ColumnType = ColumnType::Fixed;
  format->FixedSize = Pixels(60, 20);
  format->Editable = false;
  format->HeaderName = "Export";
  format->CustomEditor = cDefaultBooleanEditor;

  mTreeView->SetFormat(formatting);
}

void ExportUI::OpenExportWindow()
{
  Exporter* exporter = Exporter::GetInstance();

  Window* window = new Window(Z::gEditor);
  window->SetTitle("Export Project");
  ExportUI* exportUi = new ExportUI(window);
  window->SizeToContents();
  window->SetSize(Pixels(340, 490));
  CenterToWindow(Z::gEditor, window, false);

  HashSet<String> targets;
  forRange (StringParam target, exporter->mExportTargets.Keys())
    targets.Insert(target);

  Cog* projectCog = Z::gEditor->GetProjectCog();
  ExportSettings* exportSettings = HasOrAdd<ExportSettings>(projectCog);

  exportUi->SetAvailableTargets(targets);
  exportUi->SetActiveTargets(exportSettings->mActiveTargets);

  window->MoveToFront();
}

void ExportUI::OnExportApplication(Event* e)
{
  ExportTargetList* targetList = (ExportTargetList*)mSource->GetRoot();
  if (targetList)
  {
    // Setup export settings
    Exporter* exporter = Exporter::GetInstance();
    exporter->mApplicationName = mApplicationName->GetText();
    exporter->mOutputDirectory = mExportPath->GetText();
    // Export to all to selected targets
    HashSet<String> activeTargets = targetList->GetActiveTargets();
    SaveActiveTargets(activeTargets);
    exporter->ExportApplication(activeTargets);
    // Close the export window
    CloseTabContaining(this);
    return;
  }

  Error("Something went wrong");
}

void ExportUI::OnExportContentFolder(Event* e)
{
  ExportTargetList* targetList = (ExportTargetList*)mSource->GetRoot();
  if (targetList)
  {
    // Setup export settings
    Exporter* exporter = Exporter::GetInstance();
    exporter->mApplicationName = mApplicationName->GetText();
    exporter->mOutputDirectory = mExportPath->GetText();
    // Export to all to selected targets
    HashSet<String> activeTargets = targetList->GetActiveTargets();
    SaveActiveTargets(activeTargets);
    exporter->ExportContent(activeTargets);
    // Close the export window
    CloseTabContaining(this);
    return;
  }

  Error("Something went wrong");
}

void ExportUI::OnCancel(Event* e)
{
  CloseTabContaining(this);
}

void ExportUI::OnSelectPath(Event* e)
{
  // Set up the callback for when project file is selected
  const String CallBackEvent = "ExportLocationSelected";
  if (!GetDispatcher()->IsConnected(CallBackEvent, this))
    ConnectThisTo(this, CallBackEvent, OnFolderSelected);

  // Open the open file dialog
  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = CallBackEvent;
  config->CallbackObject = this;
  config->Title = "Select a folder";
  config->AddFilter("Project Export Folder", "*.none");
  config->DefaultFileName = mExportPath->GetText();
  config->StartingDirectory = mExportPath->GetText();
  config->Flags |= FileDialogFlags::Folder;
  Z::gEngine->has(OsShell)->SaveFile(config);
}

void ExportUI::OnFolderSelected(OsFileSelection* e)
{
  if (e->Files.Size() > 0)
  {
    String path = BuildString(FilePath::GetDirectoryPath(e->Files[0]), cDirectorySeparatorCstr);
    mExportPath->SetText(path);
  }
}

void ExportUI::SetAvailableTargets(HashSet<String>& targets)
{
  forRange (StringParam target, targets)
  {
    ExportTargetEntry* entry = new ExportTargetEntry(target);
    mTargetList.AddEntry(entry);
  }
  mTreeView->Refresh();
}

void ExportUI::SetActiveTargets(HashSet<String>& targets)
{
  mTargetList.SetActiveTargets(targets);
  mTreeView->Refresh();
}

void ExportUI::SaveActiveTargets(HashSet<String>& targets)
{
  Cog* projectCog = Z::gEditor->GetProjectCog();
  if (ExportSettings* exportSettings = HasOrAdd<ExportSettings>(projectCog))
    exportSettings->mActiveTargets = targets;
}

Exporter::Exporter()
{
  // Add all available export targets
  mExportTargets.Insert("Windows", new WindowsExportTarget(this, "Windows"));
  mExportTargets.Insert("Web", new EmscriptenExportTarget(this, "Web"));
  mDefaultTargets.Insert("Windows");
}

void Exporter::ExportAndPlay(Cog* projectCog)
{
  mProjectCog = projectCog;
  mPlay = true;

  mApplicationName = "GameTest.exe";
  mOutputDirectory = GetTemporaryDirectory();

  ExportApplication(mDefaultTargets);
}

void Exporter::ExportGameProject(Cog* projectCog)
{
  mProjectCog = projectCog;
  mPlay = false;
  ExportUI::OpenExportWindow();
}

void Exporter::UpdateIcon(ProjectSettings* project, ExecutableResourceUpdater& updater)
{
  // Assume the icon file is located in the project direction with the name
  // "Icon.ico"
  String iconFile = FilePath::Combine(project->ProjectFolder, "Icon.ico");
  if (FileExists(iconFile))
  {
    // Read the file into memory
    size_t bufferSize = 0;
    byte* buffer = ReadFileIntoMemory(iconFile.c_str(), bufferSize);

    if (buffer && bufferSize)
      updater.UpdateIcon(buffer, bufferSize);

    zDeallocate(buffer);
  }
}

void Exporter::SaveAndBuildContent()
{
  // Save all resources and build them so the
  // output directory is up to date
  Editor* editor = Z::gEditor;
  editor->SaveAll(true);
}

void Exporter::ExportApplication(HashSet<String> exportTargets)
{
  SaveAndBuildContent();

  Z::gEngine->LoadingStart();
  CreateDirectoryAndParents(mOutputDirectory);
  forRange (ExportTarget* exportTarget, mExportTargets.Values())
  {
    if (exportTargets.Contains(exportTarget->mTargetName))
      exportTarget->ExportApplication();
  }
  Z::gEngine->LoadingFinish();
  Download(mOutputDirectory);
}

void Exporter::ExportContent(HashSet<String> exportTargets)
{
  SaveAndBuildContent();

  Z::gEngine->LoadingStart();
  CreateDirectoryAndParents(mOutputDirectory);
  forRange (ExportTarget* exportTarget, mExportTargets.Values())
  {
    if (exportTargets.Contains(exportTarget->mTargetName))
      exportTarget->ExportContentFolders(mProjectCog);
  }
  Z::gEngine->LoadingFinish();
  Download(mOutputDirectory);
}

void Exporter::CopyContent(Status& status, String outputDirectory, ExportTarget* target)
{
  Assert(target, "A valid export target should always be provided");

  ProjectSettings* project = mProjectCog->has(ProjectSettings);
  String appDirectory = GetApplicationDirectory();
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);

  // Delete the old content if it was previously exported
  bool directoryEmpty = EnsureEmptyDirectory(outputDirectory);
  if (!directoryEmpty)
  {
    status.SetFailed("Unable to delete the output directory contents");
    return;
  }

  // Copy content output
  ExportUtility::CopyLibraryOut(outputDirectory, "FragmentCore");
  ExportUtility::CopyLibraryOut(outputDirectory, "Loading");
  ExportUtility::CopyLibraryOut(outputDirectory, "ZeroCore");
  ExportUtility::CopyLibraryOut(outputDirectory, "UiWidget");
  ExportUtility::CopyLibraryOut(outputDirectory, "EditorUi");
  ExportUtility::CopyLibraryOut(outputDirectory, "Editor");
  ExportUtility::CopyLibraryOut(outputDirectory, project->ProjectContentLibrary);

  // Once the build output is separated by platform this should not be needed
  HashSet<String>& additionalExcludes = target->GetAdditionalExcludedFiles();

  // Copy default configuration
  ExportUtility::RelativeCopyFile(outputDirectory, appDirectory, "Configuration.data");

  // Copy Inno Setup Template
  target->CopyInstallerSetupFile(
      outputDirectory, mainConfig->DataDirectory, project->ProjectName, project->GetProjectGuid());

  // Add all dlls (and other files next to the exe)
  ExportUtility::AddFiles(appDirectory, additionalExcludes, ExportUtility::CopyFileCallback, &outputDirectory);

  // Copy the project file
  CopyFile(FilePath::Combine(outputDirectory, "Project.zeroproj"), project->ProjectFile);

  // Add data directory
  String dataDirectory = mainConfig->DataDirectory;
  ExportUtility::AddFilesHelper(
      dataDirectory, "Data", additionalExcludes, ExportUtility::CopyFileCallback, &outputDirectory);
}

} // namespace Zero
