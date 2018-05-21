///////////////////////////////////////////////////////////////////////////////
///
/// \file WinMain.cpp
/// Entry Point and Initialization
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//Application Startup Function
bool Startup(Engine* engine, StringMap& parameters);

}

void DebugRunEngine(void* voidEngine)
{
  Zero::Engine* engine = (Zero::Engine*)voidEngine;
  engine->Run();
}

using namespace Zero;

ZeroGuiMain()
{
  //Set the log and error handlers so debug printing
  //and asserts will print to the Visual Studio Output Window.
  DebuggerListener debuggerOutput;
  if(Os::IsDebuggerAttached())
    Zero::Console::Add(&debuggerOutput);

  //Mirror console output to a log file
  FileListener fileListener;
  Zero::Console::Add(&fileListener);

  TimerBlock totalEngineTimer("Total engine run time:");

  //This assert will bring up a dialog box.
  ErrorSignaler::SetErrorHandler(Os::ErrorProcessHandler);

  //Enable the crash handler
  CrashHandler::Enable();
  CrashHandler::SetPreMemoryDumpCallback(Zero::CrashPreMemoryDumpCallback, NULL);
  CrashHandler::SetCustomMemoryCallback(Zero::CrashCustomMemoryCallback, NULL);
  CrashHandler::SetLoggingCallback(Zero::CrashLoggingCallback, &fileListener);
  CrashHandler::SetSendCrashReportCallback(Zero::SendCrashReport, NULL);
  CrashHandler::SetCrashStartCallback(Zero::CrashStartCallback, NULL);

  // Get the command line
  Array<String> commandLineArray;
  GetCommandLineStringArray(commandLineArray);

  // Initialize environments
  Environment* environment = Environment::GetInstance();
  environment->ParseCommandArgs(commandLineArray);

  // Add stdout listener (requires engine initialization to get the Environment object)
  StdOutListener stdoutListener;
  if(!environment->GetParsedArgument("logStdOut").Empty())
    Zero::Console::Add(&stdoutListener);

  // Importer is used 
  Importer import;
  ImporterResult::Enum importResult = import.CheckForImport();
  if(importResult == ImporterResult::ExecutedAnotherProcess)
    return 1;

  // Startup the engine
  ZeroStartupSettings settings;
  settings.mTweakableFileName = "EditorTweakables";
  settings.mEmbeddedPackage = (importResult == ImporterResult::Embeded);
  settings.mEmbeddedWorkingDirectory = import.mOutputDirectory;

  ZeroStartup startup;
  Engine* engine = startup.Initialize(settings);

  //Run application specific startup
  bool success = Zero::Startup(engine, environment->mParsedCommandLineArguments);

  //Failed startup do not run
  if(!success)
    return 1;

  //Use this line to test the crash handler running in
  //visual studio and remove the line below
  //Zero::RunEngine(DebugRunEngine, engine);
  
  //Run engine until termination
  engine->Run();

  startup.Shutdown();

  //Assert(!Zero::Socket::IsSocketLibraryInitialized());

  // Uninitialize platform socket library
  Zero::Status socketLibraryUninitStatus;
  Zero::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);
  //Assert(!Zero::Socket::IsSocketLibraryInitialized());

  ZPrint("Terminated\n");

  return 0;
}
