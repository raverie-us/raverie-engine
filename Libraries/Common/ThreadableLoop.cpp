// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ThreadableLoop::ThreadableLoop(StringParam threadName) : mResult(0), mCompleted(false)
{
  if (ThreadingEnabled)
    mThread.Initialize(&ThreadableLoop::ThreadFunction, this, threadName);
}

void ThreadableLoop::RunIterations(size_t iterations)
{
  // If we have threads, then the loop is already running on another thread...
  if (ThreadingEnabled)
    return;

  // Run the specified number of iterations.
  while (iterations != 0)
  {
    Update();
    --iterations;
  }
}

OsInt ThreadableLoop::Wait()
{
  if (ThreadingEnabled)
  {
    mThread.WaitForCompletion();
  }
  else
  {
    while (!mCompleted)
      Update();
  }

  return mResult;
}

void ThreadableLoop::Complete(OsInt result)
{
  mResult = result;
  mCompleted = true;
}

OsInt ThreadableLoop::ThreadFunction(void* userData)
{
  ThreadableLoop* self = (ThreadableLoop*)userData;

  while (!self->mCompleted)
    self->Update();

  return self->mResult;
}

} // namespace Zero
