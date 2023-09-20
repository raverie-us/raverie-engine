// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(JointMotor, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
  RaverieBindDependency(Joint);

  RaverieBindGetterSetterProperty(Active)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(Reverse)->RaverieSerialize(false);
  RaverieBindGetterSetterProperty(Speed)->RaverieSerialize(real(5));
  RaverieBindGetterSetterProperty(MaxImpulse)->RaverieSerialize(real(2));

  RaverieBindGetterSetter(AtomIds)->RaverieSerialize(255u);

  RaverieBindTag(Tags::Physics);
  RaverieBindTag(Tags::Joint);
}

JointMotor::JointMotor()
{
  mImpulse = real(0.0);
  mNode = nullptr;
}

JointMotor::~JointMotor()
{
  if (mNode == nullptr)
    return;

  mNode->mMotor = nullptr;
  mNode = nullptr;
}

void JointMotor::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void JointMotor::Initialize(CogInitializer& initializer)
{
  Joint* joint = GetOwner()->has(Joint);
  if (joint)
  {
    mNode = joint->mNode;
    mNode->mMotor = this;
  }
}

bool JointMotor::GetAtomIndexActive(uint atomIndexMask)
{
  return GetActive() != false && (mAtomIds & atomIndexMask) != 0;
}

bool JointMotor::IsValid()
{
  return mNode != nullptr;
}

bool JointMotor::GetActive() const
{
  return mFlags.IsSet(JointMotorFlags::Active);
}

void JointMotor::SetActive(bool active)
{
  mFlags.SetState(JointMotorFlags::Active, active);
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

bool JointMotor::GetReverse() const
{
  return mFlags.IsSet(JointMotorFlags::Reverse);
}

void JointMotor::SetReverse(bool reverse)
{
  mFlags.SetState(JointMotorFlags::Reverse, reverse);
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

uint JointMotor::GetAtomIds() const
{
  return mAtomIds;
}

void JointMotor::SetAtomIds(uint atomIds)
{
  mAtomIds = atomIds;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

real JointMotor::GetMaxImpulse() const
{
  return mMaxImpulse;
}

void JointMotor::SetMaxImpulse(real maxImpulse)
{
  mMaxImpulse = maxImpulse;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

real JointMotor::GetSpeed() const
{
  return mSpeed;
}

void JointMotor::SetSpeed(real speed)
{
  mSpeed = speed;
  if (!IsValid())
    return;

  Physics::JointHelpers::ForceAwakeJoint(mNode->mJoint);
}

} // namespace Raverie
