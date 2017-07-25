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
//                                   NetUser                                       //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetUser, builder, type)
{
  ZeroBindComponent();

  // Bind documentation
  ZeroBindDocumented();

  // Bind base class as interface
  ZeroBindInterface(NetObject);

  // Bind dependencies
  ZeroBindDependency(Cog);

  // Bind setup (can be added in the editor)
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Bind ownership interface
  ZilchBindCustomGetterProperty(AddedByMyPeer);
  ZilchBindMethod(AddedByPeer);
  ZilchBindMethod(FindOwnedNetObjectByNameInSpace);
  ZilchBindMethod(FindOwnedNetObjectByName);
  ZilchBindGetterProperty(OwnedNetObjects);
  ZilchBindGetterProperty(OwnedNetObjectCount);
  ZilchBindMethodProperty(ReleaseOwnedNetObjects);

  // Bind member properties
  ReflectionObject* netUserIdProperty = ZilchBindFieldGetterProperty(mNetUserId);
  ReflectionObject* netPeerIdProperty = ZilchBindFieldGetterProperty(mNetPeerId);
  /* METAREFACTOR - DevConfig
  if(!Z::gEngine->GetConfigCog()->has(DeveloperConfig)) // Not a developer?
  {
    // Hide from property grid
    netUserIdProperty->AddAttribute(PropertyAttributes::cHidden);
    netPeerIdProperty->AddAttribute(PropertyAttributes::cHidden);
  }*/
}

NetUser::NetUser()
  : NetObject(),
    mNetPeerId(0),
    mNetUserId(0),
    mOwnedNetObjects(),
    mRequestBundle(),
    mResponseBundle()
{
}

bool NetUser::operator ==(const NetUser& rhs) const
{
  return mNetUserId == rhs.mNetUserId;
}
bool NetUser::operator !=(const NetUser& rhs) const
{
  return mNetUserId != rhs.mNetUserId;
}
bool NetUser::operator  <(const NetUser& rhs) const
{
  return mNetUserId < rhs.mNetUserId;
}
bool NetUser::operator ==(const NetUserId& rhs) const
{
  return mNetUserId == rhs;
}
bool NetUser::operator !=(const NetUserId& rhs) const
{
  return mNetUserId != rhs;
}
bool NetUser::operator  <(const NetUserId& rhs) const
{
  return mNetUserId < rhs;
}

//
// Component Interface
//

void NetUser::Initialize(CogInitializer& initializer)
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Connect event handlers
  ConnectThisTo(owner, Events::RegisterCppNetProperties, OnRegisterCppNetProperties);

  // Use accurate timestamps by default
  SetAccurateTimestampOnOnline(true);
  SetAccurateTimestampOnChange(true);
  SetAccurateTimestampOnOffline(true);

  // Initialize as net object
  NetObject::Initialize(initializer);

  // Initialize event bundles
  mRequestBundle.SetGameSession(GetGameSession());
  mResponseBundle.SetGameSession(GetGameSession());
}
void NetUser::OnAllObjectsCreated(CogInitializer& initializer)
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // OnAllObjectsCreated as net object
  NetObject::OnAllObjectsCreated(initializer);

  // Add the net user to internal lists and allow net object ownership
  if(NetPeer* netPeer = GetNetPeer())
    netPeer->AddUserInternal(owner);
}

void NetUser::OnRegisterCppNetProperties(RegisterCppNetProperties* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Add 'built-in' net user channel
  NetChannel* netUserChannel = AddNetChannel("NetUser");
  if(!netUserChannel) // Unable?
  {
    DoNotifyError("Unable to Add NetUser C++ NetProperties", String::Format("Unable to add built-in 'NetUser' channel on the NetUser '%s'", owner->GetDescription().c_str()));
    return;
  }

  // Configure net user channel
  // (Note: Since we know these net properties never change, we don't bother detecting changes)
  netUserChannel->GetNetChannelType()->SetDetectOutgoingChanges(false);
  netUserChannel->GetNetChannelType()->SetAcceptIncomingChanges(false);
  netUserChannel->GetNetChannelType()->SetEventOnOutgoingPropertyChange(false);
  netUserChannel->GetNetChannelType()->SetEventOnIncomingPropertyChange(false);

  // Add network peer ID net property (replicates adding network peer ID changes)
  NetProperty* netPeerIdProperty = netUserChannel->AddBasicNetProperty("NetPeerId", mNetPeerId);
  if(!netPeerIdProperty) // Unable?
  {
    DoNotifyError("Unable to Add NetUser C++ NetProperties", String::Format("Unable to add built-in 'NetPeerId' NetProperty to the 'NetUser' channel on the NetUser '%s'", owner->GetDescription().c_str()));
    return;
  }

  // Add network user ID net property (replicates network user ID changes)
  NetProperty* netUserIdProperty = netUserChannel->AddBasicNetProperty("NetUserId", mNetUserId);
  if(!netUserIdProperty) // Unable?
  {
    DoNotifyError("Unable to Add NetUser C++ NetProperties", String::Format("Unable to add built-in 'NetUserId' NetProperty to the 'NetUser' channel on the NetUser '%s'", owner->GetDescription().c_str()));
    return;
  }
}

void NetUser::OnDestroy(uint flags)
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Not online?
  if(!IsOnline())
  {
    // (This can happen when a net user fails to come online properly)

    // Remove the net user from internal lists and disallow net object ownership
    if(NetPeer* netPeer = GetNetPeer())
      netPeer->RemoveUserInternal(owner);
  }

  // Uninitialize as net object
  NetObject::OnDestroy(flags);
}

//
// User Interface
//

bool NetUser::AddedByMyPeer() const
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return false;

  return AddedByPeer(netPeer->GetNetPeerId());
}
bool NetUser::AddedByPeer(NetPeerId netPeerId) const
{
  return netPeerId == mNetPeerId;
}

//
// Object Interface
//

const String& NetUser::GetNetObjectOnlineEventId() const
{
  return Events::NetUserOnline;
}
void NetUser::HandleNetObjectOnlinePreDispatch(NetObjectOnline* event)
{
  // (Nothing special to do here for net user)
}
const String& NetUser::GetNetObjectOfflineEventId() const
{
  return Events::NetUserOffline;
}
void NetUser::HandleNetObjectOfflinePostDispatch(NetObjectOffline* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Remove the net user from internal lists and disallow net object ownership
  if(NetPeer* netPeer = GetNetPeer())
    netPeer->RemoveUserInternal(owner);
}

//
// Ownership Interface
//

Cog* NetUser::FindOwnedNetObjectByNameInSpace(StringParam name, Space* space) const
{
  forRange(Cog* cog, GetOwnedNetObjects())
    if(cog->GetSpace() == space
    && cog->GetName() == name)
      return cog;

  return nullptr;
}
Cog* NetUser::FindOwnedNetObjectByName(StringParam name) const
{
  forRange(Cog* cog, GetOwnedNetObjects())
    if(cog->GetName() == name)
      return cog;

  return nullptr;
}
CogHashSetRange NetUser::GetOwnedNetObjects() const
{
  return mOwnedNetObjects.All();
}
uint NetUser::GetOwnedNetObjectCount() const
{
  return mOwnedNetObjects.Size();
}

void NetUser::ReleaseOwnedNetObjects()
{
  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Release ownership of all net objects owned by the user
    CogHashSetRange objects = GetOwnedNetObjects();
    forRange(Cog* cog, objects)
    {
      // Get cog
      if(!cog) // Unable?
        continue;

      // Get net object
      NetObject* netObject = cog->has(NetObject);
      if(!netObject) // Unable?
        continue;

      // Clear net user owner
      netObject->SetNetUserOwnerUserId(NetUserId(0));
    }
    mOwnedNetObjects.Clear();
  }
}

//---------------------------------------------------------------------------------//
//                                 NetUserRange                                    //
//---------------------------------------------------------------------------------//

NetUserRange::NetUserRange()
  : NetUserSet::range()
{
}
NetUserRange::NetUserRange(const NetUserSet::range& rhs)
  : NetUserSet::range(rhs)
{
}

//---------------------------------------------------------------------------------//
//                                PendingNetUser                                   //
//---------------------------------------------------------------------------------//

PendingNetUser::PendingNetUser()
  : mOurRequestBundle()
{
}
PendingNetUser::PendingNetUser(GameSession* gameSession)
  : mOurRequestBundle(gameSession)
{
}

} // namespace Zero
