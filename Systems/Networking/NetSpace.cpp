///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  NetSpace                                       //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetSpace, builder, type)
{
  ZeroBindComponent();

  // Bind documentation
  ZeroBindDocumented();

  // Bind base class as interface
  ZeroBindInterface(NetObject);

  // Bind dependencies
  ZeroBindDependency(Space);

  // Bind setup (can be added in the editor)
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Bind space interface
  ZilchBindGetterProperty(NetObjectCount);
  ZilchBindGetterProperty(NetUserCount);
}

NetSpace::NetSpace()
  : NetObject(),
    mPendingNetObjects(),
    mPendingNetLevelStarted(false),
    mReadyChildMap(),
    mDelayedParentMap()
{
}

//
// Component Interface
//

void NetSpace::Initialize(CogInitializer& initializer)
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Use accurate timestamps by default
  SetAccurateTimestampOnOnline(true);
  SetAccurateTimestampOnChange(true);
  SetAccurateTimestampOnOffline(true);

  // Initialize as net object
  NetObject::Initialize(initializer);

  // Connect event handlers
  ConnectThisTo(owner, Events::LevelStarted, OnLevelStarted);
}

void NetSpace::OnEngineUpdate(UpdateEvent* event)
{
  // Update net space based on our role
  switch(GetRole())
  {
  default:
  case Role::Unspecified:
    Assert(false);
  case Role::Client:
    return ClientOnEngineUpdate(event);
  case Role::Server:
    return ServerOnEngineUpdate(event);
  case Role::Offline:
    return OfflineOnEngineUpdate(event);
  }
}
void NetSpace::ClientOnEngineUpdate(UpdateEvent* event)
{
  // Has pending network level?
  if(mPendingNetLevelStarted)
  {
    // Get space
    Space* space = static_cast<Space*>(GetOwner());

    // Space has a current level?
    if(space->GetCurrentLevel())
    {
      // Handle network level started
      // (Our client peer has now fully loaded and synchronized the network level)
      GetNetPeer()->HandleNetLevelStarted(space);
    }
    mPendingNetLevelStarted = false;
  }
}
void NetSpace::ServerOnEngineUpdate(UpdateEvent* event)
{
  // Determine if this is a new net space
  bool isNewNetSpace = false;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // For all pending net objects that need to be brought online
  forRange(Cog* cog, mPendingNetObjects.All())
  {
    // Cog is no longer valid?
    if(!cog)
      continue; // Skip

    // Marked for deletion?
    if(cog->GetMarkedForDestruction())
      continue; // Skip

    // Get net object component
    NetObject* netObject = cog->has(NetObject);
    if(!netObject) // Unable?
      continue; // Skip

    // Was level initialized?
    if(netObject->WasLevelInitialized())
    {
      // Emplace net object by level now
      // (We clone later when replicating the entire net level state)
      if(!netPeer->EmplaceNetObjectBySpaceAndLevel(cog, cog->GetSpace(), netObject->GetInitializationLevelResourceIdName())) // Unable?
        continue; // Skip
    }
    // Was cog initialized?
    else
    {
      Assert(netObject->WasCogInitialized());

      // Net object not online yet?
      // (The net object's family tree hasn't been spawned yet?)
      if(!netObject->IsOnline())
      {
        // Get net object's family tree ID
        FamilyTreeId familyTreeId = netObject->GetFamilyTreeId();
        Assert(familyTreeId != 0);

        // Spawn net object's family tree now
        // (May very well include other net object's in our pending list)
        if(!netPeer->SpawnFamilyTree(familyTreeId, Route::All)) // Unable?
          continue; // Skip
      }
    }

    // Is a net space?
    if(netObject->IsNetSpace())
    {
      // (The net object must be ourself, the net space)
      Assert(cog == GetOwner() && cog == GetSpace());
      // (This case should only hit once for a given net space)
      Assert(!isNewNetSpace);

      // This is a new net space
      isNewNetSpace = true;
    }

    // (Should be online now)
    Assert(IsOnline());
  }
  mPendingNetObjects.Clear();

  // Has pending network level?
  if(mPendingNetLevelStarted)
  {
    // Get space
    Space* space = static_cast<Space*>(GetOwner());

    // Space has a current level?
    if(space->GetCurrentLevel())
    {
      // Perform network level started
      // (If this is a new net space, then this is not a level transition, otherwise it is)
      PerformNetLevelStarted(isNewNetSpace ? false : true);
    }
    mPendingNetLevelStarted = false;
  }

}
void NetSpace::OfflineOnEngineUpdate(UpdateEvent* event)
{
  // Determine if this is a new net space
  bool isNewNetSpace = false;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // For all pending net objects that need to be brought online
  forRange(Cog* cog, mPendingNetObjects.All())
  {
    // Cog is no longer valid?
    if(!cog)
      continue; // Skip

    // Marked for deletion?
    if(cog->GetMarkedForDestruction())
      continue; // Skip

    // Get net object component
    NetObject* netObject = cog->has(NetObject);
    if(!netObject) // Unable?
      continue; // Skip

    // Handle net object coming online
    netObject->HandleNetObjectOnline();

    // Is a net space?
    if(netObject->IsNetSpace())
    {
      // (The net object must be ourself, the net space)
      Assert(cog == GetOwner() && cog == GetSpace());
      // (This case should only hit once for a given net space)
      Assert(!isNewNetSpace);

      // This is a new net space
      isNewNetSpace = true;
    }

    // (Should be online now)
    Assert(IsOnline());
  }
  mPendingNetObjects.Clear();

  // Has pending network level?
  if(mPendingNetLevelStarted)
  {
    // Get space
    Space* space = static_cast<Space*>(GetOwner());

    // Space has a current level?
    if(space->GetCurrentLevel())
    {
      // Perform network level started
      // (If this is a new net space, then this is not a level transition, otherwise it is)
      PerformNetLevelStarted(isNewNetSpace ? false : true);
    }
    mPendingNetLevelStarted = false;
  }
}

void NetSpace::OnLevelStarted(GameEvent* event)
{
  // Is not server or offline?
  // (Note: We can't put this check in our initialize, around connecting to the LevelStarted event, because emplaced net spaces
  //  don't have an open peer at initialize time, but we still need to connect to this event to handle future level transitions)
  if(!IsServerOrOffline())
    return;

  // Level started on our space?
  // (Checking this only to filter out forwarded LevelStarted events)
  if(event->mSpace == GetSpace())
  {
    // Handle net level started next engine update
    mPendingNetLevelStarted = true;
  }
}

//
// Space Interface
//

uint NetSpace::GetNetObjectCount() const
{
  uint netObjectCount = 0;

  // For all cogs in this space
  forRange(Cog& cog, const_cast<NetSpace*>(this)->GetSpace()->AllObjects())
    if(cog.has(NetObject)) // Has net object component?
      ++netObjectCount;

  return netObjectCount;
}
uint NetSpace::GetNetUserCount() const
{
  uint netUserCount = 0;

  // For all cogs in this space
  forRange(Cog& cog, const_cast<NetSpace*>(this)->GetSpace()->AllObjects())
    if(cog.has(NetUser)) // Has net object component?
      ++netUserCount;

  return netUserCount;
}

void NetSpace::PerformNetLevelStarted(bool isLevelTransition)
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // (Should be server or offline)
  Assert(IsServerOrOffline());

  // Get space
  Space* space = static_cast<Space*>(GetOwner());

  // Handle network level started
  // (Our server or offline peer has now fully loaded and synchronized the network level)
  netPeer->HandleNetLevelStarted(space);

  // Is server?
  if(IsServer())
  {
    // Clone level state to clients
    netPeer->CloneNetLevel(space, isLevelTransition);
  }
}

void NetSpace::FulfillDelayedAttachments(Cog* delayedParentObject)
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // (We should be receiving a net game clone)
  Assert(netPeer->IsReceivingNetGame());

  // Get delayed parent net object ID (may be invalid)
  NetObjectId delayedParent = delayedParentObject->has(NetObject)->GetNetObjectId();

  // Get all delayed attachments waiting on this parent object (if any)
  ArrayMap< NetObjectId, ArraySet<NetObjectId> >::pointer result = mDelayedParentMap.FindPairPointer(delayedParent);
  if(!result) // Unable?
  {
    // Done (No delayed attachments to fulfill)
    return;
  }

  // For all ready children waiting on this delayed parent
  forRange(NetObjectId readyChild, result->second.All())
  {
    // Get ready child object
    Cog* readyChildObject = netPeer->GetNetObject(readyChild);
    if(!readyChildObject) // Unable?
    {
      // (We clean up ready children on destruction from our delayed attachments list, so this shouldn't fail)
      Assert(false);
      continue;
    }

    // Attach ready child object to delayed parent object
    readyChildObject->AttachTo(delayedParentObject);

    // Remove from ready child map
    mReadyChildMap.EraseValue(readyChild);
  }

  // Erase delayed attachment entries (we've fulfilled all delayed attachments waiting on this parent object)
  mDelayedParentMap.Erase(result);
}

void NetSpace::AddDelayedAttachment(NetObjectId readyChild, NetObjectId delayedParent)
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // (We should be receiving a net game clone)
  Assert(netPeer->IsReceivingNetGame());

  // Clear previous delayed attachment specified for this ready child (if any)
  bool removedPreviousDelayedAttachment = RemoveDelayedAttachment(readyChild);
  if(removedPreviousDelayedAttachment)
  {
    // Get ready child object
    if(Cog* readyChildObject = netPeer->GetNetObject(readyChild))
    {
      // METAREFACTOR GetDisplayName should be cleaned up and put on a CogComponentMeta
      //DoNotifyWarning("Delayed Attachment Unfulfilled",
      //                String::Format("A delayed attachment for ready child NetObject '%s' was not fulfilled - A newer delayed parent overwrote the previous one",
      //                GetDisplayName(readyChildObject).c_str()));
    }
  }

  // Add to ready child map
  ArrayMap< NetObjectId, NetObjectId >::pointer_bool_pair result1 = mReadyChildMap.InsertOrAssign(readyChild, delayedParent);
  Assert(result1.second); // (Insertion should have occurred)

  // Add to ready child set in delayed parent map
  ArraySet<NetObjectId>& readyChildren = mDelayedParentMap.FindOrInsert(delayedParent);
  ArraySet<NetObjectId>::pointer_bool_pair result2 = readyChildren.InsertOrAssign(readyChild);
  Assert(result2.second); // (Insertion should have occurred)
}
bool NetSpace::RemoveDelayedAttachment(NetObjectId readyChild)
{
  // A delayed attachment exists for this ready child?
  if(ArrayMap< NetObjectId, NetObjectId >::pointer result1 = mReadyChildMap.FindPairPointer(readyChild))
  {
    // Remove from ready child set in delayed parent map
    NetObjectId delayedParent = result1->second;
    ArrayMap< NetObjectId, ArraySet<NetObjectId> >::pointer result2 = mDelayedParentMap.FindPairPointer(delayedParent);
    if(!result2 || result2->second.Empty()) // Unable?
    {
      // (This shouldn't happen, we don't allow a mismatch to occur between these corresponding maps)
      Assert(false);
      return false;
    }
    result2->second.EraseValue(readyChild);

    // Ready child set in delayed parent map is now empty?
    if(result2->second.Empty())
    {
      mDelayedParentMap.Erase(result2);
    }

    // Remove from ready child map
    mReadyChildMap.Erase(result1);
    return true;
  }
  return false;
}

void NetSpace::ClearDelayedAttachments()
{
  mReadyChildMap.Clear();
  mDelayedParentMap.Clear();
}

//
// Object Interface
//

const String& NetSpace::GetNetObjectOnlineEventId() const
{
  return Events::NetSpaceOnline;
}
void NetSpace::HandleNetObjectOnlinePreDispatch(NetObjectOnline* event)
{
  // // (These net objects didn't have their create context set during initialization because that requires an online net space.
  // // So we set their create context now instead. Even though we don't need to, since these net objects are going to be emplaced,
  // // for the sake of consistency and having this information available we're going to set their create context here anyway)
  // 
  // // (Our net object ID should have been set by now)
  // Assert(GetNetObjectId() != 0);
  // 
  // // For all objects in the space
  // forRange(Cog& cog, GetSpace()->AllObjects())
  // {
  //   // Doesn't have net object component?
  //   NetObject* netObject = cog.has(NetObject);
  //   if(!netObject)
  //     continue; // Skip
  // 
  //   // Marked for deletion?
  //   if(cog.MarkedForDeletion())
  //     continue; // Skip
  // 
  //   // (Should not have a create context yet)
  //   Assert(netObject->GetCreateContext().IsEmpty());
  // 
  //   // Set create context (space net object ID)
  //   netObject->InitializeCreateContext();
  // }
}
const String& NetSpace::GetNetObjectOfflineEventId() const
{
  return Events::NetSpaceOffline;
}
void NetSpace::HandleNetObjectOfflinePostDispatch(NetObjectOffline* event)
{
  // (Nothing special to do here for net space)
}

} // namespace Zero
