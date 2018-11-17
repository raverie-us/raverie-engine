///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ProxyContactPoint
ZilchDefineType(ContactPoint, builder, type)
{
  // These should be changed later, but this is the closest behavior to how proxy
  // points worked previously. These are unsafe to store. Also I'm using HeapManager
  // instead of PointerManager since the control point is returned by value.
  type->HandleManager = ZilchManagerId(HeapManager);
  ZilchBindDefaultCopyDestructor();

  ZeroBindDocumented();
  ZilchBindGetterProperty(LocalPoint);
  ZilchBindGetterProperty(OtherLocalPoint);
  ZilchBindGetterProperty(WorldPoint);
  ZilchBindGetterProperty(WorldNormalTowardsOther);
  ZilchBindGetterProperty(NormalImpulse);
  ZilchBindGetterProperty(FrictionImpulse);
  ZilchBindGetterProperty(ComplexImpulse);
  ZilchBindGetterProperty(Penetration);
  ZilchBindGetterProperty(RelativeVelocity);
  ZeroBindTag(Tags::Physics);
}

ContactPoint::ContactPoint()
{
  mManifold = nullptr;
  mManifoldPoint = nullptr;
  mObjectIndex = 0;
}

void ContactPoint::Set(const Physics::Manifold* manifold, const Physics::ManifoldPoint* manifoldPoint, uint objectIndex)
{
  mManifold = manifold;
  mManifoldPoint = manifoldPoint;
  mObjectIndex = objectIndex;
}

Vec3 ContactPoint::GetLocalPoint()
{
  return mManifoldPoint->BodyPoints[mObjectIndex];
}

Vec3 ContactPoint::GetOtherLocalPoint()
{
  return mManifoldPoint->BodyPoints[(mObjectIndex + 1) % 2];
}

Vec3 ContactPoint::GetWorldPoint()
{
  // The user shouldn't know that the world points are different between
  // obj1 or obj2, so just return the same point no matter what
  return mManifoldPoint->WorldPoints[0];
}

Vec3 ContactPoint::GetWorldNormalTowardsOther()
{
  // The normal points from A to B, so it's already in the
  // correct direction if we're editing object 0
  if(mObjectIndex == 0)
    return mManifoldPoint->Normal;
  return -mManifoldPoint->Normal;
}

real ContactPoint::GetNormalImpulse()
{
  // Return the normal impulse value, see GetComplexImpulse for
  // an explanation of why there are no ifs or minus signs
  return mManifoldPoint->AccumulatedImpulse[0];
}

real ContactPoint::GetFrictionImpulse()
{
  // See GetComplexImpulse for an explanation of why there are no ifs or minus signs

  // Return the approximate total impulse of friction, this is just the length of f1 + f2,
  // and since they're perpendicular this is just the sqrt of the squares
  real f1 = mManifoldPoint->AccumulatedImpulse[1];
  real f2 = mManifoldPoint->AccumulatedImpulse[2];
  return Math::Sqrt(f1 * f1 + f2 * f2);
}

Vec3 ContactPoint::GetComplexImpulse()
{
  // The impulse is object 1 applies is j*n, so just return j.
  // Also, we don't have to negate the impulse for object 1 because
  // this value is to be used with the normal, which will be flipped. Aka the user
  // should be able to get this and multiply with GetMyNormalPointingTowardsOther
  // and push the objects faster away with no extra logic.
  return mManifoldPoint->AccumulatedImpulse;
}

real ContactPoint::GetPenetration()
{
  // Penetration is always positive (it's in the direction of the normal, which we negate)
  return mManifoldPoint->Penetration;
}

real ContactPoint::GetRelativeVelocity()
{
  Vec3 worldPointA = mManifoldPoint->WorldPoints[mObjectIndex];
  Collider* objA = mManifold->Objects[mObjectIndex];
  Collider* objB = mManifold->Objects[(mObjectIndex + 1) % 2];
  Vec3 pointVelocity = objA->ComputePointVelocityInternal(worldPointA) - objB->ComputePointVelocityInternal(worldPointA);
  return Math::Dot(GetWorldNormalTowardsOther(), pointVelocity);
}

//-------------------------------------------------------------------ProxyContactPointRange

ContactPointRange::ContactPointRange()
{
  mPointIndex = 0;
  mManifold = nullptr;
}

void ContactPointRange::Set(const Physics::Manifold* manifold, uint objectIndex)
{
  mManifold = manifold;
  mPoint.mObjectIndex = objectIndex;
}

ContactPoint ContactPointRange::Front()
{
  mPoint.Set(mManifold, &mManifold->Contacts[mPointIndex], mPoint.mObjectIndex);
  return mPoint;
}

void ContactPointRange::PopFront()
{
  ++mPointIndex;
}

bool ContactPointRange::Empty()
{
  return mPointIndex >= mManifold->ContactCount;
}

}//names Zero
