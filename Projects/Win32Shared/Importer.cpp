///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Importer.hpp"
#include "Editor/Export.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/FilePath.hpp"
#include "String/ToString.hpp"
#include "Support/Archive.hpp"
#include "Platform/Utilities.hpp"
#include "../Win32Shared/resource.h"

namespace Zero
{

//------------------------------------------------------------ Importer 
const uint cEmptyPackageSize = 8;

Importer::Importer()
{
  mFile = nullptr;
  mData = nullptr;
  mSize = 0;
  mThread = nullptr;
  mAlreadyExtracted = false;
}

ImporterResult::Enum Importer::CheckForImport()
{
  // Get the package as a windows resource section
  HMODULE module = nullptr;
  Status status;
  HRSRC packRes = FindResource(module, MAKEINTRESOURCE(IDR_PACK), L"PACK");
  if(packRes != INVALID_HANDLE_VALUE)
  {
    //Get the size to see if it is the empty resource (in editor)
    //or larger
    uint packageSize = SizeofResource(module, packRes);
    if(packageSize > cEmptyPackageSize)
    {
      ZPrint("Packaged Exe Loading Resources");

      //Load the resource data
      HGLOBAL resoureData = LoadResource(module, packRes);
      byte* data = (byte*)LockResource(resoureData);
      ByteBufferBlock buffer(data, packageSize, false);

      // Read the unique export name  from the data section
      u32 uniqueNameSize = 0;
      buffer.Read(status, (byte*)&uniqueNameSize, sizeof(uniqueNameSize));
      StringNode* node = String::AllocateNode(uniqueNameSize);
      buffer.Read(status, (byte*)node->Data, node->Size);
      String uniqueName(node);

      String outputDirectory = FilePath::Combine(GetUserLocalDirectory(), uniqueName);
      CreateDirectoryAndParents(outputDirectory);
      SetWorkingDirectory(outputDirectory);
      mOutputDirectory = outputDirectory;

      // For the sake of plugins, our exe MUST be named ZeroEngine.exe
      // We solve this by copying our own executable to the output directory first,
      // run the executable there, and terminate our own
      String applicationPath = GetApplication();
      String applicationName = FilePath::GetFileName(applicationPath);
      static const String ZeroEngineExecutable("ZeroEditor.exe");
      if (applicationName != ZeroEngineExecutable)
      {
        String destinationApplication = FilePath::Combine(mOutputDirectory, ZeroEngineExecutable);
        CopyFile(destinationApplication, applicationPath);
        Zero::Os::SystemOpenFile(destinationApplication.c_str());
        return ImporterResult::ExecutedAnotherProcess;
      }

      // Our application is already named correctly, check if the zeroproj already exists
      // If so, we should probably early out and not bother extracting anything else
      // Especially since we put this in a user local directory, and not just a temporary directory
      static const String ProjectFile("Project.zeroproj");
      String projectFile = FilePath::Combine(mOutputDirectory, ProjectFile);
      if (FileExists(projectFile))
      {
        mAlreadyExtracted = true;
        return ImporterResult::Embeded;
      }

      //Extract critical blocking resources (loading, etc)
      Archive engineArchive(ArchiveMode::Decompressing);
      engineArchive.ReadBuffer(ArchiveReadFlags::All, buffer);
      engineArchive.ExportToDirectory(ArchiveExportMode::OverwriteIfNewer, outputDirectory);

      //Extract the project package in background thread
      mData = buffer.GetCurrent();
      mSize = buffer.Size() - buffer.Tell();
      mThread = new Thread();
      mThread->Initialize(Thread::ObjectEntryCreator<Importer, &Importer::DoImport>,
        this, "ImportThread");
      mThread->Resume();
      return ImporterResult::Embeded;
    }
  }
  //No package 
  return ImporterResult::NotEmbeded;
}

OsInt Importer::DoImport()
{
  ByteBufferBlock buffer(mData, mSize, false);
  Archive projectArchive(ArchiveMode::Decompressing);

  //Read all the entries
  projectArchive.ReadBuffer(ArchiveReadFlags::Enum(ArchiveReadFlags::Entries | ArchiveReadFlags::Data), buffer);
  //Only decompress and overwrite if the files are newer
  //this allows packaged exes to 'install' by running once
  projectArchive.ExportToDirectory(ArchiveExportMode::OverwriteIfNewer, mOutputDirectory);
  return 0;
}

void LoadResourcePackageRelative(StringParam baseDirectory, StringParam libraryName);

bool Importer::EngineInitialized()
{
  //Load the "Loading" package for the loading screen
  LoadResourcePackageRelative(mOutputDirectory, "Loading");
  return true;
}

bool Importer::CheckImportFinished()
{
  if(mAlreadyExtracted || mThread && mThread->IsCompleted())
  {
    SafeDelete(mThread);
    return true;
  }
  else
    return false;
}


}
