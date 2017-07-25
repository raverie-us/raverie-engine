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

//--------------------------------------------------- Spherical Particle Emitter
ZilchDefineType(SphericalParticleEmitter, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
}

void SphericalParticleEmitter::Initialize(CogInitializer& initializer)
{
  ParticleEmitterShared::Initialize(initializer);
}

void SphericalParticleEmitter::Serialize(Serializer& stream)
{
  ParticleEmitterShared::Serialize(stream);
}

SphericalParticleEmitter::SphericalParticleEmitter()
{
  
}

SphericalParticleEmitter::~SphericalParticleEmitter()
{
}

int SphericalParticleEmitter::EmitParticles(ParticleList* particleList, float dt, 
                                            Mat4Ref transform, Vec3Param emitterVelocity, float timeAlive)
{
  int particlesToEmit = GetParticleEmissionCount(particleList, dt, timeAlive);

  if (particlesToEmit == 0)
    return 0;

  Math::Random& random = mGraphicsSpace->mRandom;

  Vec3 newPosition = GetTranslationFrom(transform);
  Vec3 offset = mLastFramePosition - newPosition;
  
  Vec3 offsetDelta = offset / (float)particlesToEmit;

  for(int p = 0; p < particlesToEmit; ++p)
  {
    Particle* newParticle = particleList->AllocateParticle();

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
      velocity += dirNorm * mTangentVelocity.z + 
                  crossA * mTangentVelocity.y + 
                  crossB * mTangentVelocity.x;
    }

    newParticle->Time = 0;
    newParticle->Size = random.FloatVariance(mSize, mSizeVariance);

    newParticle->Velocity = Math::TransformNormal(transform, velocity) + emitterVelocity * mEmitterVelocityPercent;

    newParticle->Position = Math::TransformPoint(transform, startingPoint);

    if (mFastMovingEmitter)
    {
      newParticle->Position += offsetDelta * (float)p;
    }

    newParticle->Lifetime = random.FloatVariance(mLifetime, mLifetimeVariance);

    newParticle->Color = Vec4(1, 1, 1, 1);

    newParticle->WanderAngle = random.FloatRange(0.0f, 2 * Math::cTwoPi);

    if(mRandomSpin)
      newParticle->Rotation = random.FloatRange(0.0f, 2 * Math::cTwoPi);
    else
      newParticle->Rotation = 0;

    newParticle->RotationalVelocity = random.FloatVariance(Math::DegToRad(mSpin),
                                                           Math::DegToRad(mSpinVariance));

    particleList->AddParticle(newParticle);
  }

  return particlesToEmit;
}

//--------------------------------------------------------- Box Particle Emitter
ZilchDefineType(BoxParticleEmitter, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
}

void BoxParticleEmitter::Initialize(CogInitializer& initializer)
{
  ParticleEmitterShared::Initialize(initializer);
}

void BoxParticleEmitter::Serialize(Serializer& stream)
{
  ParticleEmitterShared::Serialize(stream);
}

BoxParticleEmitter::BoxParticleEmitter()
{
}

BoxParticleEmitter::~BoxParticleEmitter()
{
}

int BoxParticleEmitter::EmitParticles(ParticleList* particleList, float dt, 
                                   Mat4Ref transform, Vec3Param emitterVelocity, float timeAlive)
{
  int particlesToEmit = GetParticleEmissionCount(particleList, dt, timeAlive);
  if (particlesToEmit == 0)
    return 0;

  Math::Random& random = mGraphicsSpace->mRandom;

  Vec3 newPosition = GetTranslationFrom(transform);
  Vec3 offset = mLastFramePosition - newPosition;

  Vec3 offsetDelta = offset / (float)particlesToEmit;

  for(int p = 0; p < particlesToEmit; ++p)
  {
    Particle* newParticle = particleList->AllocateParticle();

    Vec3 halfExtents = mEmitterSize * 0.5f;
    Vec3 startingPoint = Vec3(0,0,0);
    startingPoint.x = random.FloatVariance(0.0f, halfExtents.x);
    startingPoint.y = random.FloatVariance(0.0f, halfExtents.y);
    startingPoint.z = random.FloatVariance(0.0f, halfExtents.z);

    Vec3 velocity = mStartVelocity + random.PointOnUnitSphere() * mRandomVelocity;

    if(mTangentVelocity.LengthSq() > 0.0f)
    {
      Vec3 dirNorm = startingPoint;
      dirNorm.AttemptNormalize();
      Vec3 crossA = Cross(dirNorm, Vec3(0,1,0));
      Vec3 crossB = Cross(crossA, dirNorm);
      velocity += dirNorm * mTangentVelocity.z + 
        crossA * mTangentVelocity.y + 
        crossB * mTangentVelocity.x;
    }

    newParticle->Time = 0;
    newParticle->Size = random.FloatVariance(mSize, mSizeVariance);

    newParticle->Velocity = Math::TransformNormal(transform, velocity) + emitterVelocity * mEmitterVelocityPercent;

    newParticle->Position = Math::TransformPoint(transform, startingPoint);

    if (mFastMovingEmitter)
    {
      newParticle->Position += offsetDelta * (float)p;
    }

    newParticle->Lifetime = random.FloatVariance(mLifetime, mLifetimeVariance);

    newParticle->Color = Vec4(1, 1, 1, 1);

    newParticle->WanderAngle = random.FloatRange(0.0f, 2 * Math::cTwoPi);

    if(mRandomSpin)
      newParticle->Rotation = random.FloatRange(0.0f, 2 * Math::cTwoPi);
    else
      newParticle->Rotation = 0;

    newParticle->RotationalVelocity = random.FloatVariance(Math::DegToRad(mSpin),
      Math::DegToRad(mSpinVariance));

    particleList->AddParticle(newParticle);
  }

  return particlesToEmit;
}

//------------------------------------------------------- Model Particle Emitter
ZilchDefineType(MeshParticleEmitter, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  
  ZeroBindInterface(ParticleEmitterShared);
  ZeroBindDocumented();
  ZilchBindGetterSetterProperty(Mesh);
  ZilchBindGetterSetterProperty(MeshEmitMode);
  ZilchBindFieldProperty(mNormalExtrude);
}

void MeshParticleEmitter::Initialize(CogInitializer& initializer)
{
  ParticleEmitterShared::Initialize(initializer);
  Setup();
}

void MeshParticleEmitter::Serialize(Serializer& stream)
{
  ParticleEmitterShared::Serialize(stream);
  SerializeResourceName(mMesh, MeshManager);
  SerializeEnumNameDefault(MeshEmitMode, mMeshEmitMode, MeshEmitMode::Face);
  SerializeNameDefault(mNormalExtrude, 0.1f);
}

Mesh* MeshParticleEmitter::GetMesh()
{
  return mMesh;
}

void MeshParticleEmitter::SetMesh(Mesh* newMesh)
{
  if(newMesh == NULL)
    return;
  mMesh = newMesh;
  Setup();
}

void MeshParticleEmitter::Setup()
{
  mEdgeTable.Clear();
  mFaceTable.Clear();

  if(mMeshEmitMode == MeshEmitMode::Edge)
    SetupEdgeTable();
  else if(mMeshEmitMode == MeshEmitMode::Face)
    SetupFaceTable();
}

void MeshParticleEmitter::SetupEdgeTable()
{
  Mesh* mesh = mMesh;
  if (mesh == nullptr)
    return;

  VertexBuffer& vertices = mesh->mVertices;
  IndexBuffer& indices = mesh->mIndices;

  for(uint i = 0; i < indices.GetCount(); i += 3)
  {
    uint index0 = indices.Get(i);
    uint index1 = indices.Get(i + 1);
    uint index2 = indices.Get(i + 2);

    Vec3 p0 = Math::ToVector3(vertices.GetVertexData(index0, VertexSemantic::Position));
    Vec3 p1 = Math::ToVector3(vertices.GetVertexData(index1, VertexSemantic::Position));
    Vec3 p2 = Math::ToVector3(vertices.GetVertexData(index2, VertexSemantic::Position));

    // Insert each edge based off of the length of the edge
    mEdgeTable.AddItem(Edge(i, i + 1),     (p0 - p1).LengthSq());
    mEdgeTable.AddItem(Edge(i + 1, i + 2), (p1 - p2).LengthSq());
    mEdgeTable.AddItem(Edge(i + 2, i),     (p2 - p0).LengthSq());
  }
  mEdgeTable.BuildTable();
}

void MeshParticleEmitter::SetupFaceTable()
{
  Mesh* mesh = mMesh;
  if (mesh == nullptr)
    return;

  VertexBuffer& vertices = mesh->mVertices;
  IndexBuffer& indices = mesh->mIndices;

  for (uint i = 0; i < indices.GetCount(); i += 3)
  {
    uint index0 = indices.Get(i);
    uint index1 = indices.Get(i + 1);
    uint index2 = indices.Get(i + 2);

    Vec3 p0 = Math::ToVector3(vertices.GetVertexData(index0, VertexSemantic::Position));
    Vec3 p1 = Math::ToVector3(vertices.GetVertexData(index1, VertexSemantic::Position));
    Vec3 p2 = Math::ToVector3(vertices.GetVertexData(index2, VertexSemantic::Position));

    Triangle tri(p0, p1, p2);
    float area = tri.GetArea();

    mFaceTable.AddItem(i,area);
  }
  mFaceTable.BuildTable();
}

MeshParticleEmitter::MeshParticleEmitter()
{
}

MeshParticleEmitter::~MeshParticleEmitter()
{
}

int MeshParticleEmitter::EmitParticles(ParticleList* particleList, float dt, 
                                        Mat4Ref transform, Vec3Param emitterVelocity, float timeAlive)
{
  if(!mActive)
    return 0;

  Math::Random& random = mGraphicsSpace->mRandom;

  Setup();

  int particlesToEmit = GetParticleEmissionCount(particleList, dt, timeAlive);
  for(int p = 0; p < particlesToEmit; ++p)
  {
    Particle* newParticle = particleList->AllocateParticle();

    Vec3 position, normal;
    GetNextEmitPoint(&position, &normal);

    Vec3 startingPoint = position + normal * mNormalExtrude;

    Vec3 velocity = mStartVelocity + random.PointOnUnitSphere() * mRandomVelocity;

    if(mTangentVelocity.LengthSq() > 0.0f)
    {
      Vec3 dirNorm = normal;
      dirNorm.AttemptNormalize();
      Vec3 crossA = Cross(dirNorm, Vec3(0,1,0));
      Vec3 crossB = Cross(crossA, dirNorm);
      velocity += dirNorm * mTangentVelocity.z + 
                  crossA * mTangentVelocity.y + 
                  crossB * mTangentVelocity.x;
    }

    newParticle->Time = 0;
    newParticle->Size = random.FloatVariance(mSize, mSizeVariance);

    newParticle->Velocity = Math::TransformNormal(transform, velocity) + emitterVelocity * mEmitterVelocityPercent;
    newParticle->Position = Math::TransformPoint(transform, startingPoint);
    newParticle->Lifetime = random.FloatVariance(mLifetime, mLifetimeVariance);

    newParticle->Color = Vec4(1, 1, 1, 1);

    newParticle->WanderAngle = random.FloatRange(0.0f, 2 * Math::cTwoPi);

    if(mRandomSpin)
      newParticle->Rotation = random.FloatRange(0.0f, 2 * Math::cTwoPi);
    else
      newParticle->Rotation = 0;

    newParticle->RotationalVelocity = random.FloatVariance(Math::DegToRad(mSpin),
                                                           Math::DegToRad(mSpinVariance));

    particleList->AddParticle(newParticle);
  }

  return particlesToEmit;
}

void MeshParticleEmitter::GetNextEmitPoint(Vec3Ptr position, Vec3Ptr normal)
{
  Math::Random& random = mGraphicsSpace->mRandom;

  Mesh* mesh = mMesh;
  if (mesh == nullptr)
    return;

  VertexBuffer& vertices = mesh->mVertices;
  IndexBuffer& indices = mesh->mIndices;

  if(mMeshEmitMode == MeshEmitMode::Vertex)
  {
    int randomIndex = random.IntRangeInIn(0, vertices.GetVertexCount() - 1);
  
    *position = Math::ToVector3(vertices.GetVertexData(randomIndex, VertexSemantic::Position));
    *normal = Math::ToVector3(vertices.GetVertexData(randomIndex, VertexSemantic::Normal));
  }
  else if(mMeshEmitMode == MeshEmitMode::Edge)
  {
    Edge edge = mEdgeTable.Sample(random);

    Vec3 p0 = Math::ToVector3(vertices.GetVertexData(edge.first, VertexSemantic::Position));
    Vec3 p1 = Math::ToVector3(vertices.GetVertexData(edge.second, VertexSemantic::Position));

    float a = random.FloatRange(0.0f, 1.0f);
    *position = p0 + (p1 - p0) * a;

    // Temporary fix causing particles to get invalid positions
    *normal = Vec3::cZero;
  }
  else if(mMeshEmitMode == MeshEmitMode::Face)
  {
    uint triIndex = mFaceTable.Sample(random);


    uint index0 = indices.Get(triIndex);
    uint index1 = indices.Get(triIndex + 1);
    uint index2 = indices.Get(triIndex + 2);

    Vec3 p0 = Math::ToVector3(vertices.GetVertexData(index0, VertexSemantic::Position));
    Vec3 p1 = Math::ToVector3(vertices.GetVertexData(index1, VertexSemantic::Position));
    Vec3 p2 = Math::ToVector3(vertices.GetVertexData(index2, VertexSemantic::Position));

    float r1 = random.FloatRange(0.0f, 1.0f);
    float r2 = random.FloatRange(0.0f, 1.0f);
    *position = (1 - Math::Sqrt(r1)) * p0 + 
                (Math::Sqrt(r1) * (1 - r2)) * p1 + 
                (Math::Sqrt(r1) * r2) * p2;
    *normal = Math::Cross(p1 - p0, p2 - p0).Normalized();
  }
}

void MeshParticleEmitter::SetMeshEmitMode(MeshEmitMode::Enum mode)
{
  mMeshEmitMode = mode;
  Setup();
}

MeshEmitMode::Enum MeshParticleEmitter::GetMeshEmitMode()
{
  return mMeshEmitMode;
}

} // namespace Zero
