///////////////////////////////////////////////////////////////////////////////
///
/// \file Thread.cpp
/// Declaration of the Thread class.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/Thread.hpp"

namespace Zero
{
const bool ThreadingEnabled = false;

struct ThreadPrivateData
{
  OsInt mResult;
};

Thread::Thread()
{
  ZeroConstructPrivateData(ThreadPrivateData);
  self->mResult = 0;
}

Thread::~Thread()
{
  ZeroDestructPrivateData(ThreadPrivateData);
}

bool Thread::IsValid()
{
  return true;
}

bool Thread::Initialize(EntryFunction entryFunction, void* instance, StringParam threadName)
{
  ZeroGetPrivateData(ThreadPrivateData);
  ZPrint("Starting thread '%s' (single threaded)", threadName.c_str());
  self->mResult = entryFunction(instance);
  ZPrint("Finished thread '%s'", threadName.c_str());
  return true;
}

void Thread::Resume()
{
}

void Thread::Suspend()
{
}

void Thread::Close()
{
}

OsInt Thread::WaitForCompletion()
{
  return self->mResult;
}

OsInt Thread::WaitForCompletion(unsigned long milliseconds)
{
  return self->mResult;
}

bool Thread::IsCompleted()
{
  return true;
}

OsHandle Thread::Detach()
{
  return nullptr;
}

OsHandle Thread::GetThreadHandle()
{
  return nullptr;
}

size_t Thread::GetThreadId()
{
  return 0;
}

size_t Thread::GetCurrentThreadId()
{
  return 0;
}

}//namespace Zero
