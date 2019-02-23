// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);

  SetupApplication(
      1, 0, 0, 1, sWelderOrganization, sLauncherGuid, sLauncherName);

  LauncherStartup startup;
  Engine* engine = startup.Initialize();

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
    return 0;
  }

  CrashHandler::SetRestartCommandLine(Environment::GetInstance()->mCommandLine);

  startup.Startup();

  // Run application startup
  bool success = Zero::ZeroLauncherStartup();
  // Failed startup do not run
  if (!success)
    return 0;

  // Return code of 0 means don't restart the launcher
  int returnCode = 0;
  // Run engine until termination
  engine->Run(false);

  // Check to see if we need to restart after we close (in order to update)
  // and if so change the return code to tell the exe.
  Cog* configCog = engine->GetConfigCog();
  LauncherConfig* config = configCog->has(LauncherConfig);
  if (config->mRestartOnClose)
    returnCode = 1;

  engine->Shutdown();
  // Free the launcher
  SafeDelete(Z::gLauncher);

  startup.Shutdown();

  ZPrint("Terminated\n");

  return returnCode;
}
