///////////////////////////////////////////////////////////////////////////////
///
/// \file StaticAabbTree.hpp
/// Declaration of the StaticAabbTree class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///A range for iterating through the leaf nodes of the StaticAabbTree. Used to
///perform queries without having to provide a callback function.
///Note: this range will become completely invalidated if any operations are
///performed on the StaticAabbTree.
template <typename ClientDataType, typename QueryType, typename ArrayType = Array<AabbNode<ClientDataType>*>, typename PolicyType = BroadPhasePolicy<QueryType, Aabb> >
struct StaticTreeRange : public BroadPhaseTreeRange<ClientDataType, AabbNode<ClientDataType>, QueryType, PolicyType, ArrayType>
{
  typedef BroadPhaseTreeRange<ClientDataType, AabbNode<ClientDataType>, QueryType, PolicyType, ArrayType> BaseType;

  ///Constructs a range using the default BroadPhase.
  StaticTreeRange(ArrayType* scratchBuffer, typename BaseType::NodeTypeDef* root, const QueryType& queryObj)
    : BaseType(scratchBuffer,root,queryObj,PolicyType())
  {
  }

  ///Constructs a range using the policy type passed in.
  StaticTreeRange(ArrayType* scratchBuffer, typename BaseType::NodeTypeDef* root, const QueryType& queryObj, PolicyType policy)
    : BaseType(scratchBuffer,root,queryObj,policy)
  {
  }
};

///An AabbTree specialized for static objects. This tree is preferable in the
///case where objects are not moving over the DynamicAabbTree because more time
///is spent in building the tree. This allows the tree to build itself more
///optimally. Adds, updates and removes will not take effect until construct is called.
template <typename ClientDataType>
class StaticAabbTree
{
public:
  typedef StaticAabbTree<ClientDataType> TreeType;
  typedef AabbNode<ClientDataType> NodeType;
  typedef NodeType* NodePointer;
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef Array<NodePointer> NodeArray;
  typedef HashSet<NodePointer> NodeSet;

  typedef Pair<NodePointer, DataType> UpdatePair;
  typedef Array<UpdatePair> UpdateArray;

  StaticAabbTree();
  ~StaticAabbTree();

  enum ConstructionMethod{TopDown, BottomUp, Insertion};
  enum StopCriteria{XPrimitives, CutOffLimit, DepthLimit};

  void Serialize(Serializer& stream);

  void CreateProxy(BroadPhaseProxy& proxy, DataType& data);
  void RemoveProxy(BroadPhaseProxy& proxy);
  void UpdateProxy(BroadPhaseProxy& proxy, DataType& data);

  ///Returns the client data of a proxy.
  ClientDataType& GetClientData(BroadPhaseProxy& proxy);
  ///Used to compute the number of proxies.
  ///Used after serialization since we don't know the count.
  void CountProxies();
  uint GetTotalProxyCount() const;

  ///Tells the structure that it has all of the data it will ever have. Used 
  ///mainly for static BroadPhases.
  void Construct();

  ///"Destructs" the entire tree. Still holds on to the inserted objects.
  void Destruct();

  void DeleteTree();

  ///Draw the tree at a given level.
  void Draw(int level, uint debugFlags);

  ///Returns a range to iterate through the leaf nodes that collide
  ///with the object of QueryType. A policy must be provided that tells
  ///the range how to collide an Aabb with a QueryType through a
  ///function called Overlap. Most implementations should just call Query
  ///which uses the policy BroadPhasePolicy<QueryType,Aabb>. A scratch buffer
  ///array must also be provided for handling allocations. In general, one
  ///should use the forRangeBroadphaseTreePolicy macro instead of calling this directly.
  template <typename QueryType, typename ArrayType, typename Policy>
  StaticTreeRange<ClientDataType, QueryType, ArrayType, Policy> 
    QueryWithPolicy(const QueryType& queryObj, ArrayType& scratchBuffer, Policy policy)
  {
    typedef StaticTreeRange<ClientDataType,QueryType,ArrayType,Policy> RangeType;

    return RangeType(&scratchBuffer,mRoot,queryObj,policy);
  }

  ///The same functionality as the QueryWithPolicy function except
  ///the Policy is implied from the QueryType. The policy will be
  ///defaulted to BroadPhasePolicy<QueryType,Aabb>. For general cases,
  ///use the forRangeBroadphaseTree macro instead of calling this directly.
  template <typename QueryType, typename ArrayType>
  StaticTreeRange<ClientDataType, QueryType, ArrayType> 
    Query(const QueryType& queryObj, ArrayType& scratchBuffer)
  {
    typedef StaticTreeRange<ClientDataType,QueryType,ArrayType> RangeType;

    return RangeType(&scratchBuffer,mRoot,queryObj);
  }

  ///Sets the current partition method.
  void SetPartitionMethod(PartitionMethods::Enum method);
private:
  template <typename ClientDataTypeOther>
  friend void SerializeAabbTree(Serializer& stream, StaticAabbTree<ClientDataTypeOther>& tree);

  typedef uint (*PartitionNodeMethodPtr)(NodeArray&);
  PartitionNodeMethodPtr CurrPartitionMethod;

  ///Draw the tree at a given level.
  void DrawLevel(NodePointer node, uint currLevel, uint level);
  void DrawTree(NodePointer node);

  ///Deletes all of the internal nodes while collecting the leaf nodes
  ///not in the removal set into the passed in array.
  void DeleteInternalNodes(NodeArray& leafNodes);

  ConstructionMethod mConstructMethod;
  StopCriteria mStopCriteria;
  uint mXPrimitives;
  PartitionMethods::Enum mPartitionMethod;

  NodePointer mRoot;
  NodeArray mNodesAdded;
  UpdateArray mUpdateNodes;
  NodeSet mNodesRemoved;

  uint mProxyCount;
};

typedef StaticAabbTree<void*> StaticAabbTreeDefault;

template <typename ClientDataType>
AabbNode<ClientDataType>* SerializeAabbTree(Serializer& stream);

template <typename ClientDataType>
void SerializeAabbTree(Serializer& stream, StaticAabbTree<ClientDataType>& tree);

}//namespace Zero

#include "SpatialPartition/StaticAabbTree.inl"
