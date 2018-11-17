///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(TimeOfImpactDebug, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultConstructor);

  ZeroBindDependency(Collider);
  ZeroBindDependency(RigidBody);

  ZilchBindGetterSetterProperty(OtherObject);
  ZilchBindFieldProperty(mSimulatedDt);

  ZilchBindFieldProperty(mSteps);

  ZilchBindMethodProperty(Step);
}

TimeOfImpactDebug::TimeOfImpactDebug()
  : mSimulatedDt(0.0167)
  , mSteps(0)
{
}

Vec3 TimeOfImpactDebug::PointAtTime(Vec3Param p, Vec3Param v, Vec3Param r, Vec3Param w, real a, real t)
{
  Quat q = ToQuaternion(w, a * t);
  return p + v * t + Math::Multiply(q, r);
}

void TimeOfImpactDebug::DebugDraw()
{
  if (!mOtherObject.IsValid() || !mOtherObject.has(Collider))
    return;

  real impact = mSimulatedDt;
  Physics::Manifold manifold;
  TimeOfImpactData data(GetOwner()->has(Collider), mOtherObject.has(Collider), mSimulatedDt);
  data.Steps = mSteps;
  TimeOfImpact(&data);

  //find the first impact time
  for(uint i = 0; i < data.ImpactTimes.Size(); ++i)
  {
    if(data.ImpactTimes[i] < impact)
      impact = data.ImpactTimes[i];
  }

  DrawColliderAtTime(GetOwner(), impact);
  DrawColliderAtTime(mOtherObject, impact);
  //if (GetOwner()->has(Collider)->mType == Collider::cBox)
  //  DrawBoxAtTime(GetOwner(), impact);
  //
  //if (mOtherObject.has(Collider)->mType == Collider::cBox && mOtherObject.has(RigidBody))
  //  DrawBoxAtTime(mOtherObject, impact);
}

void TimeOfImpactDebug::DrawBoxAtTime(Cog* cog, real dt)
{
  Transform* transform = cog->has(Transform);
  RigidBody* body = cog->has(RigidBody);
  Vec3 pos = transform->GetTranslation();
  Quat rot = transform->GetRotation();
  Vec3 vel = body->GetVelocity();
  Vec3 angVel = body->GetAngularVelocity();
  angVel = Math::Multiply(rot, angVel);
  real rotAngle = angVel.Length();
  if (rotAngle != 0.0f)
    angVel.Normalize();

  Vec3 p[8];

  p[0] = transform->TransformPoint(Vec3( 0.5f,  0.5f,  0.5f)) - pos;
  p[1] = transform->TransformPoint(Vec3( 0.5f,  0.5f, -0.5f)) - pos;
  p[2] = transform->TransformPoint(Vec3( 0.5f, -0.5f,  0.5f)) - pos;
  p[3] = transform->TransformPoint(Vec3( 0.5f, -0.5f, -0.5f)) - pos;
  p[4] = transform->TransformPoint(Vec3(-0.5f,  0.5f,  0.5f)) - pos;
  p[5] = transform->TransformPoint(Vec3(-0.5f,  0.5f, -0.5f)) - pos;
  p[6] = transform->TransformPoint(Vec3(-0.5f, -0.5f,  0.5f)) - pos;
  p[7] = transform->TransformPoint(Vec3(-0.5f, -0.5f, -0.5f)) - pos;

  for (uint i = 0; i < 8; ++i)
    p[i] = PointAtTime(pos, vel, p[i], angVel, rotAngle, dt);

  //    5 --- 1
  //  / |   / |
  // 4 --- 0  |
  // |  |  |  |
  // |  7 -|- 3
  // |/    |/
  // 6 --- 2

  gDebugDraw->Add(Debug::Triangle(p[6], p[2], p[0]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[6], p[0], p[4]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[2], p[3], p[1]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[2], p[1], p[0]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[3], p[7], p[5]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[3], p[5], p[1]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[7], p[6], p[4]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[7], p[4], p[5]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[4], p[0], p[1]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[4], p[1], p[5]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[7], p[3], p[2]).Color(Color::Red).Alpha(50).Border(true));
  gDebugDraw->Add(Debug::Triangle(p[7], p[2], p[6]).Color(Color::Red).Alpha(50).Border(true));
}

void TimeOfImpactDebug::DrawColliderAtTime(Cog* cog, real dt)
{
  Collider* collider = cog->has(Collider);
  RigidBody* body = cog->has(RigidBody);

  if(collider == nullptr)
    return;

  Vec3 vel = Vec3::cZero;
  Vec3 angularVel = Vec3::cZero;
  if(body != nullptr)
  {
    vel = body->mVelocity;
    angularVel = body->GetAngularVelocity();
  }

  real rotationAngle = angularVel.AttemptNormalize();


  WorldTransformation* transform = collider->GetWorldTransform();
  Vec3 oldPosition = transform->GetWorldTranslation();
  Quat oldRotation = Math::ToQuaternion(transform->GetWorldRotation());
  //integrate the position and orientation forward in time
  transform->SetTranslation(oldPosition + vel * dt);
  Quat newRotation = Math::ToQuaternion(angularVel, rotationAngle * dt) * oldRotation;
  transform->SetRotation(Math::ToMatrix3(newRotation));
  
  collider->DebugDraw();

  //reset the collider's position back to where it was
  transform->SetTranslation(oldPosition);
  transform->SetRotation(Math::ToMatrix3(oldRotation));
}

Cog * TimeOfImpactDebug::GetOtherObject()
{
  return mOtherObject;
}

void TimeOfImpactDebug::SetOtherObject(Cog *cog)
{
  if (cog == nullptr)
    mOtherObject = CogId();
  else
    mOtherObject = cog->GetId();
}

} // namespace Zero
