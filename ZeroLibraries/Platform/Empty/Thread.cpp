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
};

Thread::Thread()
{
  ZeroConstructPrivateData(ThreadPrivateData);
}

Thread::~Thread()
{
  ZeroDestructPrivateData(ThreadPrivateData);
}

bool Thread::IsValid()
{
  return false;
}

bool Thread::Initialize(EntryFunction entryFunction, void* instance, StringParam threadName)
{
  return false;
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
  return 0;
}

OsInt Thread::WaitForCompletion(unsigned long milliseconds)
{
  return 0;
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
