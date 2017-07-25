///////////////////////////////////////////////////////////////////////////////
///
/// \file WinMain.cpp
///
/// Authors: Josh Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Platform/FileSystem.hpp"
#include "Platform/Utilities.hpp"
#include "Networking/WebRequest.hpp"
#include "Support/FileSupport.hpp"
#include "Platform/FilePath.hpp"
#include "Platform/Socket.hpp"
#include "Support/Archive.hpp"
#include "String/StringConversion.hpp"
#include "String/ToString.hpp"
#include "Win32LauncherDll/MiscHelpers.hpp"
#include "Platform/Windows/WString.hpp"
#include "Engine/BuildVersion.hpp"

#ifdef RunVld
#include <vld.h>
#endif

using namespace Zero;

const String UrlRoot = "https://builds.zeroengine.io";

//the startup function to run the version selector
typedef int(*StartupFunction)(const char* workingDir);

StartupFunction LoadDll(Zero::StringParam dllPath, HMODULE& module)
{
  //try to load the dll
  module = LoadLibrary(Widen(dllPath).c_str());
  if(module == NULL)
    return NULL;

  //return the run function
  return (StartupFunction)GetProcAddress(module, "RunZeroLauncher");
}

void DownloadPackageFromServer(Zero::StringParam downloadLauncherPackagePath, Zero::StringParam versionIdFilePath, int serverVersionId)
{
  //download the dll/content package from the server
  String majorVersionIdStr = ToString(GetLauncherMajorVersion());
  String url = BuildString(UrlRoot, "?Commands=RequestZeroLauncherPackage&MajorId=", majorVersionIdStr);
  BlockingWebRequest request;
  request.mUrl = url;
  request.mSendEvent = false;
  String data = request.Run();

  // Delete the old directory if it existed (so we don't keep any old resource files around)
  DeleteDirectory(downloadLauncherPackagePath);

  //extract the archive to the download directory
  Archive archive(ArchiveMode::Decompressing);
  ByteBufferBlock buffer((byte*)data.Data(), data.SizeInBytes(), false);
  archive.ReadBuffer(ArchiveReadFlags::All, buffer);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, downloadLauncherPackagePath);

  //save out a file that Contains the version id (so we know what the download's id is)
  String versionIdString = ToString(serverVersionId);
  WriteStringRangeToFile(versionIdFilePath, versionIdString);
}

void DownloadFromServerIfNewer(Zero::StringParam downloadPath, Zero::StringParam downloadVersionIdFilePath, int& downloadedVersionId, int serverVersionId)
{
  //if the server package is newer than the downloaded package then download from the server
  if(serverVersionId > downloadedVersionId)
  {
    DownloadPackageFromServer(downloadPath, downloadVersionIdFilePath, serverVersionId);
    //update the download id to be what was on the server
    downloadedVersionId = serverVersionId;
  }
}

String ChooseDllPath(Zero::StringParam localDllPath, int localDllVersionId, Zero::StringParam downloadedDllPath, int downloadedDllVersionId)
{
  //choose which dll path based upon which one is newer
  if(localDllVersionId > downloadedDllVersionId)
    return localDllPath;
  return downloadedDllPath;
}

//Os Specific Main
int WINAPI WinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow)
{
  WebRequestInitializer webRequestInitializer;

  int restart = 1;
  HMODULE module;

  // Register that we can be restarted. This is primarily so an installer can
  // restart the launcher if it was already running.
  RegisterApplicationRestartCommand(String(), 0);

  // Initialize platform socket library
  Zero::Status socketLibraryInitStatus;
  Zero::Socket::InitializeSocketLibrary(socketLibraryInitStatus);
  Assert(Zero::Socket::IsSocketLibraryInitialized());

  // As long as the program wants to restart keep trying to load a new dll and running it
  while(restart)
  {
    // Get the id of the dll on the server
    String majorVersionIdStr = ToString(GetLauncherMajorVersion());
    String url = BuildString(UrlRoot, "?Commands=CheckPatchId&MajorId=", majorVersionIdStr);
    BlockingWebRequest request;
    request.mUrl = url;
    request.mSendEvent = false;
    String serverDllDate = request.Run();
    int serverVersionId;
    ToValue(serverDllDate.c_str(), serverVersionId);

    String versionIdFileName = "ZeroLauncherVersionId.txt";
    String launcherDllName = "ZeroLauncherDll.dll";
    String appDir = GetApplicationDirectory();
    String downloadsRootDir = GetUserLocalDirectory();

    String localVersionPath = appDir;
    String localVersionIdPath = FilePath::Combine(localVersionPath, versionIdFileName);
    String localDllPath = FilePath::Combine(localVersionPath, launcherDllName);

    String launcherFolderName = BuildString("ZeroLauncher_", majorVersionIdStr, ".0");
    String downloadPath = FilePath::Combine(downloadsRootDir, launcherFolderName);
    String downloadedVersionIdPath = FilePath::Combine(downloadPath, versionIdFileName);
    String downloadedDllPath = FilePath::Combine(downloadPath, launcherDllName);

    int localVersionId = GetVersionId(localVersionIdPath);
    int downloadedVersionId = GetVersionId(downloadedVersionIdPath);

    //if the debugger is attached then we always want to run the
    //built version of the dll which is next to the exe
    if(Os::IsDebuggerAttached())
    {
      StartupFunction startupFunction = LoadDll(localDllPath, module);
      if(startupFunction != NULL)
      {
        restart = startupFunction(localVersionPath.c_str());
        // Make sure to free the module (otherwise the statics don't get cleaned up)
        FreeModule(module);
        continue;
      }
      
      return 1;
    }

    //if the server Contains a newer dll then download it always (even though we might not run it)
    DownloadFromServerIfNewer(downloadPath, downloadedVersionIdPath, downloadedVersionId, serverVersionId);

    //load whatever dll is newer between the installed one and the downloaded one
    //(if they install a newer version selector we want to run that dll instead of
    //the downloaded one and if the server was newer we would've already downloaded it)
    String dllDirectoryPath = ChooseDllPath(localVersionPath, localVersionId, downloadPath, downloadedVersionId);
    String dllPath = FilePath::Combine(dllDirectoryPath, launcherDllName);
    StartupFunction startupFunction = LoadDll(dllPath, module);

    //if we didn't get back a valid function pointer then the there wasn't a valid dll to run
    if(startupFunction == nullptr)
      return 1;

    //run the startup function
    restart = startupFunction(dllDirectoryPath.c_str());
    // Make sure to free the module (otherwise the statics don't get cleaned up)
    FreeModule(module);
  }

  // Uninitialize platform socket library
  Zero::Status socketLibraryUninitStatus;
  Zero::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);
  Assert(!Zero::Socket::IsSocketLibraryInitialized());

  // METAREFACTOR - What do here?
  // Because of the blocking web request (which tries to send events)
  // we need to shut down the meta database or we'll leak memory
  //MetaDatabase::Shutdown();
  return 0;
}
