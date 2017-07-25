///////////////////////////////////////////////////////////////////////////////
///
/// \file HeightMapModel.hpp
/// Declaration of the HeightMapModel class.
///
/// Authors: Trevor Sundberg, Nathan Carlson
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------- GraphicalPatchIndices
class GraphicalPatchIndices
{
public:
  static GraphicalPatchIndices* GetInstance();

  GraphicalPatchIndices();
  Array<uint> mIndices;
};

//--------------------------------------------------------- GraphicalHeightPatch
class GraphicalHeightPatch
{
public:
  GraphicalHeightPatch() : mWeightTexture(nullptr) {}

  Aabb mLocalAabb;
  HandleOf<Mesh> mMesh;
  GraphicalEntryData mGraphicalEntryData;

  // A pixel buffer we use to paint materials onto the height map (splatting)
  // Each channel in the pixel buffer corresponds to a weight of how much
  // we read from each of the 4 listed materials below
  PixelBuffer* mWeightTexture;
};

/// Generates a graphical mesh from every patch of height data in the HeightMap component.
class HeightMapModel : public Graphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;

  // Graphical Interface

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
  void MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum) override;
  bool TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo) override;
  String GetDefaultMaterialName() override;

  // Internal

  void AddGraphicalPatchEntry(Array<GraphicalEntry>& entries, GraphicalHeightPatch& graphicalPatch, PatchIndex index);

  void OnPatchAdded(HeightMapEvent* event);
  void OnPatchRemoved(HeightMapEvent* event);
  void OnPatchModified(HeightMapEvent* event);
  void OnSave(HeightMapEvent* event);

  void RebuildLocalAabb();
  void CreateGraphicalPatchMesh(HeightPatch* heightPatch);

  // Store a pointer back to the height map
  HeightMap* mMap;
  // Total bounding volume in local space
  Aabb mLocalAabb;
  // We need to associate each patch with graphical data
  HashMap<HeightPatch*, GraphicalHeightPatch> mGraphicalPatches;
};

} // namespace Zero
