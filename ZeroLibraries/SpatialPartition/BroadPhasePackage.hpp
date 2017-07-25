///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhasePackage.hpp
/// Declaration of the BroadPhasePackage class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Manages multiple broad phases and any queries to the broad phases.  
///Supports the comparison of broad phases accuracy and performance.  
///See BroadPhaseTracker.hpp for more info.
class BroadPhasePackage
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  BroadPhasePackage();
  virtual ~BroadPhasePackage();

  ///Serializes all broad phases.
  void Serialize(Serializer& stream);
  virtual void SaveToStream(Serializer& stream);
  virtual void LoadFromStream(Serializer& stream);

  ///Initializes tracker data if we're tracking.
  virtual void Initialize(){}

  ///Registers a broad phase to be updated.
  virtual void AddBroadPhase(uint type, IBroadPhase* broadphase);

  ///The results of collision detection should be reported through 
  ///this function.  Each index represents the lexicographical id of the 
  ///objects that collided.  This function will compare the results with 
  ///what each broad phase has recorded and report any discrepancies.
  virtual void RecordFrameResults(const Array<NodePointerPair>& results){}

  virtual bool IsTracking(){return false;}

public:
  ///Draws all broad phases (if they have something to draw).
  ///Not every algorithm will use the level.
  virtual void Draw(int level, uint debugFlags);

  ///Fills out the proxy for the broadphase at the position specified by data.
  ///The proxy can be thought of as a handle, an object should do nothing more
  ///than hold onto it and give it to broadphase when it wants to do something.
  virtual void CreateProxy(uint type, BroadPhaseProxy& proxy, 
                           BroadPhaseData& data);
  ///Batch version of CreateProxy.
  virtual void CreateProxies(uint type, BroadPhaseObjectArray& objects);
  ///Removes the given proxy.
  virtual void RemoveProxy(uint type, BroadPhaseProxy& proxy);
  ///Batch version of RemoveProxy.
  virtual void RemoveProxies(uint type, ProxyHandleArray& proxies);
  ///Updates the given proxy to the position specified by data.
  virtual void UpdateProxy(uint type, BroadPhaseProxy& proxy, 
                           BroadPhaseData& data);
  ///Batch version of UpdateProxy.
  virtual void UpdateProxies(uint type, BroadPhaseObjectArray& objects);

  ///Used to determine intersection of objects in this broadphase with other
  ///objects in the same broadphase. Mainly a physics things.
  virtual void SelfQuery(ClientPairArray& results);
  ///Finds everything that is in contact with the data.
  virtual void Query(BroadPhaseData& data, ClientPairArray& results);
  ///Batch version of Query.
  virtual void BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results);
  ///Queries both the dynamic and static broadphase.
  virtual void QueryBoth(BroadPhaseData& data, ClientPairArray& results);

  ///Tells the structure that it has all of the data it will ever have. Used 
  ///mainly for static BroadPhases.
  virtual void Construct();

  ///Casts a ray into the broad phases.  If the results is set to only grab
  ///a single object, it will cast into the static first, then use the position
  ///hit to turn the ray into a segment and cast that into the dynamic.
  virtual void CastRay(Vec3Param startPos, Vec3Param direction, 
                       ProxyCastResults& results);
  ///Casts a segment into the broad phase.
  virtual void CastSegment(Vec3Param startPos, Vec3Param endPos, 
                           ProxyCastResults& results);
  ///Casts an Aabb into the broad phases.  Returns all objects intersecting the
  ///bounding box.
  virtual void CastAabb(const Aabb& aabb, ProxyCastResults& results);
  ///Casts a Sphere into the broad phases.  Returns all objects intersecting the
  ///bounding sphere.
  virtual void CastSphere(const Sphere& sphere, ProxyCastResults& results);
  ///Casts a Frustum into the broad phases.  Returns all objects intersecting 
  ///the frustum.
  virtual void CastFrustum(const Frustum& sphere, ProxyCastResults& results);

  ///Computes all the collision pairs of objects already in the BroadPhase
  virtual void RegisterCollisions();

  ///Resets the BroadPhase after each update loop.
  virtual void Cleanup();

protected:
  typedef void (IBroadPhase::*CastFunction)(CastDataParam,ProxyCastResults&);
  virtual void CastIntoBroadphase(uint broadPhaseType, CastDataParam data, 
                               ProxyCastResults& results, CastFunction func);

  bool GetFirstContactInStatic(CastDataParam rayData, Vec3& point,
                               ProxyCastResults& results);

  //Enabling this will cause ray casting to cast into static first, convert to
  //a segment, then cast into the dynamic.
  bool mRefineRayCast;

  //A list of broad phases for each type of broad phase.
  IBroadPhase* mBroadPhases[BroadPhase::Size];
};

}//namespace Zero
