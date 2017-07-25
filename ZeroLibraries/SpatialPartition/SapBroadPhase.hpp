///////////////////////////////////////////////////////////////////////////////
///
/// \file SapBroadPhase.hpp
/// Declaration of the SapBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class SapBroadPhase : public IBroadPhase
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef Sap<void*> BroadPhaseType;
  typedef SapRange<void*,Aabb> DefaultRange;

  virtual void Serialize(Serializer& stream);
  virtual void Draw(int level, uint debugDrawFlags){}

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

private:

  BroadPhaseType mSap;
  Sap<int> temp;

  ClientPairArray mDataPairs;
};

}//namespace Zero
