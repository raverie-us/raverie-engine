///////////////////////////////////////////////////////////////////////////////
///
/// \file ProxyCast.cpp
/// Implementation of the classes for Ray Casting.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(BaseCastFilter, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(IgnoreStatic);
  ZilchBindGetterSetterProperty(IgnoreDynamic);
  ZilchBindGetterSetterProperty(IgnoreKinematic);
  ZilchBindGetterSetterProperty(IgnoreGhost);
  ZilchBindGetterSetterProperty(IgnoreChildren);
}

BaseCastFilter::BaseCastFilter()
{
  mFlags.SetFlag(BaseCastFilterFlags::Refine | BaseCastFilterFlags::GetContactNormal | 
                 BaseCastFilterFlags::IgnoreInternalCasts | BaseCastFilterFlags::IgnoreGhost);
  mIgnoredCog = nullptr;
}

BaseCastFilter::BaseCastFilter(const BaseCastFilter& rhs)
{
  mIgnoredCog = rhs.mIgnoredCog;
  mFlags.U32Field = rhs.mFlags.U32Field;
}

bool BaseCastFilter::IsSet(BaseCastFilterFlags::Enum flag)
{
  return mFlags.IsSet(flag);
}

void BaseCastFilter::Set(BaseCastFilterFlags::Enum flag)
{
  mFlags.SetFlag(flag);
  ValidateFlags();
}

void BaseCastFilter::SetState(BaseCastFilterFlags::Enum flag, bool state)
{
  mFlags.SetState(flag, state);
}

void BaseCastFilter::ClearFlag(BaseCastFilterFlags::Enum flag)
{
  mFlags.ClearFlag(flag);
}

void BaseCastFilter::IgnoreCog(void* cog)
{
  mIgnoredCog = cog;
}

void BaseCastFilter::ValidateFlags()
{
  bool refineSet = IsSet(BaseCastFilterFlags::Refine);
  bool normalSet = IsSet(BaseCastFilterFlags::GetContactNormal);
  ErrorIf(!refineSet && normalSet, 
          "Cannot get contact normals without the refine flag set.");
}

//-------------------------------------------------------------------CastData
CastData::CastData(Vec3Param start, Vec3Param vec)
{
  GetSegment().Start = start;
  GetSegment().End = vec;
}

CastData::CastData(const Ray& ray)
{
  Ray& internalRay = GetRay();
  internalRay.Start = ray.Start;
  internalRay.Direction = ray.Direction;
}

CastData::CastData(const Segment& segment)
{
  Segment& internalSegment = GetSegment();
  internalSegment.Start = segment.Start;
  internalSegment.End = segment.End;
}

CastData::CastData(const Aabb& aabb)
{
  GetAabb() = aabb;
}

CastData::CastData(const Sphere& sphere)
{
  GetSphere() = sphere;
}

CastData::CastData(const Frustum& frustum)
{
  GetFrustum() = frustum;
}

Ray& CastData::GetRay()
{
  return *(Ray*)bytes;
}

const Ray& CastData::GetRay() const
{
  return *(Ray*)bytes;
}

Segment& CastData::GetSegment()
{
  return *(Segment*)bytes;
}

const Segment& CastData::GetSegment() const
{
  return *(Segment*)bytes;
}

Aabb& CastData::GetAabb()
{
  return *(Aabb*)bytes;
}

const Aabb& CastData::GetAabb() const
{
  return *(Aabb*)bytes;
}

Sphere& CastData::GetSphere()
{
  return *(Sphere*)bytes;
}

const Sphere& CastData::GetSphere() const
{
  return *(Sphere*)bytes;
}

Frustum& CastData::GetFrustum()
{
  return *(Frustum*)bytes;
}

const Frustum& CastData::GetFrustum() const
{
  return *(Frustum*)bytes;
}

//-------------------------------------------------------------------ProxyResult
ProxyResult::ProxyResult(void* proxy, Vec3Param p1, Vec3Param p2,
                         Vec3Param contactNormal, real time)
{
  mObjectHit = proxy;
  mPoints[0] = p1;
  mPoints[1] = p2;
  mContactNormal = contactNormal;
  mTime = time;
}

void ProxyResult::operator=(const ProxyResult& rhs)
{
  mObjectHit = rhs.mObjectHit;
  mPoints[0] = rhs.mPoints[0];
  mPoints[1] = rhs.mPoints[1];
  mTime = rhs.mTime;
  mContactNormal = rhs.mContactNormal;
}

bool ProxyResult::operator==(const ProxyResult& rhs)
{
  return (mObjectHit == rhs.mObjectHit && 
          mPoints[0] == rhs.mPoints[0] &&
          mPoints[1] == rhs.mPoints[1] &&
          mTime == rhs.mTime &&
          mContactNormal == rhs.mContactNormal);
}

bool ProxyResult::operator!=(const ProxyResult& rhs)
{
  return !(*this == rhs);
}

void ProxyResult::Set(void* proxy, Vec3 points[2], 
                      Vec3Param contactNormal, real time)
{
  mObjectHit = proxy;
  mPoints[0] = points[0];
  mPoints[1] = points[1];
  mContactNormal = contactNormal;
  mTime = time;
}

//-------------------------------------------------------------------ProxyCastResults
ProxyCastResults::ProxyCastResults(ProxyCastResultArray& array, 
                                         BaseCastFilter& filter) 
 : Results(array), CurrSize(0), Filter(filter)
{

}

bool ProxyCastResults::Insert(ProxyResult& proxyResult)
{
  return Insert(proxyResult.mObjectHit, proxyResult.mPoints, 
                proxyResult.mContactNormal, proxyResult.mTime);
}

bool ProxyCastResults::Insert(void* mObjectHit, real distance)
{
  Vec3 points[2];
  points[0].ZeroOut();
  points[1].ZeroOut();
  Vec3 normal(0,0,0);
  return Insert(mObjectHit, points, normal, distance);
}

bool ProxyCastResults::Insert(void* mObjectHit, 
                                 Vec3 points[2], Vec3Param normal, real time)
{
  if(!IsValid(mObjectHit))
    return false;

  // Base case
  if(CurrSize == 0)
  {
    Results[CurrSize++].Set(mObjectHit, points, normal, time);
    return true;
  }

  // If there is room in the back of the array
  if(CurrSize < Results.Size())
  {
    // Store the proxy and increment the current size.
    Results[CurrSize++].Set(mObjectHit, points, normal, time);
  }
  else
  {
    // Check if the time of collision was sooner than the back of the results.
    if(time < Results[CurrSize - 1].mTime)
      Results[CurrSize - 1].Set(mObjectHit, points, normal, time);
    // The object collision wasn't soon enough, so return.
    else 
      return false;
  }

  // Walk from the current slot to the front.
  for(uint i = CurrSize - 1; i > 0; --i)
  {
    // Swap the two in place.
    if(Results[i].mTime < Results[i - 1].mTime)
      Swap(Results[i], Results[i - 1]);
    // Break out if the current collision was later (objects are in the correct order).
    else
      break;
  }

  return true;
}

void ProxyCastResults::Merge(ProxyCastResults& rhs)
{
  range r = rhs.All();

  while(!r.Empty())
  {
    if(!Insert(r.Front()))
      break;

    r.PopFront();
  }
}

bool ProxyCastResults::Match(ProxyCastResults& rhs)
{
  // If the sizes are different, they are not a match
  if(CurrSize != rhs.CurrSize)
    return false;

  // Check each proxy result
  for(uint i = 0; i < CurrSize; ++i)
  {
    // If any are different, they are not a match 
    if(Results[i] != rhs.Results[i])
      return false;
  }

  return true;
}

ProxyCastResults::range ProxyCastResults::All()
{
  return range(Results.Begin(), Results.Begin() + CurrSize);
}

void ProxyCastResults::Clear()
{
  CurrSize = 0;
}

bool ProxyCastResults::IsValid(void* mObjectHit)
{
  return Filter.IsValid(mObjectHit);
}

}//namespace Zero
