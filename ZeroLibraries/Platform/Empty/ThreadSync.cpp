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
#include "Platform/ThreadSync.hpp"

namespace Zero
{
//----------------------------------------------------------- Thread Lock
struct ThreadLockPrivateData
{
};

ThreadLock::ThreadLock()
{
  Error("Not implemented");
  ZeroConstructPrivateData(ThreadLockPrivateData);
}

ThreadLock::~ThreadLock()
{
  ZeroGetPrivateData(ThreadLockPrivateData);
  // Destruction logic
  ZeroDestructPrivateData(ThreadLockPrivateData);
}

void ThreadLock::Lock()
{
  ZeroGetPrivateData(ThreadLockPrivateData);
}

void ThreadLock::Unlock()
{
  ZeroGetPrivateData(ThreadLockPrivateData);
}


//----------------------------------------------------------- Os Event
OsEvent::OsEvent()
{
  Error("Not implemented");
  mHandle = NULL;
}

OsEvent::~OsEvent()
{
}

void OsEvent::Initialize(bool manualReset, bool startSignaled)
{
}

void OsEvent::Close()
{
}

void OsEvent::Signal()
{
}

void OsEvent::Reset()
{
}

void OsEvent::Wait()
{
}

//----------------------------------------------------------- Semaphore
Semaphore::Semaphore()
{
  Error("Not implemented");
  mHandle = NULL;
}

Semaphore::~Semaphore()
{
}

void Semaphore::Increment()
{
}

void Semaphore::Decrement()
{
}

void Semaphore::Reset()
{
}

void Semaphore::WaitAndDecrement()
{
}

}//namespace Zero
