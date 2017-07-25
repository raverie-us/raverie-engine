///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

template <typename T>
struct PhysicsPair
{
  PhysicsPair(void);
  PhysicsPair(T a, T b){A = a; B = b;}

  T operator[](uint index) const {return mObjects[index];}

  union
  {
    struct{ T A, B; };
    struct{ T Top, Bot; };
    T mObjects[2];
  };
};

///Stores two Colliders. Prevents having to pass around both and also
///provides helper functions that are commonly used on two Colliders.
struct ColliderPair
{
  ColliderPair();
  ColliderPair(Collider* a, Collider* b);

  Vec3 GetPointSeperatingVelocity(Vec3Param point) const;

  real GetMixedRestiution() const;
  real GetMixedFriction() const;

  real GetMinRestitution() const;
  real GetMinStaticFriction() const;
  real GetMinDynamicFriction() const;

  u64 GetId() const;

  Collider* operator[](uint index) const;
  Collider*& operator[](uint index);

  bool operator>(const ColliderPair& rhs) const;

  union
  {
    struct{ Collider* A,* B; };
    struct{ Collider* Top,* Bot; };
    Collider* mObjects[2];
  };
};

}//namespace Physics

typedef Physics::ColliderPair ColliderPair;

}//namespace Zero
