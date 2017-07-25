///////////////////////////////////////////////////////////////////////////////
///
/// \file CrashHandler.cpp
/// Implementation of the CrashHandler class.
///
/// Authors: Trevor Sundberg, Joshua Davis
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#pragma warning(push)
#pragma warning(disable:4091)
#include <DbgHelp.h>
#pragma warning(pop)

//#define OUTPUT_MODULES_AND_OTHER

#pragma comment(lib, "dbghelp.lib")

namespace Zero
{

// Provide a custom stack walker
class CustomStackWalker : public StackWalker
{
public:
  StringBuilder Stack;
  StringBuilder Modules;
  StringBuilder Other;

  void SetExtraSymbolPath(LPCSTR symPath)
  {
    m_szSymPath = _strdup(symPath);
  }

  void OnOutput(LPCSTR szText) override
  {
    // If it's a stack item...
    if (strstr(szText, STACK_TEXT) != NULL)
    {
      Stack.Append(szText + strlen(STACK_TEXT));
    }
#if OUTPUT_MODULES_AND_OTHER
    // If it's a module item...
    else if (strstr(szText, MODULE_TEXT) != NULL)
    {
      Modules.Append(szText + strlen(MODULE_TEXT);
    }
    // It's something else...
    else
    {
      Other.Append(szText);
    }
#endif
  }

  // Get the final output result
  String GetFinalOutput()
  {
    StringBuilder final;
    final.Append(Stack.ToString());
    final.Append("\r\n");
    final.Append(Other.ToString());
    final.Append("\r\n");
    final.Append(Modules.ToString());
    return final.ToString();
  }
};

// Some code I found to manually create an EXCEPTIONS_POINTERS struct. This is used only when we
// manually invoke a FatalError (since there's no exception there was no exception information generated automatically)
// http://www.codeproject.com/Articles/207464/Exception-Handling-in-Visual-Cplusplus

#ifndef _AddressOfReturnAddress
// Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use 
EXTERNC void * _AddressOfReturnAddress(void);
EXTERNC void * _ReturnAddress(void);
#endif

void GetExceptionPointers(DWORD dwExceptionCode,
  EXCEPTION_POINTERS** ppExceptionPointers)
{
  // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

  EXCEPTION_RECORD ExceptionRecord;
  CONTEXT ContextRecord;
  memset(&ContextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_
  __asm {
    mov dword ptr[ContextRecord.Eax], eax
      mov dword ptr[ContextRecord.Ecx], ecx
      mov dword ptr[ContextRecord.Edx], edx
      mov dword ptr[ContextRecord.Ebx], ebx
      mov dword ptr[ContextRecord.Esi], esi
      mov dword ptr[ContextRecord.Edi], edi
      mov word ptr[ContextRecord.SegSs], ss
      mov word ptr[ContextRecord.SegCs], cs
      mov word ptr[ContextRecord.SegDs], ds
      mov word ptr[ContextRecord.SegEs], es
      mov word ptr[ContextRecord.SegFs], fs
      mov word ptr[ContextRecord.SegGs], gs
      pushfd
      pop[ContextRecord.EFlags]
  }
  ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
  ContextRecord.Eip = (ULONG)_ReturnAddress();
  ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
  ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress() - 1);
#elif defined (_IA64_) || defined (_AMD64_)
  /* Need to fill up the Context in IA64 and AMD64. */
  RtlCaptureContext(&ContextRecord);
#else  /* defined (_IA64_) || defined (_AMD64_) */
  ZeroMemory(&ContextRecord, sizeof(ContextRecord));
#endif  /* defined (_IA64_) || defined (_AMD64_) */
  ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));
  ExceptionRecord.ExceptionCode = dwExceptionCode;
  ExceptionRecord.ExceptionAddress = _ReturnAddress();

  EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
  memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
  CONTEXT* pContextRecord = new CONTEXT;
  memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));
  *ppExceptionPointers = new EXCEPTION_POINTERS;
  (*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
  (*ppExceptionPointers)->ContextRecord = pContextRecord;
}

cstr GetExceptionCode(EXCEPTION_POINTERS* pException)
{
  if(pException == NULL)
    return "Terminated by Application Error";

  DWORD exceptionCode = pException->ExceptionRecord->ExceptionCode;
  cstr codeName = GetWindowsExceptionCode(exceptionCode);
  if(codeName)
    return codeName;
  else
    return "Unknown Exception Code";
}

bool InvokeCustomMemoryCallback(MemoryRange& memoryRange)
{
  __try
  {
    //assumed to not be NULL, otherwise this wouldn't be called
    return CrashHandler::mCustomMemoryCallback(memoryRange, CrashHandler::mCustomMemoryUserData);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    return false;
  }
}

bool IsDataSectionNeeded(const WCHAR* pModuleName, CrashInfo* info)
{
  if(pModuleName == NULL)
    return false;

  // Extract the module name
  WCHAR szFileName[_MAX_FNAME] = L"";
  _wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

  //convert to wide character
  const OsInt cFileNameBufferSize = MAX_PATH;
  WCHAR wideFilename[MAX_PATH];
  for(uint i = 0; i < info->mModuleName.SizeInBytes(); ++i)
  {
    WCHAR value = (WCHAR)info->mModuleName.Data()[i];
    if(info->mModuleName.Data()[i] >= 128)
      value = (WCHAR)'?';

    wideFilename[i] = value;
  }
  wideFilename[info->mModuleName.SizeInBytes()] = NULL;

  // Compare the name with the list of known names and decide
  // Note: For this to work, the executable name must be "mididump.exe"
  if(_wcsicmp(szFileName, wideFilename) == 0)
    return true;
  else if(_wcsicmp(szFileName, L"ntdll") == 0)
    return true;

  return false;
}

BOOL CALLBACK TestZeroMiniDumpCallback(
  PVOID                            pParam,
  const PMINIDUMP_CALLBACK_INPUT   pInput,
  PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{
  // Check parameters
  if(pInput == NULL)
    return FALSE;
  if(pOutput == NULL)
    return FALSE;

  CrashInfo* info = (CrashInfo*)pParam;

  // Process the callbacks
  switch(pInput->CallbackType)
  {
    case IncludeModuleCallback:
    {
      // Include the module into the dump
      return TRUE;
    }
    case IncludeThreadCallback:
    {
      // Include the thread into the dump
      return TRUE;
    }
    case ModuleCallback:
    {
      // Are data sections available for this module ?
      if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
      {
        // Yes, they are, but do we need them?
        if(info->mStripModules == true && !IsDataSectionNeeded(pInput->Module.FullPath, info))
        {
          // This print seems to sometimes break the mini dump...
          //printf(L"Excluding module data sections: %s \n", pInput->Module.FullPath);

          pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
          //pOutput->ModuleWriteFlags = 0;
        }
      }
      return TRUE;
    }
    case ThreadCallback:
    {
      // Include all thread information into the mini dump
      return TRUE;
    }
    case ThreadExCallback:
    {
      // Include this information
      return TRUE;
    }
    case MemoryCallback:
    {
      // If we don't have a custom memory callback then return that there's nothing to add
      if(CrashHandler::mCustomMemoryCallback == NULL)
        return FALSE;

      MemoryRange memRange;
      memRange.Begin = NULL;
      bool shouldContinue = true;
      //If null is returned windows interprets this as stopping even though we return that we should continue. 
      //In this case continue looping until they give us valid memory or they say we should stop.
      while(memRange.Begin == NULL && shouldContinue == true)
        shouldContinue = InvokeCustomMemoryCallback(memRange);

      if(shouldContinue == false)
        return FALSE;

      pOutput->MemoryBase = (ULONG64)memRange.Begin;
      pOutput->MemorySize = (ULONG)memRange.Length;
      return TRUE;
    }

    case CancelCallback:
      return FALSE;
  }

  return FALSE;
}

LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* pException)
{
  CrashHandler::mRunCrashHandlerCallback(pException, true, CrashHandler::mRunCrashHandlerUserData);

  // Tell the exception handler that it should continue (we've done our work here!)
  return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI LogException(EXCEPTION_POINTERS* pException)
{
  ExceptionFilter(pException);
  return ExceptionContinueSearch;
}

void RunEngine(RunEngineFunction runFn, void* engine)
{
  __try
  {
    runFn(engine);
  }
  __except(LogException(GetExceptionInformation()))
  {

  }
};

//-------------------------------------------------------------------CrashHandlerParameters
void CrashHandlerParameters::AddParameter(cstr name, cstr value)
{
  mParameters.Append("\"--");
  mParameters.Append(name);
  mParameters.Append("\" ");
  mParameters.Append("\"");
  mParameters.Append(value);
  mParameters.Append("\" ");
}

String CrashHandlerParameters::GetParameterString()
{
  return mParameters.ToString();
}

// Courtesy of Bruce Dawson (https://randomascii.wordpress.com/2012/07/05/when-even-crashing-doesnt-work/).
// Deals with exceptions being swallowed across kernel boundaries.
void EnableCrashingOnCrashes()
{
  typedef BOOL(WINAPI *tGetPolicy)(LPDWORD lpFlags);
  typedef BOOL(WINAPI *tSetPolicy)(DWORD dwFlags);
  const DWORD EXCEPTION_SWALLOWING = 0x1;

  HMODULE kernel32 = LoadLibraryA("kernel32.dll");
  tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
    "GetProcessUserModeExceptionPolicy");
  tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
    "SetProcessUserModeExceptionPolicy");
  if(pGetPolicy && pSetPolicy)
  {
    DWORD dwFlags;
    if(pGetPolicy(&dwFlags))
    {
      // Turn off the filter
      pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
    }
  }
}


//-------------------------------------------------------------------CrashHandler
void CrashHandler::Enable()
{
  EnableCrashingOnCrashes();

  // Set the error modes
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

  // Set the unhandled exception filter
  SetUnhandledExceptionFilter(ExceptionFilter);
}

void CrashHandler::AppendToExtraSymbolPath(StringParam path)
{
  if(mExtraSymbolPath.Empty())
    mExtraSymbolPath = path;
  else
    mExtraSymbolPath = BuildString(mExtraSymbolPath, ";", path);
}

void CrashHandler::SetCrashStartCallback(CrashStartCallback callback, void* userData)
{
  mCrashStartCallback = callback;
  mCrashStartUserData = userData;
}

void CrashHandler::SetRunCrashHandlerCallback(RunCrashHandlerCallback callback, void* userData)
{
  mRunCrashHandlerCallback = callback;
  mRunCrashHandlerUserData = userData;
}

void CrashHandler::SetPreMemoryDumpCallback(PreMemoryDumpCallback callback, void* userData)
{
  mPreMemoryDumpCallback = callback;
  mPreMemoryDumpUserData = userData;
}

void CrashHandler::SetCustomMemoryCallback(CustomMemoryCallback callback, void* userData)
{
  mCustomMemoryCallback = callback;
  mCustomMemoryUserData = userData;
}

void CrashHandler::SetLoggingCallback(LoggingCallback callback, void* userData)
{
  mLoggingCallback = callback;
  mLoggingUserData = userData;
}

void CrashHandler::SetupRescueCallback(FinalRescueCall rescueCall, void* userData)
{
  mRescueCallback = rescueCall;
  mRescueUserData = userData;
}

void CrashHandler::SetSendCrashReportCallback(SendCrashReportCallback callback, void* userData)
{
  mSendCrashReportCallback = callback;
  mSendCrashReportUserData = userData;
}

void CrashHandler::InvokeCrashStartCallback(CrashInfo& info)
{
  __try
  {
    if(CrashHandler::mCrashStartCallback != NULL)
      CrashHandler::mCrashStartCallback(info, CrashHandler::mCrashStartUserData);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {

  }
}

void CrashHandler::InvokePreMemoryDumpCallback()
{
  __try
  {
    if(CrashHandler::mPreMemoryDumpCallback != NULL)
      CrashHandler::mPreMemoryDumpCallback(CrashHandler::mPreMemoryDumpUserData);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {

  }
}

void CrashHandler::WriteMiniDump(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
  EXCEPTION_POINTERS* pException = (EXCEPTION_POINTERS*)crashData;

  //If crashed write a mini dump
  if(pException != NULL && pException->ExceptionRecord != NULL)
  {
    const size_t MAX_TEMP_PATH = MAX_PATH - 14;
    wchar_t dumpFileName[MAX_PATH] = {0};

    // Create a temporary file name
    DWORD pathLength = GetTempPath(MAX_TEMP_PATH, dumpFileName);
    ZeroStrCatW(dumpFileName, MAX_PATH, Widen(info.mDumpName).c_str());

    // Set up the exception information
    MINIDUMP_EXCEPTION_INFORMATION ExceptionParam = {0};
    ExceptionParam.ThreadId = GetCurrentThreadId();
    ExceptionParam.ExceptionPointers = pException;
    ExceptionParam.ClientPointers = FALSE;

    MINIDUMP_CALLBACK_INFORMATION mci;
    mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)TestZeroMiniDumpCallback; 
    // Send the crash info as context so we can know which modules to include
    mci.CallbackParam       = &info;

    // Create the handle for the file
    HANDLE fileHandle = CreateFile(dumpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    uint dumpType = MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithDataSegs;
    // Write the dump
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), fileHandle, (MINIDUMP_TYPE)dumpType, &ExceptionParam, NULL, &mci);

    // Close the file handle
    CloseHandle(fileHandle);

    params.AddParameter("Files", Narrow(dumpFileName).c_str());
  }
}

void WriteCallstack(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
  const size_t MAX_TEMP_PATH = MAX_PATH - 14;
  wchar_t stackFileName[MAX_PATH] = {0};

  EXCEPTION_POINTERS* pException = (EXCEPTION_POINTERS*)crashData;
  /******************** STACK TRACE ********************/
  
  // Create a stack walker that will print out our stack for us
  CustomStackWalker stackWalker;
  if(!CrashHandler::mExtraSymbolPath.Empty())
    stackWalker.SetExtraSymbolPath(CrashHandler::mExtraSymbolPath.c_str());
  
  if(pException && pException->ContextRecord)
    stackWalker.ShowCallstack(GetCurrentThread(), pException->ContextRecord);
  else
    stackWalker.ShowCallstack(GetCurrentThread());
  
  String stack = stackWalker.GetFinalOutput();
  String exceptionCode = "Fatal Engine Error";
  if(pException->ExceptionRecord != NULL)
    exceptionCode = GetExceptionCode(pException);
  stack = BuildString("Exception: ", exceptionCode, "\n", stack);
  
  // Create a temporary file name
  DWORD pathLength = GetTempPath(MAX_TEMP_PATH, stackFileName);
  ZeroStrCatW(stackFileName, MAX_TEMP_PATH, Widen(info.mStackName).c_str());
  WriteToFile(Narrow(stackFileName).c_str(), (byte*)stack.Data(), stack.SizeInBytes());
  
  // Add the stack file to the parameters
  params.AddParameter("Stack", Narrow(stackFileName).c_str());
}

void CrashHandler::InvokeWriteCallstack(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
  __try
  {
    if(WriteCallstack != NULL)
      WriteCallstack(params, crashData, info);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    params.AddParameter("ExtraData", "Writing callstack failed");
  }
}

void CrashHandler::InvokeLoggingCallback(CrashHandlerParameters& params, CrashInfo& info)
{
  __try
  {
    if(CrashHandler::mLoggingCallback != NULL)
      CrashHandler::mLoggingCallback(params, info, CrashHandler::mLoggingUserData);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    params.AddParameter("ExtraData", "Logging failed");
  }
}

void CrashHandler::InvokeRescueCallback()
{
  __try
  {
    if(CrashHandler::mRescueCallback != NULL)
      CrashHandler::mRescueCallback(CrashHandler::mRescueUserData);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {

  }
}

void CrashHandler::InvokeSendCrashReport(CrashHandlerParameters& params)
{
  __try
  {
    if(CrashHandler::mSendCrashReportCallback != NULL)
      CrashHandler::mSendCrashReportCallback(params, CrashHandler::mSendCrashReportUserData);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {

  }
}

void CrashHandler::FatalError(int errorCode)
{
  //capture the current context so the stack walker will show whatever called this at the root
  //(it seems like the stack walker skips knows to skip one level or 
  //something so the call stack doesn't show this function)
  EXCEPTION_POINTERS* exceptions = NULL;
  GetExceptionPointers(errorCode, &exceptions);

  CrashHandler::mRunCrashHandlerCallback(exceptions, false, CrashHandler::mRunCrashHandlerUserData);
  exit(errorCode);
}

void CrashHandler::DefaultRunCrashHandlerCallback(void* crashData, bool doRescueCall, void* userData)
{
  CrashInfo info;
  CrashHandler::InvokeCrashStartCallback(info);

  //Auto restart is used for programs out in the wild that want to immediately restart if something goes wrong.
  //For instance, this was used for the pacific science center projects.
  if(CrashHandler::mAutoRestart)
  {
    String appExe = GetApplication();
    Os::SystemOpenFile(appExe.c_str(), NULL, CrashHandler::mRestartCommandLine.c_str());

    return;
  }

  CrashHandlerParameters params;

  //memory dump
  CrashHandler::InvokePreMemoryDumpCallback();
  CrashHandler::WriteMiniDump(params, crashData, info);

  //logging
  CrashHandler::InvokeWriteCallstack(params, crashData, info);
  CrashHandler::InvokeLoggingCallback(params, info);

  //Send the crash data
  CrashHandler::InvokeSendCrashReport(params);

  // Finally, do the application defined rescue call (such as saving a level, etc)
  if(doRescueCall)
    CrashHandler::InvokeRescueCallback();
}

void CrashHandler::SetRestartCommandLine(StringRange commandLine)
{
  CrashHandler::mRestartCommandLine = commandLine;
}

void CrashHandler::RestartOnCrash(bool state)
{
  ZPrint("Set to auto restart on crash with '%s'\n", CrashHandler::mRestartCommandLine.c_str());
  CrashHandler::mAutoRestart = state;
}

String CrashHandler::mExtraSymbolPath = String();

CrashHandler::RunCrashHandlerCallback CrashHandler::mRunCrashHandlerCallback = CrashHandler::DefaultRunCrashHandlerCallback;
void* CrashHandler::mRunCrashHandlerUserData = NULL;

CrashHandler::CrashStartCallback CrashHandler::mCrashStartCallback = NULL;
void* CrashHandler::mCrashStartUserData = NULL;

CrashHandler::PreMemoryDumpCallback CrashHandler::mPreMemoryDumpCallback = NULL;
void* CrashHandler::mPreMemoryDumpUserData = NULL;

CrashHandler::CustomMemoryCallback CrashHandler::mCustomMemoryCallback = NULL;
void* CrashHandler::mCustomMemoryUserData = NULL;

CrashHandler::LoggingCallback CrashHandler::mLoggingCallback = NULL;
void* CrashHandler::mLoggingUserData = NULL;

CrashHandler::SendCrashReportCallback CrashHandler::mSendCrashReportCallback = NULL;
void* CrashHandler::mSendCrashReportUserData;

CrashHandler::FinalRescueCall CrashHandler::mRescueCallback = NULL;
void* CrashHandler::mRescueUserData = NULL;

bool CrashHandler::mAutoRestart = false;
String CrashHandler::mRestartCommandLine;

}//namespace Zero
