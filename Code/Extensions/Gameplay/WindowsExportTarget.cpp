// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

using namespace ExportUtility;

HashSet<String>& WindowsExportTarget::GetAdditionalExcludedFiles()
{
  static HashSet<String> files;
  files.Insert("wast");
  files.Insert("wasm");
  files.Insert("map");
  return files;
}

WindowsExportTarget::WindowsExportTarget(Exporter* exporter, String targetName) : ExportTarget(exporter, targetName)
{
}

void WindowsExportTarget::ExportApplication()
{
  String applicationOutputPath =
      FilePath::CombineWithExtension(mExporter->mOutputDirectory, mExporter->mApplicationName, ".exe");
  {
    ProfileScopeFunction();
    ProjectSettings* project = mExporter->mProjectCog->has(ProjectSettings);
    String outputPath = FilePath::Combine(GetTemporaryDirectory(), "Windows", "ZeroExport");
    String appDirectory = GetApplicationDirectory();
    Cog* configCog = Z::gEngine->GetConfigCog();
    MainConfig* mainConfig = configCog->has(MainConfig);

    CreateDirectoryAndParents(outputPath);

    String projectFileName = "Project.zeroproj";
    String projectFile = FilePath::Combine(outputPath, projectFileName);
    SaveToDataFile(*project->GetOwner(), projectFile);

    String contentOutput = Z::gContentSystem->ContentOutputPath;

    // archive all core resources
    Archive engineArchive(ArchiveMode::Compressing);
    ArchiveLibraryOutput(engineArchive, "FragmentCore");
    ArchiveLibraryOutput(engineArchive, "Loading");
    ArchiveLibraryOutput(engineArchive, "ZeroCore");
    ArchiveLibraryOutput(engineArchive, "UiWidget");
    ArchiveLibraryOutput(engineArchive, "EditorUi");
    ArchiveLibraryOutput(engineArchive, "Editor");

    // Once the build output is separated by platform this should not be needed
    HashSet<String>& additionalExcludes = GetAdditionalExcludedFiles();

    // Add all dlls (and other files next to the exe)
    AddFiles(appDirectory, additionalExcludes, ArchiveFileCallback, &engineArchive);

    engineArchive.AddFile(projectFile, "Project.zeroproj");

    // Add files from project data directory into archive data directory
    String dataDirectory = mainConfig->DataDirectory;
    AddFilesHelper(dataDirectory, "Data", additionalExcludes, ArchiveFileCallback, &engineArchive);

    SetWorkingDirectory(appDirectory);

    // Copy in ZeroCrashHandler
    String crashHandler = FilePath::Combine("Tools", "ZeroCrashHandler.exe");
    if (FileExists(crashHandler))
      engineArchive.AddFileRelative(appDirectory, crashHandler);

    Archive projectArchive(ArchiveMode::Compressing);
    if (SharedContent* sharedConent = mExporter->mProjectCog->has(SharedContent))
    {
      forRange (ContentLibraryReference libraryRef, sharedConent->ExtraContentLibraries.All())
      {
        String libraryName = libraryRef.mContentLibraryName;
        ArchiveLibraryOutput(projectArchive, libraryName);
      }
    }

    // archive project resources
    ArchiveLibraryOutput(projectArchive, project->ProjectContentLibrary);

    String tempName = BuildString(project->ProjectName, "Temp.exe");
    String tempFile = FilePath::Combine(GetTemporaryDirectory(), tempName);

    // Alright time to create the exe
    CopyFile(tempFile, FilePath::Combine(appDirectory, "WelderEditor.exe"));

    // Embed package as resource section. This is generally more windows
    // friendly than appending the data on the End() of the exe since opening
    // the exe for reading is unreliable. The resource data section IDR_PACK is
    // already in the exe and will be updated from empty to a package containing
    // all resources.

    // Each export generates a new export id so that files do not conflict
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
        return;
      }

      // Update the icon
      mExporter->UpdateIcon(project, updater);
    }

    CopyFile(applicationOutputPath, tempFile);
  }

  DoNotify("Exported", "Project has been exported for Windows.", "Disk");

  if (mExporter->mPlay)
  {
    // Wait for file system finish writing the file
    Os::ShellOpenDirectory(applicationOutputPath);
  }
}

void WindowsExportTarget::ExportContentFolders(Cog* projectCog)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);
  String outputDirectory = FilePath::Combine(GetTemporaryDirectory(), "Windows", project->ProjectName);

  Status copyStatus;
  mExporter->CopyContent(copyStatus, outputDirectory, this);

  if (copyStatus.Failed())
  {
    DoNotifyWarning("WindowsExportTarget", copyStatus.Message);
    return;
  }

  // Copy the executable
  String outputExe = FilePath::Combine(outputDirectory, "WelderEditor.exe");
  CopyFile(outputExe, GetApplication());

  // Scope the ExecutableResourceUpdater so it writes out at the end
  {
    Status status;
    ExecutableResourceUpdater updater(status, outputExe.c_str());
    if (status.Succeeded())
      mExporter->UpdateIcon(project, updater);
  }

  Os::ShellOpenDirectory(outputDirectory);
}

void WindowsExportTarget::CopyInstallerSetupFile(StringParam dest,
                                                 StringParam source,
                                                 StringParam projectName,
                                                 Guid guid)
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
    delete[] buffer;
    return;
  }

  // Create a string containing the data and delete our temporary buffer
  String fileContent((char*)buffer, filesize);
  delete[] buffer;

  // Replace the template values with the project name and generate a guid
  String outputFileContent = fileContent.Replace("%PROJECTNAME%", projectName);
  outputFileContent = outputFileContent.Replace("%GUID%", ToString(guid.mValue));

  // Open our output file and write out the updated data
  File outputFile;
  outputFile.Open(FilePath::CombineWithExtension(dest, BuildString(projectName, "InstallerSetup"), ".iss"),
                  FileMode::Write,
                  FileAccessPattern::Sequential);
  outputFile.Write((byte*)outputFileContent.Data(), outputFileContent.SizeInBytes());
  outputFile.Close();
}

} // namespace Zero
