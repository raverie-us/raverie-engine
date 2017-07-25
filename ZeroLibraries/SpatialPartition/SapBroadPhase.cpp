///////////////////////////////////////////////////////////////////////////////
///
/// \file SapBroadPhase.cpp
/// Implementation of the SapBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(SapBroadPhase, builder, type)
{
}

void SapBroadPhase::Serialize(Serializer& stream)
{
  IBroadPhase::Serialize(stream);
}

void SapBroadPhase::CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mSap.CreateProxy(proxy,data);
}

void SapBroadPhase::CreateProxies(BroadPhaseObjectArray& objects)
{
  mSap.CreateProxies(objects);
}

void SapBroadPhase::RemoveProxy(BroadPhaseProxy& proxy)
{
  mSap.RemoveProxy(proxy);
}

void SapBroadPhase::RemoveProxies(ProxyHandleArray& proxies)
{
  mSap.RemoveProxies(proxies);
}

void SapBroadPhase::UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mSap.UpdateProxy(proxy,data);
}

void SapBroadPhase::UpdateProxies(BroadPhaseObjectArray& objects)
{
  mSap.UpdateProxies(objects);
}

void SapBroadPhase::SelfQuery(ClientPairArray& results)
{
  SapPairRange<void*> range = mSap.QuerySelf();
  results.Insert(results.End(),range);
}

void SapBroadPhase::Query(BroadPhaseData& data, ClientPairArray& results)
{
  DefaultRange r = mSap.Query(data.mAabb);
  for(; !r.Empty(); r.PopFront())
  {
    ClientPair pair;
    pair.mClientData[0] = r.Front();
    pair.mClientData[1] = data.mClientData;
    results.PushBack(pair);
  }
}

void SapBroadPhase::BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results)
{
  for(uint i = 0; i < data.Size(); ++i)
    Query(data[i],results);
}

void SapBroadPhase::CastRay(CastDataParam castData, ProxyCastResults& results)
{
  SimpleRayCallback callback(mCastRayCallBack,&results);
  //Sap is currently not performing real raycasting, but rather taking the aabb
  //of the ray and finding what is in that range then returning those objects.
  //Unfortunately, a ray has a length of 1, so I am cheating here by converting
  //it to a segment with an arbitrary length of 100.
  Segment segment = ToSegment(castData.GetRay(),100);
  SapRange<void*,Segment> range = mSap.Query(segment);
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void SapBroadPhase::CastSegment(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSegmentCallback callback(mCastSegmentCallBack,&results);
  SapRange<void*,Segment> range = mSap.Query(castData.GetSegment());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void SapBroadPhase::CastAabb(CastDataParam castData, ProxyCastResults& results)
{
  SimpleAabbCallback callback(mCastAabbCallBack,&results);
  SapRange<void*,Aabb> range = mSap.Query(castData.GetAabb());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void SapBroadPhase::CastSphere(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSphereCallback callback(mCastSphereCallBack,&results);
  SapRange<void*,Sphere> range = mSap.Query(castData.GetSphere());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void SapBroadPhase::CastFrustum(CastDataParam castData, ProxyCastResults& results)
{
  SimpleFrustumCallback callback(mCastFrustumCallBack,&results);
  SapRange<void*,Frustum> range = mSap.Query(castData.GetFrustum());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void SapBroadPhase::RegisterCollisions()
{
  
}

}//namespace Zero
