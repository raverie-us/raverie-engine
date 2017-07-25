///////////////////////////////////////////////////////////////////////////////
///
/// \file BackgroundTask.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "BackgroundTask.hpp"
#include "Engine/ThreadDispatch.hpp"
#include "Engine/Tweakables.hpp"
#include "Engine/SystemObjectManager.hpp"

namespace Zero
{

namespace Events
{
  /// All events are sent on both the BackgroundTask object and gBackgroundTasks
  DefineEvent(BackgroundTaskStarted);
  DefineEvent(BackgroundTaskUpdated);
  DefineEvent(BackgroundTaskCompleted);
  DefineEvent(BackgroundTaskFailed);
  DefineEvent(BackgroundTaskCanceled);
}//namespace Events

namespace BackgroundTaskUi
{
const cstr cLocation = "EditorUi/BackgroundTasks/Default";
Tweakable(Vec4, IconColor,               Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ProgressBarPrimaryColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ProgressBackgroundColor, Vec4(1,1,1,1), cLocation);
}

namespace Z
{
  BackgroundTasks* gBackgroundTasks = nullptr;
}//namespace Z

ZilchDefineType(BackgroundTaskEvent, builder, type)
{
}


//---------------------------------------------------------- Background Task Job
//******************************************************************************
BackgroundTaskJob::BackgroundTaskJob()
{
  mState = BackgroundTaskState::Running;
  mTask = nullptr;
}

//******************************************************************************
void BackgroundTaskJob::UpdateProgress(StringParam taskName, float percentComplete,
                                       StringParam progressText)
{
  BackgroundTaskEvent* eventToSend = new BackgroundTaskEvent(mTask);
  eventToSend->Name = taskName;
  eventToSend->ProgressText = progressText;
  eventToSend->PercentComplete = percentComplete;
  eventToSend->State = mState;

  SendMainThreadEvent(Events::BackgroundTaskUpdated, eventToSend);
}

//******************************************************************************
String BackgroundTaskJob::GetName() const
{
  if(mTask != nullptr)
    return mTask->mName;
  return String();
}

//******************************************************************************
void BackgroundTaskJob::Failed()
{
  mState = BackgroundTaskState::Failed;
}

//******************************************************************************
int BackgroundTaskJob::Cancel()
{
  mState = BackgroundTaskState::Canceled;
  return 0;
}

//******************************************************************************
void BackgroundTaskJob::SendMainThreadEvent(StringParam eventId, Event* e)
{
  Z::gDispatch->Dispatch(ThreadContext::Main, Z::gBackgroundTasks, eventId, e);
}

//-------------------------------------------------------------- Background Task
//******************************************************************************
BackgroundTask::BackgroundTask(BackgroundTaskJob* job)
  : mJob(job)
{
  mHidden = false;
  mActivateOnCompleted = true;
  mCallback = nullptr;

  // We want the tasks to stay around after their completion
  job->mDeletedOnCompletion = false;
  job->mTask = this;

  mState = BackgroundTaskState::NotStarted;
  mPercentComplete = 0.0f;
  mEstimatedTotalDuration = 10.0f;
  mIndeterminate = false;
  mUseForAverage = true;

  const String cDefaultIcon = "TaskGear";
  mIconName = cDefaultIcon;
  mIconColor = BackgroundTaskUi::IconColor;
  mProgressPrimaryColor = BackgroundTaskUi::ProgressBarPrimaryColor;
  mProgressBackgroundColor = BackgroundTaskUi::ProgressBackgroundColor;
}

//******************************************************************************
BackgroundTask::~BackgroundTask()
{
  // We should always have ownership of this job as we set mDeletedOnCompletion
  // to true so it "should" always be safe to delete
  SafeDelete(mJob);
}

//******************************************************************************
void BackgroundTask::Execute()
{
  // Pass the task off to the job system
  Z::gJobs->AddJob(mJob);

  mStartTime = clock();

  // Signal that the task has started
  BackgroundTaskEvent e(this);
  e.Name = mName;
  e.ProgressText = mProgressText;
  e.PercentComplete = mPercentComplete;
  Z::gBackgroundTasks->GetDispatcher()->Dispatch(Events::BackgroundTaskStarted, &e);
}

//******************************************************************************
void BackgroundTask::Cancel()
{
  mJob->Cancel();
}

//******************************************************************************
bool BackgroundTask::IsCompleted()
{
  return mState == BackgroundTaskState::Completed;
}

//******************************************************************************
Job* BackgroundTask::GetFinishedJob()
{
  if(mState == BackgroundTaskState::Completed || mState == BackgroundTaskState::Failed)
    return mJob;
  return nullptr;
}

//******************************************************************************
float BackgroundTask::GetTimeAlive()
{
  clock_t currTime = clock();
  clock_t difference = (currTime - mStartTime);
  return (float)(difference / (double)Time::ClocksPerSecond());
}

//******************************************************************************
float BackgroundTask::GetEstimatedPercentComplete()
{
  if(IsCompleted())
    return 1.0f;

  if(mIndeterminate)
  {
    float percent = (GetTimeAlive() / mEstimatedTotalDuration);
    return Math::Min(percent, 0.95f);
  }

  return mPercentComplete;
}

//******************************************************************************
void BackgroundTask::Completed()
{
  if(mActivateOnCompleted && mCallback)
    mCallback(this, mJob);
}

//------------------------------------------------------------- Background Tasks
//******************************************************************************
ZilchDefineType(BackgroundTasks, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

//******************************************************************************
BackgroundTasks::BackgroundTasks()
{
  Z::gBackgroundTasks = this;
  ConnectThisTo(this, Events::BackgroundTaskUpdated, OnTaskUpdated);
}

//******************************************************************************
BackgroundTasks::~BackgroundTasks()
{
  DeleteObjectsInContainer(mActiveTasks);
}

//******************************************************************************
BackgroundTask* BackgroundTasks::Execute(BackgroundTaskJob* job,
                                         StringParam taskName)
{
  // Create and execute the task
  BackgroundTask* task = CreateTask(job);
  task->mName = taskName;
  task->Execute();
  return task;
}

//******************************************************************************
BackgroundTask* BackgroundTasks::CreateTask(BackgroundTaskJob* job)
{
  BackgroundTask* task = new BackgroundTask(job);
  mActiveTasks.PushBack(task);
  return task;
}

//******************************************************************************
void BackgroundTasks::OnTaskUpdated(BackgroundTaskEvent* e)
{
  BackgroundTask* task = e->mTask;
  //this is most likely for a job being called on a main thread so a task was never created
  if(task == nullptr)
    return;

  // Patch the data on the task
  task->mName = e->Name;
  task->mProgressText = e->ProgressText;
  task->mPercentComplete = e->PercentComplete;
  task->mState = e->State;

  Time::GetTime();

  switch(e->State)
  {
  case BackgroundTaskState::Failed:
    task->GetDispatcher()->Dispatch(Events::BackgroundTaskFailed, e);
    GetDispatcher()->Dispatch(Events::BackgroundTaskFailed, e);
    break;
  case BackgroundTaskState::Canceled:
    task->GetDispatcher()->Dispatch(Events::BackgroundTaskCanceled, e);
    GetDispatcher()->Dispatch(Events::BackgroundTaskCanceled, e);
    break;
  case BackgroundTaskState::Completed:
    task->Completed();
    task->GetDispatcher()->Dispatch(Events::BackgroundTaskCompleted, e);
    GetDispatcher()->Dispatch(Events::BackgroundTaskCompleted, e);
    break;
  case BackgroundTaskState::Running:
    task->GetDispatcher()->Dispatch(Events::BackgroundTaskUpdated, e);
    break;
  }
}

}//namespace Zero
