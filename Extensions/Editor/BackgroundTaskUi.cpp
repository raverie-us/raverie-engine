///////////////////////////////////////////////////////////////////////////////
///
/// \file BackgroundTaskUi.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace TaskUi
{
const cstr cLocation = "EditorUi/BackgroundTasks";
Tweakable(Vec4,  WindowBackgroundColor,   Vec4(1,1,1,1),   cLocation);
Tweakable(Vec4,  TaskBackgroundColor,     Vec4(1,1,1,1),   cLocation);
Tweakable(Vec4,  TaskBackgroundHighlight, Vec4(1,1,1,1),   cLocation);
Tweakable(Vec4,  TaskXColor,              Vec4(1,1,1,1),   cLocation);
Tweakable(Vec4,  TaskXHighlight,          Vec4(1,1,1,1),   cLocation);
Tweakable(Vec2,  TaskSize,                Pixels(195, 30), cLocation);
Tweakable(float, ProgressBarHeight,       Pixels(2),       cLocation);
}

namespace TaskButtonUi
{
const cstr cLocation = "EditorUi/BackgroundTasks/TaskButton";
Tweakable(Vec4, IconColor,               Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, IconFlash,               Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, IconBackgroundFlash,     Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ProgressPrimaryColor,    Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ProgressBackgroundColor, Vec4(1,1,1,1), cLocation);
}

//------------------------------------------------------- Background Task Button
//******************************************************************************
BackgroundTaskButton::BackgroundTaskButton(Composite* parent)
  : IconButton(parent)
{
  SetIcon("TaskDownload");
  mToolTipText = "Background Tasks";
  mIcon->SetColor(TaskButtonUi::IconColor);

  mAverageProgress = new ProgressBar(this);
  mAverageProgress->SetTextVisible(false);
  mAverageProgress->SetPrimaryColor(TaskButtonUi::ProgressPrimaryColor);
  mAverageProgress->SetBackgroundColor(TaskButtonUi::ProgressBackgroundColor);
  mAverageProgress->SetPercentage(0.0f);
  mAverageProgress->mPadding = Thickness::cZero;

  ConnectThisTo(this, Events::ButtonPressed, OnButtonPressed);
  ConnectThisTo(Z::gBackgroundTasks, Events::BackgroundTaskStarted, OnTaskStarted);
  ConnectThisTo(Z::gBackgroundTasks, Events::BackgroundTaskUpdated, OnTaskUpdated);
  ConnectThisTo(Z::gBackgroundTasks, Events::BackgroundTaskCompleted, OnTaskCompleted);

  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
  mActiveTasks = false;
}

//******************************************************************************
void BackgroundTaskButton::UpdateTransform()
{
  float height = TaskUi::ProgressBarHeight;
  mAverageProgress->SetTranslation(Vec3(Pixels(1), mSize.y - height, 0));
  mAverageProgress->SetSize(Vec2(mSize.x - Pixels(2), height));
  IconButton::UpdateTransform();
}

//******************************************************************************
void BackgroundTaskButton::OnButtonPressed(Event* e)
{
  // Cancel the flash actions
  GetActions()->Cancel();
  mIcon->SetColor(TaskButtonUi::IconColor);

  // Close the window if it's already open
  if(mTasksWindow.IsNotNull())
  {
    mTasksWindow.SafeDestroy();
    return;
  }

  // Create the window
  BackgroundTaskWindow* window = new BackgroundTaskWindow(GetRootWidget());
  LayoutArea data;
  window->SetSize(window->Measure(data));

  Rect buttonRect = GetScreenRect();
  Vec2 pos = buttonRect.BottomRight() - Vec2(window->mSize.x, 0);
  window->SetTranslation(ToVector3(pos));

  mTasksWindow = window;
}

//******************************************************************************
void BackgroundTaskButton::Flash(bool toFlash)
{
  float t = 1.0f;
  Vec4 color = TaskButtonUi::IconColor;
  if(toFlash == 1)
    color = TaskButtonUi::IconFlash;

  ActionSequence* seq = new ActionSequence(this, ActionExecuteMode::FrameUpdate);
  seq->Add(AnimatePropertyGetSet(Widget, Color, Ease::Quad::InOut, mIcon, t, color));
  seq->Add(new CallParamAction<ZilchSelf, bool, &ZilchSelf::Flash>(this, !toFlash));
}

//******************************************************************************
void BackgroundTaskButton::OnTaskStarted(BackgroundTaskEvent* e)
{
  Element* signal = GetRootWidget()->CreateAttached<Element>("TaskDownloadCenter");
  signal->SetTranslation(ToVector3(GetScreenRect().Center()));

  ActionSequence* seq = new ActionSequence(signal);
  seq->Add(SizeWidgetAction(signal, Vec2(0,0), 0.8f));
  seq->Add(DestroyAction(signal));
  signal->mSize *= 5.0f;

  // Make it transparent
  Vec4 flashColor = TaskButtonUi::IconFlash;
  flashColor.w = 0.4f;
  signal->SetColor(flashColor);
}

//******************************************************************************
void BackgroundTaskButton::OnTaskUpdated(BackgroundTaskEvent* e)
{
  UpdateProgressBar();
}

//******************************************************************************
void BackgroundTaskButton::OnTaskCompleted(BackgroundTaskEvent* e)
{
  // Ignore hidden tasks
  if(e->mTask->mHidden)
    return;

  // Notify the user that the task has been completed
  String message = String::Format("The background task %s has completed.",
                                   e->mTask->mName.c_str());
  DoNotify("Background task completed", message, "TaskGear");

  // If they have the task window
  if(mTasksWindow.IsNotNull())
    return;

  Flash(true);
}

//******************************************************************************
void BackgroundTaskButton::OnUpdate(UpdateEvent* e)
{
  UpdateProgressBar();

  Vec4 color = TaskButtonUi::IconColor;

  // Flash green if there are active tasks
  if(mActiveTasks)
  {
    // Convert a sin wave to 0-1 to be used as an interpolant between colors
    float t = Math::Sin(e->TimePassed * 5.0f) * 0.5f + 0.5f;
    // Don't go all the way green. Just looks better
    t *= 0.7;
    color = Math::Lerp(Vec4(TaskButtonUi::IconColor), Vec4(TaskButtonUi::IconFlash), t);
  }

  mIconColor = ToByteColor(color);
  mIconHoverColor = mIconColor;
  mIconClickedColor = mIconColor;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BackgroundTaskButton::UpdateProgressBar()
{
  uint count = 0;
  float averagePercent = 0.0f;
  mActiveTasks = false;
  forRange(BackgroundTask* task, Z::gBackgroundTasks->mActiveTasks.All())
  {
    // Ignore hidden tasks and tasks that shouldn't be accounted for
    if(task->mHidden || !task->mUseForAverage)
      continue;

    averagePercent += task->GetEstimatedPercentComplete();
    ++count;
    mActiveTasks = true;
  }

  averagePercent /= float(count);
  mAverageProgress->SetPercentage(averagePercent);

  // If all tasks are done, mark them so we don't use them for the average
  float cEpsilon = 0.00001f;
  if(averagePercent >= (1.0 - cEpsilon))
  {
    forRange(BackgroundTask* task, Z::gBackgroundTasks->mActiveTasks.All())
      task->mUseForAverage = false;
  }
}

//--------------------------------------------------------- Background Task Item
//******************************************************************************
BackgroundTaskItem::BackgroundTaskItem(Composite* parent, BackgroundTask* task)
  : Composite(parent), mTask(task)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(TaskUi::TaskBackgroundColor);
  mBackground->SetNotInLayout(true);

  SetLayout(CreateStackLayout());

  Composite* infoArea = new Composite(this);
  //infoArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(5,0), Thickness::cZero));
  infoArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, Pixels(1));
  {
    mIcon = infoArea->CreateAttached<Element>(task->mIconName);
    mIcon->SetColor(task->mIconColor);

    mNameText = new Text(infoArea, cText);
    mNameText->SetText(task->mName);

    mProgressText = new Text(infoArea, cText);
    mProgressText->SetText(task->mProgressText);
  }

  mXButton = CreateAttached<Element>("Close");
  mXButton->SetColor(TaskUi::TaskXColor);
  ConnectThisTo(mXButton, Events::MouseEnter, OnMouseEnterX);
  ConnectThisTo(mXButton, Events::LeftClick, OnLeftClickX);
  ConnectThisTo(mXButton, Events::MouseExit, OnMouseExitX);

  mProgressBar = new ProgressBar(this);
  mProgressBar->SetPercentage(task->mPercentComplete);
  mProgressBar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, TaskUi::ProgressBarHeight);
  mProgressBar->SetTextVisible(false);
  mProgressBar->mPadding = Thickness::cZero;
  mProgressBar->SetPrimaryColor(task->mProgressPrimaryColor);
  mProgressBar->SetBackgroundColor(task->mProgressBackgroundColor);
  ConnectThisTo(task, Events::BackgroundTaskUpdated, OnBackgroundTaskUpdated);
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnBackgroundTaskUpdated);
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);

  ConnectThisTo(this, Events::MouseEnterHierarchy, OnMouseEnter);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::MouseExitHierarchy, OnMouseExit);
}

//******************************************************************************
void BackgroundTaskItem::UpdateTransform()
{
  mNameText->SetTranslation(Pixels(30, 2, 0));
  mNameText->SizeToContents();
  mProgressText->SetTranslation(Pixels(30, 13, 0));
  mProgressText->SizeToContents();
  mBackground->SetSize(mSize);

  Rect local = GetLocalRect();
  PlaceCenterToRect(local, mXButton);
  mXButton->mTranslation.x = mSize.x - mXButton->GetSize().x - Pixels(2);
  Composite::UpdateTransform();
}

//******************************************************************************
void BackgroundTaskItem::OnBackgroundTaskUpdated(BackgroundTaskEvent* e)
{
  mProgressText->SetText(e->ProgressText);

  MarkAsNeedsUpdate();
}

//******************************************************************************
void BackgroundTaskItem::OnUpdate(Event* e)
{
  mProgressBar->SetPercentage(mTask->GetEstimatedPercentComplete());

  MarkAsNeedsUpdate();
}

//******************************************************************************
void BackgroundTaskItem::OnMouseEnter(MouseEvent* e)
{
  mBackground->SetColor(TaskUi::TaskBackgroundHighlight);
}

//******************************************************************************
void BackgroundTaskItem::OnLeftClick(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Only call it if the task was completed
  if(mTask->mCallback && mTask->mState == BackgroundTaskState::Completed)
    (mTask->mCallback)(mTask, mTask->mJob);
}

//******************************************************************************
void BackgroundTaskItem::OnMouseExit(MouseEvent* e)
{
  mBackground->SetColor(TaskUi::TaskBackgroundColor);
}

//******************************************************************************
void BackgroundTaskItem::OnMouseEnterX(MouseEvent* e)
{
  mXButton->SetColor(TaskUi::TaskXHighlight);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BackgroundTaskItem::OnLeftClickX(MouseEvent* e)
{
  e->Handled = true;

  // Hide the task and destroy ourself
  mTask->mHidden = true;
  this->Destroy();

  // Need to tell our parent we were destroyed
  GetParent()->MarkAsNeedsUpdate();
}

//******************************************************************************
void BackgroundTaskItem::OnMouseExitX(MouseEvent* e)
{
  mXButton->SetColor(TaskUi::TaskXColor);
  MarkAsNeedsUpdate();
}

//------------------------------------------------------- Background Task Window
//******************************************************************************
BackgroundTaskWindow::BackgroundTaskWindow(Composite* parent)
  : PopUp(parent, PopUpCloseMode::MouseOutTarget)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(TaskUi::WindowBackgroundColor);
  mBackground->SetNotInLayout(true);
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0,2), Thickness(2, 2, 2, 2)));

  bool foundActiveTask = false;
  forRange(BackgroundTask* task, Z::gBackgroundTasks->mActiveTasks.All())
  {
    // Ignore hidden tasks
    if(task->mHidden)
      continue;
    foundActiveTask = true;
    BackgroundTaskItem* item = new BackgroundTaskItem(this, task);
    Vec2 itemSize = TaskUi::TaskSize;
    item->SetSizing(SizeAxis::X, SizePolicy::Fixed, itemSize.x);
    item->SetSizing(SizeAxis::Y, SizePolicy::Fixed, itemSize.y);
  }
  if(foundActiveTask == false)
  {
    this->Destroy();
    return;
  }

  ConnectThisTo(Z::gBackgroundTasks, Events::BackgroundTaskStarted, OnTaskStarted);
}

//******************************************************************************
void BackgroundTaskWindow::UpdateTransform()
{
  mBackground->SetSize(mSize);
  PopUp::UpdateTransform();
}

//******************************************************************************
void BackgroundTaskWindow::OnTaskStarted(BackgroundTaskEvent* e)
{
  // Don't display hidden tasks
  if(e->mTask->mHidden)
    return;

  // Create the task item
  BackgroundTaskItem* item = new BackgroundTaskItem(this, e->mTask);
  Vec2 itemSize = TaskUi::TaskSize;
  item->SetSizing(SizeAxis::X, SizePolicy::Fixed, itemSize.x);
  item->SetSizing(SizeAxis::Y, SizePolicy::Fixed, itemSize.y);

  // Resize ourself
  LayoutArea data;
  SetSize(Measure(data));
}

}//namespace Zero
