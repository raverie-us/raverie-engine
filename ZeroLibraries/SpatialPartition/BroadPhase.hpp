///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhase.hpp
/// Declaration of the RayCastResults and IBroadPhase class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Serializer;

DeclareEnum2(BroadPhase, Dynamic, Static);
#define DynamicBit 1 << BroadPhase::Dynamic
#define StaticBit 1 << BroadPhase::Static

///An abstract interface for a BroadPhase class. Any BroadPhase to be used
///in the physics engine should derive and implement these functions. Depending
///on the implementation, some of these methods may be empty.
class IBroadPhase : public IZilchObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  IBroadPhase();
  virtual ~IBroadPhase(){}
  void SetHeap(Memory::Heap* heap);

  ///Component Serialization Interface see Serialization.h for details.
  virtual void Serialize(Serializer& stream);

  ///Draws the broad phase (if supported by the type of broad phase).
  virtual void Draw(int level, uint debugDrawFlags){}

  ///Fills out the proxy for the BroadPhase at the position specified by data.
  ///The proxy can be thought of as a handle, an object should do nothing more
  ///than hold onto it and give it to BroadPhase when it wants to do something.
  virtual void CreateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data);
  ///Batch version of CreateProxy.
  virtual void CreateProxies(BroadPhaseObjectArray& objects);
  ///Removes the given proxy.
  virtual void RemoveProxy(BroadPhaseProxy& proxy);
  ///Batch version of RemoveProxy.
  virtual void RemoveProxies(ProxyHandleArray& proxies);
  ///Updates the given proxy to the position specified by data.
  virtual void UpdateProxy(BroadPhaseProxy& proxy, BroadPhaseData& data);
  ///Batch version of UpdateProxy.
  virtual void UpdateProxies(BroadPhaseObjectArray& objects);

  ///Used to determine intersection of objects in this BroadPhase with other
  ///objects in the same BroadPhase. Mainly a physics things.
  virtual void SelfQuery(ClientPairArray& results);
  ///Finds everything that is in contact with the data. Used primarily for
  ///querying a static BroadPhase with objects from the dynamic BroadPhase.
  ///The data passed in is not inserted into this BroadPhase.
  virtual void Query(BroadPhaseData& data, ClientPairArray& results);
  ///Batch version of Query.
  virtual void BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results);

  ///Tells the structure that it has all of the data it will ever have. Used 
  ///mainly for static BroadPhases.
  virtual void Construct();

  ///Determines where and when a ray hits what object(s).
  virtual void CastRay(CastDataParam data, ProxyCastResults& results);
  ///Determines where and when a segment hits what object(s).
  virtual void CastSegment(CastDataParam data, ProxyCastResults& results);
  ///Returns objects in the given Aabb.
  virtual void CastAabb(CastDataParam data, ProxyCastResults& results);
  ///Returns objects in the given Sphere.
  virtual void CastSphere(CastDataParam data, ProxyCastResults& results);
  ///Returns objects in the given Frustum.
  virtual void CastFrustum(CastDataParam data, ProxyCastResults& results);

  ///Computes all the collision pairs of objects already in the BroadPhase.
  virtual void RegisterCollisions();

  ///Resets the BroadPhase after each update loop.
  virtual void Cleanup();

  ///Setter for the BroadPhase's Aabb
  void SetAabb(const Aabb& aabb){mWorldAabb = aabb;}
  ///Getter for the BroadPhase's Aabb
  Aabb GetAabb() {return mWorldAabb;}

  typedef bool (*RayCastCallBack)(void*, CastDataParam, ProxyResult&, BaseCastFilter&);
  typedef bool (*VolumeCastCallBack)(void*, CastDataParam, ProxyResult&, BaseCastFilter&);

  ///Sets the ray cast callback for checking if a ray collided with an object.
  static void SetCastRayCallBack(RayCastCallBack callback){mCastRayCallBack = callback;}
  static void SetCastSegmentCallBack(RayCastCallBack callback){mCastSegmentCallBack = callback;}
  static void SetCastAabbCallBack(VolumeCastCallBack callback){mCastAabbCallBack = callback;}
  static void SetCastSphereCallBack(VolumeCastCallBack callback){mCastSphereCallBack = callback;}
  static void SetCastFrustumCallBack(VolumeCastCallBack callback){mCastFrustumCallBack = callback;}

  uint GetType();
  
  ///Tests Ray against Aabb.
  static bool TestRayVsAabb(const Aabb& aabb, Vec3Param start, 
                            Vec3Param direction, real& time);
  ///Tests Segment against Aabb.
  static bool TestSegmentVsAabb(const Aabb& aabb, Vec3Param start, 
                                Vec3Param end, real& time);
  ///Tests Ray against Sphere.
  static bool TestRayVsSphere(const Sphere& sphere, Vec3Param start, 
                              Vec3Param direction, real& time);
  ///Tests Segment against Sphere.
  static bool TestSegmentVsSphere(const Sphere& sphere, Vec3Param start, 
                                  Vec3Param end, real& time);

protected:
  ///Callback for checking ray collisions.
  static RayCastCallBack mCastRayCallBack;
  static RayCastCallBack mCastSegmentCallBack;
  static VolumeCastCallBack mCastAabbCallBack;
  static VolumeCastCallBack mCastSphereCallBack;
  static VolumeCastCallBack mCastFrustumCallBack;

  /// The entire worlds Aabb.  Not currently being used.
  Aabb mWorldAabb;

  /// The current type of broad phase it's being used as (Static, Dynamic).
  uint mType;

  Memory::Heap* mHeap;
};

}//namespace Zero
