///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                   NetPeer                                       //
//---------------------------------------------------------------------------------//

/// Network Peer.
/// Acts as a host on the network.
/// Manages network object state and event replication.
class NetPeer : public NetObject, public Peer, public Replicator
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeer();

  /// Resets all configuration settings.
  void ResetConfig();

  /// Returns our project's GUID.
  Guid GetOurProjectGuid();

  //
  // Component Interface
  //

  /// Initializes the component.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Uninitializes the component (closes the peer if still open).
  void OnDestroy(uint flags) override;

  /// Called on engine update (updates the peer).
  void OnEngineUpdate(UpdateEvent* event);

  //
  // Host Interface
  //

  /// Configures the server peer to be discoverable on the local area network.
  void SetLanDiscoverable(bool lanDiscoverable = true);
  bool GetLanDiscoverable() const;

  /// Configures the server peer to be discoverable on the internet.
  /// (Requires master server subscriptions)
  void SetInternetDiscoverable(bool internetDiscoverable = true);
  bool GetInternetDiscoverable() const;

  /// Configures the peer to use connection facilitation (NAT punch-through) when establishing a connection over the internet.
  /// (Requires master server subscriptions)
  void SetFacilitateInternetConnections(bool facilitateInternetConnections = true);
  bool GetFacilitateInternetConnections() const;

  /// Configures the inclusive range of ports used to host this game.
  /// (Must be a valid port value within the range [0, 65535])
  void SetHostPortRangeStart(uint hostPortRangeStart = 8000);
  uint GetHostPortRangeStart() const;

  /// Configures the inclusive range of ports used to host this game.
  /// (Must be a valid port value within the range [0, 65535])
  void SetHostPortRangeEnd(uint hostPortRangeEnd = 9000);
  uint GetHostPortRangeEnd() const;

  /// Controls how often the peer sends a host ping message when discovering or refreshing hosts via ping messages.
  void SetHostPingInterval(float hostPingInterval = 0.25f);
  float GetHostPingInterval() const;

  /// Controls how often the internet discoverable peer sends a host record message to its master server subscriptions.
  /// (Requires master server subscriptions)
  void SetInternetHostPublishInterval(float internetHostPublishInterval = 30.0f);
  float GetInternetHostPublishInterval() const;

  /// Controls the lifetime of every host record stored on the master server.
  void SetInternetHostRecordLifetime(float internetHostRecordLifetime = 60.0f);
  float GetInternetHostRecordLifetime() const;

  /// Controls how many host records from the same IP address may be stored on the master server (used to prevent flood attacks).
  void SetInternetSameIpHostRecordLimit(uint internetSameIpHostRecordLimit = 64);
  uint GetInternetSameIpHostRecordLimit() const;

  /// Subscribes to the master server located at the specified IP address.
  /// All internet host discovery, record publishing, connection facilitation (NAT punch-through) requests
  /// are performed on these master server subscriptions in the order they were subscribed.
  void SubscribeToMasterServer(const IpAddress& ipAddress);
  /// Unsubscribes from the master server located at the specified IP address (if subscribed).
  void UnsubscribeFromMasterServer(const IpAddress& ipAddress);

  /// Returns the first equivalent host found in the given network's host list, else nullptr.
  NetHost* GetHostByIndex(Network::Enum network, uint index) const;
  NetHost* GetHostByAddress(Network::Enum network, const IpAddress& ipAddress) const;
  NetHost* GetHostByAddress(const IpAddress& ipAddress) const;

  /// Adds or finds a host in the given network's host list.
  NetHost* AddOrFindHost(Network::Enum network, const IpAddress& ipAddress);

  /// Removes the specified host from the given network's host list.
  /// Returns true if the host was found and removed, else false (the host could not be found).
  bool RemoveHost(Network::Enum network, const IpAddress& ipAddress);
  bool RemoveHost(const IpAddress& ipAddress);

  /// Returns the given network's host list.
  NetHostRange GetHostList(Network::Enum network) const;

  /// Discovers the given network's host list.
  /// Listen to the NetHost event interface to handle the results.
  /// Returns true if the discovery was successfully started, else false.
  bool DiscoverHostList(Network::Enum network, bool removeStaleHosts);

  /// Clears the given network's host list.
  void ClearHostList(Network::Enum network);
  void ClearHostLists();

  /// Refreshes the specified host in the given network's host list.
  /// Allowing discovery will enable the host to be discovered if it is not an already known host.
  /// Listen to the NetHost event interface to handle the results.
  /// Returns true if the host was found and the refresh successfully started, else false (the host could not be found).
  bool RefreshHost(Network::Enum network, const IpAddress& ipAddress, bool getExtraHostInfo, bool allowDiscovery, bool removeStaleHosts);

  /// Refreshes all hosts in the given network's host list.
  /// Allowing discovery will enable new hosts to be discovered in the process of refreshing.
  /// Listen to the NetHost event interface to handle the results.
  /// Returns true if the refresh was successfully started, else false.
  bool RefreshHostList(Network::Enum network, bool getExtraHostInfo, bool allowDiscovery, bool removeStaleHosts);

  /// Cancels all host discovery and refresh requests currently in progress (the operations will be considered unsuccessful).
  /// Listen to the NetHost event interface to handle the results.
  void CancelHostRequests();

  //
  // Peer Interface
  //

  /// Returns the peer's permanent GUID.
  /// This GUID will never change for the lifetime of this peer.
  Guid GetGuid() const;

  /// Returns true if the peer is open, else false.
  bool IsOpen() const;

  /// Opens the peer with the specified network role, port, and retry settings.
  /// If the peer is opened in offline mode (Role::Offline), the API will act as a pass-through
  /// and simulate all applicable network events locally. Always succeeds.
  /// If binding is unsuccessful, the port increments and tries again for the given number of retries.
  /// Specify port 0 to indicate any available port should be used.
  /// Returns true if successful, else false.
  bool Open(Role::Enum role, uint port, uint retries);
  bool Open(Role::Enum role, uint port);
  bool Open(Role::Enum role);
  bool OpenClient(uint port, uint retries);
  bool OpenClient(uint port);
  bool OpenClient();
  bool OpenServer(uint port);
  bool OpenServer();
  bool OpenMasterServer(uint port);
  bool OpenMasterServer();
  bool OpenOffline();

  /// Closes the peer (safe to call multiple times).
  void Close();

  /// Handles behavior after the peer is opened.
  /// Returns true if successful, else false.
  bool HandleNetPeerOpened();
  /// Handles behavior before the peer is closed.
  /// Returns true if successful, else false.
  bool HandleNetPeerClosed();

  /// Returns the open peer's network role (client, server, offline), else Role::Unspecified.
  Role::Enum GetRole() const;

  /// Returns the open peer's unique network identifier, else 0.
  /// [Server/Offline] This will always be zero.
  /// [Client] This will be non-zero once connected to a server, else zero.
  NetPeerId GetNetPeerId() const;

  /// Returns the peer's local IPv4 address, else IpAddress().
  /// Set if the peer is open with an IPv4 socket.
  const IpAddress& GetIpv4Address() const;
  /// Returns the peer's local IPv4 address host as a numeric address string, else String().
  String GetIpv4Host() const;
  /// Returns the peer's local IPv4 address port, else String().
  uint GetIpv4Port() const;

  /// Returns the peer's local IPv6 address, else IpAddress().
  /// Set if the peer is open with an IPv6 socket.
  const IpAddress& GetIpv6Address() const;
  /// Returns the peer's local IPv6 address host as a numeric address string, else String().
  String GetIpv6Host() const;
  /// Returns the peer's local IPv6 address port, else String().
  uint GetIpv6Port() const;

  /// Returns the duration in milliseconds since the creation of this peer.
  TimeMs GetCreationDuration() const;

  /// Returns the number of net objects in this game session (but not including the net peer itself).
  uint GetNetObjectCount() const;
  /// Returns the number of net users in this game session.
  uint GetNetUserCount() const;
  /// Returns the number of net spaces in this game session.
  uint GetNetSpaceCount() const;

  /// Controls when the user will be warned of their current frame's outgoing bandwidth utilization ratio on any given link.
  void SetFrameFillWarning(float frameFillWarning = 0.8);
  float GetFrameFillWarning() const;

  /// Controls when to skip change replication for the current frame because of remaining outgoing bandwidth utilization ratio on any given link.
  void SetFrameFillSkip(float frameFillSkip = 0.9);
  float GetFrameFillSkip() const;

  //
  // Link Interface
  //

  /// [Client/Server] Returns true if there is a connected network link with the specified IP address, else false.
  bool HasLink(const IpAddress& ipAddress) const;
  /// [Client/Server] Returns true if there is a connected network link with the specified peer ID, else false.
  bool HasLink(NetPeerId netPeerId) const;
  /// [Client/Server] Returns true if there are connected network links in the specified route, else false.
  bool HasLinks(const Route& route) const;
  /// [Client/Server] Returns true if there are connected network links, else false.
  bool HasLinks() const;

  /// [Client/Server] Returns the connected network link with the specified IP address, else nullptr.
  PeerLink* GetLink(const IpAddress& ipAddress) const;
  /// [Client/Server] Returns the connected network link with the specified peer ID, else nullptr.
  PeerLink* GetLink(NetPeerId netPeerId) const;
  /// [Client/Server] Returns the connected network links in the specified route.
  PeerLinkSet GetLinks(const Route& route) const;
  /// [Client/Server] Returns all connected network links.
  PeerLinkSet GetLinks() const;

  // TODO: (User facing, returns ranges of identifiers to script)
  // GetLinksByIpAddress, GetLinksByPeerId, GetLinksByGuid

  /// [Client/Server] Returns the number of connected network links managed by this peer.
  uint GetLinkCount() const;

  /// [Client] Initiates a connect attempt with the remote peer (delayed until end of frame).
  /// Listen to the NetLink event interface to handle the results.
  /// Returns true if a connect request was successfully initiated, else false.
  bool ConnectLink(const IpAddress& ipAddress, EventBundle* requestBundle);
  bool ConnectLink(const IpAddress& ipAddress, Event* requestEvent);
  bool ConnectLink(const IpAddress& ipAddress);

  /// [Client/Server] Disconnects by request from the remote peer.
  /// Listen to the NetLink event interface to handle the results.
  /// Returns true if a disconnect notice was successfully initiated, else false.
  bool DisconnectLink(const IpAddress& ipAddress, EventBundle* requestBundle);
  bool DisconnectLink(const IpAddress& ipAddress, Event* requestEvent);
  bool DisconnectLink(const IpAddress& ipAddress);
  bool DisconnectLink(NetPeerId netPeerId, EventBundle* requestBundle);
  bool DisconnectLink(NetPeerId netPeerId, Event* requestEvent);
  bool DisconnectLink(NetPeerId netPeerId);

  /// [Client/Server] Disconnects by request from all remote peers.
  /// Listen to the NetLink event interface to handle the results.
  /// Returns the number of links disconnected.
  uint DisconnectAllLinks(EventBundle* requestBundle);
  uint DisconnectAllLinks(Event* requestEvent);
  uint DisconnectAllLinks();

  /// [Client/Server] Returns the duration that the link has existed, else 0.
  TimeMs GetLinkCreationDuration(NetPeerId netPeerId) const;
  /// [Client/Server] Returns the direction in which the link was created (which peer initiated the connection), else TransmissionDirection::Unspecified.
  TransmissionDirection::Enum GetLinkCreationDirection(NetPeerId netPeerId) const;

  /// [Client/Server] Returns the link's overall status, else LinkStatus::Unspecified.
  LinkStatus::Enum GetLinkStatus(NetPeerId netPeerId) const;
  /// [Client/Server] Returns the link's specific state, else LinkState::Unspecified.
  LinkState::Enum GetLinkState(NetPeerId netPeerId) const;
  /// [Client/Server] Returns the link's specific state duration, else 0.
  TimeMs GetLinkStateDuration(NetPeerId netPeerId) const;

  /// [Client/Server] Returns the remote peer's permanent GUID, else 0.
  /// (Available if the link is connected or incoming and attempting connection)
  Guid GetLinkGuid(NetPeerId netPeerId) const;

  /// [Client/Server] Returns the remote peer's IP address (as seen from our perspective), else IpAddress().
  /// For outgoing links this is the same IP address specified in our connect call.
  /// This IP address will never change for the lifetime of this link.
  IpAddress GetLinkIpAddress(NetPeerId netPeerId) const;

  /// [Client/Server] Returns our peer's IP address (as seen from their perspective), else IpAddress().
  /// For incoming links this is the same IP address specified in their connect call.
  /// (Available if the link is connected or incoming and attempting connection)
  /// It is absolutely possible that this does not match our local IP address.
  IpAddress GetOurIpAddressFromLink(NetPeerId netPeerId) const;

  /// [Client/Server] Returns the link's IP address protocol version, else InternetProtocol::Unspecified.
  /// This IP address protocol will never change for the lifetime of this link.
  InternetProtocol::Enum GetLinkInternetProtocol(NetPeerId netPeerId) const;

  /// Returns the remote peer's unique network identifier, else 0.
  /// [Server] This will be non-zero if the client is connected, else zero.
  /// [Client] This will always be zero.
  NetPeerId GetLinkNetPeerId(const IpAddress& ipAddress) const;

  //
  // User Interface
  //

  /// Initiates a network user add request to add a new user belonging to our local peer (delayed until end of frame).
  /// Listen to the NetUser event interface to handle the results.
  /// Returns true if the request was successfully initiated, else false.
  bool AddUser(EventBundle* requestBundle);
  bool AddUser(Event* requestEvent);
  bool AddUser();

  /// Returns the specified network user, else nullptr.
  Cog* GetUser(NetUserId netUserId) const;

  /// Returns the network users added by our local peer.
  NetUserRange GetUsersAddedByMyPeer() const;
  /// Returns the network users added by the specified peer.
  NetUserRange GetUsersAddedByPeer(NetPeerId netPeerId) const;

  /// Returns all added network users.
  NetUserRange GetUsers() const;
  /// Returns the number of added network users.
  uint GetUserCount() const;

  /// [Server/Offline] Initiates a network user remove request to remove the specified user.
  /// [Client] Initiates a network user remove request to remove the specified user added by our local peer.
  /// Listen to the NetUser event interface to handle the results.
  /// Returns true if the request was successfully initiated, else false.
  bool RemoveUser(NetUserId netUserId, EventBundle* requestBundle);
  bool RemoveUser(NetUserId netUserId, Event* requestEvent);
  bool RemoveUser(NetUserId netUserId);
  bool RemoveUser(Cog* cog, EventBundle* requestBundle);
  bool RemoveUser(Cog* cog, Event* requestEvent);
  bool RemoveUser(Cog* cog);

  /// Adds the net user to internal lists and allows net object ownership.
  void AddUserInternal(Cog* cog);
  /// Removes the net user from internal lists and disallows net object ownership.
  void RemoveUserInternal(Cog* cog);

  /// Handles pending net requests now.
  void HandlePendingRequests();

  //
  // User Add Handshake
  //

  /// Sends a network user add request.
  /// Returns true if successful, else false.
  bool SendUserAddRequest(EventBundle* ourRequestBundle);
  /// Handles behavior after sending a network user add request.
  void HandleSentUserAddRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* ourRequestBundle);

  /// [Server/Offline] Receives a network user add request.
  /// Returns true if successful, else false.
  bool ReceiveUserAddRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message);
  /// [Server/Offline] Handles behavior after receiving a network user add request.
  /// Returns true to accept the request, else false to deny the request.
  NetUser* HandleReceivedUserAddRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* theirRequestBundle,
                                        EventBundle* returnOurResponseBundle, NetUserId theirNetUserId);

  /// [Server/Offline] Sends a network user add response.
  /// Returns true if successful, else false.
  bool SendUserAddResponse(NetPeerId theirNetPeerId, NetUser* theirNetUser, EventBundle* theirRequestBundle, EventBundle* ourResponseBundle);
  /// [Server/Offline] Handles behavior after sending a network user add response.
  void HandleSentUserAddResponse(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* theirRequestBundle,
                                 NetUserAddResponse::Enum ourAddResponse, EventBundle* ourResponseBundle, NetUser* theirNetUser);

  /// Receives a network user add response.
  /// Returns true if successful, else false.
  bool ReceiveUserAddResponse(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message, EventBundle* ourRequestBundle);
  /// Handles behavior after receiving a network user add response.
  void HandleReceivedUserAddResponse(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* ourRequestBundle,
                                     NetUserAddResponse::Enum theirAddResponse, EventBundle* theirResponseBundle, NetUserId ourNetUserId);

  //
  // User Remove Handshake
  //

  /// Sends a network user remove request.
  /// Returns true if successful, else false.
  bool SendUserRemoveRequest(NetUserId netUserId, EventBundle* ourRequestBundle);
  /// [Server/Offline] Receives a network user remove request.
  /// Returns true if successful, else false.
  bool ReceiveUserRemoveRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message);

  //
  // Object Interface
  //

  /// Handles behavior when the net object is brought online, dispatches events accordingly.
  const String& GetNetObjectOnlineEventId() const override;
  /// Handles behavior when the net object is taken offline, dispatches events accordingly.
  const String& GetNetObjectOfflineEventId() const override;

  //
  // Host Info Interface
  //

  /// Acquires basic project-specific host information (limited to 480 bytes) as a bitstream.
  BitStream GetBasicNetHostInfo();
  /// Acquires extra project-specific host information as a bitstream.
  BitStream GetExtraNetHostInfo();

  /// Removes all hosts from the specified network's host list not found in the responding hosts list.
  void RemoveUnresponsiveHosts(Network::Enum network, const ArraySet<NetHost*>& respondingHosts);

  //
  // Level Replication
  //

  /// [Server] Sends a network level load started message.
  /// Tells the client to start loading the specified level in the given space.
  /// Returns true if successful, else false.
  bool SendLevelLoadStarted(NetPeerId theirNetPeerId, Space* space, Level* level);
  /// [Client] Receives a network level load started message.
  /// Returns true if successful, else false.
  bool ReceiveLevelLoadStarted(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message);

  /// [Server] Sends a network level load finished message.
  /// Tells the client that the level replication is now complete.
  /// Returns true if successful, else false.
  bool SendLevelLoadFinished(NetPeerId theirNetPeerId, Space* space);
  /// [Client] Receives a network level load finished message.
  /// Returns true if successful, else false.
  bool ReceiveLevelLoadFinished(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message);

  /// Handles behavior when a network level is started.
  void HandleNetLevelStarted(Space* space);

  //
  // Game Replication
  //

  /// [Server] Sends a network game load started message.
  /// Returns true if successful, else false.
  bool SendGameLoadStarted(NetPeerId theirNetPeerId);
  /// [Client] Receives a network game load started message.
  /// Returns true if successful, else false.
  bool ReceiveGameLoadStarted(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message);

  /// [Server] Sends a network game load finished message.
  /// Returns true if successful, else false.
  bool SendGameLoadFinished(NetPeerId theirNetPeerId);
  /// [Client] Receives a network game load finished message.
  /// Returns true if successful, else false.
  bool ReceiveGameLoadFinished(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message);

  /// Handles behavior when a network game is started.
  void HandleNetGameStarted();

  //
  // ID Helpers
  //

  /// [Server/Offline] Assigns a network user ID to the specified network user.
  /// Returns true if successful, else false.
  bool AssignNetUserId(NetUser* user);
  /// [Server/Offline] Releases a network user ID from the specified network user.
  void ReleaseNetUserId(NetUser* user);

  //
  // Replication Interface
  //

  /// Returns the live net space specified if it is known locally, else nullptr.
  Space* GetNetSpace(NetObjectId netObjectId) const;
  /// Returns the live net object specified if it is known locally, else nullptr.
  Cog* GetNetObject(NetObjectId netObjectId) const;

  /// [Client] Claims the active replica stream belongs to the specified cog initializer.
  /// Intended to be called redundantly by objects in the hierarchy to either make initial claim or confirm claim.
  /// Returns true if successful, else false (could be another initializer has claim or there is currently no active replica stream).
  bool ClaimActiveReplicaStream(const CogInitializer* cogInitializer);
  /// [Client] Sets the active replica stream used while creating a net object hierarchy.
  void SetActiveReplicaStream(const ReplicaStream* replicaStream);
  /// [Client] Returns the active replica stream used while creating a net object hierarchy.
  const ReplicaStream* GetActiveReplicaStream() const;

  /// [Client/Server] Emplaces the invalid net object locally against the game setup.
  /// Returns true if successful, else false.
  bool EmplaceNetObjectByGameSetup(Cog* cog);
  /// [Client/Server] Emplaces the invalid net object locally against the specified net space and level.
  /// Returns true if successful, else false.
  bool EmplaceNetObjectBySpaceAndLevel(Cog* cog, Space* space, StringParam levelResourceIdName);

  /// [Server] Spawns all present, invalid net objects in the family tree locally and remotely along the route.
  /// Returns true if successful, else false.
  bool SpawnFamilyTree(FamilyTreeId familyTreeId, const Route& route);
  /// [Server] Clones all present, live net objects in the family tree locally and remotely along the route.
  /// Returns true if successful, else false.
  bool CloneFamilyTree(FamilyTreeId familyTreeId, const Route& route);

  /// [Server] Spawns the invalid net object locally and remotely along the route.
  /// Returns true if successful, else false.
  bool SpawnNetObject(Cog* cog, const Route& route);
  /// [Server] Clones the live net object locally and remotely along the route.
  /// Returns true if successful, else false.
  bool CloneNetObject(Cog* cog, const Route& route);
  /// [Client] Forgets the valid net object locally.
  /// [Server] Forgets the live net object locally and remotely along the route.
  /// Returns true if successful, else false.
  bool ForgetNetObject(Cog* cog, const Route& route);
  /// [Server] Destroys the live net object locally and remotely along the route.
  /// Returns true if successful, else false.
  bool DestroyNetObject(Cog* cog, const Route& route);

  /// [Server] Clones the entire network game on the specified link.
  /// Returns true if successful, else false.
  bool CloneNetGame(NetPeerId netPeerId);
  /// [Server] Clones the network space's current level and net objects on the specified link.
  /// Returns true if successful, else false.
  bool CloneNetLevel(Space* space, bool isLevelTransition, NetPeerId netPeerId);
  /// [Server] Clones the network space's current level and net objects on all links.
  /// Returns true if successful, else false.
  bool CloneNetLevel(Space* space, bool isLevelTransition);
  /// [Client] Is currently receiving the entire network game.
  bool IsReceivingNetGame() const;

  //
  // Family Tree Interface
  //

  /// [Client/Server] Adds a non-emplaced net object (ancestor or descendant) to it's appropriate family tree, creating the tree if doesn't exist.
  /// These MUST be added in depth-first pre-order traversal order!
  /// Returns true if successful, else false.
  bool AddNetObjectToFamilyTree(NetObject* ancestor, NetObject* netObject);
  /// [Client/Server] Removes a non-emplaced net object (ancestor or descendant) from it's appropriate family tree, deleting the tree if it's empty.
  /// These may be removed in any order!
  /// When removed the net object is actually just marked absent (pointer is cleared to null).
  /// Returns true if successful, else false.
  bool RemoveNetObjectFromFamilyTree(NetObject* netObject);

  //
  // Internal
  //

  /// Serializes the net event.
  /// Returns true if successful, else false.
  bool SerializeNetEvent(BitStreamExtended& bitStream, Event* netEvent, Cog* destination);
  /// Deserializes the net event.
  /// Returns true if successful, else false.
  bool DeserializeNetEvent(const BitStreamExtended& bitStream, Event*& netEvent, Cog*& destination, NetPeerId netPeerId);

  /// Handles behavior when a dispatched net event is sent.
  void HandleNetEventSent(Event* netEvent, Cog* destination, NetPeerId netPeerId);
  /// Handles behavior when a received net event is dispatched.
  void HandleNetEventReceived(Event* netEvent, Cog* destination, NetPeerId netPeerId);

  /// Validates the net event.
  /// Returns true if successful, else false.
  bool ValidateNetEvent(StringParam netEventId, Event* netEvent, TransmissionDirection::Enum direction);

  /// [Client/Server/Offline] Dispatches the net event on the local peer.
  /// Returns true if successful, else false.
  bool DispatchLocalInternal(StringParam netEventId, Event* netEvent, Cog* destination = nullptr);

  /// [Client/Server] Dispatches the net event on the remote peer.
  /// Returns true if successful, else false.
  bool DispatchRemoteInternal(StringParam netEventId, Event* netEvent, NetPeerId netPeerId, Cog* destination = nullptr);

  /// [Client/Server] Dispatches the net event on all remote peers.
  /// Returns true if successful, else false.
  bool DispatchBroadcastInternal(StringParam netEventId, Event* netEvent, Cog* destination = nullptr);

  //
  // Channel Type Management
  //

  /// Finds the specified net channel type or adds and configures it if it does not exist.
  /// Returns the found or added net channel type with the specified name.
  NetChannelType* GetOrAddReplicaChannelType(const String& netChannelTypeName, NetChannelConfig* netChannelConfig = nullptr);

  //
  // Property Type Management
  //

  /// Finds the specified net property type or adds and configures it if it does not exist.
  /// Returns the found or added net property type with the specified name.
  NetPropertyType* GetOrAddReplicaPropertyType(const String& netPropertyTypeName, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn, NetPropertyConfig* netPropertyConfig = nullptr);

  //
  // Replicator Replica Interface
  //

  /// Serializes replicas to a replica stream.
  /// Gaps may be represented as null replicas and additional data may be serialized as needed.
  /// Returns true if successful, else false.
  bool SerializeReplicas(const ReplicaArray& replicas, ReplicaStream& replicaStream) override;
  /// Deserializes replicas from a replica stream.
  /// Creates or finds replicas as necessary depending on replica info and the replica stream mode.
  /// Gaps may be represented as null replicas and additional data may be deserialized as needed.
  /// Returns true if successful, else false.
  bool DeserializeReplicas(const ReplicaStream& replicaStream, ReplicaArray& replicas) override;

  /// Creates replicas using the replica stream and creation info (create context and replica type).
  /// Gaps may be represented as null replicas.
  /// Returns true if successful, else false.
  bool CreateReplicas(const CreateContext& createContext, const ReplicaType& replicaType, const ReplicaStream& replicaStream, ReplicaArray& replicas);

  /// Deletes invalid replicas (if they were originally spawned, otherwise do nothing).
  /// Gaps may be represented as null replicas.
  /// Returns true if successful, else false.
  bool ReleaseReplicas(const ReplicaArray& replicas) override;

  /// Called before a replica is made valid (registered with the replicator).
  void OnValidReplica(Replica* replica) override;
  /// Called after a replica is made live (assigned a replica ID by the server replicator).
  void OnLiveReplica(Replica* replica) override;
  /// Called before a replica is made invalid (unregistered with the replicator).
  void OnInvalidReplica(Replica* replica, bool isForget) override;

  /// Called after a replica channel property has legitimately changed, determined using comparisons, in a particular replication phase.
  void OnReplicaChannelPropertyChange(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, Replica* replica, ReplicaChannel* replicaChannel, ReplicaProperty* replicaProperty, TransmissionDirection::Enum direction) override;

  //
  // Replicator Link Interface
  //

  /// Called before a link is added.
  void AddingLink(PeerLink* link) override;
  /// Called before a link is removed.
  void RemovingLink(PeerLink* link) override;

  //
  // Replicator Handshake Sequence Interface
  //

  /// [Client] Called after sending a connect request.
  void ClientOnConnectRequest(ReplicatorLink* link, ConnectRequestData& connectRequestData) override;
  /// [Client] Called after receiving a connect response.
  /// If accepted, our replicator ID is set immediately before this and a connect confirmation is sent after this.
  /// If denied, our replicator ID is cleared and link is destroyed immediately after this.
  BitStream ClientOnConnectResponse(ReplicatorLink* link, ConnectResponseData& connectResponseData) override;
  /// [Client] Called after sending a connect confirmation.
  void ClientOnConnectConfirmation(ReplicatorLink* link, BitStream& connectConfirmationData) override;
  /// [Client] Called after sending or receiving a disconnect notice.
  /// Our replicator ID is cleared and link is destroyed immediately after this.
  void ClientOnDisconnectNotice(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction) override;

  /// [Server] Called after receiving a connect request.
  /// If accepted, their replicator ID is assigned immediately after this.
  /// Return true to accept the connect request, else false.
  Pair<bool, BitStream> ServerOnConnectRequest(ReplicatorLink* link, ConnectRequestData& connectRequestData) override;
  /// [Server] Called after sending a connect response.
  /// If denied, their replicator ID is released and link is destroyed immediately after this.
  void ServerOnConnectResponse(ReplicatorLink* link, ConnectResponseData& connectResponseData) override;
  /// [Server] Called after receiving a connect confirmation.
  void ServerOnConnectConfirmation(ReplicatorLink* link, BitStream& connectConfirmationData) override;
  /// [Server] Called after sending or receiving a disconnect notice.
  /// Their replicator ID is released and link is destroyed immediately after this.
  void ServerOnDisconnectNotice(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction) override;

  //
  // Internal
  //

  /// Handles behavior when a net link is connected.
  void HandleNetLinkConnected(ReplicatorLink* link, BitStream& connectConfirmationData, TransmissionDirection::Enum direction);
  /// Handles behavior when a net link is disconnected.
  void HandleNetLinkDisconnected(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction);

  /// Processes a custom packet received by the peer.
  static void ProcessReceivedCustomPacket(Peer* peer, InPacket& packet);
  /// Processes a custom message received by the link.
  /// Return true to continue processing custom messages on this link, else false (will continue next update call).
  static bool ProcessReceivedCustomMessage(PeerLink* link, Message& message);


  //
  // RecordPublishing
  //

  /// On servers that are internet discoverable, it will send out a host data to all its subscribed master servers on a periodic interval.
  void UpdatePublishInterval(UpdateEvent* event);

  bool HandlePing(IpAddress const& theirIpAddress, NetHostPingData& netHostPingData);


  //
  // Master Server
  //

  /// When a master receives a publishing from a server, it becomes stored.
  void MasterServerRecievePublish(const IpAddress& theirIpAddress, EventBundle& basicHostInfo, Guid const& thierProjectGuid);
  /// Increases lifetime of server records, removes old server records.
  void UpdateHostRecords(UpdateEvent* event);
  /// Remove a NetHostRecord by IP address from a project record map. Intended to give users control over HostRecords in the master server.
  void RemoveNetHostRecord(Guid const& projectGuid, IpAddress const & netHostRecordIp);
  /// Gets a reference to the Project's HostRecord Map. If it doesn't exist, it creates it.
  HostRecordsMap& GetProjectRecordsMap(Guid const& projectGuid);
  /// When a master server receives a connection, it creates a message and sends it across the link.
  void OnNetPeerSentConnectResponse(NetPeerSentConnectResponse* event);
  /// The client looks for link connect responses and handles them accordingly.
  void OnNetPeerReceivedConnectResponse(NetPeerReceivedConnectResponse* event);
  /// Internal helper: Clients will try to connect to master servers in succession using this function.
  bool TryMasterServerConnection();
  /// When the master server receives a host publish message, it is handled here.
  void ReceiveHostPublish(IpAddress const& theirIpAddress, Message& message);
  /// Checks to see if the IP address is one of the clients subscribed master servers.
  bool IsSubscribedMasterServer(IpAddress const& ipAddress);

  //
  // Client Handle Events
  //

  void OnNetPeerSentConnectRequest(NetPeerSentConnectRequest* event);
  void OnNetLinkConnected(NetLinkConnected* event);
  void OnNetLinkDisconnected(NetLinkDisconnected* event);

  //
  // Timeout Get/Sets
  //

  void SetInternetHostListTimeout(float internetHostListTimeout = 4.0f);
  float GetInternetHostListTimeout() const;

  void SetBasicHostInfoTimeout(float internetHostListTimeout = 3.0f);
  float GetBasicHostInfoTimeout() const;

  void SetExtraHostInfoTimeout(float internetHostListTimeout = 4.0f);
  float GetExtraHostInfoTimeout() const;

  // Data
  bool                              mIsOpenOffline;                  ///< Is peer open in offline mode?
  PendingNetUserArray               mPendingUserRequests;            ///< [Client] Pending outgoing network user add requests.
  NetUserSet                        mAddedUsers;                     ///< Added network user objects.
  NetUserSet                        mOurAddedUsers;                  ///< Added network user objects added by our peer.
  ArrayMap<NetPeerId, NetUserSet>   mTheirAddedUsers;                ///< Added network user objects added by remote peers.
  IdStore<NetUserId>                mNetUserIdStore;                 ///< [Server/Offline] Network user ID store.
  NetPeerReceivedUserAddRequest*    mActiveUserAddRequest;           ///< [Server/Offline] Active net user add request (used temporarily during net user creation).
  bool                              mIsReceivingNetGame;             ///< [Client] Is currently receiving the net game?
  bool                              mPendingNetGameStarted;          ///< [Client] Delayed net game started event.
  Array<NetRequest>                 mPendingRequests;                ///< Pending outgoing network requests.
  bool                              mWaitingOnConnectResponse;       ///< [Client] Currently waiting on a connect response?
  FamilyTreeSet                     mFamilyTrees;                    ///< [Client/Server] Network object family trees.
  IdStore<FamilyTreeId>             mFamilyTreeIdStore;              ///< [Client/Server] Network object family tree ID store.
  const ReplicaStream*              mActiveReplicaStream;            ///< [Client] Active replica stream (used temporarily during net object creation).
  const CogInitializer*             mActiveCogInitializer;           ///< [Client] Active cog initializer, determines replica stream context (used temporarily during net object creation).
  bool                              mLanDiscoverable;                ///< Configures the server peer to be discoverable on the local area network.
  bool                              mInternetDiscoverable;           ///< Configures the server peer to be discoverable on the internet.
  bool                              mFacilitateInternetConnections;  ///< Configures the peer to use connection facilitation (NAT punch-through) when establishing a connection over the internet.
  uint                              mHostPortRangeStart;             ///< Configures the inclusive range of ports used to host this game.
  uint                              mHostPortRangeEnd;               ///< Configures the inclusive range of ports used to host this game.
  float                             mInternetHostPublishInterval;    ///< Controls how often the internet discoverable peer sends a host record message to its master server subscriptions.
  Array<IpAddress>                  mMasterServerSubscriptions;      ///< Master servers this peer has subscribed to.
  NetHostsMap                       mHostLists;                      ///< Network host lists.
  float                             mPublishElapsedTime;             ///< [Server] how much time has elapsed since it last published a record.
  float                             mInternetHostListTimeout;        ///< [Client/Server] Determines the amount of time the client is willing to wait to get a host list from master server.
  float                             mBasicHostInfoTimeout;           ///< [Client/Server] Determines the amount of time the client is willing to wait for.
  float                             mExtraHostInfoTimeout;           ///< [Client/Server] Determines the amount of time the client will wait for extra host info from a server.
  float                             mHostPingInterval;               ///< [Client/Server] The time between (potentially) redundant pings from a net peer.
  PingManager                       mPingManager;                    ///< [Client/Server/MtrSrv] Ping manager is capable of sending and receiving pings.
  uint                              mNextManagerId;                  ///< Ping managers need an id to be unique. We use this to prescribe unique ids.

  // Data for master server.
  float                             mInternetHostRecordLifetime;     ///< Controls the lifetime of every host record stored on the master server.
  uint                              mInternetSameIpHostRecordLimit;  ///< Controls how many host records from the same IP address may be stored on the master server (used to prevent flood attacks).
  HashMap<String, uint>             mIpAddressServerCounts;          ///< Keeps track of how many servers there are per IP address.
  HashMap<Guid, HostRecordsMap>     mProjectHostRecordMaps;          ///< A map of project to guid to host record maps. This is used to identify if Host records exist, and quickly get references to them.
  HostRecordsArray                  mHostRecords;                    ///< The structure which stores all of the HostRecords pointers.
  bool                              mIsOpenMasterServer;             ///< Is peer open in MasterServer mode?
  RecieptIpMap                      mReceiptRecipients;              ///< A Map the server uses to determine which peer links to terminate.

  /// Host Discovery.
  InternetHostDiscovery             mInternetHostDiscovery;          ///< A class which uses the net peer to discover internet hosts.
  LanHostDiscovery                  mLanHostDiscovery;               ///< A class which uses the net peer to discover LAN hosts.
};

//---------------------------------------------------------------------------------//
//                                 NetLinkData                                     //
//---------------------------------------------------------------------------------//

/// Network Link Data.
/// Contains additional NetPeer API data associated with a remote peer.
class NetLinkData
{
public:
  /// Constructor.
  NetLinkData();

  // TODO: Use this or remove it
};

} // namespace Zero
