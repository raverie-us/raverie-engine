///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(WindEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  
  ZilchBindMemberProperty(mLocalSpaceDirection)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(WindSpeed)->ZeroSerialize(real(10));
  ZilchBindGetterSetterProperty(WindDirection)->ZeroSerialize(Vec3(1, 0, 0));
  ZilchBindGetterProperty(WorldWindDirection);
}

WindEffect::WindEffect()
{
  mEffectType = PhysicsEffectType::Wind;
}

void WindEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void WindEffect::DebugDraw()
{
  if(!GetDebugDrawEffect())
    return;

  PreCalculate(0);
  Vec3 pos = TransformLocalPointToWorld(Vec3::cZero);
  Vec3 dir = mWorldWindDirection * mWindSpeed;
  gDebugDraw->Add(Debug::Line(pos, pos + dir).HeadSize(0.1f));
}

void WindEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  mWorldWindDirection = GetWorldWindDirection();
}

void WindEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  if(obj->GetKinematic())
    return;

  // Compute a wind force for each collider on this rigid body
  RigidBody::CompositeColliderList::range colliders = obj->GetColliders();
  for(; !colliders.Empty(); colliders.PopFront())
  {
    Collider& collider = colliders.Front();
    ApplyEffect(collider, obj, dt);
  }
}

void WindEffect::ApplyEffect(SpringSystem* obj, real dt)
{
  if(!GetActive())
    return;

  // Wind force is based upon v^2
  Vec3 windDir = mWorldWindDirection;
  real windSpeed = mWorldWindDirection.AttemptNormalize();
  windSpeed *= mWindSpeed;
  real windSpeedSq = windSpeed * windSpeed;

  // Apply the wind force to each face in the spring system
  for(size_t i = 0; i < obj->mFaces.Size(); ++i)
  {
    SpringSystem::Face& face = obj->mFaces[i];
    Vec3 p0 = obj->mPointMasses[face.mIndex0].mPosition;
    Vec3 p1 = obj->mPointMasses[face.mIndex1].mPosition;
    Vec3 p2 = obj->mPointMasses[face.mIndex2].mPosition;

    // The cross product gives a vector who's length is twice the area of the triangle
    Vec3 normal = Math::Cross(p0 - p1, p2 - p1);
    real area = normal.AttemptNormalize() * real(0.5f);

    // The wind will be double sided since the dot product will correct
    // our normal direction to be in the same general direction as the wind
    real strength = Math::Dot(normal, windDir) * windSpeedSq;
    Vec3 force = area * strength * normal;

    obj->mPointMasses[face.mIndex0].mForce += force;
    obj->mPointMasses[face.mIndex1].mForce += force;
    obj->mPointMasses[face.mIndex2].mForce += force;
  }
}

void WindEffect::ApplyEffect(Collider& collider, RigidBody* obj, real dt)
{
  Vec3 scale = collider.GetWorldScale();
  Mat3 rot = collider.GetWorldRotation();

  // Get the bases for the object
  Vec3 basisX = rot.BasisX();
  Vec3 basisY = rot.BasisY();
  Vec3 basisZ = rot.BasisZ();
  // Approximate the surface area along each basis (bounding box approximation)
  real forceX = Math::Abs(Math::Dot(mWorldWindDirection, basisX)) * scale.y * scale.z;
  real forceY = Math::Abs(Math::Dot(mWorldWindDirection, basisY)) * scale.x * scale.z;
  real forceZ = Math::Abs(Math::Dot(mWorldWindDirection, basisZ)) * scale.x * scale.y;

  // Make sure each basis vector is in the same hemisphere as the wind direction
  if(Math::Dot(mWorldWindDirection, basisX) < real(0))
    basisX *= real(-1);
  if(Math::Dot(mWorldWindDirection, basisY) < real(0))
    basisY *= real(-1);
  if(Math::Dot(mWorldWindDirection, basisZ) < real(0))
    basisZ *= real(-1);

  Vec3 force = mWorldWindDirection;
  force.AttemptNormalize();
  force *= mWindSpeed;
  // Apply a force to the center of each face in the direction of
  // the wind with the computed surface area ratio
  obj->ApplyForceAtOffsetVector(force * forceX, basisX);
  obj->ApplyForceAtOffsetVector(force * forceY, basisY);
  obj->ApplyForceAtOffsetVector(force * forceZ, basisZ);
}

real WindEffect::GetWindSpeed() const
{
  return mWindSpeed;
}

void WindEffect::SetWindSpeed(real speed)
{
  mWindSpeed = speed;

  CheckWakeUp();
}

Vec3 WindEffect::GetWindDirection() const
{
  return mWindDirection;
}

void WindEffect::SetWindDirection(Vec3Param direction)
{
  mWindDirection = direction;

  CheckWakeUp();
}

Vec3 WindEffect::GetWorldWindDirection() const
{
  Vec3 worldWindDirection = mWindDirection;
  if(mLocalSpaceDirection)
    worldWindDirection = TransformLocalDirectionToWorld(worldWindDirection);
  // Always re-normalize the world axis
  return worldWindDirection.AttemptNormalized();
}

Vec3 WindEffect::GetWindVelocity()
{
  return mWorldWindDirection * mWindSpeed;
}

}//namespace Zero
