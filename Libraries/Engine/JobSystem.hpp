// MIT Licensed (see LICENSE.md).
#pragma once

// Because windows...
#undef AddJob

namespace Zero
{

class Job : public ReferenceCountedEventObject
{
public:
  friend class JobSystem;
  ZilchDeclareType(Job, TypeCopyMode::ReferenceType);
  Job();
  virtual ~Job();

  // Entrant function of the job. If it returns Complete, the job may be
  // destroyed by the job system. Returning InProgress means the job will not be
  // deleted (you can call AddJob again from any thread, even on the same job
  // class).
  virtual void Execute() = 0;

  // Called from a different thread, typically setting a bool to stop
  virtual int Cancel()
  {
    return 0;
  };

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
  typedef JobSystem ZilchSelf;
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

} // namespace Zero
