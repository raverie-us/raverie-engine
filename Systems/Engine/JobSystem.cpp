///////////////////////////////////////////////////////////////////////////////
///
/// \file JobSystem.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(Job, builder, type)
{
}

Job::Job()
{
  mDeletedOnCompletion = true;
  mOsEvent = nullptr;
}

Job::~Job()
{
  SafeDelete(mOsEvent);
}

OsEvent* Job::InitializeOsEvent()
{
  mOsEvent = new OsEvent();
  mOsEvent->Initialize();
  mDeletedOnCompletion = false;
  return mOsEvent;
}

//-----------------------------------------------------------------------------
namespace Z
{
  JobSystem* gJobs = nullptr;
}

JobSystem::JobSystem()
{
  mWorkerThreadsActive = true;
  Workers.Resize(10);

  for(uint i=0;i<Workers.Size();++i)
  {
    Workers[i] = new Thread();
    Thread& thread = *Workers[i];
    thread.Initialize(&Thread::ObjectEntryCreator<JobSystem,&JobSystem::WorkerThreadEntry>, this, "Background");
    thread.Resume();
  }
}

JobSystem::~JobSystem()
{
  mWorkerThreadsActive = false;

  mLock.Lock();

  //Cancel all active Jobs

  //Active job range is safe because of the lock.
  forRange(Job& job, ActiveJobs.All())
    job.Cancel();

  //Delete all pending jobs
  DeleteObjectsIn(PendingJobs);

  mLock.Unlock();

  //increment the counter but push no jobs
  //allowing each background thread to unblock
  for(uint i=0;i<Workers.Size();++i)
  {
    mJobCounter.Increment();
  }

  //Wait for each thread to shutdown
  for(uint i=0;i<Workers.Size();++i)
  {
    Thread& thread = *Workers[i];
    thread.WaitForCompletion();
  }

  //delete all threads
  DeleteObjectsInContainer(Workers);
}

Job* JobSystem::GetNextJob()
{
  Job* job = nullptr;

  //Locked pop front
  mLock.Lock();
  if(!PendingJobs.Empty())
  {
    job = &PendingJobs.Front();
    PendingJobs.Erase(PendingJobs.Begin());
    ActiveJobs.PushBack(job);
  }
  mLock.Unlock();

  return job;
}

void JobSystem::JobFinished(Job* job)
{
  mLock.Lock();
  ActiveJobs.Erase(job);
  mLock.Unlock();

  if(job->mOsEvent)
    job->mOsEvent->Signal();

  // Only delete the job if specified
  if(job->mDeletedOnCompletion)
    delete job;
}

OsInt JobSystem::WorkerThreadEntry()
{
  for(;;)
  {
    mJobCounter.WaitAndDecrement();
    Job* job = GetNextJob();

    //No jobs and Semaphore release
    //that means we are shutting down.
    if(job == nullptr)
      return 0;

    //Run the job
    job->Execute();

    //Finish the job
    JobFinished(job);
  }
}

void JobSystem::AddJob(Job* job)
{
  if(!ThreadingEnabled)
  {
    job->Execute();
    return;
  }
  
  //lock push a job
  mLock.Lock();
  PendingJobs.PushBack(job);
  mLock.Unlock();

  //Signal that a job has been added 
  //unblocking workers
  mJobCounter.Increment();
}

}//zero
