///////////////////////////////////////////////////////////////////////////////
///
/// 
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;

namespace Physics
{

///Defines two local anchor points on two objects. Use in any joint that
///wants to lock two points together.
struct AnchorAtom
{
  AnchorAtom();

  Vec3Ref operator[](uint index);
  Vec3Param operator[](uint index) const;

  Vec3 mBodyR[2];
};

///Defines the AnchorAtom but after the points have been translated into world space.
struct WorldAnchorAtom
{
  WorldAnchorAtom() {};
  WorldAnchorAtom(const AnchorAtom& anchor, Joint* joint);
  WorldAnchorAtom(const AnchorAtom& anchor, Collider* obj1, Collider* obj2);

  void SetUp(const AnchorAtom& anchor, Collider* obj1, Collider* obj2);

  inline Vec3& operator[](uint index) {return mWorldR[index]; };
  inline const Vec3& operator[](uint index) const  {return mWorldR[index]; };

  Vec3 GetPointDifference() const;

  Vec3 mWorldR[2];
  Vec3 mWorldPoints[2];
};

///Defines two local axes on two objects. Use in any joint that wants
///to define a free axis.
struct AxisAtom
{
  AxisAtom() { mBodyAxes[0] = mBodyAxes[1] = Vec3::cYAxis; };
  inline Vec3& operator[](uint index) {return mBodyAxes[index]; };
  inline const Vec3& operator[](uint index) const  {return mBodyAxes[index]; };

  Vec3 mBodyAxes[2];
};

///Defines the AxisAtom but after the axes have been translated into world space.
struct WorldAxisAtom
{
  WorldAxisAtom() {};
  WorldAxisAtom(const AxisAtom& axes, Collider* obj1, Collider* obj2);

  inline Vec3& operator[](uint index) {return mWorldAxes[index]; };
  inline const Vec3& operator[](uint index) const  {return mWorldAxes[index]; };

  Vec3 mWorldAxes[2];
};

///Defines two local rotations on two objects. Use in any joint that wants
///to lock a rotation of one object to another.
struct AngleAtom
{
  AngleAtom() { mLocalAngles[0] = Quat::cIdentity; mLocalAngles[1] = Quat::cIdentity; };

  inline Quat& operator[](uint index) {return mLocalAngles[index]; };
  inline const Quat& operator[](uint index) const  {return mLocalAngles[index]; };

  Quat GetReferenceAngle() const;

  Quat mLocalAngles[2];
};

///Defines the AngleAtom but after the angles have been translated into world space.
struct WorldAngleAtom
{
  WorldAngleAtom();
  WorldAngleAtom(const AngleAtom& refAngle, Collider* obj1, Collider* obj2);

  inline Quat& operator[](uint index) {return mWorldAngles[index]; };
  inline const Quat& operator[](uint index) const  {return mWorldAngles[index]; };

  Quat mWorldAngles[2];
  Quat mWorldReferenceAngle;
  // Used to properly compute the error angles
  // (using a quaternion has a 2-1 mapping issue).
  Vec3 mEulerAngles;
};

///The bare minimum to define a constraint.
struct ConstraintAtom
{
  ConstraintAtom();

  real mImpulse;

  union
  {
    real mError;
    real mConstraintValue;
  };
};

///Defines a min/max limit for a impulse.
struct ImpulseLimitAtom
{
  ImpulseLimitAtom() {};
  ImpulseLimitAtom(real maxImpulse);
  ImpulseLimitAtom(real maxImpulse, real minImpulse);

  real mMinImpulse;
  real mMaxImpulse;
};

///Defines the elements used to turn a constraint atom into a soft constraint.
struct SpringAtom
{
  real mFrequencyHz;
  real mDampingRatio;
};

}//namespace Physics

}//namespace Zero
