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
}

Thread::~Thread()
{
  ZeroGetPrivateData(ThreadPrivateData);
  // Destruction logic
  ZeroDestructPrivateData(ThreadPrivateData);
}

bool Thread::IsValid()
{
  Error("Not implemented");
  return false;
}

bool Thread::Initialize(EntryFunction entryFunction, void* instance, StringParam threadName, ThreadConfig* config )
{
  Error("Not implemented");
  return false;
}

void Thread::Resume()
{
  Error("Not implemented");
}

void Thread::Suspend()
{
  Error("Not implemented");
}

void Thread::Close()
{
  Error("Not implemented");
}

OsInt Thread::WaitForCompletion()
{
  Error("Not implemented");
  return 0;
}

OsInt Thread::WaitForCompletion(unsigned long milliseconds)
{
  Error("Not implemented");
  return 0;
}

bool Thread::IsCompleted()
{
  Error("Not implemented");
  return false;
}

OsHandle Thread::Detach()
{
  Error("Not implemented");
  return nullptr;
}

OsHandle Thread::GetThreadHandle()
{
  Error("Not implemented");
  return nullptr;
}

}//namespace Zero
