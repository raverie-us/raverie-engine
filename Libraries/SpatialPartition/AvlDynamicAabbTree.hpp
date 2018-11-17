///////////////////////////////////////////////////////////////////////////////
///
/// \file AvlDynamicAabbTree.hpp
/// Declaration of the AvlDynamicTreeNode, AvlDynamicTreePolicy and
/// AvlDynamicAabbTree classes.
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

///Node used for the Avl balanced dynamic aabb tree. Different from the static
///tree node because we need a parent pointer. Different from the normal
///DynamicAabbTree because we need a height.
template <typename ClientDataType>
struct AvlDynamicTreeNode
{
  AvlDynamicTreeNode();
  ~AvlDynamicTreeNode();

  static Memory::Pool* sDynamicNodePool;
  static void* operator new(size_t size);
  static void operator delete(void* pMem, size_t size);

  bool IsLeaf();
  //Assumes that a parent exists
  AvlDynamicTreeNode* GetSibling();

  Aabb mAabb;
  ClientDataType mClientData;

  AvlDynamicTreeNode* mParent;

  union
  {
    struct
    {
      AvlDynamicTreeNode* mChild1;
      AvlDynamicTreeNode* mChild2;
    };
    AvlDynamicTreeNode* mChildren[2];
  };

  /// The height of this current node. Height of 0 means a leaf node.
  /// Height increases as you go up the tree.
  uint mHeight;
};

///Policy for the AvlDynamicAabbTree that determines
///how insertions and removals are handled.
template <typename ClientDataType>
struct AvlDynamicTreePolicy : public BaseDynamicTreePolicy<AvlDynamicTreeNode<ClientDataType> >
{
  typedef AvlDynamicTreeNode<ClientDataType> NodeType;
  typedef ClientDataType ClientDataTypeDef;
  typedef BaseDynamicTreePolicy<AvlDynamicTreeNode<ClientDataType> > BaseType;

  ///Inserts the given leaf node at the starting node. Generally, start is
  ///the root, but when updating a node it may be somewhere in the middle
  //of the tree.
  static void InsertNode(NodeType*& root, NodeType* leafNode, NodeType* start);
  ///Removes the given node. Returns the last node that did not have to be
  ///resized from removal.
  static NodeType* RemoveNode(NodeType*& root, NodeType* leafNode);

  //Takes the given node and performs an AVL rotation to balance it's children.
  static NodeType* Balance(NodeType*& root, NodeType* node);
  static NodeType* RotateUp(NodeType*& root, NodeType* oldParent, uint childIndex);
  static void FixAabbAndHeight(NodeType* node);
};

///A Hierarchical AabbTree that is meant for dynamic objects. Used to have a
///fast BroadPhases for dynamic objects where raycasting is an important issue.
///Raycasting performs much faster than most other dynamic BroadPhases, and the
///normal update is decently fast.
///This tree uses Avl rotations to balance itself.
template <typename ClientDataType>
class AvlDynamicAabbTree : public BaseDynamicAabbTree<AvlDynamicTreePolicy<ClientDataType> >
{
public:
  typedef AvlDynamicAabbTree<ClientDataType> TreeType;
  typedef BaseDynamicAabbTree<AvlDynamicTreePolicy<ClientDataType> > BaseType;

  AvlDynamicAabbTree();
  ~AvlDynamicAabbTree();

  ///Currently need this since the broadphase expects this.
  ///Balancing is taken care of during Insert and removal.
  void Rebalance(uint iterations) {}

private:
  
};

}//namespace Zero

#include "AvlDynamicAabbTree.inl"
