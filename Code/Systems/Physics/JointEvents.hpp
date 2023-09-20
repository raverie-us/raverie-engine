// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(JointExceedImpulseLimit);
DeclareEvent(JointLowerLimitReached);
DeclareEvent(JointUpperLimitReached);
} // namespace Events

struct Joint;
class Collider;

/// Sent out when a joint reaches some condition. Currently sent out when
/// a limit is reached or an impulse's limit is exceeded.
class JointEvent : public Event
{
public:
  RaverieDeclareType(JointEvent, TypeCopyMode::ReferenceType);

  JointEvent();

  /// ObjectA on the Joint.
  Cog* GetObjectA();
  /// ObjectB on the Joint.
  Cog* GetObjectB();
  /// The Cog of the Joint that signaled the event.
  Cog* GetJointCog();
  /// The Joint that triggered the event.
  Joint* GetJoint();

  Collider* mColliderA;
  Collider* mColliderB;
  Joint* mJoint;
  String mEventType;

  Link<JointEvent> link;
};

} // namespace Raverie
