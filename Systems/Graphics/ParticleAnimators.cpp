///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleAnimators.cpp
/// Implementation of the Particle animators.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------- Linear Particle Animator
ZilchDefineType(LinearParticleAnimator, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(Transform);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mForce);
  ZilchBindFieldProperty(mRandomForce);

  ZilchBindFieldProperty(mTorque);
  ZilchBindFieldProperty(mGrowth);
  ZilchBindFieldProperty(mDampening);
  ZilchBindFieldProperty(mTwist);
}

LinearParticleAnimator::LinearParticleAnimator()
{
}

void LinearParticleAnimator::Serialize(Serializer& stream)
{
  SerializeNameDefault(mForce, Vec3::cZero);
  SerializeNameDefault(mRandomForce, Vec3::cZero);
  
  SerializeNameDefault(mTorque, 0.0f);
  SerializeNameDefault(mGrowth, 0.0f);
  SerializeNameDefault(mDampening, 0.0f);
  SerializeNameDefault(mTwist, Vec3::cZero);
}

void LinearParticleAnimator::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

LinearParticleAnimator::~LinearParticleAnimator()
{
  AnimatorList::Unlink(this);
}

void LinearParticleAnimator::Animate(ParticleList* particleList, float dt,
                                     Mat4Ref transform)
{
  Particle* p = particleList->Particles;
  Math::Random& random = mGraphicsSpace->mRandom;

  Vec3 center = GetTranslationFrom(transform);

  const uint cNumberOfRandomSamples = 13;
  Vec3 randomForces[cNumberOfRandomSamples];
  for(uint i = 0; i < cNumberOfRandomSamples; ++i)
    randomForces[i] = random.PointOnUnitSphere() * mRandomForce;

  Vec3 twistVector = mTwist;
  float twistStrength = twistVector.AttemptNormalize();

  uint i = 0;
  i += random.IntRangeInIn(0, 5);

  while(p != NULL)
  {
 
    float t = p->Time;

    i =(i + 1) % cNumberOfRandomSamples;

    Vec3 velocity = p->Velocity;

    //Apply constant force
    velocity += mForce * dt;

    //Apply random force
    velocity += randomForces[i] * dt;

    //Integrate position
    Vec3 position = p->Position;
    position += velocity * dt;
    p->Position = position;

    //Expand size
    p->Size += mGrowth * dt;
    p->Size = Math::Max(p->Size, 0.0f);

    //Integrate rotation of particle
    p->Rotation += p->RotationalVelocity * dt;
    p->RotationalVelocity = p->RotationalVelocity + mTorque * dt;

    //Twist effect
    if(twistStrength != 0.0f)
    {
      Vec3 toCenter = center - p->Position;
      toCenter.AttemptNormalize();

      Vec3 twistMove = Cross(toCenter, twistVector);
      Vec3 inVector = Cross(twistVector, twistMove);
      velocity +=(twistMove + inVector) * dt * twistStrength;
    }
    
    //Damping
    {
      velocity *= Math::Clamp(1.0f - dt * mDampening, 0.0f, 1.0f);
    }

    //Store updated velocity
    p->Velocity = velocity;

    //Move to next particle.
    p = p->Next;

  }
}

//-------------------------------------------------------------- Particle Wander
ZilchDefineType(ParticleWander, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();
  ZilchBindFieldProperty(mWanderAngle);
  ZilchBindFieldProperty(mWanderAngleVariance);
  ZilchBindFieldProperty(mWanderStrength);
}

ParticleWander::ParticleWander()
{
}

void ParticleWander::Serialize(Serializer& stream)
{
  SerializeNameDefault(mWanderAngle, 1.0f);
  SerializeNameDefault(mWanderAngleVariance, 1.0f);
  SerializeNameDefault(mWanderStrength, 1.0f);
}

void ParticleWander::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

ParticleWander::~ParticleWander()
{
  AnimatorList::Unlink(this);
}

void ParticleWander::Animate(ParticleList* particleList, float dt,
                             Mat4Ref transform)
{
  Particle* p = particleList->Particles;
  Vec3 center = GetTranslationFrom(transform);

  while(p != NULL)
  {
    Vec3 velocity = p->Velocity;
    Vec3 normalizedVel = velocity;
    float l = normalizedVel.AttemptNormalize();

    if(l > 0.0f)
    {
      normalizedVel /= l;

      //Get the current wander value
      float curAngle = p->WanderAngle;
      curAngle += mGraphicsSpace->mRandom.FloatVariance(mWanderAngle, mWanderAngleVariance) * dt;

      //Get a basis(not consistent varies based on normal)
      Vec3 a,b;
      Math::GenerateOrthonormalBasis(normalizedVel, &a, &b);

      float wanderChange = dt * mWanderStrength;
      Vec3 change = Math::Cos(curAngle) * wanderChange * a + 
                    Math::Sin(curAngle) * wanderChange * b;
      velocity += change;

      //Store updated wander velocity
      p->WanderAngle = curAngle;
      p->Velocity = velocity;
    }

    //Move to next particle.
    p = p->Next;
  }

}

//--------------------------------------------------- Particle Gradient Animator
ZilchDefineType(ParticleColorAnimator, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mTimeGradient)->Add(new EditorResource(false, true));
  ZilchBindFieldProperty(mVelocityGradient)->Add(new EditorResource(false, true));
  ZilchBindFieldProperty(mMaxParticleSpeed);
}

ParticleColorAnimator::~ParticleColorAnimator()
{
  AnimatorList::Unlink(this);
}

void ParticleColorAnimator::Serialize(Serializer& stream)
{
  SerializeNullableResourceNameDefault(mTimeGradient, ColorGradientManager, "FadeInOut");
  SerializeNullableResourceNameDefault(mVelocityGradient, ColorGradientManager, nullptr);
  SerializeNameDefault(mMaxParticleSpeed, 5.0f);
}

void ParticleColorAnimator::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

void ParticleColorAnimator::Animate(ParticleList* particleList, float dt,  Mat4Ref transform)
{
  // Do nothing if neither gradients exist
  ColorGradient* timeGradient = mTimeGradient;
  ColorGradient* velocityGradient = mVelocityGradient;

  if(timeGradient == nullptr && velocityGradient == nullptr)
    return;

  float maxSpeedSq = mMaxParticleSpeed * mMaxParticleSpeed;

  // Iterate over each particle
  Particle* p = particleList->Particles;
  while(p != NULL)
  {
    Vec4 color = Vec4(1);

    // Sample time gradient
    if(timeGradient)
    {
      float normalizedT = p->Time / p->Lifetime;
      color *= timeGradient->Sample(normalizedT);
    }

    // Sample velocity gradient
    if(velocityGradient)
    {
      float speedSq = Math::LengthSq(p->Velocity);
      float normalizedT = speedSq / maxSpeedSq;

      // Don't let it go above 1
      normalizedT = Math::Min(normalizedT, 1.0f);

      color *= velocityGradient->Sample(normalizedT);
    }

    // Set the final color
    p->Color = color;

    //Move to next particle.
    p = p->Next;
  }
}

//----------------------------------------------------------- Particle Attractor
ZilchDefineType(ParticleAttractor, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mPositionSpace);
  ZilchBindFieldProperty(mAttractPosition);
  ZilchBindFieldProperty(mStrength);
  ZilchBindFieldProperty(mMinDistance);
  ZilchBindFieldProperty(mMaxDistance);
}


ParticleAttractor::~ParticleAttractor()
{
  AnimatorList::Unlink(this);
}

void ParticleAttractor::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(SystemSpace, mPositionSpace, SystemSpace::LocalSpace);
  SerializeNameDefault(mAttractPosition, Vec3::cZero);
  SerializeNameDefault(mStrength, 1.0f);
  SerializeNameDefault(mMinDistance, 0.0f);
  SerializeNameDefault(mMaxDistance, 10.0f);
}

void ParticleAttractor::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

void ParticleAttractor::Animate(ParticleList* particleList, float dt,
                                Mat4Ref transform)
{
  Vec3 center = GetTranslationFrom(transform);
  float range = mMaxDistance - mMinDistance;
  float invRange =(1.0f / range);

  Vec3 attractPosition = mAttractPosition;
  if (mPositionSpace == SystemSpace::LocalSpace)
    attractPosition = Math::TransformPoint(transform, attractPosition);

  Particle* particle = particleList->Particles;
  while(particle != NULL)
  {
    Vec3 toAttractPoint = attractPosition - particle->Position;
    float distance = toAttractPoint.AttemptNormalize();

    distance -= mMinDistance;
    distance *= invRange;

    float falloff = 1.0f - distance;
    falloff = Math::Clamp(falloff, 0.0f, 1.0f);

    Vec3 velocity = particle->Velocity;
    velocity += toAttractPoint * mStrength * falloff * dt;
    particle->Velocity = velocity;

    //Move to next particle.
    particle = particle->Next;
  }
}


//------------------------------------------------------------- Particle Twister
ZilchDefineType(ParticleTwister, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mAxis);
  ZilchBindFieldProperty(mStrength);
  ZilchBindFieldProperty(mMinDistance);
  ZilchBindFieldProperty(mMaxDistance);
}

ParticleTwister::ParticleTwister()
{

}

ParticleTwister::~ParticleTwister()
{
  AnimatorList::Unlink(this);
}

void ParticleTwister::Serialize(Serializer& stream)
{
  SerializeNameDefault(mAxis, Vec3(0,1,0));
  SerializeNameDefault(mStrength, 1.0f);
  SerializeNameDefault(mMinDistance, 0.0f);
  SerializeNameDefault(mMaxDistance, 100.0f);
}

void ParticleTwister::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

void ParticleTwister::Animate(ParticleList* particleList, float dt,
                                Mat4Ref transform)
{
  Vec3 center = GetTranslationFrom(transform);
  float range = mMaxDistance - mMinDistance;

  float invRange = 1.0f;
  if(range > 0.0f)
    invRange = (1.0f / range);

  Vec3 twistVector = mAxis;
  float strength = mStrength;

  Particle* particle = particleList->Particles;
  while(particle != NULL)
  {
    Vec3 toCenter = center - particle->Position;
    float distance = toCenter.Normalize();
    Vec3 velocity = particle->Velocity;

    distance -= mMinDistance;
    distance *= invRange;

    float falloff = 1.0f - distance;
    falloff = Math::Clamp(falloff, 0.0f, 1.0f);

    Vec3 twistMove = Cross(toCenter, twistVector);
    Vec3 inVector = Cross(twistVector, twistMove);
    velocity +=(twistMove + inVector) *(dt * strength * falloff);

    particle->Velocity = velocity;

    //Move to next particle.
    particle = particle->Next;
  }
}


//----------------------------------------------------- Particle Collision Plane
ZilchDefineType(ParticleCollisionPlane, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();
  ZilchBindFieldProperty(mPlaneSpace);
  ZilchBindFieldProperty(mPlanePosition);
  ZilchBindFieldProperty(mPlaneNormal);
  ZilchBindGetterSetterProperty(Restitution)->Add(new EditorRange(0.0f, 1.0f, 0.01f));
  ZilchBindGetterSetterProperty(Friction)->Add(new EditorRange(0.0f, 1.0f, 0.01f));
}

ParticleCollisionPlane::~ParticleCollisionPlane()
{
  AnimatorList::Unlink(this);
}

void ParticleCollisionPlane::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(SystemSpace, mPlaneSpace, SystemSpace::LocalSpace);
  SerializeNameDefault(mPlanePosition, Vec3(0, -1, 0));
  SerializeNameDefault(mPlaneNormal, Vec3::cYAxis);
  SerializeNameDefault(mRestitution, 1.0f);
  SerializeNameDefault(mFriction, 0.3f);
}

void ParticleCollisionPlane::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

void ReflectParticle(Particle* particle, Vec3Param planeNormal, float restitution, float friction)
{
  Vec3 velocity = particle->Velocity;

  // Reflect
  velocity = Math::ReflectAcrossPlane(velocity, planeNormal);

  // Split up the velocity so we can apply restitution and friction in different directions
  Vec3 velocityNormal = Math::ProjectOnVector(velocity, planeNormal);
  Vec3 velocityTangent = velocity - velocityNormal;

  velocityNormal *= restitution;
  velocityTangent *= (1.0f - friction);

  // Re-compute the velocity
  particle->Velocity = velocityNormal + velocityTangent;
}

void ParticleCollisionPlane::Animate(ParticleList* particleList, float dt, 
                                     Mat4Ref transform)
{
  Vec3 planePosition = mPlanePosition;
  Vec3 planeNormal = mPlaneNormal.AttemptNormalized();
  if(mPlaneSpace == SystemSpace::LocalSpace)
  {
    planePosition = Math::TransformPoint(transform, planePosition);
    planeNormal = Math::TransformNormal(transform, planeNormal);
  }

  Plane plane(planeNormal, planePosition);

  Particle* particle = particleList->Particles;
  while(particle != NULL)
  {
    Vec3 position = particle->Position;

    float distance = plane.SignedDistanceToPlane(position);
    if(distance < 0)
    {
      // Project the particle back onto the plane
      particle->Position = position + (planeNormal * -distance);

      ReflectParticle(particle, planeNormal, mRestitution, mFriction);
    }

    //Move to next particle.
    particle = particle->Next;
  }
}

float ParticleCollisionPlane::GetRestitution()
{
  return mRestitution;
}

void ParticleCollisionPlane::SetRestitution(float restitution)
{
  // Negative numbers can cause a particle to infinitely speed up, causing floating point errors
  mRestitution = Math::Max(0.0f, restitution);
}

float ParticleCollisionPlane::GetFriction()
{
  return mFriction;
}

void ParticleCollisionPlane::SetFriction(float friction)
{
  mFriction = Math::Clamp(friction, 0.0f, 1.0f);
}

//----------------------------------------------------- Particle Collision Heightmap
ZilchDefineType(ParticleCollisionHeightmap, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDocumented();
  ZilchBindFieldProperty(mHeightMap);
  ZilchBindGetterSetterProperty(Restitution)->Add(new EditorRange(0.0f, 1.0f, 0.01f));
  ZilchBindGetterSetterProperty(Friction)->Add(new EditorRange(0.0f, 1.0f, 0.01f));
}

ParticleCollisionHeightmap::~ParticleCollisionHeightmap()
{
  AnimatorList::Unlink(this);
}

void ParticleCollisionHeightmap::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("HeightMap", mHeightMap, CogPath());
  SerializeNameDefault(mRestitution, 0.5f);
  SerializeNameDefault(mFriction, 0.3f);
}

void ParticleCollisionHeightmap::OnAllObjectsCreated(CogInitializer& initializer)
{
  mHeightMap.RestoreLink(initializer, this, "HeightMap");
}

void ParticleCollisionHeightmap::Initialize(CogInitializer& initializer)
{
  ParticleAnimator::Initialize(initializer);
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
}

void ParticleCollisionHeightmap::Animate(ParticleList* particleList, float dt, Mat4Ref transform)
{
  Cog* cog = mHeightMap.GetCog();
  if (cog == nullptr)
    return;
  
  HeightMap* map = cog->has(HeightMap);
  if (map == NULL)
    return;

  Vec3 mapUp = map->GetWorldUp();
  Vec3 mapRight, mapForward;
  Math::GenerateOrthonormalBasis(mapUp, &mapRight, &mapForward);

  Particle* particle = particleList->Particles;
  while (particle != NULL)
  {
    Vec3 position = particle->Position;

    Vec3 normal;
    float sampleHeight = map->SampleHeight(position, -Math::PositiveMax(), &normal);
    float particleHeight = map->GetWorldPointHeight(position);
    float particleBottom = particleHeight - particle->Size;

    if (particleHeight < sampleHeight)
    {
      Vec3 velocity = particle->Velocity;

      // Move to our previous position
      particle->Position = particle->Position - velocity * dt;

      ReflectParticle(particle, normal, mRestitution, mFriction);
    }

    //Move to next particle.
    particle = particle->Next;
  }
}

float ParticleCollisionHeightmap::GetRestitution()
{
  return mRestitution;
}

void ParticleCollisionHeightmap::SetRestitution(float restitution)
{
  // Negative numbers can cause a particle to infinitely speed up, causing floating point errors
  mRestitution = Math::Max(0.0f, restitution);
}

float ParticleCollisionHeightmap::GetFriction()
{
  return mFriction;
}

void ParticleCollisionHeightmap::SetFriction(float friction)
{
  mFriction = Math::Clamp(friction, 0.0f, 1.0f);
}

} // namespace Zero
