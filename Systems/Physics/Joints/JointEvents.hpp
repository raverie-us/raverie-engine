///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(JointExceedImpulseLimit);
  DeclareEvent(JointLowerLimitReached);
  DeclareEvent(JointUpperLimitReached);
}

struct Joint;
class Collider;

//-------------------------------------------------------------------JointEvent
/// Sent out when a joint reaches some condition. Currently sent out when
/// a limit is reached or an impulse's limit is exceeded.
class JointEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  JointEvent();

  /// ObjectA on the Joint.
  Cog* GetObjectA();
  /// ObjectB on the Joint.
  Cog* GetObjectB();
  /// The Cog of the Joint that signaled the event.
  Cog* GetJointCog();
  /// The Joint that triggered the event.
  Joint* GetJoint();

  //-------------------------------------------------------------------Internal
  Collider* mColliderA;
  Collider* mColliderB;
  Joint* mJoint;
  String mEventType;

  Link<JointEvent> link;
};

}//namespace Zero
