///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Nathan Carlson
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(SkeletonModified);
  DefineEvent(SkeletonDestroyed);
}

ParentSkeletonRange::ParentSkeletonRange(Cog* bone)
{
  mSkeleton = nullptr;
  mParent = bone;
  if (mParent != nullptr)
    PopFront();
}

Skeleton* ParentSkeletonRange::Front()
{
  return mSkeleton;
}

void ParentSkeletonRange::PopFront()
{
  mSkeleton = nullptr;
  mParent = mParent->GetParent();
  while (mParent)
  {
    mSkeleton = mParent->has(Skeleton);
    if (mSkeleton != nullptr)
      return;
    mParent = mParent->GetParent();
  }
}

bool ParentSkeletonRange::Empty()
{
  return mSkeleton == nullptr;
}

ZilchDefineType(Bone, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindDependency(Transform);
}

void Bone::Serialize(Serializer& stream)
{
}

void Bone::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
  ConnectThisTo(this, Events::CogNameChanged, OnCogNameChanged);
  NotifySkeletonModified();
}

void Bone::OnDestroy(uint flags)
{
  NotifySkeletonModified();
}

void Bone::AttachTo(AttachmentInfo& info)
{
  NotifySkeletonModified();
}

void Bone::Detached(AttachmentInfo& info)
{
  NotifySkeletonModified();
}

void Bone::DebugDraw()
{
  ParentSkeletonRange range(GetOwner());
  forRange (Skeleton* skeleton, range)
    skeleton->DebugDrawSkeleton(GetOwner()->mName);
}

void Bone::OnCogNameChanged(Event* event)
{
  NotifySkeletonModified();
}

void Bone::NotifySkeletonModified()
{
  ParentSkeletonRange range(GetOwner());
  forRange (Skeleton* skeleton, range)
    skeleton->MarkModified();
}

Mat4 Bone::GetLocalTransform()
{
  Mat4 localMatrix = mTransform->GetLocalMatrix();

  Cog* parent = GetOwner()->GetParent();
  while (parent)
  {
    if (parent->has(Bone) != nullptr || parent->has(Skeleton) != nullptr)
      return localMatrix;

    if (Transform* transform = parent->has(Transform))
      localMatrix = transform->GetLocalMatrix() * localMatrix;

    parent = parent->GetParent();
  }

  return localMatrix;
}

ZilchDefineType(Skeleton, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindDependency(Transform);
}

void Skeleton::Serialize(Serializer& stream)
{
}

void Skeleton::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
  MarkModified();
}

void Skeleton::OnAllObjectsCreated(CogInitializer& initializer)
{
  BuildSkeleton();
}

void Skeleton::OnDestroy(uint flags)
{
  Event event;
  DispatchEvent(Events::SkeletonDestroyed, &event);
}

void Skeleton::DebugDraw()
{
  DebugDrawSkeleton(GetOwner()->mName);
}

void Skeleton::DebugDrawSkeleton(StringParam boneName)
{
  forRange (BoneInfo& boneInfo, mBones.All())
    DebugDrawBone(boneInfo, boneName == boneInfo.mCog->mName);
}

void Skeleton::DebugDrawSkeleton(Array<String>& boneNames)
{
  forRange (BoneInfo& boneInfo, mBones.All())
    DebugDrawBone(boneInfo, boneNames.Contains(boneInfo.mCog->mName));
}

void Skeleton::DebugDrawBone(BoneInfo& boneInfo, bool highlight)
{
  ByteColor color = highlight ? Color::DodgerBlue : Color::White;

  Vec3 pos = boneInfo.mCog->has(Transform)->GetWorldTranslation();
  float rad = GetBoneRadius(boneInfo);

  gDebugDraw->Add(Debug::Sphere(pos, rad).Color(color).OnTop(true));

  forRange (Cog* child, boneInfo.mChildren.All())
  {
    Vec3 childPos = child->has(Transform)->GetWorldTranslation();
    Vec3 dir = pos - childPos;
    float len = dir.AttemptNormalize();
    gDebugDraw->Add(Debug::Cone(childPos, dir, len, rad).Color(color).OnTop(true));
  }
}

bool Skeleton::TestRay(GraphicsRayCast& raycast)
{
  bool hitBone = false;
  raycast.mT = Math::PositiveMax();

  forRange (BoneInfo& boneInfo, mBones.All())
  {
    Vec3 pos = boneInfo.mCog->has(Transform)->GetWorldTranslation();
    float rad = GetBoneRadius(boneInfo);
        
    forRange (Cog* child, boneInfo.mChildren.All())
    {
      Vec3 childPos = child->has(Transform)->GetWorldTranslation();
      Vec3 dir = pos - childPos;
      float len = dir.AttemptNormalize();
      // Shorten capsule so child sphere can be selected
      childPos += dir * rad;

      Intersection::IntersectionPoint point;
      Intersection::Type result;
      if (len > Math::Epsilon())
        result = Intersection::RayCapsule(raycast.mRay.Start, raycast.mRay.Direction, pos, childPos, rad, &point);
      else
        result = Intersection::RaySphere(raycast.mRay.Start, raycast.mRay.Direction, pos, rad, &point);

      if (result == Intersection::None)
        continue;

      if (point.T < raycast.mT)
      {
        hitBone = true;
        raycast.mObject = boneInfo.mCog;
        raycast.mT = point.T;
      }
    }

    if (boneInfo.mChildren.Empty())
    {
      Intersection::IntersectionPoint point;
      Intersection::Type result = Intersection::RaySphere(raycast.mRay.Start, raycast.mRay.Direction, pos, rad, &point);

      if (result == Intersection::None)
        continue;

      if (point.T < raycast.mT)
      {
        hitBone = true;
        raycast.mObject = boneInfo.mCog;
        raycast.mT = point.T;
      }
    }
  }

  return hitBone;
}

void Skeleton::MarkModified()
{
  mNeedsRebuild = true;
  ConnectThisTo(GetSpace(), Events::UpdateSkeletons, OnUpdateSkeletons);
}

IndexRange Skeleton::GetBoneTransforms(Array<Mat4>& skinningBuffer, uint version)
{
  if (mNeedsRebuild)
    BuildSkeleton();

  if (version == mCachedVersion)
    return mCachedTransformRange;

  Array<Mat4> boneTransforms;
  boneTransforms.Resize(mBones.Size());

  // mBones[0] is this object and bone pointer may be null
  boneTransforms[0] = mBones[0].mCog->has(Transform)->GetLocalMatrix();
  for (uint i = 1; i < mBones.Size(); ++i)
    boneTransforms[i] = boneTransforms[mBones[i].mParentIndex] * mBones[i].mCog->has(Bone)->GetLocalTransform();

  mCachedTransformRange.start = skinningBuffer.Size();
  skinningBuffer.Append(boneTransforms.All());
  mCachedTransformRange.end = skinningBuffer.Size();

  mCachedVersion = version;
  return mCachedTransformRange;
}

void Skeleton::OnUpdateSkeletons(Event* event)
{
  if (mNeedsRebuild)
    BuildSkeleton();

  DisconnectAll(GetSpace(), this);
}

void Skeleton::BuildSkeleton()
{
  mBones.Clear();
  mNameMap.Clear();
  BuildSkeletonRecursive(*GetOwner(), -1);
  mNeedsRebuild = false;
  mCachedVersion = -1;

  Event event;
  DispatchEvent(Events::SkeletonModified, &event);
}

void Skeleton::BuildSkeletonRecursive(Cog& cog, int parentIndex)
{
  uint index = parentIndex;
  if (cog.has(Bone) != nullptr || parentIndex == -1)
  {
    BoneInfo bone;
    bone.mCog = &cog;
    bone.mParentIndex = parentIndex;

    index = mBones.Size();
    mNameMap[cog.mName] = index;
    mBones.PushBack(bone);

    if (parentIndex != -1)
      mBones[parentIndex].mChildren.PushBack(&cog);
  }

  forRange (Cog& child, cog.GetChildren())
    BuildSkeletonRecursive(child, (int)index);
}

float Skeleton::GetBoneRadius(BoneInfo& boneInfo)
{
  if (boneInfo.mChildren.Empty() && boneInfo.mParentIndex != -1)
    return GetBoneRadius(mBones[boneInfo.mParentIndex]);

  Vec3 pos = boneInfo.mCog->has(Transform)->GetWorldTranslation();

  float radius = 0.005f;
  forRange (Cog* child, boneInfo.mChildren.All())
  {
    Vec3 childPos = child->has(Transform)->GetWorldTranslation();
    radius = Math::Max(radius, Math::Length(childPos - pos) * 0.05f);
  }
        
  return radius;
}

} // namespace Zero
