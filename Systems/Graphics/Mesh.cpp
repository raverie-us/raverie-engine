///////////////////////////////////////////////////////////////////////////////
///
/// \file Mesh.cpp
/// Implementation of the Mesh class and Manager.
///
/// Authors: Chris Peters, Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace
{
  const float cMinMeshThickness = 0.025f;
}

namespace Zero
{

VertexSemanticRange::VertexSemanticRange(const FixedVertexDescription& fixedDesc)
  : mFixedDesc(fixedDesc)
  , mCurrentIndex(0)
{
}

bool VertexSemanticRange::Empty()
{
  if (mCurrentIndex >= mFixedDesc.sMaxElements)
    return true;
  else
    return mFixedDesc.mAttributes[mCurrentIndex].mSemantic == VertexSemantic::None;
}

VertexSemantic::Enum VertexSemanticRange::Front()
{
  return mFixedDesc.mAttributes[mCurrentIndex].mSemantic;
}

void VertexSemanticRange::PopFront()
{
  ++mCurrentIndex;
}

//----------------------------------------------------------------- VertexBuffer
ZilchDefineType(VertexBuffer, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(AddAttribute);
  ZilchBindMethod(GetAttributes);

  ZilchBindOverloadedMethod(AddByte,  ZilchInstanceOverload(void, int));
  ZilchBindOverloadedMethod(AddByte,  ZilchInstanceOverload(void, IntVec2));
  ZilchBindOverloadedMethod(AddByte,  ZilchInstanceOverload(void, IntVec3));
  ZilchBindOverloadedMethod(AddByte,  ZilchInstanceOverload(void, IntVec4));
  ZilchBindOverloadedMethod(AddShort, ZilchInstanceOverload(void, int));
  ZilchBindOverloadedMethod(AddShort, ZilchInstanceOverload(void, IntVec2));
  ZilchBindOverloadedMethod(AddShort, ZilchInstanceOverload(void, IntVec3));
  ZilchBindOverloadedMethod(AddShort, ZilchInstanceOverload(void, IntVec4));
  ZilchBindOverloadedMethod(AddReal,  ZilchInstanceOverload(void, real));
  ZilchBindOverloadedMethod(AddReal,  ZilchInstanceOverload(void, Vec2));
  ZilchBindOverloadedMethod(AddReal,  ZilchInstanceOverload(void, Vec3));
  ZilchBindOverloadedMethod(AddReal,  ZilchInstanceOverload(void, Vec4));
  ZilchBindOverloadedMethod(GetVertexData, ZilchInstanceOverload(Vec4, uint, VertexSemantic::Enum));
  ZilchBindOverloadedMethod(GetVertexData, ZilchInstanceOverload(Vec4, uint, VertexSemantic::Enum, VertexElementType::Enum, uint));
  ZilchBindMethod(IsValidVertexData);

  ZilchBindMethod(ClearAttributes);
  ZilchBindMethod(ClearData);

  ZilchBindMethod(GetElementType);
  ZilchBindMethod(GetElementCount);
  ZilchBindGetterProperty(VertexCount);
}

VertexBuffer::VertexBuffer()
{
  mFixedDesc.mVertexSize = 0;
  mFixedDesc.mAttributes[0].mSemantic = VertexSemantic::None;
  mData = nullptr;
  mDataCapacity = 0;
  mDataSize = 0;
}

VertexBuffer::~VertexBuffer()
{
  delete[] mData;
}

void VertexBuffer::AddAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum elementType, uint elementCount)
{
  if (elementCount < 1 || elementCount > 4)
    DoNotifyException("Invalid Element Count", "Vertex attributes must be between 1 and 4 total elements.");

  for (uint i = 0; i < mFixedDesc.sMaxElements; ++i)
  {
    VertexAttribute& attr = mFixedDesc.mAttributes[i];
    if (attr.mSemantic == semantic)
    {
      DoNotifyException("Invalid Semantic", "Semantic given has already been specified.");
    }
    else if (attr.mSemantic == VertexSemantic::None)
    {
      uint elementSize = 1;
      switch (elementType)
      {
        case VertexElementType::Byte:
        case VertexElementType::NormByte:
          elementSize = 1;
        break;

        case VertexElementType::Short:
        case VertexElementType::NormShort:
          elementSize = 2;
        break;

        case VertexElementType::Real:
          elementSize = 4;
        break;

        case VertexElementType::Half:
        default:
          DoNotifyException("Invalid Element Type", "Unsupported.");
          return;
        break;
      }

      attr.mSemantic = semantic;
      attr.mType = elementType;
      attr.mCount = elementCount;
      attr.mOffset = mFixedDesc.mVertexSize;

      mFixedDesc.mVertexSize += elementSize * elementCount;

      // Set next element to None
      if (i + 1 < mFixedDesc.sMaxElements)
        mFixedDesc.mAttributes[i + 1].mSemantic = VertexSemantic::None;

      return;
    }
  }
}

VertexSemanticRange VertexBuffer::GetAttributes()
{
  return VertexSemanticRange(mFixedDesc);
}

void VertexBuffer::AddByte(int value)
{
  WriteData((byte)value);
}

void VertexBuffer::AddByte(IntVec2 value)
{
  WriteData((byte)value.x);
  WriteData((byte)value.y);
}

void VertexBuffer::AddByte(IntVec3 value)
{
  WriteData((byte)value.x);
  WriteData((byte)value.y);
  WriteData((byte)value.z);
}

void VertexBuffer::AddByte(IntVec4 value)
{
  WriteData((byte)value.x);
  WriteData((byte)value.y);
  WriteData((byte)value.z);
  WriteData((byte)value.w);
}

void VertexBuffer::AddShort(int value)
{
  WriteData((u16)value);
}

void VertexBuffer::AddShort(IntVec2 value)
{
  WriteData((u16)value.x);
  WriteData((u16)value.y);
}

void VertexBuffer::AddShort(IntVec3 value)
{
  WriteData((u16)value.x);
  WriteData((u16)value.y);
  WriteData((u16)value.z);
}

void VertexBuffer::AddShort(IntVec4 value)
{
  WriteData((u16)value.x);
  WriteData((u16)value.y);
  WriteData((u16)value.z);
  WriteData((u16)value.w);
}

void VertexBuffer::AddReal(real value)
{
  WriteData(value);
}

void VertexBuffer::AddReal(Vec2 value)
{
  WriteData(value);
}

void VertexBuffer::AddReal(Vec3 value)
{
  WriteData(value);
}

void VertexBuffer::AddReal(Vec4 value)
{
  WriteData(value);
}

Vec4 VertexBuffer::GetVertexData(uint vertexIndex, VertexSemantic::Enum semantic)
{
  Vec4 value = Vec4::cZero;

  VertexAttribute attribute = GetAttribute(semantic);
  if (attribute.mSemantic == VertexSemantic::None)
    return value;

  uint elementSize = GetElementSize(attribute.mType);
  uint vertexOffset = (mFixedDesc.mVertexSize * vertexIndex) + attribute.mOffset;
  if (vertexOffset + elementSize * attribute.mCount > mDataSize)
    return value;

  byte* vertexData = mData + vertexOffset;
  ReadVertexData(vertexData, attribute, value);

  return value;
}

Vec4 VertexBuffer::GetVertexData(uint vertexIndex, VertexSemantic::Enum semantic, VertexElementType::Enum type, uint count)
{
  Vec4 value = Vec4::cZero;

  VertexAttribute attribute = GetAttribute(semantic);
  if (attribute.mSemantic == VertexSemantic::None)
  {
    DoNotifyException("Invalid Read", "Vertex semantic not found.");
    return value;
  }

  if (attribute.mType != type)
  {
    DoNotifyException("Invalid Read", "Element type does not match.");
    return value;
  }

  if (attribute.mCount != count)
  {
    DoNotifyException("Invalid Read", "Element count does not match.");
    return value;
  }

  uint elementSize = GetElementSize(attribute.mType);
  uint vertexOffset = (mFixedDesc.mVertexSize * vertexIndex) + attribute.mOffset;
  if (vertexOffset + elementSize * attribute.mCount > mDataSize)
  {
    DoNotifyException("Invalid Read", "No vertex data at index.");
    return value;
  }

  byte* vertexData = mData + vertexOffset;
  ReadVertexData(vertexData, attribute, value);

  return value;
}

bool VertexBuffer::IsValidVertexData(uint vertexIndex, VertexSemantic::Enum semantic, VertexElementType::Enum type, uint count)
{
  VertexAttribute attribute = GetAttribute(semantic);
  if (attribute.mSemantic == VertexSemantic::None)
    return false;

  if (attribute.mType != type)
    return false;

  if (attribute.mCount != count)
    return false;

  uint elementSize = GetElementSize(attribute.mType);
  uint vertexOffset = (mFixedDesc.mVertexSize * vertexIndex) + attribute.mOffset;
  if (vertexOffset + elementSize * attribute.mCount > mDataSize)
    return false;

  return true;
}

void VertexBuffer::ClearAttributes()
{
  mFixedDesc.mVertexSize = 0;
  mFixedDesc.mAttributes[0].mSemantic = VertexSemantic::None;
}

void VertexBuffer::ClearData()
{
  mDataSize = 0;
}

VertexElementType::Enum VertexBuffer::GetElementType(VertexSemantic::Enum semantic)
{
  VertexAttribute attribute = GetAttribute(semantic);
  if (attribute.mSemantic == VertexSemantic::None)
    DoNotifyException("Invalid Read", "Vertex semantic not found.");
  return attribute.mType;
}

uint VertexBuffer::GetElementCount(VertexSemantic::Enum semantic)
{
  VertexAttribute attribute = GetAttribute(semantic);
  if (attribute.mSemantic == VertexSemantic::None)
    DoNotifyException("Invalid Read", "Vertex semantic not found.");
  return attribute.mCount;
}

uint VertexBuffer::GetVertexCount()
{
  if (mFixedDesc.mVertexSize == 0)
    return 0;
  return mDataSize / mFixedDesc.mVertexSize;
}

void VertexBuffer::Grow(uint minExtra)
{
  uint newCapacity = mDataCapacity == 0 ? 128 : mDataCapacity * 2;
  newCapacity = Math::Max(newCapacity, mDataSize + minExtra);

  byte* newData = new byte[newCapacity];
  memcpy(newData, mData, mDataSize);
  delete[] mData;

  mData = newData;
  mDataCapacity = newCapacity;
}

VertexAttribute VertexBuffer::GetAttribute(VertexSemantic::Enum semantic)
{
  VertexAttribute attribute;
  attribute.mSemantic = VertexSemantic::None;

  for (uint i = 0; i < mFixedDesc.sMaxElements; ++i)
  {
    if (mFixedDesc.mAttributes[i].mSemantic == VertexSemantic::None)
      return mFixedDesc.mAttributes[i];

    if (mFixedDesc.mAttributes[i].mSemantic == semantic)
      return mFixedDesc.mAttributes[i];
  }

  return attribute;
}

uint VertexBuffer::GetElementSize(VertexElementType::Enum type)
{
  switch (type)
  {
    case VertexElementType::Byte:      return 1;
    case VertexElementType::Short:     return 2;
    case VertexElementType::Real:      return 4;
    case VertexElementType::NormByte:  return 1;
    case VertexElementType::NormShort: return 2;
    default: return 0;
  }
}

void VertexBuffer::ReadVertexData(byte* vertexData, VertexAttribute& attribute, Vec4& output)
{
  for (uint i = 0; i < attribute.mCount; ++i)
  {
    switch (attribute.mType)
    {
      case VertexElementType::Byte:      output[i] =   ((u8*)vertexData)[i]; break;
      case VertexElementType::Short:     output[i] =  ((u16*)vertexData)[i]; break;
      case VertexElementType::Real:      output[i] = ((real*)vertexData)[i]; break;
      case VertexElementType::NormByte:  output[i] =   ((u8*)vertexData)[i] / 255.0f;   break;
      case VertexElementType::NormShort: output[i] =  ((u16*)vertexData)[i] / 65535.0f; break;
    }
  }
}

//------------------------------------------------------------------ IndexBuffer
ZilchDefineType(IndexBuffer, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Count);

  ZilchBindMethod(Add);
  ZilchBindMethod(Get);
  ZilchBindMethod(Clear);
}

IndexBuffer::IndexBuffer()
{
  mIndexSize = 4;
  mIndexCount = 0;
  mGenerated = true;
}

IndexBuffer::~IndexBuffer()
{
}

uint IndexBuffer::GetCount()
{
  return mIndexCount;
}

void IndexBuffer::SetCount(uint count)
{
  mData.Clear();
  mIndexCount = count;
  mGenerated = false;
}

void IndexBuffer::Add(uint value)
{
  mData.PushBack(value);
  ++mIndexCount;
  mGenerated = false;
}

uint IndexBuffer::Get(uint index)
{
  if (index >= mIndexCount)
  {
    DoNotifyException("Invalid index", "Out of range.");
    return 0;
  }

  if (mData.Size() == 0)
    return index;
  else
    return mData[index];
}

void IndexBuffer::Clear()
{
  mData.Clear();
  mIndexCount = 0;
  mGenerated = true;
}

//------------------------------------------------------------------------- Mesh
ZilchDefineType(Mesh, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(CreateRuntime);

  ZilchBindFieldGetter(mVertices);
  ZilchBindFieldGetter(mIndices);
  ZilchBindFieldProperty(mPrimitiveType);

  ZilchBindMethod(Upload);
}

HandleOf<Mesh> Mesh::CreateRuntime()
{
  Mesh* mesh = MeshManager::CreateRuntime();
  mesh->mBuildTree = false;
  mesh->mAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(0.5f));

  Z::gEngine->has(GraphicsEngine)->AddMesh(mesh);
  return mesh;
}

Mesh::Mesh()
{
  mRenderData = nullptr;
  mPrimitiveType = PrimitiveType::Triangles;
}

void Mesh::Unload()
{
  mAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(0.5f));
  mPrimitiveType = PrimitiveType::Triangles;
  mVertices.ClearAttributes();
  mVertices.ClearData();
  mIndices.Clear();

  mBuildTree = true;
}

void Mesh::Upload()
{
  if (!IsRuntime())
    DoNotifyException("Invalid Upload", "Cannot upload to a non-runtime Mesh.");
  
  uint vertexSize = mVertices.mFixedDesc.mVertexSize;
  
  // Generate index data if none given
  if (mIndices.mGenerated && vertexSize != 0)
  {
    uint vertexCount = mVertices.mDataSize / vertexSize;
    mIndices.mIndexCount = vertexCount;
  }

  if (mBuildTree)
    BuildTree();
  
  Z::gEngine->has(GraphicsEngine)->AddMesh(this);
}

uint Mesh::GetPrimitiveCount()
{
  uint verticesPerPrimitve = GetVerticesPerPrimitive();
  return mIndices.mIndexCount / verticesPerPrimitve;
}

uint Mesh::GetVerticesPerPrimitive()
{
  return mPrimitiveType + 1;
}

void Mesh::BuildTree()
{
  mTree.Clear();

  uint verticesPerPrimitve = GetVerticesPerPrimitive();
  uint primitiveCount = mIndices.mIndexCount / verticesPerPrimitve;

  Aabb boundingBox;
  boundingBox.SetInvalid();

  for (uint i = 0; i < primitiveCount; ++i)
  {
    Vec3 points[3];
    bool hasPositions = GetPrimitiveData(i, VertexSemantic::Position, VertexElementType::Real, 3, points);
    if (hasPositions == false)
      continue;

    BaseBroadPhaseData<uint> data;
    data.mClientData = i;
    data.mAabb.Compute(points, verticesPerPrimitve);

    // Accumulate unaltered aabb's for final bounding box
    boundingBox.Combine(data.mAabb);

    // Make sure tree entries do not have thicknesses of zero
    Vec3 halfExtents = data.mAabb.GetHalfExtents();
    halfExtents = Math::Max(Vec3(cMinMeshThickness), halfExtents);
    data.mAabb.SetCenterAndHalfExtents(data.mAabb.GetCenter(), halfExtents);

    BroadPhaseProxy proxy;
    mTree.CreateProxy(proxy, data);
  }

  if (boundingBox.Valid() == false)
    boundingBox.SetCenterAndHalfExtents(Vec3::cZero, Vec3(cMinMeshThickness));

  // Make sure final bounding box does not have thicknesses of zero
  Vec3 halfExtents = boundingBox.GetHalfExtents();
  halfExtents = Math::Max(Vec3(cMinMeshThickness), halfExtents);
  boundingBox.SetCenterAndHalfExtents(boundingBox.GetCenter(), halfExtents);

  mAabb = boundingBox;
}

bool Mesh::TestRay(GraphicsRayCast& raycast, Mat4 worldTransform)
{
  Ray localRay = raycast.mRay.TransformInverse(worldTransform);

  Intersection::IntersectionPoint closestPoint;
  closestPoint.T = Math::PositiveMax();
  uint closestPrimitive = (uint)-1;

  forRangeBroadphaseTree (AvlDynamicAabbTree<uint>, mTree, Ray, localRay)
  {
    uint primitiveIndex = range.Front();

    Vec3 points[3];
    bool hasPositions = GetPrimitiveData(primitiveIndex, VertexSemantic::Position, VertexElementType::Real, 3, points);
    if (hasPositions == false)
      continue;

    Intersection::IntersectionPoint point;
    Intersection::Type result;

    switch (mPrimitiveType)
    {
      case Zero::PrimitiveType::Triangles:
        result = Intersection::RayTriangle(localRay.Start, localRay.Direction, points[0], points[1], points[2], &point);
      break;
      case Zero::PrimitiveType::Lines:
        result = Intersection::RayCapsule(localRay.Start, localRay.Direction, points[0], points[1], cMinMeshThickness, &point);
      break;
      case Zero::PrimitiveType::Points:
        result = Intersection::RaySphere(localRay.Start, localRay.Direction, points[0], cMinMeshThickness, &point);
      break;
    }
    
    if (result == Intersection::None)
      continue;

    if (point.T < closestPoint.T)
    {
      closestPoint = point;
      closestPrimitive = primitiveIndex;
    }
  }

  if (closestPrimitive == (uint)-1)
    return false;

  Vec3 point = closestPoint.Points[0];
  raycast.mPosition = point;
  raycast.mT = closestPoint.T;

  Vec3 points[3];
  GetPrimitiveData(closestPrimitive, VertexSemantic::Position, VertexElementType::Real, 3, points);

  // Compute weights to get interpolated normal and uv at point of intersection
  Vec3 weights = Vec3::cZero;
  switch (mPrimitiveType)
  {
    case Zero::PrimitiveType::Triangles:
      Geometry::BarycentricTriangle(point, points[0], points[1], points[2], &weights);
    break;
    case Zero::PrimitiveType::Lines:
    {
      float length0 = (point - points[0]).Length();
      float length1 = (points[1] - points[0]).Length();
      if (length1 < Math::Epsilon())
        weights.y = 0.0f;
      else
        weights.y = length0 / length1;
      weights.x = 1.0f - weights.y;
    }
    break;
    case Zero::PrimitiveType::Points:
      weights.x = 1.0f;
    break;
  }

  Vec3 normals[3];
  bool hasNormals = GetPrimitiveData(closestPrimitive, VertexSemantic::Normal, VertexElementType::Real, 3, normals);
  if (hasNormals)
  {
    raycast.mNormal = normals[0] * weights.x + normals[1] * weights.y + normals[2] * weights.z;
    Math::AttemptNormalize(raycast.mNormal);
  }
  else if (mPrimitiveType == PrimitiveType::Triangles)
  {
    // If mesh doesn't have normals, one will be calculated facing outward for CCW triangles
    raycast.mNormal = Math::Cross(points[1] - points[0], points[2] - points[0]);
    Math::AttemptNormalize(raycast.mNormal);
  }
  else
    raycast.mNormal = Vec3::cZero;

  Vec2 uvs[3];
  bool hasUvs = GetPrimitiveData(closestPrimitive, VertexSemantic::Uv, VertexElementType::Real, 2, uvs);
  if (hasUvs)
    raycast.mUv = uvs[0] * weights.x + uvs[1] * weights.y + uvs[2] * weights.z;
  else
    raycast.mUv = Vec2::cZero;

  return true;
}

bool Mesh::TestFrustum(const Frustum& frustum)
{
  forRangeBroadphaseTree (AvlDynamicAabbTree<uint>, mTree, Frustum, frustum)
  {
    uint primitiveIndex = range.Front();

    Vec3 points[3];
    bool hasPositions = GetPrimitiveData(primitiveIndex, VertexSemantic::Position, VertexElementType::Real, 3, points);
    if (hasPositions == false)
      continue;

    switch (mPrimitiveType)
    {
      case Zero::PrimitiveType::Triangles:
        if (Overlap(frustum, Triangle(points[0], points[1], points[2]))) return true;
      break;
      case Zero::PrimitiveType::Lines:
        if (Overlap(frustum, Capsule(points[0], points[1], cMinMeshThickness))) return true;
      break;
      case Zero::PrimitiveType::Points:
        if (Overlap(frustum, Sphere(points[0], cMinMeshThickness))) return true;
      break;
    }
  }

  return false;
}

uint GetIndexSize(IndexElementType::Enum indexType)
{
  switch (indexType)
  {
   case IndexElementType::Byte:
    return 1;
   case IndexElementType::Ushort:
    return 2;
   case IndexElementType::Uint:
    return 4;
  }
  return 0;
}

template<typename T>
void FillIndexBuffer(IndexBuffer* indexBuffer, byte* indexBufferData, uint indexCount)
{
  T* indexData = (T*)indexBufferData;
  for (uint i = 0; i < indexCount; ++i)
    indexBuffer->Add(indexData[i]);
}

// --------------------
// vertex buffer chunk : ('vert')
// fixed vertex description, vertex count, vertex data
// --------------------
template<typename streamType>
void LoadVertexChunk(Mesh& mesh, streamType& file)
{
  VertexBuffer* vertexBuffer = &mesh.mVertices;

  file.Read(vertexBuffer->mFixedDesc);
  
  uint numVertices;
  file.Read(numVertices);

  uint vertexBufferSize = numVertices * vertexBuffer->mFixedDesc.mVertexSize;
  byte* vertexBufferData = new byte[vertexBufferSize];
  file.ReadArraySize(vertexBufferData, vertexBufferSize);

  delete[] vertexBuffer->mData;
  vertexBuffer->mData = vertexBufferData;
  vertexBuffer->mDataCapacity = vertexBufferSize;
  vertexBuffer->mDataSize = vertexBufferSize;
}

// --------------------
// index buffer chunk : ('indx')
// index type, index count, index data
// --------------------
template<typename streamType>
void LoadIndexChunk(Mesh& mesh, streamType& file)
{
  IndexBuffer* indexBuffer = &mesh.mIndices;
  IndexElementType::Enum indexType;
  uint numIndicies;
  file.Read(indexType);
  file.Read(numIndicies);

  uint indexBufferSize = GetIndexSize(indexType) * numIndicies;
  byte* indexBufferData = new byte[indexBufferSize];
  file.ReadArray(indexBufferData, indexBufferSize);

  indexBuffer->mIndexSize = 4;
  indexBuffer->mGenerated = false;
  
  switch (indexType)
  {
  case Zero::IndexElementType::Byte:   FillIndexBuffer<byte>(indexBuffer, indexBufferData, numIndicies);
    break;
  case Zero::IndexElementType::Ushort: FillIndexBuffer<ushort>(indexBuffer, indexBufferData, numIndicies);
    break;
  case Zero::IndexElementType::Uint:   FillIndexBuffer<uint>(indexBuffer, indexBufferData, numIndicies);
    break;
  }
  delete indexBufferData;
}

template<typename streamType>
void LoadSkeletonChunk(Mesh& mesh, streamType& file)
{
  uint count;
  file.Read(count);

  mesh.mBones.Resize(count);
  forRange (MeshBone& bone, mesh.mBones.All())
  {
    file.ReadString(bone.mName);
    file.Read(bone.mBindTransform);
  }
}

//------------------------------------------------------------------ MeshLoadePattern
// Mesh contents file format, chunks may be present but can be omitted
// --------------------
// mesh header :
// file id('zmsh'), aabb, primitive type
// --------------------
// vertex buffer chunk : ('vert')
// fixed vertex description, vertex count, vertex data
// --------------------
// index buffer chunk : ('indx')
// index type, index count, index data
// --------------------

struct MeshLoadPattern
{
  template<typename readerType>
  static void Load(Mesh* mesh, readerType& reader)
  {
    MeshHeader header;
    reader.Read(header);

    if (header.mFileId != MeshFileId)
      return;

    mesh->mAabb = header.mAabb;
    mesh->mPrimitiveType = header.mPrimitiveType;
    mesh->mBindOffsetInv = header.mBindOffsetInv;

    while (true)
    {
      FileChunk chunk = reader.ReadChunkHeader();
      switch (chunk.Type)
      {
        case 0:
          mesh->BuildTree();
          return;
        case VertexChunk:
          LoadVertexChunk(*mesh, reader);
          break;
        case IndexChunk:
          LoadIndexChunk(*mesh, reader);
          break;
        case SkeletonChunk:
          LoadSkeletonChunk(*mesh, reader);
          break;
        default:
          ErrorIf(true, "Incorrect mesh data format\n");
          break;
      }
    }
  }
};

//----------------------------------------------------------------- Mesh Manager 

ImplementResourceManager(MeshManager, Mesh);

MeshManager::MeshManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("Mesh", new ChunkFileLoader<MeshManager, MeshLoadPattern>());
  mCategory = "Graphics";
  DefaultResourceName = "Cube";
  mCanReload = true;
  mCanAddFile = true;
  AddGeometryFileFilters(this);
}

} // namespace Zero
