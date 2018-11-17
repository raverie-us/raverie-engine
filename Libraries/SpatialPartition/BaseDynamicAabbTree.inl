///////////////////////////////////////////////////////////////////////////////
///
/// \file BaseDynamicAabbTree.inl
/// Implementation of the BaseDynamicTreePolicy and BaseDynamicAabbTree classes.
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

namespace BaseDynamicTreeInternal
{

static const Vec3 cAabbFatFactor = Vec3(.1f,.1f,.1f);
static const real cAabbFatScaleFactor = real(1.2);

}//namespace BaseDynamicTreeInternal

//-------------------------------------------------------------------BaseDynamicTreePolicy

template <typename NodeType>
typename BaseDynamicTreePolicy<NodeType>::NodeTypeDef*
  BaseDynamicTreePolicy<NodeType>::SelectNode(NodeType* parent, NodeType* newLeaf)
{
  //if there is no child 2 then we have to select child1
  if(parent->mChild2 == nullptr)
    return parent->mChild1;

  Vec3 child1Pos = parent->mChild1->mAabb.GetCenter();
  Vec3 child2Pos = parent->mChild2->mAabb.GetCenter();
  Vec3 leafPos = newLeaf->mAabb.GetCenter();

  real child1Dist = (child1Pos - leafPos).LengthSq();
  real child2Dist = (child2Pos - leafPos).LengthSq();
  if(child1Dist < child2Dist)
    return parent->mChild1;
  return parent->mChild2;
}

template <typename NodeType>
typename BaseDynamicTreePolicy<NodeType>::NodeTypeDef*
  BaseDynamicTreePolicy<NodeType>::CreateInternalNode(NodeType* oldParent,
                                                      NodeType* oldChild, NodeType* newChild)
{
  NodeType* internalNode = new NodeType();

  //link the internal node pointers
  internalNode->mChild1 = oldChild;
  internalNode->mChild2 = newChild;
  internalNode->mParent = oldParent;

  //link the children pointers
  oldChild->mParent = internalNode;
  newChild->mParent = internalNode;

  //replace the correct child pointer for our old parent
  if(oldParent != nullptr)
  {
    if(oldParent->mChild1 == oldChild)
      oldParent->mChild1 = internalNode;
    else
      oldParent->mChild2 = internalNode;
  }

  //compute the internal node's aabb
  internalNode->mAabb = oldChild->mAabb;
  internalNode->mAabb = internalNode->mAabb.Combined(newChild->mAabb);

  return internalNode;
}

template <typename NodeType>
void BaseDynamicTreePolicy<NodeType>::DeleteNode(NodeType* node)
{
  node->mChild1 = node->mChild2 = nullptr;
  delete node;
}

//-------------------------------------------------------------------BaseDynamicAabbTree

template <typename PolicyType>
BaseDynamicAabbTree<PolicyType>::BaseDynamicAabbTree()
{
  mRoot = nullptr;
  mProxyCount = 0;
}

template <typename PolicyType>
BaseDynamicAabbTree<PolicyType>::~BaseDynamicAabbTree()
{
  Clear();
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::CreateProxy(BroadPhaseProxy& proxy, DataType& data)
{
  Aabb aabb = data.mAabb;
  if(!aabb.Valid())
  {
    Error("Invalid Aabb inserted");

    // We got the assert (good) but we don't want to keep getting it every frame
    aabb.AttemptToCorrectInvalid();
  }

  NodeType* node = new NodeType();
  node->mClientData = data.mClientData;
  node->mAabb = aabb;
  Vec3 halfExtents = aabb.GetHalfExtents();
  halfExtents = Math::Min(halfExtents + BaseDynamicTreeInternal::cAabbFatFactor,
                          halfExtents * BaseDynamicTreeInternal::cAabbFatScaleFactor);
  node->mAabb.SetCenterAndHalfExtents(aabb.GetCenter(), halfExtents);

  PolicyType::InsertNode(mRoot,node,mRoot);
  proxy = BroadPhaseProxy(node);
  ++mProxyCount;
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::RemoveProxy(BroadPhaseProxy& proxy)
{
  NodeType* node = static_cast<NodeType*>(proxy.ToVoidPointer());
  PolicyType::RemoveNode(mRoot,node);
  delete node;
  --mProxyCount;
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::UpdateProxy(BroadPhaseProxy& proxy, DataType& data)
{
  Aabb aabb = data.mAabb;
  if(!aabb.Valid())
  {
    Error("Invalid Aabb inserted");

    // We got the assert (good) but we don't want to keep getting it every frame
    aabb.AttemptToCorrectInvalid();
  }

  NodeType* node = static_cast<NodeType*>(proxy.ToVoidPointer());
  //there could be an update where our client data changed
  //so make sure to update it (ie. a remove->Insert)
  node->mClientData = data.mClientData;
  Update(node,aabb);
}

template <typename PolicyType>
typename BaseDynamicAabbTree<PolicyType>::ClientDataType&
  BaseDynamicAabbTree<PolicyType>::GetClientData(BroadPhaseProxy& proxy)
{
  NodeType* node = static_cast<NodeType*>(proxy.ToVoidPointer());
  return node->mClientData;
}

template <typename PolicyType>
Aabb BaseDynamicAabbTree<PolicyType>::GetFatAabb(BroadPhaseProxy& proxy)
{
  NodeType* node = static_cast<NodeType*>(proxy.ToVoidPointer());
  return node->mAabb;
}

template <typename PolicyType>
uint BaseDynamicAabbTree<PolicyType>::GetTotalProxyCount() const
{
  return mProxyCount;
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::DrawEntireTree()
{
  DrawTree(mRoot);
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::Draw(int level)
{
  if(!mRoot)
    return;

  if(level == -1)
    DrawTree(mRoot);
  else
    DrawLevel(mRoot, 0, level);
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::DrawTree(NodeType* node)
{
  if(!node)
    return;

  gDebugDraw->Add(Debug::Obb(node->mAabb).Color(Color::MintCream));

  DrawTree(node->mChild1);
  DrawTree(node->mChild2);
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::DrawLevel(NodeType* node, uint currLevel, uint level)
{
  if(!node)
    return;

  if(currLevel == level)
  {
    gDebugDraw->Add(Debug::Obb(node->mAabb).Color(Color::MintCream));
    return;
  }

  DrawLevel(node->mChild1, currLevel + 1, level);
  DrawLevel(node->mChild2, currLevel + 1, level);
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::Clear()
{
  delete mRoot;
  mRoot = nullptr;
  mProxyCount = 0;
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::Validate()
{
  NodeArray nodes;
  nodes.Reserve(256);
  nodes.PushBack(mRoot);

  if(mRoot == nullptr)
    return;

  ErrorIf(mRoot->mParent != nullptr, "Root should have a null Parent.");

  while(!nodes.Empty())
  {
    NodeType* node = nodes.Back();
    nodes.PopBack();

    if(node->IsLeaf())
    {
      ErrorIf(node->mChild1 != nullptr,"Leaf should have a null Child 1 pointer.");
      ErrorIf(node->mChild2 != nullptr,"Leaf should have a null Child 2 pointer.");
    }
    else
    {
      ErrorIf(node->mChild1 == nullptr,"Child 1 of an internal node should never be null.");
      ErrorIf(node->mChild1 == nullptr,"Child 2 of an internal node should never be null.");

      Aabb& parent = node->mAabb;
      Aabb& child1 = node->mChild1->mAabb;
      Aabb& child2 = node->mChild2->mAabb;
      ErrorIf(!parent.ContainsPoint(child1.mMax) || !parent.ContainsPoint(child1.mMin),
        "Parent Aabb does not contain child 1.");
      ErrorIf(!parent.ContainsPoint(child2.mMax) || !parent.ContainsPoint(child2.mMin),
        "Parent Aabb does not contain child 2.");
      nodes.PushBack(node->mChild1);
      nodes.PushBack(node->mChild2);
    }
  }
}

template <typename PolicyType>
bool BaseDynamicAabbTree<PolicyType>::GetRootAabb(Aabb* aabb)
{
  if (!mRoot)
    return false;

  *aabb = mRoot->mAabb;
  return true;
}

template <typename PolicyType>
template <typename CallbackType>
void BaseDynamicAabbTree<PolicyType>::QuerySelfTree(CallbackType* callback)
{
  TreeSelfQuery(callback,mRoot);
}

template <typename PolicyType>
template <typename CallbackType>
void BaseDynamicAabbTree<PolicyType>::QueryTree(CallbackType* callback, const BaseTreeType* tree)
{
  QueryTreeVsTree(callback,mRoot,tree->mRoot);
}

template <typename PolicyType>
typename BaseDynamicAabbTree<PolicyType>::SelfQueryRange
  BaseDynamicAabbTree<PolicyType>::QuerySelf(NodePairArray& scratchBuffer)
{
  return SelfQueryRange(scratchBuffer,mRoot);
}

template <typename PolicyType>
void BaseDynamicAabbTree<PolicyType>::Update(NodeType* leafNode, Aabb& aabb)
{
  //our old Aabb contained our new one, so we don't have to do anything
  if(leafNode->mAabb.ContainsPoint(aabb.mMin) &&
    leafNode->mAabb.ContainsPoint(aabb.mMax))
    return;

  //remove the leaf node
  NodeType* node = PolicyType::RemoveNode(mRoot,leafNode);
  if(node == nullptr)
    node = mRoot;

  //set the new fattened aabb
  Vec3 center = aabb.GetCenter();
  Vec3 halfExtents = aabb.GetHalfExtents();
  halfExtents = Math::Min(halfExtents + BaseDynamicTreeInternal::cAabbFatFactor,
                          halfExtents * BaseDynamicTreeInternal::cAabbFatScaleFactor);
  leafNode->mAabb.SetCenterAndHalfExtents(center, halfExtents);

  //we could update at the last unaffected node, but there is no guarantee that
  //the new node is contained within that. We could iterate back up and find
  //the node to Insert from, but the speed of that is debatable. Instead,
  //just Insert from the root for now.
  PolicyType::InsertNode(mRoot,leafNode,mRoot);
}

}//namespace Zero
