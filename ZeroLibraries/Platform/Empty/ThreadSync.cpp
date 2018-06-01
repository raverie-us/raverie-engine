////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//----------------------------------------------------------- Thread Lock
struct ThreadLockPrivateData
{
};

ThreadLock::ThreadLock()
{
  Error("Not implemented");
}

ThreadLock::~ThreadLock()
{
  Error("Not implemented");
}

void ThreadLock::Lock()
{
  Error("Not implemented");
}

void ThreadLock::Unlock()
{
  Error("Not implemented");
}


//----------------------------------------------------------- Os Event
OsEvent::OsEvent()
{
  Error("Not implemented");
}

OsEvent::~OsEvent()
{
  Error("Not implemented");
}

void OsEvent::Initialize(bool manualReset, bool startSignaled)
{
  Error("Not implemented");
}

void OsEvent::Close()
{
  Error("Not implemented");
}

void OsEvent::Signal()
{
  Error("Not implemented");
}

void OsEvent::Reset()
{
  Error("Not implemented");
}

void OsEvent::Wait()
{
  Error("Not implemented");
}

//----------------------------------------------------------- Semaphore
Semaphore::Semaphore()
{
  Error("Not implemented");
}

Semaphore::~Semaphore()
{
  Error("Not implemented");
}

void Semaphore::Increment()
{
  Error("Not implemented");
}

void Semaphore::Decrement()
{
  Error("Not implemented");
}

void Semaphore::Reset()
{
  Error("Not implemented");
}

void Semaphore::WaitAndDecrement()
{
  Error("Not implemented");
}

InterprocessMutex::InterprocessMutex()
{
  Error("Not implemented");
}

InterprocessMutex::~InterprocessMutex()
{
  Error("Not implemented");
}

void InterprocessMutex::Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists)
{
  Error("Not implemented");
}

CountdownEvent::CountdownEvent()
{
  Error("Not implemented");
}

void CountdownEvent::IncrementCount()
{
  Error("Not implemented");
}

void CountdownEvent::DecrementCount()
{
  Error("Not implemented");
}

void CountdownEvent::Wait()
{
  Error("Not implemented");
}

}//namespace Zero
