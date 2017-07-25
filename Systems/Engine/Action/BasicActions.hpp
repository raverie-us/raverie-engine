///////////////////////////////////////////////////////////////////////////////
///
/// \file ActionBasic.hpp
/// Declaration of the basic actions.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once 

namespace Zero
{

////Basic Actions////////

///Delay action. Delays by time in seconds.
class ActionDelay : public Action
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ActionDelay(float duration)
    : mTimeLeft(duration)
  {
    mFlags.SetFlag(ActionFlag::Schedulable);
  }

  ActionState::Enum Update(float dt) override
  {
    mFlags.SetFlag(ActionFlag::Started);
    mTimeLeft-=dt;
    if(mTimeLeft > 0.0f)
      return ActionState::Running;
    else
      return ActionState::Completed;
  }
private:
  float mTimeLeft;
};


class ActionDelayOnce : public Action
{
public:
  ActionDelayOnce()
  {
  }

  ActionState::Enum Update(float dt) override
  {
    if(!mFlags.IsSet(ActionFlag::Started))
    {
      mFlags.SetFlag(ActionFlag::Started);
      return ActionState::Running;
    }
    else
    {
      return ActionState::Completed;
    }
  }
private:
};

/// Temporary action (needs to be reworked for zilch) that sends an
/// event to an object. Takes ownership of the event to send.
class ActionEvent : public Action
{
public:
  String mEventName;
  HandleOf<Event> mEvent;
  HandleOf<Object> mTargetObject;

  ActionEvent(Object* targetObject, StringParam eventNameToSend, Event* eventToSend)
  {
    mTargetObject = targetObject;
    mEventName = eventNameToSend;
    mEvent = eventToSend;
  }

  ActionState::Enum Update(float dt) override
  {
    if(mTargetObject.IsNull())
      return ActionState::Completed;

    if(Object* eventObject = mTargetObject)
    {
      Event* event = mEvent;
      if (event == nullptr)
      {
        // Should this be an error?
        DoNotifyError("ActionEvent Failed", "The event inside the action was destructed. Did you "
          "send a C++ event? C++ events are destructed and generally cannot be saved for later.");
        return ActionState::Completed;
      }
      
      eventObject->GetDispatcher()->Dispatch(mEventName, event);
      mEvent.Delete();
    }

    return ActionState::Completed;
  }
};

}
