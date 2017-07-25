///////////////////////////////////////////////////////////////////////////////
///
/// \file WinMain.cpp
/// Windows Os Entry Point and Initialization
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "WindowsShell/WindowsSystem.hpp"
#include "WindowsShell/WinUtility.hpp"
#include "Platform/CommandLineSupport.hpp"
#include "Platform/Windows/WString.hpp"
#include "../Win32Shared/Importer.hpp"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include "ZeroCrashCallbacks.hpp"

#ifdef RunVld
#include <vld.h>
#endif

namespace Zero
{

void EnableMemoryLeakChecking(int breakAlloc = -1)
{
#ifdef _MSC_VER
  //Set the leak checking flag
  int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpDbgFlag);

  //If a valid break alloc provided set the breakAlloc
  if(breakAlloc!=-1) _CrtSetBreakAlloc(breakAlloc);
#endif
}

bool ErrorMessageBox(Zero::ErrorSignaler::ErrorData& errorData)
{
  const size_t bufferSize = 4096;
  wchar_t OutputBuffer[bufferSize];

  //Print into buffer
  ZeroSWPrintf(OutputBuffer, bufferSize, L"%s(%d) : %s\n", 
    Widen(errorData.File).c_str(), errorData.Line, Widen(errorData.Message).c_str());

  //Print the message
  Console::Print(Filter::ErrorFilter, Narrow(OutputBuffer).c_str());

  //Message box
  MessageBoxW(NULL, OutputBuffer, L"Zero Error", 0);
  return true;
}

//Application Startup Function
bool Startup(Engine* engine, StringMap& parameters);

}

void DebugRunEngine(void* voidEngine)
{
  Zero::Engine* engine = (Zero::Engine*)voidEngine;
  engine->Run();
}

using namespace Zero;

//Os Specific Main
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow)
{
  WebRequestInitializer webRequestInitializer;

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
  //ErrorSignaler::SetErrorHandler(ErrorMessageBox);
  //Used custom dialog box
  ErrorSignaler::SetErrorHandler(WindowsErrorProcessHandler);

  //Enable the crash handler
  CrashHandler::Enable();
  CrashHandler::SetPreMemoryDumpCallback(Zero::CrashPreMemoryDumpCallback, NULL);
  CrashHandler::SetCustomMemoryCallback(Zero::CrashCustomMemoryCallback, NULL);
  CrashHandler::SetLoggingCallback(Zero::CrashLoggingCallback, &fileListener);
  CrashHandler::SetSendCrashReportCallback(Zero::SendCrashReport, NULL);
  CrashHandler::SetCrashStartCallback(Zero::CrashStartCallback, NULL);

  // Get the command line
  int numArguments = 0;
  wchar_t** commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &numArguments);
  Array<String> commandLineArray;
  CommandLineToStringArray(commandLineArray, commandLineArgs, numArguments);
  
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
