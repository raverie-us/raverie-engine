// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
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

} // namespace Raverie
