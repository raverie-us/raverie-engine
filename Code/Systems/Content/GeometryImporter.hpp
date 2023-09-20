// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class GeometryImporter
{
public:
  GeometryImporter(StringParam inputFile, StringParam outputPath, StringParam metaFile);
  ~GeometryImporter();

  uint SetupAssimpPostProcess();
  GeometryProcessorCodes::Enum ProcessModelFiles();

  bool SceneEmpty();
  void CollectNodeData();
  // Returns the unique name of the node, takes the parent nodes name that has
  // potentially been generated/made unique as to properly reference it among
  // nodes with the same name.
  String ExtractDataFromNodesRescursive(aiNode* node, String parentName);
  void SingleMeshHierarchyEntry(HierarchyData& hierarchyData, uint meshIndex);
  void MultipleMeshsHierarchicalEntry(HierarchyData& hierarchyData, aiNode* node);
  void FindAnimationNodes();

  void ComputeMeshTransforms();
  bool UpdateBuilderMetaData();

  // If Assimp fails and provides an error message that is not descriptive
  // enough return a new error message that better informs the user of what went
  // wrong if a better message has not been specified return the original
  // message
  String ProcessAssimpErrorMessage(StringParam errorMessage);

  // Raverie meta data
  GeometryContent* mGeometryContent;

  // Assimp data
  Assimp::Importer mAssetImporter;
  const aiScene* mScene;
  String mRootNodeName;

  // Provided data
  String mInputFile;
  String mOutputPath;
  String mMetaFile;

  // Processed and generated data
  String mBaseMeshName;
  MeshDataMap mMeshDataMap;
  HierarchyDataMap mHierarchyDataMap;
  AnimationNodeRedirectMap mAnimationRedirectMap;
  // An every increasing value appended to node names to result in unique names
  // in the hierarchy
  uint mUniquifyingIndex;
};

} // namespace Raverie
