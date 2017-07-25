///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2012-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------BuoyancyEffect
ZilchDefineType(BuoyancyEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindDependency(Region);
  
  ZilchBindFieldProperty(mDensity)->ZeroSerialize(real(1));
  ZilchBindFieldProperty(mGravity)->ZeroSerialize(Vec3(0, real(-9.81), 0));
  ZilchBindGetterSetterProperty(Detail)->ZeroSerialize(5u);
  ZilchBindFieldProperty(mDebugDrawRuntime)->ZeroSerialize(false);
}

BuoyancyEffect::BuoyancyEffect()
{
  mEffectType = PhysicsEffectType::Buoyancy;
  mCollider = nullptr;
  mDetail = 5;
}

void BuoyancyEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void BuoyancyEffect::Initialize(CogInitializer& initializer)
{
  PhysicsEffect::Initialize(initializer);

  Cog* owner = GetOwner();
  mCollider = owner->has(Collider);
}

void BuoyancyEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  // Buoyancy applies a force that is dependent on the volume of the object.
  // To do this we need to iterate over all colliders in the body.
  // Note: This is currently incorrect if multiple colliders of a rigid body overlap.
  RigidBody::CompositeColliderRange colliders = obj->GetColliders();
  for(; !colliders.Empty(); colliders.PopFront())
  {                     
    Collider* collider = &colliders.Front();

    // Check to make sure the collider is inside the region.
    // This will only be true for hierarchies of objects
    if(!OverlapsRegion(collider))
      continue;

    // If the collision group says to not check collision, then don't
    if(mCollider->mCollisionGroupInstance->SkipDetection(*(collider->mCollisionGroupInstance)))
      continue;

    // We still want to check to see if resolution should be
    // skipped for not applying region effects
    if(mCollider->mCollisionGroupInstance->SkipResolution((*collider->mCollisionGroupInstance)))
      continue;

    // Determine how much of the collider overlaps this region
    // (and the center of the overlapped region)
    Vec3 centerInside;
    float percentInside = ComputeOverlapPercent(collider, centerInside);

    // If there is an overlap then apply the force at the center of the overlapped region
    if(percentInside != 0)
    {
      // Get the approximate volume of intersecting region
      real volumeInside = collider->ComputeWorldVolumeInternal() * percentInside;
      obj->ApplyForceAtPoint(-mGravity * volumeInside * mDensity, centerInside);
    }    
  }
}

uint BuoyancyEffect::GetDetail()
{
  return mDetail;
}

void BuoyancyEffect::SetDetail(uint detail)
{
  if(detail == 0)
  {
    DoNotifyWarning("Invalid detail", "Detail must be greater than zero");
    return;
  }

  mDetail = detail;
}

float BuoyancyEffect::ComputeOverlapPercent(Collider* collider, Vec3& volumeCenter)
{
  // Get the world space Aabb of the object
  Aabb aabb = collider->mAabb;

  if(mDebugDrawRuntime)
    gDebugDraw->Add(Debug::Obb(aabb).Color(Color::White).OnTop(true));

  Vec3 halfExtents = aabb.GetHalfExtents();
  Vec3 min = aabb.GetCenter() - halfExtents;

  // To approximate the volume of overlap we place a grid of points in the test collider's aabb.
  // We can determine if any point is actually inside the collider to approximate the volume of the collider.
  // This point can then be tested against the region to approximate the overlap percentage.

  // Calculate the step for placing points in a grid
  Vec3 step = halfExtents * real(2);
  step /= real(mDetail);

  // Min step is used to draw the points
  real minStep = Math::Min(step.x, Math::Min(step.y, step.z));

  // We need to compute the percentage of points inside. To do this we have to keep track of
  // the number of points inside and outside (combined is the total). We also need to apply 
  // the force not at the center of mass, but the center of mass of the intersecting volume.
  Vec3 center = Vec3::cZero;
  size_t inside = 0;
  size_t outside = 0;

  // Get each point in the grid
  for(size_t z = 0; z < mDetail; ++z)
  {
    for(size_t y = 0; y < mDetail; ++y)
    {
      for(size_t x = 0; x < mDetail; ++x)
      {
        // Calculate the point to test
        Vec3 pos = min + Vec3(real(x), real(y), real(z)) * step;

        // If the point is not in the collider then ignore it
        // (we're approximating the collider by iterating over the aabb so this point is in the aabb but outside the collider)
        if(!PointInObject(collider, pos))
          continue;

        // If the point is also inside the region then we have a volume intersection
        if(PointInObject(mCollider, pos))
        {
          // Add to the averaged center position
          center += pos;
          ++inside;

          if(mDebugDrawRuntime)
            gDebugDraw->Add(Debug::Sphere(pos, minStep * real(0.15f)).Color(Color::Green).OnTop(true));
        }
        else
        {
          // Inside the object, but not inside the fluid region
          ++outside;

          if(mDebugDrawRuntime)
            gDebugDraw->Add(Debug::Sphere(pos, minStep * real(0.15f)).Color(Color::Red).OnTop(true));
        }
      }
    }
  }

  // If no points were inside then the percentage is always zero and we won't apply any force so just return now
  if(inside == 0)
    return 0;

  // Get the approximate percentage of the object inside the region
  size_t total = inside + outside;
  real percentInside = real(inside) / real(total);
  
  // Get the approximate center of mass of the part of the object inside the region
  volumeCenter = center / real(inside);

  // Draw the volume center
  if(mDebugDrawRuntime)
    gDebugDraw->Add(Debug::Sphere(volumeCenter, minStep * real(0.3f)).Color(Color::Blue).OnTop(true));

  return percentInside;
}

bool BuoyancyEffect::OverlapsRegion(Collider* object)
{
  return Physics::CollisionManager::TestIntersection(mCollider, object->mAabb);
}

bool BuoyancyEffect::PointInObject(Collider* object, Vec3Param point)
{
  return Physics::CollisionManager::TestIntersection(object, point);
}

}//namespace Zero
