///////////////////////////////////////////////////////////////////////////////
///
/// \file SkinnedModel.cpp
/// Implementation of the SkinnedModel class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SkinnedModel, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(Graphical);
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(Mesh);
  ZilchBindGetterSetterProperty(SkeletonPath);
}

void SkinnedModel::Serialize(Serializer& stream)
{
  Graphical::Serialize(stream);
  SerializeResourceName(mMesh, MeshManager);
  SerializeNameDefault(mSkeletonPath, CogPath("."));
}

void SkinnedModel::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);
  mSkeleton = nullptr;

  ConnectThisTo(MeshManager::GetInstance(), Events::ResourceModified, OnMeshModified);
  ConnectThisTo(&mSkeletonPath, Events::CogPathCogChanged, OnSkeletonPathChanged);
}

void SkinnedModel::OnAllObjectsCreated(CogInitializer& initializer)
{
  mSkeletonPath.RestoreLink(initializer, this, "SkeletonPath");
}

void SkinnedModel::DebugDraw()
{
  Obb obb = GetWorldObb();
  gDebugDraw->Add(Debug::Obb(obb).Color(Color::Wheat));

  if (mSkeleton != nullptr && mSkeleton->GetOwner() != GetOwner())
  {
    Array<String> boneNames;
    forRange (MeshBone& bone, mMesh->mBones.All())
      boneNames.PushBack(bone.mName);
    mSkeleton->DebugDrawSkeleton(boneNames);
  }
}

Aabb SkinnedModel::GetLocalAabb()
{
  return mMesh->mAabb;
}

void SkinnedModel::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  Array<Mat4>& skinningBuffer = frameBlock.mRenderQueues->mSkinningBuffer;
  Array<uint>& indexRemapBuffer = frameBlock.mRenderQueues->mIndexRemapBuffer;

  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Static;
  frameNode.mCoreVertexType = CoreVertexType::SkinnedMesh;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = mMesh->mRenderData;
  frameNode.mTextureRenderData = nullptr;

  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();
  frameNode.mLocalToWorldNormal = Math::BuildTransform(mTransform->GetWorldRotation(), mTransform->GetWorldScale());
  frameNode.mLocalToWorldNormal.Invert().Transpose();

  frameNode.mObjectWorldPosition = mTransform->GetWorldTranslation();

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
  frameNode.mIndexRemapRange = IndexRange(0, 0);

  // Try to resolve skeleton path first if null
  if (mSkeleton == nullptr)
    mSkeletonPath.RefreshIfNull();

  // If still no skeleton (or mesh is not skinned) then draw mesh as non-skinned
  if (mSkeleton == nullptr || mMesh->mBones.Size() == 0)
  {
    frameNode.mCoreVertexType = CoreVertexType::Mesh;
    return;
  }

  // If a skinned mesh object has a transform that brings it to its bind location,
  // then at every pose this transform has to be removed from each bone's bind transform.
  // This allows us to maintain the mesh object's transform without applying it twice
  // and also means that any animating that was done to the mesh object's transform is preserved.
  // This results in changing the series of transforms:
  //   world * hierarchyPose * boneBind
  // To:
  //   world * hierarchyPose * (boneSpaceBindOffsetInv) * boneBind
  // Where boneSpaceBindOffsetInv is the result of the similarity transform to boneSpace:
  //   hierarchyPoseInv * bindOffsetInv * hierarchyPose
  // This offset cannot be baked into the bone bind transforms because hierarchyPose is not constant.
  // However, we can simplify the whole transform from:
  //   world * hierarchyPose * (hierarchyPoseInv * bindOffsetInv * hierarchyPose) * boneBind
  // To:
  //   world * bindOffsetInv * hierarchyPose * boneBind
  // So that (hierarchyPose * boneBind) is the original transforms used for skinning as before,
  // and accounting for the mesh's bind offset is just a concatenation with the world matrix (world * bindOffsetInv)
  frameNode.mLocalToWorld = frameNode.mLocalToWorld * mMesh->mBindOffsetInv;
  frameNode.mLocalToWorldNormal = frameNode.mLocalToWorldNormal * Math::ToMatrix3(mMesh->mBindOffsetInv);

  frameNode.mBoneMatrixRange = mSkeleton->GetBoneTransforms(skinningBuffer, frameBlock.mRenderQueues->mSkinningBufferVersion);

  frameNode.mIndexRemapRange.start = indexRemapBuffer.Size();
  indexRemapBuffer.Append(mBoneIndexRemap.All());
  frameNode.mIndexRemapRange.end = indexRemapBuffer.Size();
}

void SkinnedModel::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  viewNode.mLocalToView = viewBlock.mWorldToView * frameNode.mLocalToWorld;
  viewNode.mLocalToViewNormal = Math::ToMatrix3(viewBlock.mWorldToView) * frameNode.mLocalToWorldNormal;
  viewNode.mLocalToPerspective = viewBlock.mViewToPerspective * viewNode.mLocalToView;
}

bool SkinnedModel::TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  if (mSkeleton != nullptr && mSkeleton->TestRay(rayCast))
    return true;
  else
    return Graphical::TestRay(rayCast, castInfo);
}

Mesh* SkinnedModel::GetMesh()
{
  return mMesh;
}

void SkinnedModel::SetMesh(Mesh* mesh)
{
  if (mesh == nullptr || mesh == mMesh)
    return;

  mMesh = mesh;
  UpdateBroadPhaseAabb();
  UpdateBoneIndexRemap();
}

CogPath SkinnedModel::GetSkeletonPath()
{
  return mSkeletonPath;
}

void SkinnedModel::SetSkeletonPath(CogPath path)
{
  // Disconnect events from previous Skeleton
  if (Cog* cog = mSkeletonPath.GetCog())
    DisconnectAll(cog, GetOwner());

  mSkeletonPath = path;
}

void SkinnedModel::UpdateBoneIndexRemap()
{
  mBoneIndexRemap.Clear();
  if (mSkeleton == nullptr)
    return;

  forRange (MeshBone& bone, mMesh->mBones.All())
  {
    // Remapped index is the index into mBoneIndexRemap
    if (mSkeleton->mNameMap.ContainsKey(bone.mName))
      mBoneIndexRemap.PushBack(mSkeleton->mNameMap[bone.mName]);
    else
      mBoneIndexRemap.PushBack(0);
  }
}

void SkinnedModel::OnMeshModified(ResourceEvent* event)
{
  if ((Mesh*)event->EventResource == mMesh)
    SetMesh(mMesh);
}

void SkinnedModel::OnSkeletonPathChanged(CogPathEvent* event)
{
  mSkeleton = nullptr;
  if (Cog* cog = mSkeletonPath.GetCog())
  {
    mSkeleton = cog->has(Skeleton);
    // Connect to skeleton changes for index remap
    ConnectThisTo(cog, Events::ComponentsModified, OnSkeletonComponentsChanged);
    ConnectThisTo(cog, Events::SkeletonModified, OnSkeletonModified);
    ConnectThisTo(cog, Events::SkeletonDestroyed, OnSkeletonDestroyed);
  }

  UpdateBoneIndexRemap();
}

void SkinnedModel::OnSkeletonComponentsChanged(Event* event)
{
  if (Cog* cog = mSkeletonPath.GetCog())
  {
    Skeleton* skeleton = cog->has(Skeleton);
    if (skeleton != mSkeleton)
    {
      mSkeleton = skeleton;
      UpdateBoneIndexRemap();
    }
  }
}

void SkinnedModel::OnSkeletonModified(Event* event)
{
  UpdateBoneIndexRemap();
}

void SkinnedModel::OnSkeletonDestroyed(Event* event)
{
  mSkeleton = nullptr;
  UpdateBoneIndexRemap();
}

} // namespace Zero
