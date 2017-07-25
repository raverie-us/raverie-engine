///////////////////////////////////////////////////////////////////////////////
///
/// \file StaticAabbTree.cpp
/// Implementation of the StaticAabbTree class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Zero
{

template <typename ClientDataType>
StaticAabbTree<ClientDataType>::StaticAabbTree()
{
  mRoot = nullptr;
  mProxyCount = 0;

  mConstructMethod = TopDown;
  mStopCriteria = XPrimitives;
  SetPartitionMethod(PartitionMethods::MinimizeVolumeSum);
}

template <typename ClientDataType>
StaticAabbTree<ClientDataType>::~StaticAabbTree()
{
  DeleteTree();
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::Serialize(Serializer& stream)
{
  SerializeEnumName(PartitionMethods, mPartitionMethod);

  if(stream.GetMode() == SerializerMode::Loading)
    SetPartitionMethod(mPartitionMethod);
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::CreateProxy(BroadPhaseProxy& proxy, 
                                                 DataType& data)
{
  //Allocate a new node and set the leaf to be the data passed in.
  //The proxy is a pointer to the node. Don't change the tree yet though, 
  //add the new node to an array for the next construction phase.
  NodePointer node = new NodeType();
  node->SetLeaf(data);
  mNodesAdded.PushBack(node);
  proxy = BroadPhaseProxy(node);
  ++mProxyCount;
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::RemoveProxy(BroadPhaseProxy& proxy)
{
  //Put the proxy into a set for the next time the tree is destroyed.
  NodePointer node = (NodePointer)proxy.ToVoidPointer();
  mNodesRemoved.Insert(node);
  --mProxyCount;
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::UpdateProxy(BroadPhaseProxy& proxy,
                                                 DataType& data)
{
  //just set the new data on the node, a new construction has to take
  //place to update the tree though
  NodePointer node = (NodePointer)proxy.ToVoidPointer();
  //there could be an update where our client data changed
  //so make sure to update it (ie. a remove->Insert)
  node->mClientData = data.mClientData;
  mUpdateNodes.PushBack(UpdatePair(node, data));
}

template <typename ClientDataType>
ClientDataType& StaticAabbTree<ClientDataType>::GetClientData(BroadPhaseProxy& proxy)
{
  NodePointer node = (NodePointer)proxy.ToVoidPointer();
  return node->mClientData;
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>:: CountProxies()
{
  if(mRoot == nullptr)
  {
    mProxyCount = 0;
    return;
  }

  //make better!!! (stack space or something)
  Array<NodePointer> stack;
  stack.PushBack(mRoot);

  while(!stack.Empty())
  {
    NodePointer node = stack.Back();
    stack.PopBack();

    //if we have a leaf node, cout it as a proxy
    if(node->IsLeaf())
    {
      ++mProxyCount;
      continue;
    }

    //since this is an internal node, put it's children onto the stack for
    //further iteration.
    stack.PushBack(node->mChild1);
    stack.PushBack(node->mChild2);
  }
}

template <typename ClientDataType>
uint StaticAabbTree<ClientDataType>::GetTotalProxyCount() const
{
  return mProxyCount;
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::Construct()
{
  //update all of the nodes that need updating
  typename UpdateArray::range range = mUpdateNodes.All();
  for(; !range.Empty(); range.PopFront())
  {
    UpdatePair& pair = range.Front();
    NodePointer node = pair.first;
    node->mClientData = pair.second.mClientData;
    node->mAabb = pair.second.mAabb;
  }
  mUpdateNodes.Clear();

  //delete the internal nodes while collecting the
  //internal nodes along with the new nodes to be added
  DeleteInternalNodes(mNodesAdded);

  if(mNodesAdded.Empty())
    return;

  //now build the tree from all of these leaf nodes
  mRoot = BuildTreeTopDownNodes<NodeType>(mNodesAdded, CurrPartitionMethod);
  mNodesAdded.Clear();
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::Destruct()
{
  //We only want to remove the internal nodes since we want any proxies
  //handed out to still be valid. We do have to free the internal
  //nodes though, since we do not use them from tree to tree.
  //Therefore, put the leaf nodes into the nodes
  //to be added list for the next construction.
  DeleteInternalNodes(mNodesAdded);
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::DeleteTree()
{
  //Deleting a tree consists of deleting all of the elements in the tree.
  //Therefore, we can delete all of the internal nodes and then delete
  //the remaining leaf nodes.
  Array<NodePointer> leafNodes;
  DeleteInternalNodes(leafNodes);

  for(uint i = 0; i < leafNodes.Size(); ++i)
    delete leafNodes[i];
  leafNodes.Clear();
  mProxyCount = 0;
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::Draw(int level, uint debugFlags)
{
  //if(!(debugFlags & Physics::DebugDraw::DrawBroadPhase))
    //return;

  if(mRoot == nullptr)
    return;

  if(level == -1)
    DrawTree(mRoot);
  else
    DrawLevel(mRoot, 0, level);
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::DrawLevel(NodePointer node, uint currLevel,
                                               uint level)
{
  if(node == nullptr)
    return;

  if(currLevel == level)
  {
    gDebugDraw->Add(Debug::Obb(node->mAabb).Color(Color::MintCream));
    return;
  }

  DrawLevel(node->mChild1, currLevel + 1, level);
  DrawLevel(node->mChild2, currLevel + 1, level);
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::SetPartitionMethod(PartitionMethods::Enum method)
{
  mPartitionMethod = method;
  if(mPartitionMethod == PartitionMethods::MinimizeVolumeSum)
    CurrPartitionMethod = &MinimizeVolumeSumNodes<NodeType>;
  else if(mPartitionMethod == PartitionMethods::MinimuzeSurfaceAreaSum)
    CurrPartitionMethod = &MinimizeSurfaceAreaSumNodes<NodeType>;
  else if(mPartitionMethod == PartitionMethods::MidPoint)
    CurrPartitionMethod = &MidPointNodes<NodeType>;
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::DrawTree(NodePointer node)
{
  if(node == nullptr)
    return;

  gDebugDraw->Add(Debug::Obb(node->mAabb).Color(Color::MintCream));

  DrawTree(node->mChild1);
  DrawTree(node->mChild2);
}

template <typename ClientDataType>
void StaticAabbTree<ClientDataType>::DeleteInternalNodes(Array<NodePointer>& leafNodes)
{
  if(mRoot == nullptr)
    return;

  //make better!!! (stack space or something)
  Array<NodePointer> stack;
  stack.PushBack(mRoot);

  while(!stack.Empty())
  {
    NodePointer node = stack.Back();
    stack.PopBack();

    //if we have a leaf node, we have to hold onto it for later building
    //unless it is in the remove set, then we have to delete it.
    if(node->IsLeaf())
    {
      if(mNodesRemoved.Find(node).Empty())
        leafNodes.PushBack(node);
      else
        delete node;
      continue;
    }

    //since this is an internal node, put it's children onto the stack for
    //further iteration and then delete this node.
    stack.PushBack(node->mChild1);
    stack.PushBack(node->mChild2);

    delete node;
  }

  mRoot = nullptr;
  mNodesRemoved.Clear();
}

template <typename ClientDataType>
void SerializeAabbTree(Serializer& stream, StaticAabbTree<ClientDataType>& tree)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    stream.StartPolymorphic("StaticAabbTree");
    SerializeAabbTree(stream, tree.mRoot);
    stream.EndPolymorphic();
  }
  else
  {
    PolymorphicNode node;
    if(stream.GetPolymorphic(node))
    {
      tree.mRoot = SerializeAabbTree<ClientDataType>(stream);
      tree.CountProxies();
      stream.EndPolymorphic();
    }
  }
}

template <typename ClientDataType>
void SerializeAabbTree(Serializer& stream, AabbNode<ClientDataType>* node)
{
  if(node == nullptr)
    return;

  stream.StartPolymorphic("Node");

  //Serialize the node
  SerializeNode(stream, *node);

  //Serialize the left
  SerializeAabbTree<ClientDataType>(stream, node->mChild1);
  SerializeAabbTree<ClientDataType>(stream, node->mChild2);

  //End the polymorphic node
  stream.EndPolymorphic();
}

template <typename ClientDataType>
AabbNode<ClientDataType>* SerializeAabbTree(Serializer& stream)
{
  typedef AabbNode<ClientDataType> Node;

  //Attempt to get a polymorphic node
  PolymorphicNode polyNode;
  if(stream.GetPolymorphic(polyNode))
  {
    //Allocate the node
    Node* node = new Node();

    //Serialize the node
    SerializeNode(stream, *node);

    //Serialize the left
    node->mChild1 = SerializeAabbTree<ClientDataType>(stream);
    node->mChild2 = SerializeAabbTree<ClientDataType>(stream);

    //If both children are null, it's a leaf node
    if(node->mChild1 == nullptr && node->mChild2 == nullptr)
      node->mLeaf = true;
    else
      node->mLeaf = false;

    //End the polymorphic node
    stream.EndPolymorphic();

    //Return the node
    return node;
  }
  return nullptr;
}

}//namespace Zero
