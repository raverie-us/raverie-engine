///////////////////////////////////////////////////////////////////////////////
///
/// \file Action.cpp
/// Implementation of the action system.
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
Action::Action()
{
}

Action::~Action()
{
}

ZilchDefineType(Action, builder, type)
{
  ZeroBindDocumented();
  ZilchBindMethod(Cancel);
  ZilchBindGetterProperty(Completed);
  ZilchBindGetterProperty(Active);
  ZilchBindGetterProperty(Started);
}

bool Action::GetCompleted()
{
  return mFlags.IsSet(ActionFlag::Completed);
}

bool Action::GetActive()
{
  return !mFlags.IsSet(ActionFlag::NotActive);
}

bool Action::GetStarted()
{
  return mFlags.IsSet(ActionFlag::Started);
}

void Action::Cancel()
{
  CancelOverride();
  mFlags.SetFlag(ActionFlag::NotActive);
}

//------------------------------------------------------------ Action Support

void RemoveAndDecRef(InActionList& list, Action* action)
{
  ErrorIf(!action->mFlags.IsSet(ActionFlag::Linked), "Action is not linked");
  action->mFlags.ClearFlag(ActionFlag::Linked);
  list.Erase(action);
  action->Release();
}

void AddToListAndIncRef(InActionList& list, Action* action)
{
  ReturnIf(action->mFlags.IsSet(ActionFlag::Linked),,"Invalid Add. Action is already on a list.");
  action->mFlags.SetFlag(ActionFlag::Linked);
  list.PushBack(action);
  action->AddReference();
}

void ReleaseActions(InActionList& list)
{
  // Iterator safe release all actions
  InList<Action>::iterator actionIt = list.Begin();
  for(;actionIt!=list.End();)
  {
    Action* action = actionIt;
    ++actionIt;
    list.Erase(action);
    action->Release();
  }
}

void CancelActions(InActionList& list)
{
  InList<Action>::iterator actionIt = list.Begin();
  for(;actionIt!=list.End();++actionIt)
    actionIt->Cancel();
}

ActionState::Enum ProcessActions(InActionList& list, float dt, bool blocking)
{
  InList<Action>::iterator actionIt = list.Begin();
  for(;actionIt!=list.End();)
  {
    Action* action = actionIt;

    // Move iterator forward now so the action
    // can be removed
    ++actionIt;

    // Action are only removed from lists in this function
    // this prevents modification during iteration errors
    if(action->mFlags.IsSet(ActionFlag::NotActive))
    {
      // Remove from list
      RemoveAndDecRef(list, action);
      continue;
    }

    // Update the action
    ActionState::Enum state = action->Update(dt);

    if(state != ActionState::Completed)
    {
      // Blocking action list stop on first non completed
      // action
      if(blocking)
        return ActionState::Running;
    }
    else
    {
      // Set the action as completed
      action->mFlags.SetFlag(ActionFlag::Completed);

      // No longer active
      action->mFlags.SetFlag(ActionFlag::NotActive);

      // Remove from list
      RemoveAndDecRef(list, action);
    }
  }

  if(list.Empty())
    return ActionState::Completed;
  else
    return ActionState::Running;
}


//------------------------------------------------------------ ActionSet

ZilchDefineType(ActionSet, builder, type)
{
}

//------------------------------------------------------------ Actions

ZilchDefineType(Actions, builder, type)
{
}

Actions::Actions(ActionSpace* space)
{
  mActive = false;
  mRealTime = false;
  mSpace = space;
}

Actions::~Actions()
{
  ReleaseActions(mLogicActions);
  ReleaseActions(mFrameActions);
  if(mActive)
    mSpace->ActiveLists.Erase(this);
}

void Actions::CancelOverride()
{
  CancelActions(mLogicActions);
  CancelActions(mFrameActions);
}

void Actions::Add(Action* action)
{
  this->Add(action, ActionExecuteMode::LogicUpdate);
}

void Actions::Add(Action* action, ActionExecuteMode::Enum mode)
{
  if(!mActive)
  {
    // Activate this action object so it gets updates
    mSpace->ActiveLists.PushBack(this);
    mActive = true;
  }

  if(mode == ActionExecuteMode::FrameUpdate)
    AddToListAndIncRef(mFrameActions, action);
  else
    AddToListAndIncRef(mLogicActions, action);
}

bool Actions::IsEmpty()
{
  return mLogicActions.Empty() && mFrameActions.Empty();
}

ActionState::Enum Actions::Update(float dt)
{
  return ActionState::Running;
}

void Actions::Update(float dt, float realDt, ActionExecuteMode::Enum mode)
{
  if(mRealTime)
    dt = realDt;

  if(mode == ActionExecuteMode::FrameUpdate)
    ProcessActions(mFrameActions, dt, false);
  else
    ProcessActions(mLogicActions, dt, false);
  
  // Remove from list for update efficiency
  if(IsEmpty())
  {
    mSpace->ActiveLists.Erase(this);
    mActive = false;
  }
}

//------------------------------------------------------------ ActionGroup

ZilchDefineType(ActionGroup, builder, type)
{
}

ActionGroup::ActionGroup()
{

}

ActionGroup::ActionGroup(Object* object, ActionExecuteMode::Enum mode)
{
  object->GetActions()->Add(this, mode);
}

ActionGroup::~ActionGroup()
{
  ReleaseActions(mActions);
}

bool ActionGroup::IsEmpty()
{
  return mActions.Empty();
}

void ActionGroup::Add(Action* action)
{
  AddToListAndIncRef(mActions, action);
}

ActionState::Enum ActionGroup::Update(float dt)
{
  mFlags.SetFlag(ActionFlag::Started);
  return ProcessActions(mActions, dt, false);
}

void ActionGroup::CancelOverride()
{
  CancelActions(mActions);
}

//------------------------------------------------------------ ActionSequence

ZilchDefineType(ActionSequence, builder, type)
{
}

ActionSequence::ActionSequence()
{

}

ActionSequence::ActionSequence(Object* object, ActionExecuteMode::Enum mode)
{
  Actions* actions = object->GetActions();
  actions->Add(this, mode);
}

ActionSequence::~ActionSequence()
{
  ReleaseActions(mActions);
}

void ActionSequence::CancelOverride()
{
  CancelActions(mActions);
}

void ActionSequence::Add(Action* action)
{
  AddToListAndIncRef(mActions, action);
}

bool ActionSequence::IsEmpty()
{
  return mActions.Empty();
}

ActionState::Enum ActionSequence::Update(float dt)
{
  mFlags.SetFlag(ActionFlag::Started);
  return ProcessActions(mActions, dt, true);
}

}
