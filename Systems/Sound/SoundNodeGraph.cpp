///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//---------------------------------------------------------------------------------- Node Print Info

//**************************************************************************************************
ZilchDefineType(NodePrintInfo, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindGetter(Connections);
  ZilchBindField(mPosition);
  ZilchBindField(mName);
  ZilchBindField(mHasOutput);
  ZilchBindField(mID);
}

//--------------------------------------------------------------------------------- Sound Node Graph

//**************************************************************************************************
SoundNodeGraph::~SoundNodeGraph()
{
  // If there are node objects in the list, delete them
  for (unsigned i = 0; i < mNodeInfoList.Size(); ++i)
    delete mNodeInfoList[i];
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Adds new NodePrintInfo to the map
void SoundNodeGraph::CreateInfo(HandleOf<SoundNode> node, HandleOf<SoundNode> outputNode, int level)
{
  // If this is an editor space, skip it and its parents
  if (node->cName == "EditorSpace")
    return;

  // If this is a space with no inputs, skip it
  if (node->cName == "Space" && !node->GetHasInputs())
  {
    // Check if the output node is also for the space
    if (outputNode->cName == "Space")
    {
      // If the child exists in the map and has no other parents, erase it
      NodePrintInfo* child = mNodeMap.FindValue(outputNode->cNodeID, nullptr);
      if (child && child->mParents.Empty())
        mNodeMap.Erase(outputNode->cNodeID);
    }

    return;
  }

  NodePrintInfo* info = mNodeMap.FindValue(node->cNodeID, nullptr);

  // Node wasn't in the map, so we need to create it
  if (!info)
  {
    info = new NodePrintInfo(level, node->cName, node->cNodeID, node->HasAudibleOutput(), node);
    mNodeMap[node->cNodeID] = info;

    // If there is an output connection, add it to the list
    if (outputNode)
    {
      NodePrintInfo* child = mNodeMap.FindValue(outputNode->cNodeID, nullptr);
      if (child)
      {
        info->mChildren.PushBack(child);
        child->mParents.PushBack(info);
      }
    }
  }
  // A node with this ID is already in the map, but it's not the same node
  // (i.e. Emitter and Volume nodes that should be displayed as one node)
  else if (info->mNode != node)
  {
    // Don't count this as a level
    --level;

    // If the output node isn't the same ID, add it as a connection
    if (outputNode && outputNode->cNodeID != node->cNodeID)
    {
      NodePrintInfo* child = mNodeMap.FindValue(outputNode->cNodeID, nullptr);
      if (child)
      {
        info->mChildren.PushBack(child);
        child->mParents.PushBack(info);
      }
    }
  }
  // This particular node is already in the map (it has multiple output connections)
  else
  {
    // If there is an output connection, add it to the list
    if (outputNode)
    {
      NodePrintInfo* child = mNodeMap.FindValue(outputNode->cNodeID, nullptr);
      if (child)
      {
        info->mChildren.PushBack(child);
        child->mParents.PushBack(info);
      }
    }

    // Don't check inputs, already handled
    return;
  }

  // Keep track of the highest level
  if (level > mMaxLevel)
    mMaxLevel = level;

  // Call this function on all inputs of this node
  const Array<HandleOf<SoundNode>>* inputs = node->GetInputs(AudioThreads::MainThread);
  for (unsigned i = 0; i < inputs->Size(); ++i)
  {
    CreateInfo((*inputs)[i], node, level + 1);
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Moves overlapping nodes away from each other
void SoundNodeGraph::CheckForCollision(NodeInfoListType& list, bool checkOrphanNodes)
{
  float minDistance(mNodeWidth * 0.95f);

  bool foundSomething(true);
  unsigned count(0);

  // We need to avoid possible infinite loops if there is no good position for the nodes,
  // and this value seems to work. Could be adjusted if needed.
  unsigned maxCount = 20;

  // Keep looking as long as something was moved last loop and we haven't reached max loops
  while (foundSomething && count < maxCount)
  {
    ++count;
    foundSomething = false;

    // Check all nodes against all other nodes
    for (unsigned i = 0; i < list.Size(); ++i)
    {
      for (unsigned j = 0; j < list.Size(); ++j)
      {
        if (i == j)
          continue;

        NodePrintInfo* info1 = list[i];
        NodePrintInfo* info2 = list[j];

        // If we're not supposed to check orphan nodes and one of the nodes has no parents, skip
        if (!checkOrphanNodes && (info1->mParents.Empty() || info2->mParents.Empty()))
          continue;

        // Check if the distance between the two nodes is too close
        float distance = Math::Abs(info1->mPosition.x - info2->mPosition.x);
        if (distance < minDistance)
        {
          // Move the nodes to be the minimum distance apart
          float moveAmount = (minDistance - distance) * 0.5f;

          if (info1->mPosition.x <= info2->mPosition.x)
          {
            info1->mPosition.x -= moveAmount;
            info2->mPosition.x += moveAmount;
          }
          else
          {
            info1->mPosition.x += moveAmount;
            info2->mPosition.x -= moveAmount;
          }

          foundSomething = true;
        }
      }
    }
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Moves spaces apart - walks up from base node altering x position
void SoundNodeGraph::AddSpacePadding(NodePrintInfo* node, float addToXPos)
{
  if (!node->mMoved)
  {
    node->mPosition.x += addToXPos;
    node->mMoved = true;
  }

  for (unsigned i = 0; i < node->mParents.Size(); ++i)
    AddSpacePadding(node->mParents[i], addToXPos);
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Does the first pass of positioning nodes up to and including the largest level
void SoundNodeGraph::FirstPassPositioning(Array<NodeInfoListType>& infoByLevel, int largestLevel)
{
  // First level node should always be audio output
  infoByLevel[0][0]->mPosition = Vec2(0.0f, 0.0f);
  ErrorIf(infoByLevel[0].Size() != 1, "Too many nodes on first row for sound node graph");

  // If there's only one level, we're done
  if (infoByLevel.Size() == 1)
    return;
  
  float halfWidth = (float)infoByLevel[largestLevel].Size() * mNodeWidth * 0.5f;

  // Go up to the largest level, starting at the bottom
  for (int level = 0; level < largestLevel; ++level)
  {
    NodeInfoListType tempList;

    // Step through each node on this level
    forRange(NodePrintInfo* child, infoByLevel[level].All())
    {
      // Step through the parents of the node
      forRange(NodePrintInfo* parent, child->mParents.All())
      {
        // If the parent is only one level above, add it to the temporary list
        if (parent->mLevel == level + 1)
        {
          tempList.PushBack(parent);
          parent->mPositionSet = true;
        }
      }
    }
    // Step through each node in the level above this one
    forRange(NodePrintInfo* parent, infoByLevel[level + 1].All())
    {
      // If it wasn't added to the temporary list, add it now
      if (!parent->mPositionSet)
      {
        tempList.PushBack(parent);
        parent->mPositionSet = true;
      }
    }

    // Make sure the sizes of the two lists are the same
    ErrorIf(tempList.Size() != infoByLevel[level + 1].Size(), "Sizes don't match");

    // Step through each node in the temporary list and set its position depending on 
    // its position in the list and the height of the level
    float yPos = (float)(level + 1) * mNodeHeight;
    for (unsigned i = 0; i < tempList.Size(); ++i)
      tempList[i]->mPosition = Vec2((i * (float)mNodeWidth) - halfWidth, yPos);
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Does the final positioning on all levels except the largest
void SoundNodeGraph::SecondPassPositioning(Array<NodeInfoListType> &infoByLevel, int largestLevel)
{
  // For levels lower than largest, set their positions to the average of their parents, then 
  // space out with collision checking (don't need to do bottom row)
  for (int level = largestLevel - 1; level > 0; --level)
  {
    // Go through each node in the level
    for (unsigned i = 0; i < infoByLevel[level].Size(); ++i)
    {
      NodePrintInfo* info = infoByLevel[level][i];

      // If it has parents, get their average position
      if (!info->mParents.Empty())
      {
        float parentAverage(0);
        if (info->mParents.Size() == 1)
          parentAverage = info->mParents[0]->mPosition.x;
        else
        {
          int count(0);
          for (unsigned j = 0; j < info->mParents.Size(); ++j)
          {
            if (info->mParents[j]->mPositionSet)
            {
              parentAverage += info->mParents[j]->mPosition.x;
              ++count;
            }
          }
          parentAverage /= count;
        }

        // Set the node's position to the parent average
        info->mPosition.x = parentAverage;
      }
    }

    // Check for overlapping and resolve
    CheckForCollision(infoByLevel[level], false);
  }

  // For levels above largest, set their positions to the average of their children, then space 
  // out with collision checking (these nodes haven't had their y position set yet)
  for (unsigned level = largestLevel + 1; level < infoByLevel.Size(); ++level)
  {
    float yPos = (float)level * (float)mNodeHeight;

    // Go through all nodes in this level
    for (unsigned i = 0; i < infoByLevel[level].Size(); ++i)
    {
      NodePrintInfo* info = infoByLevel[level][i];

      // Find the average position of all children
      float childAverage(0);
      if (info->mChildren.Size() == 1)
        childAverage = info->mChildren[0]->mPosition.x;
      else
      {
        for (unsigned j = 0; j < info->mChildren.Size(); ++j)
          childAverage += info->mChildren[j]->mPosition.x;
        childAverage /= info->mChildren.Size();
      }

      // Set the node's position to the average of the children
      info->mPosition = Vec2(childAverage, yPos);

      info->mPositionSet = true;
    }

    // Check for overlapping and resolve
    CheckForCollision(infoByLevel[level], false);
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Cleans up node positions for certain scenarios
void SoundNodeGraph::PositionCleanUp(Array<NodeInfoListType> &infoByLevel, int largestLevel)
{
  // Go through all levels 
  for (unsigned level = 0; level < infoByLevel.Size(); ++level)
  {
    SortedArray<float> positions;
    Array<NodePrintInfo*> orphans;

    // Go through all nodes in this level
    for (unsigned i = 0; i < infoByLevel[level].Size(); ++i)
    {
      NodePrintInfo* node = infoByLevel[level][i];

      // Does this node have one parent, the parent has other children,
      // the node has one child, and the child has other parents?
      if (node->mParents.Size() == 1 && node->mParents[0]->mChildren.Size() > 1
        && node->mChildren.Size() == 1 && node->mChildren[0]->mParents.Size() > 1)
      {
        NodePrintInfo* child = node->mChildren[0];
        NodePrintInfo* parent = node->mParents[0];

        // Find which of the child's parents are closest to the node's parent
        float distance = FLT_MAX;
        int index(0);
        for (unsigned k = 0; k < child->mParents.Size(); ++k)
        {
          float thisDistance = Math::Abs(child->mParents[k]->mPosition.x - parent->mPosition.x);
          if (thisDistance < distance)
          {
            distance = thisDistance;
            index = k;
          }
        }

        // If the closest one is not this node, switch positions
        if (child->mParents[index] != node)
        {
          float temp = node->mPosition.x;
          node->mPosition.x = child->mParents[index]->mPosition.x;
          child->mParents[index]->mPosition.x = temp;

          // Reset position of parent now that one of its children has changed
          float childAverage(0);
          for (unsigned j = 0; j < parent->mChildren.Size(); ++j)
            childAverage += parent->mChildren[j]->mPosition.x;
          parent->mPosition.x = childAverage / parent->mChildren.Size();
        }
      }

      // Save parent nodes without parents and positions of parent nodes with parents
      forRange(NodePrintInfo* parent, node->mParents.All())
      {
        if (parent->mLevel == level + 1)
        {
          if (parent->mParents.Empty() && node->mParents.Size() > 1)
            orphans.PushBack(parent);
          else 
            positions.Insert(parent->mPosition.x);
        }
      }
    }

    // Don't do this check for nodes on the largest level
    // Is there at least one orphan and at least two with parents?
    if (level + 1 != largestLevel && orphans.Size() >= 1 && positions.Size() >= 2)
    {
      // Step through all orphans
      for (unsigned index = 0; index < orphans.Size(); ++index)
      {
        NodePrintInfo* orphan = orphans[index];

        // Find closest gap to the orphan's child
        float childPosition = orphan->mChildren[0]->mPosition.x;
        float gap(0.0f);
        int gapIndex(-1);
        float positionDifference = FLT_MAX;
        for (unsigned i = 1; i < positions.Size(); ++i)
        {
          float thisGap = Math::Abs(positions[i] - positions[i - 1]);
          if (thisGap > mNodeWidth && positions[i] - childPosition < positionDifference)
          {
            gapIndex = i;
            gap = thisGap;
            positionDifference = positions[i] - childPosition;
          }
        }

        // If there is a valid gap, move the orphan node there
        if (gapIndex > 0)
        {
          // Check if the gap is big enough for multiple nodes and there are more nodes in the list
          if (gap / mNodeWidth > 2 && index + 1 < orphans.Size())
          {
            // The number if nodes to move is either the number that will fit in the gap or all of them
            unsigned nodesToMove = Math::Min((unsigned)(gap / mNodeWidth), orphans.Size() - index);
            // Save the base X position
            float startingXpos = positions[gapIndex];

            // Set the position of the first orphan 
            orphan->mPosition.x = startingXpos - (gap / (nodesToMove + 1));

            // Set the positions of the rest of the orphans to be moved, moving the index forward
            for (unsigned i = 1; i < nodesToMove; ++i)
            {
              ++index;
              orphans[index]->mPosition.x = startingXpos - (gap / (nodesToMove + 1) * (i + 1));
              positions.Insert(orphans[index]->mPosition.x);
            }
          }
          // Otherwise, put this node in the center of the gap
          else
            orphan->mPosition.x = positions[gapIndex] - (gap * 0.5f);
        }
        // If there is no gap, move the node to one of the ends
        else
        {
          // Is the left side closest to the node?
          if (Math::Abs(orphan->mPosition.x - positions.Front()) < Math::Abs(orphan->mPosition.x - positions.Back()))
            orphan->mPosition.x = positions.Front() - mNodeWidth;
          else
            orphan->mPosition.x = positions.Back() + mNodeWidth;
        }

        // Add the orphan to the sorted positions
        positions.Insert(orphan->mPosition.x);
      }
    }

    CheckForCollision(infoByLevel[level], true);
  }
}

//**************************************************************************************************
NodeInfoListType::range SoundNodeGraph::GetNodeInfoList()
{
  mNodeMap.Clear();
  mMaxLevel = 0;

  // Create all node info objects, starting with the system's output node
  CreateInfo(Z::gSound->mOutputNode, nullptr, 0);

  // Make sure no children are above their parents
  bool foundSomething = true;
  while (foundSomething)
  {
    foundSomething = false;

    // Step through each node
    forRange(NodePrintInfo* nodeInfo, mNodeMap.Values())
    {
      // Look at all of the node's children
      forRange(NodePrintInfo* childInfo, nodeInfo->mChildren.All())
      {
        // If the level of the child is higher, change the node's level
        if (childInfo->mLevel > nodeInfo->mLevel)
        {
          foundSomething = true;
          nodeInfo->mLevel = childInfo->mLevel + 1;
        }
      }
    }
  }

  // Sort nodes by level
  Array<Array<NodePrintInfo*>> infoByLevel(mMaxLevel + 1);
  forRange(NodePrintInfo* nodeInfo, mNodeMap.Values())
    infoByLevel[nodeInfo->mLevel].PushBack(nodeInfo);

  // Find first level with the most nodes
  int largestLevel(0);
  for (unsigned i = 0; i < infoByLevel.Size(); ++i)
  {
    if (infoByLevel[i].Size() > infoByLevel[largestLevel].Size())
      largestLevel = i;
  }

  // Do first pass of positioning nodes
  FirstPassPositioning(infoByLevel, largestLevel);

  // Do second pass of positioning
  SecondPassPositioning(infoByLevel, largestLevel);

  // Handle some specific scenarios
  PositionCleanUp(infoByLevel, largestLevel);

  // If there are already node objects in the list, delete them and clear the list
  forRange(NodePrintInfo* info, mNodeInfoList.All())
    delete info;
  mNodeInfoList.Clear();

  // Add the current node data to the list
  forRange(NodePrintInfo* nodeInfo, mNodeMap.Values())
    mNodeInfoList.PushBack(nodeInfo);

  return mNodeInfoList.All();
}

} // namespace Zero
