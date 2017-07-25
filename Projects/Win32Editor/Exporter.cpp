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

// Windows only
#include "WindowsShell/WinUtility.hpp"
#include "Platform/Windows/Windows.hpp"
#include "Platform/Windows/WindowsError.hpp"
#include <stdlib.h>
#include "../Win32Shared/resource.h"
#include "Platform/Windows/WString.hpp"


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

typedef void (*FileCallback)(StringParam fullPath, StringParam relativePath, StringParam fileName, void* userData);

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
    if (IsDirectory(fullPath))
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
  if(library)
    ArchiveLibraryOutput(archive, library);
}

void CopyLibraryOut(StringParam outputDirectory, ContentLibrary* library)
{
  String libraryPath = library->GetOutputPath();
  String libraryOutputPath = FilePath::Combine(outputDirectory, library->Name);
  CopyFolderContents(libraryOutputPath, libraryPath);
}

void CopyLibraryOut(StringParam outputDirectory, StringParam name)
{
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(name, nullptr);
  if(library)
    CopyLibraryOut(outputDirectory, library);
}

void RelativeCopyFile(StringParam dest, StringParam source, StringParam filename)
{
  CopyFile(FilePath::Combine(dest,  filename), FilePath::Combine(source, filename));
}

void CopyInstallerSetupFile(StringParam dest, StringParam source, StringParam projectName)
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
  outputFileContent = outputFileContent.Replace("%GUID%", ToString(GenerateUniqueId64()));

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
  //Z::gEngine->LoadingStart();

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
  if(file->Success)
  {
    mOutputFile = file->Files[0];
    BeginExport();
  }
}

//------------------------------------------------------------ Icon Structures

#pragma pack(push, 2)

// Icon Entry Structure in .ico file
typedef struct
{
  BYTE        bWidth;          // Width, in pixels, of the image
  BYTE        bHeight;         // Height, in pixels, of the image
  BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
  BYTE        bReserved;       // Reserved ( must be 0)
  WORD        wPlanes;         // Color Planes
  WORD        wBitCount;       // Bits per pixel
  DWORD       dwBytesInRes;    // How many bytes in this resource?
  DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;


// Icon Directory Structure in .ico file
typedef struct
{
  WORD           idReserved;   // Reserved (must be 0)
  WORD           idType;       // Resource Type (1 for icons)
  WORD           idCount;      // How many images?
  ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;
const uint IconDirSize = sizeof(ICONDIR) - sizeof(ICONDIRENTRY);


// Group Icon Entry structure in Resource section
typedef struct
{
  BYTE   bWidth;               // Width, in pixels, of the image
  BYTE   bHeight;              // Height, in pixels, of the image
  BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
  BYTE   bReserved;            // Reserved
  WORD   wPlanes;              // Color Planes
  WORD   wBitCount;            // Bits per pixel
  DWORD   dwBytesInRes;         // how many bytes in this resource?
  WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

//Group Icon Directory structure in Resource section
typedef struct 
{
  WORD            idReserved;   // Reserved (must be 0)
  WORD            idType;       // Resource type (1 for icons)
  WORD            idCount;      // How many images?
  GRPICONDIRENTRY   idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
uint GroupIconSize = sizeof(GRPICONDIR) - sizeof(GRPICONDIRENTRY) * 1;

#pragma pack( pop )

void UpdateIcon(ProjectSettings* project, HANDLE updateHandle)
{
  //Assume the icon file is located in the project direction with the name "Icon.ico"
  String iconFile = FilePath::Combine(project->ProjectFolder, "Icon.ico");
  if( FileExists(iconFile) )
  {
    //Read the file into memory
    size_t bufferSize = 0;
    byte* buffer = ReadFileIntoMemory(iconFile.c_str(), bufferSize);

    ///Add/Update a icon resources from the icon file.
    ICONDIR& iconDir = *(ICONDIR*)buffer;

    uint iconCount = iconDir.idCount;

    //Offset the new icon ids starting at one.
    const uint cIconOffset = 1;

    for(uint i = 0;i<iconCount;++i)
    {
      //Add the new icon to resources
      BOOL result = UpdateResource(updateHandle,
        MAKEINTRESOURCE(RT_ICON), 
        MAKEINTRESOURCE(i+cIconOffset),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer + iconDir.idEntries[i].dwImageOffset,
        iconDir.idEntries[i].dwBytesInRes);

      CheckWin(result, "Failed to update resource");
    }

    //Compute the size of the group index structure
    uint groupIconSize = GroupIconSize + iconCount * sizeof(GRPICONDIRENTRY);

    //Now the GRPICONDIR resource has to be updated to refer to the new icons.
    //Build a new group directory and update the icon resource.
    GRPICONDIR& groupIcon = *(GRPICONDIR*)alloca(groupIconSize);
    ZeroMemory(&groupIcon, sizeof(groupIcon));
    groupIcon.idType = 1;
    groupIcon.idCount = iconCount;

    //Copy over icon data info
    for(uint i = 0;i<iconCount;++i)
    {
      GRPICONDIRENTRY& entry = groupIcon.idEntries[i];
      ICONDIRENTRY& iconEntry = iconDir.idEntries[i];
      entry.bReserved = 0; 
      entry.bWidth = iconEntry.bWidth;
      entry.bHeight = iconEntry.bHeight;
      entry.bColorCount = iconEntry.bColorCount;
      entry.wPlanes = iconEntry.wPlanes;
      entry.wBitCount = iconEntry.wBitCount;
      entry.dwBytesInRes = iconEntry.dwBytesInRes;
      entry.nID = i+cIconOffset;
    }

    //Update the resource
    //Note: If explorer is looking at the exe when this happens it can fail and/or have 
    //cached the old icons.
    BOOL result = UpdateResource(
      updateHandle,
      RT_GROUP_ICON,
      MAKEINTRESOURCE(IDI_ICON),
      MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
      &groupIcon,
      groupIconSize);
    CheckWin(result, "Failed to update resource");

    zDeallocate(buffer);
  }
}

void UpdateBannerBitmap(HANDLE updateHandle)
{
  //If the sprite source named "LaunchBanner" is in the project
  //replace the banner in the resource data of exe to customize
  //the banner of the launch window.
  //SpriteSource* spriteSource = SpriteSourceManager::FindOrNull("LaunchBanner");

  //if(spriteSource)
  //{
  //  //Convert the sprite source image into the bitmap format
  //  uint size = 0;
  //  byte* data = NULL;
  //  CreateBitmapBuffer(&spriteSource->SourceImage, data, size);

  //  //When updating a bitmap resource section the BITMAPFILEHEADER
  //  //is not included. Move past the header and reduce the size.
  //  uint bitmapHeaderSize = sizeof(BITMAPFILEHEADER);
  //  BOOL result = UpdateResource(updateHandle,
  //    MAKEINTRESOURCE(RT_BITMAP), 
  //    MAKEINTRESOURCE(IDB_BANNER),
  //    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
  //    data+bitmapHeaderSize,
  //    size-bitmapHeaderSize);

  //  //deallocate the bitmap buffer.
  //  zDeallocate(data);
  //}

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
  ArchiveLibraryOutput(engineArchive, "ZeroCore");
  ArchiveLibraryOutput(engineArchive, "Editor");
  ArchiveLibraryOutput(engineArchive, "Loading");

  // Add all dlls (and other files next to the exe)
  AddFiles(appDirectory, ArchiveFileCallback, &engineArchive);

  engineArchive.AddFile(projectFile, "Project.zeroproj");

  // Add files from project data directory into archive data directory
  String dataDirectory = mainConfig->DataDirectory;
  AddFilesHelper(dataDirectory, "Data", ArchiveFileCallback, &engineArchive);

  SetWorkingDirectory(appDirectory);

  //Copy in ZeroCrashHandler
  String crashHandler = FilePath::Combine("Tools", "ZeroCrashHandler.exe");
  if(FileExists(crashHandler))
    engineArchive.AddFileRelative(appDirectory, crashHandler);

  Archive projectArchive(ArchiveMode::Compressing);

  if(SharedContent* sharedConent = mProjectCog->has(SharedContent))
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
  if(outputDebugFiles)
  {
    String packFile = FilePath::Combine(outputPath, "Pack.bin");
    WriteToFile(packFile.c_str(), final.GetBegin(), final.Size());
  }

  //Update the binary resource in the exported exe
  HANDLE updateHandle = BeginUpdateResource(Widen(tempFile).c_str(), FALSE);

  if (updateHandle != nullptr)
  {
    //Update the PACK resource section with the project's resources. 
    BOOL result = UpdateResource(updateHandle,
                                L"PACK", 
                                MAKEINTRESOURCE(IDR_PACK),
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                final.GetBegin(),
                                final.Size());
  }
  else
  {
    DoNotifyWarning("Export Failed", "Failed to acquire handle to exported exe.");
    SendBlockingTaskFinish();
    return;
  }

  //Update the banner bitmap
  UpdateBannerBitmap(updateHandle);

  //Update the icon
  UpdateIcon(project, updateHandle);

  //EndUpdate to commit the changes.
  BOOL success = EndUpdateResource(updateHandle, FALSE);

  CheckWin(success, "Finish Updating of resources");

  CopyFile(mOutputFile, tempFile);

  SendBlockingTaskFinish();
  }

  DoNotify("Exported", "Project has been exported.", "Disk");

  if(mPlay)
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

  CreateDirectoryAndParents(outputDirectory);

  // Copy content output
  CopyLibraryOut(outputDirectory, "FragmentCore");
  CopyLibraryOut(outputDirectory, "ZeroCore");
  CopyLibraryOut(outputDirectory, "Editor");
  CopyLibraryOut(outputDirectory, "Loading");
  CopyLibraryOut(outputDirectory, project->ProjectContentLibrary);

  // Copy default configuration
  RelativeCopyFile(outputDirectory, appDirectory, "Configuration.data");
  
  // Copy Inno Setup Template
  CopyInstallerSetupFile(outputDirectory, mainConfig->DataDirectory, project->ProjectName);

  // Add all dlls (and other files next to the exe)
  AddFiles(appDirectory, CopyFileCallback, &outputDirectory);

  //Copy the project file
  CopyFile( FilePath::Combine(outputDirectory, "Project.zeroproj"), project->ProjectFile);
  
  // Add data directory
  String dataDirectory = mainConfig->DataDirectory;
  AddFilesHelper(dataDirectory, "Data", CopyFileCallback, &outputDirectory);

  //Copy the executable
  String outputExe = FilePath::Combine(outputDirectory, "ZeroEditor.exe");
  CopyFile( outputExe, GetApplication() );

  //Update exe's icon and banner
  HANDLE updateHandle = BeginUpdateResource(Widen(outputExe).c_str(), FALSE);

  //Update the banner bitmap
  UpdateBannerBitmap(updateHandle);

  //Update the icon
  UpdateIcon(project, updateHandle);

  //EndUpdate to commit the changes.
  BOOL success = EndUpdateResource(updateHandle, FALSE);

  Os::SystemOpenFile(outputDirectory.c_str());
}

}//namespace
