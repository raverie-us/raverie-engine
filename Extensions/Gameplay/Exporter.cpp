///////////////////////////////////////////////////////////////////////////////
///
/// \file Export.cpp
/// Exporter code.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Files or extensions that are NOT automatically copied (may be handled specially)
HashSet<String>& GetExcludedFiles()
{
  static HashSet<String> files;
  files.Insert("BuildInfo.data");
  files.Insert("ZeroEditor.exe");
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

typedef void(*FileCallback)(StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData);

void AddFilesHelper(StringParam directory, StringParam relativePathFromStart, FileCallback callback, void* userData)
{
  HashSet<String>& excludedFiles = GetExcludedFiles();

  for (FileRange fileRange(directory); !fileRange.Empty(); fileRange.PopFront())
  {
    String fileName = fileRange.Front();

    if (excludedFiles.Contains(fileName) || excludedFiles.Contains(FilePath::GetExtension(fileName)))
      continue;

    String newRelativePath = FilePath::Combine(relativePathFromStart, fileName);

    String fullPath = FilePath::Combine(directory, fileName);
    if (DirectoryExists(fullPath))
    {
      AddFilesHelper(fullPath, newRelativePath, callback, userData);
    }
    else
    {
      callback(fullPath, newRelativePath, fileName, userData);
    }
  }
}

void AddFiles(StringParam directory, FileCallback callback, void* userData)
{
  AddFilesHelper(directory, String(), callback, userData);
}

void ArchiveLibraryOutput(Archive& archive, ContentLibrary* library)
{
  ResourceListing listing;
  library->BuildListing(listing);

  //Add every file in the package to the archive
  String outputPath = library->GetOutputPath();
  String archivePath = library->Name;

  forRange(ResourceEntry resource, listing.All())
  {
    String fullPath = FilePath::Combine(outputPath, resource.Location);
    String relativePath = FilePath::Combine(archivePath, resource.Location);

    // Don't export resources that are marked as template files.
    if (resource.GetResourceTemplate() && resource.Type.Contains("Zilch"))
      continue;

    archive.AddFile(fullPath, relativePath);
  }

  //Finally add the pack file
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

void CopyLibraryOut(StringParam outputDirectory, ContentLibrary* library)
{
  String libraryPath = library->GetOutputPath();
  String libraryOutputPath = FilePath::Combine(outputDirectory, library->Name);

  CreateDirectoryAndParents(libraryOutputPath);

  // Copy the .pack file
  String packFile = BuildString(library->Name, ".pack");
  String packFileSource = FilePath::Combine(libraryPath, packFile);
  String packFileDestination = FilePath::Combine(libraryOutputPath, packFile);
  CopyFile(packFileDestination, packFileSource);

  BoundType* zilchDocumentType = ZilchTypeId(ZilchDocumentResource);
  BoundType* ZilchPluginSourceType = ZilchTypeId(ZilchPluginSource);
  BoundType* zilchPluginLibraryType = ZilchTypeId(ZilchPluginLibrary);

  forRange(ContentItem* contentItem, library->GetContentItems())
  {
    bool isTemplate = contentItem->has(ResourceTemplate);

    // Copy each generated Resource
    ResourceListing listing;
    contentItem->BuildListing(listing);
    forRange(ResourceEntry& entry, listing.All())
    {
      // Skip zilch Resource Templates
      if (isTemplate)
      {
        BoundType* resourceType = MetaDatabase::FindType(entry.Type);

        // Skip zilch resource types
        if (resourceType->IsA(zilchDocumentType) ||
          resourceType->IsA(ZilchPluginSourceType) ||
          resourceType->IsA(zilchPluginLibraryType))
        {
          continue;
        }
      }

      String fileName = entry.Location;
      String source = FilePath::Combine(libraryPath, fileName);
      String destination = FilePath::Combine(libraryOutputPath, fileName);
      CopyFile(destination, source);
    }
  }
}

void CopyLibraryOut(StringParam outputDirectory, StringParam name)
{
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(name, nullptr);
  if (library)
    CopyLibraryOut(outputDirectory, library);
}

void RelativeCopyFile(StringParam dest, StringParam source, StringParam filename)
{
  CopyFile(FilePath::Combine(dest, filename), FilePath::Combine(source, filename));
}

void CopyInstallerSetupFile(StringParam dest, StringParam source, StringParam projectName, Guid guid)
{
  // Open our installer setup template file
  String setupFilename = "InnoSetupTemplate.txt";
  File setup;
  setup.Open(FilePath::Combine(source, setupFilename), FileMode::Read, FileAccessPattern::Sequential);

  // Make a buffer to hold the template data
  size_t filesize = setup.Size();
  byte* buffer = new byte[filesize];

  // Read the template from the file
  Status status;
  setup.Read(status, buffer, filesize);
  setup.Close();

  if (status.Failed())
  {
    DoNotifyWarning("File Read Error", "Failed to read template setup file, aborting installer file setup");
    // cleanup the buffer
    delete buffer;
    return;
  }

  // Create a string containing the data and delete our temporary buffer
  String fileContent((char*)buffer, filesize);
  delete buffer;

  // Replace the template values with the project name and generate a guid
  String outputFileContent = fileContent.Replace("%PROJECTNAME%", projectName);
  outputFileContent = outputFileContent.Replace("%GUID%", ToString(guid.mValue));

  // Open our output file and write out the updated data
  File outputFile;
  outputFile.Open(FilePath::CombineWithExtension(dest, BuildString(projectName, "InstallerSetup"), ".iss"), FileMode::Write, FileAccessPattern::Sequential);
  outputFile.Write((byte*)outputFileContent.Data(), outputFileContent.SizeInBytes());
  outputFile.Close();
}
//---------------------------------------------------------------- Do Export Job
class DoExportJob : public Job
{
public:
  DoExportJob(Exporter* exporter)
    :mExporter(exporter)
  {
  }

  Exporter* mExporter;
  int Execute() override
  {
    mExporter->DoExport();
    return 0;
  }
};

const String cFilesSelected = "cFilesSelected";

//--------------------------------------------------------------------- Exporter
Exporter::Exporter()
{
  ConnectThisTo(this, cFilesSelected, OnFilesSelected);
}

void Exporter::ExportAndPlay(Cog* projectCog)
{
  mProjectCog = projectCog;
  mOutputFile = FilePath::Combine(GetTemporaryDirectory(), "GameTest.exe");
  mPlay = true;
  BeginExport();
}

void Exporter::ExportGameProject(Cog* projectCog)
{
  mProjectCog = projectCog;
  mPlay = false;
  FileDialogConfig config;
  config.EventName = cFilesSelected;
  config.CallbackObject = this;
  config.Title = "Export game Exe.";
  config.AddFilter("Exe", "*.exe");
  config.mDefaultSaveExtension = "exe";
  Z::gEngine->has(OsShell)->SaveFile(config);
}

void BuildContent(ProjectSettings* project);

void Exporter::BeginExport()
{
  // Save all resources and build them so the 
  // output directory is up to date
  Editor* editor = Z::gEditor;
  editor->SaveAll(true);
  BuildContent(mProjectCog->has(ProjectSettings));

  // Begin export in background
  Z::gJobs->AddJob(new DoExportJob(this));
}

void Exporter::OnFilesSelected(OsFileSelection* file)
{
  if (file->Success)
  {
    mOutputFile = file->Files[0];
    BeginExport();
  }
}

void UpdateIcon(ProjectSettings* project, ExecutableResourceUpdater& updater)
{
  //Assume the icon file is located in the project direction with the name "Icon.ico"
  String iconFile = FilePath::Combine(project->ProjectFolder, "Icon.ico");
  if (FileExists(iconFile))
  {
    //Read the file into memory
    size_t bufferSize = 0;
    byte* buffer = ReadFileIntoMemory(iconFile.c_str(), bufferSize);

    if (buffer && bufferSize)
      updater.UpdateIcon(buffer, bufferSize);

    zDeallocate(buffer);
  }
}

void ArchiveFileCallback(StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData)
{
  Archive& archive = *(Archive*)userData;
  archive.AddFile(fullPath, relativePath);
}

void Exporter::DoExport()
{
  SendBlockingTaskStart("Exporting");

  {
    TimerBlock block("Exported Project");
    ProjectSettings* project = mProjectCog->has(ProjectSettings);
    String outputPath = FilePath::Combine(GetTemporaryDirectory(), "ZeroExport");
    String appDirectory = GetApplicationDirectory();
    Cog* configCog = Z::gEngine->GetConfigCog();
    MainConfig* mainConfig = configCog->has(MainConfig);

    CreateDirectoryAndParents(outputPath);

    String projectFileName = "Project.zeroproj";
    String projectFile = FilePath::Combine(outputPath, projectFileName);
    SaveToDataFile(*project->GetOwner(), projectFile);

    String contentOutput = Z::gContentSystem->ContentOutputPath;

    //archive all core resources
    Archive engineArchive(ArchiveMode::Compressing);
    ArchiveLibraryOutput(engineArchive, "FragmentCore");
    ArchiveLibraryOutput(engineArchive, "Loading");
    ArchiveLibraryOutput(engineArchive, "ZeroCore");
    ArchiveLibraryOutput(engineArchive, "UiWidget");
    ArchiveLibraryOutput(engineArchive, "EditorUi");
    ArchiveLibraryOutput(engineArchive, "Editor");

    // Add all dlls (and other files next to the exe)
    AddFiles(appDirectory, ArchiveFileCallback, &engineArchive);

    engineArchive.AddFile(projectFile, "Project.zeroproj");

    // Add files from project data directory into archive data directory
    String dataDirectory = mainConfig->DataDirectory;
    AddFilesHelper(dataDirectory, "Data", ArchiveFileCallback, &engineArchive);

    SetWorkingDirectory(appDirectory);

    //Copy in ZeroCrashHandler
    String crashHandler = FilePath::Combine("Tools", "ZeroCrashHandler.exe");
    if (FileExists(crashHandler))
      engineArchive.AddFileRelative(appDirectory, crashHandler);

    Archive projectArchive(ArchiveMode::Compressing);

    if (SharedContent* sharedConent = mProjectCog->has(SharedContent))
    {
      forRange(ContentLibraryReference libraryRef, sharedConent->ExtraContentLibraries.All())
      {
        String libraryName = libraryRef.mContentLibraryName;
        ArchiveLibraryOutput(projectArchive, libraryName);
      }
    }

    //archive project resources
    ArchiveLibraryOutput(projectArchive, project->ProjectContentLibrary);

    String tempName = BuildString(project->ProjectName, "Temp.exe");
    String tempFile = FilePath::Combine(GetTemporaryDirectory(), tempName);

    //Alright time to create the exe
    CopyFile(tempFile, FilePath::Combine(appDirectory, "ZeroEditor.exe"));

    //Embed package as resource section. This is generally more windows friendly than appending
    //the data on the End() of the exe since opening the exe for reading is unreliable. 
    //The resource data section IDR_PACK is already in the exe and will be updated from empty
    //to a package containing all resources.

    //Each export generates a new export id so that files do not conflict
    u64 exportId = GenerateUniqueId64();

    String uniqueName = BuildString(project->ProjectName, ToString(exportId));

    uint sizeOfEnginePacked = engineArchive.ComputeZipSize();
    uint sizeOfProjectPacked = projectArchive.ComputeZipSize();
    uint totalSizePacked = sizeOfEnginePacked + sizeOfProjectPacked + sizeof(u32) + uniqueName.SizeInBytes();

    ByteBufferBlock final(totalSizePacked);
    u32 size = (u32)uniqueName.SizeInBytes();
    final.Write((byte*)&size, sizeof(size));
    final.Write((byte*)uniqueName.Data(), size);

    engineArchive.WriteBuffer(final);
    engineArchive.Clear();

    projectArchive.WriteBuffer(final);
    projectArchive.Clear();

    // Output zip for debugging
    bool outputDebugFiles = configCog->has(DeveloperConfig) != nullptr;
    if (outputDebugFiles)
    {
      String packFile = FilePath::Combine(outputPath, gPackName);
      WriteToFile(packFile.c_str(), final.GetBegin(), final.Size());
    }

    // ExecutableResourceUpdater flushes at destruction,
    // so we use a scope to force it to write resource changes
    {
      Status status;
      ExecutableResourceUpdater updater(status, tempFile.c_str());
      if (status.Succeeded())
      {
        // Update the PACK resource section with the project's resources. 
        updater.Update(gPackName, gPackType, final.GetBegin(), final.Size());
      }
      else
      {
        DoNotifyWarning("Export Failed", "Failed to acquire handle to exported exe.");
        SendBlockingTaskFinish();
        return;
      }

      //Update the icon
      UpdateIcon(project, updater);
    }

    CopyFile(mOutputFile, tempFile);

    SendBlockingTaskFinish();
  }

  DoNotify("Exported", "Project has been exported.", "Disk");

  if (mPlay)
  {
    //Wait for file system finish writing the file
    Os::SystemOpenFile(mOutputFile.c_str());
  }
}

void CopyFileCallback(StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData)
{
  String& outputDirectory = *(String*)userData;
  String destPath = FilePath::Combine(outputDirectory, relativePath);
  CreateDirectoryAndParents(FilePath::GetDirectoryPath(destPath));
  CopyFile(destPath, fullPath);
}

void ExportContentFolders(Cog* projectCog)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);
  String outputDirectory = FilePath::Combine(GetTemporaryDirectory(), project->ProjectName);
  String appDirectory = GetApplicationDirectory();
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);

  // Delete the old content if it was previously exported
  DeleteDirectoryContents(outputDirectory);
  CreateDirectoryAndParents(outputDirectory);

  // Copy content output
  CopyLibraryOut(outputDirectory, "FragmentCore");
  CopyLibraryOut(outputDirectory, "Loading");
  CopyLibraryOut(outputDirectory, "ZeroCore");
  CopyLibraryOut(outputDirectory, "UiWidget");
  CopyLibraryOut(outputDirectory, "EditorUi");
  CopyLibraryOut(outputDirectory, "Editor");
  CopyLibraryOut(outputDirectory, project->ProjectContentLibrary);

  // Copy default configuration
  RelativeCopyFile(outputDirectory, appDirectory, "Configuration.data");

  // Copy Inno Setup Template
  CopyInstallerSetupFile(outputDirectory, mainConfig->DataDirectory, project->ProjectName, project->GetProjectGuid());

  // Add all dlls (and other files next to the exe)
  AddFiles(appDirectory, CopyFileCallback, &outputDirectory);

  //Copy the project file
  CopyFile(FilePath::Combine(outputDirectory, "Project.zeroproj"), project->ProjectFile);

  // Add data directory
  String dataDirectory = mainConfig->DataDirectory;
  AddFilesHelper(dataDirectory, "Data", CopyFileCallback, &outputDirectory);

  //Copy the executable
  String outputExe = FilePath::Combine(outputDirectory, "ZeroEditor.exe");
  CopyFile(outputExe, GetApplication());

  // Scope the ExecutableResourceUpdater so it writes out at the end
  {
    Status status;
    ExecutableResourceUpdater updater(status, outputExe.c_str());
    if (status.Succeeded())
      UpdateIcon(project, updater);
  }

  Os::SystemOpenFile(outputDirectory.c_str());
}

}//namespace
