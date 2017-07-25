///////////////////////////////////////////////////////////////////////////////
///
/// \file Process.hpp
/// Declaration of the Process class and support functions.
/// 
/// Authors: Trevor Sundberg / Joshua T. Fisher / Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"
#include "Platform/Process.hpp"
#include "Platform/FilePath.hpp"

// Bring in the Process Status API library for process information parsing
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

namespace Zero
{
struct ProcessPrivateData
{
  StackHandle mStandardIn;
  StackHandle mStandardOut;
  StackHandle mStandardError;
  StackHandle mProcess;
  StackHandle mProcessThread;
};

Process::Process()
{
  ZeroConstructPrivateData(ProcessPrivateData);
}

Process::~Process()
{
  Close();
  ZeroDestructPrivateData(ProcessPrivateData);
}

inline void DuplicateAndClose(Status& status, HANDLE currentProcess, StackHandle& handle)
{
  // This handle has not been redirected, just return
  if (handle.mHandle == cInvalidHandle)
    return;

  HANDLE noninheritingHandle;

  BOOL result = DuplicateHandle(
    currentProcess,
    handle,
    currentProcess,
    &noninheritingHandle, // Address of new handle.
    0,
    FALSE, // Make it uninheritable.
    DUPLICATE_SAME_ACCESS);

  if (result == FALSE)
    WinReturnIfStatus(status);

  CloseHandle(handle);
  handle = noninheritingHandle;
}

inline void SetupStandardHandle(Status& status, StackHandle& writeHandle, StackHandle& readHandle, bool redirect)
{
  // If we are redirecting create a new pipe.
  if (redirect)
  {
    SECURITY_ATTRIBUTES security;

    // Set up the security attributes struct.
    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = TRUE;

    SetLastError(0);
    BOOL result = CreatePipe(&readHandle.mHandle, &writeHandle.mHandle, &security, 0);
    if (result == FALSE)
      WinReturnIfStatus(status);
  }
}

void Process::Start(Status& status, StringRange commandLine, bool redirectOut, bool redirectError, bool redirectIn, bool showWindow)
{
  ProcessStartInfo info;
  info.mArguments = commandLine;
  info.mShowWindow = showWindow;
  info.mRedirectStandardOutput = redirectOut;
  info.mRedirectStandardError = redirectError;
  info.mRedirectStandardInput = redirectIn;
  info.mSearchPath = false;

  Start(status, info);
}

void SetUpStartInfo(ProcessStartInfo &info)
{
  // The application name always has to be the first argument, even if it's passed
  // in as the application name. Because of this the application name (if it exists)
  // is always quoted as the first argument.
  if (info.mApplicationName.Empty() != true)
    info.mArguments = String::Format("\"%s\" %s", info.mApplicationName.c_str(), info.mArguments.c_str());

  // In order for process to search for the application, the passed in application
  // name needs to be null. In this case we've already set the application as the
  // first argument so we can just clear out the application name.
  if (info.mSearchPath == true)
    info.mApplicationName.Clear();
}

void Process::Start(Status &status, ProcessStartInfo &info)
{
  ZeroGetPrivateData(ProcessPrivateData);

  SetUpStartInfo(info);

  HANDLE currentProcess = GetCurrentProcess();

  // These are simply stack copies of what we'll keep around to monitor 
  // if the pipe has been redirected (null otherwise)
  StackHandle standardOutRead;
  StackHandle standardErrorRead;
  StackHandle standardInWrite;

  // These are used to pass the opposite pipe to our created process.
  StackHandle standardOutWrite;
  StackHandle standardErrorWrite;
  StackHandle standardInRead;

  // Set up the handles for each standard stream.
  // Make sure to clean up the unneeded ones.
  SetupStandardHandle(status, standardOutWrite, standardOutRead, info.mRedirectStandardOutput);
  WinReturnIfStatus(status);
  DuplicateAndClose(status, currentProcess, standardOutRead);
  WinReturnIfStatus(status);

  SetupStandardHandle(status, standardErrorWrite, standardErrorRead, info.mRedirectStandardError);
  WinReturnIfStatus(status);
  DuplicateAndClose(status, currentProcess, standardErrorRead);
  WinReturnIfStatus(status);

  SetupStandardHandle(status, standardInWrite, standardInRead, info.mRedirectStandardInput);
  WinReturnIfStatus(status);
  DuplicateAndClose(status, currentProcess, standardInWrite);
  WinReturnIfStatus(status);

  // Set up the struct containing the possibly redirected IO for our child process.
  STARTUPINFO startUpInfo;
  ZeroMemory(&startUpInfo, sizeof(STARTUPINFO));
  startUpInfo.cb          = sizeof(STARTUPINFO);
  startUpInfo.hStdOutput  = standardOutWrite;
  startUpInfo.hStdError   = standardErrorWrite;
  startUpInfo.hStdInput   = standardInRead;
  startUpInfo.wShowWindow = info.mShowWindow ? SW_SHOWDEFAULT : SW_HIDE;
  startUpInfo.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

  PROCESS_INFORMATION processInfo;
  ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

  WString applicationName = Widen(info.mApplicationName);
  WString arguments = Widen(info.mArguments);
  WString workingDirectory = Widen(info.mWorkingDirectory);

  if (applicationName.c_str() == nullptr && arguments.c_str() == nullptr)
  {
    status.SetFailed("Neither application name or command line parameters were specified.");
    return;
  }

  // arguments MUST be a copy of the buffer, because CreateProcess will add null terminators to the string
  SetLastError(0);
  BOOL result = CreateProcess(
    (LPCWSTR)applicationName.Data(),
    (LPWSTR)arguments.Data(),
    NULL,
    NULL,
    TRUE,
    CREATE_NEW_CONSOLE,
    NULL,
    (LPCWSTR)workingDirectory.Data(),
    &startUpInfo,
    &processInfo);

  if (result == FALSE)
  {
    FillWindowsErrorStatus(status);
    if (status.Failed())
    {
      status.Message = String::Format("Failed to create process %s. \nCommand line '%s'\n%s",
        info.mApplicationName.c_str(), info.mArguments.c_str(), status.Message.c_str());
      return;
    }
  }

  // Store the process so we can query the return value later (and wait on it closing)
  self->mProcess = processInfo.hProcess;
  self->mProcessThread = processInfo.hThread;

  // Keep around the handles to our side of the pipe.
  self->mStandardOut = standardOutRead.Transfer();
  self->mStandardError = standardErrorRead.Transfer();
  self->mStandardIn = standardInWrite.Transfer();
}

void Process::Close()
{
  ZeroGetPrivateData(ProcessPrivateData);

  self->mProcess.Close();
  self->mProcessThread.Close();
  self->mStandardIn.Close();
  self->mStandardOut.Close();
  self->mStandardError.Close();
}

void Process::Terminate()
{
  ZeroGetPrivateData(ProcessPrivateData);
  TerminateProcess(self->mProcess, (UINT)-1);
}

bool Process::IsRunning()
{
  ZeroGetPrivateData(ProcessPrivateData);
  if (self->mProcess == cInvalidHandle)
    return false;

  OsInt exitCode = 0;
  GetExitCodeProcess(self->mProcess, &exitCode);
  return (exitCode == STILL_ACTIVE);
}

int Process::WaitForClose()
{
  ZeroGetPrivateData(ProcessPrivateData);
  if (self->mProcess == cInvalidHandle)
    return -1;

  // Wait for the process to close.
  WaitForSingleObject(self->mProcess, INFINITE);

  // Return the exit code.
  OsInt exitCode = 0;
  GetExitCodeProcess(self->mProcess, &exitCode);
  return exitCode;
}

void OpenStandardStream(StackHandle& handle, File& fileStream, FileMode::Enum mode)
{
  fileStream.Open(handle, mode);
  handle = cInvalidHandle;
}

void Process::OpenStandardOut(File& fileStream)
{
  ZeroGetPrivateData(ProcessPrivateData);
  OpenStandardStream(self->mStandardOut, fileStream, FileMode::Read);
}

void Process::OpenStandardError(File& fileStream)
{
  ZeroGetPrivateData(ProcessPrivateData);
  OpenStandardStream(self->mStandardError, fileStream, FileMode::Read);
}

void Process::OpenStandardIn(File& fileStream)
{
  ZeroGetPrivateData(ProcessPrivateData);
  OpenStandardStream(self->mStandardIn, fileStream, FileMode::Write);
}

bool Process::IsStandardOutRedirected()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return self->mStandardOut == cInvalidHandle;
}

bool Process::IsStandardErrorRedirected()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return self->mStandardError == cInvalidHandle;
}

bool Process::IsStandardInRedirected()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return self->mStandardIn == cInvalidHandle;
}

/////////////////////////////////////////////////////////////////////
// Global Functions/Helpers
/////////////////////////////////////////////////////////////////////
inline void GetProcessNameAndId(DWORD processID, String& processName, String& processPath)
{
  // We can't retrieve info about some processes so default those to some string
  static const String unknownStr = "<unknown>";
  processName = processPath = unknownStr;

  TCHAR szProcessPath[MAX_PATH] = TEXT("<unknown>");

  // Get a handle to the process.
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

  // Get the process name.
  if (NULL != hProcess)
  {
    HMODULE moduleHandle;
    DWORD resultsBytesNeeded;

    // Gets a handle for each module in the process
    if (EnumProcessModules(hProcess, &moduleHandle, sizeof(moduleHandle), &resultsBytesNeeded))
    {
      // According to the exhumation's of GetModuleBaseName it is preferable (and faster) to use 
      // GetModuleFileName (which gets the full path) and the parse the text to get the process name
      GetModuleFileNameEx(hProcess, moduleHandle, szProcessPath, sizeof(szProcessPath) / sizeof(TCHAR));

      processPath = Narrow(szProcessPath);
      // Make sure to normalize the path just in-case
      processPath = FilePath::Normalize(processPath);
      // The just the process' name (e.g. ZeroEditor.exe)
      processName = FilePath::GetFileName(processPath);
    }
  }

  // Release the handle to the process.
  CloseHandle(hProcess);
}

void GetProcesses(Array<ProcessInfo>& results)
{
  // EnumProcesses requires an array of data to be filled out,
  // currently assume there's not more than 1024 processes running
  const size_t maxProcesses = 1024;
  DWORD processIds[maxProcesses], resultSizeInBytes, numberOfProcesses;
  EnumProcesses(processIds, sizeof(processIds), &resultSizeInBytes);
  numberOfProcesses = resultSizeInBytes / sizeof(DWORD);
  // Fill out information for each process
  for (size_t i = 0; i < numberOfProcesses; ++i)
  {
    // Process id of 0 is invalid
    if (processIds[i] != 0)
    {
      ProcessInfo& info = results.PushBack();
      info.mProcessId = processIds[i];
      GetProcessNameAndId(processIds[i], info.mProcessName, info.mProcessPath);
    }
  }
}

void KillProcess(OsInt processId, int exitCode)
{
  // Open the process for termination
  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);

  if (hProcess != NULL)
    TerminateProcess(hProcess, exitCode);
  CloseHandle(hProcess);
}

void RegisterApplicationRestartCommand(StringParam commandLineArgs, uint flags)
{
  RegisterApplicationRestart(nullptr, 0);
}

}