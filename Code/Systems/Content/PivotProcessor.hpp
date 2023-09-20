// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class PivotProcessor
{
public:
  PivotProcessor(HierarchyDataMap& hierarchyData, String& rootNodeName, AnimationNodeRedirectMap& animationRedirectMap);

  void ProccessAndCollapsePivots();
  // Checks pivots for bone data to later handle animation correction
  void ProcessPivots();
  void ProcessPivotsRecursive(StringParam nodeName, bool parentIsPivot);
  // Collapse all pivots that are not animated
  void CollapsePivots();
  // Take a node and collapse all pivots between the first pivot and the
  // non-pivot node or animated pivot
  String CollapsePivotHierarchy(HierarchyData& startNode, bool preAnimationCorrection);
  // Helper function that corrects skeleton paths for after collapsing a
  // hierarchy of pivots
  void CorrectSkeletonPath(HierarchyData& node);
  // Takes an animation with single track values and builds a local transform to
  // correct a later nodes animations
  Mat4 GetLocalTransfromFromAnimation(aiNodeAnim* animationNode);

  HierarchyDataMap& mHierarchyDataMap;
  String& mRootNodeName;
  AnimationNodeRedirectMap& mAnimationRedirectMap;
  // Array that contains the name of each top level pivot in an objects
  // hierarchy
  StringArray mPivotStarts;
};

} // namespace Raverie
