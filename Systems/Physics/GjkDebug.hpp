///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class GjkDebug : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void InitEpa(void);
  void AddPoint(void);
  void StepEpa(void);
  void ToggleAnimation(void);

  GjkDebug(void);

  virtual void DebugDraw(void);

  Cog * GetOtherObject(void);
  void SetOtherObject(Cog *cog);

  // UI functions
  void ComputeCSO(void);

private:

  struct Face
  {
    Face(void) {}
    Face(Vec3 p0, Vec3 p1, Vec3 p2)
    {
      vertices[0] = p0;
      vertices[1] = p1;
      vertices[2] = p2;
    }
    Vec3 vertices[3];
  };

  Vec3 ComputeSupport(Vec3 supportVector);
  Vec3 RandomPoint(void);

  Array<Vec3> mSupports;
  Array<Face> mCSO;

  // Properties
  CogId mOtherObject;
  bool mShowCSO;
  float mSubdivisions;
  float mOpacityCSO;
  float mOpacitySimplex;

  Intersection::Epa mEpa;
  bool mAnimate;
  unsigned mMaxExpands;

  real mDt;
};

} // namespace Zero
