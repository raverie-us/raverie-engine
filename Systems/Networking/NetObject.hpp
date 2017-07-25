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
//                                  NetObject                                      //
//---------------------------------------------------------------------------------//

/// Network Object.
/// Manages the replication of a single object on the network.
class NetObject : public Replica, public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetObject();

  /// Destructor.
  ~NetObject();

  //
  // Component Interface
  //

  /// Initializes the component.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;

  /// Uninitializes the component.
  void OnDestroy(uint flags) override;

  /// Called when attached to another object.
  void OnAttached(HierarchyEvent* event);
  void OnDetached(HierarchyEvent* event);

  //
  // NetProperty Registration
  //

  /// Adds C++ component net properties to this net object.
  void AddCppNetProperties();
  void OnRegisterCppNetProperties(RegisterCppNetProperties* event);
  /// Adds script component net properties to this net object.
  void AddScriptNetProperties();
  /// Adds configured (property grid) component net properties to this net object.
  void AddConfiguredNetProperties();
  /// Adds net channel authority net properties to this net object.
  void AddNetChannelAuthorityNetProperties();

  //
  // NetObject Scope
  //

  /// Sets the is-ancestor flag (only applies to non-emplaced net objects).
  void InitializeIsAncestor();
  /// Sets the create context (space net object ID).
  void InitializeCreateContext();
  /// Sets the replica type (archetype resource ID).
  void InitializeReplicaType();

  /// [Client/Server] Creates the complete family tree representing this net object as the ancestor and all of it's net object children recursively as descendants.
  void InitializeFamilyTree();
  void AddDownFamilyTree(NetObject* ancestor);

  /// Reads identification information (such as IsAbsent, ReplicaId, IsCloned, IsEmplaced, EmplaceContext, and EmplaceId) from the replica stream.
  void ReadIdentificationInfo(const ReplicaStream* replicaStream, bool& isAbsent);
  /// Reads channel data (such as forward and reverse ReplicaChannels) from the replica stream.
  void ReadChannelData(const ReplicaStream* replicaStream);

  /// Brings the net object online (may be frame-delayed depending on role).
  /// Appropriately calls Emplace/Spawn/Emplace-Clone and HandleNetObjectOnline depending on role.
  void BringNetObjectOnline();
  /// Takes the net object offline (but is not responsible for destroying the cog itself locally).
  /// Appropriately calls Forget/Destroy and HandleNetObjectOffline depending on role.
  void TakeNetObjectOffline();

  /// Handles behavior when the net object is brought online, dispatches events accordingly.
  void HandleNetObjectOnline();
  virtual const String& GetNetObjectOnlineEventId() const;
  virtual void HandleNetObjectOnlinePreDispatch(NetObjectOnline* event);
  /// Handles behavior when the net object is taken offline, dispatches events accordingly.
  void HandleNetObjectOffline();
  virtual const String& GetNetObjectOfflineEventId() const;
  virtual void HandleNetObjectOfflinePostDispatch(NetObjectOffline* event);

  //
  // Peer Interface
  //

  /// Returns our open peer's network role (client, server, offline), else Role::Unspecified.
  Role::Enum GetRole() const;
  /// Returns true if our open peer's network role is client, else false.
  bool IsClient() const;
  /// Returns true if our open peer's network role is server, else false.
  bool IsServer() const;
  /// Returns true if our open peer's network role is offline, else false.
  bool IsOffline() const;
  /// Returns true if our open peer's network role is client or offline, else false.
  bool IsClientOrOffline() const;
  /// Returns true if our open peer's network role is server or offline, else false.
  bool IsServerOrOffline() const;
  /// Returns true if our open peer's network role is client or server, else false.
  bool IsClientOrServer() const;

  //
  // Object Interface
  //

  /// Resets all configuration settings.
  void ResetConfig();

  /// Controls whether or not net channels on this net object may detect outgoing changes.
  void SetDetectOutgoingChanges(bool detectOutgoingChanges = true);
  bool GetDetectOutgoingChanges() const;

  /// Controls whether or not net channels on this net object may accept incoming changes.
  void SetAcceptIncomingChanges(bool acceptIncomingChanges = true);
  bool GetAcceptIncomingChanges() const;

  /// Controls whether or not net channels on this net object may nap (perform change detection on longer intervals) if they haven't changed in a while.
  void SetAllowNapping(bool allowNapping = true);
  bool GetAllowNapping() const;

  /// Controls whether or not the net object will serialize an accurate timestamp value when brought online, or will instead accept an estimated timestamp value.
  void SetAccurateTimestampOnOnline(bool accurateTimestampOnOnline = false);
  bool GetAccurateTimestampOnOnline() const;

  /// Controls whether or not the net object will serialize an accurate timestamp value when changed (on any net channel), or will instead accept an estimated timestamp value.
  /// (Enabling this will override the corresponding net channel type setting for all net channels added to this net object)
  void SetAccurateTimestampOnChange(bool accurateTimestampOnChange = false);
  bool GetAccurateTimestampOnChange() const;

  /// Controls whether or not the net object will serialize an accurate timestamp value when taken offline, or will instead accept an estimated timestamp value.
  void SetAccurateTimestampOnOffline(bool accurateTimestampOnOffline = false);
  bool GetAccurateTimestampOnOffline() const;

  /// Timestamp indicating when this net object was brought online, else 0.
  float GetOnlineTimestamp() const;
  /// Timestamp indicating when this net object was last changed, else 0.
  float GetLastChangeTimestamp() const;
  /// Timestamp indicating when this net object was taken offline, else 0.
  float GetOfflineTimestamp() const;

  /// Elapsed time passed since this net object was brought online, else 0.
  float GetOnlineTimePassed() const;
  /// Elapsed time passed since this net object was last changed, else 0.
  float GetLastChangeTimePassed() const;
  /// Elapsed time passed since this net object was taken offline, else 0.
  float GetOfflineTimePassed() const;

  /// Forces all net channels on this net object to stop napping immediately.
  void WakeUp();
  /// Forces all net channels on this net object to start napping immediately.
  void TakeNap();

  /// [Client/Server] Replicates all net channels' property changes immediately (only where changes are detected).
  /// Will also update nap state as configured.
  /// Returns true if changes were replicated, else false.
  bool ReplicateNow();

  /// Returns true if all net channels on this net object are napping (performing change detection on longer intervals), else false.
  bool IsNapping() const;

  /// (Only applies to non-emplaced net objects)
  /// Returns true if the net object is an ancestor (original network object archetype hierarchy root), else false.
  bool IsAncestor() const;
  /// (Only applies to non-emplaced net objects)
  /// Returns true if the net object is a descendant (original network object archetype hierarchy child), else false.
  bool IsDescendant() const;

  /// (Only applies to non-emplaced net objects)
  /// [Client/Server] Sets the family tree ID this net object belongs to (either as an ancestor or descendant).
  void SetFamilyTreeId(FamilyTreeId familyTreeId);
  /// (Only applies to non-emplaced net objects)
  /// [Client/Server] Returns the family tree ID this net object belongs to (either as an ancestor or descendant), else 0.
  FamilyTreeId GetFamilyTreeId() const;

  /// Returns true if the net object was created as part of a level initialization, else false.
  bool WasLevelInitialized() const;
  /// Returns true if the net object was created as part of a cog initialization (not a level initialization), else false.
  bool WasCogInitialized() const;

  /// Sets the initialization level resource ID name.
  void SetInitializationLevelResourceIdName(const String& initLevelResourceIdName);
  /// Returns the initialization level resource ID name (if created as part of a level initialization), else String().
  const String& GetInitializationLevelResourceIdName() const;

  /// Returns the operating net peer (which may or may not be this net object).
  NetPeer* GetNetPeer() const;
  /// Returns the operating net space (which may or may not be this net object).
  NetSpace* GetNetSpace() const;

  /// Returns true if the net object is a net peer, else false.
  bool IsNetPeer() const;
  /// Returns true if the net object is a net space, else false.
  bool IsNetSpace() const;
  /// Returns true if the net object is a net user, else false.
  bool IsNetUser() const;

  /// Returns true if the net object is invalid, else false.
  bool IsInvalid() const;
  /// Returns true if the net object is valid, else false.
  bool IsValid() const;
  /// Returns true if the net object is live, else false.
  bool IsLive() const;
  /// Returns true if the net object is online, else false.
  bool IsOnline() const;
  /// Returns the net object ID (set if the net object is live), else 0.
  NetObjectId GetNetObjectId() const;

  /// [Client] Forgets the online net object locally.
  /// [Server] Forgets the online net object locally and remotely for all relevant peers.
  /// Effectively removes the net object from the network system without destroying it.
  /// Returns true if successful, else false.
  bool Forget();

  /// [Client/Server] Selects the remote net object on the first opposite-role peer found running in another game session instance on the engine.
  /// Will fail if the net object is not online, or not found remotely.
  /// Returns true if successful, else false.
  bool SelectRemote();

  //
  // Channel Management
  //

  /// [Client/Server] Returns true if the specified net property already belongs to a net channel, else false.
  bool DoesThisNetPropertyAlreadyBelongToAChannel(Component* component, StringParam propertyName) const;

  /// [Client/Server] Returns true if the net object has the specified net channel, else false.
  bool HasNetChannel(const String& netChannelName) const;

  /// [Client/Server] Returns the specified net channel, else nullptr.
  NetChannel* GetNetChannel(const String& netChannelName);

  /// [Client/Server] Adds the net channel.
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  /// Returns the net channel if successful, else nullptr (a net channel of that name already exists).
  NetChannel* AddNetChannel(const String& netChannelName, NetChannelConfig* netChannelConfig = nullptr);

  /// [Client/Server] Adds the net property to the specified net channel (will be added if it does not already exist).
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  /// Returns true if successful, else false (a net property of that name already exists on the specified net channel).
  bool AddNetPropertyToChannel(Component* component, Property* property, const String& netPropertyTypeName, NetPropertyConfig* netPropertyConfig, const String& netChannelName, NetChannelConfig* netChannelConfig);
  bool AddNetPropertyToChannel(Component* component, Property* property, NetPropertyConfig* netPropertyConfig, NetChannelConfig* netChannelConfig);

  /// [Client/Server] Removes the specified net channel.
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  /// Returns true if successful, else false (a net channel of that name could not be found).
  bool RemoveNetChannel(const String& netChannelName);

  /// [Client/Server] Removes all net channels.
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  void ClearNetChannels();

  //
  // Ownership Interface
  //

  /// Returns true if the net object is conceptually owned by a user, else false.
  bool IsOwnedByAUser() const;
  /// Returns true if the net object is not conceptually owned by a user, else false.
  bool IsNotOwnedByAUser() const;

  /// Returns true if the net object is conceptually owned by the specified user, else false.
  bool IsOwnedByUser(Cog* cog) const;
  bool IsOwnedByUserId(NetUserId netUserId) const;

  /// Returns true if the net object is conceptually owned by a user added by the specified peer, else false.
  bool IsOwnedByPeer(NetPeerId netPeerId) const;

  /// Returns true if the net object is conceptually owned by a user added by our local peer, else false.
  bool IsMine() const;
  /// Returns true if the net object is not conceptually owned by a user added by our local peer, else false.
  bool IsNotMine() const;

  /// Returns true if our open peer's network role is client and the net object is conceptually owned by a user added by our local peer, else false.
  bool IsClientAndMine() const;
  /// Returns true if our open peer's network role is client and the net object is not conceptually owned by a user added by our local peer, else false.
  bool IsClientButNotMine() const;

  /// Returns true if our open peer's network role is server and the net object is conceptually owned by a user added by our local peer, else false.
  bool IsServerAndMine() const;
  /// Returns true if our open peer's network role is server and the net object is not conceptually owned by a user added by our local peer, else false.
  bool IsServerButNotMine() const;

  /// Returns true if our open peer's network role is offline and the net object is conceptually owned by a user added by our local peer, else false.
  bool IsOfflineAndMine() const;
  /// Returns true if our open peer's network role is offline and the net object is not conceptually owned by a user added by our local peer, else false.
  bool IsOfflineButNotMine() const;

  /// Returns the network peer identifier of the peer who added the user this object conceptually belongs to, else 0.
  NetPeerId GetNetUserOwnerPeerId() const;
  /// Returns the network user identifier of the user this object conceptually belongs to, else 0.
  NetUserId GetNetUserOwnerUserId() const;
  /// Returns the network user this object conceptually belongs to, else nullptr.
  Cog* GetNetUserOwner() const;

  /// [Server/Offline] Sets the owning network user on this object.
  void SetNetUserOwnerUserId(NetUserId netUserId);
  void SetNetUserOwner(Cog* cog);

  /// [Server/Offline] Sets the owning network user on this object and up the tree on each parent recursively (pre-order traversal).
  void SetNetUserOwnerUpById(NetUserId netUserId);
  void SetNetUserOwnerUp(Cog* cog);

  /// [Server/Offline] Sets the owning network user on this object and down the tree on all children recursively (pre-order traversal).
  void SetNetUserOwnerDownById(NetUserId netUserId);
  void SetNetUserOwnerDown(Cog* cog);

  /// Handles a change in network ownership, dispatches events accordingly.
  void HandleNetUserOwnerChanged(NetUserId previousNetUserOwnerUserId);

  //
  // Network Dispatch Interface
  //

  /// Dispatches the net event on the net object for the local peer.
  void DispatchLocal(StringParam eventId, Event* event);

  /// Dispatches the net event on the net object for the remote peer.
  /// In Offline mode, this calls DispatchLocal only.
  void DispatchRemote(StringParam eventId, Event* event, NetPeerId netPeerId);

  /// Dispatches the net event on the net object for all remote peers.
  /// In Offline mode, this calls DispatchLocal only.
  void DispatchBroadcast(StringParam eventId, Event* event);

  /// Dispatches the net event on the net object for the local peer and for the remote peer.
  /// In Offline mode, this calls DispatchLocal only.
  void DispatchLocalAndRemote(StringParam eventId, Event* event, NetPeerId netPeerId);

  /// Dispatches the net event on the net object for the local peer and for all remote peers.
  /// In Offline mode, this calls DispatchLocal only.
  void DispatchLocalAndBroadcast(StringParam eventId, Event* event);

  //
  // Channel Configuration
  //

  /// Sets the automatic net channel configuration resource (assigned to net properties unless another channel is specified).
  void SetAutomaticChannel(NetChannelConfig* netChannelConfig);
  /// Returns the automatic net channel configuration resource (assigned to net properties unless another channel is specified).
  NetChannelConfig* GetAutomaticChannel();

  //
  // Property Info
  //

  /// Returns true if the specified net property info has been added, else false.
  bool HasNetPropertyInfo(BoundType* componentType, StringParam propertyName);

  /// Returns the net property info if it has been added, else nullptr.
  NetPropertyInfo* GetNetPropertyInfo(BoundType* componentType, StringParam propertyName);

  /// Adds a net property info.
  /// Must specify a supported property by name defined in the given component.
  /// Returns the added net property info, else nullptr.
  NetPropertyInfo* AddNetPropertyInfo(BoundType* componentType, StringParam propertyName);

  /// Removes a net property info if it was added.
  void RemoveNetPropertyInfo(BoundType* componentType, StringParam propertyName);

  // Data
  String                              mInitLevelResourceIdName; ///< Initialization level resource ID name (if created as part of a level initialization).
  bool                                mIsAncestor;              ///< Is an ancestor? (Original network object archetype hierarchy root?).
  FamilyTreeId                        mFamilyTreeId;            ///< [Client/Server] Family tree ID this net object belongs to (either as an ancestor or descendant).
  bool                                mIsOnline;                ///< Is online? (Between the NetObjectOnline/Offline scope?).
  NetUserId                           mNetUserOwnerUserId;      ///< User ID of our net user owner.
  HandleOf<NetChannelConfig>          mAutomaticChannel;        ///< Automatic net channel configuration resource applied to net properties by default.
  NetPropertyInfoArray                mNetPropertyInfos;        ///< Net property infos added through the property grid.
};

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ================================
// NetObject Initialization Phases:
// ================================

// [#] = Phase occurs for all objects in the initialization group in order

// [1] C++ Initialize
// [2] C++ AllObjectsCreated
// [:] (All net channel properties guaranteed initialized in all cases except emplacement)
// [3] Script Initialize
// [4] Script AllObjectsInitialized Event
// [:] (Frame-delay if server or offline)
// [:] (All net channel properties guaranteed initialized in all cases)
// [5] NetChannelPropertyInitialized Event
// [6] NetObjectOnline Event

// ==================================
// NetObject Uninitialization Phases:
// ==================================

// [#] = Phase occurs for all objects in the uninitialization group in order

// [:] (All net channel properties guaranteed uninitialized in all cases)
// [1] NetChannelPropertyUninitialized Event
// [2] NetObjectOffline Event
// [3] C++ OnDestroy / Script Destroyed

// ============================
// Attachments and Detachments:
// ============================

// Attachments, Detachments, and Reorders are IGNORED when serializing replicas in their original creation hierarchy structure.
// This is because we use Family Trees which represent the original net object archetype hierarchy order.
// Family Trees are carefully updated to mark destroyed or forgotten net objects as absent. Absent net objects are destroyed during their C++ AllObjectsCreated phase.
// This means replication occurs indirectly, by replicating Family Trees according to which of their net objects are still present (not absent).
// Remotely, after deserializing replicas in their original creation hierarchy structure, Attachments, Detachments, and Reorders are then applied.
// Effectively, replication occurs indirectly by transmitting replicas as they were originally created and then updating them via their net properties immediately after the fact.
// Specifically, Attachments, Detachments, and Reorders occur when reading ReplicaData inside the C++ AllObjectsCreated phase.
// Because the C++ AllObjectsCreated phase is already in a predetermined order, this hierarchy shifting will not break our deserialization order.

// "External" Attachments (attaching other objects not a part of an internal archetype hierarchy) are performed once both the Parent and Child exist.
// Normally this is not a problem, except in the case of clients receiving a network game. It's very possible that parents and children are replicated out of order.
// Because of our use of Family Trees, it's not possible to replicate external attachments in their new hierarchy order. These relations unfortunately can become cyclic between Family Trees.
// So, in the case of external attachments, it is undefined whether or not a parent or it's child will be replicated first. (Internal attachments are never a problem)
// If the Parent is replicated first: (Instant)
// When the Child is replicated, the Child will attach to the Parent inside the Child's "C++ AllObjectsCreated" phase, while the Child is reading it's ReplicaData.
// If the Child is replicated first: (Delayed)
// When the Parent is replicated, the Child will attach to the Parent inside the Parent's "C++ AllObjectsCreated" phase, *after* the Parent reads it's ReplicaData as a special case.
// Finally, it is guaranteed that all external attachments (Instant or Delayed) will be correctly applied prior to the NetLevelStarted and NetGameStarted Events.

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

} // namespace Zero

