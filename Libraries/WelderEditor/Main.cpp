// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);

  // Set the log and error handlers so debug printing
  // and asserts will print to the Visual Studio Output Window.
  DebuggerListener debuggerOutput;
  Zero::Console::Add(&debuggerOutput);

  FileSystemInitializer fileSystemInitializer(
      &PopulateVirtualFileSystemWithZip);

  // Mirror console output to a log file
  FileListener fileListener;
  Zero::Console::Add(&fileListener);

  TimerBlock totalEngineTimer("Total engine run time:");

  // This assert will bring up a dialog box.
  ErrorSignaler::SetErrorHandler(Os::ErrorProcessHandler);

  // Enable the crash handler
  CrashHandler::Enable();
  CrashHandler::SetPreMemoryDumpCallback(Zero::CrashPreMemoryDumpCallback,
                                         NULL);
  CrashHandler::SetCustomMemoryCallback(Zero::CrashCustomMemoryCallback, NULL);
  CrashHandler::SetLoggingCallback(Zero::CrashLoggingCallback, &fileListener);
  CrashHandler::SetSendCrashReportCallback(Zero::SendCrashReport, NULL);
  CrashHandler::SetCrashStartCallback(Zero::CrashStartCallback, NULL);

  Importer importer;
  ImporterResult::Type importResult = importer.CheckForImport();
  if (importResult == ImporterResult::ExecutedAnotherProcess)
    return 1;

  // Initialize environments
  Environment* environment = Environment::GetInstance();
  environment->ParseCommandArgs(gCommandLineArguments);

  // Add stdout listener (requires engine initialization to get the Environment
  // object)
  StdOutListener stdoutListener;
  if (!environment->GetParsedArgument("logStdOut").Empty())
    Zero::Console::Add(&stdoutListener);

  String appDirectory = GetApplicationDirectory();
  String projectFile = FilePath::Combine(appDirectory, "Project.zeroproj");

  // Fix the project file path for exports to be in the import's output
  // directory
  bool embededPackage = (importResult == ImporterResult::Embeded);
  if (embededPackage)
    projectFile =
        FilePath::Combine(importer.mOutputDirectory, "Project.zeroproj");

  // Startup the engine
  ZeroStartupSettings settings;
  settings.mTweakableFileName = "EditorTweakables";
  settings.mEmbeddedPackage = embededPackage;
  settings.mEmbeddedWorkingDirectory = importer.mOutputDirectory;

  ZeroStartup startup;
  Engine* engine = startup.Initialize(settings);

  // Run application specific startup
  bool success = Zero::Startup(
      engine, environment->mParsedCommandLineArguments, projectFile);

  // Failed startup do not run
  if (!success)
    return 1;

  // Run engine until termination
  engine->Run();

  startup.Shutdown();

  // Assert(!Zero::Socket::IsSocketLibraryInitialized());

  // Uninitialize platform socket library
  Zero::Status socketLibraryUninitStatus;
  Zero::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);
  // Assert(!Zero::Socket::IsSocketLibraryInitialized());

  ZPrint("Terminated\n");

  return 0;
}
