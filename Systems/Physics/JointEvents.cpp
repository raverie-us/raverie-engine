///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(JointExceedImpulseLimit);
  DefineEvent(JointLowerLimitReached);
  DefineEvent(JointUpperLimitReached);
}

//-------------------------------------------------------------------JointEvent
ZilchDefineType(JointEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindGetterProperty(ObjectA);
  ZilchBindGetterProperty(ObjectB);
  ZilchBindGetterProperty(JointCog);
  ZilchBindGetterProperty(Joint);

  ZeroBindTag(Tags::Physics);
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

}//namespace Zero
