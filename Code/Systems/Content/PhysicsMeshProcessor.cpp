// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

PhysicsMeshProcessor::PhysicsMeshProcessor(PhysicsMeshBuilder* physicsMeshBuilder, MeshDataMap& meshDataMap) :
    mBuilder(physicsMeshBuilder),
    mMeshDataMap(meshDataMap)
{
}

PhysicsMeshProcessor::~PhysicsMeshProcessor()
{
}

void PhysicsMeshProcessor::BuildPhysicsMesh(String outputPath)
{
  size_t numMeshes = mMeshDataMap.Size();

  String extension = ".physmesh";
  if (mBuilder->MeshBuilt == PhysicsMeshType::ConvexMesh)
    extension = ".convexmesh";

  for (size_t i = 0; i < numMeshes; ++i)
  {
    GeometryResourceEntry& entry = mBuilder->Meshes[i];
    MeshData& meshData = mMeshDataMap[i];
    String physicsMeshFile = FilePath::CombineWithExtension(outputPath, entry.mName, extension);
    meshData.mPhysicsMeshName = physicsMeshFile;

    BinaryFileSaver saver;
    Status status;
    saver.Open(status, physicsMeshFile.c_str());

    // Copy the vertices
    VertexPositionArray vertices;
    size_t numVertices = meshData.mVertexBuffer.Size();
    vertices.Resize(numVertices);
    for (size_t j = 0; j < numVertices; ++j)
      vertices[j] = meshData.mVertexBuffer[j].mPosition;

    // Copy the indices
    IndexArray indices = meshData.mIndexBuffer;
    // Remove degenerate triangles
    RemoveDegenerateTriangles(vertices, indices);

    if (mBuilder->MeshBuilt == PhysicsMeshType::PhysicsMesh)
      WriteStaticMesh(vertices, indices, saver);
    else
      WriteConvexMesh(vertices, indices, saver);

    saver.Close();
  }
}

void PhysicsMeshProcessor::WriteStaticMesh(VertexPositionArray& vertices, IndexArray& indices, Serializer& saver)
{
  // Start the PhysicsMesh node
  saver.StartPolymorphic("PhysicsMesh");
  // Save the vertices
  saver.SerializeField("Vertices", vertices);
  // Save the indices
  saver.SerializeField("Indices", indices);
  // Write the Aabb Tree
  WriteAabbTree(vertices, indices, saver);
  saver.EndPolymorphic();
}

void PhysicsMeshProcessor::WriteConvexMesh(VertexPositionArray& vertices, IndexArray& indices, Serializer& saver)
{
  // Start the ConvexMesh node
  saver.StartPolymorphic("ConvexMesh");

  // Save the vertices
  saver.SerializeField("Vertices", vertices);
  // Save the indices
  saver.SerializeField("Indices", indices);

  // Get the triangle count
  uint triCount = indices.Size() / 3;

  // Calculate the volume of the mesh
  float volume = Geometry::CalculateTriMeshVolume(&vertices.Front(), (uint*)(&indices.Front()), triCount);

  // Calculate the center of mass of the mesh
  Vec3 centerOfMass = Geometry::CalculateTriMeshCenterOfMass(&vertices.Front(), (uint*)(&indices.Front()), triCount);

  // Save the volume
  saver.SerializeField("Volume", volume);
  // Save the center of mass
  saver.SerializeField("CenterOfMass", centerOfMass);

  // Save the Bsp-Tree
  // WriteBspTree(saver, vertices, indices);

  saver.EndPolymorphic();
}

void PhysicsMeshProcessor::WriteAabbTree(VertexPositionArray& vertices, IndexArray& indices, Serializer& saver)
{
  // Build the Aabb-Tree
  StaticAabbTree<uint> aabbTree;
  aabbTree.SetPartitionMethod(PartitionMethods::MinimizeVolumeSum);

  // Dummy proxy. They will not be needed.
  BroadPhaseProxy proxy;

  // Populate the Aabb-Tree
  for (uint i = 0; i < indices.Size(); i += 3)
  {
    // Grab the vertices of the triangle
    Vec3 p0, p1, p2;
    p0 = vertices[indices[i]];
    p1 = vertices[indices[i + 1]];
    p2 = vertices[indices[i + 2]];

    // Build the Aabb of the triangle
    Aabb aabb;
    aabb.Compute(p0);
    aabb.Expand(p1);
    aabb.Expand(p2);

    // Create the broad phase data.
    BaseBroadPhaseData<uint> data;
    data.mClientData = i;
    data.mAabb = aabb;

    // Insert it into the tree
    aabbTree.CreateProxy(proxy, data);
  }

  // Construct the tree
  aabbTree.Construct();

  // Save the tree
  SerializeAabbTree(saver, aabbTree);
}

uint PhysicsMeshProcessor::RemoveDegenerateTriangles(VertexPositionArray& vertices, IndexArray& indicies)
{
  Array<IndexType> filteredIndices;
  filteredIndices.Reserve(indicies.Size());

  uint amountRemoved = 0;

  for (uint i = 0; i < indicies.Size(); i += 3)
  {
    uint i0 = indicies[i];
    uint i1 = indicies[i + 1];
    uint i2 = indicies[i + 2];

    Vec3 p0 = vertices[i0];
    Vec3 p1 = vertices[i1];
    Vec3 p2 = vertices[i2];

    if (Geometry::IsDegenerate(p0, p1, p2))
    {
      ++amountRemoved;
      continue;
    }

    filteredIndices.PushBack(i0);
    filteredIndices.PushBack(i1);
    filteredIndices.PushBack(i2);
  }

  indicies.Swap(filteredIndices);
  return amountRemoved;
}

} // namespace Raverie
