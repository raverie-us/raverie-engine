// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class MeshProcessor
{
public:
  MeshProcessor(MeshBuilder* meshBuilder, MeshDataMap& meshDataMap);
  ~MeshProcessor();

  void SetupTransformationMatricies();
  void ExtractAndProcessMeshData(const aiScene* scene);
  void ExportMeshData(String outputPath);

  void WriteSingleMeshes(String outputPath);
  void WriteCombinedMesh(String outputPath);

  MeshBuilder* mBuilder;

  MeshDataMap& mMeshDataMap;
};

} // namespace Raverie
