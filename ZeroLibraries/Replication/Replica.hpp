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
//                                  Replica                                        //
//---------------------------------------------------------------------------------//

/// Replica Object
/// Manages the replication of a single object
class Replica
{
public:
  /// Constructors
  /// Creates a replica of the specified application-specific type (optional for emplaced replicas)
  Replica();
  Replica(const CreateContext& createContext, const ReplicaType& replicaType);

  /// Destructor
  REPLICA_VIRTUAL ~Replica();

  /// Comparison Operators (compares replica IDs)
  bool operator ==(const Replica& rhs) const;
  bool operator !=(const Replica& rhs) const;
  bool operator  <(const Replica& rhs) const;
  bool operator ==(const ReplicaId& rhs) const;
  bool operator !=(const ReplicaId& rhs) const;
  bool operator  <(const ReplicaId& rhs) const;

  //
  // Operations
  //

  /// Returns true if the replica is invalid (unregistered with the replicator), else false
  bool IsInvalid() const;
  /// Returns true if the replica is valid (registered with the replicator), else false
  bool IsValid() const;
  /// Returns true if the replica is live (valid and assigned a replica ID by the server replicator), else false
  bool IsLive() const;
  /// Returns true if the replica is emplaced (not spawned), else false
  bool IsEmplaced() const;
  /// Returns true if the replica is spawned (not emplaced), else false
  bool IsSpawned() const;
  /// Returns true if the replica is cloned (spawned or emplaced, then cloned to our replicator), else false
  bool IsCloned() const;

  /// Returns the operating replicator (set if the replica is valid), else nullptr
  Replicator* GetReplicator() const;

  /// Sets the create context
  /// (Cannot be modified after the replica has been made valid)
  /// Returns true if successful, else false
  bool SetCreateContext(const CreateContext& createContext);
  /// Returns the create context
  const CreateContext& GetCreateContext() const;

  /// Sets the replica type
  /// (Cannot be modified after the replica has been made valid)
  /// Returns true if successful, else false
  bool SetReplicaType(const ReplicaType& replicaType);
  /// Returns the replica type
  const ReplicaType& GetReplicaType() const;

  /// Sets the replica ID
  void SetReplicaId(ReplicaId replicaId);
  /// Returns the replica ID (set if the replica is live), else 0
  ReplicaId GetReplicaId() const;

  /// Returns the emplace context (set if the replica is emplaced), else EmplaceContext()
  const EmplaceContext& GetEmplaceContext() const;
  /// Returns the emplace ID (set if the replica is emplaced), else 0
  EmplaceId GetEmplaceId() const;

  /// Sets optional user data
  void SetUserData(void* userData = nullptr);
  /// Returns optional user data
  template <typename T>
  T* GetUserData() const { return static_cast<T*>(GetUserData()); }
  void* GetUserData() const;

  /// Timestamp indicating when this replica was initialized, else cInvalidMessageTimestamp
  /// (Set immediately before the replica is made live)
  void SetInitializationTimestamp(TimeMs initializationTimestamp);
  TimeMs GetInitializationTimestamp() const;

  /// Timestamp indicating when this replica was last changed, else cInvalidMessageTimestamp
  /// (Set immediately after a change is observed on any replica channel)
  void SetLastChangeTimestamp(TimeMs lastChangeTimestamp);
  TimeMs GetLastChangeTimestamp() const;

  /// Timestamp indicating when this replica was uninitialized, else cInvalidMessageTimestamp
  /// (Set immediately before the replica is made invalid)
  void SetUninitializationTimestamp(TimeMs uninitializationTimestamp);
  TimeMs GetUninitializationTimestamp() const;

  /// Forces all replica channels to stop napping immediately
  void WakeUp();
  /// Forces all replica channels to start napping immediately
  void TakeNap();

  /// Returns true if at least one replica channel is awake (not napping), else false
  bool IsAwake() const;
  /// Returns true if all replica channels are napping (not awake), else false
  bool IsNapping() const;

  //
  // Configuration
  //

  /// Resets all configuration settings
  void ResetConfig();

  /// Controls whether or not replica channels may detect outgoing changes
  void SetDetectOutgoingChanges(bool detectOutgoingChanges = true);
  bool GetDetectOutgoingChanges() const;

  /// Controls whether or not replica channels may accept incoming changes
  void SetAcceptIncomingChanges(bool acceptIncomingChanges = true);
  bool GetAcceptIncomingChanges() const;

  /// Controls whether or not replica channels may nap (perform change detection on longer intervals) if they haven't changed in a while
  void SetAllowNapping(bool allowNapping = true);
  bool GetAllowNapping() const;

  /// Sets the change authority client by replicator ID
  /// Controls which client has change authority over all replica channels with client change authority (specified by ReplicaChannel::Authority)
  void SetAuthorityClientReplicatorId(ReplicatorId authorityClientReplicatorId = 0);
  /// Returns the change authority client by replicator ID
  ReplicatorId GetAuthorityClientReplicatorId() const;

  /// Controls whether or not the replica will serialize an accurate timestamp value when initialized, or will instead accept an estimated timestamp value
  void SetAccurateTimestampOnInitialization(bool accurateTimestampOnInitialization = false);
  bool GetAccurateTimestampOnInitialization() const;

  /// Controls whether or not the replica will serialize an accurate timestamp value when changed (on any replica channel), or will instead accept an estimated timestamp value
  /// (Enabling this will override the corresponding replica channel type setting for all replica channels added to this replica)
  void SetAccurateTimestampOnChange(bool accurateTimestampOnChange = false);
  bool GetAccurateTimestampOnChange() const;

  /// Controls whether or not the replica will serialize an accurate timestamp value when uninitialized, or will instead accept an estimated timestamp value
  void SetAccurateTimestampOnUninitialization(bool accurateTimestampOnUninitialization = false);
  bool GetAccurateTimestampOnUninitialization() const;

  //
  // Replica Channel Management
  //

  /// Returns true if the replica has the specified replica channel, else false
  bool HasReplicaChannel(const String& replicaChannelName) const;
  /// Returns the specified replica channel, else nullptr
  const ReplicaChannel* GetReplicaChannel(const String& replicaChannelName) const;
  ReplicaChannel* GetReplicaChannel(const String& replicaChannelName);
  /// Returns all replica channels managed by the replica
  const ReplicaChannelSet& GetReplicaChannels() const;
  ReplicaChannelSet& GetReplicaChannels();

  /// Adds the replica channel
  /// (Cannot be modified after the replica has been made valid)
  /// Returns the replica channel if successful, else nullptr (a replica channel of that name already exists)
  ReplicaChannel* AddReplicaChannel(ReplicaChannelPtr replicaChannel);
  /// Removes the specified replica channel
  /// (Cannot be modified after the replica has been made valid)
  /// Returns true if successful, else false (a replica channel of that name could not be found)
  bool RemoveReplicaChannel(const String& replicaChannelName);

  /// Removes all replica channels
  /// (Cannot be modified after the replica has been made valid)
  void ClearReplicaChannels();

  //
  // Internal
  //

  /// Returns true if the replica uses reverse replica channels (such as with client authority), else false
  bool UsesReverseReplicaChannels() const;

  /// Sets the 'Is cloned?' flag
  void SetIsCloned(bool isCloned);

  /// Sets the operating replicator
  void SetReplicator(Replicator* replicator);

  /// Sets the emplace context
  void SetEmplaceContext(const EmplaceContext& emplaceContext);

  /// Sets the emplace ID
  void SetEmplaceId(EmplaceId emplaceId);

  /// Reacts to all replica channel properties that have been legitimately changed, determined using comparisons, since this function was last called
  void ReactToChannelPropertyChanges(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, TransmissionDirection::Enum direction, bool generateNotifications = true, bool setLastValues = true);

  /// Data
  CreateContext     mCreateContext;                       /// Create context
  ReplicaType       mReplicaType;                         /// Replica type
  Replicator*       mReplicator;                          /// Operating replicator
  ReplicaId         mReplicaId;                           /// Replica ID
  EmplaceContext    mEmplaceContext;                      /// Emplace context
  EmplaceId         mEmplaceId;                           /// Emplace ID
  bool              mIsCloned;                            /// 'Is cloned?' flag
  TimeMs            mInitializationTimestamp;             /// Timestamp indicating when this replica was initialized
  TimeMs            mLastChangeTimestamp;                 /// Timestamp indicating when this replica was last changed (on any replica channel)
  TimeMs            mUninitializationTimestamp;           /// Timestamp indicating when this replica was uninitialized
  bool              mDetectOutgoingChanges;               /// Detect outgoing changes?
  bool              mAcceptIncomingChanges;               /// Accept incoming changes?
  bool              mAllowNapping;                        /// Allow napping?
  ReplicatorId      mAuthorityClientReplicatorId;         /// Authority client by replicator ID
  bool              mAccurateTimestampOnInitialization;   /// Accurate timestamp when initialized?
  bool              mAccurateTimestampOnChange;           /// Accurate timestamp when changed (on any replica channel)?
  bool              mAccurateTimestampOnUninitialization; /// Accurate timestamp when uninitialized?
  ReplicaChannelSet mReplicaChannels;                     /// Replica channels
  void*             mUserData;                            /// Optional user data

private:
  /// No copy constructor
  Replica(const Replica&);
  /// No copy assignment operator
  Replica& operator=(const Replica&);
};

} // namespace Zero
