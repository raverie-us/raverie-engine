///////////////////////////////////////////////////////////////////////////////
///
/// \file SkinnedModel.hpp
/// Declaration of the SkinnedModel component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Renders a mesh using the transform hierarchy of a Skeleton to apply skinning.
class SkinnedModel : public Graphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void DebugDraw() override;

  // Graphical Interface
  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
  bool TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo) override;

  // Properties

  /// Mesh that the graphical will render.
  Mesh* GetMesh();
  void SetMesh(Mesh* newMesh);
  HandleOf<Mesh> mMesh;

  /// Path to an object with a Skeleton component that will be used for skinning.
  CogPath GetSkeletonPath();
  void SetSkeletonPath(CogPath path);
  CogPath mSkeletonPath;

  /// Center of the bounding box used for culling
  Vec3 mLocalAabbCenter;
  /// Extents of the bounding box used for culling
  Vec3 mLocalAabbHalfExtents;

  // Internal

  void UpdateBoneIndexRemap();

  void OnMeshModified(ResourceEvent* event);
  void OnSkeletonPathChanged(CogPathEvent* event);
  void OnSkeletonComponentsChanged(Event* event);
  void OnSkeletonModified(Event* event);
  void OnSkeletonDestroyed(Event* event);

  Skeleton* mSkeleton;
  Array<uint> mBoneIndexRemap;
};

}// namespace Zero
