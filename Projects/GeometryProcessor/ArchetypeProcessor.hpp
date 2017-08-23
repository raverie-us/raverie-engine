//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
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

}// namespace Zero
