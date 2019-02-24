// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ArchetypeProcessor::ArchetypeProcessor(GeneratedArchetype* generatedArchetype, HierarchyDataMap& hierarchyData) :
    mGeneratedArchetype(generatedArchetype),
    mHierarchyDataMap(hierarchyData)
{
}

void ArchetypeProcessor::BuildSceneGraph(String rootNode)
{
  // We will go through the hierarchy building our scene graph starting with the
  // root All assimp scenes have their root named RootNode
  HierarchyData nodeData = mHierarchyDataMap[rootNode];

  // these nodes represent the hierarchy of the cogs that will construct an
  // archetype on import
  mSceneSource.Name = rootNode;
  mSceneSource.Root = BuildSceneNodes(nodeData);
}

SceneGraphNode* ArchetypeProcessor::BuildSceneNodes(HierarchyData nodeData)
{
  GeometryImport* geoImport = mGeneratedArchetype->mOwner->has(GeometryImport);
  Mat4 transform = geoImport->mTransform;
  Quat changeOfBasis = Math::ToQuaternion(geoImport->mChangeOfBasis);

  size_t numChildren = nodeData.mChildren.Size();

  SceneGraphNode* graphNode = new SceneGraphNode();
  graphNode->NodeName = nodeData.mNodeName;

  Mat3 rotation;
  nodeData.mLocalTransform.Decompose(&(graphNode->Scale), &rotation, &(graphNode->Translation));
  graphNode->Rotation = Math::ToQuaternion(rotation);

  graphNode->Translation = Math::TransformPoint(transform, graphNode->Translation);
  graphNode->Rotation = changeOfBasis * graphNode->Rotation * changeOfBasis.Inverted();

  graphNode->IsSkeletonRoot = nodeData.mIsSkeletonRoot;

  if (nodeData.mHasMesh)
  {
    graphNode->MeshName = nodeData.mMeshName;
    graphNode->PhysicsMeshName = nodeData.mPhysicsMeshName;
    graphNode->SkeletonRootNodePath = nodeData.mSkeletonRootNodePath;
  }

  for (size_t i = 0; i < numChildren; ++i)
  {
    String childNodeName = nodeData.mChildren[i];
    HierarchyData childNode = mHierarchyDataMap[childNodeName];

    SceneGraphNode* childGraphNode = BuildSceneNodes(childNode);
    childGraphNode->NodeName = childNodeName;

    graphNode->Children.PushBack(childGraphNode);
  }

  return graphNode;
}

void ArchetypeProcessor::ExportSceneGraph(String filename, String outputPath)
{
  String graphFile = FilePath::CombineWithExtension(outputPath, filename, ".graph.data");
  SaveToDataFile(mSceneSource, graphFile);
}

} // namespace Zero
