///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleCastCallbacks.hpp
/// Implementation of the SimpleCastCallback classes.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
//---------------------------------------------------------- Simple Ray Callback
struct SimpleRayCallback
{
  SimpleRayCallback(IBroadPhase::RayCastCallBack callback, 
                    ProxyCastResults* results)
  {
    mCallback = callback;
    mResults = results;
  }

  bool CastVsAabb(Aabb& aabb, CastDataParam castData)
  {
    real time;
    return IBroadPhase::TestRayVsAabb(aabb, castData.GetRay().Start, castData.GetRay().Direction, time);
  }

  bool CastVsSphere(Sphere& sphere, CastDataParam castData)
  {
    real time;
    return IBroadPhase::TestRayVsSphere(sphere, castData.GetRay().Start, castData.GetRay().Direction, time);
  }

  void Refine(void* clientData, CastDataParam castData)
  {
    ProxyResult result;
    if(mCallback(clientData, castData, result, mResults->Filter))
    {
      result.mObjectHit = clientData;
      mResults->Insert(result);
    }
  }

  IBroadPhase::RayCastCallBack mCallback;
  ProxyCastResults* mResults;
};

//------------------------------------------------------ Simple Segment Callback
struct SimpleSegmentCallback
{
  SimpleSegmentCallback(IBroadPhase::RayCastCallBack callback, 
                        ProxyCastResults* results)
  {
    mCallback = callback;
    mResults = results;
  }

  bool CastVsAabb(Aabb& aabb, CastDataParam castData)
  {
    real time;
    return IBroadPhase::TestSegmentVsAabb(aabb, castData.GetSegment().Start, castData.GetSegment().End, time);
  }

  bool CastVsSphere(Sphere& sphere, CastDataParam castData)
  {
    real time;
    return IBroadPhase::TestSegmentVsSphere(sphere, castData.GetSegment().Start, castData.GetSegment().End, time);
  }

  void Refine(void* clientData, CastDataParam castData)
  {
    ProxyResult result;
    if(mCallback(clientData, castData, result, mResults->Filter))
    {
      result.mObjectHit = clientData;
      mResults->Insert(result);
    }
  }

  IBroadPhase::RayCastCallBack mCallback;
  ProxyCastResults* mResults;
};

//--------------------------------------------------------- Simple AABB Callback
struct SimpleAabbCallback
{
  SimpleAabbCallback(IBroadPhase::VolumeCastCallBack callback, 
                     ProxyCastResults* results)
  {
    mCallback = callback;
    mResults = results;
  }

  bool CastVsAabb(Aabb& aabb, CastDataParam castData)
  {
    Intersection::Type testResult = Intersection::AabbAabb(
      aabb.mMin, aabb.mMax, castData.GetAabb().mMin, castData.GetAabb().mMax, nullptr);
    return testResult != Intersection::None;
  }

  bool CastVsSphere(Sphere& sphere, CastDataParam castData)
  {
    Intersection::Type testResult = Intersection::AabbSphere(
      castData.GetAabb().mMin, castData.GetAabb().mMax,
      sphere.mCenter, sphere.mRadius, nullptr);
    return testResult != Intersection::None;
  }

  void Refine(void* clientData, CastDataParam castData)
  {
    ProxyResult result;
    if(mCallback(clientData, castData, result, mResults->Filter))
    {
      result.mObjectHit = clientData;
      mResults->Insert(result);
    }
  }

  IBroadPhase::VolumeCastCallBack mCallback;
  ProxyCastResults* mResults;
};

//------------------------------------------------------- Simple Sphere Callback
struct SimpleSphereCallback
{
  SimpleSphereCallback(IBroadPhase::VolumeCastCallBack callback,
                       ProxyCastResults* results)
  {
    mCallback = callback;
    mResults = results;
  }

  bool CastVsAabb(Aabb& aabb, CastDataParam castData)
  {
    Intersection::Type testResult = Intersection::AabbSphere(aabb.mMin, aabb.mMax,
      castData.GetSphere().mCenter, castData.GetSphere().mRadius, nullptr);
    return testResult != Intersection::None;
  }

  bool CastVsSphere(Sphere& sphere, CastDataParam castData)
  {
    Intersection::Type testResult = Intersection::SphereSphere(
      castData.GetSphere().mCenter, castData.GetSphere().mRadius,
      sphere.mCenter, sphere.mRadius, nullptr);
    return testResult != Intersection::None;
  }

  void Refine(void* clientData, CastDataParam castData)
  {
    ProxyResult result;
    if(mCallback(clientData, castData, result, mResults->Filter))
    {
      result.mObjectHit = clientData;
      mResults->Insert(result);
    }
  }

  IBroadPhase::VolumeCastCallBack mCallback;
  ProxyCastResults* mResults;
};

//------------------------------------------------------ Simple Frustum Callback
struct SimpleFrustumCallback
{
  SimpleFrustumCallback(IBroadPhase::VolumeCastCallBack callback, 
                        ProxyCastResults* results)
  {
    mCallback = callback;
    mResults = results;
  }

  bool CastVsAabb(Aabb& aabb, CastDataParam castData)
  {
    Intersection::Type testResult = Intersection::AabbFrustumApproximation(
      aabb.mMin, aabb.mMax, &castData.GetFrustum().Planes[0].GetData(), nullptr);
    return testResult != Intersection::None;
  }

  bool CastVsSphere(Sphere& sphere, CastDataParam castData)
  {
    using namespace Intersection;
    Intersection::Type testResult = FrustumSphereApproximation(
      castData.GetFrustum().GetIntersectionData(),
      sphere.mCenter, sphere.mRadius, nullptr);
    return testResult != Intersection::None;
  }

  void Refine(void* clientData, CastDataParam castData)
  {
    ProxyResult result;
    if(mCallback(clientData, castData, result, mResults->Filter))
    {
      result.mObjectHit = clientData;
      mResults->Insert(result);
    }
  }

  IBroadPhase::VolumeCastCallBack mCallback;
  ProxyCastResults* mResults;
};

//----------------------------------------------------------- Broad Phase Policy
template <typename T1, typename T2>
struct BroadPhasePolicy
{
  bool Overlap(T1& obj1, T2& obj2)
  {
    return Zero::Overlap(obj1,obj2);
  }
};

template <>
struct BroadPhasePolicy<Frustum, Aabb>
{
  bool Overlap(Frustum& frustum, Aabb& aabb)
  {
    return Intersection::AabbFrustumApproximation(aabb.mMin, aabb.mMax, frustum.GetIntersectionData()) >= (Intersection::Type)0;
  }
};

template <>
struct BroadPhasePolicy<Frustum, Sphere>
{
  bool Overlap(Frustum& frustum, Sphere& sphere)
  {
    return Intersection::FrustumSphereApproximation(frustum.GetIntersectionData(), sphere.mCenter, sphere.mRadius) >= (Intersection::Type)0;
  }
};

}//namespace Zero
