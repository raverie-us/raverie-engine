///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(JointLimit, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindDependency(Joint);

  ZilchBindGetterSetterProperty(Active)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(LowerLimit)->ZeroSerialize(real(0));
  ZilchBindGetterSetterProperty(UpperLimit)->ZeroSerialize(real(5));

  ZeroBindEvent(Events::JointLowerLimitReached, JointEvent);
  ZeroBindEvent(Events::JointUpperLimitReached, JointEvent);

  if(Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig))
    ZilchBindGetterSetterProperty(AtomIds)->ZeroSerialize(255u);
  else
    ZilchBindGetterSetter(AtomIds)->ZeroSerialize(255u);

  ZeroBindTag(Tags::Physics);
  ZeroBindTag(Tags::Joint);
}

JointLimit::JointLimit()
{
  mNode = nullptr;
}

JointLimit::~JointLimit()
{
  if(mNode == nullptr)
    return;

  mNode->mLimit = nullptr;
  mNode = nullptr;
}

void JointLimit::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void JointLimit::Initialize(CogInitializer& initializer)
{
  Joint* joint = GetOwner()->has(Joint);
  if(joint)
  {
    mNode = joint->mNode;
    mNode->mLimit = this;
  }
}

bool JointLimit::IsValid()
{
  return mNode != nullptr;
}

bool JointLimit::GetAtomIndexActive(uint atomIndexMask) const
{
  return GetActive() != false && (mAtomIds & atomIndexMask) != 0;
}

bool JointLimit::GetActive() const
{
  return mState.IsSet(JointLimitState::Active);
}

void JointLimit::SetActive(bool active)
{
  mState.SetState(JointLimitState::Active, active);
  if(!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

uint JointLimit::GetAtomIds() const
{
  return mAtomIds;
}

void JointLimit::SetAtomIds(uint atomIds)
{
  mAtomIds = atomIds;
  if(!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

real JointLimit::GetLowerLimit() const
{
  return mMinErr;
}

void JointLimit::SetLowerLimit(real limit)
{
  mMinErr = limit;
  if(!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

real JointLimit::GetUpperLimit() const
{
  return mMaxErr;
}

void JointLimit::SetUpperLimit(real limit)
{
  mMaxErr = limit;
  if(!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

bool JointLimit::GetAtLowerLimit()
{
  return mState.IsSet(JointLimitState::AtLowerLimit);
}

void JointLimit::SetAtLowerLimit(bool state)
{
  return mState.SetState(JointLimitState::AtLowerLimit, state);
}

bool JointLimit::GetWasAtLowerLimit()
{
  return mState.IsSet(JointLimitState::WasAtLowerLimit);
}

void JointLimit::SetWasAtLowerLimit(bool state)
{
  return mState.SetState(JointLimitState::WasAtLowerLimit, state);
}

bool JointLimit::GetAtUpperLimit()
{
  return mState.IsSet(JointLimitState::AtUpperLimit);
}

void JointLimit::SetAtUpperLimit(bool state)
{
  return mState.SetState(JointLimitState::AtUpperLimit, state);
}

bool JointLimit::GetWasAtUpperLimit()
{
  return mState.IsSet(JointLimitState::WasAtUpperLimit);
}

void JointLimit::SetWasAtUpperLimit(bool state)
{
  return mState.SetState(JointLimitState::WasAtUpperLimit, state);
}

}//namespace Zero
