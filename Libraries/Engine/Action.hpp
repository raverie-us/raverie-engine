// MIT Licensed (see LICENSE.md).
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
  ZilchDeclareType(Action, TypeCopyMode::ReferenceType);

  IntrusiveLink(Action, link);

  // Flags
  BitField<ActionFlag::Enum> mFlags;

  virtual ActionState::Enum Update(float dt) = 0;

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

/// Base class for ActionGroup and ActionSequence. Stores a set of sub-actions.
class ActionSet : public Action
{
public:
  ZilchDeclareType(ActionSet, TypeCopyMode::ReferenceType);
  virtual void Add(Action* action) = 0;
  virtual bool IsEmpty() = 0;
};

/// The collection of actions queued up on an object.
class Actions : public ActionSet
{
public:
  ZilchDeclareType(Actions, TypeCopyMode::ReferenceType);

  IntrusiveLink(Actions, link);

  Actions(ActionSpace* actionSpace);
  ~Actions();

  void CancelOverride() override;
  bool IsEmpty() override;
  void Add(Action* action) override;
  ActionState::Enum Update(float dt) override;

  void Update(float dt, float rdt, ActionExecuteMode::Enum mode);
  void Add(Action* action, ActionExecuteMode::Enum mode);
  void SetRealTime(bool value)
  {
    mRealTime = value;
  }

private:
  ActionSpace* mSpace;
  InActionList mLogicActions;
  InActionList mFrameActions;
  bool mActive;
  bool mRealTime;
};

/// A group of actions that run in parallel with each other.
class ActionGroup : public ActionSet
{
public:
  ZilchDeclareType(ActionGroup, TypeCopyMode::ReferenceType);

  ActionGroup();
  ~ActionGroup();
  ActionGroup(Object* object, ActionExecuteMode::Enum mode = ActionExecuteMode::LogicUpdate);

  void Add(Action* action) override;
  ActionState::Enum Update(float dt) override;
  void CancelOverride() override;
  bool IsEmpty() override;

private:
  InActionList mActions;
};

/// A group of actions that run in serial. The first child action will block the
/// second from running and so on.
class ActionSequence : public ActionSet
{
public:
  ZilchDeclareType(ActionSequence, TypeCopyMode::ReferenceType);

  ActionSequence();
  ~ActionSequence();
  ActionSequence(Object* object, ActionExecuteMode::Enum mode = ActionExecuteMode::LogicUpdate);

  void Add(Action* action) override;
  ActionState::Enum Update(float dt) override;
  void CancelOverride() override;
  bool IsEmpty() override;

private:
  InActionList mActions;
};

} // namespace Zero
