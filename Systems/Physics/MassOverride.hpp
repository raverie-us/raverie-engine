///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField4(MassOverrideStates, Active, AutoComputeCenterOfMass, AutoComputeInertia, Serialized);

/// Takes a snap shot of the current mass and inertia and overrides
/// the object's mass so it can be resized while keeping it's old mass.
class MassOverride : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MassOverride();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  bool IsSerialized();

  //-------------------------------------------------------------------Properties
  /// Determines whether the RigidBody on this Cog will use
  /// the cached or actual mass and inertia.
  bool GetActive();
  void SetActive(bool state);
  /// The cached inverse mass of this object.
  real GetInverseMass();
  void SetInverseMass(real invMass);
  void SetInverseMassInternal(real invMass);
  /// Overrides the mass of this body. Inertia is updated as
  /// a ratio of the new mass to the old mass.
  real GetMass();
  void SetMass(real mass);
  void SetMassInternal(real mass);
  /// The inverse inertia tensor this object is saved with.
  Mat3 GetLocalInverseInertiaTensor();
  void SetLocalInverseInertiaTensor(Mat3Param invInertia);
  void SetLocalInverseInertiaTensorInternal(Mat3Param invInertia);
  /// The center of mass in local space to override with. When set, the center
  /// of mass will be locked to this value until AutoComputeCenterOfMass is set to true.
  Vec3 GetLocalCenterOfMass();
  void SetLocalCenterOfMass(Vec3Param localCenterOfMass);
  /// Should the center of mass be auto computed or overwritten (via script).
  bool GetAutoComputeCenterOfMass();
  void SetAutoComputeCenterOfMass(bool autoCompute);
  /// Should the inertia tensor be auto computed or overwritten (via script).
  bool GetAutoComputeInertia();
  void SetAutoComputeInertia(bool autoCompute);

  //-------------------------------------------------------------------Internal
  /// Takes a new snapshot of the current mass and inertia.
  void RecomputeMass();
  real ClampMassTerm(real value);
  /// Given a new inverse mass, this updates the mass and inertia (inertia as a ratio of old to new mass)
  void UpdateMassAndInertia(real invMass);
  /// If possible, this queues an update on the rigid body to recompute mass properties
  void QueueUpdate();

  Physics::Mass mMassOverride;
  Physics::Inertia mInertiaOverride;
  Vec3 mLocalCenterOfMass;

  BitField<MassOverrideStates::Enum> mFlags;
};

}//namespace Zero
