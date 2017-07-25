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
//                               ReplicaChannel                                    //
//---------------------------------------------------------------------------------//

ReplicaChannel::ReplicaChannel(const String& name, ReplicaChannelType* replicaChannelType)
  : mName(name),
    mReplicaChannelType(replicaChannelType),
    mReplica(nullptr),
    mIndexListLink(),
    mIndexListSize(nullptr),
    mIsNapping(false),
    mChangeFlag(false),
    mLastChangeTimestamp(cInvalidMessageTimestamp),
    mLastChangeFrameId(0),
    mAuthority(Authority::Server),
    mReplicaProperties()
{
  // Replica channel type provided?
  if(replicaChannelType)
  {
    // (Replica channel type should be valid)
    Assert(replicaChannelType->IsValid());

    // Apply replica channel type defaults
    SetAuthority(replicaChannelType->GetAuthorityDefault());
  }
  // Replica channel type not provided?
  else
    Assert(false);
}

ReplicaChannel::~ReplicaChannel()
{
  // (Should have been unscheduled when the operating replica was made invalid,
  // else there is a dangling replica channel held by the replica channel type)
  Assert(!IsScheduled());
}

bool ReplicaChannel::operator ==(const ReplicaChannel& rhs) const
{
  return mName == rhs.mName;
}
bool ReplicaChannel::operator !=(const ReplicaChannel& rhs) const
{
  return mName != rhs.mName;
}
bool ReplicaChannel::operator  <(const ReplicaChannel& rhs) const
{
  return mName < rhs.mName;
}
bool ReplicaChannel::operator ==(const String& rhs) const
{
  return mName == rhs;
}
bool ReplicaChannel::operator !=(const String& rhs) const
{
  return mName != rhs;
}
bool ReplicaChannel::operator  <(const String& rhs) const
{
  return mName < rhs;
}

//
// Operations
//

const String& ReplicaChannel::GetName() const
{
  return mName;
}

ReplicaChannelType* ReplicaChannel::GetReplicaChannelType() const
{
  return mReplicaChannelType;
}

bool ReplicaChannel::IsValid() const
{
  return (GetReplicator() != nullptr);
}

Replicator* ReplicaChannel::GetReplicator() const
{
  Replica* replica = GetReplica();
  return replica ? replica->GetReplicator() : nullptr;
}

void ReplicaChannel::SetReplica(Replica* replica)
{
  mReplica = replica;
}
Replica* ReplicaChannel::GetReplica() const
{
  return mReplica;
}

bool ReplicaChannel::IsScheduled() const
{
  return mIndexListSize != nullptr;
}

bool ReplicaChannel::IsAwake() const
{
  return !mIsNapping;
}
bool ReplicaChannel::IsNapping() const
{
  return mIsNapping;
}

void ReplicaChannel::WakeUp()
{
  // Unschedule channel from current index list (if any, awake/napping)
  mReplicaChannelType->UnscheduleChannel(this);

  // Wake up
  mIsNapping = false;

  // Reschedule channel onto appropriate index list (awake/napping)
  mReplicaChannelType->ScheduleChannel(this);
}
void ReplicaChannel::TakeNap()
{
  // Unschedule channel from current index list (if any, awake/napping)
  mReplicaChannelType->UnscheduleChannel(this);

  // Take nap
  mIsNapping = true;

  // Reschedule channel onto appropriate index list (awake/napping)
  mReplicaChannelType->ScheduleChannel(this);
}

bool ReplicaChannel::CheckChangeFlag()
{
  bool value = GetChangeFlag();
  SetChangeFlag(false);
  return value;
}

void ReplicaChannel::SetChangeFlag(bool changeFlag)
{
  mChangeFlag = changeFlag;
}
bool ReplicaChannel::GetChangeFlag() const
{
  return mChangeFlag;
}

bool ReplicaChannel::HasChangedAtAll() const
{
  forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
    if(replicaProperty->HasChangedAtAll())
      return true;

  return false;
}

bool ReplicaChannel::ObserveAndReplicateChanges(bool forceObservation, bool forceReplication, bool isRelay)
{
  // Get peer
  Peer* peer = GetReplicator()->GetPeer();

  // Get current time
  TimeMs timestamp = peer->GetLocalTime();

  // Get current frame ID
  uint64 frameId = peer->GetLocalFrameId();

  // Observe and replicate changes
  return ObserveAndReplicateChanges(timestamp, frameId, forceObservation, forceReplication, isRelay);
}
bool ReplicaChannel::ObserveAndReplicateChanges(TimeMs timestamp, uint64 frameId, bool forceObservation, bool forceReplication, bool isRelay)
{
  // Get replica channel type
  ReplicaChannelType* replicaChannelType = GetReplicaChannelType();

  // Get replica
  Replica* replica = GetReplica();

  // Get replicator
  Replicator* replicator = GetReplicator();

  // Don't force observation?
  if(!forceObservation)
  {
    // Don't detect outgoing changes for this replica?
    if(!replica->GetDetectOutgoingChanges())
      return true; // Success
  }

  // Is not being relayed?
  if(!isRelay)
  {
    // Replica channel authority does not match our role?
    if(uint(GetAuthority()) != uint(replicator->GetRole()))
    {
      // (Sanity check: We should only get here if our replica channel type's authority mode is dynamic,
      // otherwise this replica channel should not have even been scheduled for change observation in the first place)
      Assert(forceObservation ? true : (replicaChannelType->GetAuthorityMode() == AuthorityMode::Dynamic));

      // Success
      return true;
    }

    // Is client?
    if(replicator->GetRole() == Role::Client)
    {
      // Not this replica's change authority client?
      if(replicator->GetReplicatorId() != replica->GetAuthorityClientReplicatorId())
        return true; // Success
    }
  }

  // Change detected?
  if(ObserveForChange())
  {
    //    Replica channel type configured to serialize on change?
    // OR Force replication?
    if((replicaChannelType->GetSerializationFlags() & SerializationFlags::OnChange)
    || forceReplication)
    {
      // Create change route (route changes to everyone except the change authority client)
      Route changeRoute = isRelay
                        ? Route(RouteMode::Exclude, replica->GetAuthorityClientReplicatorId())
                        : Route::All;

      // Route replica channel change
      if(!replicator->RouteChange(this, changeRoute, timestamp)) // Unable?
      {
        // Failure
        Assert(false);
        return false;
      }
    }

    // Is napping?
    if(IsNapping())
    {
      // Wake up (we just detected a change)
      WakeUp();
    }

    // Set last change detected frame ID
    SetLastChangeFrameId(frameId);

    // Handle changed replica channel property values
    ReactToPropertyChanges(timestamp, ReplicationPhase::Change, TransmissionDirection::Outgoing);
  }
  // No change detected?
  else
  {
    // Is awake?
    if(IsAwake())
    {
      // Get frame duration since the last change was detected
      uint64 frameDurationSinceLastChange = (frameId - GetLastChangeFrameId());

      // Awake frame duration has elapsed since the last change was detected?
      if(frameDurationSinceLastChange >= replicaChannelType->GetAwakeDuration())
      {
        // Determine if this replica channel is allowed to nap
        bool allowedToNap = (replica->GetAllowNapping()
                          && replicaChannelType->GetAllowNapping());

        // Allowed to nap?
        if(allowedToNap)
        {
          // Take nap (we have not detected a change in a while)
          TakeNap();
        }
      }
    }
  }

  // Success
  return true;
}

void ReplicaChannel::SetLastChangeTimestamp(TimeMs lastChangeTimestamp)
{
  mLastChangeTimestamp = lastChangeTimestamp;
}
TimeMs ReplicaChannel::GetLastChangeTimestamp() const
{
  return mLastChangeTimestamp;
}

void ReplicaChannel::SetLastChangeFrameId(uint64 lastChangeFrameId)
{
  mLastChangeFrameId = lastChangeFrameId;
}
uint64 ReplicaChannel::GetLastChangeFrameId() const
{
  return mLastChangeFrameId;
}

bool ReplicaChannel::ShouldRelay() const
{
  //     Is server?
  // AND Replica channel has client change authority?
  // AND Replica channel type allows change relays?
  return GetReplicator()->GetRole() == Role::Server
      && GetAuthority() == Authority::Client
      && GetReplicaChannelType()->GetAllowRelay();
}

void ReplicaChannel::SetAuthority(Authority::Enum authority)
{
  //     Already valid?
  // AND Replica channel type is using a fixed authority mode?
  if(IsValid()
  && GetReplicaChannelType()->GetAuthorityMode() == AuthorityMode::Fixed)
  {
    // Unable to modify authority
    Error("Unable to modify Authority - Replica channel is already valid and replica channel type is using a fixed authority mode");
    return;
  }

  mAuthority = authority;
}
Authority::Enum ReplicaChannel::GetAuthority() const
{
  return mAuthority;
}

//
// Replica Property Management
//

bool ReplicaChannel::HasReplicaProperty(const String& replicaPropertyName) const
{
  return mReplicaProperties.Contains(replicaPropertyName);
}
const ReplicaProperty* ReplicaChannel::GetReplicaProperty(const String& replicaPropertyName) const
{
  const ReplicaProperty* result = mReplicaProperties.FindValue(replicaPropertyName, ReplicaPropertyPtr());
  return result;
}
ReplicaProperty* ReplicaChannel::GetReplicaProperty(const String& replicaPropertyName)
{
  const ReplicaProperty* result = mReplicaProperties.FindValue(replicaPropertyName, ReplicaPropertyPtr());
  return const_cast<ReplicaProperty*>(result);
}
const ReplicaPropertySet& ReplicaChannel::GetReplicaProperties() const
{
  return mReplicaProperties;
}
ReplicaPropertySet& ReplicaChannel::GetReplicaProperties()
{
  return mReplicaProperties;
}

ReplicaProperty* ReplicaChannel::AddReplicaProperty(ReplicaPropertyPtr replicaProperty)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify replica property configuration
    Error("Replica channel is already valid, unable to modify replica property configuration");
    return nullptr;
  }

  // Add replica property
  ReplicaPropertySet::pointer_bool_pair result = mReplicaProperties.Insert(replicaProperty);
  if(!result.second) // Unable?
  {
    Error("Replica channel already has a replica property with that name, unable to add replica property");
    return nullptr;
  }

  // Set the replica property's operating replica channel now that it's been added to us
  (*result.first)->SetReplicaChannel(this);

  // Success
  return (*result.first);
}
bool ReplicaChannel::RemoveReplicaProperty(const String& replicaPropertyName)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify replica property configuration
    Error("Replica channel is already valid, unable to modify replica property configuration");
    return nullptr;
  }

  // Remove replica property
  ReplicaPropertySet::pointer_bool_pair result = mReplicaProperties.EraseValue(replicaPropertyName);
  return result.second;
}

void ReplicaChannel::ClearReplicaProperties()
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify replica property configuration
    Error("Replica channel is already valid, unable to modify replica property configuration");
    return;
  }

  // Remove all replica properties
  mReplicaProperties.Clear();
}

void ReplicaChannel::ReactToPropertyChanges(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, TransmissionDirection::Enum direction, bool generateNotifications, bool setLastValues)
{
  // For all replica properties
  forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
  {
    // React to changes
    replicaProperty->ReactToChanges(timestamp, replicationPhase, direction, generateNotifications, setLastValues);
  }
}

//
// Internal
//

bool ReplicaChannel::ObserveForChange()
{
  // Get replica channel type
  ReplicaChannelType* replicaChannelType = GetReplicaChannelType();

  // For all replica properties
  bool propertyChanged = false;
  forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
  {
    // Property has changed?
    if(replicaProperty->HasChanged())
    {
      propertyChanged = true;
      break;
    }
  }

  // Channel has changed?
  switch(replicaChannelType->GetDetectionMode())
  {
  case DetectionMode::Assume:
    return true;

  case DetectionMode::Manual:
    return CheckChangeFlag();

  case DetectionMode::Automatic:
    return propertyChanged;

  case DetectionMode::Manumatic:
    return CheckChangeFlag() || propertyChanged;

  default:
    Assert(false);
    return false;
  }
}

bool ReplicaChannel::Serialize(BitStream& bitStream, ReplicationPhase::Enum replicationPhase, TimeMs timestamp) const
{
  // Get replica channel type
  ReplicaChannelType* replicaChannelType = GetReplicaChannelType();

  // (For the initialization replication phase we want to forcefully serialize all replica properties to ensure a valid initial value state)
  bool forceAll = (replicationPhase == ReplicationPhase::Initialization);

  //    Serialize all replica properties?
  // OR There is only a single replica property?
  // OR Force serialization of all replica properties?
  if(replicaChannelType->GetSerializationMode() == SerializationMode::All
  || GetReplicaProperties().Size() == 1
  || forceAll)
  {
    // For all replica properties
    forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
    {
      // Write replica property
      bool result = replicaProperty->Serialize(bitStream, replicationPhase, timestamp);
      if(!result) // Unable?
      {
        Assert(false);
        return false;
      }
    }
  }
  // Serialize changed?
  else
  {
    Assert(replicaChannelType->GetSerializationMode() == SerializationMode::Changed);

    // For all replica properties
    forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
    {
      // Write 'Has Changed?' Flag
      bool hasChanged = replicaProperty->HasChanged();
      bitStream.Write(hasChanged);
      if(hasChanged) // Has changed?
      {
        // Write replica property
        bool result = replicaProperty->Serialize(bitStream, replicationPhase, timestamp);
        if(!result) // Unable?
        {
          Assert(false);
          return false;
        }
      }
    }
  }

  // Success
  return true;
}
bool ReplicaChannel::Deserialize(const BitStream& bitStream, ReplicationPhase::Enum replicationPhase, TimeMs timestamp)
{
  // Get replica channel type
  ReplicaChannelType* replicaChannelType = GetReplicaChannelType();

  // (For the initialization replication phase we want to forcefully deserialize all replica properties to ensure a valid initial value state)
  bool forceAll = (replicationPhase == ReplicationPhase::Initialization);

  //    Serialize all replica properties?
  // OR There is only a single replica property?
  // OR Force serialization of all replica properties?
  if(replicaChannelType->GetSerializationMode() == SerializationMode::All
  || GetReplicaProperties().Size() == 1
  || forceAll)
  {
    // For all replica properties
    forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
    {
      // Read replica property
      bool result = replicaProperty->Deserialize(bitStream, replicationPhase, timestamp);
      if(!result) // Unable?
      {
        //Assert(false);
        return false;
      }
    }
  }
  // Serialize changed?
  else
  {
    Assert(replicaChannelType->GetSerializationMode() == SerializationMode::Changed);

    // For all replica properties
    forRange(ReplicaProperty* replicaProperty, GetReplicaProperties().All())
    {
      // Read 'Has Changed?' Flag
      bool hasChanged;
      if(!bitStream.Read(hasChanged)) // Unable?
      {
        Assert(false);
        return false;
      }
      if(hasChanged) // Has changed?
      {
        // Read replica property
        bool result = replicaProperty->Deserialize(bitStream, replicationPhase, timestamp);
        if(!result) // Unable?
        {
          //Assert(false);
          return false;
        }
      }
    }
  }

  // Success
  return true;
}

//---------------------------------------------------------------------------------//
//                             ReplicaChannelIndex                                 //
//---------------------------------------------------------------------------------//

ReplicaChannelIndex::ReplicaChannelIndex()
  : mChannelLists(),
    mChannelCount(0)
{
}

ReplicaChannelIndex::~ReplicaChannelIndex()
{
  // (Should be empty, else some replica channels weren't removed properly)
  Assert(IsEmpty());
}

bool ReplicaChannelIndex::IsEmpty() const
{
  return (mChannelCount == 0);
}

void ReplicaChannelIndex::CreateLists(uint count)
{
  // (Resizing the populated array can unsafely remove lists containing channels)
  Assert(IsEmpty());

  // Create specified number of lists
  mChannelLists.Reserve(count);
  for(size_t i = 0; i < count; ++i)
    mChannelLists.PushBack(ValueType(new PairType(0)));
}

ReplicaChannelList* ReplicaChannelIndex::GetList(size_t index)
{
  // Invalid list index?
  if(mChannelLists.Size() <= index)
    return nullptr;

  return &mChannelLists[index]->second;
}

size_t ReplicaChannelIndex::GetListCount() const
{
  return mChannelLists.Size();
}

void ReplicaChannelIndex::Insert(ReplicaChannel* channel)
{
  // Specified channel already has a pointer to a containing list's size?
  if(channel->mIndexListSize)
  {
    Assert(false);
    return;
  }

  // Find smallest list
  ValueType* smallestChannelList     = nullptr;
  size_t     smallestChannelListSize = std::numeric_limits<size_t>::max();
  forRange(ValueType& channelList, mChannelLists.All())
    if(channelList->first < smallestChannelListSize)
    {
      smallestChannelList     = &channelList;
      smallestChannelListSize = channelList->first;
    }

  // Unable to find smallest list?
  if(!smallestChannelList)
  {
    Assert(false);
    return;
  }

  // Insert channel into smallest list
  (*smallestChannelList)->second.PushBack(channel);

  // Store containing list size pointer on channel (used later when removing the channel)
  channel->mIndexListSize = &(*smallestChannelList)->first;

  // Update list channel count
  ++(*channel->mIndexListSize);

  // Update total channel count
  ++mChannelCount;
}

void ReplicaChannelIndex::Remove(ReplicaChannel* channel)
{
  // Specified channel does not have a pointer to a containing list's size?
  if(!channel->mIndexListSize)
  {
    Assert(false);
    return;
  }

  // Update list channel count
  --(*channel->mIndexListSize);

  // Remove channel from containing list
  ReplicaChannelList::Unlink(channel);

  // Clear containing list size pointer on channel (it is no longer stored in that list)
  channel->mIndexListSize = nullptr;

  // Update total channel count
  --mChannelCount;
}

//---------------------------------------------------------------------------------//
//                             ReplicaChannelType                                  //
//---------------------------------------------------------------------------------//

ReplicaChannelType::ReplicaChannelType(const String& name)
  : mName(name),
    mReplicator(nullptr)
{
  ResetConfig();
}

ReplicaChannelType::~ReplicaChannelType()
{
}

bool ReplicaChannelType::operator ==(const ReplicaChannelType& rhs) const
{
  return mName == rhs.mName;
}
bool ReplicaChannelType::operator !=(const ReplicaChannelType& rhs) const
{
  return mName != rhs.mName;
}
bool ReplicaChannelType::operator  <(const ReplicaChannelType& rhs) const
{
  return mName < rhs.mName;
}
bool ReplicaChannelType::operator ==(const String& rhs) const
{
  return mName == rhs;
}
bool ReplicaChannelType::operator !=(const String& rhs) const
{
  return mName != rhs;
}
bool ReplicaChannelType::operator  <(const String& rhs) const
{
  return mName < rhs;
}

//
// Operations
//

const String& ReplicaChannelType::GetName() const
{
  return mName;
}

bool ReplicaChannelType::IsValid() const
{
  return (GetReplicator() != nullptr);
}

void ReplicaChannelType::MakeValid(Replicator* replicator)
{
  // (Should not already be valid)
  Assert(!IsValid());

  // Create awake index lists to match our awake detection interval
  mAwakeChannelIndex.CreateLists(GetAwakeDetectionInterval());

  // Create napping index lists to match our nap detection interval
  mNappingChannelIndex.CreateLists(GetNapDetectionInterval());

  // Set operating replicator
  SetReplicator(replicator);

  // (Should now be considered valid)
  Assert(IsValid());
}

void ReplicaChannelType::SetReplicator(Replicator* replicator)
{
  mReplicator = replicator;
}
Replicator* ReplicaChannelType::GetReplicator() const
{
  return mReplicator;
}

void ReplicaChannelType::ObserveAndReplicateChanges()
{
  // Get peer
  Peer* peer = GetReplicator()->GetPeer();

  // Get current time
  TimeMs timestamp = peer->GetLocalTime();

  // Get current frame ID
  uint64 frameId = peer->GetLocalFrameId();

  // Observe awake replica channels
  ObserveAndReplicateChanges(mAwakeChannelIndex, timestamp, frameId);

  // Observe napping replica channels
  ObserveAndReplicateChanges(mNappingChannelIndex, timestamp, frameId);
}
void ReplicaChannelType::ObserveAndReplicateChanges(ReplicaChannelIndex& replicaChannelIndex, TimeMs timestamp, uint64 frameId)
{
  // (Should be valid)
  Assert(IsValid());

  // Replica channels of this type should not detect outgoing changes?
  if(!GetDetectOutgoingChanges())
  {
    // Nothing to observe
    return;
  }

  // Get index list count
  uint64 listCount = replicaChannelIndex.GetListCount();
  if(listCount == 0) // Empty?
    return;

  // Get scheduled replica channel list from index
  ReplicaChannelList* scheduledList = replicaChannelIndex.GetList(size_t(frameId % listCount));

  // For all scheduled replica channels in the list
  ReplicaChannelList::range scheduledChannels = scheduledList->All();
  while(!scheduledChannels.Empty())
  {
    // Get scheduled replica channel
    ReplicaChannel& scheduledChannel = scheduledChannels.Front();

    // Advance
    // (We do this here because observing the scheduled replica channel below may cause it's node to be removed and invalidate our list traversal)
    scheduledChannels.PopFront();

    // Observe the scheduled replica channel
    scheduledChannel.ObserveAndReplicateChanges(timestamp, frameId);
  }
}

void ReplicaChannelType::ScheduleChannel(ReplicaChannel* channel)
{
  // (Should be valid)
  Assert(IsValid());

  // Get replica
  Replica* replica = channel->GetReplica();

  // Invalid replica?
  if(!replica || !replica->IsValid())
    return;

  // Get replicator
  Replicator* replicator = GetReplicator();

  // Replica channels of this type should not detect outgoing changes?
  if(!GetDetectOutgoingChanges())
  {
    // Don't schedule change observations for this channel
    return;
  }

  // (Note: We never schedule channel relays, so we don't consider the relay case here.
  // Instead we replicate channel relays immediately upon receiving an incoming change)

  //     Replica channel authority does not match our role?
  // AND This replica channel type uses a fixed authority mode?
  if(uint(channel->GetAuthority()) != uint(replicator->GetRole())
  && GetAuthorityMode() == AuthorityMode::Fixed)
  {
    // Don't schedule change observations for this channel
    return;
  }

  // Already scheduled?
  if(channel->IsScheduled())
  {
    // (Can't be scheduled more than once at a time, duplicates are an error)
    Assert(false);
    return;
  }

  // Is awake?
  if(!channel->IsNapping())
  {
    // Add to awake channel index
    mAwakeChannelIndex.Insert(channel);
  }
  // Is napping?
  else
  {
    // Add to napping channel index
    mNappingChannelIndex.Insert(channel);
  }
}
void ReplicaChannelType::UnscheduleChannel(ReplicaChannel* channel)
{
  // (Should be valid)
  Assert(IsValid());

  // Already unscheduled?
  if(!channel->IsScheduled())
  {
    // (Nothing to do)
    return;
  }

  // Is awake?
  if(!channel->IsNapping())
  {
    // Remove from awake channel index
    mAwakeChannelIndex.Remove(channel);
  }
  // Is napping?
  else
  {
    // Remove from napping channel index
    mNappingChannelIndex.Remove(channel);
  }
}

//
// Configuration
//

void ReplicaChannelType::ResetConfig()
{
  SetDetectOutgoingChanges();
  SetAcceptIncomingChanges();
  SetNotifyOnOutgoingPropertyChange();
  SetNotifyOnIncomingPropertyChange();
  SetAuthorityMode();
  SetAuthorityDefault();
  SetAllowRelay();
  SetAllowNapping();
  SetAwakeDuration();
  SetDetectionMode();
  SetAwakeDetectionInterval();
  SetNapDetectionInterval();
  SetSerializationFlags();
  SetSerializationMode();
  SetReliabilityMode();
  SetTransferMode();
  SetAccurateTimestampOnChange();
}

void ReplicaChannelType::SetDetectOutgoingChanges(bool detectOutgoingChanges)
{
  mDetectOutgoingChanges = detectOutgoingChanges;
}
bool ReplicaChannelType::GetDetectOutgoingChanges() const
{
  return mDetectOutgoingChanges;
}

void ReplicaChannelType::SetAcceptIncomingChanges(bool acceptIncomingChanges)
{
  mAcceptIncomingChanges = acceptIncomingChanges;
}
bool ReplicaChannelType::GetAcceptIncomingChanges() const
{
  return mAcceptIncomingChanges;
}

void ReplicaChannelType::SetNotifyOnOutgoingPropertyChange(bool notifyOnOutgoingPropertyChange)
{
  mNotifyOnOutgoingPropertyChange = notifyOnOutgoingPropertyChange;
}
bool ReplicaChannelType::GetNotifyOnOutgoingPropertyChange() const
{
  return mNotifyOnOutgoingPropertyChange;
}

void ReplicaChannelType::SetNotifyOnIncomingPropertyChange(bool notifyOnIncomingPropertyChange)
{
  mNotifyOnIncomingPropertyChange = notifyOnIncomingPropertyChange;
}
bool ReplicaChannelType::GetNotifyOnIncomingPropertyChange() const
{
  return mNotifyOnIncomingPropertyChange;
}

void ReplicaChannelType::SetAuthorityMode(AuthorityMode::Enum authorityMode)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify configuration
    Error("ReplicaChannelType is already valid, unable to modify configuration");
    return;
  }

  mAuthorityMode = authorityMode;
}
AuthorityMode::Enum ReplicaChannelType::GetAuthorityMode() const
{
  return mAuthorityMode;
}

void ReplicaChannelType::SetAuthorityDefault(Authority::Enum authorityDefault)
{
  mAuthorityDefault = authorityDefault;
}
Authority::Enum ReplicaChannelType::GetAuthorityDefault() const
{
  return mAuthorityDefault;
}

void ReplicaChannelType::SetAllowRelay(bool allowRelay)
{
  mAllowRelay = allowRelay;
}
bool ReplicaChannelType::GetAllowRelay() const
{
  return mAllowRelay;
}

void ReplicaChannelType::SetAllowNapping(bool allowNapping)
{
  mAllowNapping = allowNapping;
}
bool ReplicaChannelType::GetAllowNapping() const
{
  return mAllowNapping;
}

void ReplicaChannelType::SetAwakeDuration(uint awakeDuration)
{
  // (Clamping within arbitrary, useful values)
  mAwakeDuration = Math::Clamp(awakeDuration, uint(1), uint(1000));
}
uint ReplicaChannelType::GetAwakeDuration() const
{
  return mAwakeDuration;
}

void ReplicaChannelType::SetDetectionMode(DetectionMode::Enum detectionMode)
{
  mDetectionMode = detectionMode;
}
DetectionMode::Enum ReplicaChannelType::GetDetectionMode() const
{
  return mDetectionMode;
}

void ReplicaChannelType::SetAwakeDetectionInterval(uint awakeDetectionInterval)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify configuration
    Error("ReplicaChannelType is already valid, unable to modify configuration");
    return;
  }

  // (Clamping within arbitrary, useful values)
  mAwakeDetectionInterval = Math::Clamp(awakeDetectionInterval, uint(1), uint(100));
}
uint ReplicaChannelType::GetAwakeDetectionInterval() const
{
  return mAwakeDetectionInterval;
}

void ReplicaChannelType::SetNapDetectionInterval(uint napDetectionInterval)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify configuration
    Error("ReplicaChannelType is already valid, unable to modify configuration");
    return;
  }

  // (Clamping within arbitrary, useful values)
  mNapDetectionInterval = Math::Clamp(napDetectionInterval, uint(1), uint(100));
}
uint ReplicaChannelType::GetNapDetectionInterval() const
{
  return mNapDetectionInterval;
}

void ReplicaChannelType::SetSerializationFlags(uint serializationFlags)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify configuration
    Error("ReplicaChannelType is already valid, unable to modify configuration");
    return;
  }

  mSerializationFlags = SerializationFlags::Enum(serializationFlags);
}
uint ReplicaChannelType::GetSerializationFlags() const
{
  return mSerializationFlags;
}

void ReplicaChannelType::SetSerializationMode(SerializationMode::Enum serializationMode)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify configuration
    Error("ReplicaChannelType is already valid, unable to modify configuration");
    return;
  }

  mSerializationMode = serializationMode;
}
SerializationMode::Enum ReplicaChannelType::GetSerializationMode() const
{
  return mSerializationMode;
}

void ReplicaChannelType::SetReliabilityMode(ReliabilityMode::Enum reliabilityMode)
{
  mReliabilityMode = reliabilityMode;
}
ReliabilityMode::Enum ReplicaChannelType::GetReliabilityMode() const
{
  return mReliabilityMode;
}

void ReplicaChannelType::SetTransferMode(TransferMode::Enum transferMode)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify configuration
    Error("ReplicaChannelType is already valid, unable to modify configuration");
    return;
  }

  mTransferMode = transferMode;
}
TransferMode::Enum ReplicaChannelType::GetTransferMode() const
{
  return mTransferMode;
}

void ReplicaChannelType::SetAccurateTimestampOnChange(bool accurateTimestampOnChange)
{
  mAccurateTimestampOnChange = accurateTimestampOnChange;
}
bool ReplicaChannelType::GetAccurateTimestampOnChange() const
{
  return mAccurateTimestampOnChange;
}

} // namespace Zero
