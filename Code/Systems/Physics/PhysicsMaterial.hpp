// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Describes material properties of a collider mainly used during collision
/// resolution.
class PhysicsMaterial : public DataResource
{
public:
  RaverieDeclareType(PhysicsMaterial, TypeCopyMode::ReferenceType);

  PhysicsMaterial();

  // Interface
  void Serialize(Serializer& stream) override;
  void ResourceModified() override;

  /// Creates a PhysicsMaterial for run-time modifications.
  static HandleOf<PhysicsMaterial> CreateRuntime();
  HandleOf<Resource> Clone() override;
  /// Creates a clone of this material for run-time modifications.
  HandleOf<PhysicsMaterial> RuntimeClone();

  /// How slippery or rough the object is. When friction is 0 the object will be
  /// slippery. As friction increases, sliding objects will stop quicker.
  /// The friction of two object's are combined with the formula sqrt(a * b).
  real GetFriction();
  void SetFriction(real friction);

  /// Density is used to determine the mass of an object. Mass is computed as
  /// density * volume. Density can be set to exactly 0 to produce a massless
  /// object, however this should only be done with children objects to add
  /// collision without affecting mass.
  real GetDensity();
  void SetDensity(real density);

  /// Deprecated. Use RestitutionImportance instead.
  bool GetHighPriority();
  void SetHighPriority(bool state);

  void CopyTo(PhysicsMaterial* destination);
  /// After modifying this resource, notify anyone using it to update
  /// now instead of at the next physics update.
  void UpdateAndNotifyIfModified();
  void SetDensityInternal(real density, bool markModified = true);

  /// How much an object will bounce during a collision. Values should be in the
  /// range [0,1] where 0 is an inelastic collision and 1 is a fully elastic
  /// collision. Restitution is computed as the max of the two objects. Note:
  /// due to solving constraints with baumgarte, energy will not be perfectly
  /// conserved with a restitution 1.
  real mRestitution;
  real mStaticFriction;
  real mDynamicFriction;
  real mDensity;

  /// Determines which object's restitution should be used. If object A's
  /// restitution importance value is larger than object B's then object A's
  /// restitution will be used. If both importance values are the same then the
  /// default combination logic will be used (see the description of Restitution
  /// for details).
  real mRestitutionImportance;
  /// Determines which object's friction should be used. If object A's friction
  /// importance value is larger than object B's then object A's friction will
  /// be used. If both importance values are the same then the default
  /// combination logic will be used (see the description of Friction for
  /// details).
  real mFrictionImportance;

  bool mModified;
};

// PhysicsMaterialManager
class PhysicsMaterialManager : public ResourceManager
{
public:
  DeclareResourceManager(PhysicsMaterialManager, PhysicsMaterial);
  PhysicsMaterialManager(BoundType* resourceType);

  void UpdateAndNotifyModifiedResources();

  typedef HandleOf<PhysicsMaterial> PhysicsMaterialReference;
  Array<PhysicsMaterialReference> mModifiedResources;
};

} // namespace Raverie
