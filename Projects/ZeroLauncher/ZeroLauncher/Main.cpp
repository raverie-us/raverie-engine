///////////////////////////////////////////////////////////////////////////////
///
/// \file WinMain.cpp
///
/// Authors: Josh Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String UrlRoot = "https://builds.zeroengine.io";

//the startup function to run the version selector
typedef int(*StartupFunction)(const char* workingDir);

StartupFunction LoadDll(Zero::StringParam libraryPath, ExternalLibrary& library)
{
  Status status;
  library.Load(status, libraryPath.c_str());

  if (status.Failed())
    return nullptr;

  //return the run function
  return (StartupFunction)library.GetFunctionByName("RunZeroLauncher");
}

Zero::String GetLauncherDownloadedPath()
{
  String majorVersionIdStr = ToString(GetLauncherMajorVersion());
  String launcherFolderName = BuildString("ZeroLauncher_", majorVersionIdStr, ".0");
  String searchLocation = FilePath::Combine(GetUserLocalDirectory(), launcherFolderName);

  String foundPath;
  int bestId = -1;
  // Find any directory that is just a number
  FileRange range(searchLocation);
  for(; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.FrontEntry();
    if(!DirectoryExists(entry.GetFullPath()))
      continue;

    int id = -1;
    ToValue(entry.mFileName, id);

    // Keep the largest number
    if(id > bestId)
    {
      bestId = id;
      foundPath = entry.GetFullPath();
    }
  }
  return foundPath;
}

String ChooseDllPath(Zero::StringParam localDllPath, int localDllVersionId, Zero::StringParam downloadedDllPath, int downloadedDllVersionId)
{
  if (downloadedDllPath.Empty())
    return localDllPath;

  //choose which dll path based upon which one is newer
  if(localDllVersionId > downloadedDllVersionId)
    return localDllPath;
  return downloadedDllPath;
}

int PlatformMain(const Array<String>& arguments)
{
  FileSystemInitializer fileSystemInitializer(&EngineLibrary::PopulateVirtualFileSystem);

  int restart = 1;
  ExternalLibrary library;

  // Register that we can be restarted. This is primarily so an installer can
  // restart the launcher if it was already running.
  RegisterApplicationRestartCommand(String(), 0);

  // As long as the program wants to restart keep trying to load a new dll and running it
  while(restart)
  {
    String versionIdFileName = "ZeroLauncherVersionId.txt";
    String launcherDllName = "ZeroLauncherSharedLibrary.dll";
    String appDir = GetApplicationDirectory();

    String localVersionPath = appDir;
    String localVersionIdPath = FilePath::Combine(localVersionPath, versionIdFileName);
    String localDllPath = FilePath::Combine(localVersionPath, launcherDllName);

    String downloadPath = GetLauncherDownloadedPath();
    String downloadedVersionIdPath;
    // If there was no download available then leave the version id path as an empty string.
    // This will cause us to fail to open the file and get back a default id value.
    if(!downloadPath.Empty())
      downloadedVersionIdPath = FilePath::Combine(downloadPath, versionIdFileName);

    int localVersionId = GetVersionId(localVersionIdPath);
    int downloadedVersionId = GetVersionId(downloadedVersionIdPath);

    //if the debugger is attached then we always want to run the
    //built version of the dll which is next to the exe
    if(Os::IsDebuggerAttached())
    {
      StartupFunction startupFunction = LoadDll(localDllPath, library);
      if (startupFunction != nullptr)
      {
        restart = startupFunction(localVersionPath.c_str());
        // Make sure to free the module (otherwise the statics don't get cleaned up)
        library.Unload();
        continue;
      }
      
      return 1;
    }

    //load whatever dll is newer between the installed one and the downloaded one
    //(if they install a newer version selector we want to run that dll instead of
    //the downloaded one and if the server was newer we would've already downloaded it)
    String dllDirectoryPath = ChooseDllPath(localVersionPath, localVersionId, downloadPath, downloadedVersionId);
    String dllPath = FilePath::Combine(dllDirectoryPath, launcherDllName);
    StartupFunction startupFunction = LoadDll(dllPath, library);

    //if we didn't get back a valid function pointer then the there wasn't a valid dll to run
    if(startupFunction == nullptr)
      return 1;

    //run the startup function
    restart = startupFunction(dllDirectoryPath.c_str());
    // Make sure to free the module (otherwise the statics don't get cleaned up)
    library.Unload();
  }

  // METAREFACTOR - What do here?
  // Because of the blocking web request (which tries to send events)
  // we need to shut down the meta database or we'll leak memory
  //MetaDatabase::Shutdown();
  return 0;
}
} // namespace Zero
