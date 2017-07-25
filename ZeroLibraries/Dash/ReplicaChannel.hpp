///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               ReplicaChannel                                    //
//---------------------------------------------------------------------------------//

/// Replica Channel
/// Manages the replication of a set of properties
class ReplicaChannel
{
public:
  /// Constructor
  ReplicaChannel(const String& name, ReplicaChannelType* replicaChannelType);

  /// Destructor
  REPLICA_CHANNEL_VIRTUAL ~ReplicaChannel();

  /// Comparison Operators (compares names)
  bool operator ==(const ReplicaChannel& rhs) const;
  bool operator !=(const ReplicaChannel& rhs) const;
  bool operator  <(const ReplicaChannel& rhs) const;
  bool operator ==(const String& rhs) const;
  bool operator !=(const String& rhs) const;
  bool operator  <(const String& rhs) const;

  //
  // Operations
  //

  /// Replica channel name
  const String& GetName() const;

  /// Operating replica channel type
  ReplicaChannelType* GetReplicaChannelType() const;

  /// Returns true if the replica channel is valid (added to a valid replica), else false
  bool IsValid() const;

  /// Operating replicator (set if the replica channel is valid)
  Replicator* GetReplicator() const;

  /// Operating replica (set if the replica channel is valid)
  void SetReplica(Replica* replica);
  Replica* GetReplica() const;

  /// Returns true if this replica channel is scheduled for change observation, else false
  bool IsScheduled() const;

  /// Returns true if the replica channel is currently awake (not napping), else false
  bool IsAwake() const;
  /// Returns true if the replica channel is currently napping (performing change detection on longer intervals), else false
  bool IsNapping() const;

  /// Forces the replica channel to stop napping immediately
  void WakeUp();
  /// Forces the replica channel to start napping immediately
  void TakeNap();

  /// Clears the manual change flag value and returns it's previous value
  bool CheckChangeFlag();

  /// Manual change flag (checked upon manual change observation)
  void SetChangeFlag(bool changeFlag = true);
  bool GetChangeFlag() const;

  /// Returns true if any replica property has changed at all since the last observation, else false
  bool HasChangedAtAll() const;

  /// Observes the replica channel and replicates any changes (if configured to do so)
  /// Returns true if successful, else false
  bool ObserveAndReplicateChanges(bool forceObservation = false, bool forceReplication = false, bool isRelay = false);
  bool ObserveAndReplicateChanges(TimeMs timestamp, uint64 frameId, bool forceObservation = false, bool forceReplication = false, bool isRelay = false);

  /// Timestamp indicating when this replica channel was last changed, else cInvalidMessageTimestamp
  /// (Set immediately after a change is observed on any replica property)
  void SetLastChangeTimestamp(TimeMs lastChangeTimestamp);
  TimeMs GetLastChangeTimestamp() const;

  /// Frame ID when the last change was observed on this replica channel
  /// (Note: This is only designed to be used by outgoing change detection intervals)
  void SetLastChangeFrameId(uint64 lastChangeFrameId);
  uint64 GetLastChangeFrameId() const;

  /// Returns true if the replica channel should have it's observed changes relayed to incidental peers, else false
  bool ShouldRelay() const;

  /// Controls which replicator has the authority to observe and replicate property changes
  /// (If using AuthorityMode::Dynamic) Authority may be modified at any time, even after a replica is made valid
  /// (If using AuthorityMode::Fixed) Authority may be modified only before a replica is made valid
  /// Server Authority: Indicates only the server is allowed to observe and replicate property changes
  /// Client Authority: Indicates both the client and server are allowed to observe and replicate property changes
  /// Only a single client, specified by Replica::AuthorityClientReplicatorId, may possess client authority at any given time
  /// The server is still responsible for relaying contained property changes to other clients, but will not replicate contained property changes back to the authority client
  /// However, the server is also still responsible for other replication commands (such as object creation/destruction), and these WILL be replicated to the authority client
  void SetAuthority(Authority::Enum authority);
  Authority::Enum GetAuthority() const;

  //
  // Replica Property Management
  //

  /// Returns true if the replica channel has the specified replica property, else false
  bool HasReplicaProperty(const String& replicaPropertyName) const;
  /// Returns the specified replica property, else nullptr
  const ReplicaProperty* GetReplicaProperty(const String& replicaPropertyName) const;
  ReplicaProperty* GetReplicaProperty(const String& replicaPropertyName);
  /// Returns all replica properties managed by the replica channel
  const ReplicaPropertySet& GetReplicaProperties() const;
  ReplicaPropertySet& GetReplicaProperties();

  /// Adds the replica property
  /// (Cannot be modified after the replica channel has been made valid)
  /// Returns the replica property if successful, else nullptr (a replica property of that name already exists)
  ReplicaProperty* AddReplicaProperty(ReplicaPropertyPtr replicaProperty);
  /// Removes the specified replica property
  /// (Cannot be modified after the replica channel has been made valid)
  /// Returns true if successful, else false (a replica property of that name could not be found)
  bool RemoveReplicaProperty(const String& replicaPropertyName);

  /// Removes all replica properties
  /// (Cannot be modified after the replica channel has been made valid)
  void ClearReplicaProperties();

  /// Reacts to all replica properties that have been legitimately changed, determined using comparisons, since this function was last called
  void ReactToPropertyChanges(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, TransmissionDirection::Enum direction, bool generateNotifications = true, bool setLastValues = true);

  //
  // Internal
  //

  /// Observes the replica channel for changes
  /// Returns true if a change was detected, else false
  bool ObserveForChange();

  /// Serializes the replica channel
  /// Returns true if successful, else false
  bool Serialize(BitStream& bitStream, ReplicationPhase::Enum replicationPhase, TimeMs timestamp) const;
  /// Deserializes the replica channel
  /// Returns true if successful, else false
  bool Deserialize(const BitStream& bitStream, ReplicationPhase::Enum replicationPhase, TimeMs timestamp);

  /// Data
  String               mName;                /// Replica channel name
  ReplicaChannelType*  mReplicaChannelType;  /// Operating replica channel type
  Replica*             mReplica;             /// Operating replica
  Link<ReplicaChannel> mIndexListLink;       /// Replica channel index list link (may be null)
  size_t*              mIndexListSize;       /// Replica channel index list size (may be null)
  bool                 mIsNapping;           /// Is the replica channel napping?
  bool                 mChangeFlag;          /// Manual change flag
  TimeMs               mLastChangeTimestamp; /// Timestamp indicating when this replica channel was last changed (on any replica property)
  uint64               mLastChangeFrameId;   /// Frame ID of the last detected change
  Authority::Enum      mAuthority;           /// Change authority
  ReplicaPropertySet   mReplicaProperties;   /// Replica properties
};

/// Typedefs
typedef UniquePointer<ReplicaChannel>                                       ReplicaChannelPtr;
typedef ArraySet< ReplicaChannelPtr, PointerSortPolicy<ReplicaChannelPtr> > ReplicaChannelSet;
typedef InList<ReplicaChannel, &ReplicaChannel::mIndexListLink>             ReplicaChannelList;

//---------------------------------------------------------------------------------//
//                             ReplicaChannelIndex                                 //
//---------------------------------------------------------------------------------//

/// Replica Channel Index
/// Evenly distributes a collection of replica channels
class ReplicaChannelIndex
{
public:
  /// Typedefs
  typedef ReplicaChannelList      ListType;
  typedef Pair<size_t, ListType>  PairType;
  typedef UniquePointer<PairType> ValueType;
  typedef Array<ValueType>        ArrayType;

  /// Constructor
  ReplicaChannelIndex();

  /// Destructor
  ~ReplicaChannelIndex();

  /// Returns true if the index is empty (contains no replica channels), else false
  bool IsEmpty() const;

  /// Creates the specified number of lists
  /// (Not safe to call if the index is populated)
  void CreateLists(uint count);

  /// Returns the list at the specified index, else nullptr
  ReplicaChannelList* GetList(size_t index);

  /// Returns the number of lists in the index
  size_t GetListCount() const;

  /// Inserts the replica channel into the smallest internal list
  /// (Linear operation with respect to list count, checks every list's precomputed size and selects the smallest list to insert into)
  void Insert(ReplicaChannel* channel);

  /// Removes the replica channel from it's internal list
  /// (Constant operation, simply unlinks the intrusive list node and decrements the list's size value)
  void Remove(ReplicaChannel* channel);

  /// Data
  ArrayType mChannelLists; /// Replica channel lists
  size_t    mChannelCount; /// Replica channel count
};

//---------------------------------------------------------------------------------//
//                             ReplicaChannelType                                  //
//---------------------------------------------------------------------------------//

/// Replica Channel Type
/// Configures the replication of a set of properties
class ReplicaChannelType
{
public:
  /// Constructor
  ReplicaChannelType(const String& name);

  /// Destructor
  REPLICA_CHANNEL_TYPE_VIRTUAL ~ReplicaChannelType();

  /// Comparison Operators (compares names)
  bool operator ==(const ReplicaChannelType& rhs) const;
  bool operator !=(const ReplicaChannelType& rhs) const;
  bool operator  <(const ReplicaChannelType& rhs) const;
  bool operator ==(const String& rhs) const;
  bool operator !=(const String& rhs) const;
  bool operator  <(const String& rhs) const;

  //
  // Operations
  //

  /// Replica channel type name
  const String& GetName() const;

  /// Returns true if the replica channel type is valid (registered with the replicator), else false
  bool IsValid() const;

  /// Makes the replica channel type valid, called immediately after being registered with the replicator
  void MakeValid(Replicator* replicator);

  /// Operating replicator (set if the replica channel type is valid)
  void SetReplicator(Replicator* replicator);
  Replicator* GetReplicator() const;

  /// Observes all scheduled replica channels of this type and replicates any changes
  void ObserveAndReplicateChanges();
  void ObserveAndReplicateChanges(ReplicaChannelIndex& replicaChannelIndex, TimeMs timestamp, uint64 frameId);

  /// Schedules the unscheduled replica channel for change observation
  void ScheduleChannel(ReplicaChannel* channel);
  /// Unschedules the replica channel from change observation
  void UnscheduleChannel(ReplicaChannel* channel);

  //
  // Configuration
  //

  /// Resets all configuration settings
  void ResetConfig();

  /// Controls whether or not replica channels should detect outgoing changes
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetDetectOutgoingChanges(bool detectOutgoingChanges = true);
  bool GetDetectOutgoingChanges() const;

  /// Controls whether or not replica channels should accept incoming changes
  void SetAcceptIncomingChanges(bool acceptIncomingChanges = true);
  bool GetAcceptIncomingChanges() const;

  /// Controls whether or not replica channels should call Replicator::OnReplicaChannelPropertyChange when an outgoing replica property change is detected
  void SetNotifyOnOutgoingPropertyChange(bool notifyOnOutgoingPropertyChange = false);
  bool GetNotifyOnOutgoingPropertyChange() const;

  /// Controls whether or not replica channels should call Replicator::OnReplicaChannelPropertyChange when an incoming replica property change is accepted
  void SetNotifyOnIncomingPropertyChange(bool notifyOnIncomingPropertyChange = false);
  bool GetNotifyOnIncomingPropertyChange() const;

  /// Controls when replica channels can modify their change authority
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetAuthorityMode(AuthorityMode::Enum authorityMode = AuthorityMode::Fixed);
  AuthorityMode::Enum GetAuthorityMode() const;

  /// Controls which replicator has the authority to observe and replicate property changes on each replica channel by default
  void SetAuthorityDefault(Authority::Enum authorityDefault = Authority::Server);
  Authority::Enum GetAuthorityDefault() const;

  /// Controls whether or not replica channels will have their changes immediately broadcast to all relevant, incidental peers (if any) once received
  /// (Enabling this allows a server to automatically relay client authoritative changes to other clients, otherwise this must be done manually using ReplicaChannel::ObserveAndReplicateChanges)
  void SetAllowRelay(bool allowRelay = true);
  bool GetAllowRelay() const;

  /// Controls whether or not replica channels may nap (perform change detection on longer intervals) if they haven't changed in a while
  void SetAllowNapping(bool allowNapping = true);
  bool GetAllowNapping() const;

  /// Controls the frame duration following the last detected change in which replica channels are considered actively changing and will be kept awake
  void SetAwakeDuration(uint awakeDuration = 10);
  uint GetAwakeDuration() const;

  /// Controls how replica channel changes are detected
  void SetDetectionMode(DetectionMode::Enum detectionMode = DetectionMode::Manumatic);
  DetectionMode::Enum GetDetectionMode() const;

  /// Controls the frame interval in which awake replica channels are observed for changes
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetAwakeDetectionInterval(uint awakeDetectionInterval = 1);
  uint GetAwakeDetectionInterval() const;

  /// Controls the frame interval in which napping replica channels are observed for changes
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetNapDetectionInterval(uint napDetectionInterval = 2);
  uint GetNapDetectionInterval() const;

  /// Controls when replica channels are serialized
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetSerializationFlags(uint serializationFlags = SerializationFlags::Default);
  uint GetSerializationFlags() const;

  /// Controls how replica channels are serialized
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetSerializationMode(SerializationMode::Enum serializationMode = SerializationMode::Changed);
  SerializationMode::Enum GetSerializationMode() const;

  /// Controls whether or not replica channel changes will be retransmitted should they get lost over the network
  void SetReliabilityMode(ReliabilityMode::Enum reliabilityMode = ReliabilityMode::Reliable);
  ReliabilityMode::Enum GetReliabilityMode() const;

  /// Controls how replica channel changes are to be ordered and released once received
  /// (Cannot be modified after the replica channel type has been made valid)
  void SetTransferMode(TransferMode::Enum transferMode = TransferMode::Ordered);
  TransferMode::Enum GetTransferMode() const;

  /// Controls whether or not the replica channel will serialize an accurate timestamp value when changed, or will instead accept an estimated timestamp value
  /// (This setting may be overridden for replica channels belonging to a specific replica by enabling the corresponding replica setting)
  void SetAccurateTimestampOnChange(bool accurateTimestampOnChange = false);
  bool GetAccurateTimestampOnChange() const;

  /// Data
  String                   mName;                           /// Replica channel type name
  Replicator*              mReplicator;                     /// Operating replicator
  ReplicaChannelIndex      mAwakeChannelIndex;              /// Awake replica channels index
  ReplicaChannelIndex      mNappingChannelIndex;            /// Napping replica channels index
  bool                     mDetectOutgoingChanges;          /// Detect outgoing changes?
  bool                     mAcceptIncomingChanges;          /// Accept incoming changes?
  bool                     mNotifyOnOutgoingPropertyChange; /// Notify on outgoing property change?
  bool                     mNotifyOnIncomingPropertyChange; /// Notify on incoming property change?
  AuthorityMode::Enum      mAuthorityMode;                  /// Change authority mode
  Authority::Enum          mAuthorityDefault;               /// Change authority default
  bool                     mAllowRelay;                     /// Allow relay?
  bool                     mAllowNapping;                   /// Allow napping?
  uint                     mAwakeDuration;                  /// Awake duration frame interval
  DetectionMode::Enum      mDetectionMode;                  /// Change detection mode
  uint                     mAwakeDetectionInterval;         /// Awake change detection frame interval
  uint                     mNapDetectionInterval;           /// Napping change detection frame interval
  SerializationFlags::Enum mSerializationFlags;             /// Serialization flags
  SerializationMode::Enum  mSerializationMode;              /// Serialization mode
  ReliabilityMode::Enum    mReliabilityMode;                /// Change message reliability mode
  TransferMode::Enum       mTransferMode;                   /// Change message transfer mode
  bool                     mAccurateTimestampOnChange;      /// Accurate timestamp when changed?
};

/// Typedefs
typedef UniquePointer<ReplicaChannelType>                                           ReplicaChannelTypePtr;
typedef ArraySet< ReplicaChannelTypePtr, PointerSortPolicy<ReplicaChannelTypePtr> > ReplicaChannelTypeSet;

} // namespace Zero
