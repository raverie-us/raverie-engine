///////////////////////////////////////////////////////////////////////////////
///
/// \file NSquaredBroadPhase.cpp
/// Implementation of the NSquaredBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(NSquaredBroadPhase, builder, type)
{
}

void NSquaredBroadPhase::Serialize(Serializer& stream)
{
  IBroadPhase::Serialize(stream);
  mNSquared.Serialize(stream);
}

void NSquaredBroadPhase::CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mNSquared.CreateProxy(proxy,data);
}

void NSquaredBroadPhase::CreateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mNSquared.CreateProxy(*obj.mProxy,obj.mData);
  }
}

void NSquaredBroadPhase::RemoveProxy(BroadPhaseProxy& proxy)
{
  mNSquared.RemoveProxy(proxy);
}

void NSquaredBroadPhase::RemoveProxies(ProxyHandleArray& proxies)
{
  ProxyHandleArray::range range = proxies.All();
  for(; !range.Empty(); range.PopFront())
    mNSquared.RemoveProxy(*range.Front());
}

void NSquaredBroadPhase::UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mNSquared.UpdateProxy(proxy,data);
}

void NSquaredBroadPhase::UpdateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mNSquared.UpdateProxy(*obj.mProxy,obj.mData);
  }
}

void NSquaredBroadPhase::SelfQuery(ClientPairArray& results)
{
  results.Insert(results.End(),mDataPairs.All());
}
  
void NSquaredBroadPhase::Query(BroadPhaseData& data, ClientPairArray& results)
{
  range r = mNSquared.Query();
  GetCollisions(data,results);
}

void NSquaredBroadPhase::BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results)
{
  for(uint i = 0; i < data.Size(); ++i)
    GetCollisions(data[i],results);
}

void NSquaredBroadPhase::CastRay(CastDataParam castData, ProxyCastResults& results)
{
  SimpleRayCallback callback(mCastRayCallBack,&results);
  range r = mNSquared.Query();
  for(; !r.Empty(); r.PopFront())
    callback.Refine(r.Front(),castData);
}

void NSquaredBroadPhase::CastSegment(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSegmentCallback callback(mCastSegmentCallBack,&results);
  range r = mNSquared.Query();
  for(; !r.Empty(); r.PopFront())
    callback.Refine(r.Front(),castData);
}

void NSquaredBroadPhase::CastAabb(CastDataParam castData, ProxyCastResults& results)
{
  SimpleAabbCallback callback(mCastAabbCallBack,&results);
  range r = mNSquared.Query();
  for(; !r.Empty(); r.PopFront())
    callback.Refine(r.Front(),castData);
}

void NSquaredBroadPhase::CastSphere(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSphereCallback callback(mCastSphereCallBack,&results);
  range r = mNSquared.Query();
  for(; !r.Empty(); r.PopFront())
    callback.Refine(r.Front(),castData);
}

void NSquaredBroadPhase::CastFrustum(CastDataParam castData, ProxyCastResults& results)
{
  SimpleFrustumCallback callback(mCastFrustumCallBack,&results);
  range r = mNSquared.Query();
  for(; !r.Empty(); r.PopFront())
    callback.Refine(r.Front(),castData);
}

void NSquaredBroadPhase::RegisterCollisions()
{
  mDataPairs.Clear();

  BroadPhaseType::PairRange r = mNSquared.QueryPairRange();
  for(; !r.Empty(); r.PopFront())
  {
    Pair<void*,void*> dataPair = r.Front();
    mDataPairs.PushBack(ClientPair(dataPair.first,dataPair.second));
  }
}

void NSquaredBroadPhase::GetCollisions(BroadPhaseData& data, ClientPairArray& results)
{
  range r = mNSquared.Query();
  for(; !r.Empty(); r.PopFront())
    results.PushBack(ClientPair(data.mClientData,r.Front()));
}

}//namespace Zero
