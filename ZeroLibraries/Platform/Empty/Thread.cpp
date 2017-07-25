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
  Error("Not implemented");
  ZeroConstructPrivateData(ThreadPrivateData);
}

Thread::~Thread()
{
  ZeroGetPrivateData(ThreadPrivateData);
  // Destruction logic
  ZeroDestructPrivateData(ThreadPrivateData);
}

bool Thread::IsValid()
{
  ZeroGetPrivateData(Thread);
  return false;
}

bool Thread::Initialize(EntryFunction entry, void* instance, cstr threadName)
{
  ZeroGetPrivateData(Thread);
  return false;
}

void Thread::Resume()
{
  ZeroGetPrivateData(Thread);
}

void Thread::Suspend()
{
  ZeroGetPrivateData(Thread);
}

void Thread::Close()
{
  ZeroGetPrivateData(Thread);
}

OsInt Thread::WaitForCompletion()
{
  ZeroGetPrivateData(Thread);
  return 0;
}

bool Thread::IsCompleted()
{
  ZeroGetPrivateData(Thread);
  return true;
}

OsHandle Thread::Detach()
{
  ZeroGetPrivateData(Thread);
  return NULL;
}

OsHandle Thread::GetThreadHandle()
{
  ZeroGetPrivateData(Thread);
  return NULL;
}

}//namespace Zero
