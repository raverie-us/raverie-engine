///////////////////////////////////////////////////////////////////////////////
///
/// \file ShapeCollisionHelpers.hpp
/// Declaration of a lot of helpers for the shape collision resolution.
/// Most of the helpers are for conversions of types or filling out manifolds.
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct ProxyResult;
class Collider;
class GenericPhysicsMesh;
class ConvexMeshCollider;
class MultiConvexMeshCollider;
class MeshCollider;
class HeightMapCollider;

//-------------------------------------------------------------------Misc Helpers

/// Performs a full manifold replacement (I get a full set
/// of contacts from intersection instead of a few new points)
void IntersectionToPhysicsManifoldFull(Intersection::Manifold* iManifold,
                                       Physics::Manifold* pManifold);
/// Performs a persistent manifold replacement. Used when intersection doesn't
/// report a full new set of points and we need to build up a multi-point
/// manifold on our own. Used most of the time (for instance, mpr).
void IntersectionToPhysicsManifoldPersistent(Intersection::Manifold* iManifold,
                                             Physics::Manifold* pManifold);

/// By default, when making a manifold we want to
/// use persistent apart from a few special cases.
template <typename Shape1Type, typename Shape2Type>
void IntersectionToPhysicsManifold(Intersection::Manifold* iManifold,
                                   Physics::Manifold* pManifold)
{
  IntersectionToPhysicsManifoldPersistent(iManifold,pManifold);
}

//saves a bit of typing...
//The only full manifolds now are [triangle,obb], [aabb,obb], and [obb,obb].
//Also need to make sure we handle the backwards case
#define DeclareFullManifoldSpecialization(Type1, Type2)                                      \
  template<>                                                                                 \
  inline void IntersectionToPhysicsManifold<Type1, Type2>(Intersection::Manifold* iManifold, \
                                                         Physics::Manifold* pManifold)       \
  {                                                                                          \
    IntersectionToPhysicsManifoldFull(iManifold, pManifold);                                 \
  }                                                                                          \
                                                                                             \
  template<>                                                                                 \
  inline void IntersectionToPhysicsManifold<Type2, Type1>(Intersection::Manifold* iManifold, \
                                                         Physics::Manifold* pManifold)       \
  {                                                                                          \
    IntersectionToPhysicsManifoldFull(iManifold, pManifold);                                 \
  }

//silly to macro this now, but if we had more this would be a lot cleaner...
//Specialize the shapes that need to do a full manifold instead of a persistent one
DeclareFullManifoldSpecialization(Triangle, Obb);
DeclareFullManifoldSpecialization(Aabb, Obb);
DeclareFullManifoldSpecialization(Sphere, Triangle);

template<>
inline void IntersectionToPhysicsManifold<Obb,Obb>(Intersection::Manifold* iManifold,
                                                   Physics::Manifold* pManifold)
{
  IntersectionToPhysicsManifoldFull(iManifold, pManifold);
}

#undef DeclareFullManifoldSpecialization

//-------------------------------------------------------------------Internal Edge Fixing

void FixInternalEdges(GenericPhysicsMesh* mesh, Physics::Manifold* manifold, uint objectIndex, uint contactId);

void FixInternalEdges(Collider* collider, Physics::Manifold* manifold, uint contactId);
void FixInternalEdges(ConvexMeshCollider* collider, Physics::Manifold* manifold, uint contactId);
void FixInternalEdges(MeshCollider* collider, Physics::Manifold* manifold, uint contactId);
void FixInternalEdges(HeightMapCollider* collider, Physics::Manifold* manifold, uint contactId);

//-------------------------------------------------------------------ManifoldToProxyResult

void BaseManifoldToProxyResult(Intersection::Manifold* manifold, ProxyResult* result);

//Base behavior for Manifold to proxy which is for ray and segment. Need to
//actually fix this to deal with the returned types of point/segment. (supposedly
//that is necessary for dealing with rays starting inside of a shape, the point0
//point1 issue was fixed in intersection).
template <typename CastType>
void ManifoldToProxyResult(const CastType& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result)
{
  result->mTime = manifold->PointAt(0).T;

  BaseManifoldToProxyResult(manifold, result);
}

void ManifoldToProxyResult(const Aabb& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result);
void ManifoldToProxyResult(const Frustum& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result);
void ManifoldToProxyResult(const Sphere& castShape, Collider* collider,
                           Intersection::Manifold* manifold, ProxyResult* result);

//-------------------------------------------------------------------NormalFromPointOnShape

//Unfortunately, these need a collider because of how the cylinder version works

/// Templated type for shapes we don't know about (convex mesh)
template <typename ShapeType>
Vec3 NormalFromPointOnShape(const ShapeType& shape, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start)
{
  return Vec3::cXAxis;
}

Vec3 NormalFromPointOnShape(const Aabb& aabb, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);
Vec3 NormalFromPointOnShape(const Sphere& sphere, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);
Vec3 NormalFromPointOnShape(const Obb& obb, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);
Vec3 NormalFromPointOnShape(const Cylinder& cylinder, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);
Vec3 NormalFromPointOnShape(const Ellipsoid& ellipsoid, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);
Vec3 NormalFromPointOnShape(const Capsule& capsule, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);
Vec3 NormalFromPointOnShape(const Triangle& triangle, Collider* shapeCollider,
                            Vec3Param point, Vec3Param start);

//-------------------------------------------------------------------GetNormalFromPointOnShape

//to deal with shape types that we should be getting a normal when casting against
//(we only get normals against segments and rays, other shapes we ignore this call on)
template <typename CastType, typename ShapeType>
void GetNormalFromPointOnShape(const CastType& castShape, const ShapeType& shape,
                               Collider* shapeCollider, Vec3Param point, Vec3Ref normal)
{
  //we don't do anything with this case, but set the normal to a valid
  //value so later calculations don't blow up (a normalize with a garbage vector)
  normal = Vec3::cXAxis;
}

template <typename ShapeType>
void GetNormalFromPointOnShape(const Ray& castShape, const ShapeType& shape,
                               Collider* shapeCollider, Vec3Param point, Vec3Ref normal)
{
  normal = NormalFromPointOnShape(shape, shapeCollider, point, castShape.Start);
}

template <typename ShapeType>
void GetNormalFromPointOnShape(const Segment& castShape, const ShapeType& shape,
                               Collider* shapeCollider, Vec3Param point, Vec3Ref normal)
{
  normal = NormalFromPointOnShape(shape, shapeCollider, point, castShape.Start);
}

//-------------------------------------------------------------------ColliderToShape

///Anything that asserts in here either needs a new function or should be going
///through the complex interface (meshes, multi-collider, etc...)
template <typename ShapeType>
void ColliderToShape(Collider* collider, ShapeType& shape)
{
  ErrorIf(true, "ColliderToShape not defined for type");
}

void ColliderToShape(Collider* collider, Sphere& sphere);
void ColliderToShape(Collider* collider, Aabb& aabb);
void ColliderToShape(Collider* collider, Obb& obb);
void ColliderToShape(Collider* collider, Ellipsoid& ellipsoid);
void ColliderToShape(Collider* collider, Cylinder& cylinder);
void ColliderToShape(Collider* collider, Capsule& capsule);
void ColliderToShape(Collider* collider, ConvexMeshShape& convexMesh);

//-------------------------------------------------------------------GetCastDataAabb

/// Currently I need to get the aabb of the cast shape to hand off to the
/// complex collider for filtering, however, taking the aabb of any shape
/// doesn't exactly make sense. For instance, what should the aabb of a ray be?
/// So make the generic version and then specialize a ray to have a rather large aabb.
template <typename CastType>
Aabb GetCastDataAabb(const CastType& castShape)
{
  return ToAabb(castShape);
}

/// Should probably actually intersect the ray with the complex
/// collider's aabb and take the aabb of the start point and the
/// intersection, but it doesn't matter that much for now.
/// (Doing the above will potentially generate a smaller aabb, but not always,
/// if the ray exits in the middle of a side, then the result will be smaller)
inline Aabb GetCastDataAabb(const Ray& castShape)
{
  return ToAabb(castShape, real(999999.0));
}

//-------------------------------------------------------------------CollideShapes

//No real reason for this to be templated. Only possibility is
//if we ever need to specialize a case. This most likely would happen if
//the manifold needs to be combined differently that normal. (or meshes?)
template <typename Shape1Type, typename Shape2Type>
bool CollideShapes(Shape1Type& shape1, Shape2Type& shape2,
                   Intersection::Manifold* result)
{
  return Collide(shape1, shape2, result);
}

//-------------------------------------------------------------------OverlapShapes

//No real reason for this to be templated. Only possibility is
//if we ever need to specialize a case. (Meshes?)
template <typename Shape1Type, typename Shape2Type>
bool OverlapShapes(const Shape1Type& shape1, const Shape2Type& shape2)
{
  return Overlap(shape1, shape2);
}

//-------------------------------------------------------------------CastShapes

//No real reason for this to be templated. Only possibility is
//if we ever need to specialize a case. (Meshes?)
template <typename Shape1Type, typename Shape2Type>
bool CastShapes(const Shape1Type& shape1, const Shape2Type& shape2,
                Intersection::Manifold* result)
{
  return Collide(shape1, shape2, result);
}

}//namespace Zero
