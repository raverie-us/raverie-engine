// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TimeOfImpactDebug : public Component
{
public:
  RaverieDeclareType(TimeOfImpactDebug, TypeCopyMode::ReferenceType);

  TimeOfImpactDebug();

  virtual void DebugDraw();

  Cog* GetOtherObject();
  void SetOtherObject(Cog* cog);

  void Step()
  {
    ++mSteps;
  }

private:
  Vec3 PointAtTime(Vec3Param p, Vec3Param v, Vec3Param r, Vec3Param w, real a, real t);
  void DrawBoxAtTime(Cog* cog, real dt);
  void DrawColliderAtTime(Cog* cog, real dt);

  // Properties
  CogId mOtherObject;
  real mSimulatedDt;

  int mSteps;
};

} // namespace Raverie
