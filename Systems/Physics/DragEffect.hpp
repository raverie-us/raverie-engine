///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Applies drag or damping forces to slow down an object's linear and angular velocity.
/// Drag is computed as a simple linear approximation of the drag force. Damping is a linear
/// approximation of a drag acceleration. This means that damping affects all
/// objects the same (mass independent).
class DragEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DragEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;

  // Physics Effect Interface
  void ApplyEffect(RigidBody* obj, real dt) override;

  //Properties

  /// Linear damping coefficient for applying a linear drag acceleration (accel = -bv).
  /// Note: this affects objects the same regardless of mass.
  real GetLinearDamping() const;
  void SetLinearDamping(real linearDamping);

  /// Angular damping coefficient for applying an angular drag acceleration (accel = -kw).
  /// Note: this affects objects the same regardless of mass.
  real GetAngularDamping() const;
  void SetAngularDamping(real angularDamping);

  /// The linear drag coefficient for applying a linear drag force (F = -bv).
  real GetLinearDrag() const;
  void SetLinearDrag(real linearDrag);

  /// The angular drag coefficient for applying an angular drag force (T = -kw).
  real GetAngularDrag();
  void SetAngularDrag(real angularDrag);

private:

  real mLinearDamping;
  real mAngularDamping;
  real mLinearDrag;
  real mAngularDrag;
};

}//namespace Zero
