// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ArchetypeProcessor
{
public:
  ArchetypeProcessor(GeneratedArchetype* generatedArchetype, HierarchyDataMap& hierarchyData);

  void BuildSceneGraph(String rootNode);
  SceneGraphNode* BuildSceneNodes(HierarchyData nodeData);
  void ExportSceneGraph(String filename, String outputPath);

  SceneGraphSource mSceneSource;
  GeneratedArchetype* mGeneratedArchetype;
  HierarchyDataMap& mHierarchyDataMap;
};

} // namespace Zero
