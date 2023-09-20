// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(Job, builder, type)
{
}

Job::Job() : mRunCount(0)
{
}

Job::~Job()
{
}

void Job::Execute()
{
}

void Job::ExecuteAsyncBegin()
{
  Execute();
  ExecuteAsyncEnd();
}

void Job::ExecuteAsyncEnd()
{
  Z::gJobs->JobComplete(this);
}

namespace Z
{
JobSystem* gJobs = nullptr;
}

JobSystem::JobSystem()
{
  if (ThreadingEnabled)
  {
    mWorkers.Resize(10);

    for (uint i = 0; i < mWorkers.Size(); ++i)
    {
      mWorkers[i] = new Thread();
      Thread& thread = *mWorkers[i];
      thread.Initialize(&Thread::ObjectEntryCreator<JobSystem, &JobSystem::WorkerThreadEntry>, this, "Background");
    }
  }
}

JobSystem::~JobSystem()
{
  // Cancel all active Jobs.
  mLock.Lock();
  // Active job range is safe because of the lock.
  forRange (Job& job, mActiveJobs.All())
    job.Cancel();

  // Release all active and pending job references that we own (this may delete
  // the jobs).
  mPendingJobs.Clear();
  mLock.Unlock();

  // Increment the counter but push no jobs
  // allowing each background thread to unblock.
  for (uint i = 0; i < mWorkers.Size(); ++i)
    mJobCounter.Increment();

  // Wait for each thread to shutdown.
  for (uint i = 0; i < mWorkers.Size(); ++i)
  {
    Thread& thread = *mWorkers[i];
    thread.WaitForCompletion();
  }

  // Clear all active jobs now that all threads have stopped (may release the
  // memory for jobs). There should be no more threads running, but we lock just
  // to be safe.
  mLock.Lock();
  mActiveJobs.Clear();
  mLock.Unlock();

  // Delete all threads.
  DeleteObjectsInContainer(mWorkers);
}

Job* JobSystem::GetNextJob()
{
  Job* job = nullptr;

  // Locked pop front
  mLock.Lock();
  if (!mPendingJobs.Empty())
  {
    HandleOf<Job> jobHandle = mPendingJobs.Back();
    job = jobHandle;
    mPendingJobs.PopBack();
    mActiveJobs.PushBack(jobHandle);
  }
  mLock.Unlock();

  // We don't need to return a handle because the job will
  // be kept alive by being inside of mActiveJobs.
  return job;
}

void JobSystem::RunJobsTimeSliced(double seconds)
{
  if (ThreadingEnabled)
    return;

  Timer timer;
  do
  {
    if (!RunOneJob())
      return;
  } while (timer.UpdateAndGetTime() < seconds);
}

bool JobSystem::AreAllJobsCompleted()
{
  mLock.Lock();
  bool completed = mPendingJobs.Empty() && mActiveJobs.Empty();
  mLock.Unlock();
  return completed;
}

OsInt JobSystem::WorkerThreadEntry()
{
  for (;;)
  {
    if (!RunOneJob())
      return 0;
  }
}

void JobSystem::AddJob(Job* job)
{
  if (!ThreadingEnabled && job->mRunImmediateWhenThreadingDisabled)
  {
    ++job->mRunCount;
    RunJob(job);
    return;
  }

  mLock.Lock();
  if (job->mRunCount == 0)
    mPendingJobs.PushBack(job);
  ++job->mRunCount;
  mLock.Unlock();

  // Signal that a job has been added, which will unblock the waiting workers.
  mJobCounter.Increment();
}

bool JobSystem::RunOneJob()
{
  mJobCounter.WaitAndDecrement();
  Job* job = GetNextJob();

  // No jobs and Semaphore release
  // that means we are shutting down.
  if (job == nullptr)
    return false;

  RunJob(job);
  return true;
}

void JobSystem::RunJob(Job* job)
{
  ProfileScopeFunctionArgs(RaverieVirtualTypeId(job)->Name);
  job->ExecuteAsyncBegin();
}

void JobSystem::JobComplete(Job* job)
{
  mLock.Lock();
  --job->mRunCount;
  bool completed = (job->mRunCount == 0);
  if (completed)
  {
    HandleOf<Job> jobHandle = job;
    mActiveJobs.EraseValue(jobHandle);
  }
  mLock.Unlock();

  if (!completed)
    RunJob(job);
}

} // namespace Raverie
