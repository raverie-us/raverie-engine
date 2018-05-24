//////////////////////////////////////////////////////////////////////////
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class FixedVertexDescription
{
public:
  FixedVertexDescription() {};

  uint mVertexSize;
  static const size_t sMaxElements = 16;
  VertexAttribute mAttributes[sMaxElements];
};

const String cMeshOutputType = "Mesh";

const uint MeshFileId = 'zmsh';
const uint MeshFileVersion = 1;

const uint VertexChunk = 'vert';
const uint IndexChunk = 'indx';
const uint SkeletonChunk = 'skel';

class MeshHeader
{
public:
  uint mFileId;
  Aabb mAabb;
  PrimitiveType::Enum mPrimitiveType;
  Mat4 mBindOffsetInv;
};

/// Geometry content item that builds meshes.
class MeshBuilder : public BuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MeshBuilder();

  bool mCombineMeshes;
  bool mGenerateSmoothNormals;
  float mSmoothingAngleDegreesThreshold;
  bool mGenerateTangentSpace;
  bool mInvertUvYAxis;
  bool mFlipWindingOrder;
  bool mFlipNormals;

  Array<GeometryResourceEntry> Meshes;

  //BuilderComponent Interface
  bool NeedsBuilding(BuildOptions& options) override;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildListing(ResourceListing& listing) override;
};

}// namespace Zero