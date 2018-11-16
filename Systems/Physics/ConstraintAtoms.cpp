///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
  
namespace Physics
{

AnchorAtom::AnchorAtom()
{
  mBodyR[0] = mBodyR[1] = Vec3::cZero;
}

Vec3Ref AnchorAtom::operator[](uint index)
{
  return mBodyR[index];
}

Vec3Param AnchorAtom::operator[](uint index) const
{
  return mBodyR[index];
}

WorldAnchorAtom::WorldAnchorAtom(const AnchorAtom& anchor, Joint* joint)
{
  SetUp(anchor,joint->GetCollider(0), joint->GetCollider(1));
}

WorldAnchorAtom::WorldAnchorAtom(const AnchorAtom& anchor, Collider* obj1, Collider* obj2)
{
  SetUp(anchor, obj1, obj2);
}

void WorldAnchorAtom::SetUp(const AnchorAtom& anchor, Collider* obj1, Collider* obj2)
{
  mWorldR[0] = JointHelpers::BodyRToCenterMassR(obj1, anchor.mBodyR[0]);
  mWorldR[1] = JointHelpers::BodyRToCenterMassR(obj2, anchor.mBodyR[1]);

  mWorldPoints[0] = mWorldR[0] + obj1->GetRigidBodyCenterOfMass();
  mWorldPoints[1] = mWorldR[1] + obj2->GetRigidBodyCenterOfMass();
}

Vec3 WorldAnchorAtom::GetPointDifference() const
{
  return mWorldPoints[1] - mWorldPoints[0];
}

WorldAxisAtom::WorldAxisAtom(const AxisAtom& axes, Collider* obj1, Collider* obj2)
{
  mWorldAxes[0] = JointHelpers::BodyToWorldR(obj1, axes.mBodyAxes[0]);
  mWorldAxes[1] = JointHelpers::BodyToWorldR(obj2, axes.mBodyAxes[1]);
  mWorldAxes[0].AttemptNormalize();
  mWorldAxes[1].AttemptNormalize();
}

Quat AngleAtom::GetReferenceAngle() const
{
  return mLocalAngles[0].Conjugated() * mLocalAngles[1];
}

WorldAngleAtom::WorldAngleAtom()
{
  mWorldAngles[0] = Quat::cIdentity;
  mWorldAngles[1] = Quat::cIdentity;
  mWorldReferenceAngle = Quat::cIdentity;
  mEulerAngles.ZeroOut();
}

WorldAngleAtom::WorldAngleAtom(const AngleAtom& refAngle, Collider* obj1, Collider* obj2)
{
  Quat obj1Rotation = Math::ToQuaternion(obj1->GetWorldRotation());
  Quat obj2Rotation = Math::ToQuaternion(obj2->GetWorldRotation());

  mWorldAngles[0] = obj1Rotation * refAngle.mLocalAngles[0];
  mWorldAngles[1] = obj2Rotation * refAngle.mLocalAngles[1];
  mWorldAngles[0].Normalize();
  mWorldAngles[1].Normalize();

  mWorldReferenceAngle = mWorldAngles[1] * mWorldAngles[0].Conjugated();
  Math::EulerAngles angles = Math::ToEulerAngles(mWorldReferenceAngle);
  
  mEulerAngles = angles.Angles;
}

ConstraintAtom::ConstraintAtom()
{
  mConstraintValue = mImpulse = real(0.0);
}

ImpulseLimitAtom::ImpulseLimitAtom(real maxImpulse)
{
  mMaxImpulse = maxImpulse;
  mMinImpulse = -maxImpulse;
}

ImpulseLimitAtom::ImpulseLimitAtom(real maxImpulse, real minImpulse)
{
  mMaxImpulse = maxImpulse;
  mMinImpulse = minImpulse;
}

}//namespace Physics

}//namespace Zero
