///////////////////////////////////////////////////////////////////////////////
///
/// \file BackgroundTask.hpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  /// All events are sent on both the BackgroundTask object and gBackgroundTasks
  DeclareEvent(BackgroundTaskStarted);
  DeclareEvent(BackgroundTaskUpdated);
  DeclareEvent(BackgroundTaskCompleted);
  DeclareEvent(BackgroundTaskFailed);
  DeclareEvent(BackgroundTaskCanceled);
}//namespace Events

/// Forward declarations.
class BackgroundTask;

DeclareEnum5(BackgroundTaskState, NotStarted, Running, Completed,
                                  Failed, Canceled);

//-------------------------------------------------------- Background Task Event
class BackgroundTaskEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  BackgroundTaskEvent(BackgroundTask* task) : mTask(task){}
  BackgroundTask* mTask;

  /// The reason these are here even though the job stores a pointer to the
  /// task, the task is a main thread only object, therefor the job
  /// cannot update these values on the task.
  String Name;
  String ProgressText;
  float PercentComplete;
  BackgroundTaskState::Type State;
};

//---------------------------------------------------------- Background Task Job
/// When implementing a background task, you should derive from this class.
/// When passing off to the gBackgroundTasks object, a main thread
/// BackgroundTask will be created and returned to connect to events.
class BackgroundTaskJob : public Job
{
public:
  BackgroundTaskJob();

  /// Called from the job thread to update the main thread on the
  /// progress of this task.
  void UpdateProgress(StringParam taskName, float percentComplete,
                      StringParam progressText = String());

  /// Returns the name from the task (if the task exists)
  String GetName() const;

protected:
  void Failed();

  /// It's safe to call this on a different thread.
  int Cancel() override;

  /// Sends a given event to the Z::gBackgroundTasks object on the main thread.
  /// Note that the given event should not be allocated on the stack as it's
  /// given to a different thread.
  void SendMainThreadEvent(StringParam eventId, Event* e);

  /// Simply set the state and return from the job. The gBackgroundTasks will
  /// automatically send the appropriate completion event based on this state.
  BackgroundTaskState::Type mState;

private:
  friend class BackgroundTask;
  BackgroundTask* mTask;
};

//-------------------------------------------------------------- Background Task
/// Called when the task is clicked on in the UI
typedef void (*BackgroundTaskActivatedCallback)(BackgroundTask*, Job*);

class BackgroundTask : public EventObject
{
public:
  /// Constructor.
  BackgroundTask(BackgroundTaskJob* job);
  ~BackgroundTask();

  /// Starts the job.
  void Execute();

  /// Cancels the contained job.
  void Cancel();

  /// Whether or not the task has been completed.
  bool IsCompleted();

  /// Returns null if the job is still running
  Job* GetFinishedJob();

  /// Returns the time in seconds since this task was executed.
  float GetTimeAlive();

  float GetEstimatedPercentComplete();

  /// The current state of the task.
  BackgroundTaskState::Type mState;

  /// The name displayed in the Ui.
  String mName;
  String mProgressText;
  float mPercentComplete;
  float mEstimatedTotalDuration;

  /// Set to true if mPercentComplete cannot be updated.
  bool mIndeterminate;

  /// When calculating the average percentage of all active tasks, the average 
  /// will jump backwards once a single task has finished. To avoid this jump,
  /// a task will always be used to calculate the average percentage complete
  /// until all tasks are completed, then this will be set to false.
  bool mUseForAverage;

  /// Display data.
  bool mHidden;
  String mIconName;
  Vec4 mIconColor;
  Vec4 mProgressPrimaryColor;
  Vec4 mProgressBackgroundColor;

  /// Called when the task is completed and clicked on in the Ui.
  BackgroundTaskActivatedCallback mCallback;

  /// Whether or not the "activated callback" is called immediately on
  /// completion of the task
  bool mActivateOnCompleted;

private:
  friend class BackgroundTaskItem;
  friend class BackgroundTasks;

  void Completed();

  clock_t mStartTime;

  /// The job should not be accessed while it's still running (other than
  /// canceling the job).
  Job* mJob;
};

//------------------------------------------------------------- Background Tasks
class BackgroundTasks : public ExplicitSingleton<BackgroundTasks, EventObject>
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BackgroundTasks();
  ~BackgroundTasks();

  /// Creates a background task for the given job. Uses default values for
  /// the display settings.
  BackgroundTask* Execute(BackgroundTaskJob* job, StringParam taskName);

  /// Creates the task, but does not execute it. You must call the Execute
  /// function on the task.
  BackgroundTask* CreateTask(BackgroundTaskJob* job);

  /// All active tasks (even after they're finished).
  Array<BackgroundTask*> mActiveTasks;

private:
  /// Event sent from the 
  void OnTaskUpdated(BackgroundTaskEvent* e);
};

namespace Z
{
  extern BackgroundTasks* gBackgroundTasks;
}//namespace Z

}//namespace Zero
