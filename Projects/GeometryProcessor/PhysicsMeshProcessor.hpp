//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef uint IndexType;

class PhysicsMeshProcessor
{
public:
  PhysicsMeshProcessor(PhysicsMeshBuilder* physicsMeshBuilder, MeshDataMap& meshDataMap);
  ~PhysicsMeshProcessor();

  void BuildPhysicsMesh(String outputPath);
  void WriteStaticMesh(VertexPositionArray& vertices, IndexArray& indices, Serializer& saver);
  void WriteConvexMesh(VertexPositionArray& vertices, IndexArray& indices, Serializer& saver);
  void WriteAabbTree(VertexPositionArray& vertices, IndexArray& indices, Serializer& saver);
  uint RemoveDegenerateTriangles(VertexPositionArray& vertices, IndexArray& indicies);
  
  PhysicsMeshBuilder* mBuilder;
  MeshDataMap& mMeshDataMap;
};

}// namespace Zero