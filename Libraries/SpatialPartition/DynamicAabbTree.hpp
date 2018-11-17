///////////////////////////////////////////////////////////////////////////////
///
/// \file DynamicAabbTree.hpp
/// Declaration of the DynamicTreeNode, DynamicTreePolicy and
/// DynamicAabbTree classes.
///
/// Idea and implementation based heavily upon Erin Catto's Box2D and
/// Erwin Couman's Bullet.
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Memory{class Pool;}

///Node used for the dynamic aabb tree. Different from the static
///tree node because we need a parent pointer.
template <typename ClientDataType>
struct DynamicTreeNode
{
  DynamicTreeNode();
  ~DynamicTreeNode();

  static Memory::Pool* sDynamicNodePool;
  static void* operator new(size_t size);
  static void operator delete(void* pMem, size_t size);

  bool IsLeaf();
  //Assumes that a parent exists
  DynamicTreeNode* GetSibling();

  Aabb mAabb;
  ClientDataType mClientData;

  DynamicTreeNode* mParent;

  DynamicTreeNode* mChild1;
  DynamicTreeNode* mChild2;
};

///Policy for the DynamicAabbTree that determines
///how insertions and removals are handled.
template <typename ClientDataType>
struct DynamicTreePolicy : public BaseDynamicTreePolicy<DynamicTreeNode<ClientDataType> >
{
  typedef DynamicTreeNode<ClientDataType> NodeType;
  typedef ClientDataType ClientDataTypeDef;
  typedef BaseDynamicTreePolicy<DynamicTreeNode<ClientDataType> > BaseType;

  ///Inserts the given leaf node at the starting node. Generally, start is
  ///the root, but when updating a node it may be somewhere in the middle
  //of the tree.
  static void InsertNode(NodeType*& root, NodeType* leafNode, NodeType* start);
  ///Removes the given node. Returns the last node that did not have to be
  ///resized from removal.
  static NodeType* RemoveNode(NodeType*& root, NodeType* leafNode);
};

///A Hierarchical AabbTree that is meant for dynamic objects. Used to have a
///fast BroadPhases for dynamic objects where raycasting is an important issue.
///Raycasting performs much faster than most other dynamic BroadPhases, and the
///normal update is decently fast.
template <typename ClientDataType>
class DynamicAabbTree : public BaseDynamicAabbTree<DynamicTreePolicy<ClientDataType> >
{
public:
  typedef DynamicAabbTree<ClientDataType> TreeType;
  typedef BaseDynamicAabbTree<DynamicTreePolicy<ClientDataType> > BaseType;
  typedef typename BaseType::PolicyTypeDef MyPolicyType;

  using BaseType::mRoot;

  DynamicAabbTree();
  ~DynamicAabbTree();

  ///Rebalances the tree by picking one node and reinserting it.
  ///Should be called every so often to avoid the tree getting unbalanced.
  ///(Called in Query)
  void Rebalance(uint iterations);

private:

  ///Represents what pathway to take when rebalancing the tree.
  ///The path represents taking the left or right child at a level
  ///based upon the bit #.
  uint mPath;
};

typedef DynamicTreeNode<void*> DynamicTreeNodeDefault;
typedef DynamicAabbTree<void*> DynamicAabbTreeDefault;

}//namespace Zero

#include "DynamicAabbTree.inl"
