///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct Joint;

namespace Physics
{

///Run time solving data. Used to store runtime only data in a scratch
///space for efficient solving.
struct ConstraintMolecule
{
  void SetLimit(real min, real max);

  Jacobian mJacobian;

  real mMass;
  real mGamma;
  real mBias;
  real mMinImpulse;
  real mMaxImpulse;
  real mImpulse;
  real mError;

  uint mAtomIndex;
};

///Used to give constraints an array of molecules where the underlying structure
///may be larger than a fragment. This is so the same code path can be taken
///for anything that wants to store an array of ConstraintMolecules plus something
///else.
struct MoleculeWalker
{
  MoleculeWalker();
  MoleculeWalker(void* start, uint totalStride, uint offset);

  void operator++();
  void operator+=(uint i);
  ConstraintMolecule& operator*();

  ConstraintMolecule& operator[](uint i);

  union
  {
    byte* mRawBuffer;
    ConstraintMolecule* mMolecules;
  };

  uint mStride;
};

///Stores temporary world data to compute molecules.
struct MoleculeData
{
  MoleculeData() {};
  
  void SetUp(AnchorAtom* anchor, const AngleAtom* angle, const Joint* joint);
  void SetUp(AnchorAtom* anchor, const AngleAtom* angle, AxisAtom* axis, const Joint* joint);
  void SetUp(AnchorAtom* anchor, const AngleAtom* angle, Contact* contact);

  void SetUp(AnchorAtom* anchor, const AngleAtom* angle, Collider* collider1, Collider* collider2);
  void SetLinearBasis(Vec3Param axis);
  void SetAngularBasis(Vec3Param axis);
  void SetAngularBases(Mat3Param basis);
  void SetLinearIdentity();
  void SetAngularIdentity();
  void Set2dBases(Joint* joint);

  WorldAnchorAtom mAnchors;
  WorldAngleAtom mRefAngle;
  WorldAxisAtom mAxes;

  Vec3 LinearAxes[3];
  Vec3 AngularAxes[3];

  Mat3 mWorldBases[2];
};

}//namespace Physics

}//namespace Zero
