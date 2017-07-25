///////////////////////////////////////////////////////////////////////////////
///
/// \file BoundingBoxBroadPhase.cpp
/// Implementation of the BoundingBoxBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(BoundingBoxBroadPhase, builder, type)
{
}

void BoundingBoxBroadPhase::Serialize(Serializer& stream)
{
  IBroadPhase::Serialize(stream);
  mBoundingBox.Serialize(stream);
}

void BoundingBoxBroadPhase::CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mBoundingBox.CreateProxy(proxy, data);
}

void BoundingBoxBroadPhase::CreateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mBoundingBox.CreateProxy(*obj.mProxy, obj.mData);
  }
}

void BoundingBoxBroadPhase::RemoveProxy(BroadPhaseProxy& proxy)
{
  mBoundingBox.RemoveProxy(proxy);
}

void BoundingBoxBroadPhase::RemoveProxies(ProxyHandleArray& proxies)
{
  ProxyHandleArray::range range = proxies.All();
  for(; !range.Empty(); range.PopFront())
    mBoundingBox.RemoveProxy(*range.Front());
}

void BoundingBoxBroadPhase::UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mBoundingBox.UpdateProxy(proxy,data);
}

void BoundingBoxBroadPhase::UpdateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mBoundingBox.UpdateProxy(*obj.mProxy, obj.mData);
  }
}

void BoundingBoxBroadPhase::SelfQuery(ClientPairArray& results)
{
  results.Insert(results.End(), mDataPairs.All());
}

void BoundingBoxBroadPhase::Query(BroadPhaseData& data, ClientPairArray& results)
{
  DefaultRange r = mBoundingBox.Query(data.mAabb);
  GetCollisions(data,results);
}

void BoundingBoxBroadPhase::BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results)
{
  for(uint i = 0; i < data.Size(); ++i)
    GetCollisions(data[i], results);
}

void BoundingBoxBroadPhase::CastRay(CastDataParam castData, ProxyCastResults& results)
{
  SimpleRayCallback callback(mCastRayCallBack,&results);

  BoundingBoxRange<void*,Ray> range = mBoundingBox.Query(castData.GetRay());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(), castData);
}

void BoundingBoxBroadPhase::CastSegment(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSegmentCallback callback(mCastSegmentCallBack,&results);

  BoundingBoxRange<void*,Segment> range = mBoundingBox.Query(castData.GetSegment());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(), castData);
}

void BoundingBoxBroadPhase::CastAabb(CastDataParam castData, ProxyCastResults& results)
{
  SimpleAabbCallback callback(mCastAabbCallBack,&results);

  BoundingBoxRange<void*,Aabb> range = mBoundingBox.Query(castData.GetAabb());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(), castData);
}

void BoundingBoxBroadPhase::CastSphere(CastDataParam castData, ProxyCastResults& results)
{
  SimpleSphereCallback callback(mCastSphereCallBack,&results);

  BoundingBoxRange<void*,Sphere> range = mBoundingBox.Query(castData.GetSphere());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(), castData);
}

void BoundingBoxBroadPhase::CastFrustum(CastDataParam castData, ProxyCastResults& results)
{
  SimpleFrustumCallback callback(mCastFrustumCallBack,&results);

  BoundingBoxRange<void*,Frustum> range = mBoundingBox.Query(castData.GetFrustum());
  for(; !range.Empty(); range.PopFront())
    callback.Refine(range.Front(), castData);
}

void BoundingBoxBroadPhase::RegisterCollisions()
{
  mDataPairs.Clear();

  BroadPhaseType::PairRange r = mBoundingBox.QueryPairRange();
  for(; !r.Empty(); r.PopFront())
  {
    Pair<void*,void*> dataPair = r.Front();
    mDataPairs.PushBack(ClientPair(dataPair.first, dataPair.second));
  }
}

void BoundingBoxBroadPhase::GetCollisions(BroadPhaseData& data, ClientPairArray& results)
{
  DefaultRange range = mBoundingBox.Query(data.mAabb);
  for(; !range.Empty(); range.PopFront())
    results.PushBack(ClientPair(data.mClientData, range.Front()));
}

}//namespace Zero
