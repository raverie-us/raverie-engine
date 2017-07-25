///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Intersection
{
struct Manifold;
}//namespace Intersection

namespace Zero
{

struct ProxyResult;
struct BaseCastFilter;
struct CastData;
typedef const CastData& CastDataParam;

namespace Physics
{

struct InternalImplementation;

/// Manages all of the resolving of collider types. This structures is created
/// once and used as an interface to the more complicated resolving of collider
/// types. This also helps to keep code-gen down by only having one area that
/// sees the internal templates.
class CollisionManager
{
public:
  CollisionManager();
  ~CollisionManager();

  ///Returns collision information of two objects if they collided.
  bool TestCollision(ColliderPair& pair, ManifoldArray& manifolds);
  //Tests collision, doesn't care about static or asleep objects when testing.
  bool ForceTestCollision(ColliderPair& pair, ManifoldArray& manifolds);

  //these two functions are not currently in use due to a refactor, however
  //they might become useful again when performing a collision test between
  //two complex colliders. Therefore, I am not removing this code.
  bool CollideShapes(Aabb& aabb, Collider* aabbCollider, Collider* otherCollider, Manifold* manifold);
  bool CollideShapes(Triangle& tri, Collider* triCollider, Collider* otherCollider, Manifold* manifold);

  ///Determines if the collider intersects with a point.
  static bool TestIntersection(Collider* collider, Vec3Param point);
  ///Determines if the collider intersects with an Aabb.
  static bool TestIntersection(Collider* collider, const Aabb& aabb);

  ///Used as callbacks to the casting system
  ///Casts a Ray against an object.
  static bool TestRayVsObject(void* userData, CastDataParam castData,
                              ProxyResult& result, BaseCastFilter& filter);

  ///Casts a Segment against an object.
  static bool TestSegmentVsObject(void* userData, CastDataParam castData,
                                  ProxyResult& result, BaseCastFilter& filter);

  ///Casts an Aabb against an object.
  static bool TestAabbVsObject(void* userData, CastDataParam castData,
                               ProxyResult& result, BaseCastFilter& filter);

  ///Casts a Sphere against an object.
  static bool TestSphereVsObject(void* userData, CastDataParam castData,
                                 ProxyResult& result, BaseCastFilter& filter);

  ///Casts a Frustum against an object.  Distance is the distance to the first
  ///plane.
  static bool TestFrustumVsObject(void* userData, CastDataParam castData,
                                  ProxyResult& result, BaseCastFilter& filter);

private:

  /// Private implementation of the n-squared collider type resolution.
  /// Preventing anyone who sees this from having to generate the internal
  /// templates that they don't need to know about.
  static InternalImplementation* mInternals;
};

}//namespace Physics

}//namespace Zero
