///////////////////////////////////////////////////////////////////////////////
///
/// \file BackgroundTaskUi.hpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class BackgroundTask;
class ProgressBar;
class BackgroundTaskEvent;
class BackgroundTaskWindow;
class UpdateEvent;

//------------------------------------------------------------- Downloads Button
class BackgroundTaskButton : public IconButton
{
public:
  typedef BackgroundTaskButton ZilchSelf;
  BackgroundTaskButton(Composite* parent);
  
  /// Widget Interface.
  void UpdateTransform() override;

  /// Creates the downloads window when pressed.
  void OnButtonPressed(Event* e);

private:
  /// Flash when a download has completed.
  void Flash(bool toFlash);

  /// Notify that a task has started.
  void OnTaskStarted(BackgroundTaskEvent* e);
  
  /// Updates the progress bar on the bottom of the button.
  void OnTaskUpdated(BackgroundTaskEvent* e);

  /// Notify the user that a task has completed by flashing the icon.
  void OnTaskCompleted(BackgroundTaskEvent* e);

  /// Update the average progress.
  void OnUpdate(UpdateEvent* e);

  /// Computes the average progress for all active tasks.
  void UpdateProgressBar();

  HandleOf<Composite> mTasksWindow;

  /// Shows the average progress of all 
  ProgressBar* mAverageProgress;

  /// Are there currently any non-completed tasks?
  bool mActiveTasks;
};

//---------------------------------------------------------------- Download Item
class BackgroundTaskItem : public Composite
{
public:
  typedef BackgroundTaskItem ZilchSelf;
  BackgroundTaskItem(Composite* parent, BackgroundTask* task);

  /// Widget Interface.
  void UpdateTransform() override;

  /// We need to upgrade the progress when a task is updated.
  void OnBackgroundTaskUpdated(BackgroundTaskEvent* e);
  void OnUpdate(Event* e);

private:
  /// Event response.
  void OnMouseEnter(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  /// X Button event response.
  void OnMouseEnterX(MouseEvent* e);
  void OnLeftClickX(MouseEvent* e);
  void OnMouseExitX(MouseEvent* e);

  Element* mBackground;
  Element* mIcon;
  Text* mNameText;
  Text* mProgressText;
  Element* mXButton;
  ProgressBar* mProgressBar;
  BackgroundTask* mTask;
};

//------------------------------------------------------------- Downloads Window
class BackgroundTaskWindow : public PopUp
{
public:
  typedef BackgroundTaskWindow ZilchSelf;
  BackgroundTaskWindow(Composite* parent);

  /// Widget interface.
  void UpdateTransform() override;

private:
  /// When a task is started, we want to create a new item for it.
  void OnTaskStarted(BackgroundTaskEvent* e);

  Element* mBackground;
};

}//namespace Zero
