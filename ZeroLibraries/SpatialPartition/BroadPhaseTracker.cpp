///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseTracker.cpp
/// Implementation of the BroadPhaseTracker class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------- Statistics
#define StatsProfileScope(statistics, type) \
  ProfileScopeRecord(*statistics.GetRecord(type));

Statistics::Statistics()
{
  Array<ByteColor> colors;
  colors.PushBack(Color::Lime);          //Insertion
  colors.PushBack(Color::Red);           //Removal
  colors.PushBack(Color::Gold);          //Update
  colors.PushBack(Color::OrangeRed);     //Collision
  colors.PushBack(Color::LightSeaGreen); //Construction
  colors.PushBack(Color::SteelBlue);     //RayCast
  colors.PushBack(Color::Indigo);        //VolumeCast
  colors.PushBack(Color::SlateBlue);     //Cleanup

  mPossibleCollisionsReturned = 0;
  mActualCollisions = 0;
  mIterations = 0;

  // Create each record
  mRecords.Resize(BPStats::Size);
  for(uint i = 0; i < mRecords.Size(); ++i)
  {
    // Get the name of the record
    cstr name = BPStats::Names[i];
    uint color = colors[i];
    // Allocate the record
    Profile::Record* record = new Profile::Record(name, nullptr, color);
    record->Clear();
    // Add the record
    mRecords[i] = record;
  }
}

void Statistics::Reset()
{
  // Clear each record
  for(uint i = 0; i < BPStats::Size; ++i)
    mRecords[i]->Clear();
}

Profile::Record* Statistics::GetRecord(BPStats::Type recordType)
{
  return mRecords[recordType];
}

//----------------------------------------------------------------------- Handle
BroadPhaseHandle::BroadPhaseHandle(IBroadPhase* broadPhase)
{
  mBroadPhase = broadPhase;
}

BroadPhaseProxy& BroadPhaseHandle::GetProxy(uint index)
{
  return mProxies[index];
}

void BroadPhaseHandle::InvalidateProxy(uint index)
{
  mProxies[index] = BroadPhaseProxy(uint(-1));
}

void BroadPhaseHandle::ExpandProxies()
{
  mProxies.PushBack(BroadPhaseProxy(uint(-1)));
}

uint BroadPhaseHandle::GetProxyCount()
{
  return mProxies.Size();
}

//---------------------------------------------------------------------- Tracker
ZilchDefineType(BroadPhaseTracker, builder, type)
{
}

BroadPhaseTracker::BroadPhaseTracker()
{
  
}

BroadPhaseTracker::~BroadPhaseTracker()
{
  
}

///Initializes tracker data if we're tracking.
void BroadPhaseTracker::Initialize()
{

}

void BroadPhaseTracker::SaveToStream(Serializer& stream)
{
  stream.StartPolymorphic(ZilchVirtualTypeId(this));

  //Serialize flags
  SerializeName(mRefineRayCast);
  // REMOVE: Legacy
  bool mTracking = false;
  SerializeName(mTracking);

  // Serialize Dynamic Broad Phases
  BroadPhaseVec& broadPhases = mBroadPhases[BroadPhase::Dynamic];
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    IBroadPhase* broadPhase = broadPhases[i]->mBroadPhase;
    BoundType* type = ZilchVirtualTypeId(broadPhase);
    stream.StartPolymorphic(type);
    broadPhase->Serialize(stream);
    stream.EndPolymorphic();
  }

  // Serialize Static Broad Phases
  broadPhases = mBroadPhases[BroadPhase::Static];
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    IBroadPhase* broadPhase = broadPhases[i]->mBroadPhase;
    BoundType* type = ZilchVirtualTypeId(broadPhase);
    stream.StartPolymorphic(type);
    broadPhase->Serialize(stream);
    stream.EndPolymorphic();
  }

  // End the BroadPhase polymorphic node
  stream.EndPolymorphic();
}

void BroadPhaseTracker::LoadFromStream(Serializer& stream)
{
  PolymorphicNode headNode;
  stream.GetPolymorphic(headNode);

  //Serialize flags
  SerializeName(mRefineRayCast);
  // REMOVE: Legacy
  bool mTracking;
  SerializeName(mTracking);

  //Serialize each broad phase5
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

///Registers a broad phase to be updated.
void BroadPhaseTracker::AddBroadPhase(uint type, IBroadPhase* broadphase)
{
  // Create a new handle
  BroadPhaseHandle* handle = new BroadPhaseHandle(broadphase);
  // Add the handle
  mBroadPhases[type].PushBack(handle);
}

Statistics* BroadPhaseTracker::GetStatistics(uint type, uint index)
{
  return &mBroadPhases[type][index]->mStats;
}

///The results of collision detection should be reported through 
///this function.  Each index represents the lexicographical id of the 
///objects that collided.  This function will compare the results with 
///what each broad phase has recorded and report any discrepancies.
void BroadPhaseTracker::RecordFrameResults(const Array<NodePointerPair>& results)
{
  // Walk through each pair and record the collision
  for(uint i = 0; i < results.Size(); ++i)
    RecordCollision(results[i]);

  ClearFrameData();
}

/// Draws all broad phases (if they have something to draw).
/// Not every algorithm will use the level.
void BroadPhaseTracker::Draw(int level, uint debugFlags)
{

}

/// Fills out the proxy for the broadphase at the position specified by data.
/// The proxy can be thought of as a handle, an object should do nothing more
/// than hold onto it and give it to broadphase when it wants to do something.
void BroadPhaseTracker::CreateProxy(uint type, BroadPhaseProxy& proxy, 
                                    BroadPhaseData& data)
{
  // Get a new proxy
  uint proxyIndex = GetNewProxyIndex(type);

  // Set the proxy on the object
  proxy = BroadPhaseProxy(proxyIndex);

  BroadPhaseVec& broadPhases = mBroadPhases[type];

  // Create a proxy for each 
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;    
    
    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::Insertion);
      broadPhase->CreateProxy(handle.GetProxy(proxyIndex), data);
    }
  }
}

/// Batch version of CreateProxy.
void BroadPhaseTracker::CreateProxies(uint type, BroadPhaseObjectArray& objects)
{
  // Create a proxy for each object
  BroadPhaseObjectArray::range r = objects.All();
  for(; !r.Empty(); r.PopFront())
  {
    BroadPhaseObject& obj = r.Front();

    // Create the proxy
    CreateProxy(type, *obj.mProxy, obj.mData);
  }
}

/// Removes the given proxy.
void BroadPhaseTracker::RemoveProxy(uint type, BroadPhaseProxy& proxy)
{
  // Get the index from the proxy
  uint index = proxy.ToU32();

  // Add the index to the free list
  mProxyFreeIndices[type].PushBack(index);

  BroadPhaseVec& broadPhases = mBroadPhases[type];

  // Create a proxy for each 
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;    

    BroadPhaseProxy& internalProxy = handle.GetProxy(index);

    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::Removal);
      broadPhase->RemoveProxy(internalProxy);
    }
    // Invalidate the proxy in the array for later use
    handle.InvalidateProxy(index);
  }
}

/// Batch version of RemoveProxy.
void BroadPhaseTracker::RemoveProxies(uint type, ProxyHandleArray& proxies)
{
  // Remove each proxy
  ProxyHandleArray::range r = proxies.All();
  for(; !r.Empty(); r.PopFront())
  {
    BroadPhaseProxy* proxy = r.Front();

    // Remove the proxy
    RemoveProxy(type, *proxy);
  }
}

/// Updates the given proxy to the position specified by data.
void BroadPhaseTracker::UpdateProxy(uint type, BroadPhaseProxy& proxy, 
                                    BroadPhaseData& data)
{
  // Get the index from the proxy
  uint index = proxy.ToU32();

  BroadPhaseVec& broadPhases = mBroadPhases[type];

  // Update each proxy in each broad phase
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;    

    BroadPhaseProxy& internalProxy = handle.GetProxy(index);

    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::Update);
      broadPhase->UpdateProxy(internalProxy, data);
    }
  }
}

/// Batch version of UpdateProxy.
void BroadPhaseTracker::UpdateProxies(uint type, BroadPhaseObjectArray& objects)
{
  // Update each object
  BroadPhaseObjectArray::range r = objects.All();
  for(; !r.Empty(); r.PopFront())
  {
    BroadPhaseObject& obj = r.Front();

    // Update the object
    UpdateProxy(type, *obj.mProxy, obj.mData);
  }
}

/// Used to determine intersection of objects in this broadphase with other
/// objects in the same broadphase. Mainly a physics things.
void BroadPhaseTracker::SelfQuery(ClientPairArray& results)
{
  BroadPhaseVec& broadPhases = mBroadPhases[BroadPhase::Dynamic];

  // Update each proxy in each broad phase
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;

    // Results for this current broad phase
    ClientPairArray currentResults;

    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::Collision);
      broadPhase->SelfQuery(currentResults);
    }

    // Register the collisions.  This is so that we know which broad phases
    // said which objects should be checked for collision.
    RegisterCollisions(BroadPhase::Dynamic, i, currentResults, results);
  }
}

void BroadPhaseTracker::Query(BroadPhaseData& data, ClientPairArray& results, uint broadphaseType)
{
  BroadPhaseVec& broadPhases = mBroadPhases[broadphaseType];

  // Update each proxy in each broad phase
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;    

    // Results for this current broad phase
    ClientPairArray currentResults;

    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::Collision);
      broadPhase->Query(data, currentResults);
    }

    // Register the collisions.  This is so that we know which broad phases
    // said which objects should be checked for collision.
    RegisterCollisions(broadphaseType, i, currentResults, results);
  }
}

/// Finds everything that is in contact with the data.
void BroadPhaseTracker::Query(BroadPhaseData& data, ClientPairArray& results)
{
  Query(data,results,BroadPhase::Static);
}

/// Batch version of Query.
void BroadPhaseTracker::BatchQuery(BroadPhaseDataArray& data, 
                                   ClientPairArray& results)
{
  for(uint i = 0; i < data.Size(); ++i)
    Query(data[i], results);
}

void BroadPhaseTracker::QueryBoth(BroadPhaseData& data, ClientPairArray& results)
{
  Query(data,results,BroadPhase::Static);
  Query(data,results,BroadPhase::Dynamic);
}

/// Tells the structure that it has all of the data it will ever have. Used 
/// mainly for static BroadPhases.
void BroadPhaseTracker::Construct()
{
  // For all types of broad phases
  for(uint bpType = 0; bpType < BroadPhase::Size; ++bpType)
  {
    BroadPhaseVec& broadPhases = mBroadPhases[bpType];

    // Construct each broad phase
    for(uint i = 0; i < broadPhases.Size(); ++i)
    {
      BroadPhaseHandle& handle = *broadPhases[i];
      IBroadPhase* broadPhase = handle.mBroadPhase;
      Statistics& stats = handle.mStats;

      { // Scoped for profiler
        StatsProfileScope(stats, BPStats::Construction);
        broadPhase->Construct();
      }
    }
  }
}

/// Computes all the collision pairs of objects already in the BroadPhase
void BroadPhaseTracker::RegisterCollisions()
{
  // Only Dynamic broad phases should need to register collisions
  BroadPhaseVec& broadPhases = mBroadPhases[BroadPhase::Dynamic];

  // Register collisions on each broad phase
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;

    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::Collision);
      broadPhase->RegisterCollisions();
    }
  }
}

/// Resets the BroadPhase after each update loop.
void BroadPhaseTracker::Cleanup()
{
  // For all types of broad phases
  for(uint bpType = 0; bpType < BroadPhase::Size; ++bpType)
  {
    BroadPhaseVec& broadPhases = mBroadPhases[bpType];

    // Cleanup each broad phase
    for(uint i = 0; i < broadPhases.Size(); ++i)
    {
      BroadPhaseHandle& handle = *broadPhases[i];
      IBroadPhase* broadPhase = handle.mBroadPhase;
      Statistics& stats = handle.mStats;

      { // Scoped for profiler
        StatsProfileScope(stats, BPStats::Cleanup);
        broadPhase->Construct();
      }
    }
  }
}

void BroadPhaseTracker::CastIntoBroadphase(uint broadPhaseType, 
            CastDataParam data, ProxyCastResults& results, CastFunction func)
{
  BroadPhaseVec& broadPhases = mBroadPhases[broadPhaseType];

  // Cleanup each broad phase
  for(uint i = 0; i < broadPhases.Size(); ++i)
  {
    BroadPhaseHandle& handle = *broadPhases[i];
    IBroadPhase* broadPhase = handle.mBroadPhase;
    Statistics& stats = handle.mStats;

    ProxyCastResultArray currentResultsArray;
    currentResultsArray.Resize(results.GetProxyCount());
    ProxyCastResults currentResults(currentResultsArray, results.Filter);

    { // Scoped for profiler
      StatsProfileScope(stats, BPStats::RayCast);
      (broadPhase->*func)(data, currentResults);
    }

    // Record the results
    if(i == 0)
      results.Merge(currentResults);
    else
    {
      // Compare with the first results
      if(!results.Match(currentResults))
      {
        // If they do not match, there was an error in one of them
        ErrorIf(true, "Ray cast results were bad");
        IBroadPhase* lastBroadPhase = broadPhases[i - 1]->mBroadPhase;
        cstr lastName = ZilchVirtualTypeId(lastBroadPhase)->Name.c_str();
        cstr currName = ZilchVirtualTypeId(broadPhase)->Name.c_str();
        String message = BuildString(lastName, " and ", currName, 
                               " did not have the same results on a ray cast.");
        DoNotifyTimer("Missed Ray Cast!", message, "Warning", 0.5);
      }
    }
  }
}

uint BroadPhaseTracker::GetNewProxyIndex(uint type)
{
  IndexArray& freeList = mProxyFreeIndices[type];

  // If there were no free indices already, add a new item to the back
  // and return that index
  if(freeList.Empty())
  {
    // We need to add room in each broad phase
    BroadPhaseVec& broadPhases = mBroadPhases[type];
    for(uint i = 0; i < broadPhases.Size(); ++i)
      broadPhases[i]->ExpandProxies();

    // Return the index of the back proxy that we just made room for
    return broadPhases[0]->GetProxyCount() - 1;
  }

  // Otherwise, just a index in free indices
  uint index = freeList.Back();
  freeList.PopBack();
  return index;
}

void BroadPhaseTracker::RegisterCollisions(uint type, uint broadPhaseId, 
                 ClientPairArray& currentResults, ClientPairArray& finalResults)
{
  // Record the amount of collisions being registered
  mBroadPhases[type][broadPhaseId]->mStats.mPossibleCollisionsReturned += currentResults.Size();

  // Walk each collision and register it with this broad phase
  for(uint i = 0; i < currentResults.Size(); ++i)
  {
    // The pair Id of objects to be checked
    ClientPair& pair = currentResults[i];
    NodePointerPair nodePair(pair.mClientData[0], pair.mClientData[1]);

    // If it was a new pair, add it to the final results
    if(RegisterCollision(type, broadPhaseId, nodePair))
      finalResults.PushBack(pair);
  }
}

bool BroadPhaseTracker::RegisterCollision(uint type, BroadPhaseId id, 
                                          NodePointerPair pair)
{
  //Lookup to see if the pair has already been inserted.
  CollisionMap::range r = mRegisteredCollisions[type].Find(pair);

  if(r.Empty())
  {
    //If the pair was not registered, so we need to Insert it.
    mRegisteredCollisions[type].Insert(pair, 1 << id);
    return true;
  }

  //The pair was already registered, so we must grab the bit field and set
  //the bit for the current broad phase, indicating that it did "catch"
  //this collision.
  u8& bitField = r.Front().second;
  bitField |= (1 << id);
  return false;
}

void BroadPhaseTracker::RecordCollision(NodePointerPair pair)
{
  // We first need to figure out which broad phase the pair came from
  // Try the dynamic broad phase first
  uint bpType = BroadPhase::Dynamic;
  CollisionMap::range r = mRegisteredCollisions[bpType].Find(pair);
  // If it's empty, try the static broad phase
  if(r.Empty())
  {
    bpType = BroadPhase::Static;
    r = mRegisteredCollisions[bpType].Find(pair);
  }

  // If we still have nothing, there was a collision returned that the 
  // broad phase never gave to the narrow phase
  ErrorIf(r.Empty(), "Narrow Phase returned a collision that Broad Phase did "
                     "not send to Narrow Phase.");
  
  //This bit field describes which broad phases reported it to be 
  //checked for collision
  u8 bitField = r.Front().second;

  uint count = mBroadPhases[bpType].Size();

  // All the bits to the right of count should be set.  If one is not set,
  // that means one broad phase did not return the pair as a possible
  // collision.
  if(bitField != (1 << count) - 1)
  {
    // Report the missed collision
    ReportMissedCollision(bpType, pair, bitField);
  }

  BroadPhaseVec::range broadPhaseRange = mBroadPhases[bpType].All();

  //We need to mark the collisions in the statistics field
  for(uint i = 0; !broadPhaseRange.Empty(); ++i, broadPhaseRange.PopFront())
  {
    //Continue if the bit wasn't set for this broad phase.  
    //A report should have already been sent about the miss.
    if(!(bitField & (1 << i)))
      continue;

    //Increment the actual collisions.
    BroadPhaseHandle* handle = broadPhaseRange.Front();
    ++handle->mStats.mActualCollisions;
  }
}

void BroadPhaseTracker::ClearFrameData()
{
  mRegisteredCollisions[BroadPhase::Dynamic].Clear();
  mRegisteredCollisions[BroadPhase::Static].Clear();
}

void BroadPhaseTracker::ReportMissedCollision(uint bpType, NodePointerPair pair, 
                                              char bitField)
{
  BroadPhaseVec::range r = mBroadPhases[bpType].All();

  // Find the broad phase that missed it
  for(uint i = 0; !r.Empty(); ++i)
  {
    // If the bit isn't set for the current broad phase, we found one
    if(!(bitField & (1 << i)))
    {
      // Grab the handle
      BroadPhaseHandle* handle = r.Front();
      // Increment the collisions missed
      ++handle->mStats.mCollisionsMissed;

      //Throw an error
      cstr name = ZilchVirtualTypeId(handle->mBroadPhase)->Name.c_str();
      String message = BuildString(name, " did not catch a collision.");
      ErrorIf(true, "%s", message.c_str());
      DoNotifyTimer("Missed Collision!", message, "Warning", 0.5);
    }
    r.PopFront();
  }
}

}//namespace Zero
