///////////////////////////////////////////////////////////////////////////////
///
/// \file ComplexShapeCollision.hpp
/// Declaration of the complex collision and casting functions. A complex
/// shape is defined as one that Contains multiple sub objects
/// (mesh, multi-collider). This is an attempt to minimize the code to write
/// for iterating through these sub objects as most of it is boiler-plate.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Physics/ShapeCollisionHelpers.hpp"

/// To support being a complex shape, the collider type must contain a typedef
/// called RangeInLocalSpace that is either set to true_type or false_type.
/// Also, the collider must have a GetOverlapRange(Aabb) that returns a range
/// of items to check. The item must be a class containing the Shape type to
/// check against named Shape and a uint that signifies the id of that
/// sub-shape called Index. The range will be handed an aabb in the space
/// specified by the RangeInLocalSpace typedef.
/// Finally, the class must have a typedef RangeType that is the type of
/// range returned by GetOverlapRange() (replace this with an auto?)
///
/// For special support when a collider can do it's own special logic to
/// filter a specific cast type more efficiently (such as the physics mesh
/// with it's internal AabbTree), there is the SpecialComplexCastVsShape
/// that expects a Cast(castType) function instead of GetOverlapRange().
/// The object passed in will be in the correct space specified by
/// RangeInLocalSpace.

namespace Zero
{

//These functors are to deal with colliding a shape against a complex shape.
//Some complex shapes store their data in local space and some store it in
//world space. When the collider stores data in world space, nothing extra
//needs to happen. When in local space, the aabb needs to be taken back into
//local space. Then, the shape given back by the collider needs to be
//transformed back into world space to perform the actual collision test with.

/// A functor to collide a shape against a complex collider that stores data in
/// local space. This needs to bring the aabb to local space and bring the
/// sub-shape of the collider back into world space.
struct LocalFunctor
{
  LocalFunctor(Collider* complexCollider)
  {
    transform = complexCollider->GetWorldTransform()->GetWorldMatrix();
    invTransform = transform.Inverted();
  }

  template <typename ComplexShapeType>
  typename ComplexShapeType::TransformedShapeType ToWorldShape(const ComplexShapeType& shape)
  {
    return shape.Transform(transform);
  }

  template <typename ComplexShapeType>
  typename ComplexShapeType::TransformedShapeType ToLocalShape(const ComplexShapeType& shape)
  {
    return shape.Transform(invTransform);
  }

  template <typename ComplexShapeType>
  Aabb ToLocalAabb(const ComplexShapeType& shape)
  {
    return ToAabb(ToLocalShape(shape));
  }

  Mat4 transform;
  Mat4 invTransform;
};

/// A functor to collide a shape against a complex collider that stores data in
/// world space. This only really computes the aabb of the shape.
struct WorldFunctor
{
  WorldFunctor(Collider* complexCollider)
  {
  }

  template <typename ComplexShapeType>
  ComplexShapeType ToWorldShape(const ComplexShapeType& shape)
  {
    return shape;
  }

  template <typename ComplexShapeType>
  ComplexShapeType ToLocalShape(const ComplexShapeType& shape)
  {
    return shape;
  }

  template <typename ComplexShapeType>
  Aabb ToLocalAabb(const ComplexShapeType& shape)
  {
    return ToAabb(ToLocalShape(shape));
  }
};

/// Deals with the iteration of a complex collider (a collider containing more
/// than one shape) against a normal shape. This expects a collider to support
/// an interface called GetOverlapRange(aabb) that returns a range
/// of what shapes to check collision against.
template <typename ColliderType, typename Shape2Type, typename SpaceFunctor>
bool ComplexCollideCollidersInternal(Collider* complexCollider, Collider* collider2,
                                     PodArray<Physics::Manifold>* manifolds)
{
  Shape2Type shape2;
  ColliderToShape(collider2, shape2);

  ColliderType* castedCollider = static_cast<ColliderType*>(complexCollider);
  //create the functor for this space conversion type
  SpaceFunctor functor(complexCollider);
  //grab the correct space aabb from the functor
  Aabb shape2Aabb = functor.ToLocalAabb(shape2);

  uint collisionCount = 0;
  typedef typename ColliderType::RangeType RangeType;
  //check all of the items that we get back in the range
  RangeType range = castedCollider->GetOverlapRange(shape2Aabb);
  for(; !range.Empty(); range.PopFront())
  {
    AutoDeclare(item, range.Front());
    //convert the shape to world space to perform collision detection against
    //(if manifolds is nullptr, we could keep this in local
    //space and just convert the collider shape once...)
    AutoDeclare(worldShape, functor.ToWorldShape(item.Shape));
    //need to get the type of the world shape for IntersectionToPhysicsManifold
    typedef TypeOf(worldShape) WorldShapeType;

    //deal with a boolean vs needing info test (maybe remove?)
    if(manifolds)
    {
      Intersection::Manifold iManifold;

      if(!CollideShapes(worldShape, shape2, &iManifold))
        continue;

      //fill out the manifold from the intersection results
      ColliderPair pair;
      pair.A = complexCollider;
      pair.B = collider2;

      Physics::Manifold* manifold = &manifolds->PushBack();
      manifold->SetPair(pair);
      IntersectionToPhysicsManifold<WorldShapeType, Shape2Type>(&iManifold, manifold);
      //set the id of this item on the manifold and mark that we collided with something
      manifold->ContactId = item.Index;
      FixInternalEdges(castedCollider, manifold, item.Index);
      ++collisionCount;
    }
    else
    {
      //in the boolean case, if we collide with anything then we will end up
      //returning true no matter how many sub-shape's we hit, so there
      //is no point in continuing, just return true
      if(CollideShapes(worldShape, shape2, nullptr))
        return true;
    }
  }
  //if we hit any sub-shape, then this object was hit and we return true
  return collisionCount > 0;
}

//These two functions use function overloading to determine which functor to
//pass into the above function. By using the overloaded functions with true/false_type,
//I can make sure that only the code that should actually run is generated.

///Resolves the true_type of RangeInLocalSpace to the LocalFunctor.
template <typename ColliderType, typename Shape2Type>
bool ComplexCollideCollidersResolveLocal(Collider* complexCollider, Collider* collider2,
                                         PodArray<Physics::Manifold>* manifolds, true_type)
{
  return ComplexCollideCollidersInternal<ColliderType, Shape2Type, LocalFunctor>
    (complexCollider, collider2, manifolds);
}

///Resolves the false_type of RangeInLocalSpace to the WorldFunctor.
template <typename ColliderType, typename Shape2Type>
bool ComplexCollideCollidersResolveLocal(Collider* complexCollider, Collider* collider2,
                                         PodArray<Physics::Manifold>* manifolds, false_type)
{
  return ComplexCollideCollidersInternal<ColliderType, Shape2Type, WorldFunctor>
    (complexCollider, collider2, manifolds);
}

//These two functions deal with resolving the order of which collider
//is the complex shape and pass it into the internal code above.

/// Resolves the order when the complex collider is the first collider.
template <typename ColliderType, typename Shape2Type>
bool ComplexCollideCollidersA(Collider* complexCollider, Collider* collider2,
                              PodArray<Physics::Manifold>* manifolds)
{
  //let overloading based upon the RangeInLocalSpace type take care of the functor
  return ComplexCollideCollidersResolveLocal<ColliderType, Shape2Type>
    (complexCollider, collider2, manifolds, typename ColliderType::RangeInLocalSpace());
}

/// Resolves the order when the complex collider is the second collider.
template <typename Shape1Type, typename ColliderType>
bool ComplexCollideCollidersB(Collider* collider1, Collider* complexCollider,
                              PodArray<Physics::Manifold>* manifolds)
{
  //let overloading based upon the RangeInLocalSpace type take care of the functor
  return ComplexCollideCollidersResolveLocal<ColliderType, Shape1Type>
    (complexCollider, collider1, manifolds, typename ColliderType::RangeInLocalSpace());
}

template <typename ColliderType0, typename ColliderType1, typename SpaceFunctor0, typename SpaceFunctor1>
bool ComplexVsComplexCollidersInternal(Collider* collider0, Collider* collider1, PodArray<Physics::Manifold>* manifolds)
{
  ColliderType0* castedCollider0 = static_cast<ColliderType0*>(collider0);
  ColliderType1* castedCollider1 = static_cast<ColliderType1*>(collider1);

  typedef typename ColliderType0::RangeType Range0Type;
  typedef typename ColliderType1::RangeType Range1Type;

  SpaceFunctor0 functor0(castedCollider0);
  SpaceFunctor1 functor1(castedCollider1);

  //bring the aabb of collider1 into collider0's local space
  Aabb aabb1InSpace0 = functor0.ToLocalAabb(castedCollider1->mAabb);

  uint collisionCount = 0;
  //iterate over all sub-shapes in collider0 that could possibly intersect with collider1
  Range0Type r0 = castedCollider0->GetOverlapRange(aabb1InSpace0);
  for(; !r0.Empty(); r0.PopFront())
  {
    AutoDeclare(item0, r0.Front());
    //convert the shape to world space to perform collision detection against
    //(if manifolds is nullptr, we could keep this in local
    //space and just convert the collider shape once...)
    AutoDeclare(worldShape0, functor0.ToWorldShape(item0.Shape));
    //need to get the type of the world shape for IntersectionToPhysicsManifold
    typedef TypeOf(worldShape0) WorldShape0Type;

    //now we need to take each sub-shape in collider0 and check to
    //see what sub-shapes in collider1 they could hit
    Aabb aabb0InSpace1 = ToAabb(functor1.ToLocalShape(ToAabb(worldShape0)));
    Range1Type r1 = castedCollider1->GetOverlapRange(aabb0InSpace1);
    for(; !r1.Empty(); r1.PopFront())
    {
      AutoDeclare(item1, r1.Front());
      //convert the shape to world space to perform collision detection against
      //(if manifolds is nullptr, we could keep this in local
      //space and just convert the collider shape once...)
      AutoDeclare(worldShape1, functor1.ToWorldShape(item1.Shape));
      //need to get the type of the world shape for IntersectionToPhysicsManifold
      typedef TypeOf(worldShape1) WorldShape1Type;

      //deal with a boolean vs needing info test (maybe remove?)
      if(manifolds)
      {
        Intersection::Manifold iManifold;

        if(!CollideShapes(worldShape0, worldShape1, &iManifold))
          continue;

        //fill out the manifold from the intersection results
        ColliderPair pair;
        pair.A = collider0;
        pair.B = collider1;

        Physics::Manifold* manifold = &manifolds->PushBack();
        manifold->SetPair(pair);
        IntersectionToPhysicsManifold<WorldShape0Type, WorldShape1Type>(&iManifold, manifold);

        //set the id of this item on the manifold
        //(just lexicographically pack the two indices together for now)
        manifold->ContactId = item0.Index << 16;
        manifold->ContactId |= item1.Index;

        //fix the internal edges (just try both colliders and one of them might fix it)
        FixInternalEdges(castedCollider0, manifold, item0.Index);
        FixInternalEdges(castedCollider1, manifold, item1.Index);

        //mark that we collided with something
        ++collisionCount;
      }
      else
      {
        //in the boolean case, if we collide with anything then we will end up
        //returning true no matter how many sub-shape's we hit, so there
        //is no point in continuing, just return true
        if(CollideShapes(worldShape0, worldShape1, nullptr))
          return true;
      }
    }
  }

  return collisionCount > 0;
}

template <typename ColliderType0, typename ColliderType1>
bool ComplexVsComplexColliders(Collider* collider0, Collider* collider1, PodArray<Physics::Manifold>* manifolds)
{
  ColliderType0* castedCollider0 = static_cast<ColliderType0*>(collider0);
  ColliderType1* castedCollider1 = static_cast<ColliderType1*>(collider1);

  bool type0Local = ColliderType0::RangeInLocalSpace::value;
  bool type1Local = ColliderType1::RangeInLocalSpace::value;
  //determine which collider needs a local space functor and which needs a world space functor
  //(could do some fancy template tricks to remove the if, but worry about that later)
  if(type0Local)
  {
    if(type1Local)
      return ComplexVsComplexCollidersInternal<ColliderType0, ColliderType1, LocalFunctor, LocalFunctor>(collider0, collider1, manifolds);
    else
      return ComplexVsComplexCollidersInternal<ColliderType0, ColliderType1, LocalFunctor, WorldFunctor>(collider0, collider1, manifolds);
  }
  else if(type1Local)
    return ComplexVsComplexCollidersInternal<ColliderType0, ColliderType1, WorldFunctor, LocalFunctor>(collider0, collider1, manifolds);
  else
    return ComplexVsComplexCollidersInternal<ColliderType0, ColliderType1, WorldFunctor, WorldFunctor>(collider0, collider1, manifolds);
}

template <typename ColliderType0, typename ColliderType1>
bool OverlapComplexVsComplexColliders(Collider* collider0, Collider* collider1)
{
  return ComplexVsComplexColliders<ColliderType0, ColliderType1>(collider0, collider1, nullptr);
}

/// The core logic of the complex shape cast. Helps to deal with the
/// weirdness of getting a different shape type when going to local.
template <typename CastType, typename RangeType, typename ColliderType>
bool ComplexCastInternal(const CastType& castShape, ColliderType* complexCollider,
                         RangeType& range, ProxyResult* result)
{
  bool hitItem = false;
  real closestTime = Math::PositiveMax();

  //deal with the user not wanting any results (is this even worth it?)
  if(result)
  {
    //check the cast shape against each of the items in the range
    for(; !range.Empty(); range.PopFront())
    {
      AutoDeclare(item, range.Front());

      Intersection::Manifold iManifold;

      //since we are actually storing results, we have to check every item in the range
      if(!CastShapes(castShape, item.Shape, &iManifold))
        continue;

      //if we already had a closer item, then don't record this one
      if(iManifold.Points[0].T >= closestTime)
        continue;

      closestTime = iManifold.Points[0].T;
      //convert the info to the ProxyResult and fetch the normal
      //(check the filter for the normal?)
      ManifoldToProxyResult(castShape, complexCollider, &iManifold, result);
      result->ShapeIndex = item.Index;
      GetNormalFromPointOnShape(castShape, item.Shape, complexCollider,
                                result->mPoints[0], result->mContactNormal);
      hitItem = true;
    }
  }
  else
  {
    //check the cast shape against each of the items in the range
    for(; !range.Empty(); range.PopFront())
    {
      AutoDeclare(item, range.Front());
      //since this is only a boolean, we can stop if we hit any item
      if(CastShapes(castShape, item.Shape, nullptr))
        return true;
    }
  }

  return hitItem;
}

/// Deals with colliding some shape against a complex collider. This collider is
/// expected to have a typedef RangeInLocalSpace to determine if it is in local
/// or world space and a function GetOverlapRange(aabb) to get the internal
/// shapes to check against (see comment at top for details). The reasoning
/// for this function is to allow a user to write only one code path to deal
/// with any sort of casted shape. The downside is that the collider can only
/// perform filtering based upon an aabb. If a collider can do a more efficient
/// cast against certain shapes, then it should use SpecialComplexCastVsShape declared below.
template <typename CastType, typename ColliderType>
bool ComplexCastVsShape(const CastType& castShape, Collider* complexCollider,
                        ProxyResult* result, BaseCastFilter& filter)
{
  ColliderType* castedCollider = static_cast<ColliderType*>(complexCollider);

  bool hitItem = false;

  //deal with local vs in world
  typedef typename ColliderType::RangeType RangeType;
  if(ColliderType::RangeInLocalSpace::value)
  {
    //transform the shape back into local space and take the aabb of that shape
    WorldTransformation* worldTransform = complexCollider->GetWorldTransform();
    Mat4 transform = worldTransform->GetWorldMatrix();
    Mat4 invTransform = transform.Inverted();

    //if we perform a transform normally, shapes can change (aka sphere's become ellipsoids)
    //if there's non-uniform scale. If there's uniform scale we'll do a special transform
    //that knows the shape stays the same. This will prevent casting a sphere from
    //becoming an ellipse and going through gjk or mpr when it doesn't need to.
    if(worldTransform->IsUniformlyScaled())
    {
      AutoDeclare(uniformLocalShape, castShape.UniformTransform(invTransform));
      Aabb castShapeAabb = GetCastDataAabb(uniformLocalShape);
      //get a range based upon the local space aabb
      RangeType range = castedCollider->GetOverlapRange(castShapeAabb);

      //do the internal range iteration with the local space shape
      hitItem = ComplexCastInternal(uniformLocalShape, complexCollider, range, result);
    }
    else
    {
      AutoDeclare(localShape, castShape.Transform(invTransform));
      Aabb castShapeAabb = GetCastDataAabb(localShape);
      //get a range based upon the local space aabb
      RangeType range = castedCollider->GetOverlapRange(castShapeAabb);

      //do the internal range iteration with the local space shape
      hitItem = ComplexCastInternal(localShape, complexCollider, range, result);
    }

    //Since we casted a local shape, we got local results.
    //We need to bring the results back into world space.
    if(hitItem)
    {
      result->mPoints[0] = Math::TransformPoint(transform, result->mPoints[0]);
      result->mPoints[1] = Math::TransformPoint(transform, result->mPoints[1]);
      result->mContactNormal = Math::TransformNormal(transform, result->mContactNormal).Normalized();
    }
  }
  else
  {
    //get a range of items to iterate through based upon the aabb of the cast shape
    Aabb castShapeAabb = GetCastDataAabb(castShape);
    RangeType range = castedCollider->GetOverlapRange(castShapeAabb);
    //go through the internal code that iterates over the range regardless of space
    hitItem = ComplexCastInternal(castShape, complexCollider, range, result);
  }

  return hitItem;
}

template <typename ShapeType, typename ComplexColliderType>
bool ComplexOverlapVsShape(const ShapeType& shape, Collider* complexCollider)
{
  ComplexColliderType* castedComplexCollider = static_cast<ComplexColliderType*>(complexCollider);

  Aabb shapeAabb = ToAabb(shape);
  typedef typename ComplexColliderType::RangeType RangeType;
  if(ComplexColliderType::RangeInLocalSpace::value)
  {
    LocalFunctor functor(castedComplexCollider);
    Aabb localAabb = functor.ToLocalAabb(shapeAabb);

    RangeType r = castedComplexCollider->GetOverlapRange(localAabb);
    for(; !r.Empty(); r.PopFront())
    {
      AutoDeclare(item, r.Front());
      AutoDeclare(worldShape, functor.ToWorldShape(item.Shape));

      if(Overlap(worldShape, shape) == true)
        return true;
    }
  }
  else
  {
    RangeType r = castedComplexCollider->GetOverlapRange(shapeAabb);
    for(; !r.Empty(); r.PopFront())
    {
      AutoDeclare(item, r.Front());
      AutoDeclare(worldShape, item.Shape);

      if(Overlap(worldShape, shape) == true)
        return true;
    }
  }

  return false;
}

/// Some objects might be able to do a more efficient cast of certain types if
/// they store some internal mid-phase (BSP, KD, Aabb trees). To allow the collider
/// to use that internal structure, they can use this template. All that they are
/// expected to have is the RangeInLocalSpace typedef and a function called Cast
/// of the correct cast data type (eg. Ray).
/// Note: if a collider is local and wants to specialize sphere or aabb, it needs
/// to deal with the fact that the shape it will actually receive is an ellipsoid or obb.
template <typename CastType, typename ColliderType>
bool SpecialComplexCastVsShape(const CastType& castShape, Collider* complexCollider,
                               ProxyResult* result, BaseCastFilter& filter)
{
  ColliderType* castedCollider = static_cast<ColliderType*>(complexCollider);

  bool hitItem = false;

  //deal with local vs in world
  if(ColliderType::RangeInLocalSpace::value)
  {
    //if local, we have to transform the shape back into local space
    Mat4 transform = complexCollider->GetWorldTransform()->GetWorldMatrix();
    Mat4 invTransform = transform.Inverted();

    AutoDeclare(localShape, castShape.Transform(invTransform));
    //then cast the local space shape into the collider
    hitItem = castedCollider->Cast(localShape, *result, filter);

    //Since we casted a local shape, we got local results.
    //We need to bring the results back into world space.
    if(hitItem)
    {
      result->mPoints[0] = Math::TransformPoint(transform, result->mPoints[0]);
      result->mPoints[1] = Math::TransformPoint(transform, result->mPoints[1]);
      result->mContactNormal = Math::TransformNormal(transform, result->mContactNormal).Normalized();
    }
  }
  else
  {
    //if in world, just cast the shape into the collider
    hitItem = castedCollider->Cast(castShape, *result, filter);
  }

  return hitItem;
}

}//namespace Zero
