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

void ConstraintMolecule::SetLimit(real min, real max)
{
  mMinImpulse = min;
  mMaxImpulse = max;
}

MoleculeWalker::MoleculeWalker()
{
  mStride = 0;
  mRawBuffer = nullptr;
}

MoleculeWalker::MoleculeWalker(void* start, uint totalStride, uint offset)
{
  mRawBuffer = reinterpret_cast<byte*>(start) + offset;
  mStride = totalStride;
}

void MoleculeWalker::operator++()
{
  mRawBuffer += mStride;
}

void MoleculeWalker::operator+=(uint i)
{
  mRawBuffer += mStride * i;
}

ConstraintMolecule& MoleculeWalker::operator*()
{
  return *mMolecules;
} 

ConstraintMolecule& MoleculeWalker::operator[](uint i)
{
  return *(ConstraintMolecule*)(mRawBuffer + i * mStride);
}

void MoleculeData::SetUp(AnchorAtom* anchor, const AngleAtom* angle, const Joint* joint)
{
  SetUp(anchor, angle, joint->GetCollider(0), joint->GetCollider(1));
}

void MoleculeData::SetUp(AnchorAtom* anchor, const AngleAtom* angle, AxisAtom* axis, const Joint* joint)
{
  Collider* collider0 = joint->GetCollider(0);
  Collider* collider1 = joint->GetCollider(1);
  //compute the world values if the local ones were passed in
  if(anchor)
    mAnchors = WorldAnchorAtom(*anchor, collider0, collider1);
  if(axis)
  {
    mAxes = WorldAxisAtom(*axis, collider0, collider1);

    Vec3 axes[3];
    axes[2] = axis->mBodyAxes[0];
    Math::GenerateOrthonormalBasis(axes[2], axes + 0, axes + 1);
    mWorldBases[0].SetBasis(0, axes[0]);
    mWorldBases[0].SetBasis(1, axes[1]);
    mWorldBases[0].SetBasis(2, axes[2]);

    axes[2] = axis->mBodyAxes[1];
    Math::GenerateOrthonormalBasis(axes[2], axes + 0, axes + 1);
    mWorldBases[1].SetBasis(0, axes[0]);
    mWorldBases[1].SetBasis(1, axes[1]);
    mWorldBases[1].SetBasis(2, axes[2]);

    mWorldBases[0] = collider0->GetWorldRotation() * mWorldBases[0];
    mWorldBases[1] = collider1->GetWorldRotation() * mWorldBases[1];
  }
  else
  {
    mAxes[0].ZeroOut();
    mAxes[1].ZeroOut();
  }
}

void MoleculeData::SetUp(AnchorAtom* anchor, const AngleAtom* angle, Contact* contact)
{
  SetUp(anchor, angle, contact->GetCollider(0), contact->GetCollider(1));
}

void MoleculeData::SetUp(AnchorAtom* anchor, const AngleAtom* angle, Collider* collider1, Collider* collider2)
{
  //compute the world values if the local ones were passed in
  if(anchor)
    mAnchors = WorldAnchorAtom(*anchor, collider1, collider2);
  if(angle)
  {
    mRefAngle = WorldAngleAtom(*angle, collider1, collider2);
    mWorldBases[0] = Math::ToMatrix3(mRefAngle.mWorldAngles[0]);
    mWorldBases[1] = Math::ToMatrix3(mRefAngle.mWorldAngles[1]);
  }
}

void MoleculeData::SetLinearBasis(Vec3Param axis)
{
  //generate an orthonormal basis where the passed in axis is axis 2
  LinearAxes[2] = axis;
  Math::GenerateOrthonormalBasis(LinearAxes[2], LinearAxes + 0, LinearAxes + 1);
}

void MoleculeData::SetAngularBasis(Vec3Param axis)
{
  //generate an orthonormal basis where the passed in axis is axis 2
  AngularAxes[2] = axis;
  Math::GenerateOrthonormalBasis(AngularAxes[2], AngularAxes + 0, AngularAxes + 1);
}

void MoleculeData::SetAngularBases(Mat3Param basis)
{
  AngularAxes[0] = basis.GetBasis(0);
  AngularAxes[1] = basis.GetBasis(1);
  AngularAxes[2] = basis.GetBasis(2);
}

void MoleculeData::SetLinearIdentity()
{
  LinearAxes[0] = Vec3::cXAxis;
  LinearAxes[1] = Vec3::cYAxis;
  LinearAxes[2] = Vec3::cZAxis;
}

void MoleculeData::SetAngularIdentity()
{
  AngularAxes[0] = Vec3::cXAxis;
  AngularAxes[1] = Vec3::cYAxis;
  AngularAxes[2] = Vec3::cZAxis;
}

void MoleculeData::Set2dBases(Joint* joint)
{
  Collider* collider0 = joint->GetCollider(0);
  Collider* collider1 = joint->GetCollider(1);

  Mat3 rot0 = collider0->GetWorldRotation();
  Mat3 rot1 = collider1->GetWorldRotation();

  Vec3 localAxis0 = Math::TransposedTransform(rot0, Vec3::cZAxis);
  Vec3 localAxis1 = Math::TransposedTransform(rot1, Vec3::cZAxis);

  Vec3 axes[3];
  axes[0] = localAxis0;
  Math::GenerateOrthonormalBasis(axes[0], axes + 1, axes + 2);
  mWorldBases[0].SetBasis(0, axes[0]);
  mWorldBases[0].SetBasis(1, axes[1]);
  mWorldBases[0].SetBasis(2, axes[2]);

  axes[2] = localAxis1;
  Math::GenerateOrthonormalBasis(axes[0], axes + 1, axes + 2);
  mWorldBases[1].SetBasis(0, axes[0]);
  mWorldBases[1].SetBasis(1, axes[1]);
  mWorldBases[1].SetBasis(2, axes[2]);

  mWorldBases[0] = rot0 * mWorldBases[0];
  mWorldBases[1] = rot1 * mWorldBases[1];
}

}//namespace Physics

}//namespace Zero
