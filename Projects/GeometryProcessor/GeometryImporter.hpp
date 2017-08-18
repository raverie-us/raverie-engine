//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class GeometryImporter
{
public:
  GeometryImporter(String inputFile, String outputPath, String metaFile);
  ~GeometryImporter();

  uint SetupAssimpPostProcess();
  int ProcessModelFiles();

  bool SceneEmpty();
  void CollectNodeData();
  // returns the unique name of the node, takes the parent nodes name that has potentially been
  // generated/made unique as to properly reference it among nodes with the same name.
  String ExtractDataFromNodesRescursive(aiNode* node, String parentName);
  void SingleMeshHierarchyEntry(HierarchyData& hierarchyData, uint meshIndex);
  void MultipleMeshsHierarchicalEntry(HierarchyData& hierarchyData, aiNode* node);

  void ComputeMeshTransforms();
  bool UpdateBuilderMetaData();

  // if the node name contains the fbx impot identifier remove it,
  // just having _descriptionOfTransform is enough

//   void GenerateNormals(MeshData& meshData);
//   void SmoothNormals(MeshData& meshData);

  // zero meta data
  GeometryContent* mGeometryContent;

  // assimp data
  Assimp::Importer mAssetImporter;
  const aiScene* mScene;
  String mRootNodeName;

  // provided data
  String mInputFile;
  String mOutputPath;
  String mMetaFile;
  
  // processed and generated data
  String mBaseMeshName;
  MeshDataMap mMeshDataMap;
  HierarchyDataMap mHierarchyDataMap;

  //
  uint mUniquifyingIndex;
};

}// namespace Zero
