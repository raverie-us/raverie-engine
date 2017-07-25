///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleEmitter.hpp
/// Declaration of the Particle emitter component classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(ParticlesExhausted);
  DeclareEvent(AllParticlesDead);
}

/// Particle emitters add new particles to the system and control where
/// the particles will appear (sphere emitter, mesh emitter, etc)
class ParticleEmitter : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ParticleEmitter();
  ~ParticleEmitter();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;

  // Particle Emitter Interface

  // Emit new particles
  virtual int EmitParticles(ParticleList* particleList, float dt, 
                            Mat4Ref transform, Vec3Param velocity, float timeAlive) = 0;
  
  // Reset the number of particles to emit back to EmitCount.
  virtual void ResetCount() {};

  void UpdateLastTranslation();

  Link<ParticleEmitter> link;
  Vec3 mLastFramePosition;
  Transform* mTransform;
  GraphicsSpace* mGraphicsSpace;
};

typedef InList<ParticleEmitter> EmitterList;

//------------------------------------------------- Particle Emitter Common Data
/// Particle Emitter Shared
class ParticleEmitterShared : public ParticleEmitter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ParticleEmitterShared();
  ~ParticleEmitterShared();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //Mix in Helpers
  int GetParticleEmissionCount(ParticleList* particleList, float dt, float timeAlive);
  Particle* CreateInitializedParticle(ParticleList* particleList, int particle, 
                                      Mat4Ref transform, Vec3Param emitterVelocity);

  /// Reset the number of particles to emit back to EmitCount.
  void ResetCount() override;

  /// Rate that particles spawn per second.
  float GetEmitRate();
  void SetEmitRate(float emitRate);

  /// Is this emitter currently emitting particles?
  bool mActive;

  /// Number of particles to emit per reset.
  uint mEmitCount;

  /// Current number of particles left to emit.
  int mCurrentCount;

  /// Time in seconds to delay the emission of particles from time of creation.
  float mEmitDelay;

  /// Rate that particles spawn per second.
  float mEmitRate;

  /// How much the emit can vary per sample
  float mEmitVariance;

  /// Slowly ramps up to EmitRate over this time.
  float mEmitRateSoftStartTime;

  /// Size of each particle spawned.
  float mSize;

  /// How much the emit can vary per sample.
  float mSizeVariance;

  /// How a particle's starting lifetime is.
  float mLifetime;

  /// Hom much lifetime can vary per particle.
  float mLifetimeVariance;

  /// Speed in rads per second of the particle.
  float mSpin;
  /// How much spin speed can vary per particle.
  float mSpinVariance;

  /// Each particle should start with random spin.
  bool mRandomSpin;

  /// Velocity of each particle at start
  Vec3 mStartVelocity;

  /// Velocity of each particle in x horizontal tangent y vertical tangent and z outward tangent.
  Vec3 mTangentVelocity;
  
  /// Random Velocity per particle.
  Vec3 mRandomVelocity;

  /// Size of the emitter
  Vec3 mEmitterSize;

  /// How much area of the emitter to used 0 to 1.
  float mFill;

  /// How much of the objects velocity is added to the particles
  float mEmitterVelocityPercent;

  /// Whether or not we attempt to emit along the vector between
  /// the previous position to the current position, which looks
  /// better for fast moving particle systems
  /// Note: Particle systems that teleport will emit along the teleport line
  bool mFastMovingEmitter;

  //Internals
  //Not Read only
  float mAccumulation;
  float mSample;
  float mEmitRateCurrent;

  static const float mMaxEmitRate;
};

//--------------------------------------------------------------------------------- Hide Base Filter
// This is used to hide properties on ParticleEmitterShared that don't apply to derived
// particle emitters. This is temporary and should be removed once we refactor particles.
class HideBaseFilter : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  HideBaseFilter(BoundType* hiddenOnType);

  bool Filter(Property* prop, HandleParam instance) override;

  // Do not show the property on this type
  BoundType* mHiddenOnType;
};

#define HideOnDerivedType(type) Add(new HideBaseFilter(ZilchTypeId(type)))

} // namespace Zero
