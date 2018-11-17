///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhase.cpp
/// Implementation of the IBroadPhase class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

static Memory::Heap* sDefaultBroadPhaseHeap = Memory::GetNamedHeap("BroadPhase");

///Declaration of the static ray cast callbacks.
IBroadPhase::RayCastCallBack IBroadPhase::mCastRayCallBack = nullptr;
IBroadPhase::RayCastCallBack IBroadPhase::mCastSegmentCallBack = nullptr;
IBroadPhase::VolumeCastCallBack IBroadPhase::mCastAabbCallBack = nullptr;
IBroadPhase::VolumeCastCallBack IBroadPhase::mCastSphereCallBack = nullptr;
IBroadPhase::VolumeCastCallBack IBroadPhase::mCastFrustumCallBack = nullptr;

ZilchDefineType(IBroadPhase, builder, type)
{
}


IBroadPhase::IBroadPhase()
{
  mHeap = sDefaultBroadPhaseHeap;
}

void IBroadPhase::SetHeap(Memory::Heap* heap)
{
  mHeap = heap;
}

void IBroadPhase::Serialize(Serializer& stream)
{
  SerializeEnumName(BroadPhase, mType);
}

void IBroadPhase::CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  ErrorIf(true, "CreateProxy function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::CreateProxies(BroadPhaseObjectArray& objects)
{
  ErrorIf(true, "CreateProxies function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::RemoveProxy(BroadPhaseProxy& proxy)
{
  ErrorIf(true, "RemoveProxy function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::RemoveProxies(ProxyHandleArray& proxies)
{
  ErrorIf(true, "RemoveProxies function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  ErrorIf(true, "UpdateProxy function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::UpdateProxies(BroadPhaseObjectArray& objects)
{
  ErrorIf(true, "UpdateProxies function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::SelfQuery(ClientPairArray& results)
{
  ErrorIf(true, "SelfQuery function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::Query(BroadPhaseData& data, ClientPairArray& results)
{
  ErrorIf(true, "Query function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results)
{
  ErrorIf(true, "BatchQuery function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::Construct()
{
  ErrorIf(true, "Construct function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::CastRay(CastDataParam data, ProxyCastResults& results)
{
  ErrorIf(true, "CastRay function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::CastSegment(CastDataParam data, ProxyCastResults& results)
{
  ErrorIf(true, "CastSegment function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::CastAabb(CastDataParam data, ProxyCastResults& results)
{
  ErrorIf(true, "CastAabb function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::CastSphere(CastDataParam data, ProxyCastResults& results)
{
  ErrorIf(true, "CastSphere function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::CastFrustum(CastDataParam data, ProxyCastResults& results)
{
  ErrorIf(true, "CastFrustum function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::RegisterCollisions()
{
  ErrorIf(true, "RegisterCollisions function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

void IBroadPhase::Cleanup()
{
  ErrorIf(true, "Cleanup function not implemented on BroadPhase %s",
          ZilchGetDerivedType()->Name.c_str());
}

bool IBroadPhase::TestRayVsAabb(const Aabb& aabb, Vec3Param start, 
                                Vec3Param direction, real& time)
{
  Intersection::IntersectionPoint resultData;

  Intersection::Type ret = Intersection::RayAabb(start, direction, aabb.mMin, 
                                                 aabb.mMax, &resultData);

  if(ret == Intersection::None)
    return false;

  time = resultData.T;
  return true;
}

bool IBroadPhase::TestSegmentVsAabb(const Aabb& aabb, Vec3Param start,
                                    Vec3Param end, real& time)
{
  Intersection::IntersectionPoint resultData;

  Intersection::Type ret = Intersection::SegmentAabb(start, end, aabb.mMin, 
                                                     aabb.mMax, &resultData);

  if(ret == Intersection::None)
    return false;

  time = resultData.T;
  return true;
}

///Tests Ray against Sphere.
bool IBroadPhase::TestRayVsSphere(const Sphere& sphere, Vec3Param start,
                                  Vec3Param direction, real& time)
{
  Intersection::IntersectionPoint resultData;

  Intersection::Type ret = Intersection::RaySphere(start, direction, 
                                                   sphere.mCenter, 
                                                   sphere.mRadius, &resultData);

  if(ret == Intersection::None)
    return false;

  time = resultData.T;
  return true;
}

///Tests Line Segment against Sphere.
bool IBroadPhase::TestSegmentVsSphere(const Sphere& sphere, Vec3Param start, 
                                      Vec3Param end, real& time)
{
  Intersection::IntersectionPoint resultData;

  Intersection::Type ret = Intersection::SegmentSphere(start, end, 
                                                       sphere.mCenter, 
                                                       sphere.mRadius, 
                                                       &resultData);

  if(ret == Intersection::None)
    return false;

  time = resultData.T;
  return true;
}

uint IBroadPhase::GetType()
{
  return mType;
}

}//namespace Zero
