///////////////////////////////////////////////////////////////////////////////
///
/// \file BaseDynamicAabbTree.hpp
/// Declaration of the BaseDynamicTreePolicy and BaseDynamicAabbTree classes.
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

///Base policy for DynamicAabbTrees.
///Contains core functionality that all other AabbTreePolicies need to have.
template <typename NodeType>
struct BaseDynamicTreePolicy
{
  typedef NodeType NodeTypeDef;

  ///Given a parent node and a leaf node, determines which
  ///child the new leaf should be placed with.
  static NodeType* SelectNode(NodeType* parent, NodeType* newLeaf);
  ///Creates a new internal node from the old parent and two new children.
  ///links all pointers and expands the aabb to deal with the children
  static NodeType* CreateInternalNode(NodeType* oldParent,
                                      NodeType* oldChild, NodeType* newChild);
  ///Deletes just one node in the tree. Does nothing but unlinks the children
  ///from the node. The children must still have their parent pointers fixed.
  static void DeleteNode(NodeType* node);
};

///A Hierarchical AabbTree that is meant for dynamic objects. Used to have a
///fast BroadPhases for dynamic objects where raycasting is an important issue.
///Raycasting performs much faster than most other dynamic BroadPhases, and the
///normal update is decently fast.
///This is the base functionality for all DynamicAabbTrees and Contains a lot
///of shared functionality. The policy is used to determine how to Insert
///and remove nodes since different types might deal with these differently.
///(ie. Avl balancing)
template <typename PolicyType>
class BaseDynamicAabbTree
{
public:
  typedef PolicyType PolicyTypeDef;
  typedef typename PolicyType::ClientDataTypeDef ClientDataType;
  typedef BaseDynamicAabbTree<PolicyType> BaseTreeType;
  typedef BaseBroadPhaseData<ClientDataType> DataType;

  typedef typename PolicyType::NodeType NodeType;
  typedef Pair<NodeType*,NodeType*> NodePair;
  typedef Array<NodeType*> NodeArray;
  typedef Array<NodePair> NodePairArray;

  ///A range for iterating through the leaf nodes of the tree. Used to
  ///perform queries without having to provide a callback function.
  ///Note: this range will become completely invalidated if any operations are
  ///performed on the tree.
  template <typename QueryType, typename ArrayType = Array<NodeType*>, typename QueryPolicyType = BroadPhasePolicy<QueryType, Aabb> >
  struct BaseTreeRange
    : public BroadPhaseTreeRange<ClientDataType, NodeType, QueryType, QueryPolicyType, ArrayType >
  {
    typedef BroadPhaseTreeRange<ClientDataType, NodeType, QueryType, QueryPolicyType, ArrayType> BaseType;

    ///Constructs a range using the default Policy.
    BaseTreeRange(ArrayType* scratchBuffer, NodeType* root,
                  const QueryType& queryObj)
      : BaseType(scratchBuffer,root,queryObj,QueryPolicyType())
    {
    }

    ///Constructs a range using the policy type passed in.
    BaseTreeRange(ArrayType* scratchBuffer, NodeType* root,
                  const QueryType& queryObj, QueryPolicyType policy)
      : BaseType(scratchBuffer,root,queryObj,policy)
    {
    }
  };

  ///A range for iterating through the self pairs of this tree.
  ///Used to determine all potential overlaps within the tree itself.
  ///Note: this range will become completely invalidated if any operations are
  ///performed on the tree.
  struct SelfQueryRange
    : public BroadPhaseTreeSelfRange<BaseTreeType, BoxQueryPairCheck<ClientDataType> >
  {
    typedef BroadPhaseTreeSelfRange<BaseTreeType,
      BoxQueryPairCheck<ClientDataType> > BaseType;

    SelfQueryRange(NodePairArray* scratchBuffer, NodeType* root)
      : BaseType(scratchBuffer,root)
    {
    }
  };


  BaseDynamicAabbTree();
  ~BaseDynamicAabbTree();

  void CreateProxy(BroadPhaseProxy& proxy, DataType& data);
  void RemoveProxy(BroadPhaseProxy& proxy);
  void UpdateProxy(BroadPhaseProxy& proxy, DataType& data);

  ///Returns the client data of a proxy.
  ClientDataType& GetClientData(BroadPhaseProxy& proxy);
  ///Returns the Node Aabb of the given proxy.
  Aabb GetFatAabb(BroadPhaseProxy& proxy);
  uint GetTotalProxyCount() const;

  void DrawEntireTree();
  void Draw(int level);
  void DrawTree(NodeType* node);
  void DrawLevel(NodeType* node, uint currLevel, uint level);
  ///Deletes the entire tree in one shot.
  void Clear();

  void Validate();

  ///Returns false if tree is empty and does not modify passed in aabb
  bool GetRootAabb(Aabb* aabb);

  ///Returns a range to iterate through the leaf nodes that collide
  ///with the object of QueryType. A policy must be provided that tells
  ///the range how to collide an Aabb with a QueryType through a
  ///function called Overlap. Most implementations should just call Query
  ///which uses the policy BroadPhasePolicy<QueryType,Aabb>. A scratch buffer
  ///array must also be provided for handling allocations. In general, one
  ///should use the forRangeBroadphaseTreePolicy macro instead of calling this directly.
  template <typename QueryType, typename ArrayType, typename Policy>
  BaseTreeRange<QueryType,ArrayType,Policy>
    QueryWithPolicy(const QueryType& queryObj, ArrayType& scratchBuffer, Policy policy)
  {
    typedef BaseTreeRange<QueryType,ArrayType> RangeType;

    return RangeType(&scratchBuffer,mRoot,queryObj,policy);
  }

  ///The same functionality as the QueryWithPolicy function except
  ///the Policy is implied from the QueryType. The policy will be
  ///defaulted to BroadPhasePolicy<QueryType,Aabb>. For general cases,
  ///use the forRangeBroadphaseTree macro instead of calling this directly.
  template <typename QueryType, typename ArrayType>
  BaseTreeRange<QueryType,ArrayType>
    Query(const QueryType& queryObj, ArrayType& scratchBuffer)
  {
    typedef BaseTreeRange<QueryType,ArrayType> RangeType;

    return RangeType(&scratchBuffer,mRoot,queryObj);
  }

  ///Callback is expected to have a method called
  /// QueryCallback(NodeType* node1, NodeType* node2) (world space trees)
  template <typename CallbackType>
  void QuerySelfTree(CallbackType* callback);

  ///Callback is expected to have a method called
  /// QueryCallback(NodeType* thisNode, NodeType* otherNode) (world space trees)
  template <typename CallbackType>
  void QueryTree(CallbackType* callback, const BaseTreeType* tree);

  ///Returns a range to iterate through all of the internal leaf
  ///node pairs that overlap with each other.
  SelfQueryRange QuerySelf(NodePairArray& scratchBuffer);

protected:

  ///Updates the given leaf with the passed in aabb.
  void Update(NodeType* leafNode, Aabb& aabb);

  NodeType* mRoot;
  uint mProxyCount;
};

}//namespace Zero

#include "BaseDynamicAabbTree.inl"
