///////////////////////////////////////////////////////////////////////////////
///
/// \file ShapeCollision.hpp
/// Declaration of the different lookup structs that resolve a collider's
/// type to the correct function pointer to perform collision detection on.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//macro to define a function pointer for the simple collider types.
#define GenerateCollisionLookups(FunctionName, array)                       \
  for(uint i = 0; i < (uint)Collider::cSize; ++i)                           \
    array[i] = nullptr;                                                     \
  array[Collider::cBox] = FunctionName<ShapeType, Obb>;                     \
  array[Collider::cCapsule] = FunctionName<ShapeType, Capsule>;             \
  array[Collider::cCylinder] = FunctionName<ShapeType, Cylinder>;           \
  array[Collider::cEllipsoid] = FunctionName<ShapeType, Ellipsoid>;         \
  array[Collider::cSphere] = FunctionName<ShapeType, Sphere>;               \
  array[Collider::cConvexMesh] = FunctionName<ShapeType, ConvexMeshShape>; 

#define GenerateComplexCollisionLookups(FunctionName, array)                            \
  array[Collider::cMultiConvexMesh] = FunctionName<ShapeType, MultiConvexMeshCollider>; \
  array[Collider::cMesh] = FunctionName<ShapeType, MeshCollider>;                       \
  array[Collider::cHeightMap] = FunctionName<ShapeType, HeightMapCollider>;             \

//generate the collide vs shape lookup array
template <typename ShapeType, typename ArrayType>
void GenerateArrayLookup(ArrayType& array)
{
  GenerateCollisionLookups(CollideVsShape, array);
}

//generate the collide colliders lookup array
template <typename ShapeType, typename ArrayType>
void GenerateColliderArrayLookup(ArrayType& array)
{
  GenerateCollisionLookups(CollideColliders, array); 
}

//generate the overlap colliders lookup array
template <typename ShapeType, typename ArrayType>
void GenerateOverlapCollidersArrayLookup(ArrayType& array)
{
  GenerateCollisionLookups(OverlapColliders, array); 
}

template <typename ShapeType, typename ArrayType>
void GenerateOverlapComplexCollidersArrayLookup(ArrayType& array)
{
  GenerateComplexCollisionLookups(OverlapComplexVsComplexColliders, array);
}

//generate the overlap vs shape lookup array
template <typename ShapeType, typename ArrayType>
void GenerateArrayOverlapLookup(ArrayType& array)
{
  GenerateCollisionLookups(OverlapVsShape, array);
}

template <typename ShapeType, typename ArrayType>
void GenerateArrayComplexOverlapLookup(ArrayType& array)
{
  GenerateComplexCollisionLookups(ComplexOverlapVsShape, array);
}

//generate the cast vs shape array
template <typename ShapeType, typename ArrayType>
void GenerateArrayCastLookup(ArrayType& array)
{
  GenerateCollisionLookups(CastVsShape, array);
}

//macro to set up the function pointers for the simple collider types in a table.
#define GenerateCollisionTables(FunctionName, table)           \
  for(uint i = 0; i < (uint)Collider::cSize; ++i)              \
    for(uint j = 0; j < (uint)Collider::cSize; ++j)            \
      table[i][j] = nullptr;                                   \
  FunctionName<Obb>(table[Collider::cBox]);                    \
  FunctionName<Capsule>(table[Collider::cCapsule]);            \
  FunctionName<Cylinder>(table[Collider::cCylinder]);          \
  FunctionName<Ellipsoid>(table[Collider::cEllipsoid]);        \
  FunctionName<Sphere>(table[Collider::cSphere]);              \
  FunctionName<ConvexMeshShape>(table[Collider::cConvexMesh]);

#define GenerateComplexCollisionTables(FunctionName, table)                 \
  FunctionName<MultiConvexMeshCollider>(table[Collider::cMultiConvexMesh]); \
  FunctionName<MeshCollider>(table[Collider::cMesh]);                       \
  /*FunctionName<HeightMapCollider>(table[Collider::cHeightMap]);*/             \

//generate the collision table for checking collision between two colliders
template <typename TableType>
void GenerateColliderTableLookup(TableType& table)
{
  GenerateCollisionTables(GenerateColliderArrayLookup, table);
}

//generate the collision table for checking overlap between two colliders
template <typename TableType>
void GenerateTableOverlapLookup(TableType& table)
{
  GenerateCollisionTables(GenerateOverlapCollidersArrayLookup, table);
}

template <typename TableType>
void GenerateTableComplexOverlapLookup(TableType& table)
{
  GenerateComplexCollisionTables(GenerateOverlapComplexCollidersArrayLookup, table);
}

//the proper thing to do so we never get #define clashes
#undef GenerateCollisionTables
#undef GenerateCollisionLookups

//-------------------------------------------------------------------LookupStructs

/// A 1-d lookup table for collision between a shape and a colliders. Uses the
/// collider type to index into an array of function pointers to
/// resolve what kind of shape the colliders is.
template <typename ShapeType>
struct ShapeArrayLookup
{
  typedef bool (*LookupFunc)(ShapeType&,Collider*, Collider*, Physics::Manifold*);

  ShapeArrayLookup()
  {
    GenerateArrayLookup<ShapeType>(mLookups);
  }

  void OverrideLookup(LookupFunc overrideFunc, uint colliderIndex)
  {
    ReturnIf(colliderIndex >= Collider::cInvalid, /*void*/,
      "Override index is invalid. Please use "
      "a value in the ColliderType enum in collider.");

    mLookups[colliderIndex] = overrideFunc;
  }

  bool Collide(ShapeType& shape, Collider* shapeCollider, Collider* collider,
               Physics::Manifold* manifold)
  {
    uint type = collider->GetColliderType();
    ReturnIf((type >= Collider::cInvalid), false, 
      "Object 1 is of an invalid type. Cannot check collision with it");

    ReturnIf(mLookups[type] == nullptr, false, "This lookup type has not been implemented.");

    return mLookups[type](shape, shapeCollider, collider, manifold);
  }

  LookupFunc mLookups[Collider::cSize];
};

/// A 2-d lookup table for collision between to colliders. Uses the
/// collider type to index into a table of function pointers to
/// resolve what kind of shapes the colliders are.
struct CollisionTableLookup
{
  typedef bool(*LookupFunc)(Collider*, Collider*, PodArray<Physics::Manifold>*);

  CollisionTableLookup()
  {
    GenerateColliderTableLookup(mLookups);
  }

  void OverrideLookup(LookupFunc overrideFunc, uint colliderIndex1, uint colliderIndex2)
  {
    if(colliderIndex1 >= Collider::cInvalid || colliderIndex2 >= Collider::cInvalid)
    {
      Error("Override index is invalid. Please use "
        "a value in the ColliderType enum in collider.");
      return;
    }

    mLookups[colliderIndex1][colliderIndex2] = overrideFunc;
  }

  void OverrideMprDefaults(uint mprColliderType)
  {
    //looping through convex mesh will set all of the simple types
    for(uint i = 0; i < (uint)Collider::cConvexMesh; ++i)
    {
      OverrideLookup(CollideMprColliders, mprColliderType, i);
      OverrideLookup(CollideMprColliders, i, mprColliderType);
    }
  }

  template <typename ColliderType>
  void OverrideComplexDefaults(uint complexColliderType)
  {
    //set the override for all of the simple types
    OverrideLookup(ComplexCollideCollidersA<ColliderType, Obb>, complexColliderType, Collider::cBox);
    OverrideLookup(ComplexCollideCollidersB<Obb, ColliderType>, Collider::cBox, complexColliderType);

    OverrideLookup(ComplexCollideCollidersA<ColliderType, Capsule>, complexColliderType, Collider::cCapsule);
    OverrideLookup(ComplexCollideCollidersB<Capsule, ColliderType>, Collider::cCapsule, complexColliderType);

    OverrideLookup(ComplexCollideCollidersA<ColliderType, Cylinder>, complexColliderType, Collider::cCylinder);
    OverrideLookup(ComplexCollideCollidersB<Cylinder, ColliderType>, Collider::cCylinder, complexColliderType);

    OverrideLookup(ComplexCollideCollidersA<ColliderType, Ellipsoid>, complexColliderType, Collider::cEllipsoid);
    OverrideLookup(ComplexCollideCollidersB<Ellipsoid, ColliderType>, Collider::cEllipsoid, complexColliderType);

    OverrideLookup(ComplexCollideCollidersA<ColliderType, Sphere>, complexColliderType, Collider::cSphere);
    OverrideLookup(ComplexCollideCollidersB<Sphere, ColliderType>, Collider::cSphere, complexColliderType);

    OverrideLookup(ComplexCollideCollidersA<ColliderType, ConvexMeshShape>, complexColliderType, Collider::cConvexMesh);
    OverrideLookup(ComplexCollideCollidersB<ConvexMeshShape, ColliderType>, Collider::cConvexMesh, complexColliderType);
  }

  template <typename ColliderType0, typename ColliderType1>
  void OverrideComplexComplexPair(uint type0, uint type1)
  {
    OverrideLookup(ComplexVsComplexColliders<ColliderType0, ColliderType1>, type0, type1);
    OverrideLookup(ComplexVsComplexColliders<ColliderType1, ColliderType0>, type1, type0);
  }

  template <typename ComplexColliderType>
  void OverrideComplexComplexDefaults(uint complexColliderType)
  {
    OverrideComplexComplexPair<ComplexColliderType, MultiConvexMeshCollider>(complexColliderType, Collider::cMultiConvexMesh);
    OverrideComplexComplexPair<ComplexColliderType, MeshCollider>(complexColliderType, Collider::cMesh);
    OverrideComplexComplexPair<ComplexColliderType, HeightMapCollider>(complexColliderType, Collider::cHeightMap);
  }

  bool Collide(Collider* collider1, Collider* collider2, PodArray<Physics::Manifold>* manifolds)
  {
    uint type1 = collider1->GetColliderType();
    uint type2 = collider2->GetColliderType();

    ReturnIf((type1 >= Collider::cInvalid), false, 
      "Object 1 is of an invalid type. Cannot check collision with it");
    ReturnIf((type2 >= Collider::cInvalid), false, 
      "Object 2 is of an invalid type. Cannot check collision with it");

    ReturnIf(mLookups[type1][type2] == nullptr, false, "This pair type has not been implemented.");

    return mLookups[type1][type2](collider1, collider2, manifolds);
  }

  LookupFunc mLookups[Collider::cSize][Collider::cSize];
};

/// A 2-d lookup table for overlap between to colliders. Uses the
/// collider type to index into a table of function pointers to
/// resolve what kind of shapes the colliders are.
struct OverlapTableLookup
{
  typedef bool(*LookupFunc)(Collider*,Collider*);

  OverlapTableLookup()
  {
    GenerateTableOverlapLookup(mLookups);
    GenerateTableComplexOverlapLookup(mLookups);
  }

  void OverrideLookup(LookupFunc overrideFunc, uint colliderIndex1, uint colliderIndex2)
  {
    if(colliderIndex1 >= Collider::cInvalid || colliderIndex2 >= Collider::cInvalid)
    {
      Error("Override index is invalid. Please use "
        "a value in the ColliderType enum in collider.");
      return;
    }

    mLookups[colliderIndex1][colliderIndex2] = overrideFunc;
  }

  bool Collide(Collider* collider1, Collider* collider2)
  {
    uint type1 = collider1->GetColliderType();
    uint type2 = collider2->GetColliderType();

    ReturnIf((type1 >= Collider::cInvalid), false, 
      "Object 1 is of an invalid type. Cannot check collision with it");
    ReturnIf((type2 >= Collider::cInvalid), false, 
      "Object 2 is of an invalid type. Cannot check collision with it");

    ReturnIf(mLookups[type1][type2] == nullptr, false, "This pair type has not been implemented.");

    return mLookups[type1][type2](collider1, collider2);
  }

  LookupFunc mLookups[Collider::cSize][Collider::cSize];
};

/// A 1-d lookup array for boolean overlap between a shape and a colliders.
/// Uses the collider type to index into an array of function pointers to
/// resolve what kind of shape the colliders is.
template <typename ShapeType>
struct OverlapArrayLookup
{
  typedef bool (*LookupFunc)(const ShapeType&, Collider*);

  OverlapArrayLookup()
  {
    GenerateArrayOverlapLookup<ShapeType>(mLookups);
    GenerateArrayComplexOverlapLookup<ShapeType>(mLookups);
  }

  void OverrideLookup(LookupFunc overrideFunc, uint colliderIndex)
  {
    ReturnIf(colliderIndex >= Collider::cInvalid, /*void*/,
      "Override index is invalid. Please use "
      "a value in the ColliderType enum in collider.");

    mLookups[colliderIndex] = overrideFunc;
  }

  bool Collide(const ShapeType& shape, Collider* collider)
  {
    uint type = collider->GetColliderType();
    ReturnIf((type >= Collider::cInvalid), false, 
      "Object 1 is of an invalid type. Cannot check collision with it");

    ReturnIf(mLookups[type] == nullptr, false, "This lookup type has not been implemented.");

    return mLookups[type](shape, collider);
  }

  LookupFunc mLookups[Collider::cSize];
};

/// A 1-d lookup array for casts between a shape and a colliders. Uses the
/// collider type to index into an array of function pointers to
/// resolve what kind of shape the colliders is. The difference between this and
/// the shape array is that this takes a proxy and the cast filter.
template <typename CastType>
struct CastArrayLookup
{
  typedef bool (*LookupFunc)(const CastType&, Collider*, ProxyResult*, BaseCastFilter&);

  CastArrayLookup()
  {
    GenerateArrayCastLookup<CastType>(mLookups);
  }

  void OverrideLookup(LookupFunc overrideFunc, uint colliderIndex)
  {
    ReturnIf(colliderIndex >= Collider::cInvalid, /*void*/,
      "Override index is invalid. Please use "
      "a value in the ColliderType enum in collider.");

    mLookups[colliderIndex] = overrideFunc;
  }

  bool Cast(const CastType& castShape, Collider* collider, ProxyResult* result, BaseCastFilter& filter)
  {
    uint type = collider->GetColliderType();
    ReturnIf((type >= Collider::cInvalid), false, 
      "Object 1 is of an invalid type. Cannot check collision with it");

    ReturnIf(mLookups[type] == nullptr, false, "This lookup type has not been implemented.");

    return mLookups[type](castShape, collider, result, filter);
  }

  LookupFunc mLookups[Collider::cSize];
};

}//namespace Zero
