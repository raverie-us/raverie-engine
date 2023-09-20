// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
#pragma pack(push, 4)
class FixedVertexDescription
{
public:
  FixedVertexDescription(){};

  uint mVertexSize;
  static const size_t sMaxElements = 16;
  VertexAttribute mAttributes[sMaxElements];
};
#pragma pack(pop)

const String cMeshOutputType = "Mesh";

const uint MeshFileId = 'zmsh';
const uint MeshFileVersion = 1;

const uint VertexChunk = 'vert';
const uint IndexChunk = 'indx';
const uint SkeletonChunk = 'skel';

#pragma pack(push, 4)
class MeshHeader
{
public:
  uint mFileId;
  Aabb mAabb;
  ByteEnum<PrimitiveType::Enum> mPrimitiveType;
  Mat4 mBindOffsetInv;
};
#pragma pack(pop)

/// Geometry content item that builds meshes.
class MeshBuilder : public BuilderComponent
{
public:
  RaverieDeclareType(MeshBuilder, TypeCopyMode::ReferenceType);

  MeshBuilder();

  bool mCombineMeshes;
  bool mGenerateSmoothNormals;
  float mSmoothingAngleDegreesThreshold;
  bool mGenerateTangentSpace;
  bool mInvertUvYAxis;
  bool mFlipWindingOrder;
  bool mFlipNormals;

  Array<GeometryResourceEntry> Meshes;

  // BuilderComponent Interface
  bool NeedsBuilding(BuildOptions& options) override;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildListing(ResourceListing& listing) override;
};

} // namespace Raverie
