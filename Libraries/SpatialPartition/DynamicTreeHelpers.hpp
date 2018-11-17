///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//Callback is expected to have a method called QueryCallback (world space trees)
template <typename CallbackType, typename NodeType>
void TreeSelfQuery(CallbackType* callback, NodeType* root)
{
  if(root == nullptr || root->IsLeaf())
    return;

  typedef Pair<NodeType*, NodeType*> NodePair;
  static Array<NodePair> nodePairs;

  nodePairs.Reserve(256);
  nodePairs.PushBack(MakePair(root->mChild1,root->mChild2));

  for(uint i = 0; i < nodePairs.Size(); ++i)
  {
    NodePair& pair = nodePairs[i];
    NodeType* nodeA = pair.first;
    NodeType* nodeB = pair.second;

    if(!nodeA->IsLeaf())
      nodePairs.PushBack(MakePair(nodeA->mChild1,nodeA->mChild2));
    if(!nodeB->IsLeaf())
      nodePairs.PushBack(MakePair(nodeB->mChild1,nodeB->mChild2));
  }

  while(!nodePairs.Empty())
  {
    NodeType* nodeA = nodePairs.Back().first;
    NodeType* nodeB = nodePairs.Back().second;
    nodePairs.PopBack();

    //if the nodes don't overlap, we don't care
    if(!nodeA->mAabb.Overlap(nodeB->mAabb))
      continue;

    if(nodeA->IsLeaf())
    {
      if(nodeB->IsLeaf())
      {
        callback->QueryCallback(nodeA,nodeB);
        continue;
      }
      nodePairs.PushBack(MakePair(nodeA,nodeB->mChild1));
      nodePairs.PushBack(MakePair(nodeA,nodeB->mChild2));
    }
    else if(nodeB->IsLeaf())
    {
      nodePairs.PushBack(MakePair(nodeA->mChild1,nodeB));
      nodePairs.PushBack(MakePair(nodeA->mChild2,nodeB));
    }
    else
    {
      nodePairs.PushBack(MakePair(nodeA->mChild1,nodeB->mChild1));
      nodePairs.PushBack(MakePair(nodeA->mChild1,nodeB->mChild2));
      nodePairs.PushBack(MakePair(nodeA->mChild2,nodeB->mChild1));
      nodePairs.PushBack(MakePair(nodeA->mChild2,nodeB->mChild2));
    }
  }
}

template <typename CallbackType, typename NodeType>
void QueryTreeVsTree(CallbackType* callback, NodeType* rootA, NodeType* rootB)
{
  if(rootA == nullptr || rootB == nullptr)
    return;

  //not that it matters, but this is always ordered such that pair.first is
  //from this treeA and pair.second is from treeB.
  typedef Pair<NodeType*, NodeType*> NodePair;
  Array<NodePair> nodes;
  nodes.Reserve(256);
  nodes.PushBack(MakePair(rootA,rootB));

  while(!nodes.Empty())
  {
    NodeType* nodeA = nodes.Back().first;
    NodeType* nodeB = nodes.Back().second;
    nodes.PopBack();

    //if the nodes don't overlap, we don't care
    if(!nodeA->mAabb.Overlap(nodeB->mAabb))
      continue;

    //here's the 3 cases for what to do with a node:
    //1. if both nodes are a leaf then just send the callback
    //2. if one is a leaf and the other an internal, split the internal node
    //3. else, both nodes are internal, split the largest node
    //the reasoning for splitting the largest node is to most quickly reduce
    //the total volume being tested (taken from Gino Van Den Bergen's 
    //Efficient Collision Detection of Complex Deformable Models using AABB Trees)

    if(nodeA->IsLeaf())
    {
      if(nodeB->IsLeaf())
      {
        callback->QueryCallback(nodeA,nodeB);
        continue;
      }
      nodes.PushBack(MakePair(nodeA,nodeB->mChild1));
      nodes.PushBack(MakePair(nodeA,nodeB->mChild2));
    }
    else if(nodeB->IsLeaf())
    {
      nodes.PushBack(MakePair(nodeA->mChild1,nodeB));
      nodes.PushBack(MakePair(nodeA->mChild2,nodeB));
    }
    else
    {
      if(nodeA->mAabb.GetVolume() > nodeB->mAabb.GetVolume())
      {
        nodes.PushBack(MakePair(nodeA->mChild1,nodeB));
        nodes.PushBack(MakePair(nodeA->mChild2,nodeB));
      }
      else
      {
        nodes.PushBack(MakePair(nodeA,nodeB->mChild1));
        nodes.PushBack(MakePair(nodeA,nodeB->mChild2));
      }
    }
  }
}

}//namespace Zero
