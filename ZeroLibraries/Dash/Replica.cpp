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
//                                  Replica                                        //
//---------------------------------------------------------------------------------//

Replica::Replica()
  : mCreateContext(),
    mReplicaType(),
    mReplicator(nullptr),
    mReplicaId(0),
    mEmplaceContext(),
    mEmplaceId(0),
    mIsCloned(false),
    mInitializationTimestamp(cInvalidMessageTimestamp),
    mLastChangeTimestamp(cInvalidMessageTimestamp),
    mUninitializationTimestamp(cInvalidMessageTimestamp),
    mDetectOutgoingChanges(false),
    mAcceptIncomingChanges(false),
    mAllowNapping(false),
    mAuthorityClientReplicatorId(0),
    mAccurateTimestampOnInitialization(false),
    mAccurateTimestampOnChange(false),
    mAccurateTimestampOnUninitialization(false),
    mReplicaChannels(),
    mUserData(nullptr)
{
  ResetConfig();
}
Replica::Replica(const CreateContext& createContext, const ReplicaType& replicaType)
  : mCreateContext(),
    mReplicaType(),
    mReplicator(nullptr),
    mReplicaId(0),
    mEmplaceContext(),
    mEmplaceId(0),
    mIsCloned(false),
    mInitializationTimestamp(cInvalidMessageTimestamp),
    mLastChangeTimestamp(cInvalidMessageTimestamp),
    mUninitializationTimestamp(cInvalidMessageTimestamp),
    mDetectOutgoingChanges(false),
    mAcceptIncomingChanges(false),
    mAllowNapping(false),
    mAuthorityClientReplicatorId(0),
    mAccurateTimestampOnInitialization(false),
    mAccurateTimestampOnChange(false),
    mAccurateTimestampOnUninitialization(false),
    mReplicaChannels(),
    mUserData(nullptr)
{
  ResetConfig();
  bool result = SetCreateContext(createContext);
  Assert(result);
  result = SetReplicaType(replicaType);
  Assert(result);
}

Replica::~Replica()
{
  // Deleting a valid replica is an error which will leave the replicator with a dangling replica pointer
  // It is imperative that you first inform the replicator with commands like ForgetReplica() and DestroyReplica()
  // This will unregister the replica from the replicator, at which time it will be safe to delete the now invalid replica
  ErrorIf(IsValid());
}

bool Replica::operator ==(const Replica& rhs) const
{
  return GetReplicaId() == rhs.GetReplicaId();
}
bool Replica::operator !=(const Replica& rhs) const
{
  return GetReplicaId() != rhs.GetReplicaId();
}
bool Replica::operator  <(const Replica& rhs) const
{
  return GetReplicaId() < rhs.GetReplicaId();
}
bool Replica::operator ==(const ReplicaId& rhs) const
{
  return GetReplicaId() == rhs;
}
bool Replica::operator !=(const ReplicaId& rhs) const
{
  return GetReplicaId() != rhs;
}
bool Replica::operator  <(const ReplicaId& rhs) const
{
  return GetReplicaId() < rhs;
}

//
// Operations
//

bool Replica::IsInvalid() const
{
  return !IsValid();
}
bool Replica::IsValid() const
{
  return GetReplicator() ? true : false;
}
bool Replica::IsLive() const
{
  return IsValid() && GetReplicaId() != 0;
}
bool Replica::IsEmplaced() const
{
  return GetEmplaceId() != 0;
}
bool Replica::IsSpawned() const
{
  return !IsEmplaced();
}
bool Replica::IsCloned() const
{
  return mIsCloned;
}

Replicator* Replica::GetReplicator() const
{
  return mReplicator;
}

bool Replica::SetCreateContext(const CreateContext& createContext)
{
  // Not invalid?
  if(!IsInvalid())
  {
    Error("Unable to set CreateContext - Replica must be invalid");
    return false;
  }

  // Set create context
  mCreateContext = createContext;

  // Success
  return true;
}
const CreateContext& Replica::GetCreateContext() const
{
  return mCreateContext;
}

bool Replica::SetReplicaType(const ReplicaType& replicaType)
{
  // Not invalid?
  if(!IsInvalid())
  {
    Error("Unable to set ReplicaType - Replica must be invalid");
    return false;
  }

  // Set replica type
  mReplicaType = replicaType;

  // Success
  return true;
}
const ReplicaType& Replica::GetReplicaType() const
{
  return mReplicaType;
}

void Replica::SetReplicaId(ReplicaId replicaId)
{
  mReplicaId = replicaId;
}
ReplicaId Replica::GetReplicaId() const
{
  return mReplicaId;
}

const EmplaceContext& Replica::GetEmplaceContext() const
{
  return mEmplaceContext;
}
EmplaceId Replica::GetEmplaceId() const
{
  return mEmplaceId;
}

void Replica::SetUserData(void* userData)
{
  mUserData = userData;
}
void* Replica::GetUserData() const
{
  return mUserData;
}

void Replica::SetInitializationTimestamp(TimeMs initializationTimestamp)
{
  mInitializationTimestamp = initializationTimestamp;
}
TimeMs Replica::GetInitializationTimestamp() const
{
  return mInitializationTimestamp;
}

void Replica::SetLastChangeTimestamp(TimeMs lastChangeTimestamp)
{
  mLastChangeTimestamp = lastChangeTimestamp;
}
TimeMs Replica::GetLastChangeTimestamp() const
{
  return mLastChangeTimestamp;
}

void Replica::SetUninitializationTimestamp(TimeMs uninitializationTimestamp)
{
  mUninitializationTimestamp = uninitializationTimestamp;
}
TimeMs Replica::GetUninitializationTimestamp() const
{
  return mUninitializationTimestamp;
}

void Replica::WakeUp()
{
  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, GetReplicaChannels().All())
    replicaChannel->WakeUp();
}
void Replica::TakeNap()
{
  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, GetReplicaChannels().All())
    replicaChannel->TakeNap();
}

bool Replica::IsAwake() const
{
  // For all replica channels
  forRange(const ReplicaChannel* replicaChannel, GetReplicaChannels().All())
    if(replicaChannel->IsAwake())
      return true;

  return false;
}
bool Replica::IsNapping() const
{
  return !IsAwake();
}

void Replica::ResetConfig()
{
  SetDetectOutgoingChanges();
  SetAcceptIncomingChanges();
  SetAllowNapping();
  SetAuthorityClientReplicatorId();
  SetAccurateTimestampOnInitialization();
  SetAccurateTimestampOnChange();
  SetAccurateTimestampOnUninitialization();
}

void Replica::SetDetectOutgoingChanges(bool detectOutgoingChanges)
{
  mDetectOutgoingChanges = detectOutgoingChanges;
}
bool Replica::GetDetectOutgoingChanges() const
{
  return mDetectOutgoingChanges;
}

void Replica::SetAcceptIncomingChanges(bool acceptIncomingChanges)
{
  mAcceptIncomingChanges = acceptIncomingChanges;
}
bool Replica::GetAcceptIncomingChanges() const
{
  return mAcceptIncomingChanges;
}

void Replica::SetAllowNapping(bool allowNapping)
{
  mAllowNapping = allowNapping;
}
bool Replica::GetAllowNapping() const
{
  return mAllowNapping;
}

void Replica::SetAuthorityClientReplicatorId(ReplicatorId authorityClientReplicatorId)
{
  mAuthorityClientReplicatorId = authorityClientReplicatorId;
}
ReplicatorId Replica::GetAuthorityClientReplicatorId() const
{
  return mAuthorityClientReplicatorId;
}

void Replica::SetAccurateTimestampOnInitialization(bool accurateTimestampOnInitialization)
{
  mAccurateTimestampOnInitialization = accurateTimestampOnInitialization;
}
bool Replica::GetAccurateTimestampOnInitialization() const
{
  return mAccurateTimestampOnInitialization;
}

void Replica::SetAccurateTimestampOnChange(bool accurateTimestampOnChange)
{
  mAccurateTimestampOnChange = accurateTimestampOnChange;
}
bool Replica::GetAccurateTimestampOnChange() const
{
  return mAccurateTimestampOnChange;
}

void Replica::SetAccurateTimestampOnUninitialization(bool accurateTimestampOnUninitialization)
{
  mAccurateTimestampOnUninitialization = accurateTimestampOnUninitialization;
}
bool Replica::GetAccurateTimestampOnUninitialization() const
{
  return mAccurateTimestampOnUninitialization;
}

//
// Replica Channel Management
//

bool Replica::HasReplicaChannel(const String& replicaChannelName) const
{
  return mReplicaChannels.Contains(replicaChannelName);
}
const ReplicaChannel* Replica::GetReplicaChannel(const String& replicaChannelName) const
{
  const ReplicaChannel* result = mReplicaChannels.FindValue(replicaChannelName, ReplicaChannelPtr());
  return result;
}
ReplicaChannel* Replica::GetReplicaChannel(const String& replicaChannelName)
{
  const ReplicaChannel* result = mReplicaChannels.FindValue(replicaChannelName, ReplicaChannelPtr());
  return const_cast<ReplicaChannel*>(result);
}
const ReplicaChannelSet& Replica::GetReplicaChannels() const
{
  return mReplicaChannels;
}
ReplicaChannelSet& Replica::GetReplicaChannels()
{
  return mReplicaChannels;
}

ReplicaChannel* Replica::AddReplicaChannel(ReplicaChannelPtr replicaChannel)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify replica channel configuration
    Error("Replica is already valid, unable to modify replica channel configuration");
    return nullptr;
  }

  // Add replica channel
  ReplicaChannelSet::pointer_bool_pair result = mReplicaChannels.Insert(replicaChannel);
  if(!result.second) // Unable?
  {
    Error("Replica already has a replica channel with that name, unable to add replica channel");
    return nullptr;
  }

  // Set the replica channel's operating replica now that it's been added to us
  (*result.first)->SetReplica(this);

  // Success
  return (*result.first);
}
bool Replica::RemoveReplicaChannel(const String& replicaChannelName)
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify replica channel configuration
    Error("Replica is already valid, unable to modify replica channel configuration");
    return nullptr;
  }

  // Remove replica channel
  ReplicaChannelSet::pointer_bool_pair result = mReplicaChannels.EraseValue(replicaChannelName);
  return result.second;
}

void Replica::ClearReplicaChannels()
{
  // Already valid?
  if(IsValid())
  {
    // Unable to modify replica channel configuration
    Error("Replica is already valid, unable to modify replica channel configuration");
    return;
  }

  // Remove all replica channels
  mReplicaChannels.Clear();
}

//
// Internal
//

bool Replica::UsesReverseReplicaChannels() const
{
  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, GetReplicaChannels().All())
  {
    // Get replica channel type
    ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

    //    Replica channel has client change authority?
    // OR Replica channel type has dynamic change authority mode?
    if(replicaChannel->GetAuthority() == Authority::Client
    || replicaChannelType->GetAuthorityMode() == AuthorityMode::Dynamic)
      return true; // Uses reverse replica channels
  }

  // Does not use reverse replica channels
  return false;
}

void Replica::SetIsCloned(bool isCloned)
{
  mIsCloned = isCloned;
}

void Replica::SetReplicator(Replicator* replicator)
{
  mReplicator = replicator;
}

void Replica::SetEmplaceContext(const EmplaceContext& emplaceContext)
{
  mEmplaceContext = emplaceContext;
}

void Replica::SetEmplaceId(EmplaceId emplaceId)
{
  mEmplaceId = emplaceId;
}

void Replica::ReactToChannelPropertyChanges(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, TransmissionDirection::Enum direction, bool generateNotifications, bool setLastValues)
{
  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, GetReplicaChannels().All())
  {
    // React to property changes
    replicaChannel->ReactToPropertyChanges(timestamp, replicationPhase, direction, generateNotifications, setLastValues);
  }
}

} // namespace Zero
