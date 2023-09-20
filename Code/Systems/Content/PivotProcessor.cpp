// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

PivotProcessor::PivotProcessor(HierarchyDataMap& hierarchyData, String& rootNodeName, AnimationNodeRedirectMap& animationRedirectMap) :
    mHierarchyDataMap(hierarchyData), mRootNodeName(rootNodeName), mAnimationRedirectMap(animationRedirectMap)
{
}

void PivotProcessor::ProccessAndCollapsePivots()
{
  ProcessPivots();
  CollapsePivots();
}

void PivotProcessor::ProcessPivots()
{
  ProcessPivotsRecursive(mRootNodeName, false);
}

void PivotProcessor::ProcessPivotsRecursive(StringParam nodeName, bool parentIsPivot)
{
  HierarchyData& node = mHierarchyDataMap[nodeName];

  // Check the node to see if it is the start of a pivot hierarchy
  if (node.mIsPivot)
  {
    // If this node is a skeleton bone remove it from being a pivot to collapse
    if (!node.mSkeletonRootNodePath.Empty())
      node.mIsPivot = false;
    // If its parent is not a pivot set it as the start of a pivot hierarchy
    else if (!parentIsPivot)
      mPivotStarts.PushBack(nodeName);
  }

  forRange (String childName, node.mChildren)
    ProcessPivotsRecursive(childName, node.mIsPivot);
}

void PivotProcessor::CollapsePivots()
{
  // Go to each pivot that starts a hierarchy and collapse the child pivots
  forRange (String startNodeName, mPivotStarts.All())
  {
    if (mHierarchyDataMap.ContainsKey(startNodeName))
    {
      HierarchyData& pivotStartNode = mHierarchyDataMap[startNodeName];
      CollapsePivotHierarchy(pivotStartNode, true);
    }
  }
}

String PivotProcessor::CollapsePivotHierarchy(HierarchyData& startNode, bool preAnimationCorrection)
{
  // Compute the collapsed pivots transform
  ErrorIf(startNode.mChildren.Size() > 1 || startNode.mChildren.Empty(),
          "Pivot Hierarchies should not have a pivot node with more than 1 "
          "child or be empty");

  Array<String> nodesToRemove;
  nodesToRemove.PushBack(startNode.mNodeName);
  Mat4 animationCorrection = Mat4::cIdentity;

  // Get the start nodes first child and set the transform data
  String childName = startNode.mChildren[0];
  bool isPivot = true;
  Mat4 transformData = startNode.mLocalTransform;

  // If there is an animation node set then this nodes local transform for
  // animation correction is the transform from each single track value
  if (startNode.mAnimationNode)
    animationCorrection = animationCorrection * GetLocalTransfromFromAnimation(startNode.mAnimationNode);
  else if (preAnimationCorrection)
    animationCorrection = animationCorrection * transformData;
  else
    animationCorrection = Mat4::cIdentity;

  // Go down the pivot hierarchy updating the local transform all the way down
  // to our base node
  while (isPivot)
  {
    HierarchyData& childData = mHierarchyDataMap[childName];
    transformData = transformData * childData.mLocalTransform;
    isPivot = childData.mIsPivot;
    if (isPivot)
    {
      nodesToRemove.PushBack(childName);
      childName = childData.mChildren[0];
      if (childData.mAnimationNode)
        animationCorrection = animationCorrection * GetLocalTransfromFromAnimation(childData.mAnimationNode);
      else
        animationCorrection = animationCorrection * childData.mLocalTransform;
    }
  }

  // The nodeData is now the base node that the pivot data lead to
  String baseNodeName = childName;
  HierarchyData& baseNode = mHierarchyDataMap[baseNodeName];
  // Update its transform
  if (preAnimationCorrection)
  {
    baseNode.mPreAnimationCorrection = animationCorrection;
  }
  else
  {
    baseNode.mPreAnimationCorrection = startNode.mPreAnimationCorrection;
    baseNode.mPostAnimationCorrection = animationCorrection * baseNode.mLocalTransform;
  }

  // The animation correction needs the base nodes original local transform so
  // set this data after storing the animation correction data
  baseNode.mLocalTransform = transformData;

  // Update the base nodes data to reflect the collapsed hierarchy
  HierarchyData& parentdata = mHierarchyDataMap[startNode.mParentNodeName];
  baseNode.mParentNodeName = parentdata.mNodeName;
  baseNode.mNodePath = BuildString(parentdata.mNodePath, cAnimationPathDelimiterStr, baseNode.mNodeName);

  // Remove the original child node from the pivots parent node and set it to
  // our base node
  parentdata.mChildren.EraseValue(startNode.mNodeName);
  parentdata.mChildren.PushBack(childName);

  // If a base node is a skeleton node we need to correct the bone cog path
  if (!baseNode.mSkeletonRootNodePath.Empty())
    CorrectSkeletonPath(baseNode);

  // Remove all the node data of the collapsed nodes from our hierarchy map
  for (size_t i = 0; i < nodesToRemove.Size(); ++i)
    mHierarchyDataMap.Erase(nodesToRemove[i]);

  // NOTE: Collapsing animated pivots down through a hierarchy can result in the
  // need to add new animation frames and will be discussed and revisited later.

  // Check the children nodes below our base node and collapse the animations
  // down through any pivots that are not animated themselves, only works if
  // there is one child
  // if (baseNode.mChildren.Size() == 1 && preAnimationCorrection)
  //{
  //  HierarchyData& childNode = mHierarchyDataMap[baseNode.mChildren[0]];
  //
  //  // If our new base node is not an animated pivot or if our child is also
  //  an animated pivot
  //  // do not attempt to collapse the hierarchy further
  //  if (!baseNode.mIsAnimatedPivot || childNode.mIsAnimatedPivot)
  //    return baseNode.mNodeName;
  //
  //  String nodeCollapsedToName = CollapsePivotHierarchy(baseNode, false);
  //  HierarchyData& nodeCollapsedTo = mHierarchyDataMap[nodeCollapsedToName];
  //  nodeCollapsedTo.mIsAnimatedPivot = true;
  //  // The animated node was collapsed as far down as possible
  //  // Key the old nodes animations to the node it was collapsed into for the
  //  animation processor mAnimationRedirectMap.Insert(baseNodeName,
  //  nodeCollapsedToName); return nodeCollapsedToName;
  //}

  return baseNodeName;
}

void PivotProcessor::CorrectSkeletonPath(HierarchyData& node)
{
  // Get the skeleton root nodes name
  StringRange skeletonPath = node.mSkeletonRootNodePath;
  StringRange skeletonRootStart = skeletonPath.FindLastOf(cCogPathParent);
  String skeletonRootName = skeletonPath.SubString(skeletonRootStart.End(), skeletonPath.End());

  // Walk up to the root node to build the path to the new skeleton root for the
  // collapsed hierarchy, the skeleton root will always be one level below the
  // root node
  StringBuilder skeletonRootCogPath;
  String nodeName = node.mNodeName;
  while (nodeName != "RootNode")
  {
    HierarchyData& nodeData = mHierarchyDataMap[nodeName];
    nodeName = nodeData.mParentNodeName;
    skeletonRootCogPath.Append(cCogPathParent);
  }

  skeletonRootCogPath.Append(skeletonRootName);
  node.mSkeletonRootNodePath = skeletonRootCogPath.ToString();
}

Mat4 PivotProcessor::GetLocalTransfromFromAnimation(aiNodeAnim* animationNode)
{
  Mat4 animationCorrection = Mat4::cIdentity;

  Vec3 translation = Vec3::cZero;
  Quat rotation = Quat::cIdentity;
  Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);

  if (animationNode->mNumPositionKeys)
    translation = AssimpToZeroPositionKey(animationNode->mPositionKeys[0]).Position;

  if (animationNode->mNumRotationKeys)
    rotation = AssimpToZeroRotationKey(animationNode->mRotationKeys[0]).Rotation;

  if (animationNode->mNumScalingKeys)
    scale = AssimpToZeroScalingKey(animationNode->mScalingKeys[0]).Scale;

  return BuildTransform(translation, rotation, scale);
}

} // namespace Raverie
