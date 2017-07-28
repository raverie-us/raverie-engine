///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhasePackage.cpp
/// Implementation of the BroadPhasePackage class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(BroadPhasePackage, builder, type)
{
}

BroadPhasePackage::BroadPhasePackage()
{
  mBroadPhases[BroadPhase::Dynamic] = nullptr;
  mBroadPhases[BroadPhase::Static] = nullptr;
  mRefineRayCast = false;
}

BroadPhasePackage::~BroadPhasePackage()
{
  delete mBroadPhases[BroadPhase::Dynamic];
  delete mBroadPhases[BroadPhase::Static]; 
}

void BroadPhasePackage::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Loading)
    LoadFromStream(stream);
  else
    SaveToStream(stream);
}

void BroadPhasePackage::SaveToStream(Serializer& stream)
{
  stream.StartPolymorphic(ZilchVirtualTypeId(this));

  //Serialize flags
  SerializeName(mRefineRayCast);
  // REMOVE: Legacy
  bool mTracking = false;
  SerializeName(mTracking);

  IBroadPhase* broadPhase = mBroadPhases[BroadPhase::Dynamic];
  if(broadPhase != nullptr)
  {
    BoundType* type = ZilchVirtualTypeId(broadPhase);
    stream.StartPolymorphic(type);
    broadPhase->Serialize(stream);
    stream.EndPolymorphic();
  }

  broadPhase = mBroadPhases[BroadPhase::Static];
  if(broadPhase != nullptr)
  {
    BoundType* type = ZilchVirtualTypeId(broadPhase);
    stream.StartPolymorphic(type);
    broadPhase->Serialize(stream);
    stream.EndPolymorphic(); 
  }

  stream.EndPolymorphic();
}

void BroadPhasePackage::LoadFromStream(Serializer& stream)
{
  PolymorphicNode headNode;
  stream.GetPolymorphic(headNode);

  //Serialize flags
  SerializeName(mRefineRayCast);
  // REMOVE: Legacy
  bool mTracking;
  SerializeName(mTracking);

  //Serialize each broad phase
  PolymorphicNode broadPhaseNode;

  while(stream.GetPolymorphic(broadPhaseNode))
  {
    BroadPhaseCreator* broadPhaseCreator = Z::gBroadPhaseLibrary->GetCreatorBy(broadPhaseNode);
    
    ErrorIf(broadPhaseCreator == nullptr, "Invalid broad phase node %s.", broadPhaseNode.TypeName.Data());
    if(broadPhaseCreator == nullptr)
      continue;
    //Create the broad phase.
    IBroadPhase* broadPhase = broadPhaseCreator->Create();

    //Serialize the broad phase from the data stream.
    broadPhase->Serialize(stream);

    //Add the new broad phase.
    AddBroadPhase(broadPhase->GetType(), broadPhase);
    //Any post scene processing.
    broadPhase->Construct();

    //End the broad phase node.
    stream.EndPolymorphic();
  }

  stream.EndPolymorphic();
}

void BroadPhasePackage::AddBroadPhase(uint type, IBroadPhase* broadphase)
{
  //If we aren't tracking, we can only have one broad phase registered.
  ErrorIf(mBroadPhases[type] != nullptr, 
          "Broad phase already registered.");
 
  //Add it.
  mBroadPhases[type] = broadphase;
}

void BroadPhasePackage::Draw(int level, uint debugFlags)
{
  mBroadPhases[BroadPhase::Dynamic]->Draw(level, debugFlags);
  mBroadPhases[BroadPhase::Static]->Draw(level, debugFlags); 
}

void BroadPhasePackage::CreateProxy(uint type, BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mBroadPhases[type]->CreateProxy(proxy, data);
}

void BroadPhasePackage::CreateProxies(uint type, BroadPhaseObjectArray& objects)
{
  mBroadPhases[type]->CreateProxies(objects);
}

void BroadPhasePackage::RemoveProxy(uint type, BroadPhaseProxy& proxy)
{
  mBroadPhases[type]->RemoveProxy(proxy);
}

void BroadPhasePackage::RemoveProxies(uint type, ProxyHandleArray& proxies)
{
  mBroadPhases[type]->RemoveProxies(proxies);
}

void BroadPhasePackage::UpdateProxy(uint type, BroadPhaseProxy& proxy, BroadPhaseData& data)
{
  mBroadPhases[type]->UpdateProxy(proxy, data);
}

void BroadPhasePackage::UpdateProxies(uint type, BroadPhaseObjectArray& objects)
{
  mBroadPhases[type]->UpdateProxies(objects);
}

void BroadPhasePackage::SelfQuery(ClientPairArray& results)
{
  mBroadPhases[BroadPhase::Dynamic]->SelfQuery(results);
}

void BroadPhasePackage::Query(BroadPhaseData& data, ClientPairArray& results)
{
  mBroadPhases[BroadPhase::Static]->Query(data, results);
}

void BroadPhasePackage::BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results)
{
  mBroadPhases[BroadPhase::Static]->BatchQuery(data, results);
}

void BroadPhasePackage:: QueryBoth(BroadPhaseData& data, ClientPairArray& results)
{
  mBroadPhases[BroadPhase::Static]->Query(data, results);
  mBroadPhases[BroadPhase::Dynamic]->Query(data, results);
}

void BroadPhasePackage::Construct()
{
  mBroadPhases[BroadPhase::Static]->Construct(); 
}

void BroadPhasePackage::RegisterCollisions()
{
  mBroadPhases[BroadPhase::Dynamic]->RegisterCollisions();
}

void BroadPhasePackage::Cleanup()
{
  mBroadPhases[BroadPhase::Dynamic]->Cleanup();
  mBroadPhases[BroadPhase::Static]->Cleanup(); 
}

void BroadPhasePackage::CastRay(Vec3Param startPos, Vec3Param direction, 
                                ProxyCastResults& results)
{
  CastData rayData(startPos, direction);
  BaseCastFilter& filter = results.Filter;
  const bool ignoreDynamic = filter.IsSet(BaseCastFilterFlags::IgnoreDynamic) && 
                             filter.IsSet(BaseCastFilterFlags::IgnoreKinematic) &&
                             filter.IsSet(BaseCastFilterFlags::IgnoreStatic);
  const bool ignoreStatic = filter.IsSet(BaseCastFilterFlags::IgnoreStatic);

  //If we aren't refining the ray, simply cast into both.
  //However, if we are refining the ray but we're ignoring dynamic objects,
  //there is nothing we can do so continue as normal.
  if(mRefineRayCast == false || ignoreDynamic)
  {
    //Cast into the dynamic objects.
    if(!ignoreDynamic)
    {
      CastIntoBroadphase(BroadPhase::Dynamic, rayData, results, 
                         &IBroadPhase::CastRay);
    }

    //Cast into the static objects.
    if(!ignoreStatic)
    {
      CastIntoBroadphase(BroadPhase::Static, rayData, results, 
                         &IBroadPhase::CastRay);
    }
  }
  else
  {
    Vec3 collisionPoint;
    //We only want the first result.
    ProxyCastResultArray staticArray(1);
    //Default filter
    ProxyCastResults staticResults(staticArray, results.Filter);

    if(GetFirstContactInStatic(rayData, collisionPoint, staticResults))
    {
      //Convert the ray into a segment
      CastData segmentData(startPos, collisionPoint);

      //Cast into the dynamic with this new segment
      CastIntoBroadphase(BroadPhase::Dynamic, segmentData, results,
                         &IBroadPhase::CastSegment);

      //If we still have room, merge the static results in.
      if(results.GetRemainingSize() > 0 && ignoreStatic == false)
        results.Merge(staticResults);
    }
    else
    {
      CastIntoBroadphase(BroadPhase::Dynamic, rayData, results, 
                         &IBroadPhase::CastRay);
    }
  }
}

void BroadPhasePackage::CastSegment(Vec3Param startPos, Vec3Param endPos, 
  ProxyCastResults& results)
{
  CastData data(startPos, endPos);

  BaseCastFilter& filter = results.Filter;
  const bool ignoreDynamic = filter.IsSet(BaseCastFilterFlags::IgnoreDynamic) && 
                             filter.IsSet(BaseCastFilterFlags::IgnoreKinematic) &&
                             filter.IsSet(BaseCastFilterFlags::IgnoreStatic);

  //Cast into the dynamic objects.
  if(!ignoreDynamic)
  {
    CastIntoBroadphase(BroadPhase::Dynamic, data, results, 
                       &IBroadPhase::CastSegment);
  }

  //Cast into the static objects.
  if(!results.Filter.IsSet(BaseCastFilterFlags::IgnoreStatic))
  {
    CastIntoBroadphase(BroadPhase::Static, data, results, 
                       &IBroadPhase::CastSegment);
  }
}

void BroadPhasePackage::CastAabb(const Aabb& aabb, ProxyCastResults& results)
{
  CastData data(aabb);

  BaseCastFilter& filter = results.Filter;
  const bool ignoreDynamic = filter.IsSet(BaseCastFilterFlags::IgnoreDynamic) && 
                             filter.IsSet(BaseCastFilterFlags::IgnoreKinematic) &&
                             filter.IsSet(BaseCastFilterFlags::IgnoreStatic);

  //Cast into the dynamic objects.
  if(!ignoreDynamic)
  {
    CastIntoBroadphase(BroadPhase::Dynamic, data, results, 
                       &IBroadPhase::CastAabb);
  }

  //Cast into the static objects.
  if(!results.Filter.IsSet(BaseCastFilterFlags::IgnoreStatic))
  {
    CastIntoBroadphase(BroadPhase::Static, data, results, 
                       &IBroadPhase::CastAabb);
  }
}

void BroadPhasePackage::CastSphere(const Sphere& sphere, 
  ProxyCastResults& results)
{
  CastData data(sphere);

  BaseCastFilter& filter = results.Filter;
  const bool ignoreDynamic = filter.IsSet(BaseCastFilterFlags::IgnoreDynamic) && 
                             filter.IsSet(BaseCastFilterFlags::IgnoreKinematic) &&
                             filter.IsSet(BaseCastFilterFlags::IgnoreStatic);

  //Cast into the dynamic objects.
  if(!ignoreDynamic)
  {
    CastIntoBroadphase(BroadPhase::Dynamic, data, results, 
                       &IBroadPhase::CastSphere);
  }

  //Cast into the static objects.
  if(!results.Filter.IsSet(BaseCastFilterFlags::IgnoreStatic))
  {
    CastIntoBroadphase(BroadPhase::Static, data, results, 
                       &IBroadPhase::CastSphere);
  }
}

void BroadPhasePackage::CastFrustum(const Frustum& frustum, 
  ProxyCastResults& results)
{
  CastData data(frustum);

  BaseCastFilter& filter = results.Filter;
  const bool ignoreDynamic = filter.IsSet(BaseCastFilterFlags::IgnoreDynamic) && 
                             filter.IsSet(BaseCastFilterFlags::IgnoreKinematic) &&
                             filter.IsSet(BaseCastFilterFlags::IgnoreStatic);

  //Cast into the dynamic objects.
  if(!ignoreDynamic)
  {
    CastIntoBroadphase(BroadPhase::Dynamic, data, results, 
                       &IBroadPhase::CastFrustum);
  }

  //Cast into the static objects.
  if(!results.Filter.IsSet(BaseCastFilterFlags::IgnoreStatic))
  {
    CastIntoBroadphase(BroadPhase::Static, data, results, 
                       &IBroadPhase::CastFrustum);
  }
}

void BroadPhasePackage::CastIntoBroadphase(uint broadPhaseType, 
            CastDataParam data, ProxyCastResults& results, CastFunction func)
{
  (mBroadPhases[broadPhaseType]->*func)(data, results);
}

bool BroadPhasePackage::GetFirstContactInStatic(CastDataParam rayData, 
                                      Vec3& point, ProxyCastResults& results)
{
  //Cast the ray into the static
  CastIntoBroadphase(BroadPhase::Static, rayData, results,
                     &IBroadPhase::CastRay);

  //Grab the results
  ProxyCastResults::range r = results.All();

  //If we hit something
  if(!r.Empty())
  {
    //Get the point of collision
    point = r.Front().mPoints[0];
    return true;
  }

  return false;
}

}//namespace Zero
