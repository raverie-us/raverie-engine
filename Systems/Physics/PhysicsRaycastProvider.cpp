///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String PhysicsRaycastProviderDisplayText(const BoundType* type, const byte* data)
{
  static String Name("Physics");
  return Name;
}

ZilchDefineType(PhysicsRaycastProvider, builder, type)
{
  ZeroBindDocumented();
  ZeroBindExpanded();

  ZilchBindFieldProperty(mDynamicColliders);
  ZilchBindFieldProperty(mSelectGhosts);
  ZilchBindFieldProperty(mStaticColliders);
  ZilchBindFieldProperty(mMultiSelectStatic);
  ZilchBindFieldProperty(mMultiSelectKinematic);
  
  type->ToStringFunction = PhysicsRaycastProviderDisplayText;
  type->Add(new StringNameDisplay("Physics"));
}

PhysicsRaycastProvider::PhysicsRaycastProvider()
{
  mDynamicColliders = true;
  mSelectGhosts = true;
  mStaticColliders = true;
  mMultiSelectStatic = false;
  mMultiSelectKinematic = false;
}

void PhysicsRaycastProvider::RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results)
{
  // If we don't select static or dynamic, there's nothing to do...
  if(!mDynamicColliders && !mStaticColliders)
    return;

  // Make sure there is a physics space
  PhysicsSpace* physicsSpace = castInfo.mTargetSpace->has(PhysicsSpace);
  if(physicsSpace == nullptr)
    return;

  // Set the filters for what we do select
  CastResults::Filter filter;
  filter.SetState(BaseCastFilterFlags::IgnoreGhost, !mSelectGhosts);
  filter.SetState(BaseCastFilterFlags::IgnoreDynamic, !mDynamicColliders);
  filter.SetState(BaseCastFilterFlags::IgnoreStatic, !mStaticColliders);

  // Form a physics cast result with the total result's capacity
  CastResults physicsResults(results.mCapacity, filter);

  // Cast the ray into physics
  physicsSpace->CastRay(ray, physicsResults);
  // And then add all of the results that we got
  CastResults::range range = physicsResults.All();
  for(; !range.Empty(); range.PopFront())
  {
    CastResult& item = range.Front();
    results.AddItem(item.GetObjectHit(), item.GetDistance(), item.GetWorldPosition(), item.GetNormal());
  }
}

void PhysicsRaycastProvider::FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results)
{
  // Make sure there is a physics space
  PhysicsSpace* physicsSpace = castInfo.mTargetSpace->has(PhysicsSpace);
  if(physicsSpace == nullptr)
    return;

  CastFilter filter;

  // Set up the filters for multi casting (make sure to get the flags that still matter such as ghost)
  if(!mSelectGhosts)
    filter.Set(BaseCastFilterFlags::IgnoreGhost);
  if(!mDynamicColliders)
    filter.Set(BaseCastFilterFlags::IgnoreDynamic);
  if(!mMultiSelectStatic)
    filter.Set(BaseCastFilterFlags::IgnoreStatic);
  if(!mMultiSelectKinematic)
    filter.Set(BaseCastFilterFlags::IgnoreKinematic);

  // Form a physics cast result with the total result's capacity
  CastResults physicsResults(results.mCapacity, filter);
  // Cast to get the results from physics
  physicsSpace->CastFrustum(frustum, physicsResults);
  // And then add all of the results that we got
  CastResults::range range = physicsResults.All();
  for(; !range.Empty(); range.PopFront())
  {
    CastResult& item = range.Front();
    results.AddItem(item.GetObjectHit(), item.GetDistance(), item.GetWorldPosition(), item.GetNormal());
  }
}

}//namespace Zero
