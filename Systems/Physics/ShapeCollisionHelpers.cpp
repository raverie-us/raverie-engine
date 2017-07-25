///////////////////////////////////////////////////////////////////////////////
///
/// \file ShapeCollisionHelpers.cpp
/// Declaration of a lot of helpers for the shape collision resolution.
/// Most of the helpers are for conversions of types or filling out manifolds.
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------- Misc Helpers

void IntersectionToPhysicsManifoldFull(Intersection::Manifold* iManifold,
                                       Physics::Manifold* pManifold)
{
  pManifold->ContactCount = iManifold->PointCount;
  pManifold->SetNormal(iManifold->Normal);

  Physics::ManifoldPoint manifoldPoints[cMaxContacts];
  for(unsigned int i = 0; i < iManifold->PointCount; ++i)
  {
    manifoldPoints[i].Penetration = iManifold->PointAt(i).Depth;
    manifoldPoints[i].WorldPoints[0] = iManifold->PointAt(i).Points[0];
    manifoldPoints[i].WorldPoints[1] = iManifold->PointAt(i).Points[1];
    manifoldPoints[i].Normal = iManifold->Normal;
  }
  pManifold->SetPolicy(Physics::AddingPolicy::NormalManifold);
  pManifold->AddPoints(manifoldPoints, iManifold->PointCount);
  pManifold->SetPolicy(Physics::AddingPolicy::FullManifold);
}

void IntersectionToPhysicsManifoldPersistent(Intersection::Manifold* iManifold,
                                             Physics::Manifold* pManifold)
{
  pManifold->ContactCount = iManifold->PointCount;
  pManifold->SetNormal(iManifold->Normal);

  Physics::ManifoldPoint manifoldPoints[cMaxContacts];
  for(unsigned int i = 0; i < iManifold->PointCount; ++i)
  {
    manifoldPoints[i].Penetration = iManifold->PointAt(i).Depth;
    manifoldPoints[i].WorldPoints[0] = iManifold->PointAt(i).Points[0];
    manifoldPoints[i].WorldPoints[1] = iManifold->PointAt(i).Points[1];
    manifoldPoints[i].Normal = iManifold->Normal;
  }
  pManifold->SetPolicy(Physics::AddingPolicy::NormalManifold);
  pManifold->AddPoints(manifoldPoints, iManifold->PointCount);
  pManifold->SetPolicy(Physics::AddingPolicy::PersistentManifold);
}

//-------------------------------------------------------------------Internal Edge Fixing

void FixInternalEdges(GenericPhysicsMesh* mesh, Physics::Manifold* manifold, uint objectIndex, uint contactId)
{
  CorrectInternalEdgeNormal(mesh, manifold, objectIndex, contactId);
}

void FixInternalEdges(Collider* collider, Physics::Manifold* manifold, uint contactId)
{

}

void FixInternalEdges(ConvexMeshCollider* collider, Physics::Manifold* manifold, uint contactId)
{
  if(collider == manifold->Objects[0])
    FixInternalEdges(collider->GetConvexMesh(), manifold, 0, contactId);
  else
    FixInternalEdges(collider->GetConvexMesh(), manifold, 1, contactId);
}

void FixInternalEdges(MeshCollider* collider, Physics::Manifold* manifold, uint contactId)
{
  if(collider == manifold->Objects[0])
    FixInternalEdges(collider->GetPhysicsMesh(), manifold, 0, contactId);
  else
    FixInternalEdges(collider->GetPhysicsMesh(), manifold, 1, contactId);
}

void FixInternalEdges(HeightMapCollider* collider, Physics::Manifold* manifold, uint contactId)
{
  if(collider == manifold->Objects[0])
    CorrectInternalEdgeNormal(collider, manifold, 0, contactId);
  else
    CorrectInternalEdgeNormal(collider, manifold, 1, contactId);
}

//----------------------------------------------------- Manifold To Proxy Result

void BaseManifoldToProxyResult(Intersection::Manifold* manifold, ProxyResult* result)
{
  result->mContactNormal = manifold->Normal;
  result->mPoints[0] = manifold->PointAt(0).Points[0];
  result->mPoints[1] = manifold->PointAt(0).Points[1];
}

void ManifoldToProxyResult(const Aabb& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result)
{
  //set distance as distance from center
  Vec3 objectCenter = collider->GetWorldTranslation();
  Vec3 aabbCenter = castShape.GetCenter();
  result->mDistance = (objectCenter - aabbCenter).Length();
  BaseManifoldToProxyResult(manifold, result);
}

void ManifoldToProxyResult(const Frustum& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result)
{
  //just set the distance to some valid value
  result->mDistance = real(.1);
  BaseManifoldToProxyResult(manifold, result);
}

void ManifoldToProxyResult(const Sphere& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result)
{
  //set distance as distance from center
  Vec3 objectCenter = collider->GetWorldTranslation();
  Vec3 aabbCenter = castShape.mCenter;
  result->mDistance = (objectCenter - aabbCenter).Length();
  BaseManifoldToProxyResult(manifold, result);
}

//--------------------------------------------------- Normal From Point On Shape

Vec3 NormalFromPointOnShape(const Aabb& aabb, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  return Geometry::NormalFromPointOnAabb(point, aabb.mMin, aabb.mMax);
}

Vec3 NormalFromPointOnShape(const Sphere& sphere, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  return Geometry::NormalFromPointOnSphere(point, sphere.mCenter, sphere.mRadius);
}

Vec3 NormalFromPointOnShape(const Obb& obb, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  return Geometry::NormalFromPointOnObb(point, obb.Center, obb.HalfExtents, obb.Basis);
}

Vec3 NormalFromPointOnShape(const Cylinder& cylinder, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  //currently, cylinder needs a basis and half extents for this call,
  //not two points and a radius, for now extract that info from the collider
  const Vec3 center = shapeCollider->GetWorldTranslation();
  CylinderCollider* cylinderCollider = static_cast<CylinderCollider*>(shapeCollider);
  real worldRadius = cylinderCollider->GetWorldRadius();
  real worldHalfHeight = cylinderCollider->GetWorldHalfHeight();
  uint heightIndex = cylinderCollider->GetHeightIndex();
  uint radius1Index, radius2Index;
  cylinderCollider->GetRadiiIndices(radius1Index, radius2Index);

  const Mat3 orientation = shapeCollider->GetWorldRotation();
  Mat3 rotatedOrientation;
  rotatedOrientation.SetBasis(1, orientation.GetBasis(heightIndex));
  rotatedOrientation.SetBasis(2, orientation.GetBasis(radius1Index));
  rotatedOrientation.SetBasis(0, orientation.GetBasis(radius2Index));

  Vec3 normal = Geometry::NormalFromPointOnCylinder(point, center, worldRadius, worldHalfHeight, rotatedOrientation);
  return normal;
}

Vec3 NormalFromPointOnShape(const Ellipsoid& ellipsoid, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  return Geometry::NormalFromPointOnEllipsoid(point, ellipsoid.Center,
                                              ellipsoid.Radii, ellipsoid.Basis);
}

Vec3 NormalFromPointOnShape(const Capsule& capsule, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  return Geometry::NormalFromPointOnCapsule(point, capsule.PointA,
                                            capsule.PointB, capsule.Radius);
}

Vec3 NormalFromPointOnShape(const Triangle& triangle, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  Vec3 normal = Geometry::NormalFromPointOnTriangle(point, triangle.p0,
                                                    triangle.p1, triangle.p2);

  //The normal returned should always be positive in the y, but if the ray
  //was cast from below the triangle, we want to invert it.
  if(Dot(normal, triangle.p0 - start) > 0)
    normal *= real(-1.0);
  return normal;
}

//------------------------------------------------------------ Collider To Shape

void ColliderToShape(Collider* collider, Sphere& sphere)
{
  SphereCollider* sphereCollider = static_cast<SphereCollider*>(collider);
  sphere.mCenter = sphereCollider->GetWorldTranslation() + collider->mCollisionOffset;
  sphere.mRadius = sphereCollider->GetWorldRadius();
}

void ColliderToShape(Collider* collider, Aabb& aabb)
{
  BoxCollider* boxCollider = static_cast<BoxCollider*>(collider);
  aabb = boxCollider->mAabb;
  aabb.SetCenter(aabb.GetCenter() + collider->mCollisionOffset);
}

void ColliderToShape(Collider* collider, Obb& obb)
{
  BoxCollider* boxCollider = static_cast<BoxCollider*>(collider);
  obb.Center = boxCollider->GetWorldTranslation() + collider->mCollisionOffset;
  obb.HalfExtents = boxCollider->GetWorldSize() * real(0.5f);
  obb.Basis = boxCollider->GetWorldRotation();
}

void ColliderToShape(Collider* collider, Ellipsoid& ellipsoid)
{
  EllipsoidCollider* ellipsoidCollider = static_cast<EllipsoidCollider*>(collider);
  ellipsoid.Center = ellipsoidCollider->GetWorldTranslation() + collider->mCollisionOffset;
  ellipsoid.Radii = ellipsoidCollider->GetWorldRadii();
  ellipsoid.Basis = ellipsoidCollider->GetWorldRotation();
}

void ColliderToShape(Collider* collider, Cylinder& cylinder)
{
  CylinderCollider* cylinderCollider = static_cast<CylinderCollider*>(collider);
  cylinderCollider->ComputeWorldPoints(cylinder.PointA, cylinder.PointB);
  cylinder.Radius = cylinderCollider->GetWorldRadius();
  cylinder.PointA += collider->mCollisionOffset;
  cylinder.PointB += collider->mCollisionOffset;
}

void ColliderToShape(Collider* collider, Capsule& capsule)
{
  CapsuleCollider* capsuleCollider = static_cast<CapsuleCollider*>(collider);
  capsuleCollider->ComputeWorldPoints(capsule.PointA, capsule.PointB);
  capsule.Radius = capsuleCollider->GetWorldRadius();
  capsule.PointA += collider->mCollisionOffset;
  capsule.PointB += collider->mCollisionOffset;
}

void ColliderToShape(Collider* collider, ConvexMeshShape& convexMesh)
{
  convexMesh = collider->GetSupportShape();
  convexMesh.mWorldAabb = collider->mAabb;

  Vec3 center;
  convexMesh.mSupport.GetCenter(&center);
  convexMesh.mSupport.SetCenter(center + collider->mCollisionOffset);
}

}//namespace Zero
