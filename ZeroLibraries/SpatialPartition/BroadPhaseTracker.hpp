///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseTracker.hpp
/// Declaration of the BroadPhaseTracker class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum8(BPStats, Insertion, Removal, Update, Collision,
                      Construction, RayCast, VolumeCast, Cleanup);

/// The statistics for a single broad phases entire life.
class Statistics
{
public:
  Statistics();

  /// Reset all timers.
  void Reset();
  /// Returns the record of the given type
  Profile::Record* GetRecord(BPStats::Type recordType);

  /// The name of the broad phase
  String mName;
  /// The broad phase type, used to help determine what is a more optimal
  /// broad phase for certain situations.
  uint mType;

  /// The amount of pairs of objects returned to be checked by the narrow phase.
  uint mPossibleCollisionsReturned;
  /// The amount of pairs returned that actually collided.
  uint mActualCollisions;
  /// The amount of collisions missed.
  uint mCollisionsMissed;

  /// Iterations
  uint mIterations;

  /// Points to each record for easier access.
  Array<Profile::Record*> mRecords;
};

class BroadPhaseHandle
{
public:
  BroadPhaseHandle(IBroadPhase* broadPhase);
  IBroadPhase* mBroadPhase;
  Statistics mStats;

  BroadPhaseProxy& GetProxy(uint index);
  void InvalidateProxy(uint index);
  void ExpandProxies();
  uint GetProxyCount();

private:
  Array<BroadPhaseProxy> mProxies;
};

typedef uint BroadPhaseId;

/// Records data from all broad phases and checks for discrepancies.
class BroadPhaseTracker : public BroadPhasePackage
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef Array<BroadPhaseHandle*> BroadPhaseVec;
  BroadPhaseTracker();
  ~BroadPhaseTracker();

  /// Initializes tracker data if we're tracking.
  virtual void Initialize();
  virtual void SaveToStream(Serializer& stream);
  virtual void LoadFromStream(Serializer& stream);

  /// Registers a broad phase to be updated.
  virtual void AddBroadPhase(uint type, IBroadPhase* broadphase);

  Statistics* GetStatistics(uint type, uint index);

  /// The results of collision detection should be reported through 
  /// this function.  Each index represents the lexicographical id of the 
  /// objects that collided.  This function will compare the results with 
  /// what each broad phase has recorded and report any discrepancies.
  virtual void RecordFrameResults(const Array<NodePointerPair>& results);

  virtual bool IsTracking(){return true;}
public:
  /// Draws all broad phases (if they have something to draw).
  /// Not every algorithm will use the level.
  virtual void Draw(int level, uint debugFlags);

  /// Fills out the proxy for the broadphase at the position specified by data.
  /// The proxy can be thought of as a handle, an object should do nothing more
  /// than hold onto it and give it to broadphase when it wants to do something.
  virtual void CreateProxy(uint type, BroadPhaseProxy& proxy, 
                           BroadPhaseData& data);
  /// Batch version of CreateProxy.
  virtual void CreateProxies(uint type, BroadPhaseObjectArray& objects);
  /// Removes the given proxy.
  virtual void RemoveProxy(uint type, BroadPhaseProxy& proxy);
  /// Batch version of RemoveProxy.
  virtual void RemoveProxies(uint type, ProxyHandleArray& proxies);
  /// Updates the given proxy to the position specified by data.
  virtual void UpdateProxy(uint type, BroadPhaseProxy& proxy, 
                           BroadPhaseData& data);
  /// Batch version of UpdateProxy.
  virtual void UpdateProxies(uint type, BroadPhaseObjectArray& objects);

  /// Used to determine intersection of objects in this broadphase with other
  /// objects in the same broadphase. Mainly a physics things.
  void SelfQuery(ClientPairArray& results) override;
  /// Internal function or query. Finds all overlaps in the broadphase of the given type.
  void Query(BroadPhaseData& data, ClientPairArray& results, uint broadphaseType);
  /// Finds everything that is in contact with the data.
  void Query(BroadPhaseData& data, ClientPairArray& results) override;
  /// Batch version of Query.
  void BatchQuery(BroadPhaseDataArray& data, ClientPairArray& results) override;
  /// Queries both the dynamic and static broadphase.
  void QueryBoth(BroadPhaseData& data, ClientPairArray& results) override;

  /// Tells the structure that it has all of the data it will ever have. Used 
  /// mainly for static BroadPhases.
  virtual void Construct();

  /// Computes all the collision pairs of objects already in the BroadPhase
  virtual void RegisterCollisions();

  /// Resets the BroadPhase after each update loop.
  virtual void Cleanup();

private:
  virtual void CastIntoBroadphase(uint broadPhaseType, CastDataParam data, 
                               ProxyCastResults& results, CastFunction func);

  uint GetNewProxyIndex(uint type);

  void RegisterCollisions(uint type, uint broadPhaseId, 
                ClientPairArray& currentResults, ClientPairArray& finalResults);

  /// Tells the tracker that there was a pair of objects sent to be 
  /// checked for collision.  Returns whether or not the object was added.
  /// True if it was a new pairId, false if it was already entered in.
  bool RegisterCollision(uint type, BroadPhaseId id, NodePointerPair pair);

  /// Checks to see if the passed in confirmed collision was caught by all
  /// broad phases.
  void RecordCollision(NodePointerPair pair);

  /// Cleans up all frame data to get ready for the next frame.
  void ClearFrameData();

private:
  void ReportMissedCollision(uint bpType, NodePointerPair pair, char bitField);

  /// Hashed by a lexicographic id between two Colliders.  The char represents
  /// a bit field (8 bits), each bit corresponding to a unique broad phase.
  /// Used to determine which, if any, broad phases missed a collision.
  typedef HashMap<NodePointerPair, u8> CollisionMap;
  CollisionMap mRegisteredCollisions[BroadPhase::Size];

  /// All the broad phases currently being tracked.
  BroadPhaseVec mBroadPhases[BroadPhase::Size];

  typedef Array<uint> IndexArray;
  IndexArray mProxyFreeIndices[BroadPhase::Size];
};

}//namespace Zero
