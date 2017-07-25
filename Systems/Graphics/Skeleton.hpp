///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(SkeletonModified);
  DeclareEvent(SkeletonDestroyed);
}

class ParentSkeletonRange
{
public:
  ParentSkeletonRange(Cog* bone);

  Skeleton* Front();
  void PopFront();
  bool Empty();

  Cog* mParent;
  Skeleton* mSkeleton;
};

/// Used by Skeleton to identify child objects whose transforms can be used for mesh skinning.
class Bone : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  void DebugDraw() override;

  void OnCogNameChanged(Event* event);
  void NotifySkeletonModified();
  Mat4 GetLocalTransform();

  Transform* mTransform;
};

class BoneInfo
{
public:
  Cog* mCog;
  int mParentIndex;
  Array<Cog*> mChildren;
};

/// Stores a map of Bones so that SkinnedModels can collect transform matrices for mesh skinning.
class Skeleton : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;
  void DebugDraw() override;

  void DebugDrawSkeleton(StringParam boneName);
  void DebugDrawSkeleton(Array<String>& boneNames);
  void DebugDrawBone(BoneInfo& boneInfo, bool highlight);
  bool TestRay(GraphicsRayCast& raycast);
  void MarkModified();
  IndexRange GetBoneTransforms(Array<Mat4>& skinningBuffer, uint version);

  void OnUpdateSkeletons(Event* event);
  void BuildSkeleton();
  void BuildSkeletonRecursive(Cog& cog, int parentIndex);
  float GetBoneRadius(BoneInfo& boneInfo);

  Transform* mTransform;
  Array<BoneInfo> mBones;
  HashMap<String, uint> mNameMap;

  bool mNeedsRebuild;

  uint mCachedVersion;
  IndexRange mCachedTransformRange;
};

} // namespace Zero
