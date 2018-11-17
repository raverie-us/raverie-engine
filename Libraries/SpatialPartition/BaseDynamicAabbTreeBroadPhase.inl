///////////////////////////////////////////////////////////////////////////////
///
/// \file DynamicAabbTreeBroadPhase.cpp
/// Implementation of the DynamicAabbTreeBroadPhase class.
/// 
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Zero
{

template <typename TreeType>
BaseDynamicAabbTreeBroadPhase<TreeType>::BaseDynamicAabbTreeBroadPhase()
{
  HeapAllocator allocator(mHeap);
  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery
  //mNodesToQuery.SetAllocator(allocator);
  mPairs.SetAllocator(allocator);

  mSelfQueryPolicy = BaseDAabbTreeSelfQuery::FullTree;
  mSingleObjectCountQuery = 20;
  mBuildTreeObjectCountQuery = static_cast<uint>(-1);
}

template <typename TreeType>
BaseDynamicAabbTreeBroadPhase<TreeType>::~BaseDynamicAabbTreeBroadPhase()
{

}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::Serialize(Serializer& stream)
{
  IBroadPhase::Serialize(stream);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::Draw(int level, uint debugDrawFlags)
{
  mTree.Draw(level);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CreateProxy(BroadPhaseProxy& proxy, 
                                                          BroadPhaseData& data)
{
  mTree.CreateProxy(proxy,data);
  NodeType* node = static_cast<NodeType*>(proxy.ToVoidPointer());

  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery
  //mNodesToQuery.PushBack(node);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CreateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    SelfType::CreateProxy(*obj.mProxy,obj.mData);
  }
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::RemoveProxy(BroadPhaseProxy& proxy)
{
  //remove from the tree
  mTree.RemoveProxy(proxy);

  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery

  //also have to see if the item was on our query array, if so
  //we have to remove it
  //typename Array<NodeType*>::range range = mNodesToQuery.All();
  //for(; !range.Empty(); range.PopFront())
  //{
  //  if(range.Front() == proxy.ToVoidPointer())
  //  {
  //    mNodesToQuery.Erase(&range.Front());
  //    return;
  //  }
  //}
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::RemoveProxies(ProxyHandleArray& proxies)
{
  ProxyHandleArray::range range = proxies.All();
  for(; !range.Empty(); range.PopFront())
    SelfType::RemoveProxy(*range.Front());
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::UpdateProxy(BroadPhaseProxy& proxy,
                                                          BroadPhaseData& data)
{
  mTree.UpdateProxy(proxy,data);
  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery
  //mNodesToQuery.PushBack(static_cast<NodeType*>(proxy.ToVoidPointer()));
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::UpdateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    SelfType::UpdateProxy(*obj.mProxy,obj.mData);
  }
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::SelfQuery(ClientPairArray& results)
{
  //should self query clear the pair results? means we can't do
  //multiple queries and then there is no point to having register collisions
  //(JoshD questions)
  FillOutResults(results);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::Query(BroadPhaseData& data, 
                                                    ClientPairArray& results)
{
  forRangeBroadphaseTree(typename TreeType,mTree,Aabb,data.mAabb)
    results.PushBack(ClientPair(data.mClientData,range.Front()));
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::BatchQuery(BroadPhaseDataArray& data, 
                                                         ClientPairArray& results)
{
  typename BroadPhaseDataArray::range dataRange = data.All();
  for(; !dataRange.Empty(); dataRange.PopFront())
    SelfType::Query(dataRange.Front(),results);
}


template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CastRay(CastDataParam data, 
                                                      ProxyCastResults& results)
{
  SimpleRayCallback callback(mCastRayCallBack,&results);

  forRangeBroadphaseTree(typename TreeType,mTree,Ray,data.GetRay())
    callback.Refine(range.Front(),data);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CastSegment(CastDataParam data, 
                                                          ProxyCastResults& results)
{
  SimpleSegmentCallback callback(mCastSegmentCallBack,&results);

  forRangeBroadphaseTree(typename TreeType,mTree,Segment,data.GetSegment())
    callback.Refine(range.Front(),data);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CastAabb(CastDataParam data, 
                                                       ProxyCastResults& results)
{
  SimpleAabbCallback callback(mCastAabbCallBack,&results);

  forRangeBroadphaseTree(typename TreeType,mTree,Aabb,data.GetAabb())
    callback.Refine(range.Front(),data);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CastSphere(CastDataParam data, 
                                                         ProxyCastResults& results)
{
  SimpleSphereCallback callback(mCastSphereCallBack,&results);

  forRangeBroadphaseTree(typename TreeType,mTree,Sphere,data.GetSphere())
    callback.Refine(range.Front(),data);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::CastFrustum(CastDataParam data,
                                                          ProxyCastResults& results)
{
  SimpleFrustumCallback callback(mCastFrustumCallBack,&results);

  forRangeBroadphaseTree(typename TreeType,mTree,Frustum,data.GetFrustum())
    callback.Refine(range.Front(),data);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::RegisterCollisions()
{
  mPairs.Clear();

  FullTreeQuery();

  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery

  //if(mSelfQueryPolicy == BaseDAabbTreeSelfQuery::SingleObject)
  //  SingleObjectQuery();
  //else if(mSelfQueryPolicy == BaseDAabbTreeSelfQuery::FullTree)
  //  FullTreeQuery();
  //else if(mSelfQueryPolicy == BaseDAabbTreeSelfQuery::PartialTree)
  //{
  //  if(mNodesToQuery.Size() < mSingleObjectCountQuery)
  //    SingleObjectQuery();
  //  else if(mNodesToQuery.Size() < mBuildTreeObjectCountQuery)
  //    PartialTreeQuery();
  //  else
  //    FullTreeQuery();
  //}
  //
  //mNodesToQuery.Clear();

  mTree.Rebalance(4);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::QueryCallback(void* thisProxy, void* otherProxy)
{
  if(thisProxy == otherProxy)
    return;

  //we need to somehow prevent duplicates, so we are using a set. However,
  //we need a unique key for our hash. So use the proxies
  //(also the node pointers) in the pair. When we need the client data,
  //we can retrieve the pair and therefore client data.
  mPairs.Insert(NodePointerPair(thisProxy,otherProxy));
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::SingleObjectQuery()
{
  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery

  //typename Array<NodeType*>::range range = mNodesToQuery.All();
  //for(; !range.Empty(); range.PopFront())
  //{
  //  mQueryNode = range.Front();
  //  AddQueryResult(mQueryNode->mAabb);
  //}
  //mQueryNode = nullptr;
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::PartialTreeQuery()
{
  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery

  //Querying a tree against a tree is far quicker than
  //performing hundreds of individual queries. However, using ourself
  //will not take advantage of sleeping (only need to query moving objects).
  //Therefore, with small enough sets, it should be quicker to build
  //a new temporary tree with only the nodes we need to query and
  //query that tree against ourselves.
 //BroadPhaseProxy proxy;
 //TreeType queryTree;
 //typename Array<NodeType*>::range range = mNodesToQuery.All();
 //for(; !range.Empty(); range.PopFront())
 //{
 //  mQueryNode = range.Front();
 //  BroadPhaseData data;
 //  data.mAabb = mQueryNode->mAabb;
 //  data.mClientData = mQueryNode->mClientData;
 //  queryTree.CreateProxy(proxy,data);
 //}
 //
 //mTree.QuerySelfTree(this);
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::FullTreeQuery()
{
  mTree.QuerySelfTree(this);

  //TreeType::SelfQueryRange range = mTree.QuerySelf();
  //for(; !range.Empty(); range.PopFront())
  //{
  //  TreeType::SelfQueryRange::NodePair& pair = range.proxyFront();

  //  //we need to somehow prevent duplicates, so we are using a set. However,
  //  //we need a unique key for our hash. So use the proxies
  //  //(also the node pointers) in the pair. When we need the client data,
  //  //we can retrieve the pair and therefore client data.

  //  mPairs.Insert(NodePointerPair(pair.first,pair.second));
  //}
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::FillOutResults(ClientPairArray& results)
{
  //we stored a set to avoid duplicate pairs, but now we need to return
  //an array, so loop through the set and push the results back
  //onto the array
  typename PairSet::range pairRange = mPairs.All();
  for(; !pairRange.Empty(); pairRange.PopFront())
  {
    NodeType* node1,* node2;
    NodePointerPair& pair = pairRange.Front();
    pair.Convert(node1,node2);
    results.PushBack(ClientPair(node1->mClientData,node2->mClientData));
  }
  mPairs.Clear();
}

template <typename TreeType>
void BaseDynamicAabbTreeBroadPhase<TreeType>::AddQueryResult(Aabb& aabb)
{
  forRangeBroadphaseTree(typename TreeType,mTree,Aabb,aabb)
  {
    void* proxy1 = &range.proxyFront();
    void* proxy2 = mQueryNode;
    if(proxy1 == proxy2)
      continue;

    //we need to somehow prevent duplicates, so we are using a set. However,
    //we need a unique key for our hash. So use the proxies
    //(also the node pointers) in the pair. When we need the client data,
    //we can retrieve the pair and therefore client data.

    mPairs.Insert(NodePointerPair(proxy1,proxy2));
  }
}

}//namespace Zero
