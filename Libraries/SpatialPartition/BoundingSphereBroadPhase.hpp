// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class BoundingSphereBroadPhase : public IBroadPhase
{
public:
  ZilchDeclareType(BoundingSphereBroadPhase, TypeCopyMode::ReferenceType);

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
  typedef BroadPhasePolicy<Sphere, Sphere> DefaultPolicy;
  typedef BoundingSphereRange<void*, Sphere, DefaultPolicy> DefaultRange;

  void GetCollisions(BroadPhaseData& data, ClientPairArray& results);

  typedef BoundingSphere<void*> BroadPhaseType;
  BroadPhaseType mBoundingSphere;

  ClientPairArray mDataPairs;
};

} // namespace Zero
