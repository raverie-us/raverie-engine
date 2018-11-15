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
//                               ReplicatorLink                                    //
//---------------------------------------------------------------------------------//

/// Replicator Link Plugin
/// Manages remote objects and incoming state replication
class ReplicatorLink : public LinkPlugin
{
public:
  /// Constructor
  ReplicatorLink(Replicator* replicator);

  //
  // Operations
  //

  /// Returns the operating replicator
  Replicator* GetReplicator() const;

  /// Returns their network role
  Role::Enum GetTheirRole() const;

  /// Returns their replicator ID
  ReplicatorId GetReplicatorId() const;

  /// Sends a reliable user message over the ordered command channel
  void Send(Status& status, const Message& message);

  //
  // Replica Management
  //

  /// Returns true if the specified live replica is expected remotely, else false
  bool HasReplica(ReplicaId replicaId) const;
  bool HasReplica(Replica* replica) const;
  /// Returns true if there are any replicas in the specified create context expected remotely, else false
  bool HasReplicasByCreateContext(const CreateContext& createContext) const;
  /// Returns true if there are any live replicas of the specified replica type expected remotely, else false
  bool HasReplicasByReplicaType(const ReplicaType& replicaType) const;
  /// Returns true if there are any live replicas expected remotely, else false
  bool HasReplicas() const;

  /// Returns the specified live replica if it is expected remotely, else nullptr
  Replica* GetReplica(ReplicaId replicaId) const;
  Replica* GetReplica(Replica* replica) const;
  /// Returns all replicas in the specified create context expected remotely
  ReplicaSet GetReplicasByCreateContext(const CreateContext& createContext) const;
  /// Returns all live replicas of the specified replica type expected remotely
  ReplicaSet GetReplicasByReplicaType(const ReplicaType& replicaType) const;
  /// Returns all live replicas expected remotely
  const ReplicaSet& GetReplicas() const;

  /// Returns the number of replicas in the specified create context expected remotely
  size_t GetReplicaCountByCreateContext(const CreateContext& createContext) const;
  /// Returns the number of live replicas of the specified replica type expected remotely
  size_t GetReplicaCountByReplicaType(const ReplicaType& replicaType) const;
  /// Returns the number of live replicas expected remotely
  size_t GetReplicaCount() const;

  //
  // Other Methods
  //

  /// Returns the last connect request data sent or received by this link
  const ConnectRequestData& GetLastConnectRequestData() const;
  /// Returns the last connect response data sent or received by this link
  const ConnectResponseData& GetLastConnectResponseData() const;

  /// Updated at the start of every frame
  /// Returns true if change replication should be skipped for this link
  bool ShouldSkipChangeReplication() const;

  //
  // Internal
  //

  /// Called at the start of the operating replicator's update
  void UpdateStart(TimeMs now);
  /// Called at the end of the operating replicator's update
  void UpdateEnd(TimeMs now);

  //
  // Replica Helpers
  //

  /// Adds the replica to the live set
  /// Returns true if successful, else false
  bool AddReplicaToLiveSet(Replica* replica);
  /// Removes the replica from the live set
  /// Returns true if successful, else false
  bool RemoveReplicaFromLiveSet(Replica* replica);

  /// Adds the replica to the create context set
  /// Returns true if successful, else false
  bool AddReplicaToCreateContextSet(Replica* replica);
  /// Removes the replica from the create context set
  /// Returns true if successful, else false
  bool RemoveReplicaFromCreateContextSet(Replica* replica);

  /// Adds the replica to the replica type set
  /// Returns true if successful, else false
  bool AddReplicaToReplicaTypeSet(Replica* replica);
  /// Removes the replica from the replica type set
  /// Returns true if successful, else false
  bool RemoveReplicaFromReplicaTypeSet(Replica* replica);

  /// Adds the live replica to be expected remotely
  /// Returns true if successful, else false
  bool AddLiveReplica(Replica* replica);
  /// Removes the live replica from being expected remotely
  void RemoveLiveReplica(Replica* replica);

  //
  // ID Helpers
  //

  /// [Server] Sets their replicator ID
  void SetReplicatorId(ReplicatorId replicatorId);

  //
  // Replication Helpers
  //

  /// [Server] Serializes a spawn command
  /// Returns true if successful, else false
  bool SerializeSpawn(const ReplicaArray& replicas, Message& message, TimeMs timestamp);
  /// [Client] Deserializes a spawn command
  /// Returns true if successful, else false
  bool DeserializeSpawn(const Message& message, ReplicaArray& replicas, TimeMs timestamp);
  /// Handles a spawn command
  /// Returns true if successful, else false
  bool HandleSpawn(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Sends a spawn command
  /// Returns true if successful, else false
  bool SendSpawn(const ReplicaArray& replicas, TimeMs timestamp);
  /// [Client] Receives a spawn command
  /// Returns true if successful, else false
  bool ReceiveSpawn(const Message& message);

  /// [Server] Serializes a clone command
  /// Returns true if successful, else false
  bool SerializeClone(const ReplicaArray& replicas, Message& message, TimeMs timestamp);
  /// [Client] Deserializes a clone command
  /// Returns true if successful, else false
  bool DeserializeClone(const Message& message, ReplicaArray& replicas, TimeMs timestamp);
  /// Handles a clone command
  /// Returns true if successful, else false
  bool HandleClone(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Sends a clone command
  /// Returns true if successful, else false
  bool SendClone(const ReplicaArray& replicas, TimeMs timestamp);
  /// [Client] Receives a clone command
  /// Returns true if successful, else false
  bool ReceiveClone(const Message& message);

  /// [Server] Serializes a forget command
  /// Returns true if successful, else false
  bool SerializeForget(const ReplicaArray& replicas, Message& message, TimeMs timestamp);
  /// [Client] Deserializes a forget command
  /// Returns true if successful, else false
  bool DeserializeForget(const Message& message, ReplicaArray& replicas, TimeMs timestamp);
  /// Handles a forget command
  /// Returns true if successful, else false
  bool HandleForget(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Sends a forget command
  /// Returns true if successful, else false
  bool SendForget(const ReplicaArray& replicas, TimeMs timestamp);
  /// [Client] Receives a forget command
  /// Returns true if successful, else false
  bool ReceiveForget(const Message& message);

  /// [Server] Serializes a destroy command
  /// Returns true if successful, else false
  bool SerializeDestroy(const ReplicaArray& replicas, Message& message, TimeMs timestamp);
  /// [Client] Deserializes a destroy command
  /// Returns true if successful, else false
  bool DeserializeDestroy(const Message& message, ReplicaArray& replicas, TimeMs timestamp);
  /// Handles a destroy command
  /// Returns true if successful, else false
  bool HandleDestroy(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Sends a destroy command
  /// Returns true if successful, else false
  bool SendDestroy(const ReplicaArray& replicas, TimeMs timestamp);
  /// [Client] Receives a destroy command
  /// Returns true if successful, else false
  bool ReceiveDestroy(const Message& message);

  /// [Client] Serializes reverse replica channels
  /// Returns true if successful, else false
  bool SerializeReverseReplicaChannels(const ReplicaArray& replicas, Message& message, bool& containsChannels, TimeMs timestamp);
  /// [Server] Deserializes reverse replica channels
  /// Returns true if successful, else false
  bool DeserializeReverseReplicaChannels(const Message& message, ReplicaArray& replicas, TimeMs timestamp);
  /// [Client] Sends reverse replica channels (as applicable)
  /// Returns true if successful, else false
  bool SendReverseReplicaChannels(const ReplicaArray& replicas, TimeMs timestamp);
  /// [Server] Receives reverse replica channels (as applicable)
  /// Returns true if successful, else false
  bool ReceiveReverseReplicaChannels(const Message& message);

  /// [Client] Receives create context cache items
  /// Returns true if successful, else false
  bool ReceiveCreateContextItems(const Message& message);
  /// [Client] Receives replica type cache items
  /// Returns true if successful, else false
  bool ReceiveReplicaTypeItems(const Message& message);
  /// [Client] Receives emplace context cache items
  /// Returns true if successful, else false
  bool ReceiveEmplaceContextItems(const Message& message);

  /// Deserializes a replica channel change
  /// Returns true if successful, else false
  bool DeserializeChange(const Message& message, TimeMs timestamp);
  /// Sends a replica channel change
  /// Returns true if successful, else false
  bool SendChange(ReplicaChannel* replicaChannel, Message& message);
  /// Receives a replica channel change
  /// Returns true if successful, else false
  bool ReceiveChange(const Message& message);

  /// [Server] Sends an interrupt command
  /// Returns true if successful, else false
  bool SendInterrupt(Message& message);

  //
  // Channel Helpers
  //

  /// [Server] Opens and serializes all forward (outgoing) message channels corresponding to the given replica's channels
  /// Returns true if successful, else false
  bool OpenAndSerializeForwardReplicaChannels(const Replica* replica, BitStream& bitStream);
  /// [Client] Deserializes and sets all forward (incoming) message channels corresponding to the given replica's channels
  /// Returns true if successful, else false
  bool DeserializeAndSetForwardReplicaChannels(Replica* replica, const BitStream& bitStream);

  /// [Client] Opens and serializes all reverse (outgoing) message channels corresponding to the given replica's channels
  /// Returns true if successful, else false
  bool OpenAndSerializeReverseReplicaChannels(const Replica* replica, BitStream& bitStream);
  /// [Server] Deserializes and sets all reverse (incoming) message channels corresponding to the given replica's channels
  /// Returns true if successful, else false
  bool DeserializeAndSetReverseReplicaChannels(Replica* replica, const BitStream& bitStream);

  /// Returns the command channel ID
  MessageChannelId GetCommandChannelId() const;

  /// Closes and clears both outgoing and incoming message channels corresponding to every replica channel on the specified replica respectively
  void CloseAllReplicaChannels(Replica* replica);

  /// Opens an outgoing message channel corresponding to the specified replica channel
  /// Returns the outgoing message channel ID if successful, else 0
  MessageChannelId OpenOutgoingReplicaChannel(ReplicaChannel* replicaChannel);
  /// Closes the outgoing message channel corresponding to the specified replica channel (if any)
  void CloseOutgoingReplicaChannel(ReplicaChannel* replicaChannel);
  /// Returns the outgoing message channel corresponding to the specified replica channel, else 0
  MessageChannelId GetOutgoingReplicaChannel(ReplicaChannel* replicaChannel) const;

  /// Sets the corresponding incoming message channel on the specified replica channel
  /// Returns true if successful, else false
  bool SetIncomingReplicaChannel(MessageChannelId channelId, ReplicaChannel* replicaChannel);
  /// Clears the corresponding incoming message channel on the specified replica channel (if any)
  void ClearIncomingReplicaChannel(ReplicaChannel* replicaChannel);
  /// Returns the replica channel corresponding to the specified incoming message channel, else nullptr
  ReplicaChannel* GetIncomingReplicaChannel(MessageChannelId channelId) const;

  //
  // Link Plugin Interface
  //

  /// Called after a connect request is sent
  void OnConnectRequestSend(Message& message) override;
  /// Called after a connect request is received
  void OnConnectRequestReceive(Message& message) override;

  /// Called after a connect response is sent
  void OnConnectResponseSend(Message& message) override;
  /// Called after a connect response is received
  void OnConnectResponseReceive(Message& message) override;

  /// Called after a disconnect notice is sent
  void OnDisconnectNoticeSend(Message& message) override;
  /// Called after a disconnect notice is received
  void OnDisconnectNoticeReceive(Message& message) override;
  /// Handles the disconnect notice
  void HandleDisconnectNotice(Message& message, TransmissionDirection::Enum direction);

  /// Called after the link state is changed
  void OnStateChange(LinkState::Enum prevState) override;

  /// Called after a plugin message is received
  void OnPluginMessageReceive(MoveReference<Message> message, bool& continueProcessingCustomMessages) override;

  /// Data
  Replicator* const        mReplicator;                           /// Operating replicator
  ReplicatorId             mReplicatorId;                         /// Their replicator ID
  ReplicaSet               mReplicaSet;                           /// Remotely expected live replicas
  CreateMap                mCreateMap;                            /// Remotely expected live replicas mapped by create context
  ReplicaMap               mReplicaMap;                           /// Remotely expected live replicas mapped by replica type
  MessageChannelId         mCommandChannelId;                     /// Command channel ID
  OutReplicaChannels       mOutReplicaChannels;                   /// Outgoing replica channel map (replica channel to message channel ID)
  InReplicaChannels        mInReplicaChannels;                    /// Incoming replica channel map (message channel ID to replica channel)
  InReplicaChannelsFlipped mInReplicaChannelsFlipped;             /// Incoming replica channel map flipped (replica channel to message channel ID)
  ConnectRequestData       mLastConnectRequestData;               /// Last connect request data sent/received
  ConnectResponseData      mLastConnectResponseData;              /// Last connect response data sent/received
  bool                     mShouldSkipChangeReplication;          /// Should skip change replication? (Updated at the start of every frame)
  TimeMs                   mLastFrameFillSkipNotificationTime;    /// Last frame fill skip notification time
  TimeMs                   mLastFrameFillWarningNotificationTime; /// Last frame fill warning notification time

private:
  /// No copy constructor
  ReplicatorLink(const ReplicatorLink&);
  /// No copy assignment operator
  ReplicatorLink& operator=(const ReplicatorLink&);
};

} // namespace Zero
