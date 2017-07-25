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
//                               ReplicatorLink                                    //
//---------------------------------------------------------------------------------//

ReplicatorLink::ReplicatorLink(Replicator* replicator)
  : LinkPlugin(ReplicatorMessageType::Size),
    mReplicator(replicator),
    mReplicatorId(0),
    mReplicaSet(),
    mCreateMap(),
    mReplicaMap(),
    mCommandChannelId(0),
    mOutReplicaChannels(),
    mInReplicaChannels(),
    mInReplicaChannelsFlipped(),
    mLastConnectRequestData(),
    mLastConnectResponseData(),
    mShouldSkipChangeReplication(false),
    mLastFrameFillSkipNotificationTime(0),
    mLastFrameFillWarningNotificationTime(0)
{
}

//
// Operations
//

Replicator* ReplicatorLink::GetReplicator() const
{
  return mReplicator;
}

Role::Enum ReplicatorLink::GetTheirRole() const
{
  return (GetReplicator()->GetRole() == Role::Client)
       ? Role::Server
       : Role::Client;
}

ReplicatorId ReplicatorLink::GetReplicatorId() const
{
  return mReplicatorId;
}

void ReplicatorLink::Send(Status& status, const Message& message)
{
  Assert(GetCommandChannelId());
  GetLink()->Send(status, message, true, GetCommandChannelId());
}

//
// Replica Management
//

bool ReplicatorLink::HasReplica(ReplicaId replicaId) const
{
  // Has replica in set?
  return mReplicaSet.Contains(replicaId);
}
bool ReplicatorLink::HasReplica(Replica* replica) const
{
  // Has replica in set?
  return mReplicaSet.Contains(replica);
}
bool ReplicatorLink::HasReplicasByCreateContext(const CreateContext& createContext) const
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
bool ReplicatorLink::HasReplicasByReplicaType(const ReplicaType& replicaType) const
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
bool ReplicatorLink::HasReplicas() const
{
  // Has replicas in set?
  return !mReplicaSet.Empty();
}

Replica* ReplicatorLink::GetReplica(ReplicaId replicaId) const
{
  // Find replica in set
  ReplicaSet::const_iterator iter = mReplicaSet.FindIterator(replicaId);
  if(iter != mReplicaSet.End()) // Found?
    return *iter;

  return nullptr;
}
Replica* ReplicatorLink::GetReplica(Replica* replica) const
{
  // Find replica in set
  ReplicaSet::const_iterator iter = mReplicaSet.FindIterator(replica);
  if(iter != mReplicaSet.End()) // Found?
    return *iter;

  return nullptr;
}
ReplicaSet ReplicatorLink::GetReplicasByCreateContext(const CreateContext& createContext) const
{
  // Find replicas in create context set
  CreateMap::const_iterator iter = mCreateMap.FindIterator(createContext);
  if(iter != mCreateMap.End()) // Found?
    return iter->second;

  return ReplicaSet();
}
ReplicaSet ReplicatorLink::GetReplicasByReplicaType(const ReplicaType& replicaType) const
{
  // Find replicas in replica type set
  ReplicaMap::const_iterator iter = mReplicaMap.FindIterator(replicaType);
  if(iter != mReplicaMap.End()) // Found?
    return iter->second;

  return ReplicaSet();
}
const ReplicaSet& ReplicatorLink::GetReplicas() const
{
  return mReplicaSet;
}

size_t ReplicatorLink::GetReplicaCountByCreateContext(const CreateContext& createContext) const
{
  // Count replicas in create context set
  CreateMap::const_iterator iter = mCreateMap.FindIterator(createContext);
  if(iter != mCreateMap.End()) // Found?
    return iter->second.Size();

  return 0;
}
size_t ReplicatorLink::GetReplicaCountByReplicaType(const ReplicaType& replicaType) const
{
  // Count replicas in replica type set
  ReplicaMap::const_iterator iter = mReplicaMap.FindIterator(replicaType);
  if(iter != mReplicaMap.End()) // Found?
    return iter->second.Size();

  return 0;
}
size_t ReplicatorLink::GetReplicaCount() const
{
  return mReplicaSet.Size();
}

//
// Other Methods
//

/// Returns the last connect request data sent or received by this link
const ConnectRequestData& ReplicatorLink::GetLastConnectRequestData() const
{
  return mLastConnectRequestData;
}
/// Returns the last connect response data sent or received by this link
const ConnectResponseData& ReplicatorLink::GetLastConnectResponseData() const
{
  return mLastConnectResponseData;
}

bool ReplicatorLink::ShouldSkipChangeReplication() const
{
  return mShouldSkipChangeReplication;
}

//
// Internal
//

void ReplicatorLink::UpdateStart(TimeMs now)
{
  // Update 'Should-skip-change-replication' flag
  {
    // Get frame fill info
    float frameFillSkip = GetReplicator()->GetFrameFillSkip();
    float frameFill     = GetLink()->GetOutgoingFrameFill();

    // Should skip change replication if our current frame fill ratio exceeds our configured skip threshold
    mShouldSkipChangeReplication = (frameFill >= frameFillSkip);

    // // Should skip change replication?
    // if(mShouldSkipChangeReplication)
    // {
    //   // Last notified more than a second ago?
    //   if(GetDuration(mLastFrameFillSkipNotificationTime, now) > cOneSecondTimeMs)
    //   {
    //     // Get outgoing bandwidth info
    //     String replicatorName     = GetReplicatorDisplayName(GetReplicator());
    //     String replicatorLinkName = GetReplicatorDisplayName(this);
    //     Kbps   outgoingBandwidth  = GetLink()->GetOutgoingBandwidth();
    // 
    //     // Warn the user about the replication frame skip
    //     DoNotifyWarning("Network Frame Fill Skipping",
    //                     String::Format("%s Skipping change replication this frame to %s - Exceeded the outgoing bandwidth (%.2fkbps) utilization skip threshold (%.2f) in a previous frame, with a current frame fill ratio of %.2f",
    //                     replicatorName.c_str(),
    //                     replicatorLinkName.c_str(),
    //                     outgoingBandwidth,
    //                     frameFillSkip,
    //                     frameFill));
    // 
    //     // Set last notification time
    //     mLastFrameFillSkipNotificationTime = now;
    //   }
    // }
  }
}
void ReplicatorLink::UpdateEnd(TimeMs now)
{
  // See if we should warn the user about their outgoing bandwidth utilization this frame
  {
    // Get frame fill info
    float frameFillWarning = GetReplicator()->GetFrameFillWarning();
    float frameFill        = GetLink()->GetOutgoingFrameFill();

    // Current frame's fill ratio exceeds our configured warning threshold?
    if(frameFill >= frameFillWarning)
    {
      // Last notified more than a second ago?
      if(GetDuration(mLastFrameFillWarningNotificationTime, now) > cOneSecondTimeMs)
      {
        // Get outgoing bandwidth info
        String replicatorName     = GetReplicatorDisplayName(GetReplicator());
        String replicatorLinkName = GetReplicatorDisplayName(this);
        Kbps   outgoingBandwidth  = GetLink()->GetOutgoingBandwidth();

        // Warn the user about their outgoing bandwidth utilization
        DoNotifyWarning("Network Frame Fill Warning",
                        String::Format("%s Exceeded the outgoing bandwidth (%.2fkbps) utilization warning threshold (%.2f) this frame, with a current frame fill ratio of %.2f, when replicating to %s",
                        replicatorName.c_str(),
                        outgoingBandwidth,
                        frameFillWarning,
                        frameFill,
                        replicatorLinkName.c_str()));

        // Set last notification time
        mLastFrameFillWarningNotificationTime = now;
      }
    }
  }
}

//
// Replica Helpers
//

bool ReplicatorLink::AddReplicaToLiveSet(Replica* replica)
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
bool ReplicatorLink::RemoveReplicaFromLiveSet(Replica* replica)
{
  // Remove replica from live set
  ReplicaSet::pointer_bool_pair result = mReplicaSet.EraseValue(replica);

  // Result
  return result.second;
}

bool ReplicatorLink::AddReplicaToCreateContextSet(Replica* replica)
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
bool ReplicatorLink::RemoveReplicaFromCreateContextSet(Replica* replica)
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

bool ReplicatorLink::AddReplicaToReplicaTypeSet(Replica* replica)
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
bool ReplicatorLink::RemoveReplicaFromReplicaTypeSet(Replica* replica)
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

bool ReplicatorLink::AddLiveReplica(Replica* replica)
{
  Assert(replica->GetReplicaId() != 0);
  Assert(replica->GetReplicator());
  Assert(replica->IsLive());

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

  // Success
  return true;
}
void ReplicatorLink::RemoveLiveReplica(Replica* replica)
{
  Assert(replica->GetReplicaId() != 0);
  Assert(replica->GetReplicator());
  Assert(replica->IsLive());

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
}

//
// Id Helpers
//

void ReplicatorLink::SetReplicatorId(ReplicatorId replicatorId)
{
  Assert(GetReplicator()->GetRole() == Role::Server);
  mReplicatorId = replicatorId;
}

//
// Replication Helpers
//

bool ReplicatorLink::SerializeSpawn(const ReplicaArray& replicas, Message& message, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // Serialize spawn command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Spawn, timestamp);
  if(!GetReplicator()->SerializeReplicas(replicas, replicaStream)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}
bool ReplicatorLink::DeserializeSpawn(const Message& message, ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // Deserialize spawn command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Spawn, timestamp);
  if(!GetReplicator()->DeserializeReplicas(replicaStream, replicas)) // Unable?
  {
    // Assert(false);
    return false;
  }

  // (All replicas should be invalid)
  AssertReplicas(replicas, replica->IsInvalid());

  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Success
  return true;
}
bool ReplicatorLink::HandleSpawn(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // (All replicas should not be clones)
  AssertReplicas(replicas, !replica->IsCloned());

  // Outgoing spawn command?
  if(direction == TransmissionDirection::Outgoing)
  {
    Assert(GetReplicator()->GetRole() == Role::Server);

    // *** Spawn already handled locally ***
  }
  // Incoming spawn command?
  else
  {
    Assert(GetReplicator()->GetRole() == Role::Client);

    // Handle spawn locally
    if(!GetReplicator()->HandleSpawn(replicas, direction, timestamp)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Add live replica
    if(!AddLiveReplica(replica)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // Success
  return true;
}
bool ReplicatorLink::SendSpawn(const ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // (They should not already have any of these replicas)
  AssertReplicas(replicas, !HasReplica(replica));

  // Serialize spawn command
  Message message(ReplicatorMessageType::Spawn);
  if(!SerializeSpawn(replicas, message, timestamp)) // Unable?
    return false;

  // Should include an accurate timestamp with this message?
  if(Replicator::ShouldIncludeAccurateTimestampOnInitialization(replicas))
  {
    // Set accurate timestamp
    message.SetTimestamp(timestamp);
  }

  // Send spawn message
  Assert(GetCommandChannelId());
  Status status;
  LinkPlugin::Send(status, ZeroMove(message), true, GetCommandChannelId());
  if(status.Failed()) // Unable?
    return false;

  // Handle outgoing spawn command
  bool result = HandleSpawn(replicas, TransmissionDirection::Outgoing, timestamp);
  Assert(result);

  // Success
  return true;
}
bool ReplicatorLink::ReceiveSpawn(const Message& message)
{
  Assert(GetReplicator()->GetRole() == Role::Client);
  Assert(message.GetType() == ReplicatorMessageType::Spawn);
  Assert(message.HasTimestamp());

  // Get timestamp from message (may or may not be an accurate timestamp)
  TimeMs timestamp = message.GetTimestamp();

  // Deserialize spawn command
  ReplicaArray replicas;
  if(!DeserializeSpawn(message, replicas, timestamp)) // Unable?
    return false;

  // (We should not already have any of these replicas)
  AssertReplicas(replicas, !GetReplicator()->HasReplica(replica));
  AssertReplicas(replicas, !HasReplica(replica));

  // Handle incoming spawn command
  if(!HandleSpawn(replicas, TransmissionDirection::Incoming, timestamp)) // Unable?
  {
    Assert(false);
    GetReplicator()->ReleaseReplicas(replicas);
    return false;
  }

  // Send reverse replica channels as applicable
  if(!SendReverseReplicaChannels(replicas, timestamp)) // Unable?
  {
    Assert(false);
    GetReplicator()->ReleaseReplicas(replicas);
    return false;
  }

  // Success
  return true;
}

bool ReplicatorLink::SerializeClone(const ReplicaArray& replicas, Message& message, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // Serialize clone command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Clone, timestamp);
  if(!GetReplicator()->SerializeReplicas(replicas, replicaStream)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}
bool ReplicatorLink::DeserializeClone(const Message& message, ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // Deserialize clone command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Clone, timestamp);
  if(!GetReplicator()->DeserializeReplicas(replicaStream, replicas)) // Unable?
  {
    // Assert(false);
    return false;
  }

  // (All replicas should be either invalid (clone-from-spawn) or emplaced (clone-from-emplace))
  AssertReplicas(replicas, replica->IsInvalid() || replica->IsEmplaced());

  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Success
  return true;
}
bool ReplicatorLink::HandleClone(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // Outgoing clone command?
  if(direction == TransmissionDirection::Outgoing)
  {
    Assert(GetReplicator()->GetRole() == Role::Server);

    // (All replicas should not be clones - these are the originals on the server!)
    AssertReplicas(replicas, !replica->IsCloned());

    // *** Clone already handled locally ***
  }
  // Incoming clone command?
  else
  {
    Assert(GetReplicator()->GetRole() == Role::Client);

    // (All replicas should be clones - these are copies on the client!)
    AssertReplicas(replicas, replica->IsCloned());

    // Handle clone locally
    if(!GetReplicator()->HandleClone(replicas, direction, timestamp)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Add live replica
    if(!AddLiveReplica(replica)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // Success
  return true;
}
bool ReplicatorLink::SendClone(const ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // (They should not already have any of these replicas)
  AssertReplicas(replicas, !HasReplica(replica));

  // Serialize clone command
  Message message(ReplicatorMessageType::Clone);
  if(!SerializeClone(replicas, message, timestamp)) // Unable?
    return false;

  // Should include an accurate timestamp with this message?
  if(Replicator::ShouldIncludeAccurateTimestampOnInitialization(replicas))
  {
    // Set accurate timestamp
    message.SetTimestamp(timestamp);
  }

  // Send clone message
  Assert(GetCommandChannelId());
  Status status;
  LinkPlugin::Send(status, ZeroMove(message), true, GetCommandChannelId());
  if(status.Failed()) // Unable?
    return false;

  // Handle outgoing clone command
  bool result = HandleClone(replicas, TransmissionDirection::Outgoing, timestamp);
  Assert(result);

  // Success
  return true;
}
bool ReplicatorLink::ReceiveClone(const Message& message)
{
  Assert(GetReplicator()->GetRole() == Role::Client);
  Assert(message.GetType() == ReplicatorMessageType::Clone);
  Assert(message.HasTimestamp());

  // Get timestamp from message (may or may not be an accurate timestamp)
  TimeMs timestamp = message.GetTimestamp();

  // Deserialize clone command
  ReplicaArray replicas;
  if(!DeserializeClone(message, replicas, timestamp)) // Unable?
    return false;

  // (We should not already have any of these replicas)
  AssertReplicas(replicas, !GetReplicator()->HasReplica(replica));
  AssertReplicas(replicas, !HasReplica(replica));

  // Handle incoming clone command
  if(!HandleClone(replicas, TransmissionDirection::Incoming, timestamp)) // Unable?
  {
    Assert(false);
    GetReplicator()->ReleaseReplicas(replicas);
    return false;
  }

  // Send reverse replica channels as applicable
  if(!SendReverseReplicaChannels(replicas, timestamp)) // Unable?
  {
    Assert(false);
    GetReplicator()->ReleaseReplicas(replicas);
    return false;
  }

  // Success
  return true;
}

bool ReplicatorLink::SerializeForget(const ReplicaArray& replicas, Message& message, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // Serialize forget command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Forget, timestamp);
  if(!GetReplicator()->SerializeReplicas(replicas, replicaStream)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}
bool ReplicatorLink::DeserializeForget(const Message& message, ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // Deserialize forget command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Forget, timestamp);
  if(!GetReplicator()->DeserializeReplicas(replicaStream, replicas)) // Unable?
  {
    // Assert(false);
    return false;
  }

  // (All replicas should be live)
  AssertReplicas(replicas, replica->IsLive());

  // Success
  return true;
}
bool ReplicatorLink::HandleForget(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Remove live replica
    RemoveLiveReplica(replica);

    // Close all replica channels owned by the replica
    CloseAllReplicaChannels(replica);
  }

  // Outgoing forget command?
  if(direction == TransmissionDirection::Outgoing)
  {
    Assert(GetReplicator()->GetRole() == Role::Server);

    // (All replicas should be still be live)
    AssertReplicas(replicas, replica->IsLive());

    // *** Forget not yet handled locally ***
  }
  // Incoming forget command?
  else
  {
    Assert(GetReplicator()->GetRole() == Role::Client);

    // Handle forget locally
    bool result = GetReplicator()->HandleForget(replicas, direction, timestamp);
    Assert(result);

    // (All replicas should be invalid)
    AssertReplicas(replicas, replica->IsInvalid());
  }

  // Success
  return true;
}
bool ReplicatorLink::SendForget(const ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // (They should already have all of these replicas)
  AssertReplicas(replicas, HasReplica(replica));

  // Serialize forget command
  Message message(ReplicatorMessageType::Forget);
  if(!SerializeForget(replicas, message, timestamp)) // Unable?
    return false;

  // Should include an accurate timestamp with this message?
  if(Replicator::ShouldIncludeAccurateTimestampOnUninitialization(replicas))
  {
    // Set accurate timestamp
    message.SetTimestamp(timestamp);
  }

  // Send forget message
  Assert(GetCommandChannelId());
  Status status;
  LinkPlugin::Send(status, ZeroMove(message), true, GetCommandChannelId());
  if(status.Failed()) // Unable?
    return false;

  // Handle outgoing forget command
  bool result = HandleForget(replicas, TransmissionDirection::Outgoing, timestamp);
  Assert(result);

  // Success
  return true;
}
bool ReplicatorLink::ReceiveForget(const Message& message)
{
  Assert(GetReplicator()->GetRole() == Role::Client);
  Assert(message.GetType() == ReplicatorMessageType::Forget);
  Assert(message.HasTimestamp());

  // Get timestamp from message (may or may not be an accurate timestamp)
  TimeMs timestamp = message.GetTimestamp();

  // Deserialize forget command
  ReplicaArray replicas;
  if(!DeserializeForget(message, replicas, timestamp)) // Unable?
    return false;

  // (We should already have all of these replicas)
  AssertReplicas(replicas, GetReplicator()->HasReplica(replica));
  AssertReplicas(replicas, HasReplica(replica));

  // Handle incoming forget command
  if(!HandleForget(replicas, TransmissionDirection::Incoming, timestamp)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}

bool ReplicatorLink::SerializeDestroy(const ReplicaArray& replicas, Message& message, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // Serialize destroy command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Destroy, timestamp);
  if(!GetReplicator()->SerializeReplicas(replicas, replicaStream)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}
bool ReplicatorLink::DeserializeDestroy(const Message& message, ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // Deserialize destroy command
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::Destroy, timestamp);
  if(!GetReplicator()->DeserializeReplicas(replicaStream, replicas)) // Unable?
  {
    // Assert(false);
    return false;
  }

  // (All replicas should be live)
  AssertReplicas(replicas, replica->IsLive());

  // Success
  return true;
}
bool ReplicatorLink::HandleDestroy(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp)
{
  // (All replicas should have a replica ID)
  AssertReplicas(replicas, replica->GetReplicaId() != 0);

  // For all replicas
  forRange(Replica* replica, replicas.All())
  {
    // Remove live replica
    RemoveLiveReplica(replica);

    // Close all replica channels owned by the replica
    CloseAllReplicaChannels(replica);
  }

  // Outgoing destroy command?
  if(direction == TransmissionDirection::Outgoing)
  {
    Assert(GetReplicator()->GetRole() == Role::Server);

    // (All replicas should be still be live)
    AssertReplicas(replicas, replica->IsLive());

    // *** Destroy not yet handled locally ***
  }
  // Incoming destroy command?
  else
  {
    Assert(GetReplicator()->GetRole() == Role::Client);

    // Handle destroy locally
    bool result = GetReplicator()->HandleDestroy(replicas, direction, timestamp);
    Assert(result);

    // (All replicas should be invalid)
    AssertReplicas(replicas, replica->IsInvalid());

    // Release replicas
    GetReplicator()->ReleaseReplicas(replicas);
  }

  // Success
  return true;
}
bool ReplicatorLink::SendDestroy(const ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // (They should already have all of these replicas)
  AssertReplicas(replicas, HasReplica(replica));

  // Serialize destroy command
  Message message(ReplicatorMessageType::Destroy);
  if(!SerializeDestroy(replicas, message, timestamp)) // Unable?
    return false;

  // Should include an accurate timestamp with this message?
  if(Replicator::ShouldIncludeAccurateTimestampOnUninitialization(replicas))
  {
    // Set accurate timestamp
    message.SetTimestamp(timestamp);
  }

  // Send destroy message
  Assert(GetCommandChannelId());
  Status status;
  LinkPlugin::Send(status, ZeroMove(message), true, GetCommandChannelId());
  if(status.Failed()) // Unable?
    return false;

  // Handle outgoing destroy command
  bool result = HandleDestroy(replicas, TransmissionDirection::Outgoing, timestamp);
  Assert(result);

  // Success
  return true;
}
bool ReplicatorLink::ReceiveDestroy(const Message& message)
{
  Assert(GetReplicator()->GetRole() == Role::Client);
  Assert(message.GetType() == ReplicatorMessageType::Destroy);
  Assert(message.HasTimestamp());

  // Get timestamp from message (may or may not be an accurate timestamp)
  TimeMs timestamp = message.GetTimestamp();

  // Deserialize destroy command
  ReplicaArray replicas;
  if(!DeserializeDestroy(message, replicas, timestamp)) // Unable?
    return false;

  // (We should already have all of these replicas)
  AssertReplicas(replicas, GetReplicator()->HasReplica(replica));
  AssertReplicas(replicas, HasReplica(replica));

  // Handle incoming destroy command
  if(!HandleDestroy(replicas, TransmissionDirection::Incoming, timestamp)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Success
  return true;
}

bool ReplicatorLink::SerializeReverseReplicaChannels(const ReplicaArray& replicas, Message& message, bool& containsChannels, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // Determine if we're writing any channels
  containsChannels = false;

  // Get bits written at the start
  Bits bitsWrittenStart = message.GetData().GetBitsWritten();

  // Serialize reverse replica channel mappings
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::ReverseReplicaChannels, timestamp);
  if(!GetReplicator()->SerializeReplicas(replicas, replicaStream)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Get bits written at the end
  Bits bitsWrittenEnd = message.GetData().GetBitsWritten();

  // We've written channels if the replica stream was modified
  containsChannels = (bitsWrittenStart != bitsWrittenEnd);

  // Success
  return true;
}
bool ReplicatorLink::DeserializeReverseReplicaChannels(const Message& message, ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // Deserialize reverse replica channel mappings
  ReplicaStream replicaStream(GetReplicator(), this, message.GetData(), ReplicaStreamMode::ReverseReplicaChannels, timestamp);
  if(!GetReplicator()->DeserializeReplicas(replicaStream, replicas)) // Unable?
  {
    // Assert(false);
    return false;
  }

  // Success
  return true;
}
bool ReplicatorLink::SendReverseReplicaChannels(const ReplicaArray& replicas, TimeMs timestamp)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // (They should already have all of these replicas)
  AssertReplicas(replicas, HasReplica(replica));

  // Serialize reverse replica channel mappings
  bool containsChannels = false;
  Message message(ReplicatorMessageType::ReverseReplicaChannels);
  if(!SerializeReverseReplicaChannels(replicas, message, containsChannels, timestamp)) // Unable?
    return false;

  // No replica channels were serialized?
  if(!containsChannels)
  {
    // Don't bother sending, there's nothing useful in this message
    return true;
  }

  // Send reverse replica channels message
  Assert(GetCommandChannelId());
  Status status;
  LinkPlugin::Send(status, ZeroMove(message), true, GetCommandChannelId());
  if(status.Failed()) // Unable?
    return false;

  // Success
  return true;
}
bool ReplicatorLink::ReceiveReverseReplicaChannels(const Message& message)
{
  Assert(GetReplicator()->GetRole() == Role::Server);
  Assert(message.GetType() == ReplicatorMessageType::ReverseReplicaChannels);

  // Get timestamp from message (may or may not be an accurate timestamp)
  TimeMs timestamp = message.GetTimestamp();

  // Deserialize reverse replica channel mappings
  ReplicaArray replicas;
  if(!DeserializeReverseReplicaChannels(message, replicas, timestamp)) // Unable?
    return false;

  // // (We should already have all of these replicas)
  // AssertReplicas(replicas, GetReplicator()->HasReplica(replica));
  // AssertReplicas(replicas, HasReplica(replica));

  // Success
  return true;
}

bool ReplicatorLink::ReceiveCreateContextItems(const Message& message)
{
  // Receive create context items
  return GetReplicator()->mCreateContextCacher.ReceiveItems(message);
}
bool ReplicatorLink::ReceiveReplicaTypeItems(const Message& message)
{
  // Receive replica type items
  return GetReplicator()->mReplicaTypeCacher.ReceiveItems(message);
}
bool ReplicatorLink::ReceiveEmplaceContextItems(const Message& message)
{
  // Receive emplace context items
  return GetReplicator()->mEmplaceContextCacher.ReceiveItems(message);
}

bool ReplicatorLink::DeserializeChange(const Message& message, TimeMs timestamp)
{
  Assert(message.HasTimestamp());

  // Deserialize replica channel change
  const BitStream& bitStream = message.GetData();

  // Get replica channel
  ReplicaChannel* replicaChannel = GetIncomingReplicaChannel(message.GetChannelId());
  if(!replicaChannel) // Unable?
  {
    //Assert(false);
    return false;
  }

  // Get replica channel type
  ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

  // Get replica
  Replica*              replica   = replicaChannel->GetReplica();
  ReplicaId::value_type replicaId = replica->GetReplicaId().value();
  Assert(replica && replicaId);

  // Don't accept incoming changes for this replica or replica channel type?
  if(!replica->GetAcceptIncomingChanges()
  || !replicaChannelType->GetAcceptIncomingChanges())
  {
    // Ignore
    return true;
  }

  // Is server?
  if(GetReplicator()->GetRole() == Role::Server)
  {
    // They are not the change authority client for the replica whose channel they're trying to change?
    if(GetReplicatorId() != replica->GetAuthorityClientReplicatorId())
    {
      // Ignore
      return true;
    }
  }

  // Read replica channel
  bool result = replicaChannel->Deserialize(bitStream, ReplicationPhase::Change, timestamp);
  if(!result) // Unable?
  {
    //Assert(false);
    return false;
  }

  // Replica channel has not actually changed at all?
  if(!replicaChannel->HasChangedAtAll())
  {
    // Ignore
    return true;
  }

  // Determine if this replica channel's changes should be relayed
  bool shouldRelay = replicaChannel->ShouldRelay();

  // Handle changed replica channel property values
  // (Don't set properties last values to their current values when reacting to property changes if changes need to be relayed)
  // (The last values will be set when relaying outgoing property changes after this call)
  replicaChannel->ReactToPropertyChanges(timestamp, ReplicationPhase::Change, TransmissionDirection::Incoming, true, !shouldRelay);

  // Changes need to be relayed?
  if(shouldRelay)
  {
    // Get peer
    Peer* peer = GetReplicator()->GetPeer();

    // Get current frame ID
    uint64 frameId = peer->GetLocalFrameId();

    // Observe and replicate changes now
    bool result = replicaChannel->ObserveAndReplicateChanges(timestamp, frameId, false, false, true);
    Assert(result);
  }

  // Success
  return true;
}
bool ReplicatorLink::SendChange(ReplicaChannel* replicaChannel, Message& message)
{
  Assert(message.GetType() == ReplicatorMessageType::Change);

  // Get replica channel type
  ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

  // Get replica
  Replica* replica = replicaChannel->GetReplica();

  Assert(HasReplica(replica));

  // Get message channel
  MessageChannelId channelId = GetOutgoingReplicaChannel(replicaChannel);
  if(channelId == 0) // Unable?
  {
    Assert(false);
    return false;
  }

  // Send change message
  Status status;
  LinkPlugin::Send(status, message, (replicaChannelType->GetReliabilityMode() == ReliabilityMode::Reliable), channelId, false);
  if(status.Failed()) // Unable?
    return false;

  // Success
  return true;
}
bool ReplicatorLink::ReceiveChange(const Message& message)
{
  Assert(message.GetType() == ReplicatorMessageType::Change);

  // Get timestamp from message (may or may not be an accurate timestamp)
  TimeMs timestamp = message.GetTimestamp();

  // Deserialize replica channel change
  return DeserializeChange(message, timestamp);
}

bool ReplicatorLink::SendInterrupt(Message& message)
{
  Assert(GetReplicator()->GetRole() == Role::Server);
  Assert(message.GetType() == ReplicatorMessageType::Interrupt);

  // Send interrupt message
  Assert(GetCommandChannelId());
  Status status;
  LinkPlugin::Send(status, message, true, GetCommandChannelId());
  if(status.Failed()) // Unable?
    return false;

  // Success
  return true;
}

//
// Channel Helpers
//

bool ReplicatorLink::OpenAndSerializeForwardReplicaChannels(const Replica* replica, BitStream& bitStream)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Open outgoing message channel
    MessageChannelId channelId = OpenOutgoingReplicaChannel(replicaChannel);
    if(channelId == 0) // Unable?
    {
      Assert(false);
      return false;
    }

    // Write outgoing message channel ID
    bitStream.Write(channelId);
  }

  // Success
  return true;
}
bool ReplicatorLink::DeserializeAndSetForwardReplicaChannels(Replica* replica, const BitStream& bitStream)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Read incoming message channel ID
    MessageChannelId channelId;
    if(!bitStream.Read(channelId)) // Unable?
    {
      Assert(false);
      return false;
    }
    Assert(channelId != 0);

    // Set incoming message channel
    if(!SetIncomingReplicaChannel(channelId, replicaChannel)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // Success
  return true;
}

bool ReplicatorLink::OpenAndSerializeReverseReplicaChannels(const Replica* replica, BitStream& bitStream)
{
  Assert(GetReplicator()->GetRole() == Role::Client);

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Get replica channel type
    ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

    //    Replica channel has client change authority?
    // OR Replica channel type has dynamic change authority mode?
    if(replicaChannel->GetAuthority() == Authority::Client
    || replicaChannelType->GetAuthorityMode() == AuthorityMode::Dynamic)
    {
      // Open outgoing message channel
      MessageChannelId channelId = OpenOutgoingReplicaChannel(replicaChannel);
      if(channelId == 0) // Unable?
      {
        Assert(false);
        return false;
      }

      // Write outgoing message channel ID
      bitStream.Write(channelId);
    }
  }

  // Success
  return true;
}
bool ReplicatorLink::DeserializeAndSetReverseReplicaChannels(Replica* replica, const BitStream& bitStream)
{
  Assert(GetReplicator()->GetRole() == Role::Server);

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Get replica channel type
    ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

    //    Replica channel has client change authority?
    // OR Replica channel has dynamic change authority mode?
    if(replicaChannel->GetAuthority() == Authority::Client
    || replicaChannelType->GetAuthorityMode() == AuthorityMode::Dynamic)
    {
      // Read incoming message channel ID
      MessageChannelId channelId;
      if(!bitStream.Read(channelId)) // Unable?
      {
        Assert(false);
        return false;
      }
      Assert(channelId != 0);

      // Set incoming message channel
      if(!SetIncomingReplicaChannel(channelId, replicaChannel)) // Unable?
      {
        Assert(false);
        return false;
      }
    }
  }

  // Success
  return true;
}

MessageChannelId ReplicatorLink::GetCommandChannelId() const
{
  return mCommandChannelId;
}

void ReplicatorLink::CloseAllReplicaChannels(Replica* replica)
{
  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, replica->GetReplicaChannels().All())
  {
    // Close outgoing message channel (if any)
    CloseOutgoingReplicaChannel(replicaChannel);

    // Clear incoming message channel (if any)
    ClearIncomingReplicaChannel(replicaChannel);
  }
}

MessageChannelId ReplicatorLink::OpenOutgoingReplicaChannel(ReplicaChannel* replicaChannel)
{
  // (An outgoing message channel should not already be open for this replica channel)
  Assert(!GetOutgoingReplicaChannel(replicaChannel));

  // Get replica channel type
  ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

  // Open outgoing message channel
  OutMessageChannel* channel = LinkPlugin::GetLink()->OpenOutgoingChannel(replicaChannelType->GetTransferMode());
  if(!channel) // Unable?
  {
    Assert(false);
    return 0;
  }
  MessageChannelId channelId = channel->GetChannelId();
  Assert(channelId != 0);

  // Add outgoing message channel
  OutReplicaChannels::pointer_bool_pair result = mOutReplicaChannels.Insert(replicaChannel, channelId);
  if(!result.second) // Unable?
  {
    // Close outgoing message channel
    LinkPlugin::GetLink()->CloseOutgoingChannel(channelId);

    Assert(false);
    return 0;
  }

  // Success
  return channelId;
}
void ReplicatorLink::CloseOutgoingReplicaChannel(ReplicaChannel* replicaChannel)
{
  // Find outgoing message channel
  OutReplicaChannels::iterator iter = mOutReplicaChannels.FindIterator(replicaChannel);
  if(iter == mOutReplicaChannels.End()) // Unable?
    return;

  // Close outgoing message channel
  LinkPlugin::GetLink()->CloseOutgoingChannel(iter->second);

  // Remove outgoing message channel
  mOutReplicaChannels.Erase(iter);
}
MessageChannelId ReplicatorLink::GetOutgoingReplicaChannel(ReplicaChannel* replicaChannel) const
{
  // Get outgoing message channel
  return mOutReplicaChannels.FindValue(replicaChannel, MessageChannelId(0));
}

bool ReplicatorLink::SetIncomingReplicaChannel(MessageChannelId channelId, ReplicaChannel* replicaChannel)
{
  // (An incoming message channel should not already be set for this replica channel)
  Assert(!GetIncomingReplicaChannel(channelId));

  // Add incoming message channel (in regular map)
  InReplicaChannels::pointer_bool_pair result1 = mInReplicaChannels.Insert(channelId, replicaChannel);
  if(!result1.second) // Unable?
  {
    Assert(false);
    return false;
  }

  // Add incoming message channel (in flipped map)
  InReplicaChannelsFlipped::pointer_bool_pair result2 = mInReplicaChannelsFlipped.Insert(replicaChannel, channelId);
  if(!result2.second) // Unable?
  {
    // Remove incoming message channel (in regular map)
    mInReplicaChannels.Erase(result1.first);

    Assert(false);
    return false;
  }

  // Success
  return true;
}
void ReplicatorLink::ClearIncomingReplicaChannel(ReplicaChannel* replicaChannel)
{
  // Find incoming message channel (in flipped map)
  InReplicaChannelsFlipped::iterator iter = mInReplicaChannelsFlipped.FindIterator(replicaChannel);
  if(iter == mInReplicaChannelsFlipped.End()) // Unable?
    return;

  // Get incoming message channel ID
  MessageChannelId channelId = iter->second;

  // Remove incoming message channel (in flipped map)
  mInReplicaChannelsFlipped.Erase(iter);

  // Remove incoming message channel (in regular map)
  mInReplicaChannels.EraseValue(channelId);
}
ReplicaChannel* ReplicatorLink::GetIncomingReplicaChannel(MessageChannelId channelId) const
{
  // Get incoming message channel (in regular map)
  return mInReplicaChannels.FindValue(channelId, nullptr);
}

//
// Link Plugin Interface
//

void ReplicatorLink::OnConnectRequestSend(Message& message)
{
  // Not client?
  if(GetReplicator()->GetRole() != Role::Client)
  {
    Assert(false);
    return;
  }

  // Clear bits read (just in case it was modified)
  message.GetData().ClearBitsRead();

  // Process connect request
  Assert(message.GetType() == ProtocolMessageType::ConnectRequest);
  Assert(message.GetData().GetBitsRead() == 0);

  // Read connect request message
  ConnectRequestData connectRequestData;
  if(message.GetData().Read(connectRequestData)) // Successful?
  {
    // Store connect request data for users who need it later
    mLastConnectRequestData = connectRequestData;

    //[Client Callback]
    GetReplicator()->ClientOnConnectRequest(this, connectRequestData);
  }
}
void ReplicatorLink::OnConnectRequestReceive(Message& message)
{
  // Not server?
  if(GetReplicator()->GetRole() != Role::Server)
  {
    Assert(false);
    return;
  }

  // Clear bits read (just in case it was modified)
  message.GetData().ClearBitsRead();

  // Process connect request
  Assert(message.GetType() == ProtocolMessageType::ConnectRequest);
  Assert(message.GetData().GetBitsRead() == 0);

  // Read connect request message
  ConnectRequestData connectRequestData;
  if(message.GetData().Read(connectRequestData)) // Successful?
  {
    // Store connect request data for users who need it later
    mLastConnectRequestData = connectRequestData;

    //[Server Callback]
    BitStream extraData;
    Pair<bool, BitStream> result = GetReplicator()->ServerOnConnectRequest(this, connectRequestData);
    if(result.first) // Accepted?
    {
      // Assign replicator ID
      result.first = GetReplicator()->AssignReplicatorId(this);
      if(result.first) // Successful?
      {
        // Write replicator ID
        extraData.Write(GetReplicatorId());

        // Write extra data
        extraData.AppendAll(result.second);
      }
    }

    // Respond to connect request accordingly
    GetLink()->RespondToConnectRequest(result.first, extraData);
  }
}

void ReplicatorLink::OnConnectResponseSend(Message& message)
{
  // Not server?
  if(GetReplicator()->GetRole() != Role::Server)
  {
    Assert(false);
    return;
  }

  // Clear bits read (just in case it was modified)
  message.GetData().ClearBitsRead();

  // Process connect response
  Assert(message.GetType() == ProtocolMessageType::ConnectResponse);
  Assert(message.GetData().GetBitsRead() == 0);

  // Read connect response message
  ConnectResponseData connectResponseData;
  if(message.GetData().Read(connectResponseData)) // Successful?
  {
    // Store connect response data for users who need it later
    mLastConnectResponseData = connectResponseData;

    // Accepted?
    if(connectResponseData.mConnectResponse == ConnectResponse::Accept)
    {
      // Send all create context items
      GetReplicator()->mCreateContextCacher.RouteAllItems(Route(this));

      // Send all replica type items
      GetReplicator()->mReplicaTypeCacher.RouteAllItems(Route(this));

      // Send all emplace context items
      GetReplicator()->mEmplaceContextCacher.RouteAllItems(Route(this));
    }

    //[Server Callback]
    GetReplicator()->ServerOnConnectResponse(this, connectResponseData);

    // Denied?
    if(connectResponseData.mConnectResponse != ConnectResponse::Accept)
    {
      // Release replicator ID
      GetReplicator()->ReleaseReplicatorId(this);

      // Destroy link
      GetLink()->GetPeer()->DestroyLink(GetLink()->GetTheirIpAddress());
    }
  }
}
void ReplicatorLink::OnConnectResponseReceive(Message& message)
{
  // Not client?
  if(GetReplicator()->GetRole() != Role::Client)
  {
    Assert(false);
    return;
  }

  // Clear bits read (just in case it was modified)
  message.GetData().ClearBitsRead();

  // Process connect response
  Assert(message.GetType() == ProtocolMessageType::ConnectResponse);
  Assert(message.GetData().GetBitsRead() == 0);

  // Read connect response message
  ConnectResponseData connectResponseData;
  if(message.GetData().Read(connectResponseData)) // Successful?
  {
    // Accepted?
    if(connectResponseData.mConnectResponse == ConnectResponse::Accept)
    {
      // Read replicator ID
      ReplicatorId replicatorId;
      BitStream extraData = connectResponseData.mExtraData;
      if(extraData.Read(replicatorId))
      {
        // Set replicator ID
        Assert(replicatorId);
        GetReplicator()->SetReplicatorId(replicatorId);
      }
      else
        Assert(false);

      // Trim replicator ID (to get remaining "extra data", if any)
      extraData.TrimFront();

      // Create updated connect response data
      ConnectResponseData userConnectResponseData;
      userConnectResponseData.mIpAddress       = connectResponseData.mIpAddress;
      userConnectResponseData.mConnectResponse = connectResponseData.mConnectResponse;
      userConnectResponseData.mExtraData       = extraData;

      // Store connect response data for users who need it later
      mLastConnectResponseData = userConnectResponseData;

      //[Client Callback]
      BitStream confirmExtraData = GetReplicator()->ClientOnConnectResponse(this, userConnectResponseData);

      // Send connect confirmation message
      Assert(GetCommandChannelId());
      Status status;
      Message message(ReplicatorMessageType::ConnectConfirmation, ZeroMove(confirmExtraData));
      LinkPlugin::Send(status, message, true, GetCommandChannelId());
      if(status.Succeeded()) // Successful?
      {
        //[Client Callback]
        GetReplicator()->ClientOnConnectConfirmation(this, message.GetData());
      }
    }
    // Denied?
    else
    {
      // Store connect response data for users who need it later
      mLastConnectResponseData = connectResponseData;

      //[Client Callback]
      GetReplicator()->ClientOnConnectResponse(this, connectResponseData);

      // Clear replicator ID
      GetReplicator()->SetReplicatorId(0);

      // Destroy link
      GetLink()->GetPeer()->DestroyLink(GetLink()->GetTheirIpAddress());
    }
  }
}

void ReplicatorLink::OnDisconnectNoticeSend(Message& message)
{
  // Handle the disconnect notice send
  HandleDisconnectNotice(message, TransmissionDirection::Outgoing);
}
void ReplicatorLink::OnDisconnectNoticeReceive(Message& message)
{
  // Handle the disconnect notice receive
  HandleDisconnectNotice(message, TransmissionDirection::Incoming);
}
void ReplicatorLink::HandleDisconnectNotice(Message& message, TransmissionDirection::Enum direction)
{
  // Clear bits read (just in case it was modified)
  message.GetData().ClearBitsRead();

  // Process disconnect notice
  Assert(message.GetType() == ProtocolMessageType::DisconnectNotice);
  Assert(message.GetData().GetBitsRead() == 0);

  // Read disconnect notice message
  DisconnectNoticeData disconnectNoticeData;
  if(message.GetData().Read(disconnectNoticeData)) // Successful?
  {
    // Is server?
    if(GetReplicator()->GetRole() == Role::Server)
    {
      //[Server Callback]
      GetReplicator()->ServerOnDisconnectNotice(this, disconnectNoticeData, direction);

      // Release replicator ID
      GetReplicator()->ReleaseReplicatorId(this);

      // Destroy link
      GetLink()->GetPeer()->DestroyLink(GetLink()->GetTheirIpAddress());
    }
    // Is client?
    else if(GetReplicator()->GetRole() == Role::Client)
    {
      //[Client Callback]
      GetReplicator()->ClientOnDisconnectNotice(this, disconnectNoticeData, direction);

      // Clear replicator ID
      GetReplicator()->SetReplicatorId(0);

      // Destroy link
      GetLink()->GetPeer()->DestroyLink(GetLink()->GetTheirIpAddress());
    }
  }
}

/// Called after the link state is changed
void ReplicatorLink::OnStateChange(LinkState::Enum prevState)
{
  // Now connected?
  if(LinkPlugin::GetLink()->GetState() == LinkState::Connected)
  {
    Assert(mCommandChannelId == 0);

    // Open command channel
    OutMessageChannel* commandChannel = LinkPlugin::GetLink()->OpenOutgoingChannel(TransferMode::Ordered);
    if(!commandChannel) // Unable?
    {
      Assert(false);
      return;
    }

    // Set command channel ID
    mCommandChannelId = commandChannel->GetChannelId();
  }
  else if(LinkPlugin::GetLink()->GetState() == LinkState::Disconnected)
  {
    // Command channel is open?
    if(mCommandChannelId != 0)
    {
      // Close command channel
      LinkPlugin::GetLink()->CloseOutgoingChannel(mCommandChannelId);
    }
  }
}

void ReplicatorLink::OnPluginMessageReceive(MoveReference<Message> message, bool& continueProcessingCustomMessages)
{
  // Is link in any disconnected state?
  if(LinkPlugin::GetLink()->GetStatus() == LinkStatus::Disconnected)
  {
    // Ignore message
    return;
  }

  //
  // Process Message
  //

  // Clear bits read (just in case it was modified)
  message->GetData().ClearBitsRead();

  // Is server?
  if(GetReplicator()->GetRole() == Role::Server)
  {
    // Process message
    switch(message->GetType())
    {
    case ReplicatorMessageType::ConnectConfirmation:
      if(GetLink()->GetStatus() == LinkStatus::Connected)
      {
        //[Server Callback]
        GetReplicator()->ServerOnConnectConfirmation(this, message->GetData());
      }
      break;

    case ReplicatorMessageType::Change:
      ReceiveChange(message);
      break;

    case ReplicatorMessageType::ReverseReplicaChannels:
      ReceiveReverseReplicaChannels(message);
      break;

    default:
      Assert(false);
      break;
    }
  }
  // Is client?
  else if(GetReplicator()->GetRole() == Role::Client)
  {
    // Process message
    switch(message->GetType())
    {
    case ReplicatorMessageType::CreateContextItems:
      ReceiveCreateContextItems(message);
      break;

    case ReplicatorMessageType::ReplicaTypeItems:
      ReceiveReplicaTypeItems(message);
      break;

    case ReplicatorMessageType::EmplaceContextItems:
      ReceiveEmplaceContextItems(message);
      break;

    case ReplicatorMessageType::Spawn:
      ReceiveSpawn(message);
      break;

    case ReplicatorMessageType::Clone:
      ReceiveClone(message);
      break;

    case ReplicatorMessageType::Forget:
      ReceiveForget(message);
      break;

    case ReplicatorMessageType::Destroy:
      ReceiveDestroy(message);
      break;

    // case ReplicatorMessageType::Reset:
    //   ReceiveReset(message);
    //   break;

    case ReplicatorMessageType::Change:
      ReceiveChange(message);
      break;

    case ReplicatorMessageType::Interrupt:
      continueProcessingCustomMessages = false;
      break;

    default:
      Assert(false);
      break;
    }
  }
}

} // namespace Zero
