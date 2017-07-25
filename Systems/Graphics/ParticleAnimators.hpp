///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------- Linear Particle Animator
///Basic Particle Animation Effects
class LinearParticleAnimator : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  LinearParticleAnimator();
  ~LinearParticleAnimator();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //ParticleAnimator Interface 
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;

private:

  /// Constance force applied to particles.
  Vec3 mForce;

  /// Random force applied to particles
  Vec3 mRandomForce;

  /// Force that applies spin.
  float mTorque;

  /// Velocity dampening
  float mDampening;

  /// Rate of particle size growth.
  float mGrowth;

  /// Twist applies a twisting/tornado force to the particles.
  Vec3 mTwist;
};

//-------------------------------------------------------------- Particle Wander
///Particle animator that causes particle to wander
/// or smoothly vary directions
class ParticleWander : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ParticleWander();
  ~ParticleWander();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //ParticleAnimator Interface 
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;
private:
  float mWanderAngle;
  float mWanderAngleVariance;
  float mWanderStrength;
};

//--------------------------------------------------- Particle Color Animator
/// Linear interpolate colors across the particles lifetime.
class ParticleColorAnimator : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ~ParticleColorAnimator();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //ParticleAnimator Interface 
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;

private:
  friend class LinearParticleAnimator;

  HandleOf<ColorGradient> mTimeGradient;
  HandleOf<ColorGradient> mVelocityGradient;
  float mMaxParticleSpeed;
};

//----------------------------------------------------------- Particle Attractor
class ParticleAttractor : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ~ParticleAttractor();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  // ParticleAnimator Interface 
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;

private:
  SystemSpace::Enum mPositionSpace;
  Vec3 mAttractPosition;

  float mStrength;
  float mMaxDistance;
  float mMinDistance;
};

//------------------------------------------------------------- Particle Twister
class ParticleTwister : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ParticleTwister();
  ~ParticleTwister();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  //ParticleAnimator Interface
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform)  override;

private:
  Vec3 mAxis;
  float mStrength;
  float mMaxDistance;
  float mMinDistance;
};

//----------------------------------------------------- Particle Collision Plane
class ParticleCollisionPlane : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ~ParticleCollisionPlane();

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// ParticleAnimator Interface.
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;

  /// How much the particle will bounce during a collision. Values should be in the range
  /// of [0, 1], where 0 is an in-elastic collision and 1 is a fully elastic collision (bouncy).
  /// If the value is greater than 1, the particle will gain energy and move
  /// faster after the bounce.
  float GetRestitution();
  void SetRestitution(float restitution);

  /// How slippery or rough the particle is. When friction is 0, the object will be slippery.
  /// When friction is 1, it will completely stop in the direction tangential to the
  /// collision normal. Values should be in the range [0, 1].
  float GetFriction();
  void SetFriction(float friction);

private:
  SystemSpace::Enum mPlaneSpace;
  Vec3 mPlanePosition;
  Vec3 mPlaneNormal;
  float mRestitution;
  float mFriction;
};

//----------------------------------------------------- Particle Collision Heightmap
class ParticleCollisionHeightmap : public ParticleAnimator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ~ParticleCollisionHeightmap();

  //Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;

  //ParticleAnimator Interface 
  void Animate(ParticleList* particleList, float dt, Mat4Ref transform) override;

  /// How much the particle will bounce during a collision. Values should be in the range
  /// of [0, 1], where 0 is an in-elastic collision and 1 is a fully elastic collision (bouncy).
  /// If the value is greater than 1, the particle will gain energy and move
  /// faster after the bounce.
  float GetRestitution();
  void SetRestitution(float restitution);

  /// How slippery or rough the particle is. When friction is 0, the object will be slippery.
  /// When friction is 1, it will completely stop in the direction tangential to the
  /// collision normal.
  float GetFriction();
  void SetFriction(float friction);

private:
  CogPath mHeightMap;
  float mRestitution;
  float mFriction;
};

}//namespace Zero
