///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleLogic.cpp
/// Implementation of the Particle system component classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace
{
  Math::Random gRandom;
}

//******************************************************************************
void GenerateSplineBasis(Vec3* normal, Vec3Param splineTangent, Mat4* mat)
{
  Vec3 crossA = Cross(splineTangent, *normal);
  crossA.AttemptNormalize();
  *normal = Cross(crossA, splineTangent);

  *mat = Mat4::cIdentity;
  mat->SetBasis(0, splineTangent, 0);
  mat->SetBasis(1, crossA, 0);
  mat->SetBasis(2, *normal, 0);
}

//******************************************************************************
Vec3 GetSplineNormal(float radians, Vec3* normal, Vec3Param splineTangent)
{
  Vec3 localSpin(0, Math::Cos(radians), Math::Sin(radians));
  localSpin.Normalize();

  Mat4 rotation;
  GenerateSplineBasis(normal, splineTangent, &rotation);
  
  return Math::TransformNormal(rotation, localSpin);
}

//------------------------------------------------------ Spline Particle Emitter

//******************************************************************************
ZilchDefineType(SplineParticleEmitter, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZilchBindFieldProperty(mEmitRadius);
  ZilchBindFieldProperty(mSpawnT)->Add(new EditorRange(0, 1, 0.001f));
  ZilchBindFieldProperty(mSpawnTVariance)->Add(new EditorRange(0, 0.5f, 0.001f));
  ZilchBindFieldProperty(mClampT);
  ZilchBindFieldProperty(mTargetSplineCog);
  ZilchBindGetterSetterProperty(Spline);

  // EmitterSize on the base doesn't do anything, so hide it
  // METAREFACTOR - What should we do here..?
  //meta->GetProperty("EmitterSize")->SetHidden(true);
}

//******************************************************************************
void SplineParticleEmitter::Serialize(Serializer& stream)
{
  ParticleEmitterShared::Serialize(stream);
  SerializeNameDefault(mEmitRadius, 0.0f);
  SerializeNameDefault(mSpawnT, 0.5f);
  SerializeNameDefault(mSpawnTVariance, 0.5f);
  SerializeNameDefault(mClampT, false);
  SerializeNameDefault(mTargetSplineCog, CogPath("."));
}

//******************************************************************************
void SplineParticleEmitter::Initialize(CogInitializer& initializer)
{
  ParticleEmitter::Initialize(initializer);
  ConnectThisTo(&mTargetSplineCog, Events::CogPathCogChanged, OnTargetSplineCogPathChanged);
}

//******************************************************************************
void SplineParticleEmitter::OnAllObjectsCreated(CogInitializer& initializer)
{
  mTargetSplineCog.RestoreLink(initializer, GetOwner(), "TargetSplineCog");
  FindSpline();
}

//******************************************************************************
void SplineParticleEmitter::OnTargetSplineCogPathChanged(Event* e)
{
  FindSpline();
}

//******************************************************************************
void SplineParticleEmitter::FindSpline()
{
  // Send an event to our target cog to find a spline
  Cog* target = mTargetSplineCog.GetCog();
  if(target != nullptr)
  {
    SplineEvent toSend;
    target->DispatchEvent(Events::QuerySpline, &toSend);
    // Should this replace ourself with a null spline?
    mSpline = toSend.mSpline;
  }
}

//******************************************************************************
int SplineParticleEmitter::EmitParticles(ParticleList* particleList, float dt, 
                                   Mat4Ref transform, Vec3Param emitterVelocity, float timeAlive)
{
  // If there are no particles to emit, no need to do anything
  int particlesToEmit = GetParticleEmissionCount(particleList, dt, timeAlive);
  if (particlesToEmit == 0)
    return 0;

  Spline* spline = mSpline;
  if(spline == nullptr)
    return 0;

  Vec3 newPosition = GetTranslationFrom(transform);
  Vec3 offset = mLastFramePosition - newPosition;

  Vec3 offsetDelta = offset / (float)particlesToEmit;

  // Sampling the curve is always in world space, so we need to bring it back
  // into local space before using it
  Transform* trans = GetOwner()->has(Transform);

  for(int p = 0; p < particlesToEmit; ++p)
  {
    // Create a new particle
    Particle* newParticle = particleList->AllocateParticle();
    
    // Generate a normalized time to sample the curve and clamp if specified
    float t = gRandom.FloatVariance(mSpawnT, mSpawnTVariance);
    if(mClampT)
      t = Math::Clamp(t, 0.0f, 1.0f);

    // Sample the curve
    float curveLength = spline->GetTotalDistance();
    SplineSampleData sample = spline->SampleDistance(t * curveLength);

    // The curve always samples in world space, so bring the sample back into
    // local space
    Vec3 startingPoint = trans->TransformPointInverse(sample.mWorldPoint);
    
    // Random velocity
    Vec3 velocity = mStartVelocity + gRandom.PointOnUnitSphere() * mRandomVelocity;

    // No need to generate an orthonormal basis if it won't be used
    if(mTangentVelocity.LengthSq() > 0.0f || mEmitRadius > 0.0f)
    {
      // Generate the orthonormal basis from the local tangent
      Vec3 tangent = trans->TransformNormalInverse(sample.mWorldTangent);
      tangent.AttemptNormalize();

      Vec3 crossA, normal;
      GenerateOrthonormalBasis(tangent, &crossA, &normal);

      // Generate a random direction normal to the curve to offset the position
      float randomRotation = gRandom.FloatRange(0, Math::cTwoPi);
      Vec3 dir = GetSplineNormal(randomRotation, &normal, tangent);
      float fill = gRandom.FloatRange(mFill, 1.0f);
      startingPoint += dir * mEmitRadius * fill;

      // Add tangent velocity
      velocity += tangent * mTangentVelocity.z + 
                  crossA * mTangentVelocity.y + 
                  normal * mTangentVelocity.x;
    }

    newParticle->Time = 0;
    newParticle->Size = gRandom.FloatVariance(mSize, mSizeVariance);

    newParticle->Velocity = Math::TransformNormal(transform, velocity) + emitterVelocity * mEmitterVelocityPercent;

    newParticle->Position = Math::TransformPoint(transform, startingPoint);

    if (mFastMovingEmitter)
    {
      newParticle->Position += offsetDelta * (float)p;
    }

    newParticle->Lifetime = gRandom.FloatVariance(mLifetime, mLifetimeVariance);

    newParticle->Color = Vec4(1, 1, 1, 1);

    newParticle->WanderAngle = gRandom.FloatRange(0.0f, 2 * Math::cTwoPi);

    if(mRandomSpin)
      newParticle->Rotation = gRandom.FloatRange(0.0f, 2 * Math::cTwoPi);
    else
      newParticle->Rotation = 0;

    newParticle->RotationalVelocity = gRandom.FloatVariance(Math::DegToRad(mSpin), 
      Math::DegToRad(mSpinVariance));

    particleList->AddParticle(newParticle);
  }

  return particlesToEmit;
}

//******************************************************************************
Spline* SplineParticleEmitter::GetSpline() const
{
  return mSpline;
}

//******************************************************************************
void SplineParticleEmitter::SetSpline(Spline* spline)
{
  mSpline = spline;
}

//----------------------------------------------------- Spline Particle Animator
//******************************************************************************
ZilchDefineType(SplineParticleAnimator, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(Transform);
  ZeroBindDependency(ParticleSystem);
  ZeroBindDependency(SplineParticleEmitter);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Speed);
  ZilchBindFieldProperty(mAutoCalculateLifetime);

  ZilchBindFieldProperty(mHelix)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mHelixRadius)->ZeroFilterBool(mHelix);
  ZilchBindFieldProperty(mHelixWaveLength)->ZeroFilterBool(mHelix);
  ZilchBindFieldProperty(mHelixOffset)->ZeroFilterBool(mHelix);

  ZilchBindFieldProperty(mMode)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mSpringFrequencyHz)->ZeroFilterEquality(mMode, SplineAnimatorMode::Enum, SplineAnimatorMode::Spring);
  ZilchBindFieldProperty(mSpringDampingRatio)->ZeroFilterEquality(mMode, SplineAnimatorMode::Enum, SplineAnimatorMode::Spring);
}

//******************************************************************************
SplineParticleAnimator::~SplineParticleAnimator()
{
  AnimatorList::Unlink(this);
}

//******************************************************************************
void SplineParticleAnimator::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSpeed, 3.0f);
  SerializeNameDefault(mAutoCalculateLifetime, false);

  SerializeNameDefault(mHelix, false);
  SerializeNameDefault(mHelixRadius, 0.5f);
  SerializeNameDefault(mHelixWaveLength, 3.0f);
  SerializeNameDefault(mHelixOffset, 0.0f);

  SerializeEnumNameDefault(SplineAnimatorMode, mMode, SplineAnimatorMode::Exact);
  SerializeNameDefault(mSpringFrequencyHz, 6.0f);
  SerializeNameDefault(mSpringDampingRatio, 0.2f);
}

//******************************************************************************
void SplineParticleAnimator::Initialize(CogInitializer& initializer)
{
  GetOwner()->has(ParticleSystem)->AddAnimator(this);
  mEmitter = GetOwner()->has(SplineParticleEmitter);
}

//******************************************************************************
void SplineParticleAnimator::Animate(ParticleList* particleList, float dt,
                                     Mat4Ref transform)
{
  // Spring constants
  const float f = 1.0f + 2.0f * dt * mSpringDampingRatio * mSpringFrequencyHz;
  const float frequencySquared = mSpringFrequencyHz * mSpringFrequencyHz;
  const float hoo = dt * frequencySquared;
  const float hhoo = dt * hoo;
  const float detInv = 1.0f / (f + hhoo);

  Spline* spline = mEmitter->mSpline;
  if(spline == nullptr)
    return;

  float curveLength = spline->GetTotalDistance();

  /// The time required for each particle to travel the entire spline
  float timeToFinish = curveLength / mSpeed;

  // Update the base lifetime if specified
  if(mAutoCalculateLifetime)
    mEmitter->mLifetime = timeToFinish;

  // Sampling the curve is always in world space, so we need to bring it back
  // into local space before using it
  Transform* trans = GetOwner()->has(Transform);

  // To generate the helix, we need a consistent basis at every point on the
  // curve. We can't just use a single normal because at some orientation,
  // there will be a break in the helix.
  // Because of this, we first generate an orthonormal basis from the tangent
  // at the start of the curve, store its normal, and then use the newly
  // generated normal at each particle
  Vec3 startTangent = spline->SampleDistance(0).mWorldTangent;
  startTangent = trans->TransformNormalInverse(startTangent);

  Vec3 crossA, previousNormal;
  GenerateOrthonormalBasis(startTangent, &crossA, &previousNormal);

  forRange(Particle* p, particleList->All())
  {
    // How far (in meters) the particle has traveled
    float distanceTraveled = p->Time * mSpeed;

    // The percentage of the spline the particle has traveled
    float percentTraveled = distanceTraveled / curveLength;
    
    // The sample value for where the particle currently is
    float sampleTime = percentTraveled;

    // Clamp the sample if the path is closed
    sampleTime = Math::Clamp(sampleTime, 0.0f, 1.0f);

    // Sample the curve
    SplineSampleData sample = spline->SampleDistance(sampleTime * curveLength);

    // The curve is in world space, so bring it into our local space
    // Bring the sample into our local space and transform back
    // depending on which mode we're in
    Vec3 splineSample = trans->TransformPointInverse(sample.mWorldPoint);

    if(mHelix)
    {
      float radians = distanceTraveled / mHelixWaveLength * Math::cTwoPi;

      // Apply the offset
      radians += mHelixOffset;

      // Bring the tangent into our local space
      Vec3 tangent = trans->TransformNormalInverse(sample.mWorldTangent);

      // Generate the spin vector
      Vec3 localSpin = GetSplineNormal(radians, &previousNormal, tangent);

      // Update the position of the particle based on the spin vector
      splineSample = splineSample + localSpin * mHelixRadius;
    }

    // In world / local space
    splineSample = Math::TransformPoint(transform, splineSample);

    if(mMode == SplineAnimatorMode::Exact)
    {
      // Update the velocity so that Beam rendering still works
      p->Velocity = (splineSample - p->Position);
      p->Position = splineSample;
    }
    else // mMode == SplineAnimatorMode::Spring
    {
      Vec3 detX = f * p->Position + dt * p->Velocity + hhoo * splineSample;
      Vec3 detV = p->Velocity + hoo * (splineSample - p->Position);
      p->Position = detX * detInv;
      p->Velocity = detV * detInv;
    }
  }
}

//******************************************************************************
void SplineParticleAnimator::SetSpeed(float speed)
{
  mSpeed = speed;

  // Need access to all particles
  ParticleSystem* data = GetOwner()->has(ParticleSystem);

  // No need to re-base everything if we aren't changing the lifetime
  if(!mAutoCalculateLifetime || data == nullptr)
    return;

  Spline* spline = mEmitter->mSpline;
  if(spline == nullptr)
    return;

  float curveLength = spline->GetTotalDistance();

  /// The time required for each particle to travel the entire spline
  float timeToFinish = curveLength / speed;
  
  // Re-base each particles lifetime so that 
  forRange(Particle* p, data->AllParticles())
  {
    float percentAlive = p->Time / p->Lifetime;

    p->Time = percentAlive * timeToFinish;
    p->Lifetime = timeToFinish;
  }
}

//******************************************************************************
float SplineParticleAnimator::GetSpeed()
{
  return mSpeed;
}

}//namespace Zero
