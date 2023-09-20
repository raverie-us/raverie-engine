// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

const String cCogPathParent("../");

class SkeletonProcessor
{
public:
  SkeletonProcessor(HierarchyDataMap& hierarchyData, MeshDataMap& meshData, String& rootNodeName);

  void ProcessSkeletonHierarchy(const aiScene* scene);
  String FindSkeletonRootFromBone(String boneName);
  String CreateCogPathToSkeletonRoot(String meshNode, String skeletonRootNode);
  // When our scene root is the skeleton root we need to generate
  // a dumby root node to prevent double animation translation
  void GenerateObjectRoot();
  void UpdateCogPaths(String nodeName);

  HierarchyDataMap& mHierarchyDataMap;
  MeshDataMap& mMeshDataMap;
  String& mRootNodeName;
};

} // namespace Raverie
