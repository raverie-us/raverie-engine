///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleSystem.cpp
///
/// Authors: Chris Peters, Joshua Claeys, Nathan Carlson
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
int EmitParticles(ParticleSystem* main, ParticleEmitter* emitter,
                  ParticleList* particleList, float dt, Mat4Ref parentTransform, float timeAlive)
{
  int particlesEmitted = 0;

  if (emitter->GetOwner() != main->GetOwner())
  {
    Mat4 transform = parentTransform * emitter->mTransform->GetLocalMatrix();

    Vec3 newPosition = GetTranslationFrom(transform);
    Vec3 emitterVelocity = newPosition - emitter->mLastFramePosition;

    particlesEmitted = emitter->EmitParticles(particleList, dt, transform, emitterVelocity, timeAlive);
    emitter->mLastFramePosition = newPosition;
  }
  else
  {
    Vec3 newPosition = GetTranslationFrom(parentTransform);
    Vec3 emitterVelocity = newPosition - emitter->mLastFramePosition;

    particlesEmitted = emitter->EmitParticles(particleList, dt, parentTransform, emitterVelocity, timeAlive);
    emitter->mLastFramePosition = newPosition;
  }

  return particlesEmitted;
}

//******************************************************************************
void RunAnimator(ParticleSystem* main, ParticleAnimator* animator,
                 ParticleList* particleList, float dt, Mat4Ref parentTransform)
{
  // Could not find anywhere that ParticleAnimator::mTransform was being initialized, so I removed it
  //if(animator->GetOwner() != main->GetOwner())
  //{
  //  Mat4 transform = parentTransform * animator->mTransform->GetLocalMatrix();
  //  animator->Animate(particleList, dt, transform);
  //}
  //else
  {
    animator->Animate(particleList, dt, parentTransform);
  }
}

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(ParticlesSpawned);
}

//--------------------------------------------------------------- Particle Event
ZilchDefineType(ParticleEvent, builder, type)
{
  ZilchBindGetterProperty(NewParticles);
  ZilchBindGetterProperty(NewParticleCount);
}

//******************************************************************************
uint ParticleEvent::GetNewParticleCount()
{
  return mNewParticleCount;
}

//******************************************************************************
ParticleListRange ParticleEvent::GetNewParticles()
{
  return mNewParticles;
}

//-------------------------------------------------------------- Particle System
ZilchDefineType(ParticleSystem, builder, type)
{
  ZeroBindDocumented();
  ZeroBindInterface(Graphical);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(BoundingBoxSize);
  ZilchBindGetterSetterProperty(ChildSystem);
  ZilchBindFieldProperty(mSystemSpace);
  ZilchBindGetterSetterProperty(WarmUpTime);
  ZilchBindGetterSetterProperty(PreviewInEditor);

  ZilchBindMethod(AllParticles);
  ZilchBindMethod(Clear);

  ZeroBindTag(Tags::Particle);
  ZeroBindEvent(Events::ParticlesSpawned, ParticleEvent);
}

//******************************************************************************
void ParticleSystem::Serialize(Serializer& stream)
{
  Graphical::Serialize(stream);
  SerializeNameDefault(mBoundingBoxSize, 2.0f);
  SerializeNameDefault(mChildSystem, false);
  SerializeEnumNameDefault(SystemSpace, mSystemSpace, SystemSpace::WorldSpace);
  SerializeNameDefault(mWarmUpTime, 0.0f);
  SerializeNameDefault(mPreviewInEditor, false);
}

//******************************************************************************
void ParticleSystem::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);

  if (mChildSystem)
  {
    if (ParticleSystem* parentSystem = GetParentSystem())
      parentSystem->AddChildSystem(this);
  }

  mParticleList.Initialize();
  mTimeAlive = 0.0f;
  mDebugDrawing = false;

  if (Z::gRuntimeEditor)
  {
    Z::gRuntimeEditor->Visualize(this, "SpriteParticleSystem");
    ConnectThisTo(Z::gRuntimeEditor->GetActiveSelection(), Events::SelectionFinal, OnSelectionFinal);
  }

  if (mPreviewInEditor && GetSpace()->IsEditorMode())
    ConnectThisTo(GetSpace(), Events::FrameUpdate, OnUpdate);
  else
    ConnectThisTo(GetSpace(), Events::LogicUpdate, OnUpdate);
}

//******************************************************************************
void ParticleSystem::OnAllObjectsCreated(CogInitializer& initializer)
{
  // Use the engines dt
  float timeStep = Z::gEngine->has(TimeSystem)->GetTargetDt();

  float timeLeft = mWarmUpTime;
  while (timeLeft > 0.0f)
  {
    // Subtract the current time step
    timeLeft -= timeStep;

    // This can do an extra step, if timeLeft is negative at this point,
    // however the warm up isn't meant to be exact, so it should be fine
    SystemUpdate(timeStep);
  }
}

//******************************************************************************
void ParticleSystem::AttachTo(AttachmentInfo& info)
{
  if (mChildSystem && info.Child == GetOwner())
  {
    if (ParticleSystem* parentSystem = info.Parent->has(ParticleSystem))
      parentSystem->AddChildSystem(this);
  }

  Graphical::AttachTo(info);
}

//******************************************************************************
void ParticleSystem::Detached(AttachmentInfo& info)
{
  if (mChildSystem)
  {
    if (ParticleSystem* parentSystem = info.Parent->has(ParticleSystem))
      parentSystem->RemoveChildSystem(this);
  }

  Graphical::Detached(info);
}

//******************************************************************************
void ParticleSystem::OnDestroy(uint flags)
{
  if (mChildSystem)
  {
    if (ParticleSystem* parentSystem = GetParentSystem())
      parentSystem->RemoveChildSystem(this);
  }

  Clear();

  Graphical::OnDestroy(flags);
}

//******************************************************************************
void ParticleSystem::DebugDraw()
{
  bool editorMode = GetSpace()->IsEditorMode();

  // Don't update twice if we're in game or space isn't paused
  if (!editorMode || !GetSpace()->has(TimeSpace)->mPaused)
    return;

  // Don't update twice if we're in preview mode
  if (mPreviewInEditor && editorMode)
    return;

  mDebugDrawing = true;
  TimeSpace* timeSpace = GetSpace()->has(TimeSpace);
  SystemUpdate(timeSpace->mScaledClampedDt);
}

//******************************************************************************
Aabb ParticleSystem::GetLocalAabb()
{
  return Aabb(Vec3::cZero, Vec3(mBoundingBoxSize * 0.5f));
}

//******************************************************************************
float ParticleSystem::GetBoundingBoxSize()
{
  return mBoundingBoxSize;
}

//******************************************************************************
void ParticleSystem::SetBoundingBoxSize(float size)
{
  mBoundingBoxSize = Math::Max(size, 0.2f);
  UpdateBroadPhaseAabb();
}

//******************************************************************************
bool ParticleSystem::GetChildSystem()
{
  return mChildSystem;
}

//******************************************************************************
void ParticleSystem::SetChildSystem(bool state)
{
  if (ParticleSystem* parentSystem = GetParentSystem())
  {
    if (state && !mChildSystem)
      parentSystem->AddChildSystem(this);
    else if (!state && mChildSystem)
      parentSystem->RemoveChildSystem(this);
  }

  mChildSystem = state;
}

//******************************************************************************
float ParticleSystem::GetWarmUpTime()
{
  return mWarmUpTime;
}

//******************************************************************************
void ParticleSystem::SetWarmUpTime(float time)
{
  const float cMaxWarmUpTime = 20.0f;
  if (time > cMaxWarmUpTime)
  {
    DoNotifyWarning("WarmUpTime too high", "Max warm up time is 20 seconds. "
                    "Setting this too high can cause large stalls.");
    return;
  }
  mWarmUpTime = time;
}

//******************************************************************************
bool ParticleSystem::GetPreviewInEditor()
{
  return mPreviewInEditor;
}

//******************************************************************************
void ParticleSystem::SetPreviewInEditor(bool previewInEditor)
{
  if (previewInEditor == mPreviewInEditor)
    return;

  mPreviewInEditor = previewInEditor;

  bool inEditor = GetSpace()->IsEditorMode();
  if (!GetSpace()->IsEditorMode())
    return;

  if (mPreviewInEditor)
  {
    mDebugDrawing = false;
    ConnectThisTo(GetSpace(), Events::FrameUpdate, OnUpdate);
    GetSpace()->GetDispatcher()->DisconnectEvent(Events::LogicUpdate, this);
  }
  else
  {
    ConnectThisTo(GetSpace(), Events::LogicUpdate, OnUpdate);
    GetSpace()->GetDispatcher()->DisconnectEvent(Events::FrameUpdate, this);

    // If we're selected in the editor, it's being updated by DebugDraw(), so don't clear the particles
    if (!IsSelectedInEditor())
      Clear();
  }
}

//******************************************************************************
ParticleListRange ParticleSystem::AllParticles()
{
  return mParticleList.All();
}

//******************************************************************************
void ParticleSystem::Clear()
{
  mParticleList.FreeParticles();
}

//******************************************************************************
void ParticleSystem::OnUpdate(UpdateEvent* event)
{
  SystemUpdate(event->Dt);
}

//******************************************************************************
void ParticleSystem::SystemUpdate(float dt)
{
  // Our parent will update us if we're a child system
  if (mChildSystem == false)
    BaseUpdate(dt);

  UpdateLifetimes(dt);
  mParticleList.ClearDestroyed();
}

//******************************************************************************
uint ParticleSystem::BaseUpdate(float dt)
{
  if (mAnimators.Empty())
    return 0;

  mTimeAlive += dt;

  Mat4 worldTransform = Mat4::cIdentity;
  if (mSystemSpace == SystemSpace::WorldSpace)
    worldTransform = mTransform->GetWorldMatrix();

  // Emit Particles
  int emitCount = 0;
  Particle* oldFront = mParticleList.Particles;
  for (EmitterList::range r = mEmitters.All(); !r.Empty(); r.PopFront())
    emitCount += EmitParticles(this, &r.Front(), &mParticleList, dt, worldTransform, mTimeAlive);

  // Send out an event if particles were spawned
  if (emitCount > 0)
  {
    ParticleEvent eventToSend;
    eventToSend.mNewParticleCount = (uint)emitCount;
    eventToSend.mNewParticles = ParticleList::range(mParticleList.Particles, oldFront);
    GetOwner()->DispatchEvent(Events::ParticlesSpawned, &eventToSend);
  }

  // Run animators on all particles
  for (AnimatorList::range r = mAnimators.All(); !r.Empty(); r.PopFront())
    RunAnimator(this, &r.Front(), &mParticleList, dt, worldTransform);

  for (ParticleSystemList::range r = mChildSystems.All(); !r.Empty(); r.PopFront())
    r.Front().ChildUpdate(dt, &mParticleList, emitCount);

  return emitCount;
}

//******************************************************************************
void ParticleSystem::ChildUpdate(float dt, ParticleList* parentList, uint parentEmitCount)
{
  uint emitCount = 0;
  Mat4 worldTransform = mTransform->GetWorldMatrix();

  Particle* particle = parentList->Particles;

  while (particle != nullptr)
  {
    SetTranslationOn(&worldTransform, particle->Position);

    for (EmitterList::range r = mEmitters.All(); !r.Empty(); r.PopFront())
      emitCount += r.Front().EmitParticles(&mParticleList, dt, worldTransform, particle->Velocity, particle->Time);

    particle = particle->Next;
  }

  for (AnimatorList::range r = mAnimators.All(); !r.Empty(); r.PopFront())
    RunAnimator(this, &r.Front(), &mParticleList, dt, worldTransform);

  for (ParticleSystemList::range r = mChildSystems.All(); !r.Empty(); r.PopFront())
    r.Front().ChildUpdate(dt, &mParticleList, emitCount);

  mParticleList.ClearDestroyed();
}

//******************************************************************************
void ParticleSystem::UpdateLifetimes(float dt)
{
  // Begin particle update pass removing dead particles
  Particle* particle = mParticleList.Particles;
  Particle** prev = &mParticleList.Particles;

  while (particle != nullptr)
  {
    particle->Time += dt;

    if (particle->Time >= particle->Lifetime)
    {
      // Particle is dead remove from active list
      (*prev) = particle->Next;

      Particle* deadParticle = particle;

      // Move to next particle
      particle = particle->Next;

      mParticleList.DestroyParticle(deadParticle);

      if (mParticleList.Particles == nullptr)
      {
        ObjectEvent event(this);
        DispatchEvent(Events::AllParticlesDead, &event);
      }
    }
    else
    {
      // Set the prev to this particle
      prev = &particle->Next;

      // Move to next particle.
      particle = particle->Next;
    }
  }
}

//******************************************************************************
void ParticleSystem::AddEmitter(ParticleEmitter* emitter)
{
  mEmitters.PushBack(emitter);
}

//******************************************************************************
void ParticleSystem::AddAnimator(ParticleAnimator* animator)
{
  mAnimators.PushBack(animator);
}

//******************************************************************************
void ParticleSystem::AddChildSystem(ParticleSystem* child)
{
  mChildSystems.PushBack(child);
}

//******************************************************************************
void ParticleSystem::RemoveChildSystem(ParticleSystem* child)
{
  mChildSystems.Erase(child);
}

//******************************************************************************
ParticleSystem* ParticleSystem::GetParentSystem()
{
  if (Cog* parent = GetOwner()->GetParent())
    return parent->has(ParticleSystem);
  return nullptr;
}

//******************************************************************************
void ParticleSystem::OnSelectionFinal(SelectionChangedEvent* selectionEvent)
{
  if (mDebugDrawing && !IsSelectedInEditor())
  {
    mDebugDrawing = false;
    Clear();
    forRange (ParticleEmitter& emitter, mEmitters.All())
      emitter.ResetCount();
  }
}

//******************************************************************************
bool ParticleSystem::IsSelectedInEditor()
{
  return Z::gRuntimeEditor->GetActiveSelection()->Contains(GetOwner());
}

} // namespace Zero
