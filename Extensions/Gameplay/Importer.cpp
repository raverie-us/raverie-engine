///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Importer.hpp"

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
  // Get the package as an executable resource
  ByteBufferBlock buffer;
  if (GetExecutableResource(gPackName, gPackType, buffer))
  {
    //Get the size to see if it is the empty resource (in editor)
    //or larger
    if(buffer.Size() > cEmptyPackageSize)
    {
      ZPrint("Packaged Exe Loading Resources");

      // Read the unique export name  from the data section
      Status status;
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
