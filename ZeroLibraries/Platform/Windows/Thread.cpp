///////////////////////////////////////////////////////////////////////////////
///
/// \file Thread.cpp
/// Implementation of the Thread class.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#ifdef _MSC_VER

///Used to set the thread name in Visual Studio. This raises an exception that 
//Visual Studio catches and then sets the thread name.
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType;     // must be 0x1000
  LPCSTR szName;    // pointer to name (in user addr space)
  DWORD dwThreadID; // thread ID (-1=caller thread)
  DWORD dwFlags;    // reserved for future use, must be zero
} THREADNAME_INFO;

inline void SetThreadDebugName(DWORD dwThreadID, LPCSTR szThreadName)
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = szThreadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  // Windows API never exposed a way to set the name of a thread, so instead they posted
  // this bit of specialized code that exposes internal details about how they set thread names
  // This trick uses the above defined structure, as well as raising an exception to set the name
  // The exception is caught by Windows/Visual Studio which then sets the thread name
  __try
  {
    // If you hit a breakpoint/exception here, CONTINUE past it
    RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (CONST ULONG_PTR*)&info);
  }
  __except(EXCEPTION_CONTINUE_EXECUTION)
  {
  }
}

#else

inline void SetThreadDebugName(DWORD dwThreadID, LPCSTR szThreadName)
{
}

#endif

namespace Zero
{
const bool ThreadingEnabled = true;

struct ThreadPrivateData
{
  OsInt mThreadId;
  OsHandle mHandle;
};

Thread::Thread()
{
  ZeroConstructPrivateData(ThreadPrivateData);

  self->mHandle = NULL;
  self->mThreadId = 0;
}

Thread::~Thread()
{
  Close();
  ZeroDestructPrivateData(ThreadPrivateData);
}

OsHandle Thread::GetThreadHandle()
{
  ZeroGetPrivateData(ThreadPrivateData);
  return self->mHandle;
}

bool Thread::Initialize(EntryFunction entry, void* instance, StringParam threadName, ThreadConfig* config)
{
  ZeroGetPrivateData(ThreadPrivateData);

  mThreadName = threadName;

  const int cStackSize = 65536;
  self->mHandle = ::CreateThread( NULL, //No Security
                           cStackSize,
                           (LPTHREAD_START_ROUTINE)entry,
                           (LPVOID)instance, 
                           CREATE_SUSPENDED,
                           &self->mThreadId);

  CheckWin(self->mHandle != INVALID_HANDLE_VALUE, 
          "Failed to create thread named %s", threadName.c_str());

  if(self->mHandle != INVALID_HANDLE_VALUE)
  {
    SetThreadDebugName(self->mThreadId, threadName.c_str());
    return true;
  }
  else
  {
    self->mHandle = NULL;
    return false;
  }
}

bool Thread::IsValid()
{
  ZeroGetPrivateData(ThreadPrivateData);
  return self->mHandle != NULL;
}

void Thread::Resume()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(IsValid())
    VerifyWin(ResumeThread(self->mHandle), "Failed to resume thread. Thread name: %", 
              mThreadName.c_str());
}

void Thread::Suspend()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(IsValid())
    VerifyWin(SuspendThread(self->mHandle),
              "Failed to suspend thread. Thread name: %", mThreadName.c_str());
}

//Close the thread handle. 
void Thread::Close()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(IsValid())
    VerifyWin(CloseHandle(self->mHandle),
              "Failed to close thread handle. Thread name: %", mThreadName.c_str());
  self->mHandle = NULL;
}

OsHandle Thread::Detach()
{
  ZeroGetPrivateData(ThreadPrivateData);
  OsHandle handle = self->mHandle;
  self->mHandle = NULL;
  return handle;
}

OsInt Thread::WaitForCompletion()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(!IsValid())
    return (OsInt)-1;

  DWORD result = WaitForSingleObject(self->mHandle, INFINITE);
  if(result != WAIT_OBJECT_0)
  {
    DebugPrint("Failed to wait on thread. Thread name: %s", mThreadName.c_str());
    return (OsInt)-1;
  }
  else
  {
    OsInt returnCode;
    GetExitCodeThread(self->mHandle, &returnCode);
    return returnCode;
  }
}

bool Thread::IsCompleted()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(!IsValid())
    return true;

  OsInt returnCode = 0;
  GetExitCodeThread(self->mHandle, &returnCode);
  bool threadActive = (returnCode == STILL_ACTIVE);
  return !threadActive;
}

ThreadConfig::ThreadConfig()
{

}

ThreadConfig::~ThreadConfig()
{

}

void ThreadConfig::SetParameter(StringParam name, void* value)
{
  mConfigValues.Insert(name, value);
}

void* ThreadConfig::GetParameter(StringParam name)
{
  return mConfigValues.FindValue(name, nullptr);
}

}//namespace Zero
