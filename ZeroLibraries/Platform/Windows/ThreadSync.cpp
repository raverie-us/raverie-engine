///////////////////////////////////////////////////////////////////////////////
///
/// \file ThreadSync.cpp
/// Implementation of Thread synchronization classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ThreadLock::ThreadLock()
{
  ZeroConstructPrivateData(CRITICAL_SECTION);
  ::InitializeCriticalSection(self);
}

ThreadLock::~ThreadLock()
{
  ZeroGetPrivateData(CRITICAL_SECTION);
  ::DeleteCriticalSection(self);
  ZeroDestructPrivateData(CRITICAL_SECTION);
}

void ThreadLock::Lock()
{
  ZeroGetPrivateData(CRITICAL_SECTION);
  ::EnterCriticalSection(self);
}

void ThreadLock::Unlock()
{
  ZeroGetPrivateData(CRITICAL_SECTION);
  ::LeaveCriticalSection(self);
}

OsEvent::OsEvent()
:mHandle(NULL)
{
}

OsEvent::~OsEvent()
{
  Close();
}

void OsEvent::Initialize(bool manualReset, bool startSignaled)
{
  mHandle = CreateEvent(NULL, manualReset, startSignaled, NULL);
  CheckWin(mHandle!=INVALID_HANDLE_VALUE, "Failed to create event.");
}

void OsEvent::Close()
{
  if(mHandle!=NULL)
  {
    VerifyWin(CloseHandle(mHandle), "Failed to close event.");
  }
}

void OsEvent::Signal()
{
  VerifyWin(SetEvent(mHandle), "Failed to Signal event.");
}

void OsEvent::Wait()
{
  DWORD result = WaitForSingleObject(mHandle, INFINITE);
  if(result == WAIT_FAILED)
  {
    VerifyWin(0, "Failed to Signal event.");
  }
}

void OsEvent::Reset()
{
  VerifyWin(ResetEvent(mHandle), "Failed to Reset event.");
}

Semaphore::Semaphore()
{
  mHandle = CreateSemaphore(NULL, 0, MaxSemaphoreCount, NULL);
}

Semaphore::~Semaphore()
{
  VerifyWin(CloseHandle(mHandle),"Failed to close Semaphore handle");
}

void Semaphore::Increment()
{
  VerifyWin(ReleaseSemaphore(mHandle, 1, NULL), "Failed to increment semaphore");
}

void Semaphore::Decrement()
{
  WaitForSingleObject(mHandle, 0);
}

void Semaphore::Reset()
{
  VerifyWin(CloseHandle(mHandle),"Failed to close Semaphore handle");
  mHandle = CreateSemaphore(NULL, 0, MaxSemaphoreCount, NULL);
}

void Semaphore::WaitAndDecrement()
{
  OsInt result = WaitForSingleObject(mHandle, INFINITE);
  if(result != WAIT_OBJECT_0)
  {
  }
}

Mutex::Mutex()
{
  ZeroConstructPrivateData(HANDLE);
}

Mutex::~Mutex()
{
  ZeroGetPrivateData(HANDLE);
  CloseHandle(*self);

  ZeroDestructPrivateData(HANDLE);
}

void Mutex::Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists)
{
  ZeroGetPrivateData(HANDLE);
  *self = CreateMutex(NULL, FALSE, Widen(mutexName).c_str());

  DWORD error = GetLastError();
  if(*self == nullptr)
    status.SetFailed("Mutex initialization error.", error);
  else if(failIfAlreadyExists && error == ERROR_ALREADY_EXISTS)
    status.SetFailed("The handle already existed", error);
  else
    status.Succeeded();
}

CountdownEvent::CountdownEvent()
  : mCount(0)
{
  mWaitEvent.Initialize(true, true);
}

void CountdownEvent::IncrementCount()
{
  mThreadLock.Lock();
  // If count is initially zero, reset event
  if (mCount == 0)
    mWaitEvent.Reset();
  ++mCount;
  mThreadLock.Unlock();
}

void CountdownEvent::DecrementCount()
{
  mThreadLock.Lock();
  --mCount;
  // If count is now zero, signal event
  if (mCount == 0)
    mWaitEvent.Signal();
  mThreadLock.Unlock();
}

void CountdownEvent::Wait()
{
  mWaitEvent.Wait();
}

}//namespace Zero
