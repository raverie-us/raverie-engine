///////////////////////////////////////////////////////////////////////////////
///
/// \file ThreadSync.hpp
/// Declaration of Thread synchronization classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Thread.hpp"
#include "PrivateImplementation.hpp"

namespace Zero
{
/// Thread Lock
/// Safe to lock multiple times from the same thread
class ZeroShared ThreadLock
{
public:
  ThreadLock();
  ~ThreadLock();
  void Lock();
  void Unlock();

private:
  ZeroDeclarePrivateData(ThreadLock, 48);
};

//Wrapper around an unnamed event.
class ZeroShared OsEvent
{
public:
  OsEvent();
  ~OsEvent();
  void Initialize(bool manualReset = false, bool startSignaled = false);
  void Close();
  void Signal();
  void Reset();
  void Wait();
  OsHandle GetHandle() { return mHandle; }
private:
  OsHandle mHandle;
};

const int MaxSemaphoreCount = 0x0FFFFFFF;

//Semaphore class. Multithreaded counter / gatekeeper.
class ZeroShared Semaphore
{
public:
  Semaphore();
  ~Semaphore();
  void Increment();
  void Decrement();
  void Reset();
  void WaitAndDecrement();
private:
  OsHandle mHandle;

  ZeroDeclarePrivateData(Semaphore, 8);
};

/// Not fully implemented as it's currently only needed for interprocess communication
class ZeroShared Mutex
{
public:
  Mutex();
  ~Mutex();

  void Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists = false);

private:
  ZeroDeclarePrivateData(Mutex, 8);
};

class CountdownEvent
{
public:
  CountdownEvent();

  void IncrementCount();
  void DecrementCount();
  void Wait();

private:
  OsEvent mWaitEvent;
  ThreadLock mThreadLock;
  int mCount;
};

}//namespace Zero
