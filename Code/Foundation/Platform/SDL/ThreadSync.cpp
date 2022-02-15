// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

struct ThreadLockData
{
  ThreadLockData()
  {
    mMutex = nullptr;
  }

  SDL_mutex* mMutex;
};

ThreadLock::ThreadLock()
{
  ZeroConstructPrivateData(ThreadLockData);

  self->mMutex = SDL_CreateMutex();
  if (self->mMutex == nullptr)
    Warn(SDL_GetError());
}

ThreadLock::~ThreadLock()
{
  ZeroGetPrivateData(ThreadLockData);
  SDL_DestroyMutex(self->mMutex);
  ZeroDestructPrivateData(ThreadLockData);
}

void ThreadLock::Lock()
{
  ZeroGetPrivateData(ThreadLockData);
  int result = SDL_LockMutex(self->mMutex);
  if (result != 0)
    Warn(SDL_GetError());
}

void ThreadLock::Unlock()
{
  ZeroGetPrivateData(ThreadLockData);
  int result = SDL_UnlockMutex(self->mMutex);
  if (result != 0)
    Warn(SDL_GetError());
}

struct OsEventPrivateData
{
  OsEventPrivateData()
  {
  }

  bool mManualReset;
  Atomic<bool> mIsGateOpen;
};

OsEvent::OsEvent()
{
  ZeroConstructPrivateData(OsEventPrivateData);
}

OsEvent::~OsEvent()
{
  Close();
}

void OsEvent::Initialize(bool manualReset, bool startSignaled)
{
  ZeroGetPrivateData(OsEventPrivateData);
  self->mManualReset = manualReset;
  self->mIsGateOpen = startSignaled;
}

void OsEvent::Close()
{
  ZeroGetPrivateData(OsEventPrivateData);
  self->mManualReset = true;
  self->mIsGateOpen = true;
}

void OsEvent::Signal()
{
  ZeroGetPrivateData(OsEventPrivateData);
  self->mIsGateOpen = true;
}

void OsEvent::Reset()
{
  ZeroGetPrivateData(OsEventPrivateData);
  self->mIsGateOpen = false;
}

void OsEvent::Wait()
{
  ZeroGetPrivateData(OsEventPrivateData);

  if (self->mManualReset)
  {
    while (!self->mIsGateOpen)
      ;
  }
  else
  {
    while (!self->mIsGateOpen.CompareExchange(false, true))
      ;
  }
}

OsHandle OsEvent::GetHandle()
{
  ZeroGetPrivateData(OsEventPrivateData);
  return this;
}

struct SemaphorePrivateData
{
  SemaphorePrivateData()
  {
    mSemaphore = nullptr;
  }

  SDL_sem* mSemaphore;
};

Semaphore::Semaphore()
{
  ZeroConstructPrivateData(SemaphorePrivateData);
  self->mSemaphore = SDL_CreateSemaphore(0);
  if (self->mSemaphore == nullptr)
    Warn(SDL_GetError());
}

Semaphore::~Semaphore()
{
  ZeroGetPrivateData(SemaphorePrivateData) SDL_DestroySemaphore(self->mSemaphore);
  ZeroDestructPrivateData(SemaphorePrivateData);
}

void Semaphore::Increment()
{
  ZeroGetPrivateData(SemaphorePrivateData) int result = SDL_SemPost(self->mSemaphore);
  if (result != 0)
    Warn(SDL_GetError());
}

void Semaphore::Decrement()
{
  ZeroGetPrivateData(SemaphorePrivateData);
  int result = SDL_SemTryWait(self->mSemaphore);
  if (result != 0)
    Warn(SDL_GetError());
}

void Semaphore::Reset()
{
  ZeroGetPrivateData(SemaphorePrivateData);

  while (SDL_SemValue(self->mSemaphore) != 0)
    Decrement();
}

void Semaphore::WaitAndDecrement()
{
  ZeroGetPrivateData(SemaphorePrivateData);
  int result = SDL_SemWait(self->mSemaphore);
  if (result != 0)
    Warn(SDL_GetError());
}

struct InterprocessMutexPrivateData
{
  InterprocessMutexPrivateData()
  {
    mFile = nullptr;
  }

  File* mFile;
};

InterprocessMutex::InterprocessMutex()
{
  ZeroConstructPrivateData(InterprocessMutexPrivateData);
  self->mFile = new File();
}

InterprocessMutex::~InterprocessMutex()
{
  ZeroGetPrivateData(InterprocessMutexPrivateData) delete self->mFile;
  ZeroDestructPrivateData(InterprocessMutexPrivateData);
}

void InterprocessMutex::Initialize(Status& status, const char* mutexName, bool failIfAlreadyExists)
{
  ZeroGetPrivateData(InterprocessMutexPrivateData)

      // This approach is a bit silly, but instead of using an actual inter
      // process mutex we open a common file for write access. We use the temp
      // directory because we know it will be shared between all running
      // instances and it is guaranteed writable. We also keep the file open for
      // write until the InterprocessMutex is destructed.
      if (!failIfAlreadyExists) return;

  // Sanitize the mutex name for files. We guarantee uniqueness because we don't
  // allow the '-' character even though it is legal in file names, and any
  // illegal character we find we replace with -XX where XX is the hex code.
  StringBuilder builder;
  while (*mutexName != '\0')
  {
    char c = *mutexName;

    if (isalnum(c))
    {
      builder.Append(c);
    }
    else
    {
      builder.Append('-');
      builder.Append(ToString((int)c));
    }

    ++mutexName;
  }

  String sharedMutexPathName = FilePath::Combine(GetTemporaryDirectory(), builder.ToString());

  self->mFile->Open(
      sharedMutexPathName, FileMode::Write, FileAccessPattern::Sequential, FileShare::Unspecified, &status);
}

CountdownEvent::CountdownEvent() : mCount(0)
{
  mWaitEvent.Initialize(true, true);
}

void CountdownEvent::IncrementCount()
{
  mThreadLock.Lock();
  // If count is initially zero, reset event
  if (mCount == 0)
    mWaitEvent.Reset();
  ++mCount;
  mThreadLock.Unlock();
}

void CountdownEvent::DecrementCount()
{
  mThreadLock.Lock();
  --mCount;
  // If count is now zero, signal event
  if (mCount == 0)
    mWaitEvent.Signal();
  mThreadLock.Unlock();
}

void CountdownEvent::Wait()
{
  mWaitEvent.Wait();
}

} // namespace Zero
