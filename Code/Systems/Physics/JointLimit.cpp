// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(JointLimit, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
  RaverieBindDependency(Joint);

  RaverieBindGetterSetterProperty(Active)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(LowerLimit)->RaverieSerialize(real(0));
  RaverieBindGetterSetterProperty(UpperLimit)->RaverieSerialize(real(5));

  RaverieBindEvent(Events::JointLowerLimitReached, JointEvent);
  RaverieBindEvent(Events::JointUpperLimitReached, JointEvent);

  RaverieBindGetterSetter(AtomIds)->RaverieSerialize(255u);

  RaverieBindTag(Tags::Physics);
  RaverieBindTag(Tags::Joint);
}

JointLimit::JointLimit()
{
  mNode = nullptr;
}

JointLimit::~JointLimit()
{
  if (mNode == nullptr)
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
  if (joint)
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
  if (!IsValid())
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
  if (!IsValid())
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
  if (!IsValid())
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
  if (!IsValid())
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

} // namespace Raverie
