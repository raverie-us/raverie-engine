// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
void LauncherStartup::UserInitializeLibraries()
{
  LauncherLibrary::Initialize();
}

void LauncherStartup::UserInitializeConfig(Cog* configCog)
{
  // Force certain config components to exist. There's a few upgrade cases
  // where one of these could be missing otherwise.
  HasOrAdd<TextEditorConfig>(configCog);
  HasOrAdd<RecentProjects>(configCog);

  LauncherConfig* versionConfig = HasOrAdd<LauncherConfig>(configCog);
  versionConfig->mLauncherLocation = GetApplication();

  // Apply any command line arguments (mostly auto-run settings).
  versionConfig->ApplyCommandLineArguments();
}

void LauncherStartup::UserInitialize()
{
  // Check and see if another launcher is already open (has to happen after
  // startup)
  Status status;
  String mutexId = BuildString("ZeroLauncherMutex:{", GetGuidString(), "}");
  InterprocessMutex mutex;
  mutex.Initialize(status, mutexId.c_str(), true);
  if (status.Failed())
  {
    ZPrint("Mutex is already open. Sending a message to the open launcher and "
      "closing\n");
    Zero::LauncherSingletonCommunication communicator;
    return Exit(0);
  }

  CrashHandler::SetRestartCommandLine(Environment::GetInstance()->mCommandLine);

  mWindowSize = IntVec2(1024, 595);
  mMinimumWindowSize = IntVec2(1024, 595);
  mWindowCentered = true;
  mWindowState = WindowState::Windowed;
}

void LauncherStartup::UserStartup()
{
  Array<String> coreLibs;
  coreLibs.PushBack("ZeroCore");
  coreLibs.PushBack("ZeroLauncherResources");
  LoadCoreContent(coreLibs);
}

void LauncherStartup::UserCreation()
{
  Z::gLauncher = new Launcher(mMainWindow);
}

void LauncherStartup::UserShutdown()
{
  // Check to see if we need to restart after we close (in order to update)
  // and if so change the return code to tell the exe.
  Cog* configCog = Z::gEngine->GetConfigCog();
  LauncherConfig* config = configCog->has(LauncherConfig);
  if (config->mRestartOnClose)
    sReturnCode = 1;

  SafeDelete(Z::gLauncher);
}

void LauncherStartup::UserShutdownLibraries()
{
  LauncherLibrary::Shutdown();
  LauncherLibrary::GetInstance().ClearLibrary();
}

} // namespace Zero
