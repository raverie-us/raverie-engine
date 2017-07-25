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

}//namespace Zero
