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
//                                  NetSpace                                       //
//---------------------------------------------------------------------------------//

/// Network Space.
/// Manages the replication of a single space on the network.
class NetSpace : public NetObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetSpace();

  //
  // Component Interface
  //

  /// Initializes the component.
  void Initialize(CogInitializer& initializer) override;

  /// Called on engine update.
  void OnEngineUpdate(UpdateEvent* event);
  /// [Client] Called on engine update.
  void ClientOnEngineUpdate(UpdateEvent* event);
  /// [Server] Called on engine update.
  void ServerOnEngineUpdate(UpdateEvent* event);
  /// [Offline] Called on engine update.
  void OfflineOnEngineUpdate(UpdateEvent* event);

  /// Called on level started.
  void OnLevelStarted(GameEvent* event);

  //
  // Space Interface
  //

  /// Returns the number of net objects in this space (but not including the net space itself).
  uint GetNetObjectCount() const;
  /// Returns the number of net users in this space.
  uint GetNetUserCount() const;

  /// [Server/Offline] Dispatches the network level started event and clones the current level state to clients.
  void PerformNetLevelStarted(bool isLevelTransition);

  /// [Client] Performs any delayed attachments now that the delayed parent exists locally.
  /// (Only used during net game cloning, missing attachment parents are an error otherwise)
  void FulfillDelayedAttachments(Cog* delayedParentObject);

  /// [Client] Adds a delayed attachment, to be performed once the delayed parent exists locally.
  /// If a pre-existing delayed attachment is already specified for this child it will be overwritten.
  /// (Only used during net game cloning, missing attachment parents are an error otherwise)
  void AddDelayedAttachment(NetObjectId readyChild, NetObjectId delayedParent);
  /// [Client] Removes a delayed attachment.
  /// Returns true if a delayed attachment was removed, else false.
  bool RemoveDelayedAttachment(NetObjectId readyChild);

  /// [Client] Clears all delayed attachments.
  void ClearDelayedAttachments();

  //
  // Object Interface
  //

  /// Handles behavior when the net object is brought online, dispatches events accordingly.
  const String& GetNetObjectOnlineEventId() const override;
  void HandleNetObjectOnlinePreDispatch(NetObjectOnline* event) override;
  /// Handles behavior when the net object is taken offline, dispatches events accordingly.
  const String& GetNetObjectOfflineEventId() const override;
  void HandleNetObjectOfflinePostDispatch(NetObjectOffline* event) override;

  // Data
  Array<CogId>                                   mPendingNetObjects;      ///< [Server/Offline] Delayed net objects that need to be brought online.
  bool                                           mPendingNetLevelStarted; ///< Delayed net level started event.
  ArrayMap< NetObjectId, NetObjectId >           mReadyChildMap;          ///< Maps a ready child to a delayed parent.
  ArrayMap< NetObjectId, ArraySet<NetObjectId> > mDelayedParentMap;       ///< Maps a delayed parent to ready children.
};

} // namespace Zero
