///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleShapeCollision.hpp
/// Declaration of the Simple collision and casting functions. A simple shape
/// is a collider that consists of only one shape from the shape library
/// (no meshes).
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//Not currently in use. Due to some shapes needing the ability to return
//multiple manifolds, all shapes have to deal with that possibility
//so they use the code below.
template <typename Shape1Type, typename Shape2Type>
bool CollideCollidersSingleResult(Collider* collider1, Collider* collider2,
                                  Physics::Manifold* manifold)
{
  //convert the collider's to their corresponding shapes
  Shape1Type shape1;
  ColliderToShape(collider1, shape1);
  Shape2Type shape2;
  ColliderToShape(collider2, shape2);

  if(manifold)
  {
    Intersection::Manifold iManifold;

    if(!CollideShapes(shape1, shape2, &iManifold))
      return false;

    //fill out the manifold from the intersection results
    ColliderPair pair;
    pair.A = collider1;
    pair.B = collider2;

    manifold->SetPair(pair);
    manifold->ContactId = 0;
    IntersectionToPhysicsManifold<Shape1Type, Shape2Type>(&iManifold, manifold);
    return true;
  }

  return CollideShapes(shape1, shape2, nullptr);
}

/// The code path for simple collider types to check collision
/// against each other. This will test for intersection between
/// the colliders then fill out the manifold.
template <typename Shape1Type, typename Shape2Type>
bool CollideColliders(Collider* collider1, Collider* collider2,
                      PodArray<Physics::Manifold>* manifolds)
{
  //convert the collider's to their corresponding shapes
  Shape1Type shape1;
  ColliderToShape(collider1, shape1);
  Shape2Type shape2;
  ColliderToShape(collider2, shape2);

  //if we have something to fill out
  if(manifolds)
  {
    Intersection::Manifold iManifold;

    if(!CollideShapes(shape1, shape2, &iManifold))
      return false;

    //fill out the manifold from the intersection results
    ColliderPair pair;
    pair.A = collider1;
    pair.B = collider2;

    Physics::Manifold* manifold = &manifolds->PushBack();
    manifold->ContactId = 0;
    manifold->SetPair(pair);
    //convert the intersection manifold to a physics one (the template
    //will determine if this should be a persistent or a full manifold)
    IntersectionToPhysicsManifold<Shape1Type, Shape2Type>(&iManifold, manifold);
    return true;
  }

  //if we have nothing to fill out, just return the result
  return CollideShapes(shape1, shape2, nullptr);
}

/// Given Two colliders, collide them using mpr. Used for
/// things that can't go through the collide overloads because
/// there is no corresponding shape type yet (meshes).
/// No longer used since meshes have a shape type, might still be useful to
/// specialize something to Mpr so it is being left in. (Testing results between the two)
inline bool CollideMprColliders(Collider* collider1, Collider* collider2,
                                PodArray<Physics::Manifold>* manifolds)
{
  //Test for collision.
  Intersection::SupportShape a = collider1->GetSupportShape();
  Intersection::SupportShape b = collider2->GetSupportShape();
  Intersection::Mpr mpr;

  if(manifolds)
  {
    Intersection::Manifold iManifold;

    Intersection::Type ret = mpr.Test(&a, &b, &iManifold);
    if(ret < (Intersection::Type)0)
      return false;

    //make sure the manifold info is facing the right direction
    FlipSupportShapeManifoldInfo(a, b, &iManifold);

    //fill out the manifold from the intersection results
    ColliderPair pair;
    pair.A = collider1;
    pair.B = collider2;

    Physics::Manifold* manifold = &manifolds->PushBack();
    manifold->ContactId = 0;
    manifold->SetPair(pair);
    IntersectionToPhysicsManifoldPersistent(&iManifold, manifold);
    return true;
  }

  Intersection::Type ret = mpr.Test(&a, &b, nullptr);
  return ret < (Intersection::Type)0;
}

/// Collides a shape with a collider and fills out a manifold. The manifold
/// needs two objects though, so this function requires the collider that is
/// associated with the shape. (Useful for meshes and multi-collider since
/// they need to collide with an internal type but need to make a manifold
/// with itself).
template <typename Shape1Type, typename Shape2Type>
bool CollideVsShape(Shape1Type& shape, Collider* shapeCollider, Collider* collider,
                    Physics::Manifold* manifold)
{
  Shape2Type shape2;
  ColliderToShape(collider, shape2);

  if(manifold)
  {
    Intersection::Manifold iManifold;

    if(!CollideShapes(shape, shape2, &iManifold))
      return false;

    //fill out the manifold from the intersection results
    ColliderPair pair;
    pair.A = shapeCollider;
    pair.B = collider;

    manifold->ContactId = 0;
    manifold->SetPair(pair);
    IntersectionToPhysicsManifold<Shape1Type, Shape2Type>(&iManifold, manifold);
    return true;
  }

  return CollideShapes(shape, shape2, nullptr);
}

/// Boolean overlap between two colliders. Doesn't require any
/// extra stuff (manifold, collider) since it doesn't have a manifold to fill out.
template <typename Shape1Type, typename Shape2Type>
bool OverlapColliders(Collider* collider1, Collider* collider2)
{
  Shape1Type shape1;
  ColliderToShape(collider1, shape1);

  Shape2Type shape2;
  ColliderToShape(collider2, shape2);

  return OverlapShapes(shape1, shape2);
}

/// Boolean overlap between a shape type and a collider. Doesn't require any
/// extra stuff (manifold, collider) since it doesn't have a manifold to fill out.
template <typename Shape1Type, typename Shape2Type>
bool OverlapVsShape(const Shape1Type& shape, Collider* collider)
{
  Shape2Type shape2;
  ColliderToShape(collider, shape2);

  return OverlapShapes(shape, shape2);
}


//should make a cast version (ray,segment) and a volume version
//(sphere,aabb,frustum) to deal better with the manifold weirdness.

/// Casts some shape against a collider. The only real difference between this
/// and CollideVsShape is that this deals with a proxy result instead of a
/// manifold. Saves the user the trouble of converting the manifold.
template <typename CastType, typename Shape2Type>
bool CastVsShape(const CastType& castShape, Collider* collider,
                 ProxyResult* result, BaseCastFilter& filter)
{
  Shape2Type shape2;
  ColliderToShape(collider, shape2);

  if(result)
  {
    Intersection::Manifold iManifold;

    if(!CastShapes(castShape, shape2, &iManifold))
      return false;

    if(filter.IsSet(BaseCastFilterFlags::IgnoreInternalCasts) && iManifold.PointCount == 1)
      return false;

    //fill out the ProxyResult from the intersection results
    result->mObjectHit = collider;
    ManifoldToProxyResult(castShape, collider, &iManifold, result);
    //should check the filter here...(JoshD questions)
    GetNormalFromPointOnShape(castShape, shape2, collider, result->mPoints[0], result->mContactNormal);
    return true;
  }

  return CastShapes(castShape, shape2, nullptr);
}

}//namespace Zero
