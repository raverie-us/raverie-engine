///////////////////////////////////////////////////////////////////////////////
///
/// \file DynamicAabbTreeBroadPhase.hpp
/// Declaration of the DynamicAabbTreeBroadPhase class.
/// 
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum3(BaseDAabbTreeSelfQuery, SingleObject, FullTree, PartialTree);

///The base code for any DynamicAabbTreeBroadPhase class.
///Used to reduce code duplication since the core functionality
///only differs by the internal tree type.
template <typename TreeType>
class BaseDynamicAabbTreeBroadPhase : public IBroadPhase
{
public:
  typedef BaseDynamicAabbTreeBroadPhase<TreeType> SelfType;
  typedef SelfType self_type;
  typedef typename TreeType::NodeType NodeType;

  BaseDynamicAabbTreeBroadPhase();
  ~BaseDynamicAabbTreeBroadPhase();

  virtual void Serialize(Serializer& stream);

  virtual void Draw(int level, uint debugDrawFlags);

  virtual void CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data);
  virtual void CreateProxies(BroadPhaseObjectArray& objects);
  virtual void RemoveProxy(BroadPhaseProxy& proxy);
  virtual void RemoveProxies(ProxyHandleArray& proxies);
  virtual void UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data);
  virtual void UpdateProxies(BroadPhaseObjectArray& objects);

  virtual void SelfQuery(ClientPairArray& results);
  virtual void Query(BroadPhaseData& data, ClientPairArray& results);
  virtual void BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results);

  virtual void Construct() {};

  virtual void CastRay(CastDataParam data, ProxyCastResults& results);
  virtual void CastSegment(CastDataParam data, ProxyCastResults& results);
  virtual void CastAabb(CastDataParam data, ProxyCastResults& results);
  virtual void CastSphere(CastDataParam data, ProxyCastResults& results);
  virtual void CastFrustum(CastDataParam data, ProxyCastResults& results);

  virtual void RegisterCollisions();

  virtual void Cleanup() {};

public:
  void QueryCallback(void* thisProxy, void* otherProxy);
protected:

  void SingleObjectQuery();
  void PartialTreeQuery();
  void FullTreeQuery();

  ///Converts the internal HashSet into the array.
  void FillOutResults(ClientPairArray& results);

  void AddQueryResult(Aabb& aabb);

  TreeType mTree;
  ///The proxy currently being queried. Used to avoid self pairs.
  NodeType* mQueryNode;

  // Temporarily Disabled: Leaks in the editor because nothing cleans up mNodesToQuery
  ///The proxies that have been inserted/updated since the last query.
  ///We only need to return pairs where at least 1 object is moving.
  //Array<NodeType*> mNodesToQuery;

  typedef HashSet<NodePointerPair> PairSet;
  PairSet mPairs;

  BaseDAabbTreeSelfQuery::Enum mSelfQueryPolicy;

  //remove later or something...
  uint mSingleObjectCountQuery;
  uint mBuildTreeObjectCountQuery;
};

}//namespace Zero

#include "SpatialPartition/BaseDynamicAabbTreeBroadPhase.inl"
