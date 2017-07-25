///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua T. Fisher, Chris Peters Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

// Include protection
#pragma once
#ifndef ZERO_PROCESS_HPP
#define ZERO_PROCESS_HPP
#include "Utility/TextStream.hpp"

namespace Zero
{
class ProcessStartInfo
{
public:
  ProcessStartInfo();

  String mApplicationName;      // Will be automatically quoted
  String mArguments;            // Will not be automatically quoted
  String mWorkingDirectory;     // Will be automatically quoted
  bool mShowWindow;             // Default: false
  bool mSearchPath;             // Default: true
  bool mRedirectStandardOutput; // Default: false
  bool mRedirectStandardError;  // Default: false
  bool mRedirectStandardInput;  // Default: false
};

//---------------------------------------------------------------------- Process
/// Process class used for managing external processes and redirecting their stdio.
/// Used to launch and monitor various external programs, compilers and tools.
class Process
{
public:
  Process();
  ~Process();

  // Begin execution of another process using the given parameters.
  void Start(Status& status, ProcessStartInfo& info);

  // Begins the execution of another process using the given parameters.
  // NOTE: commandLine cannot and will not be quoted. If the program
  //       being launched has spaces in it's path, you'll
  //       need to do it manually, or use the ProcessStartInfo
  //       version of this function.
  // NOTE: Will always search the path for the application after
  //       not finding it in the working directory.
  void Start(Status& status, StringRange commandLine, bool redirectOut = false, bool redirectError = false,
            bool redirectIn = false, bool showWindow = false);

  // Wait for the process to close (returns the exit code)
  // Do not call this after closing a process
  int WaitForClose();

  //  Returns if the process is running or not.
  bool IsRunning();

  // Close the process handle this does not force the process to exit.
  void Close();

  // Terminate the process (unsafe)
  void Terminate();

  // Opening these will allow for reading/writing through the provided filestream interface.
  // Close should close all these pipes unless they have been stolen by the below functions:
  void OpenStandardOut(File& fileStream);
  void OpenStandardError(File& fileStream);
  void OpenStandardIn(File& fileStream);

  bool IsStandardOutRedirected();
  bool IsStandardErrorRedirected();
  bool IsStandardInRedirected();
protected:
  ZeroDeclarePrivateData(Process, 32);
};


struct ProcessInfo
{
  ProcessInfo();

  /// Unique identifier for a process
  OsInt mProcessId;
  /// The name of the process by itself (e.g. ZeroEditor.exe)
  String mProcessName;
  /// The full path to the process
  String mProcessPath;
};

/// Fill out an array with all active processes.
ZeroShared void GetProcesses(Array<ProcessInfo>& results);
/// Given a process id (from the ProcessInfo struct) kill the process.
ZeroShared void KillProcess(OsInt processId, int exitCode = 1);
/// Find a process by name (returns 0 for the id and empty strings if it fails)
ZeroShared ProcessInfo FindProcess(StringParam processName);
/// Register the current application with the operating system's restart services.
/// Allows an installer to restart this application when running.
/// The application name is auto-added to the arguments. See the specific platform for flags.
ZeroShared void RegisterApplicationRestartCommand(StringParam commandLineArgs, uint flags = 0);

class SimpleProcess : public Process
{
public:
  ~SimpleProcess();

  // Begin execution of another process. All output from this process 
  // will be passed to the provided stream. This call will return immediately
  void ExecProcess(StringParam debugName, StringParam commandLine,
    TextStream* stream = nullptr, bool showWindow = false);

  // Wait for the process to close and read the rest of the output (returns the exit code)
  // Do not call this after closing a process
  int WaitForClose();

  void Cancel();

private:
  static OsInt ReadThreadEntryPoint(void* data);
  Thread mReadThread;
  volatile bool mCancel;
  TextStream* mStream;
  File mStandardOut;
  String mDebugName;
};

}//namespace Zero

#endif