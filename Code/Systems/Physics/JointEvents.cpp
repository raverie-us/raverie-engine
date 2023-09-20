// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(JointExceedImpulseLimit);
DefineEvent(JointLowerLimitReached);
DefineEvent(JointUpperLimitReached);
} // namespace Events

RaverieDefineType(JointEvent, builder, type)
{
  RaverieBindDocumented();
  RaverieBindGetterProperty(ObjectA);
  RaverieBindGetterProperty(ObjectB);
  RaverieBindGetterProperty(JointCog);
  RaverieBindGetterProperty(Joint);

  RaverieBindTag(Tags::Physics);
}

JointEvent::JointEvent()
{
  mColliderA = nullptr;
  mColliderB = nullptr;
  mJoint = nullptr;
}

Cog* JointEvent::GetObjectA()
{
  return mColliderA->GetOwner();
}

Cog* JointEvent::GetObjectB()
{
  return mColliderB->GetOwner();
}

Cog* JointEvent::GetJointCog()
{
  return mJoint->GetOwner();
}

Joint* JointEvent::GetJoint()
{
  return mJoint;
}

} // namespace Raverie
