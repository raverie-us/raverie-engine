// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class BoundingBoxBroadPhase : public IBroadPhase
{
public:
  ZilchDeclareType(BoundingBoxBroadPhase, TypeCopyMode::ReferenceType);

  virtual void Serialize(Serializer& stream);
  virtual void Draw(int level, uint debugDrawFlags)
  {
  }

  virtual void CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data);
  virtual void CreateProxies(BroadPhaseObjectArray& objects);
  virtual void RemoveProxy(BroadPhaseProxy& proxy);
  virtual void RemoveProxies(ProxyHandleArray& proxies);
  virtual void UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data);
  virtual void UpdateProxies(BroadPhaseObjectArray& objects);

  virtual void SelfQuery(ClientPairArray& results);
  virtual void Query(BroadPhaseData& data, ClientPairArray& results);
  virtual void BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results);

  virtual void Construct(){};

  virtual void CastRay(CastDataParam data, ProxyCastResults& results);
  virtual void CastSegment(CastDataParam data, ProxyCastResults& results);
  virtual void CastAabb(CastDataParam data, ProxyCastResults& results);
  virtual void CastSphere(CastDataParam data, ProxyCastResults& results);
  virtual void CastFrustum(CastDataParam data, ProxyCastResults& results);

  virtual void RegisterCollisions();

  virtual void Cleanup(){};

private:
  typedef BroadPhasePolicy<Aabb, Aabb> DefaultPolicy;
  typedef BoundingBoxRange<void*, Aabb, DefaultPolicy> DefaultRange;

  void GetCollisions(BroadPhaseData& data, ClientPairArray& results);

  typedef BoundingBox<void*> BroadPhaseType;
  BroadPhaseType mBoundingBox;

  ClientPairArray mDataPairs;
};

} // namespace Zero
