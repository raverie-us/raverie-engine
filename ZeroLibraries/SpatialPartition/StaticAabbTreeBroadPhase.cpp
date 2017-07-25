///////////////////////////////////////////////////////////////////////////////
///
/// \file StaticAabbTreeBroadPhase.cpp
/// Implementation of the StaticAabbTreeBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(StaticAabbTreeBroadPhase, builder, type)
{
}

void StaticAabbTreeBroadPhase::Serialize(Serializer& stream)
{
  IBroadPhase::Serialize(stream);
  mTree.Serialize(stream);
}

void StaticAabbTreeBroadPhase::Draw(int level, uint debugDrawFlags)
{
  mTree.Draw(level,debugDrawFlags);
}

void StaticAabbTreeBroadPhase::CreateProxy(BroadPhaseProxy& proxy, 
                                           BroadPhaseData& data)
{
  mTree.CreateProxy(proxy,data);
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::CreateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mTree.CreateProxy(*obj.mProxy,obj.mData);
  }
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::RemoveProxy(BroadPhaseProxy& proxy)
{
  mTree.RemoveProxy(proxy);
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::RemoveProxies(ProxyHandleArray& proxies)
{
  ProxyHandleArray::range range = proxies.All();
  for(; !range.Empty(); range.PopFront())
    mTree.RemoveProxy(*range.Front());
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::UpdateProxy(BroadPhaseProxy& proxy, 
                                           BroadPhaseData& data)
{
  mTree.UpdateProxy(proxy,data);
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::UpdateProxies(BroadPhaseObjectArray& objects)
{
  BroadPhaseObjectArray::range range = objects.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseObject& obj = range.Front();
    mTree.UpdateProxy(*obj.mProxy,obj.mData);
  }
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::SelfQuery(ClientPairArray& results)
{
  ErrorIf(true, "Static Aabb Tree cannot be used as a Dynamic BroadPhase.");
}

void StaticAabbTreeBroadPhase::Query(BroadPhaseData& data, 
                                     ClientPairArray& results)
{
  forRangeBroadphaseTree(TreeType,mTree,Aabb,data.mAabb)
    results.PushBack(ClientPair(data.mClientData,range.Front()));
}

void StaticAabbTreeBroadPhase::BatchQuery(BroadPhaseDataArray& data, 
                                          ClientPairArray& results)
{
  BroadPhaseDataArray::range range = data.All();
  for(; !range.Empty(); range.PopFront())
    Query(range.Front(),results);
}

void StaticAabbTreeBroadPhase::Construct()
{
  mTree.Construct();
}

void StaticAabbTreeBroadPhase::CastRay(CastDataParam data, 
                                       ProxyCastResults& results)
{
  SimpleRayCallback callback(mCastRayCallBack,&results);

  forRangeBroadphaseTree(TreeType,mTree,Ray,data.GetRay())
    callback.Refine(range.Front(),data);
}

void StaticAabbTreeBroadPhase::CastSegment(CastDataParam data, 
                                           ProxyCastResults& results)
{
  SimpleSegmentCallback callback(mCastSegmentCallBack,&results);

  forRangeBroadphaseTree(TreeType,mTree,Segment,data.GetSegment())
    callback.Refine(range.Front(),data);
}

void StaticAabbTreeBroadPhase::CastAabb(CastDataParam data, 
                                        ProxyCastResults& results)
{
  SimpleAabbCallback callback(mCastAabbCallBack,&results);

  forRangeBroadphaseTree(TreeType,mTree,Aabb,data.GetAabb())
    callback.Refine(range.Front(),data);
}

void StaticAabbTreeBroadPhase::CastSphere(CastDataParam data, 
                                          ProxyCastResults& results)
{
  SimpleSphereCallback callback(mCastSphereCallBack,&results);

  forRangeBroadphaseTree(TreeType,mTree,Sphere,data.GetSphere())
    callback.Refine(range.Front(),data);
}

void StaticAabbTreeBroadPhase::CastFrustum(CastDataParam data, 
                                           ProxyCastResults& results)
{
  SimpleFrustumCallback callback(mCastFrustumCallBack,&results);

  forRangeBroadphaseTree(TreeType,mTree,Frustum,data.GetFrustum())
    callback.Refine(range.Front(),data);
}

}//namespace Zero
