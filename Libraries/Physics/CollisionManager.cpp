///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

InternalImplementation* CollisionManager::mInternals = nullptr;

//Used so anyone including the collision manager doesn't
//have to see the templates and possibly generate all the code as well
struct InternalImplementation
{
  //simple helper to wrap setting all of the cast shapes
  template <typename ColliderType>
  void OverrideCastsWithComplex(uint colliderType)
  {
    mRayCastLookup.OverrideLookup(ComplexCastVsShape<Ray, ColliderType>, colliderType);
    mSegmentCastLookup.OverrideLookup(ComplexCastVsShape<Segment, ColliderType>, colliderType);
    mAabbCastLookup.OverrideLookup(ComplexCastVsShape<Aabb, ColliderType>, colliderType);
    mSphereCastLookup.OverrideLookup(ComplexCastVsShape<Sphere, ColliderType>, colliderType);
    mFrustumCastLookup.OverrideLookup(ComplexCastVsShape<Frustum, ColliderType>, colliderType);
  }

  //testing a sub object of a collider (was used in complex colliders, not used at the moment)
  Zero::ShapeArrayLookup<Sphere> mSphereLookups;
  Zero::ShapeArrayLookup<Aabb> mAabbLookups;
  Zero::ShapeArrayLookup<Triangle> mTriangleLookups;

  //cast functions
  Zero::CastArrayLookup<Ray> mRayCastLookup;
  Zero::CastArrayLookup<Segment> mSegmentCastLookup;
  Zero::CastArrayLookup<Aabb> mAabbCastLookup;
  Zero::CastArrayLookup<Sphere> mSphereCastLookup;
  Zero::CastArrayLookup<Frustum> mFrustumCastLookup;

  //specific shape overlap tests
  Zero::OverlapArrayLookup<Vec3> mPointOverlapLookups;
  Zero::OverlapArrayLookup<Aabb> mAabbOverlapLookups;

  //table lookups (two colliders) for boolean and manifold versions
  Zero::CollisionTableLookup mCollisionTable;
  Zero::OverlapTableLookup mOverlapTable;
};

CollisionManager::CollisionManager()
{
  mInternals = new InternalImplementation();

  //frustum vs Cylinder, Ellipsoid, Capsule and ConvexMesh is not implemented.
  //Replace those with an aabb check as frustum casting is important to have.
  //Also default any other complex shape to frustum aabb so it can be group selected.
  for(uint i = Collider::cCylinder; i < (uint)Collider::cSize; ++i)
  {
    mInternals->mFrustumCastLookup.OverrideLookup(CastVsShape<Frustum, Aabb>, i);
  }

  //replace the default of all complex shapes to aabb
  //(makes sure everything is safe although we replace some later)
  for(uint i = Collider::cMesh; i < (uint)Collider::cSize; ++i)
  {
    mInternals->mRayCastLookup.OverrideLookup(CastVsShape<Ray, Aabb>, i);
    mInternals->mSegmentCastLookup.OverrideLookup(CastVsShape<Segment, Aabb>, i);
    mInternals->mAabbCastLookup.OverrideLookup(CastVsShape<Aabb, Aabb>, i);
    mInternals->mSphereCastLookup.OverrideLookup(CastVsShape<Sphere, Aabb>, i);
  }

  //replace all of the cast functions for the complex shapes
  mInternals->OverrideCastsWithComplex<ConvexMeshCollider>(Collider::cConvexMesh);
  mInternals->OverrideCastsWithComplex<MultiConvexMeshCollider>(Collider::cMultiConvexMesh);
  mInternals->OverrideCastsWithComplex<MeshCollider>(Collider::cMesh);
  mInternals->OverrideCastsWithComplex<HeightMapCollider>(Collider::cHeightMap);
  //special override so it goes through the mesh's midphase
  mInternals->mRayCastLookup.OverrideLookup(SpecialComplexCastVsShape<Ray, MultiConvexMeshCollider>, Collider::cMultiConvexMesh);
  mInternals->mRayCastLookup.OverrideLookup(SpecialComplexCastVsShape<Ray, MeshCollider>, Collider::cMesh);
  mInternals->mRayCastLookup.OverrideLookup(SpecialComplexCastVsShape<Ray, HeightMapCollider>, Collider::cHeightMap);
  //override the complex collision tests for the world mesh and multi collider
  mInternals->mCollisionTable.OverrideComplexDefaults<MultiConvexMeshCollider>(Collider::cMultiConvexMesh);
  mInternals->mCollisionTable.OverrideComplexDefaults<MeshCollider>(Collider::cMesh);
  mInternals->mCollisionTable.OverrideComplexDefaults<HeightMapCollider>(Collider::cHeightMap);
  //override complex vs complex defaults
  mInternals->mCollisionTable.OverrideComplexComplexDefaults<MultiConvexMeshCollider>(Collider::cMultiConvexMesh);
}

CollisionManager::~CollisionManager()
{
  SafeDelete(mInternals);
}

bool CollisionManager::TestCollision(ColliderPair& pair, ManifoldArray& manifolds)
{
  //make sure to determine if we even need to test these objects
  //(without this, composites that are in contact with their children will
  //always have energy because they will be resolving collision)
  if(!pair.Top->ShouldCollide(pair.Bot))
    return false;

  return mInternals->mCollisionTable.Collide(pair.Top, pair.Bot, &manifolds);
}

bool CollisionManager::ForceTestCollision(ColliderPair& pair, ManifoldArray& manifolds)
{
  return mInternals->mCollisionTable.Collide(pair.Top, pair.Bot, &manifolds);
}

bool CollisionManager::CollideShapes(Aabb& aabb, Collider* aabbCollider,
                                     Collider* otherCollider, Manifold* manifold)
{
  return mInternals->mAabbLookups.Collide(aabb, aabbCollider, otherCollider, manifold);
}

bool CollisionManager::CollideShapes(Triangle& tri, Collider* triCollider,
                                     Collider* otherCollider, Manifold* manifold)
{
  return mInternals->mTriangleLookups.Collide(tri, triCollider, otherCollider, manifold);
}

bool CollisionManager::TestIntersection(Collider* collider, Vec3Param point)
{
  return mInternals->mPointOverlapLookups.Collide(point, collider);
}

bool CollisionManager::TestIntersection(Collider* collider, const Aabb& aabb)
{
  return mInternals->mAabbOverlapLookups.Collide(aabb, collider);
}

bool CollisionManager::TestRayVsObject(void* userData, CastDataParam castData,
                                       ProxyResult& result, BaseCastFilter& filter)
{
  //Cast the data to a collider.
  Collider* collider = static_cast<Collider*>(userData);
  return mInternals->mRayCastLookup.Cast(castData.GetRay(), collider, &result, filter);
}

bool CollisionManager::TestSegmentVsObject(void* userData, CastDataParam castData,
                                           ProxyResult& result, BaseCastFilter& filter)
{
  //Cast the data to a collider.
  Collider* collider = static_cast<Collider*>(userData);
  return mInternals->mSegmentCastLookup.Cast(castData.GetSegment(), collider, &result, filter);
}

//-------------------------------------------------------------------- AABB Cast
//Tests a given collider (userData) against an AABB.
bool CollisionManager::TestAabbVsObject(void* userData, CastDataParam castData, 
                                        ProxyResult& result, BaseCastFilter& filter)
{
  //Cast the data to a collider.
  Collider* collider = static_cast<Collider*>(userData);
  return mInternals->mAabbCastLookup.Cast(castData.GetAabb(), collider, &result, filter);
}

//------------------------------------------------------------------ Sphere Cast
//Tests a given collider (userData) against a Sphere.
bool CollisionManager::TestSphereVsObject(void* userData, CastDataParam castData,
                                          ProxyResult& result, BaseCastFilter& filter)
{
  //Cast the data to a collider.
  Collider* collider = static_cast<Collider*>(userData);
  return mInternals->mSphereCastLookup.Cast(castData.GetSphere(), collider, &result, filter);
}

//----------------------------------------------------------------- Frustum Cast
//Casts a Frustum against an object. Distance is the distance to the first
//plane.
bool CollisionManager::TestFrustumVsObject(void* userData, CastDataParam castData,
                                           ProxyResult& result, BaseCastFilter& filter)
{
  //Cast the data to a collider.
  Collider* collider = static_cast<Collider*>(userData);
  return mInternals->mFrustumCastLookup.Cast(castData.GetFrustum(), collider, &result, filter);
}

}//namespace Physics

}//namespace Zero
