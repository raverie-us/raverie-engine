///////////////////////////////////////////////////////////////////////////////
///
/// \file BoundingSphereBroadPhase.cpp
/// Implementation of the BoundingSphereBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(BoundingSphereBroadPhase, builder, type)
{
}

void BoundingSphereBroadPhase::Serialize(Serializer& stream)
{
  IBroadPhase::Serialize(stream);
  mBoundingSphere.Serialize(stream);
}

void BoundingSphereBroadPhase::CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mBoundingSphere.CreateProxy(proxy,data);
}

void BoundingSphereBroadPhase::CreateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mBoundingSphere.CreateProxy(*obj.mProxy,obj.mData);
  }
}

void BoundingSphereBroadPhase::RemoveProxy(BroadPhaseProxy& proxy)
{
  mBoundingSphere.RemoveProxy(proxy);
}

void BoundingSphereBroadPhase::RemoveProxies(ProxyHandleArray& proxies)
{
  ProxyHandleArray::range range = proxies.All();
  for(; !range.Empty(); range.PopFront())
    mBoundingSphere.RemoveProxy(*range.Front());
}

void BoundingSphereBroadPhase::UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mBoundingSphere.UpdateProxy(proxy,data);
}

void BoundingSphereBroadPhase::UpdateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mBoundingSphere.UpdateProxy(*obj.mProxy,obj.mData);
  }
}

void BoundingSphereBroadPhase::SelfQuery(ClientPairArray& results)
{
  results.Insert(results.End(),mDataPairs.All());
}

void BoundingSphereBroadPhase::Query(BroadPhaseData& data, ClientPairArray& results)
{
  DefaultRange r = mBoundingSphere.Query(data.mBoundingSphere);
  GetCollisions(data,results);
}

void BoundingSphereBroadPhase::BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results)
{
  for(uint i = 0; i < data.Size(); ++i)
    GetCollisions(data[i],results);
}

void BoundingSphereBroadPhase::CastRay(CastDataParam castData, ProxyCastResults& results)
{
  SimpleRayCallback callback(mCastRayCallBack,&results);

  BoundingSphereRange<void*,Ray> range = mBoundingSphere.Query(castData.GetRay());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void BoundingSphereBroadPhase::CastSegment(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSegmentCallback callback(mCastSegmentCallBack,&results);

  BoundingSphereRange<void*,Segment> range = mBoundingSphere.Query(castData.GetSegment());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void BoundingSphereBroadPhase::CastAabb(CastDataParam castData, ProxyCastResults& results)
{
  SimpleAabbCallback callback(mCastAabbCallBack,&results);

  BoundingSphereRange<void*,Aabb> range = mBoundingSphere.Query(castData.GetAabb());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void BoundingSphereBroadPhase::CastSphere(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSphereCallback callback(mCastSphereCallBack,&results);

  BoundingSphereRange<void*,Sphere> range = mBoundingSphere.Query(castData.GetSphere());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void BoundingSphereBroadPhase::CastFrustum(CastDataParam castData, ProxyCastResults& results)
{
  SimpleFrustumCallback callback(mCastFrustumCallBack,&results);

  BoundingSphereRange<void*,Frustum> range = mBoundingSphere.Query(castData.GetFrustum());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(),castData);
}

void BoundingSphereBroadPhase::RegisterCollisions()
{
  mDataPairs.Clear();

  BroadPhaseType::PairRange r = mBoundingSphere.QueryPairRange();
  for(; !r.Empty(); r.PopFront())
  {
    Pair<void*,void*> dataPair = r.Front();
    mDataPairs.PushBack(ClientPair(dataPair.first,dataPair.second));
  }
}

void BoundingSphereBroadPhase::GetCollisions(BroadPhaseData& data, ClientPairArray& results)
{
  DefaultRange range = mBoundingSphere.Query(data.mBoundingSphere);
  for(; !range.Empty(); range.PopFront())
    results.PushBack(ClientPair(data.mClientData,range.Front()));
}

}//namespace Zero
