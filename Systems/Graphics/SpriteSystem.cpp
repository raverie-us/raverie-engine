///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteSystem.cpp
/// Implementation of the Particle Rendering component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SpriteParticleSystem, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindInterface(ParticleSystem);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mVertexColor);
  ZilchBindFieldProperty(mGeometryMode)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mSpriteSource);
  ZilchBindFieldProperty(mParticleAnimation);
  ZilchBindFieldProperty(mParticleSort);
  ZilchBindFieldProperty(mBeamBaseScale)->ZeroFilterEquality(mGeometryMode, SpriteParticleGeometryMode::Enum, SpriteParticleGeometryMode::Beam);
  ZilchBindFieldProperty(mBeamVelocityScale)->ZeroFilterEquality(mGeometryMode, SpriteParticleGeometryMode::Enum, SpriteParticleGeometryMode::Beam);
}

void SpriteParticleSystem::Serialize(Serializer& stream)
{
  ParticleSystem::Serialize(stream);
  SerializeNameDefault(mVertexColor, Vec4(1.0f));
  SerializeEnumNameDefault(SpriteParticleGeometryMode, mGeometryMode, SpriteParticleGeometryMode::Billboarded);
  SerializeResourceNameDefault(mSpriteSource, SpriteSourceManager, "Circle");
  SerializeEnumNameDefault(SpriteParticleAnimationMode, mParticleAnimation, SpriteParticleAnimationMode::Single);
  SerializeEnumNameDefault(SpriteParticleSortMode, mParticleSort, SpriteParticleSortMode::None);
  SerializeNameDefault(mBeamBaseScale, 1.0f);
  SerializeNameDefault(mBeamVelocityScale, 1.0f);
}

void SpriteParticleSystem::Initialize(CogInitializer& initializer)
{
  ParticleSystem::Initialize(initializer);
}

String SpriteParticleSystem::GetDefaultMaterialName()
{
  return "AdditiveSprite";
}

void SpriteParticleSystem::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;

  frameNode.mMeshRenderData = nullptr;
  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mTextureRenderData = mSpriteSource->mTexture->mRenderData;

  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();
  frameNode.mLocalToWorldNormal = Mat3::cIdentity;

  frameNode.mObjectWorldPosition = Vec3::cZero;

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void SpriteParticleSystem::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  viewNode.mLocalToView = viewBlock.mWorldToView;

  if (mSystemSpace != SystemSpace::WorldSpace)
    viewNode.mLocalToView = viewNode.mLocalToView * mTransform->GetWorldMatrix();

  viewNode.mStreamedVertexType = PrimitiveType::Triangles;
  viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  Vec3 emitterPos = mTransform->GetWorldTranslation();

  CheckSort(viewBlock);
  Particle* particle = mParticleList.Particles;

  while (particle != nullptr)
  {
    float particleWidth = particle->Size * 0.5f;

    Vec3 center, right, up;

    switch (mGeometryMode)
    {
      case SpriteParticleGeometryMode::Billboarded:
      {
        float cosAngle = Math::Cos(particle->Rotation);
        float sinAngle = Math::Sin(particle->Rotation);

        center = Math::TransformPoint(viewNode.mLocalToView, particle->Position);
        right = Vec3(cosAngle, sinAngle, 0) * particleWidth;
        up = Vec3(-sinAngle, cosAngle, 0) * particleWidth;
      }
      break;

      case SpriteParticleGeometryMode::Beam:
      {
        Vec3 velocityDir = Math::TransformNormal(viewNode.mLocalToView, particle->Velocity);
        float speed = velocityDir.AttemptNormalize();

        center = Math::TransformPoint(viewNode.mLocalToView, particle->Position);
        right = velocityDir * (speed * mBeamVelocityScale + mBeamBaseScale) * particleWidth;
        up = Cross(Vec3(0, 0, 1), velocityDir) * particleWidth;
      }
      break;

      case SpriteParticleGeometryMode::Outward:
      {
        Vec3 zAxis = particle->Position - emitterPos;
        zAxis.AttemptNormalize();

        Vec3 xAxis, yAxis;
        Math::GenerateOrthonormalBasis(zAxis, &xAxis, &yAxis);

        xAxis = Math::TransformNormal(viewNode.mLocalToView, xAxis);
        xAxis.AttemptNormalize();
        yAxis = Math::TransformNormal(viewNode.mLocalToView, yAxis);
        yAxis.AttemptNormalize();
        zAxis = Math::TransformNormal(viewNode.mLocalToView, zAxis);
        zAxis.AttemptNormalize();

        center = Math::TransformPoint(viewNode.mLocalToView, particle->Position);
        right = (xAxis * Math::Cos(particle->Rotation) + yAxis * Math::Sin(particle->Rotation));
        up = Cross(zAxis, right) * particleWidth;
        right *= particleWidth;
      }
      break;

      case SpriteParticleGeometryMode::FaceVelocity:
      {
        Vec3 zAxis = particle->Velocity;
        zAxis.AttemptNormalize();

        Vec3 xAxis, yAxis;
        Math::GenerateOrthonormalBasis(zAxis, &xAxis, &yAxis);

        xAxis = Math::TransformNormal(viewNode.mLocalToView, xAxis);
        xAxis.AttemptNormalize();
        yAxis = Math::TransformNormal(viewNode.mLocalToView, yAxis);
        yAxis.AttemptNormalize();
        zAxis = Math::TransformNormal(viewNode.mLocalToView, zAxis);
        zAxis.AttemptNormalize();

        center = Math::TransformPoint(viewNode.mLocalToView, particle->Position);
        right = (xAxis * Math::Cos(particle->Rotation) + yAxis * Math::Sin(particle->Rotation));
        up = Cross(zAxis, right) * particleWidth;
        right *= particleWidth;
      }
      break;

      case SpriteParticleGeometryMode::Flat:
      {
        Vec3 facing = Math::TransformNormal(viewNode.mLocalToView, Vec3::cZAxis);
        facing.AttemptNormalize();
        Vec3 xAxis = Math::TransformNormal(viewNode.mLocalToView, Vec3::cXAxis);
        xAxis.AttemptNormalize();
        Vec3 yAxis = Math::TransformNormal(viewNode.mLocalToView, Vec3::cYAxis);
        yAxis.AttemptNormalize();

        center = Math::TransformPoint(viewNode.mLocalToView, particle->Position);
        right = (xAxis * Math::Cos(particle->Rotation) + yAxis * Math::Sin(particle->Rotation));
        up = Cross(facing, right) * particleWidth;
        right *= particleWidth;
      }
      break;
    }

    Vec3 pos[4] =
    {
      center - right + up,
      center - right - up,
      center + right - up,
      center + right + up,
    };


    // Compute particle Uv Rect based on animation
    UvRect uvRect = mSpriteSource->GetUvRect(0);
    if (mSpriteSource->FrameCount > 1)
    {
      // Update particle frame
      uint frame;
      if (mParticleAnimation == SpriteParticleAnimationMode::Single)
        frame = (uint)(particle->Time / particle->Lifetime * (float)mSpriteSource->FrameCount);
      else
        frame = (uint)(particle->Time / mSpriteSource->FrameDelay) % mSpriteSource->FrameCount;
      uvRect = mSpriteSource->GetUvRect(frame);
    }

    Vec2 uv0 = uvRect.TopLeft;
    Vec2 uv1 = uvRect.BotRight;

    Vec4 color = particle->Color * mVertexColor;

    frameBlock.mRenderQueues->AddStreamedQuadView(viewNode, pos, uv0, uv1, color);

    particle = particle->Next;
  }
}

struct ParticleSortInfo
{
  Particle* mParticle;
  u32 mSortValue;
};

struct LocalSpriteSorter
{
  bool operator()(const ParticleSortInfo& lhs, const ParticleSortInfo& rhs)
  {
    return lhs.mSortValue < rhs.mSortValue;
  }
};

u32 GetParticleSortValue(SpriteParticleSortMode::Enum sortMode, Vec3 pos, Vec3 camPos, Vec3 camDir)
{
  u32 value = 0;
  float* floatValue = (float*)&value;

  switch (sortMode)
  {
    case SpriteParticleSortMode::BackToFrontView: *floatValue = -Math::Dot(pos - camPos, camDir); break;
    case SpriteParticleSortMode::FrontToBackView: *floatValue = Math::Dot(pos - camPos, camDir); break;
    case SpriteParticleSortMode::NegativeToPositiveX: *floatValue = pos.x; break;
    case SpriteParticleSortMode::PositiveToNegativeX: *floatValue = -pos.x; break;
    case SpriteParticleSortMode::NegativeToPositiveY: *floatValue = pos.y; break;
    case SpriteParticleSortMode::PositiveToNegativeY: *floatValue = -pos.y; break;
    case SpriteParticleSortMode::NegativeToPositiveZ: *floatValue = pos.z; break;
    case SpriteParticleSortMode::PositiveToNegativeZ: *floatValue = -pos.z; break;
  }

  // If sign bit, flip all bits. If no sign bit, flip sign bit.
  // Makes negative smaller than positive and makes both positive and negative compare in the same direction.
  value ^= -s32(value >> 31) | 0x80000000;
  return value;
}

void SpriteParticleSystem::CheckSort(ViewBlock& viewBlock)
{
  Particle* particle = mParticleList.Particles;

  // As long as we're in sort mode, and we have particles to be sorted...
  if (mParticleSort == SpriteParticleSortMode::None || particle == nullptr)
    return;

  // An array to store all sorted particles
  Array<ParticleSortInfo> sortedParticles;

  // Reserve space in the sorted particle array
  sortedParticles.Reserve(mParticleList.mActiveParticles);

  // Particle info for the sorter
  ParticleSortInfo particleInfo;

  Vec3 cameraPos = viewBlock.mEyePosition;
  Vec3 cameraDir = viewBlock.mEyeDirection;

  // Loop through all the particles
  while (particle != nullptr)
  {
    // Fill in the particle info and push it back
    particleInfo.mParticle = particle;
    particleInfo.mSortValue = GetParticleSortValue(mParticleSort, particle->Position, cameraPos, cameraDir);

    // Push them into the array
    sortedParticles.PushBack(particleInfo);

    // Iterate to the next particle
    particle = particle->Next;
  }

  // Sort the array
  Sort(sortedParticles.All(), LocalSpriteSorter());

  // Loop through all the sorted particles
  for (size_t i = 0; i < (sortedParticles.Size() - 1); ++i)
  {
    // Re-point the particle's next pointer
    sortedParticles[i].mParticle->Next = sortedParticles[i + 1].mParticle;
  }

  // Make sure we set the last particle in the array to point at null for its "next" pointer
  sortedParticles.Back().mParticle->Next = nullptr;

  // The particle list is now pointing to the front of our array
  particle = sortedParticles.Front().mParticle;

  // Since we resorted the list and actually modified the particles, we need to make sure we set the new head of the list
  mParticleList.Particles = particle;
}

} // namespace Zero
