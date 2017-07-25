///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                 Replicator                                      //
//---------------------------------------------------------------------------------//

void Replicator::ResetSession()
{
  mRole = Role::Unspecified;
  mReplicatorId = 0;
  mReplicatorIdStore.Reset();
  mReplicaIdStore.Reset();
  mEmplaceIdStores.Clear();
  mReplicaSet.Clear();
  mCreateMap.Clear();
  mReplicaMap.Clear();
  mEmplaceMap.Clear();
  mCreateContextCacher.Reset();
  mReplicaTypeCacher.Reset();
  mEmplaceContextCacher.Reset();
  // mUserData = nullptr;
  // mFrameFillWarning = 0;
  // mFrameFillSkip = 0;
  // mReplicaChannelTypes.Clear();
  // mReplicaPropertyTypes.Clear();
}

Replicator::Replicator(Role::Enum role)
  : PeerPlugin(),
    mCreateContextCacher(this, ReplicatorMessageType::CreateContextItems),
    mReplicaTypeCacher(this, ReplicatorMessageType::ReplicaTypeItems),
    mEmplaceContextCacher(this, ReplicatorMessageType::EmplaceContextItems)
{
  ResetSession();
  ResetConfig();
  SetRole(role);
}

//
// Operations
//

void Replicator::SetRole(Role::Enum role)
{
  // Already initialized?
  if(IsInitialized())
  {
    Assert(false);
    return;
  }

  mRole = role;
}
Role::Enum Replicator::GetRole() const
{
  return mRole;
}

ReplicatorId Replicator::GetReplicatorId() const
{
  return mReplicatorId;
}

bool Replicator::HasLink(const IpAddress& ipAddress) const
{
  return GetLink(ipAddress) ? true : false;
}
bool Replicator::HasLink(ReplicatorId replicatorId) const
{
  return GetLink(replicatorId) ? true : false;
}
bool Replicator::HasLinks(const Route& route) const
{
  return !GetLinks(route).Empty();
}
bool Replicator::HasLinks() const
{
  return !GetLinks().Empty();
}

PeerLink* Replicator::GetLink(const IpAddress& ipAddress) const
{
  // Get link with the specified IP address
  PeerLink* link = GetPeer()->GetLink(ipAddress);

  // Is a connected replicator link?
  if(link && link->GetStatus() == LinkStatus::Connected && link->GetPlugin<ReplicatorLink>("ReplicatorLink"))
    return link; // Success

  // Failure
  return nullptr;
}
PeerLink* Replicator::GetLink(ReplicatorId replicatorId) const
{
  // For all links
  PeerLinkSet links = GetPeer()->GetLinks();
  forRange(PeerLink* link, links.All())
  {
    // Not connected?
    if(link->GetStatus() != LinkStatus::Connected)
      continue; // Skip

    // Is a replicator link and has the specified replicator ID?
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");
    if(replicatorLink && replicatorLink->GetReplicatorId() == replicatorId)
      return link; // Success
  }

  // Failure
  return nullptr;
}
PeerLinkSet Replicator::GetLinks(const Route& route) const
{
  // Is route all?
  if(&route == &Route::All)
    return GetLinks();
  // Is route none?
  else if(&route == &Route::None)
    return PeerLinkSet();

  // Otherwise route is custom
  PeerLinkSet result;
  result.Reserve(GetPeer()->GetLinkCount());

  // Add links according to route mode
  PeerLinkSet links = GetPeer()->GetLinks();
  switch(route.mMode)
  {
  case RouteMode::Exclude:
    // For all links
    forRange(PeerLink* link, links.All())
    {
      // Not connected?
      if(link->GetStatus() != LinkStatus::Connected)
        continue; // Skip

      // Is a replicator link and is not listed as a route target?
      ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");
      if(replicatorLink && !route.mTargets.Contains(replicatorLink->GetReplicatorId()))
        result.Insert(link); // Link is a part of this route
    }
    break;

  case RouteMode::Include:
    // For all links
    forRange(PeerLink* link, links.All())
    {
      // Not connected?
      if(link->GetStatus() != LinkStatus::Connected)
        continue; // Skip

      // Is a replicator link and is listed as a route target?
      ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");
      if(replicatorLink && route.mTargets.Contains(replicatorLink->GetReplicatorId()))
        result.Insert(link); // Link is a part of this route
    }
    break;

  default:
    Assert(false);
    break;
  }

  return result;
}
PeerLinkSet Replicator::GetLinks() const
{
  PeerLinkSet result;
  result.Reserve(GetPeer()->GetLinkCount());

  // For all links
  PeerLinkSet links = GetPeer()->GetLinks();
  forRange(PeerLink* link, links.All())
  {
    // Not connected?
    if(link->GetStatus() != LinkStatus::Connected)
      continue; // Skip

    // Is a replicator link?
    if(link->GetPlugin<ReplicatorLink>("ReplicatorLink"))
      result.Insert(link);
  }

  return result;
}

void Replicator::Send(Status& status, const Message& message, const Route& route)
{
  bool result = false;

  // For all replicator links in route
  PeerLinkSet links = GetLinks(route);
  forRange(PeerLink* link, links.All())
  {
    // Send user message
    Status linkSendStatus;
    link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(linkSendStatus, message);
    if(linkSendStatus.Succeeded())
      result = true;
    else
      status.SetFailed(linkSendStatus.Message);
  }

  // At least one send succeeded?
  if(result)
    status.SetSucceeded();
}

void Replicator::SetUserData(void* userData)
{
  mUserData = userData;
}
void* Replicator::GetUserData() const
{
  return mUserData;
}

//
// Replica Management
//

bool Replicator::HasReplica(ReplicaId replicaId) const
{
  // Has replica in set?
  return mReplicaSet.Contains(replicaId);
}
bool Replicator::HasReplica(Replica* replica) const
{
  // Has replica in set?
  return mReplicaSet.Contains(replica);
}
bool Replicator::HasReplicasByCreateContext(const CreateContext& createContext) const
{
  // Has replicas in create context set?
  CreateMap::const_iterator iter = mCreateMap.FindIterator(createContext);
  if(iter != mCreateMap.End()) // Found?
  {
    Assert(!iter->second.Empty());
    return true;
  }

  return false;
}
bool Replicator::HasReplicasByReplicaType(const ReplicaType& replicaType) const
{
  // Has replicas in replica type set?
  ReplicaMap::const_iterator iter = mReplicaMap.FindIterator(replicaType);
  if(iter != mReplicaMap.End()) // Found?
  {
    Assert(!iter->second.Empty());
    return true;
  }

  return false;
}
bool Replicator::HasReplicasByEmplaceContext(const EmplaceContext& emplaceContext) const
{
  // Has replicas in emplace context set?
  EmplaceMap::const_iterator iter = mEmplaceMap.FindIterator(emplaceContext);
  if(iter != mEmplaceMap.End()) // Found?
  {
    Assert(!iter->second.Empty());
    return true;
  }

  return false;
}
bool Replicator::HasReplicaByEmplaceContext(const EmplaceContext& emplaceContext, EmplaceId emplaceId) const
{
  return GetReplicaByEmplaceContext(emplaceContext, emplaceId) ? true : false;
}
bool Replicator::HasReplicas() const
{
  // Has replicas in set?
  return !mReplicaSet.Empty();
}

Replica* Replicator::GetReplica(ReplicaId replicaId) const
{
  // Find replica in set
  ReplicaSet::const_iterator iter = mReplicaSet.FindIterator(replicaId);
  if(iter != mReplicaSet.End()) // Found?
    return *iter;

  return nullptr;
}
Replica* Replicator::GetReplica(Replica* replica) const
{
  // Find replica in set
  ReplicaSet::const_iterator iter = mReplicaSet.FindIterator(replica);
  if(iter != mReplicaSet.End()) // Found?
    return *iter;

  return nullptr;
}
ReplicaSet Replicator::GetReplicasByCreateContext(const CreateContext& createContext) const
{
  // Find replicas in create context set
  CreateMap::const_iterator iter = mCreateMap.FindIterator(createContext);
  if(iter != mCreateMap.End()) // Found?
    return iter->second;

  return ReplicaSet();
}
ReplicaSet Replicator::GetReplicasByReplicaType(const ReplicaType& replicaType) const
{
  // Find replicas in replica type set
  ReplicaMap::const_iterator iter = mReplicaMap.FindIterator(replicaType);
  if(iter != mReplicaMap.End()) // Found?
    return iter->second;

  return ReplicaSet();
}
ReplicaSet Replicator::GetReplicasByEmplaceContext(const EmplaceContext& emplaceContext) const
{
  // Find replicas in emplace context set
  EmplaceMap::const_iterator iter = mEmplaceMap.FindIterator(emplaceContext);
  if(iter != mEmplaceMap.End()) // Found?
    return iter->second;

  return ReplicaSet();
}
Replica* Replicator::GetReplicaByEmplaceContext(const EmplaceContext& emplaceContext, EmplaceId emplaceId) const
{
  // Find replicas in emplace context set
  EmplaceMap::const_iterator iter = mEmplaceMap.FindIterator(emplaceContext);
  if(iter != mEmplaceMap.End()) // Found?
  {
    // Find replica
    const ReplicaSet& contextSet = iter->second;
    forRange(Replica* replica, contextSet.All())
      if(replica->GetEmplaceId() == emplaceId) // Found?
        return replica;
  }

  return nullptr;
}
const ReplicaSet& Replicator::GetReplicas() const
{
  return mReplicaSet;
}

size_t Replicator::GetReplicaCountByCreateContext(const CreateContext& createContext) const
{
  // Count replicas in create context set
  CreateMap::const_iterator iter = mCreateMap.FindIterator(createContext);
  if(iter != mCreateMap.End()) // Found?
    return iter->second.Size();

  return 0;
}
size_t Replicator::GetReplicaCountByReplicaType(const ReplicaType& replicaType) const
{
  // Count replicas in replica type set
  ReplicaMap::const_iterator iter = mReplicaMap.FindIterator(replicaType);
  if(iter != mReplicaMap.End()) // Found?
    return iter->second.Size();

  return 0;
}
size_t Replicator::GetReplicaCountByEmplaceContext(const EmplaceContext& emplaceContext) const
{
  // Count replicas in emplace context set
  EmplaceMap::const_iterator iter = mEmplaceMap.FindIterator(emplaceContext);
  if(iter != mEmplaceMap.End()) // Found?
    return iter->second.Size();

  return 0;
}
size_t Replicator::GetReplicaCount() const
{
  return mReplicaSet.Size();
}

//
// Replica Commands
//

bool Replicator::EmplaceReplica(Replica* replica, const EmplaceContext& emplaceContext)
{
  // Emplace single replica
  ReplicaArray replicas(ZeroInit, replica);
  return EmplaceReplicas(replicas, emplaceContext) != 0;
}
bool Replicator::EmplaceReplicas(const ReplicaArray& replicas, const EmplaceContext& emplaceContext)
{
  // (All replicas should be invalid)
  // (Hitting an assert here usually means an incorrect assumption was made in the calling logic about a replica's state)
  AssertReplicas(replicas, replica->IsInvalid());
  AssertReplicas(replicas, replica->GetInitializationTimestamp()   == cInvalidMessageTimestamp);
  AssertReplicas(replicas, replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

  // Create timestamp
  TimeMs timestamp = GetPeer()->GetLocalTime();

  // Handle replica emplace
  if(!HandleEmplace(replicas, emplaceContext, timestamp)) // Unable?
    return false;

  // (All replicas should be valid (if client) or live (if server))
  AssertReplicas(replicas, GetRole() == Role::Client ? replica->IsValid() : replica->IsLive());

  // Success
  return true;
}

bool Replicator::SpawnReplica(Replica* replica, const Route& route)
{
  // Spawn single replica
  ReplicaArray replicas(ZeroInit, replica);
  return SpawnReplicas(replicas, route) != 0;
}
bool Replicator::SpawnReplicas(const ReplicaArray& replicas, const Route& route)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should be invalid)
  // (Hitting an assert here usually means an incorrect assumption was made in the calling logic about a replica's state)
  AssertReplicas(replicas, replica->IsInvalid());
  AssertReplicas(replicas, replica->GetInitializationTimestamp()   == cInvalidMessageTimestamp);
  AssertReplicas(replicas, replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

  // Create timestamp
  TimeMs timestamp = GetPeer()->GetLocalTime();

  // Handle replica spawn
  if(!HandleSpawn(replicas, TransmissionDirection::Outgoing, timestamp)) // Unable?
    return false;

  // Route replica spawn
  RouteSpawn(replicas, route, timestamp);

  // (All replicas should be live)
  AssertReplicas(replicas, replica->IsLive());

  // Success
  return true;
}

bool Replicator::CloneReplica(Replica* replica, const Route& route)
{
  // Clone single replica
  ReplicaArray replicas(ZeroInit, replica);
  return CloneReplicas(replicas, route) != 0;
}
bool Replicator::CloneReplicas(const ReplicaArray& replicas, const Route& route)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should be live)
  // (Hitting an assert here usually means an incorrect assumption was made in the calling logic about a replica's state)
  AssertReplicas(replicas, replica->IsLive());
  AssertReplicas(replicas, replica->GetInitializationTimestamp()   != cInvalidMessageTimestamp);
  AssertReplicas(replicas, replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

  // Get timestamp from replicas original initialization time
  TimeMs timestamp = GetInitializationTimestamp(replicas);

  // Route replica clone
  RouteClone(replicas, route, timestamp);

  // Success
  return true;
}
bool Replicator::CloneAllReplicas(const Route& route)
{
  // Clone all replicas
  ReplicaArray replicas = GetReplicas();
  return CloneReplicas(replicas, route);
}

bool Replicator::ForgetReplica(Replica* replica, const Route& route)
{
  // Forget single replica
  ReplicaArray replicas(ZeroInit, replica);
  return ForgetReplicas(replicas, route) != 0;
}
bool Replicator::ForgetReplicas(const ReplicaArray& replicas, const Route& route)
{
  // (All replicas should be valid (if client) or live (if server))
  // (Hitting an assert here usually means an incorrect assumption was made in the calling logic about a replica's state)
  AssertReplicas(replicas, GetRole() == Role::Client ? replica->IsValid() : replica->IsLive());
  AssertReplicas(replicas, GetRole() == Role::Client ? true : replica->GetInitializationTimestamp()   != cInvalidMessageTimestamp);
  AssertReplicas(replicas,                                    replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

  // Create timestamp
  TimeMs timestamp = GetPeer()->GetLocalTime();

  // Is server?
  if(GetRole() == Role::Server)
  {
    // Route replica forget
    RouteForget(replicas, route, timestamp);
  }

  // Handle replica forget
  if(!HandleForget(replicas, TransmissionDirection::Outgoing, timestamp)) // Unable?
    return false;

  // (All replicas should be invalid)
  AssertReplicas(replicas, replica->IsInvalid());

  // Success
  return true;
}
bool Replicator::ForgetAllReplicas(const Route& route)
{
  // Forget all replicas
  ReplicaArray replicas = GetReplicas();
  return ForgetReplicas(replicas, route);
}

bool Replicator::DestroyReplica(Replica* replica, const Route& route)
{
  // Destroy single replica
  ReplicaArray replicas(ZeroInit, replica);
  return DestroyReplicas(replicas, route) != 0;
}
bool Replicator::DestroyReplicas(const ReplicaArray& replicas, const Route& route)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should be live)
  // (Hitting an assert here usually means an incorrect assumption was made in the calling logic about a replica's state)
  AssertReplicas(replicas, replica->IsLive());
  AssertReplicas(replicas, replica->GetInitializationTimestamp()   != cInvalidMessageTimestamp);
  AssertReplicas(replicas, replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

  // Create timestamp
  TimeMs timestamp = GetPeer()->GetLocalTime();

  // Route replica destroy
  RouteDestroy(replicas, route, timestamp);

  // Handle replica destroy
  if(!HandleDestroy(replicas, TransmissionDirection::Outgoing, timestamp)) // Unable?
    return false;

  // (All replicas should be invalid)
  AssertReplicas(replicas, replica->IsInvalid());

  // Success
  return true;
}
bool Replicator::DestroyAllReplicas(const Route& route)
{
  // Destroy all replicas
  ReplicaArray replicas = GetReplicas();
  return DestroyReplicas(replicas, route);
}

bool Replicator::Interrupt(const Route& route)
{
  Assert(GetRole() == Role::Server);

  // Route interrupt
  return RouteInterrupt(route);
}

//
// Configuration
//

void Replicator::ResetConfig()
{
  SetFrameFillWarning();
  SetFrameFillSkip();
}

void Replicator::SetFrameFillWarning(float frameFillWarning)
{
  mFrameFillWarning = frameFillWarning;
}
float Replicator::GetFrameFillWarning() const
{
  return mFrameFillWarning;
}

void Replicator::SetFrameFillSkip(float frameFillSkip)
{
  mFrameFillSkip = frameFillSkip;
}
float Replicator::GetFrameFillSkip() const
{
  return mFrameFillSkip;
}

//
// Replica Channel Type Management
//

bool Replicator::HasReplicaChannelType(const String& replicaChannelTypeName) const
{
  return mReplicaChannelTypes.Contains(replicaChannelTypeName);
}
const ReplicaChannelType* Replicator::GetReplicaChannelType(const String& replicaChannelTypeName) const
{
  const ReplicaChannelType* result = mReplicaChannelTypes.FindValue(replicaChannelTypeName, ReplicaChannelTypePtr());
  return result;
}
ReplicaChannelType* Replicator::GetReplicaChannelType(const String& replicaChannelTypeName)
{
  const ReplicaChannelType* result = mReplicaChannelTypes.FindValue(replicaChannelTypeName, ReplicaChannelTypePtr());
  return const_cast<ReplicaChannelType*>(result);
}
const ReplicaChannelTypeSet& Replicator::GetReplicaChannelTypes() const
{
  return mReplicaChannelTypes;
}
ReplicaChannelTypeSet& Replicator::GetReplicaChannelTypes()
{
  return mReplicaChannelTypes;
}

ReplicaChannelType* Replicator::AddReplicaChannelType(ReplicaChannelTypePtr replicaChannelType)
{
  // Add replica channel type
  ReplicaChannelTypeSet::pointer_bool_pair result = mReplicaChannelTypes.Insert(replicaChannelType);
  if(result.second) // Successful?
  {
    // Make the replica channel type valid now that it's registered with our replicator
    (*result.first)->MakeValid(this);

    // Success
    return (*result.first);
  }

  // Failure
  return nullptr;
}
bool Replicator::RemoveReplicaChannelType(const String& replicaChannelTypeName)
{
  // Remove replica channel type
  ReplicaChannelTypeSet::pointer_bool_pair result = mReplicaChannelTypes.EraseValue(replicaChannelTypeName);
  return result.second;
}

void Replicator::ClearReplicaChannelTypes()
{
  // Remove all replica channel types
  mReplicaChannelTypes.Clear();
}

//
// Replica Property Type Management
//

bool Replicator::HasReplicaPropertyType(const String& replicaPropertyTypeName) const
{
  return mReplicaPropertyTypes.Contains(replicaPropertyTypeName);
}
const ReplicaPropertyType* Replicator::GetReplicaPropertyType(const String& replicaPropertyTypeName) const
{
  const ReplicaPropertyType* result = mReplicaPropertyTypes.FindValue(replicaPropertyTypeName, ReplicaPropertyTypePtr());
  return result;
}
ReplicaPropertyType* Replicator::GetReplicaPropertyType(const String& replicaPropertyTypeName)
{
  const ReplicaPropertyType* result = mReplicaPropertyTypes.FindValue(replicaPropertyTypeName, ReplicaPropertyTypePtr());
  return const_cast<ReplicaPropertyType*>(result);
}
const ReplicaPropertyTypeSet& Replicator::GetReplicaPropertyTypes() const
{
  return mReplicaPropertyTypes;
}
ReplicaPropertyTypeSet& Replicator::GetReplicaPropertyTypes()
{
  return mReplicaPropertyTypes;
}

ReplicaPropertyType* Replicator::AddReplicaPropertyType(ReplicaPropertyTypePtr replicaPropertyType)
{
  // Add replica property type
  ReplicaPropertyTypeSet::pointer_bool_pair result = mReplicaPropertyTypes.Insert(replicaPropertyType);
  if(result.second) // Successful?
  {
    // Make the replica property type valid now that it's registered with our replicator
    (*result.first)->MakeValid(this);

    // Success
    return (*result.first);
  }

  // Failure
  return nullptr;
}
bool Replicator::RemoveReplicaPropertyType(const String& replicaPropertyTypeName)
{
  // Remove replica property type
  ReplicaPropertyTypeSet::pointer_bool_pair result = mReplicaPropertyTypes.EraseValue(replicaPropertyTypeName);
  return result.second;
}

void Replicator::ClearReplicaPropertyTypes()
{
  // Remove all replica property types
  mReplicaPropertyTypes.Clear();
}

//
// Replica Interface
//

void Replicator::ValidReplica(Replica* replica)
{
  // User callback
  OnValidReplica(replica);
}
void Replicator::LiveReplica(Replica* replica)
{
  // (Sanity check)
  Assert(replica->GetInitializationTimestamp()   != cInvalidMessageTimestamp);
  Assert(replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Schedule replica channel for change observation (as needed)
    replicaChannel->GetReplicaChannelType()->ScheduleChannel(replicaChannel);

    // (Scheduling replica properties for change convergence occurs once its first change is received, so there's nothing to do here)
  }

  // User callback
  OnLiveReplica(replica);
}
void Replicator::InvalidReplica(Replica* replica, bool isForget)
{
  // (Sanity check)
  Assert(replica->GetUninitializationTimestamp() != cInvalidMessageTimestamp);

  // User callback
  OnInvalidReplica(replica, isForget);

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Unschedule replica channel for change observation (as needed)
    replicaChannel->GetReplicaChannelType()->UnscheduleChannel(replicaChannel);

    // For all replica properties
    forRange(ReplicaProperty* replicaProperty, replicaChannel->GetReplicaProperties().All())
    {
      // Unschedule replica property for change convergence (as needed)
      replicaProperty->SetConvergenceState(ConvergenceState::None);
    }
  }

  // Clear replica timestamps
  replica->SetInitializationTimestamp(cInvalidMessageTimestamp);
  replica->SetLastChangeTimestamp(cInvalidMessageTimestamp);
  replica->SetUninitializationTimestamp(cInvalidMessageTimestamp);
}

//
// Replica Helpers
//

bool Replicator::AddReplicaToLiveSet(Replica* replica)
{
  // Add replica to live set
  ReplicaSet::pointer_bool_pair result = mReplicaSet.Insert(replica);
  if(!result.second) // Unable?
  {
    // Return failure (No clean up necessary)
    return false;
  }

  // Success
  return true;
}
bool Replicator::RemoveReplicaFromLiveSet(Replica* replica)
{
  // Remove replica from live set
  ReplicaSet::pointer_bool_pair result = mReplicaSet.EraseValue(replica);

  // Result
  return result.second;
}

bool Replicator::AddReplicaToCreateContextSet(Replica* replica)
{
  // Add replica to create context set
  ReplicaSet::pointer_bool_pair result = mCreateMap.FindOrInsert(replica->GetCreateContext()).Insert(replica);
  if(!result.second) // Unable?
  {
    // Clean up and return failure
    RemoveReplicaFromCreateContextSet(replica);
    return false;
  }

  // Success
  return true;
}
bool Replicator::RemoveReplicaFromCreateContextSet(Replica* replica)
{
  // Get create context set
  CreateMap::iterator iter = mCreateMap.FindIterator(replica->GetCreateContext());
  if(iter != mCreateMap.End()) // Found?
  {
    // Remove replica from create context set
    ReplicaSet::pointer_bool_pair result = iter->second.EraseValue(replica);

    // Replica type set is empty?
    if(iter->second.Empty())
      mCreateMap.Erase(iter); // Erase create context set

    // Result
    return result.second;
  }

  // Failure
  return false;
}

bool Replicator::AddReplicaToReplicaTypeSet(Replica* replica)
{
  // Add replica to replica type set
  ReplicaSet::pointer_bool_pair result = mReplicaMap.FindOrInsert(replica->GetReplicaType()).Insert(replica);
  if(!result.second) // Unable?
  {
    // Clean up and return failure
    RemoveReplicaFromReplicaTypeSet(replica);
    return false;
  }

  // Success
  return true;
}
bool Replicator::RemoveReplicaFromReplicaTypeSet(Replica* replica)
{
  // Get replica type set
  ReplicaMap::iterator iter = mReplicaMap.FindIterator(replica->GetReplicaType());
  if(iter != mReplicaMap.End()) // Found?
  {
    // Remove replica from replica type set
    ReplicaSet::pointer_bool_pair result = iter->second.EraseValue(replica);

    // Replica type set is empty?
    if(iter->second.Empty())
      mReplicaMap.Erase(iter); // Erase replica type set

    // Result
    return result.second;
  }

  // Failure
  return false;
}

bool Replicator::AddLiveReplica(Replica* replica)
{
  Assert(replica->GetReplicaId() != 0);

  // Add replica to live set
  {
    bool result = AddReplicaToLiveSet(replica);
    if(!result) // Unable?
    {
      // Return failure (No clean up necessary)
      Assert(false);
      return false;
    }
  }

  // Add replica to create context set
  {
    bool result = AddReplicaToCreateContextSet(replica);
    if(!result) // Unable?
    {
      // Clean up and return failure
      Assert(false);
      RemoveReplicaFromLiveSet(replica);
      return false;
    }
  }

  // Add replica to replica type set
  {
    bool result = AddReplicaToReplicaTypeSet(replica);
    if(!result) // Unable?
    {
      // Clean up and return failure
      Assert(false);
      RemoveReplicaFromCreateContextSet(replica);
      RemoveReplicaFromLiveSet(replica);
      return false;
    }
  }

  // Set replicator
  replica->SetReplicator(this);

  Assert(replica->IsLive());

  // Success
  return true;
}
void Replicator::RemoveLiveReplica(Replica* replica)
{
  Assert(replica->GetReplicaId() != 0);

  // Remove replica from replica type set
  {
    bool result = RemoveReplicaFromReplicaTypeSet(replica);
    Assert(result); // (Erase should have succeeded)
  }

  // Remove replica from create context set
  {
    bool result = RemoveReplicaFromCreateContextSet(replica);
    Assert(result); // (Erase should have succeeded)
  }

  // Remove replica from live set
  {
    bool result = RemoveReplicaFromLiveSet(replica);
    Assert(result); // (Erase should have succeeded)
  }

  // Clear replicator
  replica->SetReplicator(nullptr);

  Assert(replica->IsInvalid());
}

bool Replicator::AddEmplacedReplica(Replica* replica)
{
  Assert(replica->GetEmplaceId() != 0);

  // Add replica to emplace context set
  {
    // Get/create emplace context set
    ReplicaSet& contextSet = mEmplaceMap.FindOrInsert(replica->GetEmplaceContext());

    // Is client?
    if(GetRole() == Role::Client)
    {
      // Linear search for replica in set
      forRange(Replica* item, contextSet.All())
        if(item == replica) // Already in set?
        {
          Assert(false);
          return false;
        }

      // Push back replica
      static_cast<ReplicaSet::array_type&>(contextSet).PushBack(replica);
    }
    // Is server?
    else if(GetRole() == Role::Server)
    {
      Assert(replica->GetReplicaId() != 0);

      // Insert replica at sorted position
      ReplicaSet::pointer_bool_pair result = contextSet.Insert(replica);
      if(!result.second) // Unable?
      {
        Assert(false);
        return false;
      }
    }
  }

  // Set replicator
  replica->SetReplicator(this);

  Assert(replica->IsValid());

  // Success
  return true;
}
void Replicator::RemoveEmplacedReplica(Replica* replica)
{
  Assert(replica->GetEmplaceId() != 0);

  // Remove replica from emplace context set
  {
    // Get emplace context set
    EmplaceMap::iterator iter = mEmplaceMap.FindIterator(replica->GetEmplaceContext());
    if(iter != mEmplaceMap.End()) // Found?
    {
      // Get emplace context set
      ReplicaSet& contextSet = iter->second;

      // Is client?
      if(GetRole() == Role::Client)
      {
        // Linear search for replica in set
        bool result = false;
        for(ReplicaSet::iterator iter = contextSet.Begin(); iter != contextSet.End(); ++iter)
          if(*iter == replica) // Found?
          {
            // Erase replica
            static_cast<ReplicaSet::array_type&>(contextSet).Erase(iter);
            result = true;
            break;
          }
        Assert(result);
      }
      // Is server?
      else if(GetRole() == Role::Server)
      {
        Assert(replica->GetReplicaId() != 0);

        // Erase replica from sorted position
        ReplicaSet::pointer_bool_pair result = contextSet.EraseValue(replica);
        Assert(result.second);
      }

      // Context set is empty?
      if(contextSet.Empty())
        mEmplaceMap.Erase(iter); // Erase emplace context set
    }
    else
      Assert(false);
  }

  // Clear replicator
  replica->SetReplicator(nullptr);

  Assert(replica->IsInvalid());
}

//
// ID Helpers
//

void Replicator::SetReplicatorId(ReplicatorId replicatorId)
{
  Assert(GetRole() == Role::Client);
  mReplicatorId = replicatorId;
}

bool Replicator::AssignReplicatorId(ReplicatorLink* link)
{
  Assert(GetRole() == Role::Server);
  Assert(link->GetReplicatorId() == 0);

  // Acquire replicator ID
  ReplicatorId replicatorId = mReplicatorIdStore.AcquireId();
  if(replicatorId == 0) // Unable?
    return false;

  // Assign replicator ID
  link->SetReplicatorId(replicatorId);

  Assert(link->GetReplicatorId() != 0);

  // Success
  return true;
}
void Replicator::ReleaseReplicatorId(ReplicatorLink* link)
{
  Assert(GetRole() == Role::Server);
  Assert(link->GetReplicatorId() != 0);

  // Free replicator ID
  bool result = mReplicatorIdStore.FreeId(link->GetReplicatorId());
  Assert(result);

  // Clear replicator ID
  link->SetReplicatorId(0);

  Assert(link->GetReplicatorId() == 0);
}

bool Replicator::AssignReplicaId(Replica* replica)
{
  Assert(GetRole() == Role::Server);
  Assert(replica->GetReplicaId() == 0);

  // Acquire replica ID
  ReplicaId replicaId = mReplicaIdStore.AcquireId();
  if(replicaId == 0) // Unable?
    return false;

  // Assign replica ID
  replica->SetReplicaId(replicaId);

  Assert(replica->GetReplicaId() != 0);

  // Success
  return true;
}
void Replicator::ReleaseReplicaId(Replica* replica)
{
  Assert(GetRole() == Role::Server);
  Assert(replica->GetReplicaId() != 0);

  // Free replica ID
  bool result = mReplicaIdStore.FreeId(replica->GetReplicaId());
  Assert(result);

  // Clear replica ID
  replica->SetReplicaId(0);

  Assert(replica->GetReplicaId() == 0);
}

bool Replicator::AssignEmplaceId(Replica* replica)
{
  Assert(replica->GetEmplaceId() == 0);

  // Acquire emplace ID
  EmplaceId emplaceId = mEmplaceIdStores.FindOrInsert(replica->GetEmplaceContext()).AcquireId();
  if(emplaceId == 0) // Unable?
    return false;

  // Assign emplace ID
  replica->SetEmplaceId(emplaceId);

  Assert(replica->GetEmplaceId() != 0);

  // Success
  return true;
}
void Replicator::ReleaseEmplaceId(Replica* replica)
{
  Assert(replica->GetEmplaceId() != 0);

  // Get emplace ID store
  EmplaceIdStores::iterator iter = mEmplaceIdStores.FindIterator(replica->GetEmplaceContext());
  if(iter != mEmplaceIdStores.End()) // Found?
  {
    // Free emplace ID
    bool result = iter->second.FreeId(replica->GetEmplaceId());
    Assert(result);

    // No emplace IDs left in use?
    if(!iter->second.HasAcquiredIds())
      mEmplaceIdStores.Erase(iter); // Erase emplace ID store
  }
  else
    Assert(false);

  // Clear emplace ID
  replica->SetEmplaceId(0);

  Assert(replica->GetEmplaceId() == 0);
}

//
// Replication Helpers
//

bool Replicator::ShouldIncludeAccurateTimestampOnInitialization(const ReplicaArray& replicas)
{
  forRange(Replica* replica, replicas.All())
    if(replica && replica->GetAccurateTimestampOnInitialization())
      return true;

  return false;
}
bool Replicator::ShouldIncludeAccurateTimestampOnUninitialization(const ReplicaArray& replicas)
{
  forRange(Replica* replica, replicas.All())
    if(replica && replica->GetAccurateTimestampOnUninitialization())
      return true;

  return false;
}
bool Replicator::ShouldIncludeAccurateTimestampOnChange(ReplicaChannel* replicaChannel)
{
  return replicaChannel->GetReplica()->GetAccurateTimestampOnChange()
      || replicaChannel->GetReplicaChannelType()->GetAccurateTimestampOnChange();
}

TimeMs Replicator::GetInitializationTimestamp(const ReplicaArray& replicas)
{
#ifdef ZeroDebug

  // Verify all replica timestamps are the same
  // (We only serialize a single timestamp for all replicas in the replication command, so we're assuming they're all the same!)
  forRange(Replica* replica1, replicas.All())
    forRange(Replica* replica2, replicas.All())
      if((replica1 && replica2)
      && (replica1->GetInitializationTimestamp() != replica2->GetInitializationTimestamp()))
        Assert(false);

#endif

  forRange(Replica* replica, replicas.All())
    if(replica && replica->GetInitializationTimestamp() != cInvalidMessageTimestamp)
      return replica->GetInitializationTimestamp();

  return cInvalidMessageTimestamp;
}
TimeMs Replicator::GetUninitializationTimestamp(const ReplicaArray& replicas)
{
#ifdef ZeroDebug

  // Verify all replica timestamps are the same
  // (We only serialize a single timestamp for all replicas in the replication command, so we're assuming they're all the same!)
  forRange(Replica* replica1, replicas.All())
    forRange(Replica* replica2, replicas.All())
      if((replica1 && replica2)
      && (replica1->GetUninitializationTimestamp() != replica2->GetUninitializationTimestamp()))
        Assert(false);

#endif

  forRange(Replica* replica, replicas.All())
    if(replica && replica->GetUninitializationTimestamp() != cInvalidMessageTimestamp)
      return replica->GetUninitializationTimestamp();

  return cInvalidMessageTimestamp;
}

bool Replicator::HandleEmplace(const ReplicaArray& replicas, const EmplaceContext& emplaceContext, TimeMs timestamp)
{
  //
  // Handle ValidReplica and ReactToChannelPropertyChanges
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    Assert(replica->GetEmplaceId() == 0);

    // Set emplace context
    replica->SetEmplaceContext(emplaceContext);

    // Assign emplace ID
    if(!AssignEmplaceId(replica)) // Unable?
    {
      Assert(false);
      return false;
    }

    Assert(replica->GetEmplaceId() != 0);

    // Replica is about to made valid
    Assert(replica->IsInvalid());
    ValidReplica(replica);

    // Is server?
    if(GetRole() == Role::Server)
    {
      Assert(replica->GetInitializationTimestamp() == cInvalidMessageTimestamp);

      // Set replica's initialization timestamp
      replica->SetInitializationTimestamp(timestamp);

      Assert(replica->GetInitializationTimestamp() != cInvalidMessageTimestamp);
      Assert(replica->GetReplicaId() == 0);

      // Emplace context not mapped to an ID?
      if(!mEmplaceContextCacher.IsItemMapped(replica->GetEmplaceContext()))
      {
        // Map emplace context to ID
        bool result = mEmplaceContextCacher.MapItem(replica->GetEmplaceContext());
        Assert(result);
      }

      // Create context not mapped to an ID?
      if(!mCreateContextCacher.IsItemMapped(replica->GetCreateContext()))
      {
        // Map create context to ID
        bool result = mCreateContextCacher.MapItem(replica->GetCreateContext());
        Assert(result);
      }

      // Replica type not mapped to an ID?
      if(!mReplicaTypeCacher.IsItemMapped(replica->GetReplicaType()))
      {
        // Map replica type to ID
        bool result = mReplicaTypeCacher.MapItem(replica->GetReplicaType());
        Assert(result);
      }

      // Assign replica ID
      if(!AssignReplicaId(replica)) // Unable?
      {
        Assert(false);
        return false;
      }

      Assert(replica->GetReplicaId() != 0);
    }
    // Is client?
    else if(GetRole() == Role::Client)
    {
      Assert(replica->GetReplicaId() == 0);
    }

    // Add emplaced replica
    if(!AddEmplacedReplica(replica)) // Unable?
    {
      Assert(false);
      return false;
    }

    // Replica is now valid
    Assert(replica->IsValid());

    // Is server?
    if(GetRole() == Role::Server)
    {
      // Add live replica
      if(!AddLiveReplica(replica)) // Unable?
      {
        Assert(false);
        return false;
      }
    }

    // Handle initial replica channel property values
    // (Generate replica channel property changed notifications only if we're the server)
    // (Because emplacements on the server make the object live immediately, but not on the client where theirs occurs on receiving an incoming clone)
    replica->ReactToChannelPropertyChanges(timestamp, ReplicationPhase::Initialization, TransmissionDirection::Outgoing, (GetRole() == Role::Server));
  }

  //
  // Handle LiveReplica
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Is server?
    if(GetRole() == Role::Server)
    {
      // Replica is now live
      Assert(replica->IsLive());
      LiveReplica(replica);
    }

    Assert(replica->IsEmplaced());
  }

  // Success
  return true;
}

bool Replicator::HandleSpawn(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  //
  // Handle ValidReplica and ReactToChannelPropertyChanges
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    Assert(replica->GetInitializationTimestamp() == cInvalidMessageTimestamp);

    // Set replica's initialization timestamp
    replica->SetInitializationTimestamp(timestamp);

    Assert(replica->GetInitializationTimestamp() != cInvalidMessageTimestamp);

    // Replica is about to made valid
    Assert(replica->IsInvalid());
    ValidReplica(replica);

    // Outgoing spawn command?
    if(direction == TransmissionDirection::Outgoing)
    {
      Assert(GetRole() == Role::Server);
      Assert(replica->GetReplicaId() == 0);

      // Create context not mapped to an ID?
      if(!mCreateContextCacher.IsItemMapped(replica->GetCreateContext()))
      {
        // Map create context to ID
        bool result = mCreateContextCacher.MapItem(replica->GetCreateContext());
        Assert(result);
      }

      // Replica type not mapped to an ID?
      if(!mReplicaTypeCacher.IsItemMapped(replica->GetReplicaType()))
      {
        // Map replica type to ID
        bool result = mReplicaTypeCacher.MapItem(replica->GetReplicaType());
        Assert(result);
      }

      // Assign replica ID
      if(!AssignReplicaId(replica)) // Unable?
      {
        Assert(false);
        return false;
      }

      Assert(replica->GetReplicaId() != 0);
    }
    // Incoming spawn command?
    else
    {
      Assert(GetRole() == Role::Client);
      Assert(replica->GetReplicaId() != 0);
    }

    // Add live replica
    if(!AddLiveReplica(replica)) // Unable?
    {
      Assert(false);
      return false;
    }

    // Replica is now valid
    Assert(replica->IsValid());

    // Handle initial replica channel property values
    replica->ReactToChannelPropertyChanges(timestamp, ReplicationPhase::Initialization, direction, true);
  }

  //
  // Handle LiveReplica
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Replica is now live
    Assert(replica->IsLive());
    LiveReplica(replica);

    Assert(replica->IsSpawned());
  }

  // Success
  return true;
}
bool Replicator::RouteSpawn(const ReplicaArray& replicas, const Route& route, TimeMs timestamp)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Get links in route
  PeerLinkSet links = GetLinks(route);

  // For all links in route
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Send spawn command
    replicatorLink->SendSpawn(replicas, timestamp);
  }

  // Success
  return true;
}

bool Replicator::HandleClone(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  // Incoming clone command?
  if(direction == TransmissionDirection::Incoming)
  {
    Assert(GetRole() == Role::Client);

    //
    // Handle ValidReplica and ReactToChannelPropertyChanges
    //

    // For all replicas
    forRange(Replica* replica, replicas.All())
    {
      // Absent replica?
      if(!replica)
        continue; // Skip

      Assert(replica->GetInitializationTimestamp() == cInvalidMessageTimestamp);

      // Set replica's initialization timestamp
      replica->SetInitializationTimestamp(timestamp);

      Assert(replica->GetInitializationTimestamp() != cInvalidMessageTimestamp);
      Assert(replica->GetReplicaId() != 0);

      // Is spawned?
      if(replica->IsSpawned())
      {
        // Replica is about to made valid
        Assert(replica->IsInvalid());
        ValidReplica(replica);
      }

      // Add live replica
      if(!AddLiveReplica(replica)) // Unable?
      {
        Assert(false);
        return false;
      }

      // Replica is now valid
      Assert(replica->IsValid());

      // Handle initial replica channel property values
      replica->ReactToChannelPropertyChanges(timestamp, ReplicationPhase::Initialization, direction, true);
    }

    //
    // Handle LiveReplica
    //

    // For all replicas
    forRange(Replica* replica, replicas.All())
    {
      // Absent replica?
      if(!replica)
        continue; // Skip

      // Replica is now live
      Assert(replica->IsLive());
      LiveReplica(replica);
    }
  }

  // Success
  return true;
}
bool Replicator::RouteClone(const ReplicaArray& replicas, const Route& route, TimeMs timestamp)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Get links in route
  PeerLinkSet links = GetLinks(route);

  // For all links in route
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Send clone command
    replicatorLink->SendClone(replicas, timestamp);
  }

  // Success
  return true;
}

bool Replicator::HandleForget(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  //
  // Handle ReactToChannelPropertyChanges
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    Assert(replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

    // Set replica's uninitialization timestamp
    replica->SetUninitializationTimestamp(timestamp);

    Assert(replica->GetUninitializationTimestamp() != cInvalidMessageTimestamp);

    // Handle final replica channel property values
    replica->ReactToChannelPropertyChanges(timestamp, ReplicationPhase::Uninitialization, direction, true);
  }

  //
  // Handle InvalidReplica
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Replica is about to become invalid
    Assert(replica->IsValid());
    InvalidReplica(replica, true);

    // Replica is live?
    if(replica->IsLive())
    {
      // Remove live replica
      RemoveLiveReplica(replica);
    }

    // Replica was emplaced?
    if(replica->IsEmplaced())
    {
      // Remove emplaced replica
      RemoveEmplacedReplica(replica);

      Assert(replica->GetEmplaceId() != 0);

      // Release emplace ID
      ReleaseEmplaceId(replica);

      Assert(replica->GetEmplaceId() == 0);
    }

    // Is server?
    if(GetRole() == Role::Server)
    {
      Assert(direction == TransmissionDirection::Outgoing);
      Assert(replica->GetReplicaId() != 0);

      // Release replica ID
      ReleaseReplicaId(replica);

      Assert(replica->GetReplicaId() == 0);
    }
    // Is client?
    else if(GetRole() == Role::Client)
    {
      // Clear replica ID (if any)
      replica->SetReplicaId(0);

      Assert(replica->GetReplicaId() == 0);
    }

    Assert(replica->IsInvalid());
  }

  // Success
  return true;
}
bool Replicator::RouteForget(const ReplicaArray& replicas, const Route& route, TimeMs timestamp)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Get links in route
  PeerLinkSet links = GetLinks(route);

  // For all links in route
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Send forget command
    replicatorLink->SendForget(replicas, timestamp);
  }

  // Success
  return true;
}

bool Replicator::HandleDestroy(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  //
  // Handle ReactToChannelPropertyChanges
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    Assert(replica->GetUninitializationTimestamp() == cInvalidMessageTimestamp);

    // Set replica's uninitialization timestamp
    replica->SetUninitializationTimestamp(timestamp);

    Assert(replica->GetUninitializationTimestamp() != cInvalidMessageTimestamp);

    // Handle final replica channel property values
    replica->ReactToChannelPropertyChanges(timestamp, ReplicationPhase::Uninitialization, direction, true);
  }

  //
  // Handle InvalidReplica
  //

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Replica is about to become invalid
    Assert(replica->IsLive());
    InvalidReplica(replica, false);

    // Remove live replica
    RemoveLiveReplica(replica);

    // Replica was emplaced?
    if(replica->IsEmplaced())
    {
      // Remove emplaced replica
      RemoveEmplacedReplica(replica);

      Assert(replica->GetEmplaceId() != 0);

      // Release emplace ID
      ReleaseEmplaceId(replica);

      Assert(replica->GetEmplaceId() == 0);
    }

    // Outgoing destroy command?
    if(direction == TransmissionDirection::Outgoing)
    {
      Assert(GetRole() == Role::Server);
      Assert(replica->GetReplicaId() != 0);

      // Release replica ID
      ReleaseReplicaId(replica);

      Assert(replica->GetReplicaId() == 0);
    }
    // Incoming destroy command?
    else
    {
      Assert(GetRole() == Role::Client);
      Assert(replica->GetReplicaId() != 0);

      // Clear replica ID
      replica->SetReplicaId(0);

      Assert(replica->GetReplicaId() == 0);
    }

    Assert(replica->IsInvalid());
  }

  // Success
  return true;
}
bool Replicator::RouteDestroy(const ReplicaArray& replicas, const Route& route, TimeMs timestamp)
{
  Assert(GetRole() == Role::Server);

  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Get links in route
  PeerLinkSet links = GetLinks(route);

  // For all links in route
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Send destroy command
    replicatorLink->SendDestroy(replicas, timestamp);
  }

  // Success
  return true;
}

bool Replicator::RouteChange(ReplicaChannel* replicaChannel, const Route& route, TimeMs timestamp)
{
  // Get replica
  Replica*              replica   = replicaChannel->GetReplica();
  ReplicaId::value_type replicaId = replica->GetReplicaId().value();
  Assert(replica && replicaId);

  // Route replica channel change
  PeerLinkSet links = GetLinks(route);
  if(!links.Empty()) // Links in route?
  {
    // Serialize replica channel change
    Message message(ReplicatorMessageType::Change);
    if(!SerializeChange(replicaChannel, message, timestamp)) // Unable?
      return false;

    // Should include an accurate timestamp with this message?
    if(Replicator::ShouldIncludeAccurateTimestampOnChange(replicaChannel))
    {
      // Set accurate timestamp
      message.SetTimestamp(timestamp);
    }

    // For all replicator links in route
    forRange(PeerLink* link, links.All())
    {
      // Get replicator link
      ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

      // Should skip change replication?
      if(replicatorLink->ShouldSkipChangeReplication())
        continue; // Skip link

      // Has replica remotely?
      if(replicatorLink->HasReplica(replica))
        replicatorLink->SendChange(replicaChannel, message); // Send replica channel change
    }
  }

  // Success
  return true;
}
bool Replicator::SerializeChange(ReplicaChannel* replicaChannel, Message& message, TimeMs timestamp)
{
  // Serialize replica channel change
  BitStream& bitStream = message.GetData();

  // Write replica channel
  bool result = replicaChannel->Serialize(bitStream, ReplicationPhase::Change, timestamp);
  if(!result) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}

bool Replicator::RouteInterrupt(const Route& route)
{
  Assert(GetRole() == Role::Server);

  // Route interrupt command
  PeerLinkSet links = GetLinks(route);
  if(!links.Empty()) // Links in route?
  {
    // Serialize interrupt command
    Message message(ReplicatorMessageType::Interrupt);

    // For all replicator links in route
    forRange(PeerLink* link, links.All())
    {
      // Send interrupt command
      ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");
      replicatorLink->SendInterrupt(message);
    }
  }

  // Success
  return true;
}

//
// Peer Plugin Interface
//

bool Replicator::ShouldDeleteAfterRemoval()
{
  return false;
}

bool Replicator::OnInitialize()
{
  switch(GetRole())
  {
  default:
  case Role::Unspecified:
    {
      // Invalid role, do not use replicator plugin
      return false;
    }

  case Role::Client:
    {
      // Set connect response mode to deny
      // Clients should not accept any incoming connections
      GetPeer()->SetConnectResponseMode(ConnectResponseMode::Deny);

      // Success, use replicator plugin
      return true;
    }

  case Role::Server:
    {
      // Set connect response mode to custom
      // Server should decide when to accept incoming connections
      GetPeer()->SetConnectResponseMode(ConnectResponseMode::Custom);

      // Success, use replicator plugin
      return true;
    }
  }
}
void Replicator::OnUninitialize()
{
  // Forget any remaining replicas locally
  bool result = ForgetAllReplicas(Route::None);
  Assert(result);

  // (Sanity check)
  Assert(mReplicaSet.Empty());
  Assert(mCreateMap.Empty());
  Assert(mReplicaMap.Empty());
  Assert(mEmplaceMap.Empty());
  Assert(!mReplicaIdStore.HasAcquiredIds());
  Assert(mEmplaceIdStores.Empty());

  // Reset session data
  ResetSession();
}

void Replicator::OnUpdate()
{
  ProfileScopeTree("Replication", "NetPeer", Color::Orange);

  // Get current time
  TimeMs now = GetPeer()->GetLocalTime();

  //
  // Update Start
  //

  // For all links
  PeerLinkSet links = GetLinks();
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Handle update start
    replicatorLink->UpdateStart(now);
  }

  //
  // Update
  //

  // For all replica channel types
  forRange(ReplicaChannelTypePtr& replicaChannelType, mReplicaChannelTypes.All())
  {
    // Observe all scheduled replica channels of this type and replicate any changes
    replicaChannelType->ObserveAndReplicateChanges();
  }

  // For all replica property types
  forRange(ReplicaPropertyTypePtr& replicaPropertyType, mReplicaPropertyTypes.All())
  {
    // Converge all scheduled replica properties of this type
    replicaPropertyType->ConvergeNow();
  }

  //
  // Update End
  //

  // For all links
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Handle update end
    replicatorLink->UpdateEnd(now);
  }
}

bool Replicator::OnLinkAdd(PeerLink* link)
{
  // Add replicator link plugin
  ReplicatorLink* replicatorLink = new ReplicatorLink(this);
  if(!link->AddPlugin(replicatorLink, "ReplicatorLink")) // Unable?
  {
    delete replicatorLink;
    Assert(false);
    return false;
  }

  // User callback
  AddingLink(link);

  // Success
  return true;
}
void Replicator::OnLinkRemove(PeerLink* link)
{
  // User callback
  RemovingLink(link);
}

String GetReplicatorDisplayName(Replicator* replicator)
{
  return GetReplicatorDisplayName(replicator->GetPeer()->GetLocalIpv4Address(), replicator->GetRole(), replicator->GetReplicatorId());
}
String GetReplicatorDisplayName(ReplicatorLink* replicatorLink)
{
  return GetReplicatorDisplayName(replicatorLink->GetLink()->GetTheirIpAddress(), replicatorLink->GetTheirRole(), replicatorLink->GetReplicatorId());
}
String GetReplicatorDisplayName(const IpAddress& ipAddress, Role::Enum role, ReplicatorId replicatorId)
{
  // Is client? (Replicator ID may be non-zero?)
  if(role == Role::Client)
  {
    return String::Format("(%s) [%s %u]", ipAddress.GetString().c_str(), Role::Names[role], replicatorId.value());
  }
  // Is other role?
  else
  {
    return String::Format("(%s) [%s]", ipAddress.GetString().c_str(), Role::Names[role]);
  }
}

} // Namespace Zero
