///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
Linear Constraint:
  Let i correspond to the x, y, z axes.
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

  Angular Constraint:
  Let i correspond the 3 basis vectors formed from the motor axis and two
  orthonormal vectors (t1,t2) to the motor axis where:
  i = 0 -> t1, i = 1 -> t2, i = 2 -> motor axis
  Let i correspond to the x,y,z axes
  Ci   : theta2_i - theta1_i = 0
  cDoti: dot(w2,i) - dot(w1,i) = 0
  Ji   : [0, -i, 0, i]

  normally i = 2 is only solved when it's limit is breached, when a motor is
  present, or is just not solved at all
*/

namespace Zero
{

namespace Physics
{

JointInfo RevoluteJoint::sInfo = JointInfo(6, 0x20);

ImplementJointType(RevoluteJoint);

ImplementAnchorAccessors(RevoluteJoint, mAnchors);
ImplementAngleAccessors(RevoluteJoint, mReferenceAngle);

ZilchDefineType(RevoluteJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  // Instead of binding the angles as usual, bind them with basis editors for cleaner updating with gizmos
  //BindAngleAccessors();
  ZilchBindGetterSetterProperty(LocalBasisA)->Add(new EditorRotationBasis("RevoluteJointBasisGizmo", "EditorRevoluteGizmoName", 0b01));
  ZilchBindGetterSetterProperty(LocalBasisB)->Add(new EditorRotationBasis("RevoluteJointBasisGizmo", "EditorRevoluteGizmoName", 0b10));
  ZilchBindGetterSetterProperty(WorldBasis)->Add(new EditorRotationBasis("RevoluteJointBasisGizmo", "EditorRevoluteGizmoName", 0b11));
  ZilchBindGetterSetterProperty(FrameOfReference);

  ZilchBindMethod(SetWorldFrame);
}

RevoluteJoint::RevoluteJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void RevoluteJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  // Turn the frame index into a bool for whether or not object A's frame is used
  bool UseFrameA = (mPrimaryFrameIndex == 0);
  SerializeNameDefault(UseFrameA, true);

  SerializeAnchors(stream, mAnchors, Vec3(1));
  SerializeAngles(stream, mReferenceAngle);

  // When loading, turn the bool back into the index
  if(stream.GetMode() == SerializerMode::Loading)
  {
    mPrimaryFrameIndex = 0;
    if(!UseFrameA)
      mPrimaryFrameIndex = 1;
  }
}

void RevoluteJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void RevoluteJoint::ComputeInitialConfiguration()
{
  // First just grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);
  
  // Next compute the axis of the revolute as the axis between the two points
  Vec3 p0 = GetWorldPointA();
  Vec3 p1 = GetWorldPointB();
  Vec3 axis = p1 - p0;
  real length = axis.AttemptNormalize();
  // If we got an invalid axis then just use the y axis...
  if(length == real(0.0))
    axis = Vec3::cYAxis;
  SetWorldAxis(axis);
}

void RevoluteJoint::ComponentAdded(BoundType* typeId, Component* component)
{
  Joint::ComponentAdded(typeId,component);
  if(typeId == ZilchTypeId(JointLimit))
  {
    JointLimit* limit = static_cast<JointLimit*>(component);
    limit->mMinErr = -Math::cPi * real(0.25);
    limit->mMaxErr = Math::cPi * real(0.25);
  }
}

void RevoluteJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, &mReferenceAngle, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularBases(moleculeData.mWorldBases[mPrimaryFrameIndex]);
  moleculeData.mAxes[0] = moleculeData.mWorldBases[0].GetBasis(2);
  moleculeData.mAxes[1] = moleculeData.mWorldBases[1].GetBasis(2);
}

void RevoluteJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, DefaultAngularLimitPolicy<RevoluteJoint>());
}

uint RevoluteJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void RevoluteJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, DefaultAngularLimitPolicy<RevoluteJoint>());
}

void RevoluteJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void RevoluteJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void RevoluteJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint RevoluteJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void RevoluteJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, DefaultAngularLimitPolicy<RevoluteJoint>());
}

void RevoluteJoint::DebugDraw()
{
  if(!GetValid())
    return;

  Collider* collider0 = GetCollider(0);
  Collider* collider1 = GetCollider(1);
  if(collider0 == nullptr || collider1 == nullptr)
    return;

  Mat3 basis0 = collider0->GetWorldRotation() * Math::ToMatrix3(mReferenceAngle[0]);
  Mat3 basis1 = collider1->GetWorldRotation() * Math::ToMatrix3(mReferenceAngle[1]);
  DrawHinge(this, mAnchors, basis0, basis1, mPrimaryFrameIndex);
}

uint RevoluteJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 3)
    return LinearAxis;
  else if(atomIndex < 6)
    return AngularAxis;
  return 0;
}

void RevoluteJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint RevoluteJoint::GetDefaultLimitIds() const
{
  return SingleAngularAxis;
}

uint RevoluteJoint::GetDefaultMotorIds() const
{
  return SingleAngularAxis;
}

uint RevoluteJoint::GetDefaultSpringIds() const
{
  return SingleAngularAxis;
}

real RevoluteJoint::GetJointAngle() const
{
  uint objFrame = mPrimaryFrameIndex;
  uint otherFrame = (objFrame + 1) % 2;

  MoleculeData data;
  data.SetUp(nullptr, &mReferenceAngle, this);
  Vec3 basisX = data.mWorldBases[objFrame].GetBasis(0);
  Vec3 basisY = data.mWorldBases[objFrame].GetBasis(1);
  Vec3 testAxis = data.mWorldBases[otherFrame].GetBasis(0);

  real xProj = Math::Dot(testAxis, basisX);
  real yProj = Math::Dot(testAxis, basisY);
  return Math::ArcTan2(yProj, xProj);
}

Vec3 RevoluteJoint::GetWorldAxis() const
{
  uint objFrame = mPrimaryFrameIndex;
  Collider* collider = GetCollider(objFrame);
  if(collider == nullptr)
    return Vec3::cZero;

  Mat3 rot = collider->GetWorldRotation();
  Mat3 frame = rot * Math::ToMatrix3(mReferenceAngle[objFrame]);
  return frame.GetBasis(2);
}

void RevoluteJoint::SetWorldAxis(Vec3Param axis)
{
  if(axis == Vec3::cZero)
    return;

  Vec3 normalizedAxis = axis.AttemptNormalized();

  uint objFrame = mPrimaryFrameIndex;
  Collider* collider = GetCollider(objFrame);
  if(collider == nullptr)
    return;

  UpdateColliderCachedTransforms();

  // Bring our primary frame into world space
  Quat rot = Math::ToQuaternion(collider->GetWorldRotation());
  Quat worldFrame = rot * mReferenceAngle[objFrame];
  // Now build a new frame from that axis and world frame
  Quat newFrame = BuildFrameFromAxis(worldFrame, normalizedAxis);
  // Set up the new world frame
  SetWorldFrame(newFrame);
}

void RevoluteJoint::SetWorldFrame(QuatParam rot)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "LocalBasisA", GetLocalBasisA());
    OperationQueue::RegisterSideEffect(this, "LocalBasisB", GetLocalBasisB());
  }

  // This could get called twice since SetWorldAxis calls this,
  // but it's a very cheap check if there's nothing to do.
  UpdateColliderCachedTransforms();

  // Bring the world frame back into each object's space
  Collider* collider0 = GetCollider(0);
  if(collider0 != nullptr)
  {
    Quat obj0Rot = Math::ToQuaternion(collider0->GetWorldRotation());
    mReferenceAngle[0] = obj0Rot.Inverted() * rot;
  }

  Collider* collider1 = GetCollider(1);
  if(collider1 != nullptr)
  {
    Quat obj1Rot = Math::ToQuaternion(collider1->GetWorldRotation());
    mReferenceAngle[1] = obj1Rot.Inverted() * rot;
  }
}

Quat RevoluteJoint::GetWorldBasis()
{
  Collider* collider = GetCollider(mPrimaryFrameIndex);
  if(collider == nullptr)
    return Quat::cIdentity;

  UpdateColliderCachedTransforms();

  Quat localToWorld = Math::ToQuaternion(collider->GetWorldRotation());
  Quat localBasis = mReferenceAngle[mPrimaryFrameIndex];
  return localToWorld * localBasis;
}

void RevoluteJoint::SetWorldBasis(QuatParam basis)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "LocalBasisA", GetLocalBasisA());
    OperationQueue::RegisterSideEffect(this, "LocalBasisB", GetLocalBasisB());
  }

  UpdateColliderCachedTransforms();

  Collider* collider0 = GetCollider(0);
  Collider* collider1 = GetCollider(1);

  if(collider0 != nullptr)
  {
    QuatParam localToWorld = Math::ToQuaternion(collider0->GetWorldRotation());
    mReferenceAngle[0] = localToWorld.Inverted() * basis;
  }
  if(collider1 != nullptr)
  {
    QuatParam localToWorld = Math::ToQuaternion(collider1->GetWorldRotation());
    mReferenceAngle[1] = localToWorld.Inverted() * basis;
  }
}

JointFrameOfReference::Enum RevoluteJoint::GetFrameOfReference() const
{
  if(mPrimaryFrameIndex == 0)
    return JointFrameOfReference::ObjectA;
  return JointFrameOfReference::ObjectB;
}

void RevoluteJoint::SetFrameOfReference(JointFrameOfReference::Enum objectFrame)
{
  if(objectFrame == JointFrameOfReference::ObjectA)
    mPrimaryFrameIndex = 0;
  else
    mPrimaryFrameIndex = 1;
}

Quat RevoluteJoint::BuildFrameFromAxis(QuatParam oldWorldFrame, Vec3Param axis)
{
  Mat3 frame = Math::ToMatrix3(oldWorldFrame);
  // Set the hinge axis
  frame.SetBasis(2, axis);
  // Build up the y frame
  Vec3 newYFrame = Math::Cross(axis, frame.GetBasis(0));
  Vec3 newXFrame;
  // If the x axis and the new hinge were almost the same, we'll
  // build an invalid matrix, if so switch to building the y axis
  if(newYFrame.LengthSq() < real(.01))
  {
    newXFrame = Math::Cross(frame.GetBasis(1), axis);
    newYFrame = Math::Cross(axis, newXFrame);
  }
  else
    newXFrame = Math::Cross(newYFrame, axis);

  // Set both frames so that we preserve the passed in axis
  frame.SetBasis(0, newXFrame.Normalized());
  frame.SetBasis(1, newYFrame.Normalized());
  frame.Orthonormalize();
  return Math::ToQuaternion(frame).Normalized();
}

void RevoluteJoint::SetBodyAxisInternal(uint objIndex, Vec3Param axis)
{
  QuatRef refAngle = mReferenceAngle[objIndex];

  refAngle = BuildFrameFromAxis(refAngle, axis);
}

}//namespace Physics

}//namespace Zero
