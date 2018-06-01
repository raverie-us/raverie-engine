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
#include "SDL_thread.h"
#include "SDL_mutex.h"

namespace Zero
{
const bool ThreadingEnabled = true;

struct ThreadPrivateData
{
  SDL_Thread* mHandle;
};

Thread::Thread()
{
  ZeroConstructPrivateData(ThreadPrivateData);

  self->mHandle = nullptr;
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

bool Thread::Initialize(EntryFunction entry, void* instance, StringParam threadName)
{
  ZeroGetPrivateData(ThreadPrivateData);

  mThreadName = threadName;

  const int cStackSize = 65536;
  self->mHandle = SDL_CreateThread((SDL_ThreadFunction)entry, threadName.c_str(), instance);

  if (self->mHandle == nullptr)
  {
    String errorString = SDL_GetError();
    String message = String::Format("Failed to create thread: %s", errorString.c_str());
    Error(message.c_str());
    return false;
  }

  return true;
}

bool Thread::IsValid()
{
  ZeroGetPrivateData(ThreadPrivateData);
  return self->mHandle != nullptr;
}

// Detaches the thread and it runs until it is finished. The thread cleans itself up.
// Do not call close on a thread that has called WaitForCompletion()
void Thread::Close()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(IsValid())
    SDL_DetachThread(self->mHandle);

  self->mHandle = nullptr;
}

OsHandle Thread::Detach()
{
  ZeroGetPrivateData(ThreadPrivateData);
  OsHandle handle = self->mHandle;
  self->mHandle = nullptr;
  return handle;
}

OsInt Thread::WaitForCompletion()
{
  return WaitForCompletion(0xFFFFFFFF);
}

OsInt Thread::WaitForCompletion(unsigned long milliseconds)
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(!IsValid())
    return (OsInt)-1;

  int result;
  SDL_WaitThread(self->mHandle, &result);
  return result;
}

bool Thread::IsCompleted()
{
  ZeroGetPrivateData(ThreadPrivateData);
  if(!IsValid())
    return true;

  return false;
}

}//namespace Zero
