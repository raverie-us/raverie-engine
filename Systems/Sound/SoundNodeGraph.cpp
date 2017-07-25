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
void SoundNodeGraph::CreateInfo(Audio::SoundNode* node, Audio::SoundNode* outputNode, int level)
{
  // Keep track of the highest level
  if (level > mMaxLevel)
    mMaxLevel = level;

  NodePrintInfo* info = mNodeMap.FindValue(node->NodeID, nullptr);

  // Node wasn't in the map, so we need to create it
  if (!info)
  {
    info = new NodePrintInfo(level, node->Name, node->HasAudibleOutput());
    mNodeMap[node->NodeID] = info;

    // If there is an output connection, add it to the list
    if (outputNode)
    {
      NodePrintInfo* child = mNodeMap.FindValue(outputNode->NodeID, nullptr);
      if (child)
      {
        info->mChildren.PushBack(child);
        child->mParents.PushBack(info);
      }
    }
  }
  // A node with this ID is already in the map
  else
  {
    // Don't count this as a level
    --level;

    // If the output node isn't the same ID, add it as a connection
    if (outputNode && outputNode->NodeID != node->NodeID)
    {
      NodePrintInfo* child = mNodeMap.FindValue(outputNode->NodeID, nullptr);
      if (child)
      {
        info->mChildren.PushBack(child);
        child->mParents.PushBack(info);
      }
    }
  }

  // Call this function on all inputs of this node
  const Array<Audio::SoundNode*>* inputs = node->GetInputs();
  for (unsigned i = 0; i < inputs->Size(); ++i)
  {
    CreateInfo((*inputs)[i], node, level + 1);
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Moves overlapping nodes away from each other
void SoundNodeGraph::CheckForCollision(NodeInfoListType& list, float& minXpos)
{
  float moveIncrement(mNodeWidth * 0.1f);
  float minDistance(mNodeWidth * 0.85f);

  bool foundSomething(true);
  unsigned count(0);

  // Keep looking as long as something was moved last loop and we haven't reached max loops
  while (foundSomething && count < 5)
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

        // If the distance between the two nodes is too close
        if (Math::Abs(list[i]->mPosition.x - list[j]->mPosition.x) < minDistance)
        {
          foundSomething = true;

          NodePrintInfo* info1 = list[i];
          NodePrintInfo* info2 = list[j];

          // Check if one node has parents and the other doesn't
          NodePrintInfo* noParentNode(nullptr);
          if (info1->mParents.Empty() && !info2->mParents.Empty())
            noParentNode = info1;
          else if (!info1->mParents.Empty() && info2->mParents.Empty())
            noParentNode = info2;

          // If yes, and the orphan node would be moved, don't do anything
          if (noParentNode && noParentNode->mChildren.Size() == 1 
            && noParentNode->mChildren[0]->mParents.Size() >= 3)
          {
            foundSomething = false;
          }
          // Otherwise move the two nodes apart
          else
          {
            if (info1->mPosition.x <= info2->mPosition.x)
            {
              info1->mPosition.x -= moveIncrement;
              info2->mPosition.x += moveIncrement;
            }
            else
            {
              info1->mPosition.x += moveIncrement;
              info2->mPosition.x -= moveIncrement;
            }

            if (info1->mPosition.x < minXpos)
              minXpos = info1->mPosition.x;
            if (info2->mPosition.x < minXpos)
              minXpos = info2->mPosition.x;
          }
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
// (calculates positions based on top left corner being [0,0])
void SoundNodeGraph::FirstPassPositioning(Array<NodeInfoListType>& infoByLevel, int largestLevel)
{
  float width = (float)infoByLevel[largestLevel].Size() * mNodeWidth;
  float height = (float)(infoByLevel.Size() - 1) * mNodeHeight;

  // Find positions of nodes on first level
  // If only one node, just place in the center
  if (infoByLevel[0].Size() == 1)
    infoByLevel[0][0]->mPosition = Vec2(width * 0.5f, -height - mNodeHeight);
  // If multiple nodes, space equally
  else
  {
    float spacing = 1.0f / infoByLevel[0].Size() * width;
    for (unsigned i = 0; i < infoByLevel[0].Size(); ++i)
      infoByLevel[0][i]->mPosition = Vec2((spacing * 0.5f) + (i * spacing), -height - mNodeHeight);
  }

  // Find positions of other nodes, up to largest level
  float totalLevels = (float)infoByLevel.Size() - 1.0f;
  for (int level = 1; level <= largestLevel; ++level)
  {
    float yPos = ((1.0f - ((float)level / totalLevels)) * -height) - mNodeHeight;
    unsigned levelCount = infoByLevel[level].Size();
    float spacing = 1.0f / levelCount * width;
    float halfSpacing = spacing * 0.5f;

    // Go through all nodes in this level
    for (unsigned j = 0; j < levelCount; ++j)
    {
      NodePrintInfo* info = infoByLevel[level][j];

      // Find average position of all connections
      if (info->mChildren.Size() == 1)
        info->mConnectAvgPos = info->mChildren[0]->mPosition.x;
      else
      {
        info->mConnectAvgPos = 0.0f;
        for (unsigned k = 0; k < info->mChildren.Size(); ++k)
          info->mConnectAvgPos += info->mChildren[k]->mPosition.x;
        info->mConnectAvgPos /= info->mChildren.Size();
      }

      // If there are multiple nodes, space them equally
      if (levelCount > 1)
        info->mPosition = Vec2(halfSpacing + (j * spacing), yPos);
      // Otherwise, just set to the connection average position
      else
        info->mPosition = Vec2(info->mConnectAvgPos, yPos);

      info->mPositionSet = true;
    }

    // If there are multiple nodes in this level, make sure they're close to their connections
    if (levelCount > 1)
    {
      // Sort by average connection position
      SortedArray<NodeInfoSortingPosition> sortedNodes;
      for (unsigned j = 0; j < levelCount; ++j)
        sortedNodes.Insert(NodeInfoSortingPosition(infoByLevel[level][j]));

      // Move node positions to match with connection positions
      bool foundSomething(true);
      while (foundSomething)
      {
        foundSomething = false;
        for (unsigned j = 0; j < levelCount - 1; ++j)
        {
          NodePrintInfo* first = sortedNodes[j].mNodeInfo;
          NodePrintInfo* second = sortedNodes[j + 1].mNodeInfo;
          if (first->mPosition.x > second->mPosition.x)
          {
            float temp = first->mPosition.x;
            first->mPosition.x = second->mPosition.x;
            second->mPosition.x = temp;
            foundSomething = true;
          }
        }
      }
    }
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Does the final positioning on all levels except the largest
void SoundNodeGraph::SecondPassPositioning(Array<NodeInfoListType> &infoByLevel, 
  int largestLevel, float& minXpos)
{
  // For levels lower than largest, set their positions to the average 
  // of their parents, then space out with collision checking
  for (int level = largestLevel - 1; level >= 0; --level)
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

        if (parentAverage < minXpos)
          minXpos = parentAverage;
      }
    }

    // Check for overlapping and resolve
    CheckForCollision(infoByLevel[level], minXpos);
  }

  // For levels above largest, set their positions to the average 
  // of their children, then space out with collision checking
  // (these nodes haven't had their y position set yet)
  float totalLevels = (float)infoByLevel.Size() - 1.0f;
  float height = (float)(infoByLevel.Size() - 1) * mNodeHeight;
  for (unsigned level = largestLevel + 1; level < infoByLevel.Size(); ++level)
  {
    float yPos = ((1.0f - ((float)level / totalLevels)) * -height) - mNodeHeight;

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

      if (childAverage < minXpos)
        minXpos = childAverage;
    }

    // Check for overlapping and resolve
    CheckForCollision(infoByLevel[level], minXpos);
  }
}

//**************************************************************************************************
// Helper function for GetNodeInfoList
// Cleans up node positions for certain scenarios
void SoundNodeGraph::PositionCleanUp(Array<NodeInfoListType> &infoByLevel, 
  int largestLevel, float& minXpos)
{
  // Go through all levels
  for (unsigned level = 0; level < infoByLevel.Size(); ++level)
  {
    // Go through all nodes in this level
    for (unsigned i = 0; i < infoByLevel[level].Size(); ++i)
    {
      NodePrintInfo* node = infoByLevel[level][i];

      // Don't do this check for nodes on the largest level
      if (level != largestLevel - 1)
      {
        // Walk through parents of this node, saving positions of nodes with parents
        // and indexes of nodes without parent
        SortedArray<float> positions;
        Array<unsigned> orphans;
        for (unsigned j = 0; j < node->mParents.Size(); ++j)
        {
          if (node->mParents[j]->mParents.Empty())
            orphans.PushBack(j);
          else
            positions.Insert(node->mParents[j]->mPosition.x);
        }

        // Is there at least one orphan and at least two with parents?
        if (orphans.Size() >= 1 && positions.Size() >= 2)
        {
          // Walk through this node's parents with no parents
          for (unsigned j = 0; j < orphans.Size(); ++j)
          {
            NodePrintInfo* orphan = node->mParents[orphans[j]];

            // Find largest gap
            float gap(0.0f);
            int gapIndex(-1);
            for (unsigned k = 1; k < positions.Size(); ++k)
            {
              float thisGap = positions[k] - positions[k - 1];
              if (thisGap > mNodeWidth && thisGap > gap)
              {
                gapIndex = k;
                gap = thisGap;
              }
            }

            // If there is a valid gap, move the orphan node there
            if (gapIndex > 0)
            {
              // If the gap is big enough for two nodes and there is another node in the list,
              // space them both equally in the gap
              if (gap / mNodeWidth > 2 && j + 1 < orphans.Size())
              {
                // Set this node's position
                orphan->mPosition.x = positions[gapIndex] - (gap / 3.0f * 2.0f);
                // Set the other node's position and add it to the sorted list
                node->mParents[orphans[j + 1]]->mPosition.x = positions[gapIndex] - (gap / 3.0f);
                positions.Insert(node->mParents[orphans[j + 1]]->mPosition.x);
                // Skip the next node
                ++j;
              }
              // Otherwise, put this node in the center of the gap
              else
                orphan->mPosition.x = positions[gapIndex] - (gap * 0.5f);
            }
            // If there is no gap, move the node to one of the ends
            else
            {
              // Is the left side closest to the node?
              if (node->mPosition.x - positions.Front() < positions.Back() - node->mPosition.x)
              {
                orphan->mPosition.x = positions.Front() - mNodeWidth;

                if (orphan->mPosition.x < minXpos)
                  minXpos = orphan->mPosition.x;
              }
              else
                orphan->mPosition.x = positions.Back() + mNodeWidth;
            }

            // Add the orphan to the sorted positions
            positions.Insert(orphan->mPosition.x);
          }
        }
      }

      // Does this node have one parent, the parent has other children,
      // the node has one child, and the child has other parents?
      if (node->mParents.Size() == 1 && node->mParents[0]->mChildren.Size() > 1
        && node->mChildren.Size() == 1 && node->mChildren[0]->mParents.Size() > 1)
      {
        NodePrintInfo* child = node->mChildren[0];
        NodePrintInfo* parent = node->mParents[0];

        // Find which of the child's parents are closest to the node's parent
        float distance(100000.0f);
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
    }
  }
}

//**************************************************************************************************
NodeInfoListType::range SoundNodeGraph::GetNodeInfoList()
{
  float minXpos(0.0f);
  mNodeMap.Clear();

  // Go through all current spaces
  forRange(SoundSpace& space, Z::gSound->mSpaces.All())
  {
    // Don't print the editor spaces
    if (((Space*)space.GetOwner())->IsEditorMode())
      continue;

    // Don't print spaces with no graph
    if (!space.mInputNode->GetHasInputs())
      continue;

    // Add all nodes in this space
    CreateInfo(space.mInputNode->mNode, nullptr, 0);
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
  SecondPassPositioning(infoByLevel, largestLevel, minXpos);

  // Handle some specific scenarios
  PositionCleanUp(infoByLevel, largestLevel, minXpos);

  // Move everything over if there are nodes with a negative X position
  // (for UI display, X positions have to be positive)
  if (minXpos < 0)
  {
    for (unsigned i = 0; i < infoByLevel.Size(); ++i)
    {
      for (unsigned j = 0; j < infoByLevel[i].Size(); ++j)
      {
        infoByLevel[i][j]->mPosition.x -= minXpos;
      }
    }
  }

  // Add additional room between spaces
  for (unsigned i = 1; i < infoByLevel[0].Size(); ++i)
    AddSpacePadding(infoByLevel[0][i], i * mNodeWidth * 0.75f);

  // If there are already node objects in the list, delete them and clear the list
  forRange(NodePrintInfo* info, mNodeInfoList.All())
    delete info;
  mNodeInfoList.Clear();

  // Add the current node data to the list
  forRange(NodePrintInfo* nodeInfo, mNodeMap.Values())
    mNodeInfoList.PushBack(nodeInfo);

  return mNodeInfoList.All();
}

}