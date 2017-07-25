///////////////////////////////////////////////////////////////////////////////
///
/// \file ParticleEmitter.cpp
/// Implementation of the Particle emitter component classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ParticlesExhausted);
  DefineEvent(AllParticlesDead);
}

//------------------------------------------------------------- Particle Emitter
ZilchDefineType(ParticleEmitter, builder, type)
{
  ZeroBindDocumented();
  ZeroBindTag(Tags::Particle);
}

ParticleEmitter::ParticleEmitter()
{
  mGraphicsSpace = nullptr;
}

ParticleEmitter::~ParticleEmitter()
{
  EmitterList::Unlink(this);
}

void ParticleEmitter::Initialize(CogInitializer& initializer)
{
  mGraphicsSpace = GetSpace()->has(GraphicsSpace);

  GetOwner()->has(ParticleSystem)->AddEmitter(this);
  mTransform = GetOwner()->has(Transform);
  if(mTransform)
  {
    Vec3 newPosition = mTransform->GetWorldTranslation();
    mLastFramePosition = newPosition;
  }
}


//--------------------------------------------------------------------------------- Hide Base Filter
ZilchDefineType(HideBaseFilter, builder, type)
{
}

HideBaseFilter::HideBaseFilter(BoundType* hiddenOnType) : 
  mHiddenOnType(hiddenOnType)
{

}

bool HideBaseFilter::Filter(Property* prop, HandleParam instance)
{
  if (instance.StoredType->IsA(mHiddenOnType))
    return false;
  return true;
}

//--------------------------------------------------------------------- Particle Emitter Common Data
const float ParticleEmitterShared::mMaxEmitRate = 50000.0f;

ZilchDefineType(ParticleEmitterShared, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindInterface(ParticleEmitter);
  ZeroBindDependency(Transform);
  ZeroBindDependency(ParticleSystem);
  ZeroBindEvent(Events::ParticlesExhausted, ObjectEvent);
  ZeroBindEvent(Events::AllParticlesDead, ObjectEvent);

  ZilchBindFieldProperty(mActive);

  ZilchBindFieldProperty(mEmitDelay);
  ZilchBindFieldProperty(mEmitCount);

  ZilchBindGetterSetterProperty(EmitRate);
  ZilchBindFieldProperty(mEmitVariance);
  ZilchBindFieldProperty(mEmitRateSoftStartTime);

  ZilchBindFieldProperty(mSize);
  ZilchBindFieldProperty(mSizeVariance);

  ZilchBindFieldProperty(mLifetime);
  ZilchBindFieldProperty(mLifetimeVariance);

  ZilchBindFieldProperty(mSpin);
  ZilchBindFieldProperty(mSpinVariance);

  ZilchBindFieldProperty(mRandomSpin);

  ZilchBindFieldProperty(mFill)->Add(new EditorRange(0, 1, 0.01f))->HideOnDerivedType(MeshParticleEmitter);
  ZilchBindFieldProperty(mEmitterVelocityPercent);

  ZilchBindFieldProperty(mStartVelocity);
  ZilchBindFieldProperty(mRandomVelocity);
  ZilchBindFieldProperty(mTangentVelocity);

  ZilchBindFieldProperty(mEmitterSize)->HideOnDerivedType(MeshParticleEmitter);

  ZilchBindFieldProperty(mFastMovingEmitter);

  ZilchBindMethodProperty(ResetCount);
}

ParticleEmitterShared::ParticleEmitterShared()
{
  mAccumulation = 0.0f;
  mEmitRateCurrent = 0.0f;
  mSample = 0.0f;
}

ParticleEmitterShared::~ParticleEmitterShared()
{
}

void ParticleEmitterShared::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);

  SerializeNameDefault(mEmitCount, (uint)0);

  SerializeNameDefault(mEmitDelay, 0.0f);
  SerializeNameDefault(mEmitRate, 10.0f);
  SerializeNameDefault(mEmitVariance, 0.0f);
  SerializeNameDefault(mEmitRateSoftStartTime, 0.0f);

  SerializeNameDefault(mSize, 1.0f);
  SerializeNameDefault(mSizeVariance, 0.5f);

  SerializeNameDefault(mLifetime, 1.0f);
  SerializeNameDefault(mLifetimeVariance, 0.2f);

  SerializeNameDefault(mSpin, 0.0f);
  SerializeNameDefault(mSpinVariance, 50.0f);
  SerializeNameDefault(mRandomSpin, true);

  SerializeNameDefault(mFill, 1.0f);
  SerializeNameDefault(mEmitterVelocityPercent, 0.0f);

  SerializeNameDefault(mStartVelocity, Vec3::cZero);
  SerializeNameDefault(mRandomVelocity, Vec3(2,2,2));
  SerializeNameDefault(mTangentVelocity, Vec3::cZero);

  SerializeNameDefault(mEmitterSize, Vec3::cZero);

  SerializeNameDefault(mFastMovingEmitter, false);

  if(stream.GetMode() == SerializerMode::Loading)
  {
    mEmitRate = Math::Min(mEmitRate, mMaxEmitRate);
  }
}

void ParticleEmitterShared::Initialize(CogInitializer& initializer)
{
  ParticleEmitter::Initialize(initializer);
  mCurrentCount = mEmitCount;
  mFill = Math::Min(1.0f, mFill);
}

void ParticleEmitterShared::ResetCount()
{
  mCurrentCount = mEmitCount;
}

float ParticleEmitterShared::GetEmitRate()
{
  return mEmitRate;
}

void ParticleEmitterShared::SetEmitRate(float emitRate)
{
  if(emitRate > mMaxEmitRate)
  {
    String msg = String::Format("The value %f is outside the valid range of [0, %f]. "
                                "The value will be clamped", emitRate, mMaxEmitRate);
    DoNotifyWarning("Particle emit rate too large", msg);
    emitRate = mMaxEmitRate;
  }

  mEmitRate = emitRate;
}

int ParticleEmitterShared::GetParticleEmissionCount(ParticleList* particleList, float dt, float timeAlive)
{
  if(!mActive)
    return 0;

  if(mEmitDelay > timeAlive)
    return 0;


  mSample -= dt;
  const float sampleTiming = 0.25f;

  if(mSample < sampleTiming)
  {
    mSample+=sampleTiming;
    mEmitRateCurrent = mGraphicsSpace->mRandom.FloatVariance(mEmitRate, mEmitVariance);
  }

  if(mEmitRateCurrent <= 0.0f)
    return 0;

  float emitRateScalar = 1.0f;
  if(mEmitRateSoftStartTime > 0.0f)
  {
    emitRateScalar = (timeAlive - mEmitDelay) / mEmitRateSoftStartTime;
    emitRateScalar = Math::Clamp(emitRateScalar, 0.0f, 1.0f);
  }

  uint emitRate = (uint)(float(mEmitRateCurrent) * emitRateScalar);

  mAccumulation += dt * emitRate;
  int particlesToEmit =(int)mAccumulation;
  mAccumulation-= float(particlesToEmit);

  if(mEmitCount != 0)
  {
    if(mCurrentCount > 0)
    {
      if(mCurrentCount > particlesToEmit)
      {
        mCurrentCount -= particlesToEmit;
      }
      else
      {
        particlesToEmit = mCurrentCount;
        mCurrentCount = 0;
        // this emitter has exhausted all the particles it will ever emit.
        ObjectEvent e(this);
        GetOwner()->GetDispatcher()->Dispatch(Events::ParticlesExhausted, &e);
      }
    }
    else
    {
      return 0;
    }
  }

  return particlesToEmit;
}

Particle* ParticleEmitterShared::CreateInitializedParticle(ParticleList* particleList,
                                                           int particle, 
                                                           Mat4Ref transform, 
                                                           Vec3Param emitterVelocity)
{
  Particle* newParticle = particleList->AllocateParticle();
  Math::Random& random = mGraphicsSpace->mRandom;

  Vec3 direction;

  if(mEmitterSize.x == 0)
    direction = random.PointOnUnitCircleX();
  else if (mEmitterSize.y == 0)
    direction = random.PointOnUnitCircleY();
  else if (mEmitterSize.z == 0)
    direction = random.PointOnUnitCircleZ();
  else
    direction = random.PointOnUnitSphere();

  Vec3 startingPoint = direction;
  startingPoint *= random.FloatRange(mFill, 1.0f);
  startingPoint *= mEmitterSize;

  Vec3 velocity = mStartVelocity + random.PointOnUnitSphere() * mRandomVelocity;

  if(mTangentVelocity.LengthSq() > 0.0f)
  {
    Vec3 dirNorm = startingPoint;
    dirNorm.AttemptNormalize();
    Vec3 crossA = Cross(dirNorm, Vec3(0,1,0));
    Vec3 crossB = Cross(crossA, dirNorm);
    velocity += dirNorm * mTangentVelocity.z + crossA * mTangentVelocity.y + crossB * mTangentVelocity.x;
  }

  newParticle->Time = 0;
  newParticle->Size = random.FloatVariance(mSize, mSizeVariance);

  newParticle->Velocity = Math::TransformNormal(transform, velocity) + emitterVelocity * mEmitterVelocityPercent;
  newParticle->Position = Math::TransformPoint(transform, startingPoint);
  newParticle->Lifetime = random.FloatVariance(mLifetime, mLifetimeVariance);

  newParticle->WanderAngle = random.FloatRange(0.0f, 2 * Math::cTwoPi);

  if(mRandomSpin)
    newParticle->Rotation = random.FloatRange(0.0f, 2 * Math::cTwoPi);
  else
    newParticle->Rotation = 0;

  newParticle->RotationalVelocity = random.FloatVariance(Math::DegToRad(mSpin),
                                                          Math::DegToRad(mSpinVariance));

  particleList->AddParticle(newParticle);
  
  return newParticle;
}

} // namespace Zero
