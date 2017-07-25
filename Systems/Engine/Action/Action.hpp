///////////////////////////////////////////////////////////////////////////////
///
/// \file Action.hpp
/// Declaration of the action system.
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField5(ActionFlag, Started, Completed, NotActive, Linked, Schedulable);

DeclareEnum2(ActionExecuteMode, LogicUpdate, FrameUpdate);
DeclareEnum2(ActionState, Running, Completed);

/// Base action class
class Action : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  IntrusiveLink(Action, link);

  //Flags
  BitField<ActionFlag::Enum> mFlags;

  virtual ActionState::Enum Update(float dt)=0;

  /// Cancel the action and all child actions.
  void Cancel();

  /// Allow inherited actions to perform custom logic
  virtual void CancelOverride(){};

  /// The action ran until it completed.
  bool GetCompleted();

  /// Has the action started?
  bool GetStarted();

  /// The action is queued and not stared or running.
  bool GetActive();

  Action();
  ~Action();
};

class ActionSpace;
class TimeSpace;
typedef InList<Action> InActionList;

/// Base class for ActionGroup and ActionSet
class ActionSet : public Action
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  virtual void Add(Action* action) = 0;
  virtual bool IsEmpty() = 0;
};

/// Actions stored on a object
class Actions : public ActionSet
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  IntrusiveLink(Actions, link);

  Actions(ActionSpace* actionSpace);
  ~Actions();

  void CancelOverride() override;
  bool IsEmpty() override;
  void Add(Action* action) override;
  ActionState::Enum Update(float dt) override;

  void Update(float dt, float rdt, ActionExecuteMode::Enum mode);
  void Add(Action* action, ActionExecuteMode::Enum mode);
  void SetRealTime(bool value){mRealTime = value;}

private:
  ActionSpace* mSpace;
  InActionList mLogicActions;
  InActionList mFrameActions;
  bool mActive;
  bool mRealTime;
};

/// A action group is just a group of actions.
/// Non blocking.
class ActionGroup : public ActionSet
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ActionGroup();
  ~ActionGroup();
  ActionGroup(Object* object, ActionExecuteMode::Enum mode = ActionExecuteMode::LogicUpdate);

  void Add(Action* action) override;
  ActionState::Enum  Update(float dt) override;
  void CancelOverride() override;
  bool IsEmpty() override;

private:
  InActionList mActions;
};

/// An action Sequence performs action in Sequence.
/// Blocking the next action.
class ActionSequence : public ActionSet
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ActionSequence();
  ~ActionSequence();
  ActionSequence(Object* object, ActionExecuteMode::Enum mode = ActionExecuteMode::LogicUpdate);

  void Add(Action* action) override;
  ActionState::Enum  Update(float dt) override;
  void CancelOverride() override;
  bool IsEmpty() override;

private:
  InActionList mActions;
};

}
