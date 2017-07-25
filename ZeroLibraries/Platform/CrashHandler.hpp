///////////////////////////////////////////////////////////////////////////////
///
/// \file CrashHandler.hpp
/// Declaration of the CrashHandler class.
///
/// Authors: Trevor Sundberg, Joshua Davis
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/StringBuilder.hpp"

namespace Zero
{

class Engine;

// For debugging, this is so the crash handler can be called within visual studio.
// This can't know about the engine though, so it has to take a callback that calls run on the engine...
typedef void (*RunEngineFunction)(void* engine);
void RunEngine(RunEngineFunction runFn, void* engine);

// Wraps parameters being passed to the crash handler.
// Maybe change this to some interface type?
class CrashHandlerParameters
{
public:
  void AddParameter(cstr name, cstr value);

  String GetParameterString();

private:
  StringBuilder mParameters;
};

// Denotes a memory range that is used to Insert extra memory into a crash dump.
struct MemoryRange
{
  byte* Begin;
  size_t Length;
};

// Startup information needed by the crash handler to find certain files.
struct CrashInfo
{
  CrashInfo()
  {
    mDumpName = "ZeroDump.dmp";
    mLogName = "ZeroLog.txt";
    mStackName = "ZeroStack.txt";
    mModuleName = "ZeroEditor";
    mStripModules = false;
  }

  // The names of the files to open/send.
  // In zero these have a date/time-stamp which is set in the CrashStartCallback.
  String mDumpName;
  String mLogName;
  String mStackName;

  // Do we strip modules? If so we include ntdll and whatever is in mModuleName.
  // Make sure to set mModuleName to the name of your program if you set mStripModules to true!
  bool mStripModules;
  String mModuleName;
};

struct CrashHandler
{
  // Enable the crash handler to start catching hardware exceptions. When a crash happens,
  // mRunCrashHandlerCallback will be called to control all of the crash handler logic.
  static void Enable();

  // If there are any extra locations to search for symbols then they need to be added to a path internal to the stack walker.
  // For instance, the launcher needs to add the path to the dll that is actually being run (since crash handler only checks next to the .exe).
  static void AppendToExtraSymbolPath(StringParam path);

  typedef void (*RunCrashHandlerCallback)(void* crashData, bool doRescueCall, void* userData);
  // Called when a crash happens. This function controls all of the behavior of the crash handler,
  // including making the minidump and sending off the crash information.
  static void SetRunCrashHandlerCallback(RunCrashHandlerCallback callback, void* userData);

  typedef void (*CrashStartCallback)(CrashInfo&, void* userData);
  // The crash start callback is used primarily to get the dump/log/stack file name.
  // Any other initial setup for the crash handler can also be performed here.
  static void SetCrashStartCallback(CrashStartCallback callback, void* userData);

  typedef void (*PreMemoryDumpCallback)(void* userData);
  // Called before the memory dump is run. This allows for any setup before the CustomMemoryCallback is called.
  static void SetPreMemoryDumpCallback(PreMemoryDumpCallback callback, void* userData);

  typedef bool (*CustomMemoryCallback)(MemoryRange& memoryRange, void* userData);
  // Used to inject a range of memory manually into the crash dump.
  // This is a re-entrant call that will be continually called until false is returned.
  // Used currently to put Zilch op-code into the dump for debugging.
  // Can be used for any bit of memory used to make debugging easier (Strings?).
  static void SetCustomMemoryCallback(CustomMemoryCallback callback, void* userData);

  typedef void (*LoggingCallback)(CrashHandlerParameters& params, CrashInfo& info, void* userData);
  // Perform any logging before the crash report is sent.
  // This may need to flush any log files as well as add the log file's name to the parameters.
  static void SetLoggingCallback(LoggingCallback callback, void* userData);

  typedef void (*SendCrashReportCallback)(CrashHandlerParameters& params, void* userData);
  // Send the crash report out somehow. The params should contain all
  // of the files/parameters to properly invoke the crash reporter.
  static void SetSendCrashReportCallback(SendCrashReportCallback callback, void* userData);

  typedef void (*FinalRescueCall)(void* userData);
  // The rescue call happens after the crash report is sent so we can attempt to
  // rescue any user settings/data (such as modified levels or files).
  static void SetupRescueCallback(FinalRescueCall rescueCall, void* userData);

  // These functions invoke the various callbacks while wrapping each
  // call in the platform specific __try __except exceptions handlers for safety.
  static void InvokeCrashStartCallback(CrashInfo& info);
  static void InvokePreMemoryDumpCallback();
  static void WriteMiniDump(CrashHandlerParameters& params, void* crashData, CrashInfo& info);
  static void InvokeWriteCallstack(CrashHandlerParameters& params, void* crashData, CrashInfo& info);
  static void InvokeLoggingCallback(CrashHandlerParameters& params, CrashInfo& info);
  static void InvokeRescueCallback();
  static void InvokeSendCrashReport(CrashHandlerParameters& params);

  // Used to signal that a fatal error has happened and send out a crash report.
  // No minidump will be generated as there was no hardware exception,
  // but stack and log information will be sent. Also doesn't invoke the rescue callback.
  static void FatalError(int errorCode);
  
  // The default logic for the crash handler.
  static void DefaultRunCrashHandlerCallback(void* crashData, bool doRescueCall, void* userData);
  static void SetRestartCommandLine(StringRange commandLine);
  static void RestartOnCrash(bool state);

  static String mExtraSymbolPath;

  static RunCrashHandlerCallback mRunCrashHandlerCallback;
  static void* mRunCrashHandlerUserData;
  static CrashStartCallback mCrashStartCallback;
  static void* mCrashStartUserData;
  static PreMemoryDumpCallback mPreMemoryDumpCallback;
  static void* mPreMemoryDumpUserData;
  static CustomMemoryCallback mCustomMemoryCallback;
  static void* mCustomMemoryUserData;
  static LoggingCallback mLoggingCallback;
  static void* mLoggingUserData;
  static SendCrashReportCallback mSendCrashReportCallback;
  static void* mSendCrashReportUserData;
  static FinalRescueCall mRescueCallback;
  static void* mRescueUserData;

  // Do we auto restart the program instead of sending a crash report?
  static bool mAutoRestart;
  // The command line to use to restart ourself.
  static String mRestartCommandLine;
};

}//namespace Zero
