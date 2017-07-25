///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Regions are used to make PhysicsEffects affect a region of space. Any effects attached to a
/// Cog with a Region will apply to all objects in contact with this region.
class Region : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Region();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags /* = 0 */) override;

  /// Dispatches an event to all objects in this region.
  void DispatchEvent(StringParam eventId, Event* toSend);

  void AddEffect(PhysicsEffect* effect);
  void RemoveEffect(PhysicsEffect* effect);

  /// Applies effects to all of the objects in the region.
  void Update(real dt);
  /// Helper to apply effects to a body
  void ApplyEffects(RigidBody* body, real dt);
  /// Wakes up all bodies in contact with this region (so effects will take affect)
  void WakeUpAll();

  /// A range to wrap what this region is in contact with.
  class RegionContactRange
  {
  public:
    explicit RegionContactRange(const ContactRange& range);

    Collider* Front();
    void PopFront();
    bool Empty();

  private:
    ContactRange mRange;
  };

  RegionContactRange All();

  Link<Region> SpaceLink;

private:

  PhysicsSpace* mSpace;
  Collider* mCollider;

  /// Determines if all objects in the region should be woken up when an
  /// effect is changed. Used to make sure that changes in effects will
  /// be applied to an object event if it is asleep.
  bool mWakeUpOnEffectChange;

  PhysicsEffectList mEffects;
};

typedef InList<Region, &Region::SpaceLink>         RegionList;

}//namespace Zero
