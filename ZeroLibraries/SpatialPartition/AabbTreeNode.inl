///////////////////////////////////////////////////////////////////////////////
///
/// \file AabbTreeNode.cpp
/// Implementation of the AabbNode struct.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Zero
{

template <typename ClientDataType>
AabbNode<ClientDataType>::AabbNode()
{
  mChild1 = nullptr;
  mChild2 = nullptr;
  GetDefaultClientDataValue(mClientData);
  mLeaf = true;
}

template <typename ClientDataType>
void AabbNode<ClientDataType>::SetLeaf(DataType& data)
{
  mChild1 = nullptr;
  mChild2 = nullptr;
  mAabb = data.mAabb;
  mClientData = data.mClientData;
  mLeaf = true;
}

template <typename ClientDataType>
void AabbNode<ClientDataType>::SetChildren(NodeType* child1, NodeType* child2)
{
  mChild1 = child1;
  mChild2 = child2;
  GetDefaultClientDataValue(mClientData);
  mAabb = child1->mAabb;
  mAabb.Combine(child2->mAabb);
  mLeaf = false;
}

template <typename ClientDataType>
bool AabbNode<ClientDataType>::IsLeaf() const
{
  return mLeaf;
}

template <typename ClientDataType>
void SerializeNode(Serializer& stream, AabbNode<ClientDataType>& node)
{
  //Serialize the Aabb
  stream.SerializeField("AabbMin", node.mAabb.mMin);
  stream.SerializeField("AabbMax", node.mAabb.mMax);
  //Client data
  stream.SerializeField("ClientData", node.mClientData);
}

// SerializeNode is a function and cannot specialize on a pointer type
//template <typename ClientDataType>
//void SerializeNode<ClientDataType*>(Serializer& stream, AabbNode<ClientDataType*>& node)
//{
//  ErrorIf(true, "Cannot Serialize a Static Aabb-Tree with client data "
//                "of pointer type.");
//}

}//namespace Zero
