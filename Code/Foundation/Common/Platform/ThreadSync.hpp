// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
/// Thread Lock
/// Safe to lock multiple times from the same thread
class ThreadLock
{
public:
  ThreadLock();
  ~ThreadLock();
  void Lock();
  void Unlock();

private:
  // TODO(trevor): To support threads, must add a handle/pointer to thread primitive here
};

// Wrapper around an unnamed event.
class OsEvent
{
public:
  OsEvent();
  ~OsEvent();
  void Initialize(bool manualReset = false, bool startSignaled = false);
  void Close();
  void Signal();
  void Reset();
  void Wait();

private:
  // TODO(trevor): To support threads, must add a handle/pointer to thread primitive here
};

const int MaxSemaphoreCount = 0x0FFFFFFF;

// Semaphore class. Multithreaded counter / gatekeeper.
class Semaphore
{
public:
  Semaphore();
  ~Semaphore();
  void Increment();
  void Decrement();
  void Reset();
  void WaitAndDecrement();

private:
  // TODO(trevor): To support threads, must add a handle/pointer to thread primitive here
};

/// Not fully implemented as it's currently only needed for interprocess
/// communication
class InterprocessMutex
{
public:
  InterprocessMutex();
  ~InterprocessMutex();

  void Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists = false);

private:
  // TODO(trevor): To support threads, must add a handle/pointer to thread primitive here
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

} // namespace Raverie
