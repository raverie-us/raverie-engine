///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseRanges.hpp
/// Declaration of the BroadPhaseArrayRange and the
/// BroadPhaseTreeRange classes.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Default behavior for a range. Returns true to any query attempt.
template <typename ClientDataType, typename QueryType, typename Policy>
struct BasicQueryCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<DataType> DataTypeArray;

  bool CheckPolicy(DataType& data)
  {
    return true;
  }
};

///Behavior for a range that is querying against spheres.
///Uses the policy to check for overlap against the sphere.
template <typename ClientDataType, typename QueryType, typename Policy>
struct SphereQueryCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<DataType> DataTypeArray;

  SphereQueryCheck(const QueryType& queryObj, Policy policy)
    : mQueryObj(queryObj), mPolicy(policy)
  {
  }

  bool CheckPolicy(DataType& data)
  {
    return mPolicy.Overlap(mQueryObj,data.mBoundingSphere);
  }

  QueryType mQueryObj;
  Policy mPolicy;
};

///Behavior for a range that is querying against Aabbs.
///Uses the policy to check for overlap against the Aabb.
template <typename ClientDataType, typename QueryType, typename Policy>
struct BoxQueryCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<DataType> DataTypeArray;

  BoxQueryCheck(const QueryType& queryObj, Policy policy)
    : mQueryObj(queryObj), mPolicy(policy)
  {
  }

  bool CheckPolicy(DataType& data)
  {
    return mPolicy.Overlap(mQueryObj,data.mAabb);
  }

  QueryType mQueryObj;
  Policy mPolicy;
};

///Default behavior for a pair range. Returns true to any query attempt.
template <typename ClientDataType>
struct BasicQueryPairCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<DataType> DataTypeArray;

  bool CheckPolicy(DataType& data1, DataType& data2)
  {
    return true;
  }
};

///Behavior for a pair range of spheres. Checks the spheres for overlap.
template <typename ClientDataType>
struct SphereQueryPairCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<DataType> DataTypeArray;

  bool CheckPolicy(DataType& data1, DataType& data2)
  {
    return data1.mBoundingSphere.Overlap(data2.mBoundingSphere);
  }
};

///Behavior for a pair range of Aabbs. Checks the Aabbs for overlap.
template <typename ClientDataType>
struct BoxQueryPairCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<DataType> DataTypeArray;

  bool CheckPolicy(DataType& data1, DataType& data2)
  {
    return data1.mAabb.Overlap(data2.mAabb);
  }
};

///A range to iterate through the valid items in a BroadPhase where the
///underlying structure is an array. Used to perform casts and queries
///against the BroadPhase. Any specific implementation should provide a
///CheckPolicy function in place of the basic query check.
template <typename ClientDataType, typename QueryType = void*, typename Policy = void*, 
          typename QueryCheck = BasicQueryCheck<ClientDataType, QueryType, Policy> >
struct BroadPhaseArrayRange : public QueryCheck
{
  typedef ClientDataType ClientDataTypeDef;
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef BaseDataArrayObject<ClientDataType> ArrayObjectType;
  typedef Array<ArrayObjectType> ObjectArray;

  BroadPhaseArrayRange(ObjectArray* data)
    : mData(data)
  {
    mIndex = 0;
    mIndex = SkipDead();
  }

  BroadPhaseArrayRange(ObjectArray* data, const QueryType& queryObj, Policy policy)
    : QueryCheck(queryObj, policy), mData(data)
  {
    mIndex = 0;
    mIndex = SkipDead();
  }

  void PopFront()
  {
    ++mIndex;
    mIndex = SkipDead();
  }

  ClientDataType& Front()
  {
    return (*mData)[mIndex].mData.mClientData;
  }

  bool Empty() const
  {
    return mData->Size() == mIndex;
  }

  //temporary now so that the proxy can be retrieved
  uint& proxyFront()
  {
    return mIndex;
  }

  uint SkipDead()
  {
    for(uint i = mIndex; i < mData->Size(); ++i)
    {
      ArrayObjectType& obj = (*mData)[i];

      if(obj.mValid == false)
        continue;

      BroadPhaseData& data = obj.mData;

      if(!QueryCheck::CheckPolicy(data))
       continue;

      return i;
    }
    return mData->Size();
  }

  uint mIndex;
  ObjectArray* mData;
};

///A range to iterate through the valid items in a BroadPhase where the
///underlying structure is a tree. Used to perform casts and queries
///against the BroadPhase.
template <typename ClientDataType, typename NodeType, typename QueryType, typename PolicyType, typename ArrayType = Array<NodeType*> >
struct BroadPhaseTreeRange
{
  typedef ClientDataType ClientDataTypeDef;
  typedef NodeType NodeTypeDef;
  typedef ArrayType NodeTypePointerArray;

  BroadPhaseTreeRange(NodeTypePointerArray* scratchBuffer, NodeType* root, 
                      const QueryType& queryObj, PolicyType policy)
    : mQueryObj(queryObj), mPolicy(policy)
  {
    mScratchSpace = scratchBuffer;
    mScratchSpace->Clear();
    if(root != nullptr)
      mScratchSpace->PushBack(root);
    SkipDead();
  }

  virtual ~BroadPhaseTreeRange()
  {
    //mScratchSpace->Clear();
  }

  void PopFront()
  {
    mScratchSpace->PopBack();
    SkipDead();
  }

  ClientDataType& Front()
  {
    uint size = mScratchSpace->Size();
    ErrorIf(size == 0,"Cannot pop an empty range.");
    return (*mScratchSpace)[size - 1]->mClientData;
  }

  bool Empty() const
  {
    return mScratchSpace->Size() == 0;
  }

  //temporary now so that the proxy can be retrieved
  NodeType& proxyFront()
  {
    uint size = mScratchSpace->Size();
    ErrorIf(size == 0,"Cannot pop an empty range.");
    return *(*mScratchSpace)[size - 1];
  }

  void SkipDead()
  {
    NodeTypePointerArray& nodes = *mScratchSpace;
    while(!nodes.Empty())
    {
      NodeType* node = nodes.Back();

      //if this node doesn't overlap the aabb, we don't care
      if(!mPolicy.Overlap(mQueryObj,node->mAabb))
      {
        nodes.PopBack();
        continue;
      }

      //if it is a leaf then return it
      if(node->IsLeaf())
        return;

      //otherwise push both children onto the stack (if both are nodes)
      ErrorIf(node->mChild1 == nullptr,"Child 1 of an internal node should never be null.");
      ErrorIf(node->mChild2 == nullptr,"Child 2 of an internal node should never be null.");

      nodes.PopBack();
      nodes.PushBack(node->mChild1);
      nodes.PushBack(node->mChild2);
    }
  }

  QueryType mQueryObj;
  PolicyType mPolicy;
  NodeTypePointerArray* mScratchSpace;
};

///A range to iterate through the valid items in a BroadPhase where the
///underlying structure is an array. Used to perform casts and queries
///against the BroadPhase. Any specific implementation should provide a
///CheckPolicy function in place of the basic query check.
template <typename ClientDataType, typename QueryCheck = BasicQueryPairCheck<ClientDataType> >
struct BroadPhaseArrayPairRange : public QueryCheck
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef BaseDataArrayObject<ClientDataType> ArrayObjectType;
  typedef Array<ArrayObjectType> ObjectArray;
  typedef Pair<ClientDataType,ClientDataType> PairType;

  BroadPhaseArrayPairRange(ObjectArray* data)
    : mData(data)
  {
    mIndex1 = 0;
    mIndex2 = 1;
    SkipDead(mIndex1,mIndex2);
  }

  void PopFront()
  {
    ++mIndex2;
    if(mIndex2 == mData->Size())
    {
      ++mIndex1;
      mIndex2 = mIndex1 + 1;
    }
    SkipDead(mIndex1,mIndex2);
  }

  PairType& Front()
  {
    return mPair;
  }

  bool Empty() const
  {
    return mData->Size() == mIndex1;
  }

  void SkipDead(uint& index1, uint& index2)
  {
    ObjectArray& data = *mData;
    //Add a pair between each object.
    //This has to check for the mValid for false due to how we
    //hand out proxies. We can't shrink the array otherwise
    //the old proxies will be invalid.
    uint size = data.Size();
    for(uint i = index1; i < size; ++i)
    {
      ArrayObjectType& firstObj = data[i];
      if(firstObj.mValid == false)
      {
        ++index1;
        index2 = index1 + 1;
        continue;
      }

      for(uint j = index2; j < size; ++j)
      {
        ArrayObjectType& secondObj = data[j];
        if(secondObj.mValid == false)
        {
          ++index2;
          continue;
        }
        
        DataType& first = firstObj.mData;
        DataType& second = secondObj.mData;
        if(!QueryCheck::CheckPolicy(first,second))
        {
          ++index2;
          continue;
        }

        mPair = PairType(first.mClientData,second.mClientData);
        return;
      }
      ++index1;
      index2 = index1 + 1;
    }
  }

  uint mIndex1,mIndex2;
  ObjectArray* mData;
  PairType mPair;
};

template <typename TreeType, typename QueryCheck = BasicQueryPairCheck<typename TreeType::ClientDataTypeDef> >
struct BroadPhaseTreeSelfRange : public QueryCheck
{
  typedef TreeType TreeTypeDef;
  typedef typename TreeType::NodeType NodeType;
  typedef typename TreeType::ClientDataTypeDef ClientDataType;
  typedef Pair<NodeType*,NodeType*> NodePair;
  typedef Array<NodePair> NodePairArray;

  typedef Pair<ClientDataType,ClientDataType> PairType;

  BroadPhaseTreeSelfRange(NodePairArray* scratchBuffer, NodeType* root)
  {
    mScratchSpace = scratchBuffer;
    mScratchSpace->Clear();
    
    InitialSetup(root);
    SkipDead();
  }

  virtual ~BroadPhaseTreeSelfRange()
  {
    
  }

  void PopFront()
  {
    mScratchSpace->PopBack();
    SkipDead();
  }

  PairType& Front()
  {
    return mPair;
  }

  NodePair& proxyFront()
  {
    return mNodePair;
  }

  bool Empty() const
  {
    return mScratchSpace->Size() == 0;
  }

  void InitialSetup(NodeType* root)
  {
    if(root == nullptr || root->IsLeaf())
      return;

    typedef Pair<NodeType*, NodeType*> NodePair;
    NodePairArray& nodePairs = *mScratchSpace;

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
  }

  void SkipDead()
  {
    NodePairArray& nodePairs = *mScratchSpace;
    while(!nodePairs.Empty())
    {
      mNodePair = nodePairs.Back();

      NodeType* nodeA = mNodePair.first;
      NodeType* nodeB = mNodePair.second;

      //if the nodes don't overlap, we don't care
      if(!nodeA->mAabb.Overlap(nodeB->mAabb))
      {
        nodePairs.PopBack();
        continue;
      }

      if(nodeA->IsLeaf())
      {
        if(nodeB->IsLeaf())
        {
          mPair = MakePair(nodeA->mClientData,nodeB->mClientData);
          return;
        }

        nodePairs.PopBack();

        nodePairs.PushBack(MakePair(nodeA,nodeB->mChild1));
        nodePairs.PushBack(MakePair(nodeA,nodeB->mChild2));
      }
      else if(nodeB->IsLeaf())
      {
        nodePairs.PopBack();

        nodePairs.PushBack(MakePair(nodeA->mChild1,nodeB));
        nodePairs.PushBack(MakePair(nodeA->mChild2,nodeB));
      }
      else
      {
        nodePairs.PopBack();

        nodePairs.PushBack(MakePair(nodeA->mChild1,nodeB->mChild1));
        nodePairs.PushBack(MakePair(nodeA->mChild1,nodeB->mChild2));
        nodePairs.PushBack(MakePair(nodeA->mChild2,nodeB->mChild1));
        nodePairs.PushBack(MakePair(nodeA->mChild2,nodeB->mChild2));
      }
    }
  }

  NodePair mNodePair;
  PairType mPair;
  NodePairArray* mScratchSpace;
};

//Range to iterate over a query to a tree-based broadphase. Trees need a
//scratch space buffer to allocate a stack during traversal, but allocating
//that buffer is inefficient. Instead, we can create a scratch buffer on
//the stack. However, this produces some annoying and nasty code to copy
//everywhere we do a cast. This range attempts to simplify things from the
//users perspective. Unfortunately, this range cannot be used multiple
//times in the same scope.
#define forRangeBroadphaseTree(treeType, tree, queryType, queryObj)               \
  Array<treeType::NodeType*,LocalStackAllocator> nodeArray_;                      \
  uint totalProxyCount_ = tree.GetTotalProxyCount();                              \
  LocalStackAllocator stackAllocator_(alloca(totalProxyCount_ * sizeof(void*)));  \
  nodeArray_.SetAllocator(stackAllocator_);                                      \
  nodeArray_.Reserve(totalProxyCount_);                                           \
  typedef TypeOf(tree.Query(queryObj,nodeArray_)) _RangeType;                     \
  _RangeType range = tree.Query(queryObj,nodeArray_);                             \
  for(; !range.Empty(); range.PopFront())

//Same as above, but allows the user to provide a policy object to customize
//how we check a node against the query object type.
#define forRangeBroadphaseTreePolicy(treeType, tree, queryType, queryObj, policy)\
  Array<treeType::NodeType*,LocalStackAllocator> nodeArray_;                     \
  uint totalProxyCount_ = tree.GetTotalProxyCount();                             \
  LocalStackAllocator stackAllocator_(alloca(totalProxyCount_ * sizeof(void*))); \
  nodeArray_.SetAllocator(stackAllocator_);                                     \
  nodeArray_.Reserve(totalProxyCount_);                                          \
  typedef TypeOf(tree.QueryWithPolicy(queryObj,nodeArray_,policy)) _RangeType;   \
  _RangeType range = tree.QueryWithPolicy(queryObj,nodeArray_,policy);           \
  for(; !range.Empty(); range.PopFront())

}//namespace Zero
