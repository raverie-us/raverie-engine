//////////////////////////////////////////////////////////////////////////
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

const size_t cMaxBonesWeights = 4;

// Do not reorder, value used to calculate number of vertices per primitive
DeclareEnum3(PrimitiveType, Points, Lines, Triangles);

// Semantics are used to communicate what this element is
// with the vertex shader
DeclareEnum17(VertexSemantic,
              Position,
              Normal,
              Tangent,
              Bitangent,
              Uv,
              UvAux,
              Color,
              ColorAux,
              BoneWeights,
              BoneIndices,
              Aux0,
              Aux1,
              Aux2,
              Aux3,
              Aux4,
              Aux5,
              None);

// Type of element in vertex
DeclareEnum6(VertexElementType,
             Byte,
             Short,
             Half,
             Real,
             // Normalized Types map 0 to 1
             NormByte,
             NormShort);

DeclareEnum3(IndexElementType,
             Byte,
             Ushort,
             Uint
);

class VertexAttribute
{
public:
  VertexAttribute() {};
  VertexAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum type, byte count, byte offset);

  VertexSemantic::Enum mSemantic : 8;
  VertexElementType::Enum mType : 8;
  byte mCount;
  byte mOffset;
};

class FixedVertexDescription
{
public:
  FixedVertexDescription() {};

  uint mVertexSize;
  static const size_t sMaxElements = 16;
  VertexAttribute mAttributes[sMaxElements];
};

class MeshBone
{
public:
  String mName;
  Mat4 mBindTransform;
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

  Array<MeshEntry> Meshes;

  //BuilderComponent Interface
  bool NeedsBuilding(BuildOptions& options) override;
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildListing(ResourceListing& listing) override;
};

}// namespace Zero