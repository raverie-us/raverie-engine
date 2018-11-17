///////////////////////////////////////////////////////////////////////////////
///
/// \file DynamicAabbTree.inl
/// Implementation of the DynamicTreeNode, DynamicTreePolicy and
/// DynamicAabbTree classes.
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

//-------------------------------------------------------------------DynamicTreeNode

template <typename ClientDataType>
DynamicTreeNode<ClientDataType>::DynamicTreeNode()
{
  mChild1 = nullptr;
  mChild2 = nullptr;
  mParent = nullptr;
  GetDefaultClientDataValue(mClientData);
}

template <typename ClientDataType>
DynamicTreeNode<ClientDataType>::~DynamicTreeNode()
{
  delete mChild1;
  delete mChild2;
}

template <typename ClientDataType>
Memory::Pool* DynamicTreeNode<ClientDataType>::sDynamicNodePool =
  new Memory::Pool("DynamicNodes", Memory::GetNamedHeap("BroadPhase"),
                   sizeof(DynamicTreeNode<ClientDataType>), 200);

template <typename ClientDataType>
void* DynamicTreeNode<ClientDataType>::operator new(size_t size)
{
  return sDynamicNodePool->Allocate(size);
}

template <typename ClientDataType>
void DynamicTreeNode<ClientDataType>::operator delete(void* pMem, size_t size)
{
  return sDynamicNodePool->Deallocate(pMem, size);
}

template <typename ClientDataType>
bool DynamicTreeNode<ClientDataType>::IsLeaf()
{
  return mChild1 == nullptr;
}

template <typename ClientDataType>
DynamicTreeNode<ClientDataType>* DynamicTreeNode<ClientDataType>::GetSibling()
{
  if(mParent->mChild1 == this)
    return mParent->mChild2;
  return mParent->mChild1;
}

//-------------------------------------------------------------------DynamicTreePolicy

template <typename ClientDataType>
void DynamicTreePolicy<ClientDataType>::InsertNode(NodeType*& root, NodeType* leafNode,
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

  //deal with fixing the root pointer if the tree contained only 1 node
  if(nullptr == oldParent)
    root = newParent;
}

template <typename ClientDataType>
typename DynamicTreePolicy<ClientDataType>::NodeType* 
  DynamicTreePolicy<ClientDataType>::RemoveNode(NodeType*& root, NodeType* leafNode)
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
    Aabb oldAabb = grandParent->mAabb;
    grandParent->mAabb = grandParent->mChild1->mAabb;
    grandParent->mAabb = grandParent->mAabb.Combined(grandParent->mChild2->mAabb);

    //if our old Aabb and our new Aabb are of the same size, then there
    //is no point in continuing up the tree since none of our parent's will grow
    int result = memcmp(&oldAabb,&(grandParent->mAabb),sizeof(Aabb));
    if(result == 0)
      return grandParent;

    grandParent = grandParent->mParent;
  }
  return grandParent;
}

//-------------------------------------------------------------------DynamicAabbTree

template <typename ClientDataType>
DynamicAabbTree<ClientDataType>::DynamicAabbTree()
{
  mPath = 0;
}

template <typename ClientDataType>
DynamicAabbTree<ClientDataType>::~DynamicAabbTree()
{
  BaseType::Clear();
}

template <typename ClientDataType>
void DynamicAabbTree<ClientDataType>::Rebalance(uint iterations)
{
  if(mRoot == nullptr)
    return;

  for(uint i = 0; i < iterations; ++i)
  {
    uint bit = 0;
    typename BaseType::NodeType* node = mRoot;

    //shuffle down the leaves based upon a path variable.
    //each bit represents whether to go left or right at the level
    //corresponding to bit #.
    while(!node->IsLeaf())
    {
      uint selection = (mPath >> bit) & 0x1;
      if(selection == 0)
        node = node->mChild1;
      else
        node = node->mChild2;

      //since path is 32 bits, we need to keep
      //bit between 0 and 31
      bit = (bit + 1) & 0x1F;
    }
    ++mPath;

    MyPolicyType::RemoveNode(mRoot,node);
    MyPolicyType::InsertNode(mRoot,node,mRoot);
  }
}

}//namespace Zero
