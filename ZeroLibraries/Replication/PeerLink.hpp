///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//---------------------------------------------------------------------------------//
//                           PeerLink Configuration                                //
//---------------------------------------------------------------------------------//

/// Should peer links disconnect via latency?
/// Will disconnect if RTT exceeds latency limit
#ifdef ZeroDebug
  #define PEER_LINK_ENABLE_DISCONNECT_VIA_LATENCY 0
#else
  #define PEER_LINK_ENABLE_DISCONNECT_VIA_LATENCY 1
#endif

/// Should peer links disconnect via timeout?
/// Will disconnect if (RTT/2) * timeout factor elapses without receiving any packets
#ifdef ZeroDebug
  #define PEER_LINK_ENABLE_DISCONNECT_VIA_TIMEOUT 0
#else
  #define PEER_LINK_ENABLE_DISCONNECT_VIA_TIMEOUT 1
#endif

/// Should peer links fail connect attempts via timeout?
/// Will disconnect if (RTT/2) * connect attempt factor elapses during a connection attempt stage
#ifdef ZeroDebug
  #define PEER_LINK_ENABLE_FAIL_CONNECT_VIA_TIMEOUT 0
#else
  #define PEER_LINK_ENABLE_FAIL_CONNECT_VIA_TIMEOUT 1
#endif

namespace Zero
{

/// Typedefs
typedef ArraySet< LinkPlugin*, PointerSortPolicy<LinkPlugin*> > LinkPluginSet;

//---------------------------------------------------------------------------------//
//                                  PeerLink                                       //
//---------------------------------------------------------------------------------//

/// Maintains connection state associated with a remote peer
class PeerLink : public BandwidthStats<false>
{
public:
  /// Resets the link's session data to it's default state
  void ResetSession();

  /// Constructor
  PeerLink(Peer* peer, const IpAddress& ipAddress, TransmissionDirection::Enum creationDirection);

  /// Comparison Operators (compares their IP addresses)
  bool operator ==(const PeerLink& rhs) const;
  bool operator !=(const PeerLink& rhs) const;
  bool operator  <(const PeerLink& rhs) const;
  bool operator ==(const IpAddress& rhs) const;
  bool operator !=(const IpAddress& rhs) const;
  bool operator  <(const IpAddress& rhs) const;

  //
  // Member Functions
  //

  /// Returns the operating peer
  Peer* GetPeer() const;

  /// Returns the duration that the link has existed
  TimeMs GetCreationDuration() const;
  /// Returns the direction in which the link was created (which peer initiated the connection)
  TransmissionDirection::Enum GetCreationDirection() const;

  /// Returns the link's overall status
  LinkStatus::Enum GetStatus() const;
  /// Returns the link's specific state
  LinkState::Enum GetState() const;
  /// Returns the link's specific state duration
  TimeMs GetStateDuration() const;

  /// Returns the remote peer's permanent GUID, else 0
  /// (Available if the link is connected or incoming and attempting connection)
  Guid GetTheirGuid() const;

  /// Returns the remote peer's IP address (as seen from our perspective)
  /// For outgoing links this is the same IP address specified in our connect call
  /// This IP address will never change for the lifetime of this link
  const IpAddress& GetTheirIpAddress() const;

  /// Returns our peer's IP address (as seen from their perspective), else IpAddress()
  /// For incoming links this is the same IP address specified in their connect call
  /// (Available if the link is connected or incoming and attempting connection)
  /// It is absolutely possible that this does not match our local IP address
  const IpAddress& GetOurIpAddress() const;

  /// Returns the link's IP address protocol version
  /// This IP address protocol will never change for the lifetime of this link
  InternetProtocol::Enum GetInternetProtocol() const;

  /// Initiates a connect attempt with the remote peer on the disconnected link
  /// Returns true if a connect request was successfully initiated, else false
  bool Connect(const BitStream& extraData = BitStream());

  /// Responds to a pending connect request on the attempting connection link
  /// Returns true if a connect response was successfully initiated, else false
  bool RespondToConnectRequest(bool accept, const BitStream& extraData = BitStream());

  /// Disconnects by request from the remote peer on the connected or attempting connection link
  /// Returns true if a disconnect notice was successfully initiated, else false
  bool Disconnect(const BitStream& extraData = BitStream());

  /// Sends a user message on the connected link
  /// Enable reliable to continually resend this message every determined NAK for the lifetime of this message
  /// Specify an outgoing message channel open on this link to indicate this message belongs to that channel's message sequence
  /// Include a timestamp in local time on the message to be serialized and automatically translated to remote time once received by the remote peer
  /// Enable receipt to be notified of this message's arrival status in a future update call event message
  /// Give a higher message priority value to indicate this message should be sent before other queued messages with lower priority values
  /// Messages with equal priority are sent in chronological order with respect to internal fragmentation preferences
  /// Provide a message lifetime to indicate how long this message should be kept alive in the send queue before being discarded
  /// Returns the message's receipt ID if send queuing successful and receipt was enabled, else 0
  MessageReceiptId Send(Status& status, const Message& message,         bool reliable, MessageChannelId channelId = 0, bool receipt = false, MessagePriority priority = 0, TimeMs lifetime = cInfiniteTimeMs);
  MessageReceiptId Send(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId = 0, bool receipt = false, MessagePriority priority = 0, TimeMs lifetime = cInfiniteTimeMs);

  /// Returns the current local update time
  TimeMs GetLocalTime() const;
  /// Returns the current local update delta time
  TimeMs GetLocalDeltaTime() const;
  /// Returns the current local update frame ID
  uint64 GetLocalFrameId() const;

  /// Returns the current estimated remote update time, else local time
  /// (Available if the link is connected or incoming and attempting connection)
  TimeMs GetRemoteTime() const;

  /// Translates the local time value (relative to our local Peer) to a remote time value (relative to their remote Peer), else does nothing
  /// (Available if the link is connected or incoming and attempting connection)
  TimeMs LocalToRemoteTime(TimeMs localTime) const;
  /// Translates the remote time value (relative to their remote Peer) to a local time value (relative to our local Peer), else does nothing
  /// (Available if the link is connected or incoming and attempting connection)
  TimeMs RemoteToLocalTime(TimeMs remoteTime) const;

  /// Difference between our local time values and their remote time values, else 0
  /// (Available if the link is connected or incoming and attempting connection)
  void SetLocalToRemoteTimeDifference(TimeMs localToRemoteTimeDifference);
  TimeMs GetLocalToRemoteTimeDifference() const;

  /// Sets optional user data associated with the link
  void SetUserData(void* userData = nullptr);
  /// Returns optional user data associated with the link
  template<typename T>
  T* GetUserData() const { return static_cast<T*>(GetUserData()); }
  void* GetUserData() const;

  //
  // Outgoing Message Channel Management
  //

  /// Opens an outgoing message channel with the specified transfer mode if there are available outgoing message channel IDs on this connected link
  /// Returns the new outgoing message channel if successful, else nullptr
  OutMessageChannel* OpenOutgoingChannel(TransferMode::Enum transferMode);

  /// Returns the outgoing message channel corresponding to the specified message channel ID open on this link, else nullptr
  OutMessageChannel* GetOutgoingChannel(MessageChannelId channelId) const;
  /// Returns all outgoing message channels open on this link
  ArraySet<OutMessageChannel>::range GetOutgoingChannels() const;
  /// Returns the number of outgoing message channels open on this link
  uint GetOutgoingChannelCount() const;

  /// Closes an outgoing message channel open on this link
  void CloseOutgoingChannel(MessageChannelId channelId);
  /// Closes all outgoing message channels open on this link
  void CloseOutgoingChannels();

  //
  // Incoming Message Channel Management
  //

  /// Returns the incoming message channel corresponding to the specified message channel ID open on this link, else nullptr
  InMessageChannel* GetIncomingChannel(MessageChannelId channelId) const;
  /// Returns all incoming message channels open on this link
  ArraySet<InMessageChannel>::range GetIncomingChannels() const;
  /// Returns the number of incoming message channels open on this link
  uint GetIncomingChannelCount() const;

  //
  // Link Plugin Management
  //

  /// Adds a link plugin corresponding to the specified name if one is not already active on this link
  /// Returns the added link plugin, else nullptr
  LinkPlugin* AddPlugin(LinkPlugin* plugin, StringParam name);

  /// Returns the link plugin corresponding to the specified name active on this link, else nullptr
  template<typename T>
  T* GetPlugin(StringParam name) const { return static_cast<T*>(GetPlugin(name)); }
  LinkPlugin* GetPlugin(StringParam name) const;

  /// Returns all link plugins active on this link
  LinkPluginSet::range GetPlugins() const;
  /// Returns the number of link plugins active on this link
  uint GetPluginCount() const;

  /// Removes a link plugin active on this link
  void RemovePlugin(StringParam name);
  /// Removes all link plugins active on this link
  void RemovePlugins();

  //
  // Link Configuration
  //

  /// Resets all link configuration settings
  void ResetConfig();

  /// TODO: Remove this after implementing AIMD, no longer necessary
  /// Sets the number of packets this link may send per second
  void SetSendRate(uint sendRate = 85 /* ~1Mbps */);
  /// Returns the number of packets this link may send per second
  uint GetSendRate() const;

  /// TODO: Remove this after implementing AIMD, no longer necessary
  /// Sets the outgoing packet data size left over for messages
  void SetPacketDataBytes(Bytes packetDataBytes = MaxPacketDataBytes /* ~1Mbps */);
  /// Returns the outgoing packet data size left over for messages
  Bytes GetPacketDataBytes() const;

  /// Sets the internal pre-connection round trip time (actual RTT substitute)
  void SetPreconnectionRoundTripTime(TimeMs preconnectionRoundTripTime = 250);
  /// Returns the internal pre-connection round trip time (actual RTT substitute)
  TimeMs GetPreconnectionRoundTripTime() const;

  /// Sets the internal floor round trip time (internal RTT minimum)
  void SetFloorRoundTripTime(TimeMs floorRoundTripTime = 30);
  /// Sets the internal floor round trip time (internal RTT minimum)
  TimeMs GetFloorRoundTripTime() const;

  /// TODO: Remove this after implementing AIMD, no longer necessary
  /// Sets the packet sequence history range factor (packet sequence history range = send rate * packet sequence history range factor)
  void SetPacketSequenceHistoryRangeFactor(float packetSequenceHistoryRangeFactor = 2);
  /// Returns the packet sequence history range factor (packet sequence history range = send rate * packet sequence history range factor)
  float GetPacketSequenceHistoryRangeFactor() const;

  /// TODO: Remove this after implementing AIMD, no longer necessary
  /// Sets the packet sequence history rate factor [0, 1] (packet sequence history rate = send rate * packet sequence history rate factor)
  void SetPacketSequenceHistoryRateFactor(float packetSequenceHistoryRateFactor = 1);
  /// Returns the packet sequence history rate factor [0, 1] (packet sequence history rate = send rate * packet sequence history rate factor)
  float GetPacketSequenceHistoryRateFactor() const;

  /// Sets the packet NAK factor (if RTT * NAK factor elapses without a reply to a sent packet, it is assumed to be NAKd)
  void SetPacketNAKFactor(float packetNAKFactor = 10);
  /// Returns the packet NAK factor (if RTT * NAK factor elapses without a reply to a sent packet, it is assumed to be NAKd)
  float GetPacketNAKFactor() const;

  /// Sets the number of heartbeat packets to send per second (in the absence of messages) in order to keep the connection alive
  void SetHeartbeatPacketRate(uint heartbeatPacketRate = 15);
  /// Returns the number of heartbeat packets to send per second (in the absence of messages) in order to keep the connection alive
  uint GetHeartbeatPacketRate() const;

  /// Sets the link timeout factor (if (RTT/2) * timeout factor elapses without receiving any packets, the link disconnects via timeout)
  void SetTimeoutFactor(float timeoutFactor = 300);
  /// Returns the link timeout factor (if (RTT/2) * timeout factor elapses without receiving any packets, the link disconnects via timeout)
  float GetTimeoutFactor() const;

  /// Sets the link latency limit (if RTT exceeds latency limit, the link disconnects via latency)
  void SetLatencyLimit(TimeMs latencyLimit = cOneSecondTimeMs * 3);
  /// Returns the link latency limit (if RTT exceeds latency limit, the link disconnects via latency)
  TimeMs GetLatencyLimit() const;

  /// Sets the connect attempt factor (if (RTT/2) * connect attempt factor elapses during a connection attempt stage, the link disconnects)
  void SetConnectAttemptFactor(float connectAttemptFactor = 80);
  /// Returns the connect attempt factor (if (RTT/2) * connect attempt factor elapses during a connection attempt stage, the link disconnects)
  float GetConnectAttemptFactor() const;

  /// Sets the disconnect attempt factor (if (RTT/2) * disconnect attempt factor elapses during a disconnection attempt stage, the link disconnects)
  void SetDisconnectAttemptFactor(float disconnectAttemptFactor = 2);
  /// Returns the disconnect attempt factor (if (RTT/2) * disconnect attempt factor elapses during a disconnection attempt stage, the link disconnects)
  float GetDisconnectAttemptFactor() const;

  /// Returns a summary of all link configuration settings as an array of key-value string pairs
  Array< Pair<String, String> > GetConfigSummary() const;
  /// Returns a summary of all link configuration settings as a single multi-line string (intended for debugging convenience)
  String GetConfigSummaryString() const;

  //
  // Link Statistics
  //

  /// Returns the minimum round trip time (RTT)
  TimeMs GetMinRoundTripTime() const;
  /// Returns the average round trip time (RTT)
  TimeMs GetAvgRoundTripTime() const;
  /// Returns the maximum round trip time (RTT)
  TimeMs GetMaxRoundTripTime() const;

  /// Returns the minimum internal round trip time (RTT used internally, unaffected by ResetStats)
  TimeMs GetMinInternalRoundTripTime() const;
  /// Returns the average internal round trip time (RTT used internally, unaffected by ResetStats)
  TimeMs GetAvgInternalRoundTripTime() const;
  /// Returns the maximum internal round trip time (RTT used internally, unaffected by ResetStats)
  TimeMs GetMaxInternalRoundTripTime() const;

  /// Returns a summary of all link statistics as an array of pairs containing the property name and array of minimum, average, and maximum values
  Array< Pair< String, Array<String> > > GetStatsSummary() const;
  /// Returns a summary of all link statistics as a single multi-line string (intended for debugging convenience)
  String GetStatsSummaryString() const;

  //
  // Link Frame Data
  //

  /// Sets the outgoing bandwidth (data transfer rate) supported by this link
  void SetOutgoingBandwidth(Kbps outgoingBandwidth);
  /// Returns the outgoing bandwidth (data transfer rate) supported by this link
  Kbps GetOutgoingBandwidth() const;

  /// Sets the amount of outgoing bandwidth available since our last update
  void SetOutgoingFrameCapacity(Bits outgoingFrameCapacity);
  /// Returns the amount of outgoing bandwidth available since our last update, updated at the start of every update
  Bits GetOutgoingFrameCapacity() const;

  /// Returns the amount of outgoing data sent, deducted by frame capacity at the start of every update
  void SetOutgoingFrameSize(Bits outgoingFrameSize);
  /// Returns the amount of outgoing data sent, deducted by frame capacity at the start of every update
  Bits GetOutgoingFrameSize() const;

  /// Returns the outgoing bandwidth frame fill ratio (frame size / frame capacity), may exceed 1.0
  float GetOutgoingFrameFill() const;

  //
  // Internal
  //

  /// Resets all applicable link statistics to start over relative to a new statistics period
  void ResetStats();

  /// Initializes all link statistics
  void InitializeStats();

  /// Updates the round trip time statistics
  void UpdateRoundTripTime(TimeMs sample, TimeMs floor);

  /// Sends a protocol or custom message on the link
  MessageReceiptId SendInternal(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId = 0, bool receipt = false, MessagePriority priority = 0, TimeMs lifetime = cInfiniteTimeMs, bool isProtocol = true);

  /// Returns true if the absolute message type is within the user's range, else false
  bool AbsoluteIsInRange(MessageType absoluteType) const;
  /// Returns a user relative message type from an absolute message type
  MessageType RelativeToAbsolute(MessageType relativeType) const;
  /// Returns an absolute message type from a user relative message type
  MessageType AbsoluteToRelative(MessageType absoluteType) const;

  /// Returns the user message type count
  inline MessageType GetUserMessageTypeCount() const
  {
    return mUserMessageTypeStart // User message type range not exhausted?
         ? MessageTypeMax - mUserMessageTypeStart + 1
         : 0;
  }

  /// Sets the link's state
  void SetState(LinkState::Enum state);

  /// Receives an incoming packet to be processed later
  void ReceivePacket(MoveReference<InPacket> inPacket);

  /// Sends an outgoing packet now
  void SendPacket(OutPacket& outPacket);

  /// Processes incoming packets, updates link state, and generates outgoing packets
  void UpdateLinkState();
  /// Processes all received custom messages
  void ProcessReceivedCustomMessages();
  /// Processes a custom message received by the link
  /// Returns true to continue processing custom messages on this link, else false (will continue next update call)
  bool ProcessReceivedCustomMessage(Message& message, bool isEvent);

  /// Called after a connect request is sent
  void PluginEventOnConnectRequestSend(Message& message);
  /// Called after a connect request is received
  void PluginEventOnConnectRequestReceive(Message& message);

  /// Called after a connect response is sent
  void PluginEventOnConnectResponseSend(Message& message);
  /// Called after a connect response is received
  void PluginEventOnConnectResponseReceive(Message& message);

  /// Called after a disconnect notice is sent
  void PluginEventOnDisconnectNoticeSend(Message& message);
  /// Called after a disconnect notice is received
  void PluginEventOnDisconnectNoticeReceive(Message& message);

  /// Called after the link state is changed
  void PluginEventOnStateChange(LinkState::Enum prevState);
  /// Called after the link status is changed
  void PluginEventOnStatusChange(LinkStatus::Enum prevStatus);

  /// Called before a packet is sent
  /// Return true to continue sending the packet, else false
  bool PluginEventOnPacketSend(OutPacket& packet);
  /// Called after a packet is received
  /// Return true to continue receiving the packet, else false
  bool PluginEventOnPacketReceive(InPacket& packet);

  /// Called before a message is sent
  /// Return true to continue sending the message, else false
  bool PluginEventOnMessageSend(OutMessage& message);
  /// Called after a sent message is receipted
  /// Return true to continue receipting the message, else false
  bool PluginEventOnMessageReceipt(OutMessage& message, Receipt::Enum receipt);
  /// Called after a message is received
  /// Return true to continue receiving the message, else false
  bool PluginEventOnMessageReceive(Message& message);

  /// Called before a plugin message is sent
  /// Return true to continue sending the message, else false
  bool PluginEventOnPluginMessageSend(OutMessage& message);

  /// Called before an incoming channel is opened
  /// Return true to continue opening the incoming channel, else false
  bool PluginEventOnIncomingChannelOpen(MessageChannelId channelId);
  /// Called before an incoming channel is closed
  void PluginEventOnIncomingChannelClose(MessageChannelId channelId);

  /// Attempts to receipt the message type according to the plugin it is intended for (if there is one)
  /// Returns true if successful, else false (the message type does not belong to any current plugin)
  bool AttemptPluginMessageReceipt(MoveReference<OutMessage> message, Receipt::Enum receipt);
  /// Attempts to receive the message type according to the plugin it is intended for (if there is one)
  /// Returns true if successful, else false (the message type does not belong to any current plugin)
  bool AttemptPluginMessageReceive(MoveReference<Message> message, bool& continueProcessingCustomMessages);

  /// Pushes a link connect requested user event message
  void LinkEventConnectRequested(TransmissionDirection::Enum direction, const ConnectRequestData& connectRequestData);
  /// Pushes a link connect responded user event message
  void LinkEventConnectResponded(TransmissionDirection::Enum direction, const ConnectResponseData& connectResponseData);
  /// Pushes a link disconnect noticed user event message
  void LinkEventDisconnectNoticed(TransmissionDirection::Enum direction, const DisconnectNoticeData& disconnectNoticeData);
  /// Pushes a link incoming channel opened user event message
  void LinkEventIncomingChannelOpened(MessageChannelId channelId, TransferMode::Enum transferMode);
  /// Pushes a link incoming channel closed user event message
  void LinkEventIncomingChannelClosed(MessageChannelId channelId);
  /// Pushes a link state change user event message
  void LinkEventStateChange(LinkState::Enum newState);
  /// Pushes a link status change user event message
  void LinkEventStatusChange(LinkStatus::Enum newStatus);
  /// Pushes a link receipt user or protocol event message
  void LinkEventReceipt(MessageReceiptId receiptId, Receipt::Enum receipt, bool forUser);

  /// Pushes an event message to be received later by the user
  void PushUserEventMessage(MoveReference<Message> message);
  /// Pushes an event message to be received later by the protocol
  void PushProtocolEventMessage(MoveReference<Message> message);

  /// Operating Data
  Peer*       mPeer;                 /// Operating peer
  Guid        mTheirGuid;            /// Their peer's permanent GUID
  IpAddress   mTheirIpAddress;       /// Their peer's IP address as seen from our perspective
  IpAddress   mOurIpAddress;         /// Our peer's IP address as seen from their perspective
  LinkInbox   mInbox;                /// Incoming packet manager
  LinkOutbox  mOutbox;               /// Outgoing packet manager
  MessageType mUserMessageTypeStart; /// User messages type start
  void*       mUserData;             /// Optional user data

  /// State Data
  TimeMs                      mLocalToRemoteTimeDifference; /// Local to remote time difference (remote time elapsed - local time elapsed)
  TimeMs                      mCreationTime;                /// Time that the link was created
  TransmissionDirection::Enum mCreationDirection;           /// Direction in which the link was created (which peer initiated the connection)
  bool                        mConnectRequested;            /// Outgoing connect requested?
  BitStream                   mConnectRequestExtraData;     /// Outgoing connect request data
  bool                        mDisconnectRequested;         /// Outgoing disconnect requested?
  BitStream                   mDisconnectRequestExtraData;  /// Outgoing disconnect request data
  UserConnectResponse::Enum   mConnectResponded;            /// Outgoing connect responded?
  BitStream                   mConnectResponseExtraData;    /// Outgoing connect response data
  LinkState::Enum             mState;                       /// Specific link state
  TimeMs                      mStateTime;                   /// Specific link state set time
  MessageReceiptId            mStateACKId;                  /// Specific link state ACK ID
  bool                        mSessionCompleted;            /// Session completed?

  /// Plugin Data
  LinkPluginSet mAddedPlugins;   /// Link plugins which were just added, need to be initialized
  LinkPluginSet mPlugins;        /// Link plugins which are currently active
  LinkPluginSet mRemovedPlugins; /// Link plugins which were just removed, need to be uninitialized and deleted

  /// Configuration Settings
  uint32 mSendRate;                         /// Packet send rate
  uint32 mPacketDataBytes;                  /// Packet data bytes
  TimeMs mPreconnectionRoundTripTime;       /// Preconnection round trip time
  TimeMs mFloorRoundTripTime;               /// Floor round trip time
  float  mPacketSequenceHistoryRangeFactor; /// Packet sequence history range factor
  float  mPacketSequenceHistoryRateFactor;  /// Packet sequence history rate factor
  float  mPacketNAKFactor;                  /// Packet NAK factor
  uint32 mHeartbeatPacketRate;              /// Heartbeat packet send rate
  float  mTimeoutFactor;                    /// Timeout factor
  TimeMs mLatencyLimit;                     /// Latency limit
  float  mConnectAttemptFactor;             /// Connect attempt factor
  float  mDisconnectAttemptFactor;          /// Disconnect attempt factor

  /// Statistics
  bool   mRoundTripTimeUpdated;     /// Round trip time updated?
  TimeMs mRoundTripTimeMin;         /// Minimum round trip time
  TimeMs mRoundTripTimeAvg;         /// Average round trip time
  TimeMs mRoundTripTimeMax;         /// Maximum round trip time

  TimeMs mInternalRoundTripTimeMin; /// Minimum round trip time
  TimeMs mInternalRoundTripTimeAvg; /// Average round trip time
  TimeMs mInternalRoundTripTimeMax; /// Maximum round trip time

  /// Frame Data
  Kbps mOutgoingBandwidth;     /// Outgoing bandwidth (data transfer rate) supported by this link
  Bits mOutgoingFrameCapacity; /// Outgoing bandwidth available since our last update, updated at the start of every update
  Bits mOutgoingFrameSize;     /// Outgoing data sent, deducted by frame capacity at the start of every update

private:
  /// No Copy Constructor
  PeerLink(const PeerLink&);
  /// No Copy Assignment Operator
  PeerLink& operator=(const PeerLink&);

  /// Friends
  friend class Peer;
  friend class LinkInbox;
  friend class LinkOutbox;
  friend class LinkPlugin;
};

//---------------------------------------------------------------------------------//
//                                 LinkPlugin                                      //
//---------------------------------------------------------------------------------//

/// Provides an immediate event interface to customize link behavior
class LinkPlugin
{
public:
  /// Destructor
  virtual ~LinkPlugin();

  //
  // Member Functions
  //

  /// Comparison Operators (compares names)
  bool operator ==(const LinkPlugin& rhs) const;
  bool operator !=(const LinkPlugin& rhs) const;
  bool operator  <(const LinkPlugin& rhs) const;
  bool operator ==(const String& rhs) const;
  bool operator !=(const String& rhs) const;
  bool operator  <(const String& rhs) const;

  /// Returns the link plugin's unique instance name
  const String& GetName() const;

  /// Returns true if the link plugin is initialized, else false
  bool IsInitialized() const;

  /// Returns the initialized link plugin's operating link
  PeerLink* GetLink() const;

  /// Sends a plugin message on the connected link
  /// Enable reliable to continually resend this message every determined NAK for the lifetime of this message
  /// Specify an outgoing message channel open on this link to indicate this message belongs to that channel's message sequence
  /// Enable receipt to be notified of this message's arrival status in a future plugin event callback
  /// Give a higher message priority value to indicate this message should be sent before other queued messages with lower priority values
  /// Messages with equal priority are sent in chronological order with respect to internal fragmentation preferences
  /// Provide a message lifetime to indicate how long this message should be kept alive in the send queue before being discarded
  /// Returns the message's receipt ID if send queing successful and receipt was enabled, else 0
  MessageReceiptId Send(Status& status, const Message& message,         bool reliable, MessageChannelId channelId = 0, bool receipt = false, MessagePriority priority = 0, TimeMs lifetime = cInfiniteTimeMs);
  MessageReceiptId Send(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId = 0, bool receipt = false, MessagePriority priority = 0, TimeMs lifetime = cInfiniteTimeMs);

protected:
  /// Constructor
  LinkPlugin(size_t messageTypeCount);

  //
  // Link Plugin Interface
  //

  /// Called after the link is added, before the plugin is added
  /// Return true to continue using the plugin, else false
  virtual bool OnInitialize() { return true; }
  /// Called before the link is removed, before the plugin is removed
  virtual void OnUninitialize() {}

  /// Called after the link is updated
  virtual void OnUpdate() {}

  /// Called after a connect request is sent
  virtual void OnConnectRequestSend(Message& message) {}
  /// Called after a connect request is received
  virtual void OnConnectRequestReceive(Message& message) {}

  /// Called after a connect response is sent
  virtual void OnConnectResponseSend(Message& message) {}
  /// Called after a connect response is received
  virtual void OnConnectResponseReceive(Message& message) {}

  /// Called after a disconnect notice is sent
  virtual void OnDisconnectNoticeSend(Message& message) {}
  /// Called after a disconnect notice is received
  virtual void OnDisconnectNoticeReceive(Message& message) {}

  /// Called after the link state is changed
  virtual void OnStateChange(LinkState::Enum prevState) {}
  /// Called after the link status is changed
  virtual void OnStatusChange(LinkStatus::Enum prevStatus) {}

  /// Called before a packet is sent
  /// Return true to continue sending the packet, else false
  virtual bool OnPacketSend(OutPacket& packet) { return true; }
  /// Called after a packet is received
  /// Return true to continue receiving the packet, else false
  virtual bool OnPacketReceive(InPacket& packet) { return true; }

  /// Called before a message is sent
  /// Return true to continue sending the message, else false
  virtual bool OnMessageSend(OutMessage& message) { return true; }
  /// Called after a sent message is receipted
  /// Return true to continue receipting the message, else false
  virtual bool OnMessageReceipt(OutMessage& message, Receipt::Enum receipt) { return true; }
  /// Called after a message is received
  /// Return true to continue receiving the message, else false
  virtual bool OnMessageReceive(Message& message) { return true; }

  /// Called before a plugin message is sent
  /// Return true to continue sending the message, else false
  virtual bool OnPluginMessageSend(OutMessage& message) { return true; }
  /// Called after a plugin message is receipted
  virtual void OnPluginMessageReceipt(MoveReference<OutMessage> message, Receipt::Enum receipt) {}
  /// Called after a plugin message is received
  virtual void OnPluginMessageReceive(MoveReference<Message> message, bool& continueProcessingCustomMessages) { continueProcessingCustomMessages = true; }

  /// Called before an incoming channel is opened
  /// Return true to continue opening the incoming channel, else false
  virtual bool OnIncomingChannelOpen(MessageChannelId channelId) { return true; }
  /// Called before an incoming channel is closed
  virtual void OnIncomingChannelClose(MessageChannelId channelId) {}

private:
  //
  // Internal
  //

  /// Sets the link plugin's unique instance name
  void SetName(const String& name);

  /// Initializes the link plugin
  /// Returns true to continue using the link plugin, else false
  bool Initialize(PeerLink* link);
  /// Uninitializes the link plugin
  void Uninitialize();

  /// Returns true if the absolute message type is within the plugin's range, else false
  bool AbsoluteIsInRange(MessageType absoluteType) const;
  /// Returns an absolute message type from a plugin relative message type
  MessageType RelativeToAbsolute(MessageType relativeType) const;
  /// Returns a plugin relative message type from an absolute message type
  MessageType AbsoluteToRelative(MessageType absoluteType) const;

  /// Returns the message type count
  MessageType GetMessageTypeCount() const;
  /// Sets the absolute message type start
  void SetMessageTypeStart(MessageType messageTypeStart);

  /// Unique instance name
  String      mName;
  /// Operating link
  PeerLink*   mLink;
  /// Absolute message type start
  MessageType mMessageTypeStart;
  /// Number of message types this plugin uses
  MessageType mMessageTypeCount;

  /// No Copy Constructor
  LinkPlugin(const LinkPlugin&);
  /// No Copy Assignment Operator
  LinkPlugin& operator=(const LinkPlugin&);

  /// Friends
  friend class PeerLink;
  friend class LinkInbox;
  friend class LinkOutbox;
};

} // namespace Zero
