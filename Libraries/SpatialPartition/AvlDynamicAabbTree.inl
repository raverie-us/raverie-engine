///////////////////////////////////////////////////////////////////////////////
///
/// \file AvlDynamicAabbTree.hpp
/// Implementation of the AvlDynamicTreeNode, AvlDynamicTreePolicy and
/// AvlDynamicAabbTree classes.
///
/// Idea and implementation based heavily upon Erin Catto's Box2D and
/// Erwin Couman's Bullet.
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Zero
{

//-------------------------------------------------------------------AvlDynamicTreeNode

template <typename ClientDataType>
AvlDynamicTreeNode<ClientDataType>::AvlDynamicTreeNode()
{
  mChild1 = nullptr;
  mChild2 = nullptr;
  mParent = nullptr;
  mHeight = 0;
  GetDefaultClientDataValue(mClientData);
}

template <typename ClientDataType>
AvlDynamicTreeNode<ClientDataType>::~AvlDynamicTreeNode()
{
  delete mChild1;
  delete mChild2;
}

template <typename ClientDataType>
Memory::Pool* AvlDynamicTreeNode<ClientDataType>::sDynamicNodePool =
  new Memory::Pool("DynamicNodes", Memory::GetNamedHeap("BroadPhase"),
                    sizeof(AvlDynamicTreeNode<ClientDataType>), 200);

template <typename ClientDataType>
void* AvlDynamicTreeNode<ClientDataType>::operator new(size_t size)
{
  return sDynamicNodePool->Allocate(size);
}

template <typename ClientDataType>
void AvlDynamicTreeNode<ClientDataType>::operator delete(void* pMem, size_t size)
{
  return sDynamicNodePool->Deallocate(pMem, size);
}

template <typename ClientDataType>
bool AvlDynamicTreeNode<ClientDataType>::IsLeaf()
{
  return mChild1 == nullptr;
}

template <typename ClientDataType>
AvlDynamicTreeNode<ClientDataType>* AvlDynamicTreeNode<ClientDataType>::GetSibling()
{
  if(mParent->mChild1 == this)
    return mParent->mChild2;
  return mParent->mChild1;
}

//-------------------------------------------------------------------AvlDynamicTreePolicy

template <typename ClientDataType>
void AvlDynamicTreePolicy<ClientDataType>::InsertNode(NodeType*& root, NodeType* leafNode,
                                                      NodeType* start)
{
  //if we have no root, then this node is the root
  if(root == nullptr)
  {
    root = leafNode;
    root->mParent = nullptr;
    return;
  }

  //traverse until we find the correct leaf node to add at
  NodeType* node = start;
  while(!node->IsLeaf())
  {
    //expand the aabb's of our parent as we go down
    node->mAabb = node->mAabb.Combined(leafNode->mAabb);  
    //choose the correct node between the left and right node
    node = BaseType::SelectNode(node,leafNode);
  }

  //all our objects must be on leaf nodes, so we have
  //to make a new internal node to put the old leaf and the new leaf
  NodeType* oldParent = node->mParent;
  NodeType* newParent = BaseType::CreateInternalNode(oldParent,node,leafNode);
  newParent->mHeight = 1;

  //deal with fixing the root pointer if the tree contained only 1 node
  if(nullptr == oldParent)
    root = newParent;


  NodeType* nodeToBalance = newParent;
  while(nodeToBalance != nullptr)
  {
    nodeToBalance = Balance(root,nodeToBalance);

    FixAabbAndHeight(nodeToBalance);
    nodeToBalance = nodeToBalance->mParent;
  }
}

template <typename ClientDataType>
typename AvlDynamicTreePolicy<ClientDataType>::NodeType* 
  AvlDynamicTreePolicy<ClientDataType>::RemoveNode(NodeType*& root, NodeType* leafNode)
{
  ErrorIf(leafNode->mChild1 != nullptr,"Can only remove leaf nodes.");
  ErrorIf(leafNode->mChild2 != nullptr,"Can only remove leaf nodes.");

  //deal with removing the root
  if(leafNode == root)
  {
    root = nullptr;
    return root;
  }

  NodeType* parent = leafNode->mParent;
  NodeType* grandParent = parent->mParent;
  NodeType* sibling = leafNode->GetSibling();
  //if our parent is the root, then our sibling will have to
  //become the new root
  if(grandParent == nullptr)
  {
    BaseType::DeleteNode(root);
    sibling->mParent = nullptr;
    root = sibling;
    return root;
  }

  //set our sibling to be where our old parent was,
  //then delete our parent
  if(grandParent->mChild1 == parent)
    grandParent->mChild1 = sibling;
  else
    grandParent->mChild2 = sibling;
  sibling->mParent = grandParent;
  BaseType::DeleteNode(parent);

  //work up the tree shrinking the Aabb's to account for us being removed
  while(grandParent)
  {
    grandParent = Balance(root,grandParent);

    FixAabbAndHeight(grandParent);
    grandParent = grandParent->mParent;
  }

  return grandParent;
}

template <typename ClientDataType>
typename AvlDynamicTreePolicy<ClientDataType>::NodeType* 
  AvlDynamicTreePolicy<ClientDataType>::Balance(NodeType*& root, NodeType* node)
{
  if(node->mHeight < 2)
    return node;

  NodeType* B = node->mChild1;
  NodeType* C = node->mChild2;
  int balance = C->mHeight - B->mHeight;

  if(balance > 1)
    return RotateUp(root,node,1);
  else if(balance < -1)
    return RotateUp(root,node,0);
  return node;
}

template <typename ClientDataType>
typename AvlDynamicTreePolicy<ClientDataType>::NodeType* 
  AvlDynamicTreePolicy<ClientDataType>::RotateUp(NodeType*& root, NodeType* oldParent, 
                                                 uint childIndex)
{
  NodeType* newParent = oldParent->mChildren[childIndex];
  uint largeIndex,smallIndex;
  NodeType* smallChild;

  if(newParent->mChild1->mHeight > newParent->mChild2->mHeight)
    largeIndex = 0;
  else
    largeIndex = 1;
  smallIndex = (largeIndex + 1) % 2;
  smallChild = newParent->mChildren[smallIndex];

  //swap new and old parent
  newParent->mChildren[smallIndex] = oldParent;
  newParent->mParent = oldParent->mParent;
  oldParent->mParent = newParent;
  //fix the new parent's parent if it exists to point back down correctly
  if(newParent->mParent != nullptr)
  {
    if(newParent->mParent->mChild1 == oldParent)
      newParent->mParent->mChild1 = newParent;
    else
      newParent->mParent->mChild2 = newParent;
  }
  else
    root = newParent;
  //put the small child under c
  oldParent->mChildren[childIndex] = smallChild;
  smallChild->mParent = oldParent;
  //fix the aabbs and heights of the old and new parent
  FixAabbAndHeight(oldParent);
  FixAabbAndHeight(newParent);

  return newParent;
}

template <typename ClientDataType>
void AvlDynamicTreePolicy<ClientDataType>::FixAabbAndHeight(NodeType* node)
{
  NodeType* child1 = node->mChild1;
  NodeType* child2 = node->mChild2;
  node->mHeight = 1 + Math::Max(child1->mHeight,child2->mHeight);
  node->mAabb = child1->mAabb;
  node->mAabb.Combine(child2->mAabb);
}

//-------------------------------------------------------------------AvlDynamicAabbTree

template <typename ClientDataType>
AvlDynamicAabbTree<ClientDataType>::AvlDynamicAabbTree()
{

}

template <typename ClientDataType>
AvlDynamicAabbTree<ClientDataType>::~AvlDynamicAabbTree()
{
  BaseType::Clear();
}

}//namespace Zero
