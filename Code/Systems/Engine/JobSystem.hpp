// MIT Licensed (see LICENSE.md).
#pragma once

// Because windows...
#undef AddJob

namespace Raverie
{

class Job : public ReferenceCountedEventObject
{
public:
  friend class JobSystem;
  RaverieDeclareType(Job, TypeCopyMode::ReferenceType);
  Job();
  virtual ~Job();

  // Warning: Only overwrite this if your job has its own threads and runs,
  // asynchronously otherwise implement Execute.
  // Anyone who implements must call ExecuteAsyncEnd() at the end.
  virtual void ExecuteAsyncBegin();

  // Called from a different thread, typically setting a bool to stop
  virtual int Cancel()
  {
    return 0;
  };

protected:
  // The default behavior is that a job completes everything synchronously on the
  // worker thread, however some jobs may have their own threads and run asynchronously.
  // Called by the default implementation of ExecuteAsyncBegin.
  virtual void Execute();

  // Only should be called by ExecuteAsyncBegin when the job is complete.
  void ExecuteAsyncEnd();

public:
  // When threading is disabled, should we run this task immediately when AddJob is called?
  bool mRunImmediateWhenThreadingDisabled = false;

private:
  // This value is incremented by the job system every time we add the job.
  // If the value is greater than 1, the thread will run it multiple times.
  // Must be locked by the JobSystem.
  size_t mRunCount;
};

class JobSystem : public EventObject
{
public:
  friend class Job;
  typedef JobSystem RaverieSelf;
  JobSystem();
  ~JobSystem();

  // Add's a job to be worked on (can be called from any thread).
  // Note that a job can be queued up again after it completes.
  void AddJob(Job* job);
  OsInt WorkerThreadEntry();

  // Runs until a slice of time is taken (only when ThreadingEnabled is false).
  // Returns false if there is no work to be done.
  // Executing jobs may run over the given slice of time, but will never undershoot it.
  void RunJobsTimeSliced(double seconds = 1.0 / 60.0);

  bool AreAllJobsCompleted();

private:
  // Takes a job from the job queue and runs it.
  // If no jobs are available, this will return false.
  bool RunOneJob();

  void RunJob(Job* job);

  void JobComplete(Job* job);

  ThreadLock mLock;
  Array<HandleOf<Job>> mPendingJobs;
  Array<HandleOf<Job>> mActiveJobs;
  Array<Thread*> mWorkers;
  Semaphore mJobCounter;
  Job* GetNextJob();
  friend class Job;
};

namespace Z
{
extern JobSystem* gJobs;
}

} // namespace Raverie
