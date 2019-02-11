// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// If the platform supports threading, the threadable loop will run until it
/// returns
class ThreadableLoop
{
public:
  ThreadableLoop(StringParam threadName = "ThreadableLoop");

  /// If single threaded, this runs a given number of iterations of the loop.
  /// Otherwise if the platform supports threads, this function does nothing
  /// (but provides compatability).
  void RunIterations(size_t iterations = 1);

  /// Runs the loop until it completes.
  OsInt Wait();

protected:
  /// Call this when you finish all the iterations of your loop.
  /// This should only ever be called from inside Update(), on the update thread
  /// if one exists.
  void Complete(OsInt result = 0);

  /// This will be called for each iteration of the loop. It will be called on a
  /// thread if available.
  virtual void Update() = 0;

private:
  static OsInt ThreadFunction(void* userData);

  volatile bool mCompleted;
  volatile OsInt mResult;
  Thread mThread;
};

} // namespace Zero
