// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Physics
{

class CollisionManager;

} // namespace Physics

class PhysicsSpace;
System* CreatePhysicsSystem();

/// Manages all the active Physics spaces.  When the space component is created,
/// it is added to a list and updated by this system.
class PhysicsEngine : public System
{
public:
  RaverieDeclareType(PhysicsEngine, TypeCopyMode::ReferenceType);

  PhysicsEngine(void);
  virtual ~PhysicsEngine(void);

  // System Interface
  cstr GetName() override;
  void Initialize(SystemInitializer& initializer) override;
  void Update(bool debugger) override;

  typedef InList<PhysicsSpace, &PhysicsSpace::EngineLink> SpaceList;
  SpaceList::range GetSpaces();

  BroadPhaseLibrary mBroadPhaseLibrary;

private:
  friend class PhysicsSpace;

  /// Send out events for all objects that have moved in all spaces.
  void Publish();

  /// Track current PhysicsSpaces.
  void AddSpace(PhysicsSpace* space);
  void RemoveSpace(PhysicsSpace* space);

  Physics::CollisionManager* mCollisionManager;

  /// All active spaces
  SpaceList mSpaces;

public:
  Memory::Heap* mHeap;
};

} // namespace Raverie
