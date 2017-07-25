///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleSystem.hpp
///
/// Authors: Chris Peters, Joshua Claeys, Nathan Carlson
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(ParticlesSpawned);
}

/// Event for getting a list of newly created particles.
class ParticleEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ParticleEvent(){}

  uint GetNewParticleCount();
  uint mNewParticleCount;

  ParticleListRange GetNewParticles();
  ParticleListRange mNewParticles;
};

DeclareEnum2(SystemSpace, WorldSpace, LocalSpace);

/// An interface for generating and managing particles of a generic definition using emitters and animators.
class ParticleSystem : public Graphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  void OnDestroy(uint flags = 0) override;
  void DebugDraw() override;

  // Graphical Interface

  Aabb GetLocalAabb() override;

  // Properties

  /// Size of all sides of the bounding box used for frustum culling.
  float GetBoundingBoxSize();
  void SetBoundingBoxSize(float size);
  float mBoundingBoxSize;

  /// If set, particle emission will happen for each particle in a parent system.
  bool GetChildSystem();
  void SetChildSystem(bool state);
  bool mChildSystem;

  /// If particles are emitted into world space or if transform data remains relative to the transform of the system object.
  SystemSpace::Enum mSystemSpace;

  /// The amount of time to simulate the particle system on startup. This will
  /// be done on AllObjectsCreated, and will use the engines dt. This is good
  /// for when you want the particle effect to be in full bloom when you first
  /// see it. However, it can hurt performance at high values on startup.
  float GetWarmUpTime();
  void SetWarmUpTime(float time);
  float mWarmUpTime;

  /// If the particle system should run on frame update in the editor instead of logic update.
  bool GetPreviewInEditor();
  void SetPreviewInEditor(bool state);
  bool mPreviewInEditor;

  /// A list of all particles currently active in the system.
  ParticleListRange AllParticles();

  /// Clear all current particles.
  void Clear();

  // Internal

  Link<ParticleSystem> SystemLink;
  typedef InList<ParticleSystem, &ParticleSystem::SystemLink> ParticleSystemList;

  void OnUpdate(UpdateEvent* event);

  void SystemUpdate(float dt);
  uint BaseUpdate(float dt);
  void ChildUpdate(float dt, ParticleList* parentList, uint emitCount);
  void UpdateLifetimes(float dt);

  void AddEmitter(ParticleEmitter* emitter);
  void AddAnimator(ParticleAnimator* animator);
  void AddChildSystem(ParticleSystem* child);
  void RemoveChildSystem(ParticleSystem* child);
  ParticleSystem* GetParentSystem();

  void OnSelectionFinal(SelectionChangedEvent* selectionEvent);
  bool IsSelectedInEditor();

  // Particles on this system.
  ParticleList mParticleList;
  // All animators affecting this system.
  AnimatorList mAnimators;
  // All emitters affecting this system.
  EmitterList mEmitters;
  // Child Particle Systems.
  ParticleSystemList mChildSystems;
  // Amount of time that the system has been updating particles.
  float mTimeAlive;
  // Flag for resetting particles when selection changes.
  bool mDebugDrawing;
};

} // namespace Zero
