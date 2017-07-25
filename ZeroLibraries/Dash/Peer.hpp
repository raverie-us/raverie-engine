///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Typedefs
typedef ArraySet< PeerLink*, PointerSortPolicy<PeerLink*> >     PeerLinkSet;
typedef ArraySet< PeerPlugin*, PointerSortPolicy<PeerPlugin*> > PeerPluginSet;

/// Processes a custom packet received by the peer
typedef void (*ProcessReceivedCustomPacketFn)(Peer* peer, InPacket& packet);

/// Processes a custom message received by the link
/// Return true to continue processing custom messages on this link, else false (will continue next update call)
typedef bool (*ProcessReceivedCustomMessageFn)(PeerLink* link, Message& message);

//---------------------------------------------------------------------------------//
//                                    Peer                                         //
//---------------------------------------------------------------------------------//

/// Acts as a host on the network
class Peer : public BandwidthStats<true>
{
  /// Resets the peer's session data to it's default state
  void ResetSession();

public:
  /// Creates a closed peer and assigns a permanent GUID
  Peer(ProcessReceivedCustomPacketFn processReceivedCustomPacketFn, ProcessReceivedCustomMessageFn processReceivedCustomMessageFn);

  /// Destroys the peer (closes the peer if still open)
  ~Peer();

  /// Comparison Operators (compares GUIDs)
  bool operator ==(const Peer& rhs) const;
  bool operator !=(const Peer& rhs) const;
  bool operator  <(const Peer& rhs) const;
  bool operator ==(Guid rhs) const;
  bool operator !=(Guid rhs) const;
  bool operator  <(Guid rhs) const;

  //
  // Member Functions
  //

  /// Returns the Dash Peer protocol identifier
  static ProtocolId GetProtocolId();

  /// Returns the peer's permanent GUID
  /// This GUID will never change for the lifetime of this peer
  Guid GetGuid() const;

  /// Returns the peer's local IPv4 address, else IpAddress()
  /// Set if the peer is open with an IPv4 socket
  const IpAddress& GetLocalIpv4Address() const;

  /// Returns the peer's local IPv6 address, else IpAddress()
  /// Set if the peer is open with an IPv6 socket
  const IpAddress& GetLocalIpv6Address() const;

  /// Returns true if the peer is open, else false
  bool IsOpen() const;

  /// Returns the open peer's IP address protocol version, else InternetProtocol::Unspecified
  InternetProtocol::Enum GetInternetProtocol() const;

  /// Returns the open peer's transport layer protocol, else TransportProtocol::Unspecified
  TransportProtocol::Enum GetTransportProtocol() const;

  /// Opens the closed peer on the specified port (closes the peer if already open)
  /// Acquires socket and thread resources used to run the peer
  /// Initializes any pre-existing links and plugins managed by this peer
  /// Specifying InternetProtocol::Both will attempt to open both IPv4 and IPv6 sockets
  void Open(Status& status, ushort port = AnyPort, InternetProtocol::Enum internetProtocol = InternetProtocol::Both, TransportProtocol::Enum transportProtocol = TransportProtocol::Udp);

  /// Closes the peer (safe to call multiple times)
  /// Uninitializes any initialized links and plugins managed by this peer
  /// Frees socket and thread resources used to run the peer
  void Close();

  /// Sends a standalone packet to the specified remote address on the open peer
  /// Returns true if successful, else false
  bool Send(const IpAddress& ipAddress, const Message& message);
  bool Send(const IpAddress& ipAddress, const Array<Message>& messages);

  /// Processes incoming packets, updates peer and link state, and generates outgoing packets
  /// Received peer packets and link messages are processed in this call
  /// Should be called every application step
  /// Returns true if successful, else false
  bool Update();

  /// Returns the current local update time
  TimeMs GetLocalTime() const;
  /// Returns the current local update delta time
  TimeMs GetLocalDeltaTime() const;
  /// Returns the current local update frame ID
  uint64 GetLocalFrameId() const;

  /// Sets optional user data associated with the peer
  void SetUserData(void* userData = nullptr);
  /// Returns optional user data associated with the peer
  template<typename T>
  T* GetUserData() const { return static_cast<T*>(GetUserData()); }
  void* GetUserData() const;

  //
  // Peer Link Management
  //

  /// Creates a link corresponding to the specified address if one is not already managed by this peer
  /// Returns the new link, else nullptr
  PeerLink* CreateLink(const IpAddress& ipAddress);

  /// Returns the link corresponding to the specified address managed by this peer, else nullptr
  PeerLink* GetLink(const IpAddress& ipAddress) const;
  /// Returns all links managed by this peer
  PeerLinkSet GetLinks() const;
  /// Returns the number of links with the specified status managed by this peer
  uint GetLinkCount(LinkStatus::Enum linkStatus) const;
  /// Returns the number of links managed by this peer
  uint GetLinkCount() const;

  /// Destroys a link managed by this peer
  void DestroyLink(const IpAddress& ipAddress);
  /// Destroys all links managed by this peer
  void DestroyLinks();

  //
  // Peer Plugin Management
  //

  /// Adds a peer plugin corresponding to the specified name if one is not already active on this peer
  /// Returns the added peer plugin, else nullptr
  PeerPlugin* AddPlugin(PeerPlugin* plugin, StringParam name);

  /// Returns the peer plugin corresponding to the specified name active on this peer, else nullptr
  template<typename T>
  T* GetPlugin(StringParam name) const { return static_cast<T*>(GetPlugin(name)); }
  PeerPlugin* GetPlugin(StringParam name) const;
  /// Returns all peer plugins active on this peer
  PeerPluginSet GetPlugins() const;
  /// Returns the number of peer plugins active on this peer
  uint GetPluginCount() const;

  /// Removes a peer plugin active on this peer
  void RemovePlugin(StringParam name);
  /// Removes all peer plugins active on this peer
  void RemovePlugins();

  //
  // Peer Configuration
  //

  /// Resets all peer configuration settings
  void ResetConfig();

  /// Sets the maximum number of links this peer may have
  /// This affects how many remote peers this peer can respond to at any given time
  void SetLinkLimit(uint linkLimit = 256);
  /// Returns the maximum number of links this peer may have
  uint GetLinkLimit() const;

  /// Sets the maximum number of connected links this peer may have
  /// This affects how many remote peers this peer can be connected to at any given time
  void SetConnectionLimit(uint connectionLimit = 64);
  /// Returns the maximum number of connected links this peer may have
  uint GetConnectionLimit() const;

  /// Sets the connect response policy this peer will use upon receiving an incoming connect request
  /// This affects how and when remote peers are able to initiate incoming links
  void SetConnectResponseMode(ConnectResponseMode::Enum connectResponseMode = ConnectResponseMode::Accept);
  /// Returns the connect response policy this peer will use upon receiving an incoming connect request
  ConnectResponseMode::Enum GetConnectResponseMode() const;

  /// Returns a summary of all peer configuration settings as an array of key-value string pairs
  Array< Pair<String, String> > GetConfigSummary() const;
  /// Returns a summary of all peer configuration settings as a single multi-line string (intended for debugging convenience)
  String GetConfigSummaryString() const;

  //
  // Peer Statistics
  //

  /// Resets all applicable peer and link statistics to start over relative to a new statistics period
  void ResetStats();

  /// Returns the minimum number of links
  uint GetMinLinks() const;
  /// Returns the average number of links
  float GetAvgLinks() const;
  /// Returns the maximum number of links
  uint GetMaxLinks() const;

  /// Returns the minimum number of connected links
  uint GetMinConnections() const;
  /// Returns the average number of connected links
  float GetAvgConnections() const;
  /// Returns the maximum number of connected links
  uint GetMaxConnections() const;

  /// Returns a summary of all peer statistics as an array of pairs containing the property name and array of minimum, average, and maximum values
  Array< Pair< String, Array<String> > > GetStatsSummary() const;
  /// Returns a summary of all peer statistics as a single multi-line string (intended for debugging convenience)
  String GetStatsSummaryString() const;

  //
  // Internal
  //

  /// Initializes all peer statistics
  void InitializeStats();

  /// Updates the link statistics
  void UpdateLinks(uint32 sample);
  /// Updates the connection statistics
  void UpdateConnections(uint32 sample);

  /// Updates and returns the current local update time
  /// (Exclusively used by the user thread)
  TimeMs UpdateAndGetLocalTime();
  /// Updates and returns the current packet send time
  /// (Exclusively used by the Peer's send thread)
  TimeMs UpdateAndGetSendTime();
  /// Updates and returns the current packet receive time
  /// (Exclusively used by the Peer's receive thread)
  TimeMs UpdateAndGetReceiveTime();

  /// Sends an outgoing packet to the network
  /// Returns true if successful, else false
  bool SendPacket(OutPacket& outPacket);

  /// Updates packet send statistics
  void UpdateSendStats(Bytes sentPacketBytes);
  /// Updates packet receive statistics
  void UpdateReceiveStats(Bytes receivedPacketBytes);

  /// Returns true if the provided raw packet is valid for our protocol, else false
  static bool IsValidRawPacket(RawPacket& rawPacket);

  /// Receives incoming IPv4 packets from the network
  OsInt Ipv4ReceiveThreadFn();
  /// Receives incoming IPv6 packets from the network
  OsInt Ipv6ReceiveThreadFn();

  /// Processes incoming packets, updates peer and link state, and generates outgoing packets
  void UpdatePeerState();
  /// Processes all received custom packets
  void ProcessReceivedCustomPackets();
  /// Processes a custom packet received by the peer
  void ProcessReceivedCustomPacket(InPacket& packet);

  // Translate raw incoming packets into packets that can be processed
  void TranslateRawPackets(Array<RawPacket>& rawPackets, Array<InPacket>& inPackets);

  /// Called before a packet is sent
  /// Return true to continue sending the packet, else false
  bool PluginEventOnPacketSend(OutPacket& packet);
  /// Called after a packet is received
  /// Return true to continue receiving the packet, else false
  bool PluginEventOnPacketReceive(InPacket& packet);

  /// Called before a link is added
  /// Return true to continue adding the link, else false
  bool PluginEventOnLinkAdd(PeerLink* link);
  /// Called before a link is removed
  void PluginEventOnLinkRemove(PeerLink* link);

  /// Pushes a peer incoming link created event message
  void PeerEventIncomingLinkCreated(const IpAddress& ipAddress);
  /// Pushes a peer fatal error event message
  void PeerEventFatalError(const String& errorString);

  /// Pushes an event message to be received later by the user
  void PushUserEventMessage(MoveReference<Message> message);

  /// Operating Data
  Guid                           mGuid;                           /// Permanent GUID
  ProcessReceivedCustomPacketFn  mProcessReceivedCustomPacketFn;  /// ProcessReceivedCustomPacket user function
  ProcessReceivedCustomMessageFn mProcessReceivedCustomMessageFn; /// ProcessReceivedCustomMessage user function
  IpAddress                      mIpv4Address;                    /// IPv4 peer address
  IpAddress                      mIpv6Address;                    /// IPv6 peer address
  Socket                         mIpv4Socket;                     /// IPv4 TCP/UDP socket
  Socket                         mIpv6Socket;                     /// IPv6 TCP/UDP socket
  InternetProtocol::Enum         mInternetProtocol;               /// IP address protocol version
  TransportProtocol::Enum        mTransportProtocol;              /// Transport layer protocol
  void*                          mUserData;                       /// Optional user data

  /// Thread Data
  Atomic<bool>   mFatalError;            /// Fatal error occurred?
  mutable Thread mIpv4ReceiveThread;     /// IPv4 socket receive thread
  Atomic<bool>   mExitIpv4ReceiveThread; /// Exit IPv4 socket receive thread?
  mutable Thread mIpv6ReceiveThread;     /// IPv6 socket receive thread
  Atomic<bool>   mExitIpv6ReceiveThread; /// Exit IPv6 socket receive thread?

  /// State Data
  Timer  mLocalTimer;   /// Local update timer
  Timer  mSendTimer;    /// Packet send timer
  Timer  mReceiveTimer; /// Packet receive timer
  uint64 mLocalFrameId; /// Local update frame ID

  /// Packet Data
  Array<RawPacket>   mIpv4RawPackets;            /// Raw incoming IPv4 packets
  mutable ThreadLock mIpv4RawPacketsLock;        /// Raw incoming IPv4 packets thread lock
  Array<RawPacket>   mIpv6RawPackets;            /// Raw incoming IPv6 packets
  mutable ThreadLock mIpv6RawPacketsLock;        /// Raw incoming IPv6 packets thread lock
  BitStream          mSendBitStream;             /// Reusable outgoing packet bitstream
  mutable ThreadLock mReceiveStatsLock;          /// Receive stats thread lock
  Array<InPacket>    mReleasedCustomPackets;     /// Released incoming user packets
  mutable ThreadLock mReleasedCustomPacketsLock; /// Released incoming user packets thread lock

  /// Link Data
  PeerLinkSet mCreatedLinks;   /// Links which were just created, need to be added
  PeerLinkSet mLinks;          /// Links which are currently active
  PeerLinkSet mDestroyedLinks; /// Links which were just destroyed, need to be removed and deleted

  /// Plugin Data
  PeerPluginSet mAddedPlugins;   /// Peer plugins which were just added, need to be initialized
  PeerPluginSet mPlugins;        /// Peer plugins which are currently active
  PeerPluginSet mRemovedPlugins; /// Peer plugins which were just removed, need to be uninitialized and deleted

  /// Configuration Settings
  Atomic<uint32> mLinkLimit;           /// Maximum number of links this peer may have
  Atomic<uint32> mConnectionLimit;     /// Maximum number of connected links this peer may have
  Atomic<uint32> mConnectResponseMode; /// Connect response policy this peer will use upon receiving an incoming connect request

  /// Statistics
  Atomic<bool>   mLinksUpdated;       /// Links updated?
  Atomic<uint32> mLinksMin;           /// Minimum links
  Atomic<float>  mLinksAvg;           /// Average links
  Atomic<uint32> mLinksMax;           /// Maximum links

  Atomic<bool>   mConnectionsUpdated; /// Connections updated?
  Atomic<uint32> mConnectionsMin;     /// Minimum connections
  Atomic<float>  mConnectionsAvg;     /// Average connections
  Atomic<uint32> mConnectionsMax;     /// Maximum connections

private:
  /// No Copy Constructor
  Peer(const Peer&);
  /// No Copy Assignment Operator
  Peer& operator=(const Peer&);

  /// Friends
  friend class PeerLink;
  friend class PeerPlugin;
};

//---------------------------------------------------------------------------------//
//                                 PeerPlugin                                      //
//---------------------------------------------------------------------------------//

/// Provides an immediate event interface to customize peer behavior
class PeerPlugin
{
public:
  /// Destructor
  /// NOTE: This is NOT virtual, it's the responsibility of the user to clean-up their plugin!
  ~PeerPlugin();

  //
  // Member Functions
  //

  /// Comparison Operators (compares names)
  bool operator ==(const PeerPlugin& rhs) const;
  bool operator !=(const PeerPlugin& rhs) const;
  bool operator  <(const PeerPlugin& rhs) const;
  bool operator ==(const String& rhs) const;
  bool operator !=(const String& rhs) const;
  bool operator  <(const String& rhs) const;

  /// Returns the peer plugin's unique instance name
  const String& GetName() const;

  /// Returns true if the peer plugin is initialized, else false
  bool IsInitialized() const;

  /// Returns the initialized peer plugin's operating peer
  Peer* GetPeer() const;

protected:
  /// Constructor
  PeerPlugin();

  //
  // Peer Plugin Interface
  //

  /// Return true if this peer plugin should be deleted after being removed, else false
  virtual bool ShouldDeleteAfterRemoval() { return true; }

  /// Called after the peer is opened, before the peer plugin is added
  /// Return true to continue using the peer plugin, else false
  virtual bool OnInitialize() { return true; }
  /// Called before the peer is closed, before the peer plugin is removed
  virtual void OnUninitialize() {}

  /// Called after the peer is updated
  virtual void OnUpdate() {}

  /// Called before a packet is sent
  /// Return true to continue sending the packet, else false
  virtual bool OnPacketSend(OutPacket& packet) { return true; }
  /// Called after a packet is received
  /// Return true to continue receiving the packet, else false
  virtual bool OnPacketReceive(InPacket& packet) { return true; }

  /// Called before a link is added
  /// Return true to continue adding the link, else false
  virtual bool OnLinkAdd(PeerLink* link) { return true; }
  /// Called before a link is removed
  virtual void OnLinkRemove(PeerLink* link) {}

private:
  //
  // Internal
  //

  /// Sets the peer plugin's unique instance name
  void SetName(const String& name);

  /// Initializes the peer plugin
  /// Returns true to continue using the peer plugin, else false
  bool Initialize(Peer* peer);
  /// Uninitializes the peer plugin
  void Uninitialize();

  /// Unique instance name
  String mName;
  /// Operating peer
  Peer*  mPeer;

  /// No Copy Constructor
  PeerPlugin(const PeerPlugin&);
  /// No Copy Assignment Operator
  PeerPlugin& operator=(const PeerPlugin&);

  /// Friends
  friend class Peer;
};

//
// Additional Notes:
//
// Peers act as a host on the network. Links maintain connection state associated with a remote peer.
// Links provide bi-directional communication with a remote peer, similar to TCP but with more flexibility.
// Links are considered outgoing if they were initiated by our local peer, or incoming if they were initiated by a remote peer.
//
// Peers manage their own links, but links are created and destroyed by user request. Incoming links are always created by the peer.
// Links may be reused to manage infinitely many sessions with a remote peer as all session-specific state is reset upon disconnect.
//
// Peers are implemented as multithreaded objects to ensure maximum responsiveness over the network.
//
// Plugins provide an immediate event handling interface to customize peer and link behavior.
// Links and plugins may be added and removed from the peer at any time regardless of whether it's open or closed.
// Links and plugins are initialized at the beginning of the open peer's update and are uninitialized at the end of the open peer's update.
// This means links and plugins effectively operate within a peer's open-close period on update.
//

} // namespace Zero
