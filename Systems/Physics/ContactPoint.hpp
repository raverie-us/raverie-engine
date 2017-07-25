///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{
  struct Manifold;
  struct ManifoldPoint;
}//namespace Physics

/// Information about one point of contact in a collision. This is useful for
/// evaluating info about the collision after it happened such as where the objects hit.
/// WARNING: Do not hold onto this after an event is sent out.
struct ContactPoint
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ContactPoint();

  void Set(const Physics::Manifold* manifold, const Physics::ManifoldPoint* manifoldPoint, uint objectIndex);

  /// The point in local space of myself in this collision.
  Vec3 GetLocalPoint();
  /// The point in local space of the other object in this collision.
  Vec3 GetOtherLocalPoint();
  /// The point in world space of this contact point.
  Vec3 GetWorldPoint();
  /// The world space normal that points from myself towards the other object.
  Vec3 GetWorldNormalTowardsOther();
  /// The total impulse that this object applied in the direction of the normal.
  real GetNormalImpulse();
  /// The total friction impulse that this object applied.
  real GetFrictionImpulse();
  /// The total impulse this object applied (only for more complicated logic).
  /// The impulse is a Vector3 of the values (normal, friction1, friction2).
  Vec3 GetComplexImpulse();
  /// The penetration of this contact point in the direction of the normal.
  /// Note: penetration is always positive and is not flipped for object A or object B.
  real GetPenetration();
  /// The relative velocity of this point in the direction of the normal.
  /// The relative point velocity is defined as Dot(p1 - p0, n) where p1 and p0
  /// are the velocities of the contact points in the collision of myself and the
  /// other object respectively. This value can be used to see how fast the objects
  /// are now separating. Also, in pre-collision this value can be used to approximate
  /// the impulse of the collision since the impulse values will not have
  /// been calculated yet in pre-collision.
  real GetRelativeVelocity();

  /// The manifold point that this is wrapping.
  const Physics::ManifoldPoint* mManifoldPoint;
  /// The manifold that this point is from. Used mainly to
  /// access the two objects in the collision.
  const Physics::Manifold* mManifold;
  /// The object in the manifold that is currently the "active" object.
  /// When this event is sent out to object A this will be 0 and everything will
  /// be normal. However when this is sent out to object B most values will be
  /// flipped (such as point B becomes point A) due to this.
  uint mObjectIndex;
};

/// A range to allow iteration over all of the points in a contact. This
/// returns a sub-struct containing info for each contact point.
struct ContactPointRange
{
  typedef ContactPointRange self_type;
  typedef ContactPoint value_type;
  typedef ContactPoint FrontResult;

  ContactPointRange();

  void Set(const Physics::Manifold* manifold, uint objectIndex);

  // Range interface
  FrontResult Front();
  void PopFront();
  bool Empty();

  uint mPointIndex;
  const Physics::Manifold* mManifold;
  ContactPoint mPoint;
};

}//namespace Zero
