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
#include "SDL_thread.h"
#include "SDL_mutex.h"

namespace Zero
{

struct ThreadLockData
{
  ThreadLockData()
  {
    mMutex = nullptr;
  }

  SDL_mutex* mMutex;
};

ThreadLock::ThreadLock()
{
  ZeroConstructPrivateData(ThreadLockData);

  self->mMutex = SDL_CreateMutex();
  if (self->mMutex == nullptr)
  {
    String errorMessage = String::Format("Failed to create mutex: %s", SDL_GetError());
    Error(errorMessage.c_str());
  }
}

ThreadLock::~ThreadLock()
{
  ZeroGetPrivateData(ThreadLockData);
  SDL_DestroyMutex(self->mMutex);
  ZeroDestructPrivateData(ThreadLockData);
}

void ThreadLock::Lock()
{
  ZeroGetPrivateData(ThreadLockData);
  int ret = SDL_LockMutex(self->mMutex);
  if (ret != 0)
  {
    String errorMessage = String::Format("Failed to lock mutex: %s", SDL_GetError());
    Warn(errorMessage.c_str());
  }
}

void ThreadLock::Unlock()
{
  ZeroGetPrivateData(ThreadLockData);
  int ret = SDL_UnlockMutex(self->mMutex);
  if (ret != 0)
  {
    String errorMessage = String::Format("Failed to unlock mutex: %s", SDL_GetError());
    Warn(errorMessage.c_str());
  }
}

struct OsEventPrivateData
{
  OsEventPrivateData()
  {
    mConditional = nullptr;
  }

  SDL_cond* mConditional;
};

OsEvent::OsEvent()
{
  ZeroConstructPrivateData(OsEventPrivateData);
}

OsEvent::~OsEvent()
{
  Close();
}

void OsEvent::Initialize(bool manualReset, bool startSignaled)
{
  ZeroGetPrivateData(OsEventPrivateData);
  self->mConditional = SDL_CreateCond();
}

void OsEvent::Close()
{
  ZeroGetPrivateData(OsEventPrivateData);
  if (self->mConditional)
  {
    SDL_DestroyCond(self->mConditional);
    self->mConditional = nullptr;
  }
}

void OsEvent::Signal()
{
  ZeroGetPrivateData(OsEventPrivateData);
  int ret = SDL_CondSignal(self->mConditional);
  if (ret != 0)
  {
    String errorMessage = String::Format("Failed to signal OsEvent: %s", SDL_GetError());
    Warn(errorMessage.c_str());
  }
}

void OsEvent::Reset()
{
  Close();
  Initialize();
}

void OsEvent::Wait()
{
  ZeroGetPrivateData(OsEventPrivateData);
  SDL_mutex* mutex = SDL_CreateMutex();
  SDL_LockMutex(mutex);
  int result = SDL_CondWait(self->mConditional, mutex);
  if (result != 0)
  {
    String errorMessage = String::Format("Failed to signal OsEvent: %s", SDL_GetError());
    Warn(errorMessage.c_str());
  }
  SDL_UnlockMutex(mutex);
  SDL_DestroyMutex(mutex);
}

OsHandle OsEvent::GetHandle()
{
  ZeroGetPrivateData(OsEventPrivateData);
  return self->mConditional;
}

struct SemaphorePrivateData
{
  SemaphorePrivateData()
  {
    mSemaphore = nullptr;
  }

  SDL_sem* mSemaphore;
};

Semaphore::Semaphore()
{
  ZeroConstructPrivateData(SemaphorePrivateData);
  self->mSemaphore = SDL_CreateSemaphore(MaxSemaphoreCount);
  if (self->mSemaphore == nullptr)
  {
    String errorString = SDL_GetError();
    Warn("Failed to create semaphore");
  }
}

Semaphore::~Semaphore()
{
  ZeroGetPrivateData(SemaphorePrivateData)
  SDL_DestroySemaphore(self->mSemaphore);
  ZeroDestructPrivateData(SemaphorePrivateData);
}

void Semaphore::Increment()
{
  ZeroGetPrivateData(SemaphorePrivateData)
  int ret = SDL_SemPost(self->mSemaphore);
  if (ret != 0)
  {
    String errorString = SDL_GetError();
    Warn("Failed to increment semaphore: %s", errorString.c_str());
  }
}

void Semaphore::Decrement()
{
  ZeroGetPrivateData(SemaphorePrivateData);
  int ret = SDL_SemTryWait(self->mSemaphore);
  if (ret != 0)
  {
    String errorString = SDL_GetError();
    Warn("Failed to decrement on semaphore: %s", errorString.c_str());
  }
}

void Semaphore::Reset()
{
  ZeroGetPrivateData(SemaphorePrivateData)
  SDL_DestroySemaphore(self->mSemaphore);

  self->mSemaphore = SDL_CreateSemaphore(0);
  if (self->mSemaphore == nullptr)
  {
    String errorString = SDL_GetError();
    Warn("Failed to create semaphore");
  }
}

void Semaphore::WaitAndDecrement()
{
  ZeroGetPrivateData(SemaphorePrivateData);
  int ret = SDL_SemWait(self->mSemaphore);
  if (ret != 0)
  {
    String errorString = SDL_GetError();
    Warn("Failed to wait on semaphore: %s", errorString.c_str());
  }
}

InterprocessMutex::InterprocessMutex()
{
  Error("Not Implemented. Potentially use a file opened for writing as as global mutex lock");
}

InterprocessMutex::~InterprocessMutex()
{
  Error("Not Implemented. Potentially use a file opened for writing as as global mutex lock");
}

void InterprocessMutex::Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists)
{
  Error("Not Implemented. Potentially use a file opened for writing as as global mutex lock");
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
