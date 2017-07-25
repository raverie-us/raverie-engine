///////////////////////////////////////////////////////////////////////////////
///
/// \file JobSystem.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Because windows...
#undef AddJob

namespace Zero
{

//-------------------------------------------------------------------------- Job
class Job : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  Job();
  virtual ~Job();

  // Entrant function of the job.
  virtual int Execute()=0;

  // Called from a different thread, typically setting a bool to stop
  virtual int Cancel(){return 0;};
  
  /// We don't want to create an OsEvent for every job, so only call this if you
  /// need it. Creating an OsEvent on this job will also set 
  /// mDeletedOnCompletion to false, as the job must be alive to wait on the OsEvent.
  OsEvent* InitializeOsEvent();

  bool mDeletedOnCompletion;
  OsEvent* mOsEvent;
  Link<Job> link;
};

//------------------------------------------------------------------- Job System
class JobSystem
{
public:
  JobSystem();
  ~JobSystem();
  
  void AddJob(Job* job);
  OsInt WorkerThreadEntry();

private:
  ThreadLock mLock;
  InList<Job> PendingJobs;
  InList<Job> ActiveJobs;
  Array<Thread*> Workers;
  Semaphore mJobCounter;
  Job* GetNextJob();
  void JobFinished(Job* job);
  bool mWorkerThreadsActive;
  friend class Job;
};

namespace Z
{
  extern JobSystem* gJobs;
}

} // namespace Zero
