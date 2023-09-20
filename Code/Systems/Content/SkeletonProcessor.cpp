// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

SkeletonProcessor::SkeletonProcessor(HierarchyDataMap& hierarchyData, MeshDataMap& meshData, String& rootNodeName) :
    mHierarchyDataMap(hierarchyData),
    mMeshDataMap(meshData),
    mRootNodeName(rootNodeName)
{
}

void SkeletonProcessor::ProcessSkeletonHierarchy(const aiScene* scene)
{
  aiMesh** meshes = scene->mMeshes;
  size_t numMeshes = scene->mNumMeshes;

  // using a bone we will find the skeleton root node
  for (size_t meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
  {
    aiMesh* mesh = meshes[meshIndex];
    MeshData& meshData = mMeshDataMap[meshIndex];

    // no bones then proceed to the next mesh
    if (!mesh->HasBones())
      continue;

    aiBone** bones = mesh->mBones;
    size_t numBones = mesh->mNumBones;

    // this is probably unnecessary, but just in case
    if (numBones > 0)
    {
      aiBone* bone = bones[0];

      // using the first bone, find the hierarchy node and proceed up
      // until we find what we define as the skeleton root node
      String boneName = CleanAssetName(bone->mName.C_Str());
      // on the mesh node set the path to the skeleton root node we just found
      String skeletonRootNode = FindSkeletonRootFromBone(boneName);
      HierarchyData& hierarchyData = mHierarchyDataMap[meshData.mMeshName];
      hierarchyData.mSkeletonRootNodePath = CreateCogPathToSkeletonRoot(meshData.mMeshName, skeletonRootNode);
    }
  }
}

String SkeletonProcessor::FindSkeletonRootFromBone(String boneName)
{
  // get the node we are currently checking
  HierarchyData& data = *mHierarchyDataMap.FindPointer(boneName);

  // first check if we are the root node
  if (data.mParentNodeName != mRootNodeName && boneName != mRootNodeName)
    return FindSkeletonRootFromBone(data.mParentNodeName);

  // the parent is the root node or we are the root node, so mark our current
  // node as the skeletons root
  data.mIsSkeletonRoot = true;
  return data.mNodeName;
}

String SkeletonProcessor::CreateCogPathToSkeletonRoot(String meshNode, String skeletonRootNode)
{
  // the skeleton will always be one level down from the root node
  // so we just need to count up the number of levels to the root that the mesh
  // is from
  HierarchyData meshNodeData = *mHierarchyDataMap.FindPointer(meshNode);

  // if the root is the skeleton set the proper cog path for self
  if (meshNodeData.mNodeName == mRootNodeName)
  {
    GenerateObjectRoot();
    meshNodeData = *mHierarchyDataMap.FindPointer(meshNode);
  }

  StringBuilder skeletonRootCogPath;
  while (!meshNodeData.mParentNodeName.Empty())
  {
    meshNodeData = mHierarchyDataMap[meshNodeData.mParentNodeName];
    skeletonRootCogPath.Append(cCogPathParent);
  }

  skeletonRootCogPath.Append(skeletonRootNode);
  return skeletonRootCogPath.ToString();
}

void SkeletonProcessor::GenerateObjectRoot()
{
  // Create our new object root that will be empty and contains the original
  // root only
  HierarchyData newRoot;
  newRoot.mNodeName = String("ObjectRoot");
  newRoot.mLocalTransform = Mat4::cIdentity;
  newRoot.mChildren.PushBack(mRootNodeName);
  mHierarchyDataMap.Insert(newRoot.mNodeName, newRoot);

  // set the new root as the parent of the original root
  mHierarchyDataMap[mRootNodeName].mParentNodeName = newRoot.mNodeName;

  // update the root node name to our newest root
  mRootNodeName = newRoot.mNodeName;

  // update cogpaths for everything
  UpdateCogPaths(mRootNodeName);
}

void SkeletonProcessor::UpdateCogPaths(String nodeName)
{
  HierarchyData node = mHierarchyDataMap[nodeName];
  for (uint i = 0; i < node.mChildren.Size(); ++i)
  {
    UpdateCogPaths(node.mChildren[i]);
  }

  if (nodeName == mRootNodeName)
    mHierarchyDataMap[nodeName].mNodePath = nodeName;
  else
    mHierarchyDataMap[nodeName].mNodePath =
        BuildString(mRootNodeName, cAnimationPathDelimiterStr, mHierarchyDataMap[nodeName].mNodePath);
}

} // namespace Raverie
