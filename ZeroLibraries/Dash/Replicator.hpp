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
//                                 Replicator                                      //
//---------------------------------------------------------------------------------//

/// Replicator Peer Plugin
/// Manages local objects and outgoing state replication
class Replicator : public PeerPlugin
{
public:
  /// Resets all session data, as if the replicator was just created
  void ResetSession();

  /// Constructor
  /// Creates a replicator with the specified network role
  Replicator(Role::Enum role = Role::Unspecified);

  //
  // Operations
  //

  /// Sets our network role
  /// (Cannot be modified after the replicator is initialized)
  void SetRole(Role::Enum role);
  /// Returns our network role
  Role::Enum GetRole() const;

  /// [Server] This will always be zero
  /// [Client] This will be non-zero once connected to a server, else zero
  /// Returns our replicator ID
  ReplicatorId GetReplicatorId() const;

  /// Returns true if there is a connected replicator link with the specified IP address, else false
  bool HasLink(const IpAddress& ipAddress) const;
  /// Returns true if there is a connected replicator link with the specified replicator ID, else false
  bool HasLink(ReplicatorId replicatorId) const;
  /// Returns true if there are connected replicator links in the specified route, else false
  bool HasLinks(const Route& route) const;
  /// Returns true if there are connected replicator links, else false
  bool HasLinks() const;

  /// Returns the connected replicator link with the specified IP address, else nullptr
  PeerLink* GetLink(const IpAddress& ipAddress) const;
  /// Returns the connected replicator link with the specified replicator ID, else nullptr
  PeerLink* GetLink(ReplicatorId replicatorId) const;
  /// Returns the connected replicator links in the specified route
  PeerLinkSet GetLinks(const Route& route) const;
  /// Returns all connected replicator links
  PeerLinkSet GetLinks() const;

  /// Sends a reliable user message over the ordered command channel remotely along the route
  void Send(Status& status, const Message& message, const Route& route = Route::All);

  /// Sets optional user data
  void SetUserData(void* userData = nullptr);
  /// Returns optional user data
  template <typename T>
  T* GetUserData() const { return static_cast<T*>(GetUserData()); }
  void* GetUserData() const;

  //
  // Replica Management
  //

  /// Returns true if the specified live replica is known locally, else false
  bool HasReplica(ReplicaId replicaId) const;
  bool HasReplica(Replica* replica) const;
  /// Returns true if there are any replicas in the specified create context known locally, else false
  bool HasReplicasByCreateContext(const CreateContext& createContext) const;
  /// Returns true if there are any live replicas of the specified replica type known locally, else false
  bool HasReplicasByReplicaType(const ReplicaType& replicaType) const;
  /// Returns true if there are any replicas in the specified emplace context known locally, else false
  bool HasReplicasByEmplaceContext(const EmplaceContext& emplaceContext) const;
  /// Returns true if the replica in the specified emplace context is known locally, else false
  bool HasReplicaByEmplaceContext(const EmplaceContext& emplaceContext, EmplaceId emplaceId) const;
  /// Returns true if there are any live replicas known locally, else false
  bool HasReplicas() const;

  /// Returns the specified live replica if it is known locally, else nullptr
  Replica* GetReplica(ReplicaId replicaId) const;
  Replica* GetReplica(Replica* replica) const;
  /// Returns all replicas in the specified create context known locally
  ReplicaSet GetReplicasByCreateContext(const CreateContext& createContext) const;
  /// Returns all live replicas of the specified replica type known locally
  ReplicaSet GetReplicasByReplicaType(const ReplicaType& replicaType) const;
  /// Returns all replicas in the specified emplace context known locally
  ReplicaSet GetReplicasByEmplaceContext(const EmplaceContext& emplaceContext) const;
  /// Returns the replica in the specified emplace context if it is known locally, else nullptr
  Replica* GetReplicaByEmplaceContext(const EmplaceContext& emplaceContext, EmplaceId emplaceId) const;
  /// Returns all live replicas known locally
  const ReplicaSet& GetReplicas() const;

  /// Returns the number of replicas in the specified create context known locally
  size_t GetReplicaCountByCreateContext(const CreateContext& createContext) const;
  /// Returns the number of live replicas of the specified replica type known locally
  size_t GetReplicaCountByReplicaType(const ReplicaType& replicaType) const;
  /// Returns the number of replicas in the specified emplace context known locally
  size_t GetReplicaCountByEmplaceContext(const EmplaceContext& emplaceContext) const;
  /// Returns the number of live replicas known locally
  size_t GetReplicaCount() const;

  //
  // Replica Commands
  //

  /// Emplaces the invalid replica in the emplace context locally
  /// [Client] This makes the replica valid
  /// [Server] This makes the replica live, ready to be cloned to clients who already have emplaced this replica remotely
  /// Returns true if successful, else false
  bool EmplaceReplica(Replica* replica, const EmplaceContext& emplaceContext);
  /// Emplaces the invalid replicas in the emplace context locally
  /// [Client] This makes the replicas valid
  /// [Server] This makes the replicas live, ready to be cloned to clients who already have emplaced these replica remotely
  /// Returns true if successful, else false
  bool EmplaceReplicas(const ReplicaArray& replicas, const EmplaceContext& emplaceContext);

  /// [Server] Spawns the invalid replica locally and remotely along the route
  /// This makes the replica live, ready to be cloned to clients who do not have this replica remotely
  /// Returns true if successful, else false
  bool SpawnReplica(Replica* replica, const Route& route = Route::All);
  /// [Server] Spawns the invalid replicas locally and remotely along the route
  /// This makes the replicas live, ready to be cloned to clients who do not have these replicas remotely
  /// Returns true if successful, else false
  bool SpawnReplicas(const ReplicaArray& replicas, const Route& route = Route::All);

  /// [Server] Clones the live replica remotely along the route
  /// If emplaced, this makes the valid replica live remotely, if spawned, this creates the replica and makes it live remotely
  /// Returns true if successful, else false
  bool CloneReplica(Replica* replica, const Route& route = Route::All);
  /// [Server] Clones the live replicas remotely along the route
  /// If emplaced, this makes the valid replicas live remotely, if spawned, this creates the replicas and makes them live remotely
  /// Returns true if successful, else false
  bool CloneReplicas(const ReplicaArray& replicas, const Route& route = Route::All);
  /// [Server] Clones all live replicas remotely along the route
  /// If emplaced, this makes the valid replicas live remotely, if spawned, this creates the replicas and makes them live remotely
  /// Note: Behavior may not be as expected if UseHeirarchySpawnOptimization is enabled
  /// Returns true if successful, else false
  bool CloneAllReplicas(const Route& route = Route::All);

  /// [Client] Forgets the valid replica locally
  /// [Server] Forgets the live replica locally and remotely along the route
  /// This makes the replica invalid
  /// Returns true if successful, else false
  bool ForgetReplica(Replica* replica, const Route& route = Route::All);
  /// [Client] Forgets the valid replicas locally
  /// [Server] Forgets the live replicas locally and remotely along the route
  /// This makes the replicas invalid
  /// Returns true if successful, else false
  bool ForgetReplicas(const ReplicaArray& replicas, const Route& route = Route::All);
  /// [Client] Forgets all valid replicas locally
  /// [Server] Forgets all live replicas locally and remotely along the route
  /// This makes the replicas invalid
  /// Returns true if successful, else false
  bool ForgetAllReplicas(const Route& route = Route::All);

  /// [Server] Destroys the live replica locally and remotely along the route
  /// Locally, this only makes the replica invalid, remotely, this makes the replica invalid and deletes it
  /// Returns true if successful, else false
  bool DestroyReplica(Replica* replica, const Route& route = Route::All);
  /// [Server] Destroys the live replicas locally and remotely along the route
  /// Locally, this only makes the replicas invalid, remotely, this makes the replicas invalid and deletes them
  /// Returns true if successful, else false
  bool DestroyReplicas(const ReplicaArray& replicas, const Route& route = Route::All);
  /// [Server] Destroys all live replicas locally and remotely along the route
  /// Locally, this only makes the replicas invalid, remotely, this makes the replicas invalid and deletes them
  /// Returns true if successful, else false
  bool DestroyAllReplicas(const Route& route = Route::All);

  /// [Server] Interrupts the current step remotely along the route
  /// When received, custom message processing stops for the current step and is resumed next step
  /// Designed to support fixed frame-delay behavior without introducing an explicit handshake sequence
  /// Returns true if successful, else false
  bool Interrupt(const Route& route = Route::All);

  //
  // Configuration
  //

  /// Resets all configuration settings
  void ResetConfig();

  /// Controls when the user will be warned of their current frame's outgoing bandwidth utilization ratio on any given link
  void SetFrameFillWarning(float frameFillWarning = 0.8);
  float GetFrameFillWarning() const;

  /// Controls when to skip change replication for the current frame because of remaining outgoing bandwidth utilization ratio on any given link
  void SetFrameFillSkip(float frameFillSkip = 0.9);
  float GetFrameFillSkip() const;

  //
  // Replica Channel Type Management
  //

  /// Returns true if the replicator has the specified replica channel type, else false
  bool HasReplicaChannelType(const String& replicaChannelTypeName) const;
  /// Returns the specified replica channel type, else nullptr
  const ReplicaChannelType* GetReplicaChannelType(const String& replicaChannelTypeName) const;
  ReplicaChannelType* GetReplicaChannelType(const String& replicaChannelTypeName);
  /// Returns all replica channel types managed by the replicator
  const ReplicaChannelTypeSet& GetReplicaChannelTypes() const;
  ReplicaChannelTypeSet& GetReplicaChannelTypes();

  /// Adds the replica channel type
  /// Returns the replica channel type if successful, else nullptr (a replica channel type of that name already exists)
  ReplicaChannelType* AddReplicaChannelType(ReplicaChannelTypePtr replicaChannelType);
  /// Removes the specified replica channel type
  /// (Note: Not safe to call while the replica channel type is in use by replica channels!)
  /// Returns true if successful, else false (a replica channel type of that name could not be found)
  bool RemoveReplicaChannelType(const String& replicaChannelTypeName);

  /// Removes all replica channel types
  /// (Note: Not safe to call while the replica channel types are in use by replica channels!)
  void ClearReplicaChannelTypes();

  //
  // Replica Property Type Management
  //

  /// Returns true if the replicator has the specified replica property type, else false
  bool HasReplicaPropertyType(const String& replicaPropertyTypeName) const;
  /// Returns the specified replica property type, else nullptr
  const ReplicaPropertyType* GetReplicaPropertyType(const String& replicaPropertyTypeName) const;
  ReplicaPropertyType* GetReplicaPropertyType(const String& replicaPropertyTypeName);
  /// Returns all replica property types managed by the replicator
  const ReplicaPropertyTypeSet& GetReplicaPropertyTypes() const;
  ReplicaPropertyTypeSet& GetReplicaPropertyTypes();

  /// Adds the replica property type
  /// Returns the replica property type if successful, else nullptr (a replica property type of that name already exists)
  ReplicaPropertyType* AddReplicaPropertyType(ReplicaPropertyTypePtr replicaPropertyType);
  /// Removes the specified replica property type
  /// (Note: Not safe to call while the replica property type is in use by replica properties!)
  /// Returns true if successful, else false (a replica property type of that name could not be found)
  bool RemoveReplicaPropertyType(const String& replicaPropertyTypeName);

  /// Removes all replica property types
  /// (Note: Not safe to call while the replica property types are in use by replica properties!)
  void ClearReplicaPropertyTypes();

  //
  // Replica Interface
  //

  /// Serializes replicas to a replica stream
  /// Gaps may be represented as null replicas and additional data may be serialized as needed
  /// Returns true if successful, else false
  virtual bool SerializeReplicas(const ReplicaArray& replicas, ReplicaStream& replicaStream) = 0;
  /// Deserializes replicas from a replica stream
  /// Creates or finds replicas as necessary depending on replica info and the replica stream mode
  /// Gaps may be represented as null replicas and additional data may be deserialized as needed
  /// Returns true if successful, else false
  virtual bool DeserializeReplicas(const ReplicaStream& replicaStream, ReplicaArray& replicas) = 0;

  /// Deletes invalid replicas (if they were originally spawned, otherwise do nothing)
  /// Gaps may be represented as null replicas
  /// Returns true if successful, else false
  virtual bool ReleaseReplicas(const ReplicaArray& replicas) = 0;

  /// Called before a replica is made valid (registered with the replicator)
  void ValidReplica(Replica* replica);
  virtual void OnValidReplica(Replica* replica) {}
  /// Called after a replica is made live (assigned a replica ID by the server replicator)
  void LiveReplica(Replica* replica);
  virtual void OnLiveReplica(Replica* replica) {}
  /// Called before a replica is made invalid (unregistered with the replicator)
  void InvalidReplica(Replica* replica, bool isForget);
  virtual void OnInvalidReplica(Replica* replica, bool isForget) {}

  /// Called after a replica channel property has legitimately changed, determined using comparisons, in a particular replication phase
  virtual void OnReplicaChannelPropertyChange(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, Replica* replica, ReplicaChannel* replicaChannel, ReplicaProperty* replicaProperty, TransmissionDirection::Enum direction) {}

  //
  // Link Interface
  //

  /// Called before a link is added
  virtual void AddingLink(PeerLink* link) {}
  /// Called before a link is removed
  virtual void RemovingLink(PeerLink* link) {}

  //
  // Handshake Sequence Interface
  //

  /// [Client] Called after sending a connect request
  virtual void ClientOnConnectRequest(ReplicatorLink* link, ConnectRequestData& connectRequestData) {}
  /// [Client] Called after receiving a connect response
  /// If accepted, our replicator ID is set immediately before this and a connect confirmation is sent after this
  /// If denied, our replicator ID is cleared and link is destroyed immediately after this
  virtual BitStream ClientOnConnectResponse(ReplicatorLink* link, ConnectResponseData& connectResponseData) { return BitStream(); }
  /// [Client] Called after sending a connect confirmation
  virtual void ClientOnConnectConfirmation(ReplicatorLink* link, BitStream& connectConfirmationData) {}
  /// [Client] Called after sending or receiving a disconnect notice
  /// Our replicator ID is cleared and link is destroyed immediately after this
  virtual void ClientOnDisconnectNotice(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction) {}

  /// [Server] Called after receiving a connect request
  /// If accepted, their replicator ID is assigned immediately after this
  /// Return true to accept the connect request, else false
  virtual Pair<bool, BitStream> ServerOnConnectRequest(ReplicatorLink* link, ConnectRequestData& connectRequestData) { return Pair<bool, BitStream>(true, BitStream()); }
  /// [Server] Called after sending a connect response
  /// If denied, their replicator ID is released and link is destroyed immediately after this
  virtual void ServerOnConnectResponse(ReplicatorLink* link, ConnectResponseData& connectResponseData) {}
  /// [Server] Called after receiving a connect confirmation
  virtual void ServerOnConnectConfirmation(ReplicatorLink* link, BitStream& connectConfirmationData) {}
  /// [Server] Called after sending or receiving a disconnect notice
  /// Their replicator ID is released and link is destroyed immediately after this
  virtual void ServerOnDisconnectNotice(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction) {}

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

  /// Adds the live replica to be known locally
  /// Returns true if successful, else false
  bool AddLiveReplica(Replica* replica);
  /// Removes the live replica from being known locally
  void RemoveLiveReplica(Replica* replica);

  /// Adds the emplaced replica to be known locally
  /// Returns true if successful, else false
  bool AddEmplacedReplica(Replica* replica);
  /// Removes the emplaced replica from being known locally
  void RemoveEmplacedReplica(Replica* replica);

  //
  // ID Helpers
  //

  /// [Client] Sets our replicator ID
  void SetReplicatorId(ReplicatorId replicatorId);

  /// [Server] Assigns a replicator ID to the specified replicator link
  /// Returns true if successful, else false
  bool AssignReplicatorId(ReplicatorLink* link);
  /// [Server] Releases a replicator ID from the specified replicator link
  void ReleaseReplicatorId(ReplicatorLink* link);

  /// [Server] Assigns a replica ID to the specified replica
  /// Returns true if successful, else false
  bool AssignReplicaId(Replica* replica);
  /// [Server] Releases a replica ID from the specified replica
  void ReleaseReplicaId(Replica* replica);

  /// Assigns an emplace ID to the specified replica
  /// Returns true if successful, else false
  bool AssignEmplaceId(Replica* replica);
  /// Releases an emplace ID from the specified replica
  void ReleaseEmplaceId(Replica* replica);

  //
  // Replication Helpers
  //

  /// Returns true if the message should include an accurate local timestamp, else false (a timestamp will be estimated by the remote peer)
  static bool ShouldIncludeAccurateTimestampOnInitialization(const ReplicaArray& replicas);
  static bool ShouldIncludeAccurateTimestampOnUninitialization(const ReplicaArray& replicas);
  static bool ShouldIncludeAccurateTimestampOnChange(ReplicaChannel* replicaChannel);

  /// Returns the first appropriate timestamp found in the replicas array, else cInvalidMessageTimestamp
  static TimeMs GetInitializationTimestamp(const ReplicaArray& replicas);
  static TimeMs GetUninitializationTimestamp(const ReplicaArray& replicas);

  /// Handles an emplace command
  /// Returns true if successful, else false
  bool HandleEmplace(const ReplicaArray& replicas, const EmplaceContext& emplaceContext, TimeMs timestamp);

  /// Handles a spawn command
  /// Returns true if successful, else false
  bool HandleSpawn(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Routes a spawn command
  /// Returns true if successful, else false
  bool RouteSpawn(const ReplicaArray& replicas, const Route& route, TimeMs timestamp);

  /// Handles a clone command
  /// Returns true if successful, else false
  bool HandleClone(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Routes a clone command
  /// Returns true if successful, else false
  bool RouteClone(const ReplicaArray& replicas, const Route& route, TimeMs timestamp);

  /// Handles a forget command
  /// Returns true if successful, else false
  bool HandleForget(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Routes a forget command
  /// Returns true if successful, else false
  bool RouteForget(const ReplicaArray& replicas, const Route& route, TimeMs timestamp);

  /// Handles a destroy command
  /// Returns true if successful, else false
  bool HandleDestroy(const ReplicaArray& replicas, TransmissionDirection::Enum direction, TimeMs timestamp);
  /// [Server] Routes a destroy command
  /// Returns true if successful, else false
  bool RouteDestroy(const ReplicaArray& replicas, const Route& route, TimeMs timestamp);

  /// Routes a replica channel change
  /// Returns true if successful, else false
  bool RouteChange(ReplicaChannel* replicaChannel, const Route& route, TimeMs timestamp);
  /// Serializes a replica channel change
  /// Returns true if successful, else false
  bool SerializeChange(ReplicaChannel* replicaChannel, Message& message, TimeMs timestamp);

  /// [Server] Routes an interrupt command
  /// Returns true if successful, else false
  bool RouteInterrupt(const Route& route);

  //
  // Peer Plugin Interface
  //

  /// Return true if this peer plugin should be deleted after being removed, else false
  bool ShouldDeleteAfterRemoval() override;

  /// Called after the peer is opened, before the peer plugin is added
  /// Return true to continue using the peer plugin, else false
  bool OnInitialize() override;
  /// Called before the peer is closed, before the peer plugin is removed
  void OnUninitialize() override;

  /// Called after the peer is updated
  void OnUpdate() override;

  /// Called before a link is added
  /// Return true to continue adding the link, else false
  bool OnLinkAdd(PeerLink* link) override;
  /// Called before a link is removed
  void OnLinkRemove(PeerLink* link) override;

  //---------------------------------------------------------------------------------//
  //                                 ItemCacher                                      //
  //---------------------------------------------------------------------------------//

  /// Item Cache Helper
  /// Provides custom behavior around ItemCache for easier replication
  template <typename Id>
  class ItemCacher : private ItemCache<Variant, Id>
  {
    /// Typedefs
    typedef Variant                             Item;
    typedef typename ItemCache<Variant, Id>     ItemCache;
    typedef typename ItemCache::item_id_range   item_id_range;
    typedef typename ItemCache::item_id_pointer item_id_pointer;

  public:
    /// Constructor
    ItemCacher(Replicator* replicator, ReplicatorMessageType::Enum replicatorMessageType)
      : ItemCache(),
        mReplicator(replicator),
        mReplicatorMessageType(replicatorMessageType)
    {
    }

    /// Resets the item cache to it's default state
    void Reset()
    {
      ItemCache::Reset();
    }

    /// [Server] Maps a unique item to a generated ID
    /// Returns true if successful, else false
    bool MapItem(const Item& item)
    {
      Assert(mReplicator->GetRole() == Role::Server);
      Assert(!IsItemMapped(item));

      // Map item to ID
      item_id_pointer iter = ItemCache::MapItem(item);
      if(!iter) // Unable?
      {
        Error("Unable to map item to ID - Error mapping item");
        return false;
      }

      // Route item
      item_id_range range(iter, iter + 1);
      bool result = RouteItems(range, Route::All);
      Assert(result);

      Assert(IsItemMapped(item));

      // Success
      return true;
    }
    /// [Server] Returns true if the item is mapped to an ID, else false
    bool IsItemMapped(const Item& item) const
    {
      Assert(mReplicator->GetRole() == Role::Server);

      return ItemCache::IsItemMapped(item);
    }
    /// [Server] Returns the ID the item is mapped to, else 0 (invalid ID)
    Id GetMappedItemId(const Item& item) const
    {
      Assert(mReplicator->GetRole() == Role::Server);
      Assert(IsItemMapped(item));

      return ItemCache::GetMappedItemId(item);
    }

    /// [Client] Maps a unique ID to a given item
    void MapId(Id id, const Item& item)
    {
      Assert(mReplicator->GetRole() == Role::Client);
      Assert(!IsIdMapped(id));

      // Map ID to item
      if(!ItemCache::MapIdOverwrite(id, item)) // Performed an overwrite?
        Warn("Potential error when mapping ID to item - Overwrite performed instead of an Insert");

      Assert(IsIdMapped(id));
    }
    /// [Client] Returns true if the ID is mapped to a item, else false
    bool IsIdMapped(Id id) const
    {
      Assert(mReplicator->GetRole() == Role::Client);

      return ItemCache::IsIdMapped(id);
    }
    /// [Client] Returns the item the ID is mapped to, else nullptr
    Item GetMappedIdItem(Id id) const
    {
      Assert(mReplicator->GetRole() == Role::Client);
      Assert(IsIdMapped(id));

      return ItemCache::GetMappedIdItemValue(id, Item());
    }

    /// [Server] Routes cache items
    /// Returns true if successful, else false
    bool RouteItems(item_id_range items, const Route& route)
    {
      Assert(mReplicator->GetRole() == Role::Server);

      // Route items
      PeerLinkSet links = mReplicator->GetLinks(route);
      if(!links.Empty()) // Links in route?
      {
        // Serialize items
        Message message(mReplicatorMessageType);
        if(!SerializeItems(items, message)) // Unable?
          return false;

        // For all replicator links in route
        forRange(PeerLink* link, links.All())
        {
          // Send items
          ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");
          SendItems(replicatorLink, message);
        }
      }

      // Success
      return true;
    }
    bool RouteAllItems(const Route& route)
    {
      return RouteItems(ItemCache::GetItemIdMap().All(), route);
    }
    /// [Server] Serializes cache items
    /// Returns true if successful, else false
    bool SerializeItems(item_id_range items, Message& message)
    {
      Assert(mReplicator->GetRole() == Role::Server);

      // Serialize items
      BitStream& bitStream = message.GetData();

      // For all items
      forRange(item_id_range::value_type& item, items)
      {
        // Write item ID
        if(!bitStream.Write(item.second)) // Unable?
        {
          Assert(false);
          return false;
        }

        // Write item
        if(!SerializeUnknownBasicVariant(SerializeDirection::Write, bitStream, item.first)) // Unable?
        {
          Assert(false);
          return false;
        }
      }

      // Success
      return true;
    }
    /// [Client] Deserializes cache items
    /// Returns true if successful, else false
    bool DeserializeItems(const Message& message)
    {
      Assert(mReplicator->GetRole() == Role::Client);

      // Deserialize items
      const BitStream& bitStream = message.GetData();

      // For all items
      while(bitStream.GetBitsUnread())
      {
        // Read item ID
        Id id;
        if(!bitStream.Read(id)) // Unable?
        {
          Assert(false);
          return false;
        }
        Assert(id != 0);

        // Read item
        Item item;
        if(!SerializeUnknownBasicVariant(SerializeDirection::Read, const_cast<BitStream&>(bitStream), item)) // Unable?
        {
          Assert(false);
          return false;
        }
        // Assert(item.IsValid());

        // Map ID to item
        MapId(id, item);
      }

      // Success
      return true;
    }

    /// [Server] Sends cache items
    /// Returns true if successful, else false
    bool SendItems(ReplicatorLink* replicatorLink, Message& message)
    {
      Assert(mReplicator->GetRole() == Role::Server);
      Assert(message.GetType() == mReplicatorMessageType);

      // Send items message
      Assert(replicatorLink->GetCommandChannelId());
      Status status;
      replicatorLink->LinkPlugin::Send(status, message, true, replicatorLink->GetCommandChannelId());
      if(status.Failed()) // Unable?
        return false;

      // Success
      return true;
    }
    /// [Client] Receives cache items
    /// Returns true if successful, else false
    bool ReceiveItems(const Message& message)
    {
      Assert(mReplicator->GetRole() == Role::Client);
      Assert(message.GetType() == mReplicatorMessageType);

      // Deserialize items
      return DeserializeItems(message);
    }

    /// Data
    Replicator*                 mReplicator;            /// Operating replicator
    ReplicatorMessageType::Enum mReplicatorMessageType; /// Replicator message type
  };
  typedef ItemCacher<CreateContextId>  CreateContextCacher;
  typedef ItemCacher<ReplicaTypeId>    ReplicaTypeCacher;
  typedef ItemCacher<EmplaceContextId> EmplaceContextCacher;

  /// Data
  Role::Enum             mRole;                 /// Network role
  ReplicatorId           mReplicatorId;         /// Our replicator ID
  IdStore<ReplicatorId>  mReplicatorIdStore;    /// [Server] Replicator ID store
  IdStore<ReplicaId>     mReplicaIdStore;       /// [Server] Replica ID store
  EmplaceIdStores        mEmplaceIdStores;      /// Emplace ID stores mapped by emplace context
  ReplicaSet             mReplicaSet;           /// Locally known live replicas
  CreateMap              mCreateMap;            /// Locally known created replicas mapped by create context
  ReplicaMap             mReplicaMap;           /// Locally known live replicas mapped by replica type
  EmplaceMap             mEmplaceMap;           /// Locally known emplaced replicas mapped by emplace context
  CreateContextCacher    mCreateContextCacher;  /// CreateContext item cacher
  ReplicaTypeCacher      mReplicaTypeCacher;    /// ReplicaType item cacher
  EmplaceContextCacher   mEmplaceContextCacher; /// EmplaceContext item cacher
  void*                  mUserData;             /// Optional user data
  float                  mFrameFillWarning;     /// Controls when the user will be warned of their current frame's outgoing bandwidth utilization ratio on any given link
  float                  mFrameFillSkip;        /// Controls when to skip change replication for the current frame because of remaining outgoing bandwidth utilization ratio on any given link
  ReplicaChannelTypeSet  mReplicaChannelTypes;  /// Replica channel type set
  ReplicaPropertyTypeSet mReplicaPropertyTypes; /// Replica property type set

private:
  /// No copy constructor
  Replicator(const Replicator&);
  /// No copy assignment operator
  Replicator& operator=(const Replicator&);
};

/// Returns the replicator's display name (provided for convenience)
String GetReplicatorDisplayName(Replicator* replicator);
String GetReplicatorDisplayName(ReplicatorLink* replicatorLink);
String GetReplicatorDisplayName(const IpAddress& ipAddress, Role::Enum role, ReplicatorId replicatorId);

} // namespace Zero
