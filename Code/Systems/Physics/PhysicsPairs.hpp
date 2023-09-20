// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Physics
{

template <typename T>
struct PhysicsPair
{
  PhysicsPair(void);
  PhysicsPair(T a, T b)
  {
    A = a;
    B = b;
  }

  T operator[](uint index) const
  {
    return mObjects[index];
  }

  union
  {
    struct
    {
      T A, B;
    };
    struct
    {
      T Top, Bot;
    };
    T mObjects[2];
  };
};

/// Stores two Colliders. Prevents having to pass around both and also
/// provides helper functions that are commonly used on two Colliders.
struct ColliderPair
{
  ColliderPair();
  ColliderPair(Collider* a, Collider* b);

  Vec3 GetPointSeperatingVelocity(Vec3Param point) const;

  real GetMixedRestitution() const;
  real GetMixedFriction() const;

  u64 GetId() const;

  Collider* operator[](uint index) const;
  Collider*& operator[](uint index);

  bool operator>(const ColliderPair& rhs) const;

  union
  {
    struct
    {
      Collider *A, *B;
    };
    struct
    {
      Collider *Top, *Bot;
    };
    Collider* mObjects[2];
  };
};

} // namespace Physics

typedef Physics::ColliderPair ColliderPair;

} // namespace Raverie
