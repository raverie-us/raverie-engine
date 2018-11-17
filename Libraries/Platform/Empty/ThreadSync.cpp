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
}

ThreadLock::~ThreadLock()
{
}

void ThreadLock::Lock()
{
}

void ThreadLock::Unlock()
{
}


//----------------------------------------------------------- Os Event
OsEvent::OsEvent()
{
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

OsHandle OsEvent::GetHandle()
{
  return nullptr;
}

//----------------------------------------------------------- Semaphore
Semaphore::Semaphore()
{
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

InterprocessMutex::InterprocessMutex()
{
}

InterprocessMutex::~InterprocessMutex()
{
}

void InterprocessMutex::Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists)
{
}

CountdownEvent::CountdownEvent()
{
}

void CountdownEvent::IncrementCount()
{
}

void CountdownEvent::DecrementCount()
{
}

void CountdownEvent::Wait()
{
}

}//namespace Zero
