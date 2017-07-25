///////////////////////////////////////////////////////////////////////////////
///
/// \file HeightMapModel.cpp
/// Implementation of the HeightMapModel class.
///
/// Authors: Trevor Sundberg, Nathan Carlson
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const uint WeightTextureSize = 128;

//-------------------------------------------------------- GraphicalPatchIndices
GraphicalPatchIndices* GraphicalPatchIndices::GetInstance()
{
  static GraphicalPatchIndices sInstance;
  return &sInstance;
}

GraphicalPatchIndices::GraphicalPatchIndices()
{
  mIndices.Reserve(HeightPatch::NumIndicesTotal);
  for (uint y = 0; y < HeightPatch::NumQuadsPerSide; ++y)
  {
    for (uint x = 0; x < HeightPatch::NumQuadsPerSide; ++x)
    {
      uint topLeft = x + y * HeightPatch::NumVerticesPerSide;

      mIndices.PushBack(topLeft);
      mIndices.PushBack(topLeft + HeightPatch::NumVerticesPerSide);
      mIndices.PushBack(topLeft + HeightPatch::NumVerticesPerSide + 1);

      mIndices.PushBack(topLeft + HeightPatch::NumVerticesPerSide + 1);
      mIndices.PushBack(topLeft + 1);
      mIndices.PushBack(topLeft);
    }
  }
}

//--------------------------------------------------------------- HeightMapModel
ZilchDefineType(HeightMapModel, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindInterface(Graphical);
  ZeroBindDependency(HeightMap);
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

void HeightMapModel::Serialize(Serializer& stream)
{
  Graphical::Serialize(stream);
}

void HeightMapModel::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);

  // Disable for the terrain generations (#INF is used for skirts)
  // Not using #INF anymore
  //FpuExceptionsDisabler();

  // Grab the height-map component so we can read its data
  mMap = GetOwner()->has(HeightMap);

  // Loop through all the patches on the map
  forRange (HeightPatch* patch, mMap->GetAllPatches())
    CreateGraphicalPatchMesh(patch);

  RebuildLocalAabb();

  ConnectThisTo(GetOwner(), Events::HeightMapPatchAdded, OnPatchAdded);
  ConnectThisTo(GetOwner(), Events::HeightMapPatchRemoved, OnPatchRemoved);
  ConnectThisTo(GetOwner(), Events::HeightMapPatchModified, OnPatchModified);
  ConnectThisTo(GetOwner(), Events::HeightMapSave, OnSave);
}

void HeightMapModel::OnDestroy(uint flags)
{
  Graphical::OnDestroy(flags);

  // Have to manually clean up allocated weight textures
  forRange (GraphicalHeightPatch& patch, mGraphicalPatches.Values())
    delete patch.mWeightTexture;
}

Aabb HeightMapModel::GetLocalAabb()
{
  return mLocalAabb;
}

void HeightMapModel::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  GraphicalEntryData* entryData = frameNode.mGraphicalEntry->mData;

  PatchIndex patchIndex = *(PatchIndex*)&entryData->mUtility;
  HeightPatch* heightPatch = mMap->GetPatchAtIndex(patchIndex);
  GraphicalHeightPatch& graphicalPatch = mGraphicalPatches[heightPatch];

  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Static;
  frameNode.mCoreVertexType = CoreVertexType::Mesh;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = graphicalPatch.mMesh->mRenderData;
  frameNode.mTextureRenderData = graphicalPatch.mWeightTexture->Image->mRenderData;

  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();
  frameNode.mLocalToWorldNormal = Math::BuildTransform(mTransform->GetWorldRotation(), mTransform->GetWorldScale());
  frameNode.mLocalToWorldNormal.Invert().Transpose();

  frameNode.mObjectWorldPosition = mTransform->GetWorldTranslation();

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void HeightMapModel::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  viewNode.mLocalToView = viewBlock.mWorldToView * frameNode.mLocalToWorld;
  viewNode.mLocalToViewNormal = Math::ToMatrix3(viewBlock.mWorldToView) * frameNode.mLocalToWorldNormal;
  viewNode.mLocalToPerspective = viewBlock.mViewToPerspective * viewNode.mLocalToView;
}

void HeightMapModel::MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum)
{
  typedef HashMap<HeightPatch*, GraphicalHeightPatch>::pair GraphicalPatchPair;
  if (frustum == nullptr)
  {
    forRange (GraphicalPatchPair& pair, mGraphicalPatches.All())
    {
      HeightPatch* heightPatch = pair.first;
      GraphicalHeightPatch& graphicalPatch = pair.second;

      AddGraphicalPatchEntry(entries, graphicalPatch, heightPatch->Index);
    }
  }
  else
  {
    Mat4 worldMatrix = mTransform->GetWorldMatrix();
    forRange (GraphicalPatchPair& pair, mGraphicalPatches.All())
    {
      HeightPatch* heightPatch = pair.first;
      GraphicalHeightPatch& graphicalPatch = pair.second;

      Aabb aabb = graphicalPatch.mLocalAabb.TransformAabb(worldMatrix);
      if (Overlap(aabb, *frustum))
        AddGraphicalPatchEntry(entries, graphicalPatch, heightPatch->Index);
    }
  }
}

bool HeightMapModel::TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  HeightMapRayRange results = mMap->CastWorldRay(rayCast.mRay);

  if (results.Empty())
    return false;

  HeightMapRayRange::TriangleInfo& contact = results.Front();
  rayCast.mObject = GetOwner();
  rayCast.mT = contact.mIntersectionInfo.T;

  return true;
}

String HeightMapModel::GetDefaultMaterialName()
{
  return "DefaultHeightMapMaterial";
}

void HeightMapModel::AddGraphicalPatchEntry(Array<GraphicalEntry>& entries, GraphicalHeightPatch& graphicalPatch, PatchIndex index)
{
  GraphicalEntryData& entryData = graphicalPatch.mGraphicalEntryData;
  entryData.mGraphical = this;
  entryData.mFrameNodeIndex = -1;
  entryData.mPosition = mTransform->GetWorldTranslation();
  entryData.mUtility = *(u64*)&index;

  GraphicalEntry entry;
  entry.mData = &entryData;
  entry.mSort = 0;
      
  entries.PushBack(entry);
}

void HeightMapModel::OnPatchAdded(HeightMapEvent* event)
{
  HeightPatch* heightPatch = event->Patch;

  Aabb patchAabb = mMap->GetPatchLocalAabb(heightPatch);
  if (mGraphicalPatches.Empty())
    mLocalAabb = patchAabb;
  else
    mLocalAabb.Combine(patchAabb);

  UpdateBroadPhaseAabb();

  CreateGraphicalPatchMesh(heightPatch);
}

void HeightMapModel::OnPatchRemoved(HeightMapEvent* event)
{
  HeightPatch* heightPatch = event->Patch;

  ErrorIf(!mGraphicalPatches.ContainsKey(heightPatch), "No GraphicalHeightPatch found for HeightPatch.");
  GraphicalHeightPatch& graphicalHeightPatch = mGraphicalPatches[heightPatch];
  delete graphicalHeightPatch.mWeightTexture;
  mGraphicalPatches.Erase(heightPatch);
  RebuildLocalAabb();
}

void HeightMapModel::OnPatchModified(HeightMapEvent* event)
{
  HeightPatch* heightPatch = event->Patch;

  CreateGraphicalPatchMesh(heightPatch);
  RebuildLocalAabb();
}

void HeightMapModel::OnSave(HeightMapEvent* event)
{
  HeightMapSource* source = event->Source;

  typedef Pair<HeightPatch*, GraphicalHeightPatch> PairType;
  forRange (PairType patchPair, mGraphicalPatches.All())
  {
    GraphicalHeightPatch& patch = patchPair.second;
    PatchLayer* layer = source->GetLayerData(patchPair.first->Index, PatchLayerType::Weights);
    layer->LayerType = PatchLayerType::Weights;
    layer->ElementSize = sizeof(ByteColor);
    layer->Width = WeightTextureSize;
    layer->Height = WeightTextureSize;
    uint size = layer->Size();
    layer->Allocate();
    memcpy(layer->Data, patch.mWeightTexture->Data, size);
  }
}

void HeightMapModel::RebuildLocalAabb()
{
  if (mGraphicalPatches.Empty())
  {
    mLocalAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(0.5f));
  }
  else
  {
    mLocalAabb.SetInvalid();
    forRange (GraphicalHeightPatch& graphicalPatch, mGraphicalPatches.Values())
      mLocalAabb.Combine(graphicalPatch.mLocalAabb);
  }

  UpdateBroadPhaseAabb();
}

void HeightMapModel::CreateGraphicalPatchMesh(HeightPatch* heightPatch)
{
  Aabb patchAabb = mMap->GetPatchLocalAabb(heightPatch);
  GraphicalHeightPatch& graphicalPatch = mGraphicalPatches[heightPatch];
  graphicalPatch.mLocalAabb = patchAabb;
  graphicalPatch.mMesh = Mesh::CreateRuntime();

  Mesh* mesh = graphicalPatch.mMesh;
  mesh->mPrimitiveType = PrimitiveType::Triangles;
  mesh->mAabb = patchAabb;
  VertexBuffer* vertices = &mesh->mVertices;
  IndexBuffer* indices = &mesh->mIndices;

  // Get the local space vertices
  Array<Vec3> positions;
  uint paddedWidth = HeightPatch::PaddedNumVerticesPerSide;
  PatchIndex patchIndex = heightPatch->Index;
  mMap->GetPaddedHeightPatchVertices(heightPatch, positions);

  vertices->AddAttribute(VertexSemantic::Position, VertexElementType::Real, 3);
  vertices->AddAttribute(VertexSemantic::Uv, VertexElementType::Real, 2);
  vertices->AddAttribute(VertexSemantic::Normal, VertexElementType::Real, 3);
  vertices->AddAttribute(VertexSemantic::Tangent, VertexElementType::Real, 3);
  vertices->AddAttribute(VertexSemantic::Bitangent, VertexElementType::Real, 3);

  // Iterate over the middle of the padded area and adjacent indexing will always be valid
  for (uint y = 1; y < paddedWidth - 1; ++y)
  {
    for (uint x = 1; x < paddedWidth - 1; ++x)
    {
      uint index = x + y * paddedWidth;

      Vec3 pos = positions[index];

      Vec2 uv;
      uv.x = (x - 1) / (real)(HeightPatch::NumVerticesPerSide - 1);
      uv.y = (y - 1) / (real)(HeightPatch::NumVerticesPerSide - 1);

      // Sample adjacent positions
      Vec3 n = positions[index - paddedWidth];
      Vec3 s = positions[index + paddedWidth];
      Vec3 w = positions[index - 1];
      Vec3 e = positions[index + 1];

      // Use the west->east vector as the tangent
      Vec3 tangent = e - w;
      tangent.Normalize();

      // Use the north->south vector as the bitangent
      Vec3 bitangent = s - n;
      bitangent.Normalize();

      // Cross the tangent and bitangent to determine our normal
      Vec3 normal = Math::Cross(bitangent, tangent);
      normal.Normalize();

      vertices->AddReal(pos);
      vertices->AddReal(uv);
      vertices->AddReal(normal);
      vertices->AddReal(tangent);
      vertices->AddReal(bitangent);
    }
  }

  indices->mData = GraphicalPatchIndices::GetInstance()->mIndices;
  indices->mIndexSize = 4;
  indices->mIndexCount = indices->mData.Size();
  indices->mGenerated = false;

  mesh->Upload();

  // Load the splat control texture
  ByteColor color = ToByteColor(Vec4(1,0,0,0));

  if (graphicalPatch.mWeightTexture == nullptr)
  {
    graphicalPatch.mWeightTexture = new PixelBuffer(color, WeightTextureSize, WeightTextureSize);
    graphicalPatch.mWeightTexture->Image->mAddressingX = TextureAddressing::Clamp;
    graphicalPatch.mWeightTexture->Image->mAddressingY = TextureAddressing::Clamp;
    graphicalPatch.mWeightTexture->Image->mFiltering = TextureFiltering::Bilinear;
    graphicalPatch.mWeightTexture->Upload();

    if (mMap->mSource)
    {
      PatchLayer* patchLayer = mMap->mSource->GetLayerData(heightPatch->Index, PatchLayerType::Weights);
      if (patchLayer->Data)
        graphicalPatch.mWeightTexture->SetAll((byte*)patchLayer->Data);
    }
  }
}

} // namespace Zero
