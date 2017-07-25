///////////////////////////////////////////////////////////////////////////////
///
/// \file AabbTreeNode.hpp
/// Declaration of the AabbNode struct.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Serializer;

///A node for an aabb tree that does not need parent pointers.
template <typename ClientDataType>
struct AabbNode
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef AabbNode<ClientDataType> NodeType;
  AabbNode();

  void SetLeaf(DataType& data);
  void SetChildren(NodeType* child1, NodeType* child2);

  bool IsLeaf() const;

  ///The first 4 bytes of this struct are used for a free list.
  Aabb mAabb;

  //remove leaf var later if possible
  bool mLeaf;

  ClientDataType mClientData;
  AabbNode* mChild1;
  AabbNode* mChild2;
};

typedef AabbNode<void*> AabbNodeDefault;

template <typename ClientDataType>
void SerializeNode(Serializer& stream, AabbNode<ClientDataType>& node);

}//namespace Zero

#include "SpatialPartition/AabbTreeNode.inl"
