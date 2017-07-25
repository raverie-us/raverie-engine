///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleEmitters.hpp
/// Declaration of the Particle emitters.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------ Spline Particle Emitter
class SplineParticleEmitter : public ParticleEmitterShared
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnTargetSplineCogPathChanged(Event* e);
  void FindSpline();

  /// ParticleEmitter Interface.
  int EmitParticles(ParticleList* particleList, float dt,
                    Mat4Ref transform, Vec3Param velocity, float timeAlive) override;

  /// The current spline being emitted along
  Spline* GetSpline() const;
  void SetSpline(Spline* spline);

  /// Path to an object to query for a spline
  CogPath mTargetSplineCog;
  /// Particles will be created along this spline
  HandleOf<Spline> mSpline;

  float mEmitRadius;
  float mSpawnT, mSpawnTVariance;
  bool mClampT;
};

//----------------------------------------------------- Spline Particle Animator
DeclareEnum2(SplineAnimatorMode, Exact, Spring);

class SplineParticleAnimator : public ParticleAnimator
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ~SplineParticleAnimator();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// ParticleAnimator Interface.
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;

  /// Speed setter / getter.
  void SetSpeed(float speed);
  float GetSpeed();

  /// 
  SplineParticleEmitter* mEmitter;

  /// The speed at which the particles move in meters / second.
  float mSpeed;

  /// If checked, the lifetime on the SplineParticleEmitter will be updated
  /// to the time it would take to travel the entire path at the current speed.
  bool mAutoCalculateLifetime;

  bool mHelix;

  /// The radius of the helix.
  float mHelixRadius;

  /// How fast the helix rotates in radians / second.
  float mHelixWaveLength;

  /// Offset in radians for where the helix starts.
  float mHelixOffset;

  /// The current animate mode.
  SplineAnimatorMode::Enum mMode;

  /// Spring properties.
  float mSpringFrequencyHz;
  float mSpringDampingRatio;
};

}//namespace Zero
