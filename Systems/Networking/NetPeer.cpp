///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Because windows...
#undef SetPort

/// Basic net host information size (must fit within a single packet).
static const Bytes BasicNetHostInfoMaxSize = 480;

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                   NetPeer                                       //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeer, builder, type)
{
  ZeroBindComponent();

  // Bind documentation
  ZeroBindDocumented();

  // Bind base class as interface
  ZeroBindInterface(NetObject);

  // Bind dependencies
  ZeroBindDependency(GameSession);

  // Bind setup (can be added in the editor)
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Bind host interface
  ZilchBindGetterSetterProperty(LanDiscoverable);
  ZilchBindGetterSetterProperty(InternetDiscoverable);
  //ZilchBindGetterSetterProperty(FacilitateInternetConnections);
  ZilchBindGetterSetterProperty(HostPortRangeStart);
  ZilchBindGetterSetterProperty(HostPortRangeEnd);
  ZilchBindGetterSetterProperty(HostPingInterval);
  ZilchBindGetterSetterProperty(InternetHostPublishInterval);
  ZilchBindGetterSetterProperty(InternetHostRecordLifetime);
  ZilchBindGetterSetterProperty(InternetSameIpHostRecordLimit);
  ZilchBindMethod(SubscribeToMasterServer);
  ZilchBindMethod(UnsubscribeFromMasterServer);
  ZilchBindMethod(GetHostByIndex);
  ZilchBindOverloadedMethod(GetHostByAddress, ZilchConstInstanceOverload(NetHost*, Network::Enum, const IpAddress&));
  ZilchBindOverloadedMethod(GetHostByAddress, ZilchConstInstanceOverload(NetHost*, const IpAddress&));
  ZilchBindMethod(GetHostList);
  ZilchBindMethod(DiscoverHostList);
  ZilchBindMethod(ClearHostList);
  ZilchBindMethod(ClearHostLists);
  ZilchBindMethod(RefreshHost);
  ZilchBindMethod(RefreshHostList);
  ZilchBindMethod(CancelHostRequests);

  // Bind peer interface
  //ZilchBindGetterProperty(Guid)->AddAttribute(cHiddenAttribute);
  ZilchBindCustomGetterProperty(IsOpen);
  ZilchBindOverloadedMethod(Open, ZilchInstanceOverload(bool, Role::Enum, uint, uint));
  ZilchBindOverloadedMethod(Open, ZilchInstanceOverload(bool, Role::Enum, uint));
  ZilchBindOverloadedMethod(Open, ZilchInstanceOverload(bool, Role::Enum));
  ZilchBindOverloadedMethod(OpenClient, ZilchInstanceOverload(bool, uint, uint));
  ZilchBindOverloadedMethod(OpenClient, ZilchInstanceOverload(bool, uint));
  ZilchBindOverloadedMethod(OpenClient, ZilchInstanceOverload(bool));
  ZilchBindOverloadedMethod(OpenServer, ZilchInstanceOverload(bool, uint));
  ZilchBindOverloadedMethod(OpenServer, ZilchInstanceOverload(bool));
  //ZilchBindOverloadedMethod(OpenMasterServer, ZilchInstanceOverload(bool, uint));
  //ZilchBindOverloadedMethod(OpenMasterServer, ZilchInstanceOverload(bool));
  ZilchBindOverloadedMethod(OpenOffline, ZilchInstanceOverload(bool));
  ZilchBindMethod(Close);
  ZilchBindGetterProperty(NetPeerId);
  ZilchBindGetterProperty(Ipv4Address);
  ZilchBindGetterProperty(Ipv4Host);
  ZilchBindGetterProperty(Ipv4Port);
  ZilchBindGetterProperty(Ipv6Address);
  ZilchBindGetterProperty(Ipv6Host);
  ZilchBindGetterProperty(Ipv6Port);
  //ZilchBindGetterProperty(CreationDuration)->AddAttribute(cHiddenAttribute);
  ZilchBindGetterProperty(NetObjectCount);
  ZilchBindGetterProperty(NetUserCount);
  ZilchBindGetterProperty(NetSpaceCount);
  ZilchBindGetterSetterProperty(FrameFillWarning);
  ZilchBindGetterSetterProperty(FrameFillSkip);

  // Bind link interface
  ZilchBindGetterProperty(LinkCount);
  ZilchBindOverloadedMethod(ConnectLink, ZilchInstanceOverload(bool, const IpAddress&, EventBundle*));
  ZilchBindOverloadedMethod(ConnectLink, ZilchInstanceOverload(bool, const IpAddress&, Event*));
  ZilchBindOverloadedMethod(ConnectLink, ZilchInstanceOverload(bool, const IpAddress&));
  ZilchBindOverloadedMethod(DisconnectLink, ZilchInstanceOverload(bool, const IpAddress&, EventBundle*));
  ZilchBindOverloadedMethod(DisconnectLink, ZilchInstanceOverload(bool, const IpAddress&, Event*));
  ZilchBindOverloadedMethod(DisconnectLink, ZilchInstanceOverload(bool, const IpAddress&));
  ZilchBindOverloadedMethod(DisconnectLink, ZilchInstanceOverload(bool, NetPeerId, EventBundle*));
  ZilchBindOverloadedMethod(DisconnectLink, ZilchInstanceOverload(bool, NetPeerId, Event*));
  ZilchBindOverloadedMethod(DisconnectLink, ZilchInstanceOverload(bool, NetPeerId));
  ZilchBindOverloadedMethod(DisconnectAllLinks, ZilchInstanceOverload(uint, EventBundle*));
  ZilchBindOverloadedMethod(DisconnectAllLinks, ZilchInstanceOverload(uint, Event*));
  ZilchBindOverloadedMethod(DisconnectAllLinks, ZilchInstanceOverload(uint));
  // ZilchBindMethod(GetLinkCreationDuration);
  ZilchBindMethod(GetLinkCreationDirection);
  ZilchBindMethod(GetLinkStatus);
  ZilchBindMethod(GetLinkState);
  // ZilchBindMethod(GetLinkStateDuration);
  // ZilchBindMethod(GetLinkGuid);
  ZilchBindMethod(GetLinkIpAddress);
  ZilchBindMethod(GetOurIpAddressFromLink);
  ZilchBindMethod(GetLinkInternetProtocol);
  ZilchBindMethod(GetLinkNetPeerId);

  // Bind user interface
  ZilchBindOverloadedMethod(AddUser, ZilchInstanceOverload(bool, EventBundle*));
  ZilchBindOverloadedMethod(AddUser, ZilchInstanceOverload(bool, Event*));
  ZilchBindOverloadedMethod(AddUser, ZilchInstanceOverload(bool));
  ZilchBindMethod(GetUser);
  ZilchBindGetterProperty(UsersAddedByMyPeer);
  ZilchBindMethod(GetUsersAddedByPeer);
  ZilchBindGetterProperty(Users);
  ZilchBindGetterProperty(UserCount);
  // ZilchBindOverloadedMethod(RemoveUser, ZilchInstanceOverload(bool, NetUserId, EventBundle*));
  // ZilchBindOverloadedMethod(RemoveUser, ZilchInstanceOverload(bool, NetUserId, Event*));
  // ZilchBindOverloadedMethod(RemoveUser, ZilchInstanceOverload(bool, NetUserId));
  ZilchBindOverloadedMethod(RemoveUser, ZilchInstanceOverload(bool, Cog*, EventBundle*));
  ZilchBindOverloadedMethod(RemoveUser, ZilchInstanceOverload(bool, Cog*, Event*));
  ZilchBindOverloadedMethod(RemoveUser, ZilchInstanceOverload(bool, Cog*));

  ZilchBindGetterSetterProperty(InternetHostListTimeout);
  ZilchBindGetterSetterProperty(BasicHostInfoTimeout);
  ZilchBindGetterSetterProperty(ExtraHostInfoTimeout);

  // Bind replication interface
  ZilchBindMethod(GetNetSpace);
  ZilchBindMethod(GetNetObject);

  // Bind network events
  BindNetEvents(builder, type);
}

NetPeer::NetPeer()
  : NetObject(),
    Peer(ProcessReceivedCustomPacket, ProcessReceivedCustomMessage),
    Replicator(),
    mIsOpenOffline(false),
    mPendingUserRequests(),
    mAddedUsers(),
    mOurAddedUsers(),
    mTheirAddedUsers(),
    mNetUserIdStore(),
    mActiveUserAddRequest(nullptr),
    mIsReceivingNetGame(false),
    mPendingNetGameStarted(false),
    mPendingRequests(),
    mWaitingOnConnectResponse(false),
    mFamilyTrees(),
    mFamilyTreeIdStore(),
    mActiveReplicaStream(nullptr),
    mActiveCogInitializer(nullptr),
    mLanDiscoverable(false),
    mInternetDiscoverable(false),
    mFacilitateInternetConnections(false),
    mHostPortRangeStart(0),
    mHostPortRangeEnd(0),
    mInternetHostPublishInterval(0),
    mInternetHostListTimeout(0.0f),
    mBasicHostInfoTimeout(0.0f),
    mExtraHostInfoTimeout(0.0f),
    mNextManagerId(1),
    mInternetHostRecordLifetime(0),
    mInternetSameIpHostRecordLimit(0),
    mMasterServerSubscriptions(),
    mHostLists(),
    mPublishElapsedTime(0),
    mPingManager(this),
    mLanHostDiscovery(this),
    mInternetHostDiscovery(this)
{
  //Warning: Newing stuff in constructor!
  //Set the callback in the ping manager so we can handle receiving pings.
  mPingManager.SetPingCallback( CreateCallback( &NetPeer::HandlePing, this) );

  ResetConfig();
}

void NetPeer::ResetConfig()
{
  // Host settings
  SetLanDiscoverable();
  SetInternetDiscoverable();
  SetFacilitateInternetConnections();
  SetHostPortRangeStart();
  SetHostPortRangeEnd();
  SetHostPingInterval();
  SetInternetHostPublishInterval();
  SetInternetHostRecordLifetime();
  SetInternetSameIpHostRecordLimit();

  // Peer settings
  SetFrameFillWarning();
  SetFrameFillSkip();

  // Timeout settings
  SetInternetHostListTimeout();
  SetBasicHostInfoTimeout();
  SetExtraHostInfoTimeout();
}

Guid NetPeer::GetOurProjectGuid()
{
  // Get project settings

  ProjectSettings* projectSettings = Z::gEngine->GetProjectSettings();
  if(!projectSettings) // Unable?
    return 0;

  // Get project GUID
  return projectSettings->GetProjectGuid();
}

//
// Component Interface
//

void NetPeer::Serialize(Serializer& stream)
{
  // Serialize as net object
  NetObject::Serialize(stream);
  
  // Serialize host settings
  SerializeNameDefault(mLanDiscoverable, GetLanDiscoverable());
  SerializeNameDefault(mInternetDiscoverable, GetInternetDiscoverable());
  //SerializeNameDefault(mFacilitateInternetConnections, GetFacilitateInternetConnections());
  SerializeNameDefault(mHostPortRangeStart, GetHostPortRangeStart());
  SerializeNameDefault(mHostPortRangeEnd, GetHostPortRangeEnd());
  SerializeNameDefault(mHostPingInterval, GetHostPingInterval());
  SerializeNameDefault(mInternetHostPublishInterval, GetInternetHostPublishInterval());
  SerializeNameDefault(mInternetHostRecordLifetime, GetInternetHostRecordLifetime());
  SerializeNameDefault(mInternetSameIpHostRecordLimit, GetInternetSameIpHostRecordLimit());

  // Serialize peer settings
  SerializeNameDefault(mFrameFillWarning, GetFrameFillWarning());
  SerializeNameDefault(mFrameFillSkip, GetFrameFillSkip());

  //Serialize peer timeouts
  SerializeNameDefault(mInternetHostListTimeout, GetInternetHostListTimeout() );
  SerializeNameDefault(mBasicHostInfoTimeout, GetBasicHostInfoTimeout());
  SerializeNameDefault(mExtraHostInfoTimeout, GetExtraHostInfoTimeout());

}
void NetPeer::Initialize(CogInitializer& initializer)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

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
  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnEngineUpdate);

  //Used by master server to send host list data.
  ConnectThisTo(owner, Events::NetPeerSentConnectResponse, OnNetPeerSentConnectResponse);
  //Used by Client/Server to know when master server has responded, failed.
  ConnectThisTo(owner, Events::NetPeerReceivedConnectResponse, OnNetPeerReceivedConnectResponse);

  //Clients Ignore these events when communicating with master server.
  ConnectThisTo(owner, Events::NetPeerSentConnectRequest, OnNetPeerSentConnectRequest);
  ConnectThisTo(owner, Events::NetLinkConnected, OnNetLinkConnected);
  ConnectThisTo(owner, Events::NetLinkDisconnected, OnNetLinkDisconnected);

  //Connect events in LAN and internet host discovery.
  mLanHostDiscovery.Initialize();
  mInternetHostDiscovery.Initialize();
}

void NetPeer::OnDestroy(uint flags)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Uninitialize as net object
  NetObject::OnDestroy(flags);

  // Close peer
  Close();
}

void NetPeer::OnEngineUpdate(UpdateEvent* event)
{
  ProfileScopeTree("Networking", "Engine", Color::DeepSkyBlue);

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Not open or is open offline?
  if(!IsOpen() || mIsOpenOffline)
    return;

  // Handle pending host ping requests as needed
  //HandlePendingHostPings(); //OLD WAY: TODO

  //NEW WAY: 
  mLanHostDiscovery.Update(event);
  mInternetHostDiscovery.Update(event);

  //TODO: REMOVE PING MANAGER FROM NETPEER IF NO ONE USES IT. (MASTER SERVER USE IT?)
  mPingManager.UpdatePendingPings();
  
  // Handle pending network requests as needed (users requests)
  HandlePendingRequests();

  //
  // Update NetSpaces
  //

  // For All spaces
  SpaceMap::valueRange spaces = owner->GetAllSpaces();
  forRange(Space* space, spaces)
  {
    // Marked for deletion?
    if(space->GetMarkedForDestruction())
      continue; // Skip

    // Get net space component
    NetSpace* netSpace = space->has(NetSpace);
    if(!netSpace) // Unable? (Not a net space?)
      continue; // Skip

    // Update net space
    netSpace->OnEngineUpdate(event);
  }

  // Has pending net game started event?
  if(mPendingNetGameStarted)
  {
    Assert(IsClient());

    // Handle network game started
    // (Our client peer has now fully joined the network game)
    HandleNetGameStarted();

    mPendingNetGameStarted = false;
  }

  //
  // Update Peer
  //
  {
    ProfileScopeTree("NetPeer", "Networking", Color::OrangeRed);

    // Update Peer
    if(!Peer::Update()) // Error?
    {
      DoNotifyError("NetPeer", "Error updating NetPeer");
      return;
    }
  }

  // Publish server records
  UpdatePublishInterval(event);

  // Master server update host records
  UpdateHostRecords(event);
}

//
// Host Interface
//

void NetPeer::SetLanDiscoverable(bool lanDiscoverable)
{
  mLanDiscoverable = lanDiscoverable;
}
bool NetPeer::GetLanDiscoverable() const
{
  return mLanDiscoverable;
}

void NetPeer::SetInternetDiscoverable(bool internetDiscoverable)
{
  mInternetDiscoverable = internetDiscoverable;
}
bool NetPeer::GetInternetDiscoverable() const
{
  return mInternetDiscoverable;
}

void NetPeer::SetFacilitateInternetConnections(bool facilitateInternetConnections)
{
  mFacilitateInternetConnections = facilitateInternetConnections;
}
bool NetPeer::GetFacilitateInternetConnections() const
{
  return mFacilitateInternetConnections;
}

void NetPeer::SetHostPortRangeStart(uint hostPortRangeStart)
{
  mHostPortRangeStart = Math::Clamp(hostPortRangeStart, uint(0), uint(65535));

  // Range start is greater than end?
  if(mHostPortRangeStart > mHostPortRangeEnd)
    SetHostPortRangeEnd(mHostPortRangeStart); // Set end to the same port
}
uint NetPeer::GetHostPortRangeStart() const
{
  return mHostPortRangeStart;
}

void NetPeer::SetHostPortRangeEnd(uint hostPortRangeEnd)
{
  mHostPortRangeEnd = Math::Clamp(hostPortRangeEnd, uint(0), uint(65535));

  // Range end is less than start?
  if(mHostPortRangeEnd < mHostPortRangeStart)
    SetHostPortRangeStart(mHostPortRangeEnd); // Set start to the same port
}
uint NetPeer::GetHostPortRangeEnd() const
{
  return mHostPortRangeEnd;
}

void NetPeer::SetHostPingInterval(float hostPingInterval)
{
  mHostPingInterval = hostPingInterval;
  mPingManager.mPingInterval = hostPingInterval;
  mLanHostDiscovery.mPingManager.mPingInterval = hostPingInterval;
  mInternetHostDiscovery.mPingManager.mPingInterval = hostPingInterval;
}
float NetPeer::GetHostPingInterval() const
{
  return mHostPingInterval;
}

void NetPeer::SetInternetHostPublishInterval(float internetHostPublishInterval)
{
  mInternetHostPublishInterval = internetHostPublishInterval;
}
float NetPeer::GetInternetHostPublishInterval() const
{
  return mInternetHostPublishInterval;
}

void NetPeer::SetInternetHostRecordLifetime(float internetHostRecordLifetime)
{
  mInternetHostRecordLifetime = internetHostRecordLifetime;
}
float NetPeer::GetInternetHostRecordLifetime() const
{
  return mInternetHostRecordLifetime;
}

void NetPeer::SetInternetSameIpHostRecordLimit(uint internetSameIpHostRecordLimit)
{
  mInternetSameIpHostRecordLimit = internetSameIpHostRecordLimit;
}
uint NetPeer::GetInternetSameIpHostRecordLimit() const
{
  return mInternetSameIpHostRecordLimit;
}

void NetPeer::SubscribeToMasterServer(const IpAddress& ipAddress)
{
  // Add master server subscription (allowing duplicates)
  mMasterServerSubscriptions.PushBack(ipAddress);
}
void NetPeer::UnsubscribeFromMasterServer(const IpAddress& ipAddress)
{
  // Remove master server subscription
  mMasterServerSubscriptions.EraseValue(ipAddress);
}

NetHost* NetPeer::GetHostByIndex(Network::Enum network, uint index) const
{
  // Get network's host list
  NetHostSet* hostList = mHostLists.FindPointer(network);
  if(!hostList) // Unable?
    return nullptr;

  // Out of bounds?
  if(index >= hostList->Size())
    return nullptr;

  // Get host by index
  NetHostPtr* host = &hostList->Data()[index];
  return *host;
}
NetHost* NetPeer::GetHostByAddress(Network::Enum network, const IpAddress& ipAddress) const
{
  // Get network's host list
  NetHostSet* hostList = mHostLists.FindPointer(network);
  if(!hostList) // Unable?
    return nullptr;

  // Get host by IP address
  NetHostPtr* host = hostList->FindPointer(ipAddress);
  if(host)
  {
    return *host;
  }
  else
  {
    return nullptr;
  }
}
NetHost* NetPeer::GetHostByAddress(const IpAddress& ipAddress) const
{
  // For All network host lists
  forRange(NetHostSet& hostList, mHostLists.Values())
  {
    // Get host by IP address
    if(NetHostPtr* host = hostList.FindPointer(ipAddress))
      return *host;
  }

  // Failure
  return nullptr;
}

NetHost* NetPeer::AddOrFindHost(Network::Enum network, const IpAddress& ipAddress)
{
  // Get or create network's host list
  NetHostSet& hostList = mHostLists.FindOrInsert(network);

  // Find or add the specified host
  NetHostPtr& host = hostList.FindOrInsert(NetHostPtr(new NetHost(ipAddress)));
  return host;
}

bool NetPeer::RemoveHost(Network::Enum network, const IpAddress& ipAddress)
{
  // Get network's host list
  NetHostSet* hostList = mHostLists.FindPointer(network);
  if(!hostList) // Unable?
    return false;

  // Remove the specified host (if found)
  NetHostSet::pointer_bool_pair result = hostList->EraseValue(ipAddress);
  return result.second;
}
bool NetPeer::RemoveHost(const IpAddress& ipAddress)
{
  // For All network host lists
  forRange(NetHostSet& hostList, mHostLists.Values())
  {
    // Remove the specified host (if found)
    if(hostList.EraseValue(ipAddress).second)
      return true;
  }

  // Failure
  return false;
}

NetHostRange NetPeer::GetHostList(Network::Enum network) const
{
  // Get network's host list
  NetHostSet* hostList = mHostLists.FindPointer(network);
  if(!hostList) // Unable?
    return NetHostRange();

  // Get host list range
  return hostList->All();
}

bool NetPeer::DiscoverHostList(Network::Enum network, bool removeStaleHosts)
{
  // Not open?
  if(!IsOpen())
  {
    // Assume peer should be opened as a client
    if(!OpenClient()) // Unable?
    {
      DoNotifyWarning("Unable to discover host list", "NetPeer could not be opened as a client");
      return false;
    }
  }

  return RefreshHostList(network, false, true, removeStaleHosts);
}

void NetPeer::ClearHostList(Network::Enum network)
{
  // Get network's host list
  NetHostSet* hostList = mHostLists.FindPointer(network);
  if(!hostList) // Unable?
    return;

  // Clear host list
  hostList->Clear();
}
void NetPeer::ClearHostLists()
{
  // For All network host lists
  forRange(NetHostSet& hostList, mHostLists.Values())
  {
    // Clear host list
    hostList.Clear();
  }
}

bool NetPeer::RefreshHost(Network::Enum network, const IpAddress& ipAddress, bool getExtraHostInfo, bool allowDiscovery, bool removeStaleHosts)
{
  // Not open?
  if(!IsOpen())
  {
    // Assume peer should be opened as a client
    if(!OpenClient()) // Unable?
    {
      DoNotifyWarning("Unable to refresh host", "NetPeer could not be opened as a client");
      return false;
    }
  }

  if(network == Network::LAN)
    mLanHostDiscovery.SingleHostRefresh(ipAddress, allowDiscovery, getExtraHostInfo, removeStaleHosts);
  else
    mInternetHostDiscovery.SingleHostRefresh(ipAddress, allowDiscovery, getExtraHostInfo, removeStaleHosts);

  return true;
}


bool NetPeer::RefreshHostList(Network::Enum network, bool getExtraHostInfo, bool allowDiscovery, bool removeStaleHosts)
{
  // Not open?
  if(!IsOpen())
  {
    // Assume peer should be opened as a client
    if(!OpenClient()) // Unable?
    {
      DoNotifyWarning("Unable to refresh host list", "NetPeer could not be opened as a client");
      return false;
    }
  }

  if(network == Network::LAN)
    mLanHostDiscovery.RefreshAll(allowDiscovery, getExtraHostInfo, removeStaleHosts);
  else
    mInternetHostDiscovery.RefreshAll(allowDiscovery, getExtraHostInfo, removeStaleHosts);

  //TODO: maybe refresh all should return success or failure so that users may be aware of it inability to send messages.
  return true;
}

void NetPeer::CancelHostRequests()
{
  mLanHostDiscovery.CancelRefreshes();
  mInternetHostDiscovery.CancelRefreshes();
}

//
// Peer Interface
//

Guid NetPeer::GetGuid() const
{
  return Peer::GetGuid();
}

bool NetPeer::IsOpen() const
{
  return mIsOpenOffline
      || (Peer::IsOpen() && Replicator::IsInitialized() );
}

bool NetPeer::Open(Role::Enum role, uint port, uint retries)
{
  // Reset net peer state
  Close();

  // Unspecified role?
  if(role == Role::Unspecified)
  {
    DoNotifyWarning("Error Opening NetPeer",
                    String::Format("Unable to open %s NetPeer socket on port %u with %u retries - Must specify a valid role",
                    Role::Names[role], port, retries));

    // Failure
    return false;
  }
  // Offline role?
  else if(role == Role::Offline)
  {
    // Open peer in offline mode
    mIsOpenOffline = true;

    // Success
    return true;
  }
  else if(role == Role::MasterServer)
  {
    mIsOpenMasterServer = true;
    role = Role::Server; //Pretend to be a server are far as the replicator is concerned.
  }

  // Set specified role
  Replicator::SetRole(role);

  // Add replicator peer plugin
  Assert(!Replicator::IsInitialized());
  if(!Peer::AddPlugin(this, "Replicator")) // Unable?
  {
    DoNotifyWarning("Error Opening NetPeer",
                    String::Format("Unable to open %s NetPeer socket on port %u with %u retries - There was an error initializing the replicator",
                    Role::Names[role], port, retries));

    // Clean up and return failure
    Close();
    return false;
  }

  // Open peer, attempting up to the specified number of retries on incrementing ports
  for(uint i = 0; i <= retries; ++i)
  {
    Status status;
    Peer::Open(status, (port + i), InternetProtocol::V4);
    if(status.Succeeded()) // Successful?
      break;
  }
  if(!Peer::IsOpen()) //Unable?
  {
    DoNotifyWarning("Error Opening NetPeer",
                    String::Format("Unable to open %s NetPeer socket on port %u with %u retries - The port may already be in use by another socket",
                    Role::Names[role], port, retries));

    // Clean up and return failure
    Close();
    return false;
  }

  
  Assert(Replicator::IsInitialized());

  // Handle net peer opened
  if(!HandleNetPeerOpened()) // Unable?
  {
    DoNotifyWarning("Error Opening NetPeer",
                    String::Format("Unable to open %s NetPeer socket on port %u with %u retries - There was an error emplacing existing NetObjects",
                    Role::Names[role], port, retries));

    // Clean up and return failure
    Close();
    return false;
  }

  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Handle network game started
    // (Our server or offline peer is now hosting a network game)
    HandleNetGameStarted();
  }

  // Success
  return true;
}
bool NetPeer::Open(Role::Enum role, uint port)
{
  return Open(role, port, 0);
}
bool NetPeer::Open(Role::Enum role)
{
  return Open(role, 0, 0);
}
bool NetPeer::OpenClient(uint port, uint retries)
{
  return Open(Role::Client, port, retries);
}
bool NetPeer::OpenClient(uint port)
{
  return Open(Role::Client, port);
}
bool NetPeer::OpenClient()
{
  return Open(Role::Client);
}
bool NetPeer::OpenServer(uint port)
{
  return Open(Role::Server, port);
}
bool NetPeer::OpenServer()
{
  return Open(Role::Server, GetHostPortRangeStart(), GetHostPortRangeEnd() - GetHostPortRangeStart());
}
bool NetPeer::OpenMasterServer(uint port)
{
  return Open(Role::MasterServer, port);
}
bool NetPeer::OpenMasterServer()
{
  return Open(Role::MasterServer);
}
bool NetPeer::OpenOffline()
{
  return Open(Role::Offline);
}

void NetPeer::Close()
{
  //
  // Peer Scope Clean-up:
  //


  // Is opened?
  if(IsOpen())
  {
    // Cancel host requests if we are open.
    CancelHostRequests();
    // Clear host data
    ClearHostLists();

    // Handle net peer closed
    HandleNetPeerClosed();
  }

  // Close peer and delete plugins
  Peer::Close();

  // Clear replicator data
  mIsOpenOffline = false;
  mIsOpenMasterServer = false;

  // Clear user data
  mPendingUserRequests.Clear();
  mAddedUsers.Clear();
  mOurAddedUsers.Clear();
  mTheirAddedUsers.Clear();
  mNetUserIdStore.Reset();
  mActiveUserAddRequest = nullptr;

  // Clear connection data
  mIsReceivingNetGame = false;
  mPendingNetGameStarted = false;
  mPendingRequests.Clear();
  mWaitingOnConnectResponse = false;

  // Clear family tree data
  mFamilyTrees.Clear();
  mFamilyTreeIdStore.Reset();

  // Clear active replica stream
  mActiveReplicaStream = nullptr;
  mActiveCogInitializer = nullptr;

  // Clear Master Server data
  mIpAddressServerCounts.Clear();
  mProjectHostRecordMaps.Clear();
  mHostRecords.Clear();
  mReceiptRecipients.Clear(); // I assume peer links will be auto closed.
  
  // Clear Server Data
  mPublishElapsedTime = mInternetHostPublishInterval; //So that a new ping is sent when it is reopened.


  //TODO: Make sure that netpeer closes host discovery and Ping Manager.

}

bool NetPeer::HandleNetPeerOpened()
{
  // (NetPeer should be considered open by this point)
  Assert(IsOpen());

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Get All spaces in game session
  SpaceMap::valueRange spaces = owner->GetAllSpaces();

  //
  // Dispatch NetPeerOpened
  //

  // Dispatch event
  NetPeerOpened event;
  owner->DispatchEvent(Events::NetPeerOpened, &event);

  //
  // Emplace Network GameSession
  //

  // Emplace game session object
  // only clients and servers emplace net objects
  if(IsClientOrServer()) 
  {
    if(!EmplaceNetObjectByGameSetup(owner)) // Unable?
      return false;
  

    //
    // Emplace Network Spaces
    //

    // For All spaces
    forRange(Space* space, spaces)
    {
      // Doesn't have net space component?
      if(!space->has(NetSpace))
        continue; // Skip

      // Marked for deletion?
      if(space->GetMarkedForDestruction())
        continue; // Skip

      // Emplace space object
      if(!EmplaceNetObjectByGameSetup(space)) // Unable?
        return false;
    }

    //
    // Emplace Network Objects
    //

    // For All spaces
    forRange(Space* space, spaces)
    {
      // Doesn't have net space component?
      if(!space->has(NetSpace))
        continue; // Skip

      // Marked for deletion?
      if(space->GetMarkedForDestruction())
        continue; // Skip

      // For All objects in the space
      forRange(Cog& cog, space->AllObjects())
      {
        // Doesn't have net object component?
        if(!cog.has(NetObject))
          continue; // Skip

        // Marked for deletion?
        if(cog.GetMarkedForDestruction())
          continue; // Skip

        // Emplace net object
        if(!EmplaceNetObjectByGameSetup(&cog)) // Unable?
          return false;
      }
    }

  }

  // Success
  return true;
}
bool NetPeer::HandleNetPeerClosed()
{
  // (NetPeer should still be considered open by this point)
  Assert(IsOpen());

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Get All spaces in game session
  SpaceMap::valueRange spaces = owner->GetAllSpaces();

  // Determine if this was caused by the game quitting
  bool causedByGameQuitting = owner->mQuiting;

  // Not caused by the game quitting?
  if(!causedByGameQuitting)
  {
    //
    // Forget Network Objects
    //

    // For All spaces
    forRange(Space* space, spaces)
    {
      // Doesn't have net space component?
      if(!space->has(NetSpace))
        continue; // Skip

      // Marked for deletion?
      if(space->GetMarkedForDestruction())
        continue; // Skip

      // For All objects in the space
      forRange(Cog& cog, space->AllObjects())
      {
        // Doesn't have net object component?
        if(!cog.has(NetObject))
          continue; // Skip

        // Marked for deletion?
        if(cog.GetMarkedForDestruction())
          continue; // Skip

        // Forget net object
        if(!ForgetNetObject(&cog, Route::None)) // Unable?
          return false;
      }
    }

    //
    // Forget Network Spaces
    //

    // For All spaces
    forRange(Space* space, spaces)
    {
      // Doesn't have net space component?
      if(!space->has(NetSpace))
        continue; // Skip

      // Marked for deletion?
      if(space->GetMarkedForDestruction())
        continue; // Skip

      // Forget space object
      if(!ForgetNetObject(space, Route::None)) // Unable?
        return false;
    }

    //
    // Forget Network GameSession
    //

    // Not marked for deletion?
    if(!owner->GetMarkedForDestruction())
    {
      // Forget game session object
      if(!ForgetNetObject(owner, Route::None)) // Unable?
        return false;
    }

    // (All net objects should have been forgotten by now)
    Assert(Replicator::GetReplicaCount() == 0);
  }

  //
  // Dispatch NetPeerClosed
  //

  // Dispatch event
  NetPeerClosed event;
  owner->DispatchEvent(Events::NetPeerClosed, &event);

  // Success
  return true;
}

Role::Enum NetPeer::GetRole() const
{
  if (mIsOpenMasterServer)
  {
    return Role::MasterServer;
  }
  else if (mIsOpenOffline)
  {
    return Role::Offline;
  }
  else
  {
    return Replicator::GetRole();
  }
}

NetPeerId NetPeer::GetNetPeerId() const
{
  return Replicator::GetReplicatorId().value();
}

const IpAddress& NetPeer::GetIpv4Address() const
{
  return Peer::GetLocalIpv4Address();
}
String NetPeer::GetIpv4Host() const
{
  return GetIpv4Address().GetHost();
}
uint NetPeer::GetIpv4Port() const
{
  return GetIpv4Address().GetPort();
}

const IpAddress& NetPeer::GetIpv6Address() const
{
  return Peer::GetLocalIpv6Address();
}
String NetPeer::GetIpv6Host() const
{
  return GetIpv6Address().GetHost();
}
uint NetPeer::GetIpv6Port() const
{
  return GetIpv6Address().GetPort();
}

TimeMs NetPeer::GetCreationDuration() const
{
  return Peer::GetLocalTime();
}

uint NetPeer::GetNetObjectCount() const
{
  uint netObjectCount = 0;

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // For All spaces
  SpaceMap::valueRange spaces = owner->GetAllSpaces();
  forRange(Space* space, spaces)
  {
    // Get net space component
    NetSpace* netSpace = space->has(NetSpace);
    if(!netSpace) // Unable?
      continue;

    // Add net object count from space
    netObjectCount += netSpace->GetNetObjectCount();

    // Plus 1 for the net space
    ++netObjectCount;
  }

  return netObjectCount;
}
uint NetPeer::GetNetUserCount() const
{
  uint netUserCount = 0;

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // For All spaces
  SpaceMap::valueRange spaces = owner->GetAllSpaces();
  forRange(Space* space, spaces)
  {
    // Get net space component
    NetSpace* netSpace = space->has(NetSpace);
    if(!netSpace) // Unable?
      continue;

    // Add net user count from space
    netUserCount += netSpace->GetNetUserCount();
  }

  return netUserCount;
}
uint NetPeer::GetNetSpaceCount() const
{
  uint netSpaceCount = 0;

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // For All spaces
  SpaceMap::valueRange spaces = owner->GetAllSpaces();
  forRange(Space* space, spaces)
  {
    // Get net space component
    NetSpace* netSpace = space->has(NetSpace);
    if(!netSpace) // Unable?
      continue;

    // Plus 1 for the net space
    ++netSpaceCount;
  }

  return netSpaceCount;
}

void NetPeer::SetFrameFillWarning(float frameFillWarning)
{
  Replicator::SetFrameFillWarning(frameFillWarning);
}
float NetPeer::GetFrameFillWarning() const
{
  return Replicator::GetFrameFillWarning();
}

void NetPeer::SetFrameFillSkip(float frameFillSkip)
{
  Replicator::SetFrameFillSkip(frameFillSkip);
}
float NetPeer::GetFrameFillSkip() const
{
  return Replicator::GetFrameFillSkip();
}

//
// Link Interface
//

bool NetPeer::HasLink(const IpAddress& ipAddress) const
{
  return Replicator::HasLink(ipAddress);
}
bool NetPeer::HasLink(NetPeerId netPeerId) const
{
  return Replicator::HasLink(netPeerId);
}
bool NetPeer::HasLinks(const Route& route) const
{
  return Replicator::HasLinks(route);
}
bool NetPeer::HasLinks() const
{
  return Replicator::HasLinks();
}

PeerLink* NetPeer::GetLink(const IpAddress& ipAddress) const
{
  return Replicator::GetLink(ipAddress);
}
PeerLink* NetPeer::GetLink(NetPeerId netPeerId) const
{
  return Replicator::GetLink(netPeerId);
}
PeerLinkSet NetPeer::GetLinks(const Route& route) const
{
  return Replicator::GetLinks(route);
}
PeerLinkSet NetPeer::GetLinks() const
{
  return Replicator::GetLinks();
}

uint NetPeer::GetLinkCount() const
{
  // TODO: Change this to use Replicator instead of Peer
  return Peer::GetLinkCount();
}

bool NetPeer::ConnectLink(const IpAddress& ipAddress, EventBundle* requestBundle)
{
  // Not open?
  if(!IsOpen())
  {
    // Assume peer should be opened as a client
    if(!OpenClient()) // Unable?
    {
      DoNotifyWarning("Unable to connect link", "NetPeer could not be opened as a client");
      return false;
    }
  }

  // Not a client?
  if(!IsClient())
  {
    DoNotifyWarning("Unable to connect link", "NetPeer is not open as a client");
    return false;
  }

  //    Waiting on a connect response?
  // OR Already connected/attempting-connection on that link?
  if(mWaitingOnConnectResponse
  || GetLink(ipAddress))
  {
    DoNotifyWarning("Unable to connect link", "NetPeer is already connected, attempting connection, or about to attempt connection on that link");
    return false;
  }

  // Add pending connect request (will be handled next engine update)
  mPendingRequests.PushBack(NetRequest(NetRequestType::Connect, ipAddress, *requestBundle));
  mWaitingOnConnectResponse = true; // Wait for a connect response before allowing another connect request
  return true;
}
bool NetPeer::ConnectLink(const IpAddress& ipAddress, Event* requestEvent)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()), requestEvent);
  return ConnectLink(ipAddress, &requestBundle);
}
bool NetPeer::ConnectLink(const IpAddress& ipAddress)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()));
  return ConnectLink(ipAddress, &requestBundle);
}

bool NetPeer::DisconnectLink(const IpAddress& ipAddress, EventBundle* requestBundle)
{
  // Get network link
  PeerLink* link = GetLink(ipAddress);
  if(!link) // Unable?
    return false;

  // Attempt to initiate disconnect notice
  // (The replicator plugin will destroy the link once the disconnect grace period ends)
  return link->Disconnect(requestBundle->GetBitStream());
}
bool NetPeer::DisconnectLink(const IpAddress& ipAddress, Event* requestEvent)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()), requestEvent);
  return DisconnectLink(ipAddress, &requestBundle);
}
bool NetPeer::DisconnectLink(const IpAddress& ipAddress)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()));
  return DisconnectLink(ipAddress, &requestBundle);
}
bool NetPeer::DisconnectLink(NetPeerId netPeerId, EventBundle* requestBundle)
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return false;

  // Attempt to initiate disconnect notice
  // (The replicator plugin will destroy the link once the disconnect grace period ends)
  return link->Disconnect(requestBundle->GetBitStream());
}
bool NetPeer::DisconnectLink(NetPeerId netPeerId, Event* requestEvent)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()), requestEvent);
  return DisconnectLink(netPeerId, &requestBundle);
}
bool NetPeer::DisconnectLink(NetPeerId netPeerId)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()));
  return DisconnectLink(netPeerId, &requestBundle);
}

uint NetPeer::DisconnectAllLinks(EventBundle* requestBundle)
{
  uint result = 0;

  // For All links
  PeerLinkSet peerLinks = Peer::GetLinks();
  forRange(PeerLink* link, peerLinks.All())
  {
    // Attempt to initiate disconnect notice
    // (The replicator plugin will destroy the link once the disconnect grace period ends)
    if(link->Disconnect(requestBundle->GetBitStream()))
      ++result;
  }

  return result;
}
uint NetPeer::DisconnectAllLinks(Event* requestEvent)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()), requestEvent);
  return DisconnectAllLinks(&requestBundle);
}
uint NetPeer::DisconnectAllLinks()
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()));
  return DisconnectAllLinks(&requestBundle);
}

TimeMs NetPeer::GetLinkCreationDuration(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return 0;

  // Success
  return link->GetCreationDuration();
}
TransmissionDirection::Enum NetPeer::GetLinkCreationDirection(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return TransmissionDirection::Unspecified;

  // Success
  return link->GetCreationDirection();
}

LinkStatus::Enum NetPeer::GetLinkStatus(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return LinkStatus::Unspecified;

  // Success
  return link->GetStatus();
}
LinkState::Enum NetPeer::GetLinkState(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return LinkState::Unspecified;

  // Success
  return link->GetState();
}
TimeMs NetPeer::GetLinkStateDuration(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return 0;

  // Success
  return link->GetStateDuration();
}

Guid NetPeer::GetLinkGuid(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return 0;

  // Success
  return link->GetTheirGuid();
}

IpAddress NetPeer::GetLinkIpAddress(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return IpAddress();

  // Success
  return link->GetTheirIpAddress();
}

IpAddress NetPeer::GetOurIpAddressFromLink(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return IpAddress();

  // Success
  return link->GetOurIpAddress();
}

InternetProtocol::Enum NetPeer::GetLinkInternetProtocol(NetPeerId netPeerId) const
{
  // Get network link
  PeerLink* link = GetLink(netPeerId);
  if(!link) // Unable?
    return InternetProtocol::Unspecified;

  // Success
  return link->GetInternetProtocol();
}

NetPeerId NetPeer::GetLinkNetPeerId(const IpAddress& ipAddress) const
{
  // Get network link
  PeerLink* link = GetLink(ipAddress);
  if(!link) // Unable?
    return 0;

  // Success
  return link->GetPlugin<ReplicatorLink>("ReplicatorLink")->GetReplicatorId().value();
}

//
// User Interface
//

bool NetPeer::AddUser(EventBundle* requestBundle)
{
  // Not open?
  if(!IsOpen())
    return false;

  // Add pending user add request (will be handled next engine update)
  mPendingRequests.PushBack(NetRequest(NetRequestType::AddUser, IpAddress(), *requestBundle));
  return true;
}
bool NetPeer::AddUser(Event* requestEvent)
{
  // Add user
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()), requestEvent);
  return AddUser(&requestBundle);
}
bool NetPeer::AddUser()
{
  // Add user
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()));
  return AddUser(&requestBundle);
}

Cog* NetPeer::GetUser(NetUserId netUserId) const
{
  return mAddedUsers.FindValue(netUserId, nullptr);
}

NetUserRange NetPeer::GetUsersAddedByMyPeer() const
{
  // Return our added users
  return mOurAddedUsers.All();
}
NetUserRange NetPeer::GetUsersAddedByPeer(NetPeerId netPeerId) const
{
  // Is our peer?
  if(netPeerId == GetNetPeerId())
  {
    // Return our added users
    return mOurAddedUsers.All();
  }
  // Is another peer?
  else
  {
    // Get their added users
    NetUserSet* theirAddedUsers = mTheirAddedUsers.FindPointer(netPeerId);
    if(!theirAddedUsers) // Unable?
      return NetUserRange();

    // Return their added users
    return theirAddedUsers->All();
  }
}

NetUserRange NetPeer::GetUsers() const
{
  return mAddedUsers.All();
}
uint NetPeer::GetUserCount() const
{
  return mAddedUsers.Size();
}

bool NetPeer::RemoveUser(NetUserId netUserId, EventBundle* requestBundle)
{
  // Not open?
  if(!IsOpen())
    return false;

  // Initiate user remove request
  return SendUserRemoveRequest(netUserId, requestBundle);
}
bool NetPeer::RemoveUser(NetUserId netUserId, Event* requestEvent)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()), requestEvent);
  return RemoveUser(netUserId, &requestBundle);
}
bool NetPeer::RemoveUser(NetUserId netUserId)
{
  EventBundle requestBundle(static_cast<GameSession*>(GetOwner()));
  return RemoveUser(netUserId, &requestBundle);
}
bool NetPeer::RemoveUser(Cog* cog, EventBundle* requestBundle)
{
  // Get net user
  NetUser* netUser = cog->has(NetUser);
  if(!netUser) // Unable?
  {
    DoNotifyWarning("NetPeer", "Unable to remove user - Cog must have a NetUser component");
    return false;
  }

  // Remove user
  return RemoveUser(netUser->mNetUserId, requestBundle);
}
bool NetPeer::RemoveUser(Cog* cog, Event* requestEvent)
{
  // Get net user
  NetUser* netUser = cog->has(NetUser);
  if(!netUser) // Unable?
  {
    DoNotifyWarning("NetPeer", "Unable to remove user - Cog must have a NetUser component");
    return false;
  }

  // Remove user
  return RemoveUser(netUser->mNetUserId, requestEvent);
}
bool NetPeer::RemoveUser(Cog* cog)
{
  // Get net user
  NetUser* netUser = cog->has(NetUser);
  if(!netUser) // Unable?
  {
    DoNotifyWarning("NetPeer", "Unable to remove user - Cog must have a NetUser component");
    return false;
  }

  // Remove user
  return RemoveUser(netUser->mNetUserId);
}

void NetPeer::AddUserInternal(Cog* cog)
{
  // Get net user
  NetUser* netUser = cog->has(NetUser);
  Assert(netUser);

  //
  // Assign NetUserId
  //

  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Not created as part of a user add request handshake?
    if(!mActiveUserAddRequest)
    {
      DoNotifyWarning("Illegal NetUser Emplacement Attempted",
                        String::Format("A NetUser '%s' was illegally emplaced (not spawned as part of a user add request handshake)."
                        " NetUser emplacement is currently unsupported."
                        " Try requesting a NetUser by calling AddUser on your NetPeer instead.",
                        cog->GetDescription().c_str()));

      // Remove this net object from it's family tree (if it hasn't been removed already)
      RemoveNetObjectFromFamilyTree(this);

      // Mark object for destruction and return
      cog->Destroy();
      return;
    }

    // Assign net user ID
    if(!AssignNetUserId(netUser)) // Unable?
    {
      DoNotifyWarning("Error Adding NetUser",
                        String::Format("Unable to add NetUser '%s' - Error assigning a net user ID",
                        cog->GetDescription().c_str()));

      // Remove this net object from it's family tree (if it hasn't been removed already)
      RemoveNetObjectFromFamilyTree(this);

      // Mark object for destruction and return
      cog->Destroy();
      return;
    }
    Assert(netUser->mNetUserId);

    // Set their net peer as the adding net peer of the net user
    netUser->mNetPeerId = mActiveUserAddRequest->mTheirNetPeerId;

    // Store request bundle for later
    netUser->mRequestBundle = mActiveUserAddRequest->mTheirRequestBundle;
  }

  //
  // Add User To Lists
  //

  // Add user to added users set
  NetUserSet::pointer_bool_pair result = mAddedUsers.Insert(cog->mObjectId);
  Assert(result.second); // (Insertion should have succeeded)

  // Added by our local peer?
  if(netUser->mNetPeerId == GetNetPeerId())
  {
    // Add user to our added users set
    NetUserSet::pointer_bool_pair result = mOurAddedUsers.Insert(cog->mObjectId);
    Assert(result.second); // (Insertion should have succeeded)
  }
  // Added by a remote peer?
  else
  {
    // Add user to their added users set
    NetUserSet::pointer_bool_pair result = mTheirAddedUsers.FindOrInsert(netUser->mNetPeerId).Insert(cog->mObjectId);
    Assert(result.second); // (Insertion should have succeeded)
  }

  //
  // Take Ownership (Of Self And Children)
  //

  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Set net user's object and it's children to be owned by itself
    netUser->SetNetUserOwnerDownById(netUser->mNetUserId);
  }
}
void NetPeer::RemoveUserInternal(Cog* cog)
{
  // Get net user
  NetUser* netUser = cog->has(NetUser);
  Assert(netUser);

  //
  // Release Ownership (Of All Owned Objects)
  //

  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Release ownership of any remaining objects owned by the user
    netUser->ReleaseOwnedNetObjects();
  }

  //
  // Remove User From Lists
  //

  // Remove user from added users set
  NetUserSet::pointer_bool_pair result = mAddedUsers.EraseValue(cog->mObjectId);
  // Assert(result.second); // (Erase should have succeeded)

  // Added by our local peer?
  if(netUser->mNetPeerId == GetNetPeerId())
  {
    // Remove user from our added users set
    NetUserSet::pointer_bool_pair result = mOurAddedUsers.EraseValue(cog->mObjectId);
    // Assert(result.second); // (Erase should have succeeded)
  }
  // Added by a remote peer?
  else
  {
    // Remove user from their added users set
    NetUserSet::pointer_bool_pair result = mTheirAddedUsers.FindOrInsert(netUser->mNetPeerId).EraseValue(cog->mObjectId);
    // Assert(result.second); // (Erase should have succeeded)
  }

  //
  // Release NetUserId
  //

  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Release net user ID
    ReleaseNetUserId(netUser);
  }
}

void NetPeer::HandlePendingRequests()
{
  // Perform All pending net requests
  for(uint i = 0; i < mPendingRequests.Size(); )
  {
    NetRequest& netRequest = mPendingRequests[i];

    // Handle pending request by type
    switch(netRequest.mNetRequestType)
    {
    case NetRequestType::Connect:
      {
        // Count user add requests following this connect request, preceding any following connect requests
        uint addUserRequestCount = 0;
        {
          // For All following requests
          for(uint j = i + 1; j < mPendingRequests.Size(); ++j)
            if(mPendingRequests[j].mNetRequestType == NetRequestType::AddUser)
              ++addUserRequestCount;
            else if(mPendingRequests[j].mNetRequestType == NetRequestType::Connect)
              break;
        }

        // Attempt to create link to remote peer
        PeerLink* link = Peer::CreateLink(netRequest.mTheirIpAddress);
        if(!link) // Unable?
        {
          // Get link if it already exists
          link = Peer::GetLink(netRequest.mTheirIpAddress);
          if(!link) // Unable?
            continue; // Advance
        }

        // Create network connect request data
        NetConnectRequestData netConnectRequestData;
        netConnectRequestData.mAddUserRequestCount = addUserRequestCount;
        netConnectRequestData.mEventBundleData     = ZeroMove(netRequest.mOurRequestBundle.GetBitStream());

        // Write network connect request data
        BitStream extraData;
        extraData.Write(netConnectRequestData);

        // Attempt to initiate connect request
        link->Connect(extraData);

        // Erase and advance
        mPendingRequests.EraseAt(i);
        continue;
      }
      break;

    case NetRequestType::AddUser:
      {
        // Is client?
        if(IsClient())
        {
          // Waiting on a connect response?
          if(mWaitingOnConnectResponse)
            return;

          // Server link not connected?
          PeerLink* link = Replicator::GetLink(0);
          if(!link || link->GetStatus() != LinkStatus::Connected)
            return;
        }

        // Initiate user add request
        SendUserAddRequest(&netRequest.mOurRequestBundle);

        // Erase and advance
        mPendingRequests.EraseAt(i);
        continue;
      }
      break;

    case NetRequestType::Unspecified:
    default:
      Assert(false);
      break;
    }

    // Advance
    ++i;
  }
}

//
// User Add Handshake
//

bool NetPeer::SendUserAddRequest(EventBundle* ourRequestBundle)
{
  Assert(IsOpen());

  // Create network user add request message
  Message netUserAddRequestMessage(NetPeerMessageType::NetUserAddRequest);

  NetUserAddRequestData netUserAddRequestData;
  netUserAddRequestData.mEventBundleData = ourRequestBundle->GetBitStream();

  netUserAddRequestMessage.GetData().Write(netUserAddRequestData);

  // Determine if we should send or pass-through this message
  // (We want to pass-through if we are the server or offline since we are also the receiver)
  bool passThrough = IsClient() ? false : true;

  // Pass-through message?
  if(passThrough)
  {
    Assert(IsServerOrOffline());

    // Handle "sent" network user add request
    HandleSentUserAddRequest(0, IpAddress(), ourRequestBundle);

    // Receive network user add request directly
    if(!ReceiveUserAddRequest(0, IpAddress(), netUserAddRequestMessage)) // Unable?
      Assert(false);

    // Success
    return true;
  }
  // Send message?
  else
  {
    Assert(IsClient());

    // Get server link
    PeerLink* link = Replicator::GetLink(0);
    if(!link) // Unable?
    {
      DoNotifyError("Unable to send user add request", "NetPeer client is not connected to a server");
      return false;
    }

    // Send network user add request
    Status status;
    link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netUserAddRequestMessage);
    if(status.Failed()) // Unable?
    {
      Warn("Unable to send user add request - Error sending message (%s)", status.Message.c_str());
      return false;
    }

    // Handle sent network user add request
    HandleSentUserAddRequest(0, link->GetTheirIpAddress(), ourRequestBundle);

    // Success
    return true;
  }
}
void NetPeer::HandleSentUserAddRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* ourRequestBundle)
{
  // Is client?
  if(IsClient())
  {
    // Create outgoing pending net user
    mPendingUserRequests.PushBack(PendingNetUser(static_cast<GameSession*>(GetOwner())));
    PendingNetUser& ourPendingNetUser = mPendingUserRequests.Back();

    // Store request bundle for later
    ourPendingNetUser.mOurRequestBundle = *ourRequestBundle;
  }

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetPeerSentUserAddRequest event(owner);
  event.mTheirNetPeerId   = theirNetPeerId;
  event.mTheirIpAddress   = theirIpAddress;
  event.mOurRequestBundle = *ourRequestBundle;

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerSentUserAddRequest, &event);
}

bool NetPeer::ReceiveUserAddRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message)
{
  Assert(IsOpen());
  Assert(IsServerOrOffline());

  // Read network user add request message
  NetUserAddRequestData netUserAddRequestData;
  if(!message.GetData().Read(netUserAddRequestData)) // Unable?
  {
    Assert(false);
    return false;
  }
  EventBundle theirRequestBundle(static_cast<GameSession*>(GetOwner()));
  theirRequestBundle = ZeroMove(netUserAddRequestData.mEventBundleData);

  // Handle received network user add request
  EventBundle ourResponseBundle(static_cast<GameSession*>(GetOwner()));
  NetUser* theirNetUser = HandleReceivedUserAddRequest(theirNetPeerId, theirIpAddress, &theirRequestBundle, &ourResponseBundle, mNetUserIdStore.GetNextId());

  // Send user add response
  if(!SendUserAddResponse(theirNetPeerId, theirNetUser, &theirRequestBundle, &ourResponseBundle)) // Unable?
    Assert(false);

  // Success
  return true;
}
NetUser* NetPeer::HandleReceivedUserAddRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* theirRequestBundle,
                                               EventBundle* returnOurResponseBundle, NetUserId theirNetUserId)
{
  Assert(IsOpen());
  Assert(IsServerOrOffline());

  // Determine our add response
  bool ourAddResponse = false;

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetPeerReceivedUserAddRequest event(owner);
  event.mTheirNetPeerId       = theirNetPeerId;
  event.mTheirIpAddress       = theirIpAddress;
  event.mTheirRequestBundle   = *theirRequestBundle;
  event.mReturnOurAddResponse = true;           // Optionally set by the event receiver
  event.mReturnOurResponseBundle;               // Optionally set by the event receiver
  event.mReturnTheirNetUser;                    // Optionally set by the event receiver
  event.mTheirNetUserId       = theirNetUserId; // (Released back to the store if not accepted)

  // Set active net user add request (used temporarily during net user creation)
  mActiveUserAddRequest = &event;

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerReceivedUserAddRequest, &event);

  // Clear active net user add request
  mActiveUserAddRequest = nullptr;

  // Get our add response as provided by the event handler
  ourAddResponse = event.mReturnOurAddResponse;
  (*returnOurResponseBundle) = ZeroMove(event.mReturnOurResponseBundle.GetBitStream());

  // User request accepted?
  if(ourAddResponse)
  {
    // Get their net user object as provided by the event handler
    Cog* cog = event.mReturnTheirNetUser;
    if(!cog) // Unable?
    {
      DoNotifyWarning("Unable to accept NetUser", "Their NetUser object was not provided by the NetPeerReceivedUserAddRequest event handler");

      // User add request denied
      return nullptr;
    }

    // Get net user
    NetUser* netUser = cog->has(NetUser);
    if(!netUser) // Unable?
    {
      DoNotifyWarning("Unable to accept NetUser", "Cog does not have NetUser Component");

      // User add request denied
      return nullptr;
    }

    // Marked for deletion?
    if(cog->GetMarkedForDestruction())
    {
      // User add request denied
      return nullptr;
    }

    // User add request accepted
    return netUser;
  }
  // User request denied?
  else
  {
    // Get their net user object as provided by the event handler
    Cog* cog = event.mReturnTheirNetUser;

    // Created as part of a denied user add request?
    if(cog)
    {
      DoNotifyWarning("Illegal NetUser Creation Attempted",
                        String::Format("A NetUser '%s' was illegally created while denying a user add request."
                        " A NetUser may only be created when accepting a user add request."
                        " Try accepting the user add request and then creating the NetUser instead.",
                        cog->GetDescription().c_str()));

      // Remove this net object from it's family tree (if it hasn't been removed already)
      RemoveNetObjectFromFamilyTree(this);

      // Mark object for destruction and return
      cog->Destroy();

      // User add request denied
      return nullptr;
    }
  }

  // User add request denied
  return nullptr;
}

bool NetPeer::SendUserAddResponse(NetPeerId theirNetPeerId, NetUser* theirNetUser, EventBundle* theirRequestBundle, EventBundle* ourResponseBundle)
{
  Assert(IsOpen());
  Assert(IsServerOrOffline());

  // Create network user add response message
  Message netUserAddResponseMessage(NetPeerMessageType::NetUserAddResponse);

  NetUserAddResponseData netUserAddResponseData;
  netUserAddResponseData.mAddResponse     = theirNetUser ? NetUserAddResponse::Accept : NetUserAddResponse::Deny;
  netUserAddResponseData.mNetUserId       = theirNetUser ? theirNetUser->mNetUserId : 0;
  netUserAddResponseData.mEventBundleData = ourResponseBundle->GetBitStream();

  netUserAddResponseMessage.GetData().Write(netUserAddResponseData);

  // Determine if we should send or pass-through this message
  // (We want to pass-through if we, the server or offline, are the requester)
  bool passThrough = theirNetPeerId == 0 ? true : false;

  // Pass-through message?
  if(passThrough)
  {
    Assert(theirNetPeerId == 0);

    // Handle "sent" network user add response
    HandleSentUserAddResponse(0, IpAddress(), theirRequestBundle, netUserAddResponseData.mAddResponse, ourResponseBundle, theirNetUser);

    // Receive network user add response directly
    ReceiveUserAddResponse(0, IpAddress(), netUserAddResponseMessage, theirRequestBundle);

    // Success
    return true;
  }
  // Send message?
  else
  {
    Assert(theirNetPeerId != 0);

    // Get link
    PeerLink* link = Replicator::GetLink(theirNetPeerId);
    if(!link) // Unable?
    {
      DoNotifyError("Unable to send user add response", "Unable to get network link");
      return false;
    }

    // Send network user add response
    Status status;
    link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netUserAddResponseMessage);
    if(status.Failed()) // Unable?
    {
      Warn("Unable to send user add response - Error sending message (%s)", status.Message.c_str());
      return false;
    }

    // Handle sent network user add response
    HandleSentUserAddResponse(theirNetPeerId, link->GetTheirIpAddress(), theirRequestBundle, netUserAddResponseData.mAddResponse, ourResponseBundle, theirNetUser);

    // Success
    return true;
  }
}
void NetPeer::HandleSentUserAddResponse(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* theirRequestBundle,
                                        NetUserAddResponse::Enum ourAddResponse, EventBundle* ourResponseBundle, NetUser* theirNetUser)
{
  Assert(IsOpen());
  Assert(IsServerOrOffline());

  // User accepted?
  if(theirNetUser)
  {
    // Store response bundle for later
    theirNetUser->mResponseBundle = *ourResponseBundle;
  }

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetPeerSentUserAddResponse event(owner);
  event.mTheirNetPeerId     = theirNetPeerId;
  event.mTheirIpAddress     = theirIpAddress;
  event.mTheirRequestBundle = *theirRequestBundle;
  event.mOurAddResponse     = ourAddResponse;
  event.mOurResponseBundle  = *ourResponseBundle;
  event.mTheirNetUserId     = theirNetUser ? theirNetUser->mNetUserId : 0;       // (Set only if accepted)
  event.mTheirNetUser       = theirNetUser ? theirNetUser->GetOwner() : nullptr; // (Set only if accepted)

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerSentUserAddResponse, &event);
}

bool NetPeer::ReceiveUserAddResponse(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message, EventBundle* ourRequestBundle)
{
  Assert(IsOpen());
  Assert(theirNetPeerId == 0); // (The sender should always be the server or offline peer)
  Assert(ourRequestBundle);

  // Read network user add response message
  NetUserAddResponseData netUserAddResponseData;
  if(!message.GetData().Read(netUserAddResponseData)) // Unable?
  {
    Assert(false);
    return false;
  }
  EventBundle theirResponseBundle(static_cast<GameSession*>(GetOwner()));
  theirResponseBundle = ZeroMove(netUserAddResponseData.mEventBundleData);

  // (If accepted, we should have been provided a valid user ID)
  Assert(netUserAddResponseData.mAddResponse == NetUserAddResponse::Accept
       ? netUserAddResponseData.mNetUserId != 0
       : netUserAddResponseData.mNetUserId == 0);

  // Handle received user add response
  HandleReceivedUserAddResponse(theirNetPeerId, theirIpAddress, ourRequestBundle, netUserAddResponseData.mAddResponse, &theirResponseBundle, netUserAddResponseData.mNetUserId);

  // Success
  return true;
}
void NetPeer::HandleReceivedUserAddResponse(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, EventBundle* ourRequestBundle,
                                            NetUserAddResponse::Enum theirAddResponse, EventBundle* theirResponseBundle, NetUserId ourNetUserId)
{
  Assert(IsOpen());

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetPeerReceivedUserAddResponse event(owner);
  event.mTheirNetPeerId      = theirNetPeerId;
  event.mTheirIpAddress      = theirIpAddress;
  event.mOurRequestBundle    = *ourRequestBundle;
  event.mTheirAddResponse    = theirAddResponse;
  event.mTheirResponseBundle = *theirResponseBundle;
  event.mOurNetUserId        = ourNetUserId; // (Set only if accepted)

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerReceivedUserAddResponse, &event);
}

//
// User Remove Handshake
//

bool NetPeer::SendUserRemoveRequest(NetUserId netUserId, EventBundle* ourRequestBundle)
{
  Assert(IsOpen());

  // Create network user remove request message
  Message netUserRemoveRequestMessage(NetPeerMessageType::NetUserRemoveRequest);

  NetUserRemoveRequestData netUserRemoveRequestData;
  netUserRemoveRequestData.mNetUserId       = netUserId;
  netUserRemoveRequestData.mEventBundleData = ourRequestBundle->GetBitStream();

  netUserRemoveRequestMessage.GetData().Write(netUserRemoveRequestData);

  // Determine if we should send or pass-through this message
  // (We want to pass-through if we are the server or offline since we are also the receiver)
  bool passThrough = IsClient() ? false : true;

  // Pass-through message?
  if(passThrough)
  {
    Assert(IsServerOrOffline());

    // Receive network user remove request directly
    if(!ReceiveUserRemoveRequest(0, IpAddress(), netUserRemoveRequestMessage)) // Unable?
      Assert(false);

    // Success
    return true;
  }
  // Send message?
  else
  {
    Assert(IsClient());

    // Get server link
    PeerLink* link = Replicator::GetLink(0);
    if(!link) // Unable?
    {
      DoNotifyError("Unable to send user remove request", "NetPeer client is not connected to a server");
      return false;
    }

    // Send network user remove request
    Status status;
    link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netUserRemoveRequestMessage);
    if(status.Failed()) // Unable?
    {
      Warn("Unable to send user remove request - Error sending message (%s)", status.Message.c_str());
      return false;
    }

    // Success
    return true;
  }
}
bool NetPeer::ReceiveUserRemoveRequest(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message)
{
  Assert(IsOpen());
  Assert(IsServerOrOffline());

  // Read network user remove request message
  NetUserRemoveRequestData netUserRemoveRequestData;
  if(!message.GetData().Read(netUserRemoveRequestData)) // Unable?
  {
    Assert(false);
    return false;
  }
  EventBundle theirRequestBundle(static_cast<GameSession*>(GetOwner()));
  theirRequestBundle = ZeroMove(netUserRemoveRequestData.mEventBundleData);

  // (Should be a valid user ID)
  Assert(netUserRemoveRequestData.mNetUserId);

  // Get net user object
  Cog* cog = GetUser(netUserRemoveRequestData.mNetUserId);
  if(!cog) // Unable?
    return false;

  // Get net user component
  NetUser* netUser = cog->has(NetUser);
  if(!netUser) // Unable?
  {
    Assert(false);
    return false;
  }

  // Determine if this was a sent or pass-through this message
  // (This is a pass-through if we, the server or offline, are the requester)
  bool passThrough = theirNetPeerId == 0 ? true : false;

  // Net user was added by the requesting peer?
  // Or this is a pass-through request? (Requested by us, the server/offline peer)
  if(netUser->mNetPeerId == theirNetPeerId
  || passThrough)
  {
    // Remove the net user object
    cog->Destroy();

    // Success
    return true;
  }

  // User remove request denied
  return false;
}

//
// Object Interface
//

const String& NetPeer::GetNetObjectOnlineEventId() const
{
  return Events::NetGameOnline;
}
const String& NetPeer::GetNetObjectOfflineEventId() const
{
  return Events::NetGameOffline;
}

//
// Host Info Interface
//

BitStream NetPeer::GetBasicNetHostInfo()
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  AcquireNetHostInfo event(owner);
  event.mReturnHostInfo; // Optionally set by the event receiver

  // Dispatch event
  owner->DispatchEvent(Events::AcquireBasicNetHostInfo, &event);

  // Write host info
  BitStream bitStream = ZeroMove(event.mReturnHostInfo.GetBitStream());

  // Invalid host info size?
  if(bitStream.GetBytesWritten() > BasicNetHostInfoMaxSize)
  {
    // Failure
    DoNotifyError("Basic Host Info Too Large", "Unable to serialize acquired basic NetHost information - Serialized data must fit within 480 bytes");
    return BitStream();
  }

  // Success
  return bitStream;
}

BitStream NetPeer::GetExtraNetHostInfo()
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  AcquireNetHostInfo event(owner);
  event.mReturnHostInfo; // Optionally set by the event receiver

  // Dispatch event
  owner->DispatchEvent(Events::AcquireExtraNetHostInfo, &event);

  // Write host info
  BitStream bitStream = ZeroMove(event.mReturnHostInfo.GetBitStream());

  if (bitStream.GetBytesWritten() > MaxMessageWholeDataBytes)
  {
    // Failure
    DoNotifyError("Extra Host Info Too Large", "Unable to serialize acquired extra NetHost information - Serialized data must fit within MaxMessageWholeDataBytes");
    return BitStream();
  }

  // Success
  return bitStream;
}

void NetPeer::RemoveUnresponsiveHosts(Network::Enum network, const ArraySet<NetHost*>& respondingHosts)
{
  // Get network's host list
  NetHostSet* hostList = mHostLists.FindPointer(network);
  if(!hostList          //    Unable?
  || hostList->Empty()) // Or the host list is already empty?
  {
    // Nothing to remove
    return;
  }

  // No hosts responded?
  if(respondingHosts.Empty())
  {
    // Clear entire host list, All hosts were unresponsive
    hostList->Clear();
  }
  // Some hosts responded?
  else
  {
    // For All hosts in the current host list
    for(uint i = 0; i < hostList->Size(); )
    {
      NetHostPtr& netHost = (*hostList)[i];

      // Check if the net host responded to this ping request
      bool responded = respondingHosts.Contains(netHost);

      // Host did not respond?
      if(!responded)
      {
        // Erase and advance
        hostList->EraseAt(i);
        continue;
      }
      // Host responded?
      else
      {
        // Advance
        ++i;
        continue;
      }
    }
  }
}

//
// Level Replication
//

bool NetPeer::SendLevelLoadStarted(NetPeerId theirNetPeerId, Space* space, Level* level)
{
  Assert(IsServer());

  // Create network level load started message
  Message netLevelLoadStartedMessage(NetPeerMessageType::NetLevelLoadStarted);

  NetLevelLoadStartedData netLevelLoadStartedData;
  netLevelLoadStartedData.mNetSpaceObjectId = space->has(NetObject)->GetNetObjectId();
  netLevelLoadStartedData.mLevelResourceId  = (u64)(level ? level->mResourceId : ResourceId(0));

  netLevelLoadStartedMessage.GetData().Write(netLevelLoadStartedData);

  // Get link
  PeerLink* link = Replicator::GetLink(theirNetPeerId);
  if(!link) // Unable?
  {
    DoNotifyError("Unable to send level load started", "NetPeer link is not connected");
    return false;
  }

  // Send network level load started
  Status status;
  link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netLevelLoadStartedMessage);
  if(status.Failed()) // Unable?
  {
    Warn("Unable to send level load started - Error sending message (%s)", status.Message.c_str());
    return false;
  }

  // Success
  return true;
}
bool NetPeer::ReceiveLevelLoadStarted(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message)
{
  Assert(IsClient());

  // Read network level load started message
  NetLevelLoadStartedData netLevelLoadStartedData;
  if(!message.GetData().Read(netLevelLoadStartedData)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Get net space
  Space* space = GetNetSpace(netLevelLoadStartedData.mNetSpaceObjectId);
  if(!space) // Unable?
  {
    DoNotifyError("Error receiving NetSpace level state", "Unable to find specified NetSpace");
    return false;
  }

  // Level resource specified?
  // (It's entirely valid for there not to be a level specified,
  // we use the NetLevelLoadStarted and NetLevelLoadFinished messages to denote entire Space cloning as well,
  // which may or may not have a level loaded when it occurs.)
  if(netLevelLoadStartedData.mLevelResourceId != 0)
  {
    // Get level resource
    Level* level = static_cast<Level*>(Z::gResources->GetResource((ResourceId)netLevelLoadStartedData.mLevelResourceId));
    if(!level) // Unable?
    {
      DoNotifyError("Error receiving NetSpace level state", "Unable to find specified Level resource");
      return false;
    }

    // Not emplaced against game setup?
    // Or We're not on this level?
    if(space->has(NetSpace)->GetEmplaceContext() != cGameSetup
    || space->GetCurrentLevel() != level)
    {
      // Load level specified
      space->LoadLevel(level);
    }
  }

  // Success
  return true;
}

bool NetPeer::SendLevelLoadFinished(NetPeerId theirNetPeerId, Space* space)
{
  Assert(IsServer());

  // Create network level load finished message
  Message netLevelLoadFinishedMessage(NetPeerMessageType::NetLevelLoadFinished);

  NetLevelLoadFinishedData netLevelLoadFinishedData;
  netLevelLoadFinishedData.mNetSpaceObjectId = space->has(NetObject)->GetNetObjectId();

  netLevelLoadFinishedMessage.GetData().Write(netLevelLoadFinishedData);

  // Get link
  PeerLink* link = Replicator::GetLink(theirNetPeerId);
  if(!link) // Unable?
  {
    DoNotifyError("Unable to send level load finished", "NetPeer link is not connected");
    return false;
  }

  // Send network level load finished
  Status status;
  link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netLevelLoadFinishedMessage);
  if(status.Failed()) // Unable?
  {
    Warn("Unable to send level load finished - Error sending message (%s)", status.Message.c_str());
    return false;
  }

  // Success
  return true;
}
bool NetPeer::ReceiveLevelLoadFinished(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message)
{
  Assert(IsClient());

  // Read network level load finished message
  NetLevelLoadFinishedData netLevelLoadFinishedData;
  if(!message.GetData().Read(netLevelLoadFinishedData)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Get net space
  Space* space = GetNetSpace(netLevelLoadFinishedData.mNetSpaceObjectId);
  if(!space) // Unable?
  {
    DoNotifyError("Error receiving NetSpace level state", "Unable to find specified NetSpace");
    return false;
  }

  //
  // Delete any remaining offline emplaced net objects in this space
  // (These net objects were originally emplaced as part of the level and have since been forgotten/destroyed by the server
  // Because the server did not clone these to us in the level synchronization phase, we can infer that they must no longer exist on the server and should be destroyed here too)
  //

  // For All objects in the space
  forRange(Cog& cog, space->AllObjects())
  {
    // Doesn't have net object component?
    NetObject* netObject = cog.has(NetObject);
    if(!netObject)
      continue; // Skip

    // Net object is emplaced but not online?
    if(netObject->IsEmplaced() && !netObject->IsOnline())
    {
      // Destroy the net object
      // (It no longer exists as an online object on the server)
      cog.Destroy();
    }
  }

  // Space has current level?
  if(space->GetCurrentLevel())
  {
    // Handle network level started next engine update
    // (We need to wait for any offline emplaced net objects to be destroyed at the start of next frame before dispatching NetLevelStarted)
    space->has(NetSpace)->mPendingNetLevelStarted = true;
  }

  // Success
  return true;
}

void NetPeer::HandleNetLevelStarted(Space* space)
{
  // (Space should have a current level and have already been brought online)
  Assert(space->GetCurrentLevel());
  Assert(space->has(NetObject)->IsOnline());

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetLevelStarted event;
  event.mGameSession = owner;
  event.mSpace       = space;
  event.mLevelName   = space->GetCurrentLevel()->Name;

  // Dispatch event
  owner->DispatchEvent(Events::NetLevelStarted, &event);
  space->DispatchEvent(Events::NetLevelStarted, &event);
}

//
// Game Replication
//

bool NetPeer::SendGameLoadStarted(NetPeerId theirNetPeerId)
{
  Assert(IsServer());

  // Create network game load started message
  Message netGameLoadStartedMessage(NetPeerMessageType::NetGameLoadStarted);

  // Get link
  PeerLink* link = Replicator::GetLink(theirNetPeerId);
  if(!link) // Unable?
  {
    DoNotifyError("Unable to send game load started", "NetPeer link is not connected");
    return false;
  }

  // Send network game load started
  Status status;
  link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netGameLoadStartedMessage);
  if(status.Failed()) // Unable?
  {
    Warn("Unable to send game load started - Error sending message (%s)", status.Message.c_str());
    return false;
  }

  // Success
  return true;
}
bool NetPeer::ReceiveGameLoadStarted(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message)
{
  UnusedParameter(message);
  Assert(IsClient());

  // Currently receiving the net game
  mIsReceivingNetGame = true;

  // Do nothing (We may dispatch a "NetGameLoading" event here later)

  // Success
  return true;
}

bool NetPeer::SendGameLoadFinished(NetPeerId theirNetPeerId)
{
  Assert(IsServer());

  // Create network game load finished message
  Message netGameLoadFinishedMessage(NetPeerMessageType::NetGameLoadFinished);

  // Get link
  PeerLink* link = Replicator::GetLink(theirNetPeerId);
  if(!link) // Unable?
  {
    DoNotifyError("Unable to send game load finished", "NetPeer link is not connected");
    return false;
  }

  // Send network game load finished
  Status status;
  link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netGameLoadFinishedMessage);
  if(status.Failed()) // Unable?
  {
    Warn("Unable to send game load finished - Error sending message (%s)", status.Message.c_str());
    return false;
  }

  // Success
  return true;
}
bool NetPeer::ReceiveGameLoadFinished(NetPeerId theirNetPeerId, const IpAddress& theirIpAddress, const Message& message)
{
  UnusedParameter(message);
  Assert(IsClient());

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Get All spaces in game session
  SpaceMap::valueRange spaces = owner->GetAllSpaces();

  //
  // Delete any remaining offline emplaced net spaces and game session
  // (These net spaces were originally emplaced as part of the game session and have since been forgotten/destroyed by the server
  // Because the server did not clone these to us in the level synchronization phase, we can infer that they must no longer exist on the server and should be destroyed here too)
  //

  // For All spaces
  forRange(Space* space, spaces)
  {
    // Doesn't have net space component?
    NetSpace* netSpace = space->has(NetSpace);
    if(!netSpace)
      continue; // Skip

    // Net space is emplaced but not online?
    if(netSpace->IsEmplaced() && !netSpace->IsOnline())
    {
      // Destroy the net space
      // (It no longer exists as an online object on the server)
      space->Destroy();
    }
  }

  // Get net peer component
  NetPeer* netPeer = owner->has(NetPeer);
  Assert(netPeer == this); // (Should be ourself)

  // Net peer is emplaced but not online?
  if(netPeer->IsEmplaced() && !netPeer->IsOnline())
  {
    // Destroy the game session
    // (It no longer exists as an online object on the server)
    owner->Destroy();
  }

  // Get All spaces in game session
  spaces = owner->GetAllSpaces();

  // For All spaces
  forRange(Space* space, spaces)
  {
    // Doesn't have net space component?
    NetSpace* netSpace = space->has(NetSpace);
    if(!netSpace)
      continue; // Skip

    // Clear delayed attachments (if any)
    netSpace->ClearDelayedAttachments();
  }

  // Finished receiving the net game
  mIsReceivingNetGame = false;

  // Handle network game started next engine update
  // (We need to wait for any frame-delayed NetLevelStarted events to be dispatched first)
  mPendingNetGameStarted = true;

  // Success
  return true;
}

void NetPeer::HandleNetGameStarted()
{
  Assert(IsOpen());

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetGameStarted event;
  event.mGameSession = owner;

  // Dispatch event
  owner->DispatchEvent(Events::NetGameStarted, &event);
  owner->DispatchOnSpaces(Events::NetGameStarted, &event);
}

//
// ID Helpers
//

bool NetPeer::AssignNetUserId(NetUser* user)
{
  Assert(IsServerOrOffline());
  Assert(user->mNetUserId == 0);

  // Acquire network user ID
  NetUserId netUserId = mNetUserIdStore.AcquireId();
  if(netUserId == 0) // Unable?
    return false;

  // Assign network user ID
  user->mNetUserId = netUserId;
  Assert(user->mNetUserId != 0);

  // Success
  return true;
}
void NetPeer::ReleaseNetUserId(NetUser* user)
{
  Assert(IsServerOrOffline());
  Assert(user->mNetUserId != 0);

  // Free network user ID
  bool result = mNetUserIdStore.FreeId(user->mNetUserId);
  Assert(result);

  // OK HACK:
  // This change might get replicated at the end?
  // // Clear replica ID
  // user->mNetUserId = 0;
  // Assert(user->mNetUserId == 0);
}

//
// Replication Interface
//

Space* NetPeer::GetNetSpace(NetObjectId netObjectId) const
{
  // Find net object cog
  Cog* netObjectCog = GetNetObject(netObjectId);
  if(!netObjectCog) // Unable?
    return nullptr;

  // Not a space?
  if(netObjectCog != netObjectCog->GetSpace())
    return nullptr;

  // Doesn't have a net space component?
  if(!netObjectCog->has(NetSpace))
    return nullptr;

  // Success
  return static_cast<Space*>(netObjectCog);
}
Cog* NetPeer::GetNetObject(NetObjectId netObjectId) const
{
  // Not open or is open offline?
  if(!IsOpen() || mIsOpenOffline)
    return nullptr;

  // Get net object
  NetObject* netObject = static_cast<NetObject*>(Replicator::GetReplica(netObjectId));
  if(!netObject) // Unable?
    return nullptr;

  // Success
  return netObject->GetOwner();
}

bool NetPeer::ClaimActiveReplicaStream(const CogInitializer* cogInitializer)
{
  Assert(IsClient());

  // No active replica stream?
  if(!mActiveReplicaStream)
  {
    // Nothing to claim
    Assert(!mActiveCogInitializer);
    return false;
  }

  // We already have claim?
  if(mActiveCogInitializer == cogInitializer)
    return true;

  // Available for claim?
  if(mActiveCogInitializer == nullptr)
  {
    // Claim active replica stream
    mActiveCogInitializer = cogInitializer;
    return true;
  }

  // Otherwise, unavailable for claim
  Assert(mActiveCogInitializer);
  return false;
}
void NetPeer::SetActiveReplicaStream(const ReplicaStream* replicaStream)
{
  Assert(IsClient());
  mActiveReplicaStream  = replicaStream;
  mActiveCogInitializer = nullptr; // Reset claim
}
const ReplicaStream* NetPeer::GetActiveReplicaStream() const
{
  Assert(IsClient());
  return mActiveReplicaStream;
}

bool NetPeer::EmplaceNetObjectByGameSetup(Cog* cog)
{
  Assert(IsClientOrServer());

  // Emplace net object with the "GameSetup" string as the emplace context
  bool result = Replicator::EmplaceReplica(cog->has(NetObject), EmplaceContext(cGameSetup));
  if(!result) // Unable?
    DoNotifyWarning("Unable To Emplace NetObject By GameSetup",
                    String::Format("There was an error emplacing the NetObject '%s' against GameSetup",
                    cog->GetDescription().c_str()));
  return result;
}
bool NetPeer::EmplaceNetObjectBySpaceAndLevel(Cog* cog, Space* space, StringParam levelResourceIdName)
{
  Assert(IsClientOrServer());

  // (Net space should have already been brought online)
  Assert(space->has(NetObject)->IsOnline());

  // Create space net object ID and level resource ID name string
  String spaceAndLevelName = String::Format("NetSpace_%u_Level_%s", space->has(NetObject)->GetNetObjectId(), levelResourceIdName.c_str());

  // Emplace net object with space net object ID and level resource ID name string as the emplace context
  bool result = Replicator::EmplaceReplica(cog->has(NetObject), EmplaceContext(spaceAndLevelName));
  if(!result) // Unable?
    DoNotifyWarning("Unable To Emplace NetObject By Space And Level",
                    String::Format("There was an error emplacing the NetObject '%s' against the NetSpace '%s' with NetObjectId '%u' on Level '%s'",
                    cog->GetDescription().c_str(), space->GetDescription().c_str(), space->has(NetObject)->GetNetObjectId(), levelResourceIdName.c_str()));
  return result;
}

bool NetPeer::SpawnFamilyTree(FamilyTreeId familyTreeId, const Route& route)
{
  Assert(IsServer());

  // (Family tree ID should be valid)
  Assert(familyTreeId != 0);

  // Get family tree
  const FamilyTree* familyTree = mFamilyTrees.FindValue(familyTreeId, FamilyTreePtr());
  if(!familyTree) // Unable?
  {
    Assert(false);
    return false;
  }

  // (Family tree should not be empty)
  Assert(!familyTree->IsEmpty());

  // Spawn net objects in family tree
  bool result = Replicator::SpawnReplicas(familyTree->GetReplicas(), route);
  if(!result) // Unable?
    DoNotifyWarning("Unable To Spawn NetObject Family Tree",
                    String::Format("There was an error spawning the NetObject Family Tree originating from Ancestor '%s'",
                    familyTree->GetAncestorDisplayName().c_str()));
  return result;
}
bool NetPeer::CloneFamilyTree(FamilyTreeId familyTreeId, const Route& route)
{
  Assert(IsServer());

  // (Family tree ID should be valid)
  Assert(familyTreeId != 0);

  // Get family tree
  const FamilyTree* familyTree = mFamilyTrees.FindValue(familyTreeId, FamilyTreePtr());
  if(!familyTree) // Unable?
  {
    Assert(false);
    return false;
  }

  // (Family tree should not be empty)
  Assert(!familyTree->IsEmpty());

  // Clone net objects in family tree
  bool result = Replicator::CloneReplicas(familyTree->GetReplicas(), route);
  if(!result) // Unable?
    DoNotifyWarning("Unable To Clone NetObject Family Tree",
                    String::Format("There was an error cloning the NetObject Family Tree originating from Ancestor '%s'",
                    familyTree->GetAncestorDisplayName().c_str()));
  return result;
}

bool NetPeer::SpawnNetObject(Cog* cog, const Route& route)
{
  Assert(IsServer());

  // Spawn net object
  bool result = Replicator::SpawnReplica(cog->has(NetObject), route);
  if(!result) // Unable?
    DoNotifyWarning("Unable To Spawn NetObject",
                    String::Format("There was an error spawning the NetObject '%s'",
                    cog->GetDescription().c_str()));
  return result;
}
bool NetPeer::CloneNetObject(Cog* cog, const Route& route)
{
  Assert(IsServer());

  // Clone net object
  bool result = Replicator::CloneReplica(cog->has(NetObject), route);
  if(!result) // Unable?
    DoNotifyWarning("Unable To Clone NetObject",
                    String::Format("There was an error cloning the NetObject '%s'",
                    cog->GetDescription().c_str()));
  return result;
}
bool NetPeer::ForgetNetObject(Cog* cog, const Route& route)
{
  Assert(IsClientOrServer());

  // Forget net object
  bool result = Replicator::ForgetReplica(cog->has(NetObject), route);
  if(!result) // Unable?
    DoNotifyWarning("Unable To Forget NetObject",
                    String::Format("There was an error forgetting the NetObject '%s'",
                    cog->GetDescription().c_str()));
  return result;
}
bool NetPeer::DestroyNetObject(Cog* cog, const Route& route)
{
  Assert(IsServer());

  // Destroy net object
  bool result = Replicator::DestroyReplica(cog->has(NetObject), route);
  if(!result) // Unable?
    DoNotifyWarning("Unable To Destroy NetObject",
                    String::Format("There was an error destroying the NetObject '%s'",
                    cog->GetDescription().c_str()));
  return result;
}

bool NetPeer::CloneNetGame(NetPeerId netPeerId)
{
  if(GetRole() == Role::MasterServer) return true;

  Assert(IsServer());

  //
  // Start Net Game Load
  //

  // Send game load started message
  if(!SendGameLoadStarted(netPeerId)) // Unable?
    return false;

  //
  // Clone Network GameSession
  //

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());
  if(!owner || !owner->has(NetPeer) || !owner->has(NetPeer)->IsOnline()) // Invalid?
    return false;

  // Clone game session object
  if(!CloneNetObject(owner, Route(netPeerId))) // Unable?
    return false;

  //
  // Clone Network Spaces
  //

  // For All spaces
  SpaceMap::valueRange spaces = owner->GetAllSpaces();
  forRange(Space* space, spaces)
  {
    // Doesn't have net space component?
    if(!space->has(NetSpace))
      continue; // Skip

    // Marked for deletion?
    if(space->GetMarkedForDestruction())
      continue; // Skip

    // Clone space object
    if(!CloneNetObject(space, Route(netPeerId))) // Unable?
      return false;
  }

  //
  // Clone Network Levels (Includes All Remaining Objects)
  //

  // For All spaces
  forRange(Space* space, spaces)
  {
    // Doesn't have net space component?
    if(!space->has(NetSpace))
      continue; // Skip

    // Marked for deletion?
    if(space->GetMarkedForDestruction())
      continue; // Skip

    // Clone level state
    if(!CloneNetLevel(space, false, netPeerId)) // Unable?
      return false;
  }

  //
  // Finish Net Game Load
  //

  // Send game load finished message
  if(!SendGameLoadFinished(netPeerId)) // Unable?
    return false;

  // Success
  return true;
}
bool NetPeer::CloneNetLevel(Space* space, bool isLevelTransition, NetPeerId netPeerId)
{
  Assert(IsServer());

  // (Space should have already been brought online)
  Assert(space->has(NetObject)->IsOnline());

  // Get link
  PeerLink* link = GetLink(netPeerId);

  // Get replicator link
  ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

  // They don't have this net space yet?
  if(!replicatorLink->HasReplica(static_cast<Replica*>(space->has(NetObject))))
  {
    Assert(false);
    return false;
  }

  //
  // Start Net Level Load
  //

  // Get current level (if any)
  Level* level = space->GetCurrentLevel();
  String levelResourceId = level ? level->ResourceIdName : String();

  // Send level load started message
  if(!SendLevelLoadStarted(netPeerId, space, level)) // Unable?
    return false;

  //
  // Artificial Frame-Delay (Interrupt)
  //

  // Is a level transition? (The net space contained a previous level?)
  //if(isLevelTransition)
  if(true)
  {
    // Send a step interrupt to the client to account for the single frame-delay incurred by a level transition
    // We do this because the cloned net objects that follow need the new level to be loaded before being applied
    // This method also avoids an explicit handshake sequence which is inefficient, error-prone, and in this case unnecessary
    if(!Replicator::Interrupt(Route(netPeerId)))
    {
      Assert(false);
      return false;
    }
  }

  //
  // Clone Network Objects
  //

  // Keep track of which family trees we've already cloned here
  FamilyTreeIdSet clonedFamilyTreeIds;
  clonedFamilyTreeIds.Reserve(mFamilyTrees.Size());

  // For All objects in the space
  bool result = true;
  forRange(Cog& cog, space->AllObjects())
  {
    // Doesn't have net object component?
    NetObject* netObject = cog.has(NetObject);
    if(!netObject)
      continue; // Skip

    // Net object not online?
    // (This means the net object is not ready and not needed to be cloned here)
    // (The net object will be replicated later, most likely next frame, but not as a clone)
    if(!netObject->IsOnline())
      continue; // Skip

    // Marked for deletion?
    if(cog.GetMarkedForDestruction())
      continue; // Skip

    // Was emplaced?
    if(netObject->IsEmplaced())
    {
      // Clone net object
      if(!CloneNetObject(&cog, Route(netPeerId))) // Unable?
      {
        // Skip
        result = false;
        continue;
      }
    }
    // Was spawned?
    else
    {
      Assert(netObject->IsSpawned());

      // Get net object's family tree ID
      FamilyTreeId familyTreeId = netObject->GetFamilyTreeId();
      Assert(familyTreeId != 0);

      // That family tree has already been cloned?
      if(clonedFamilyTreeIds.Contains(familyTreeId))
        continue; // Skip

      // Clone net object's family tree
      if(!CloneFamilyTree(familyTreeId, Route(netPeerId))) // Unable?
      {
        // Skip
        result = false;
        continue;
      }

      // Add to cloned family trees list
      clonedFamilyTreeIds.Insert(familyTreeId);
    }
  }

  // Error cloning one or more net objects?
  if(!result)
  {
    DoNotifyWarning("Error Cloning Level",
                    String::Format("There was an error cloning one or more NetObjects in the Level '%s' in NetSpace '%s'",
                    levelResourceId.c_str(), space->GetDescription().c_str()));
  }

  //
  // Finish Net Level Load
  //

  // Send level load finished message
  if(!SendLevelLoadFinished(netPeerId, space)) // Unable?
    return false;

  // Success
  return true;
}
bool NetPeer::CloneNetLevel(Space* space, bool isLevelTransition)
{
  Assert(IsServer());

  // For All links
  PeerLinkSet links = GetLinks();
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Get their net peer ID
    NetPeerId theirNetPeerId = replicatorLink->GetReplicatorId().value();

    // Clone level state
    if(!CloneNetLevel(space, isLevelTransition, theirNetPeerId)) // Unable?
    {
      // Skip
      continue;
    }
  }

  // Success
  return true;
}
bool NetPeer::IsReceivingNetGame() const
{
  return mIsReceivingNetGame;
}

//
// Family Tree Interface
//

bool NetPeer::AddNetObjectToFamilyTree(NetObject* ancestor, NetObject* netObject)
{
  Assert(IsClientOrServer());

  // Get ancestor's family tree ID
  FamilyTreeId ancestorFamilyTreeId = ancestor->GetFamilyTreeId();

  // Ancestor's family tree ID not set?
  if(!ancestorFamilyTreeId)
  {
    // (This should be the ancestor)
    Assert(ancestor == netObject);
    Assert(netObject->IsAncestor());

    // Acquire family tree ID
    FamilyTreeId familyTreeId = mFamilyTreeIdStore.AcquireId();
    if(familyTreeId == 0) // Unable?
    {
      Assert(false);
      return false;
    }

    // Add new family tree with ancestor to family tree set
    FamilyTreeSet::pointer_bool_pair result = mFamilyTrees.Insert(FamilyTreePtr(new FamilyTree(familyTreeId, ancestor)));
    if(!result.second) // Unable?
    {
      Assert(false);
      return false;
    }

    // Success
    return true;
  }
  // Ancestor's family tree ID already set?
  else
  {
    // (This should be a descendant)
    Assert(ancestor != netObject);
    Assert(netObject->IsDescendant());

    // Get ancestor's family tree (this is our family tree too)
    FamilyTreePtr* familyTreePtr = mFamilyTrees.FindPointer(ancestorFamilyTreeId);
    if(!familyTreePtr) // Unable?
    {
      Assert(false);
      return false;
    }
    FamilyTree* familyTree = *familyTreePtr;

    // Add descendant to family tree
    bool result = familyTree->AddNetObject(netObject);
    if(!result) // Unable?
    {
      Assert(false);
      return false;
    }

    // Success
    return true;
  }
}
bool NetPeer::RemoveNetObjectFromFamilyTree(NetObject* netObject)
{
  Assert(IsClientOrServer());

  // Get family tree ID
  FamilyTreeId familyTreeId = netObject->GetFamilyTreeId();
  if(familyTreeId == 0) // Not set?
  {
    //Assert(false);
    return false;
  }

  // Get net object's family tree
  FamilyTreePtr* familyTreePtr = mFamilyTrees.FindPointer(familyTreeId);
  if(!familyTreePtr) // Unable?
  {
    // Assert(false);
    return false;
  }
  FamilyTree* familyTree = *familyTreePtr;

  // Remove net object (ancestor or descendant) from family tree
  bool result = familyTree->RemoveNetObject(netObject);
  if(!result) // Unable?
  {
    //Assert(false);
    return false;
  }

  // Family tree is now empty? (All net objects in the family tree are absent?)
  if(familyTree->IsEmpty())
  {
    // Remove family tree from family tree set
    mFamilyTrees.Erase(familyTreePtr);
  }

  // Success
  return true;
}

//
// Internal
//

bool NetPeer::SerializeNetEvent(BitStreamExtended& bitStream, Event* netEvent, Cog* destination)
{
  // Get destination net object ID
  // (Using ReplicaId to take advantage of WriteQuantized)
  ReplicaId destinationId = 0;
  if(destination)
    destinationId = destination->has(NetObject)->GetNetObjectId();

  // Write destination net object ID
  if(!bitStream.Write(destinationId))
    return false;

  // Write event
  return bitStream.WriteEvent(netEvent);
}
bool NetPeer::DeserializeNetEvent(const BitStreamExtended& bitStream, Event*& netEvent, Cog*& destination, NetPeerId netPeerId)
{
  // Read destination net object ID
  // (Using ReplicaId to take advantage of ReadQuantized)
  ReplicaId destinationId;
  if(!bitStream.Read(destinationId)) // Unable?
  {
    netEvent    = nullptr;
    destination = nullptr;
    return false;
  }

  // Get destination object (if possible)
  if(destinationId == 0)
    destination = GetOwner(); // (0 specifies the NetPeer)
  else
    destination = GetNetObject(destinationId.value());

  // Read event
  netEvent = bitStream.ReadEvent(static_cast<GameSession*>(GetOwner()));
  if(!netEvent) // Unable?
  {
    netEvent    = nullptr;
    destination = nullptr;
    return false;
  }

  // Automatically set properties with the [NetPeerId] attribute
  // So they can know which peer this event truly came from (if they care to know)
  SetNetPeerIdProperties(netEvent, netPeerId);

  // Success
  return true;
}

void NetPeer::HandleNetEventSent(Event* netEvent, Cog* destination, NetPeerId netPeerId)
{
  // Create event
  NetEventSent event;
  event.mTheirNetPeerId = netPeerId;
  event.mNetEvent       = netEvent;
  event.mDestination    = destination;

  // Dispatch event on NetPeer
  GetOwner()->DispatchEvent(Events::NetEventSent, &event);

  // Null destination?
  if(!destination)
  {
    Assert(false); // (This shouldn't happen when sending net events)
    return;
  }

  // Destination is not NetPeer?
  if(destination != GetOwner())
  {
    // Dispatch event on destination object
    destination->DispatchEvent(Events::NetEventSent, &event);
  }
}
void NetPeer::HandleNetEventReceived(Event* netEvent, Cog* destination, NetPeerId netPeerId)
{
  // Create event
  NetEventReceived event;
  event.mTheirNetPeerId = netPeerId;
  event.mNetEvent       = netEvent;
  event.mDestination    = destination;
  event.mReturnAllow    = true; // Optionally set by the event receiver

  // Dispatch event on NetPeer
  GetOwner()->DispatchEvent(Events::NetEventReceived, &event);

  // NetPeer does not allow the net event to be dispatched on the destination object?
  if(!event.mReturnAllow)
    return;

  // Null destination?
  if(!destination)
  {
    // (This can definitely happen when receiving net events if the destination net object is not found locally)
    return;
  }

  // Destination is not NetPeer?
  if(destination != GetOwner())
  {
    // Dispatch event on destination object
    destination->DispatchEvent(Events::NetEventReceived, &event);

    // Destination object does not allow the net event to be dispatched on the destination object?
    if(!event.mReturnAllow)
      return;
  }

  // Dispatch net event on destination object
  destination->DispatchEvent(netEvent->EventId, netEvent);
}

bool NetPeer::ValidateNetEvent(StringParam netEventId, Event* netEvent, TransmissionDirection::Enum direction)
{
  // Null event?
  if(!netEvent)
  {
    if(direction == TransmissionDirection::Outgoing)
      DoNotifyException("NetPeer", "Unable to net dispatch event - Null event");
    return false;
  }

  // Empty event ID?
  if(netEventId.Empty())
  {
    if(direction == TransmissionDirection::Outgoing)
      DoNotifyException("NetPeer", "Unable to net dispatch event - Empty event ID");
    return false;
  }

  // Get registered event type based on event ID
  BoundType* registeredEventIdType = MetaDatabase::GetInstance()->mEventMap.FindValue(netEventId, nullptr);
  if(!registeredEventIdType) // Unable?
  {
    if(direction == TransmissionDirection::Outgoing)
      DoNotifyException("NetPeer", "Unable to net dispatch event - EventId must be registered with a sends Event declaration");
    return false;
  }

  // Dispatched event type does not match the registered event type? (This prevents event spoofing)
  BoundType* dispatchedEventType = ZilchVirtualTypeId(netEvent);
  if(!dispatchedEventType->IsA(registeredEventIdType))
  {
    if(direction == TransmissionDirection::Outgoing)
      DoNotifyException("NetPeer", "Unable to net dispatch event - Event does not match the registered EventId type (see the sends Event declaration)");
    return false;
  }

  // Valid
  return true;
}

bool NetPeer::DispatchLocalInternal(StringParam netEventId, Event* netEvent, Cog* destination)
{
  // (Should be open)
  Assert(IsOpen());

  // Dispatched on this NetPeer (GameSession)?
  Cog* writeDestination = destination;
  if(writeDestination == GetOwner())
    writeDestination = nullptr; // We represent this as null

  // Invalid net event?
  if(!ValidateNetEvent(netEventId, netEvent, TransmissionDirection::Outgoing))
    return false;

  // Serialize net event
  netEvent->EventId = netEventId;
  BitStreamExtended bitStream;
  if(!SerializeNetEvent(bitStream, netEvent, writeDestination)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Get our net peer ID
  NetPeerId ourNetPeerId = GetNetPeerId();

  // Handle net event sent
  HandleNetEventSent(netEvent, destination, ourNetPeerId);

  // Deserialize net event
  Event* readNetEvent    = nullptr;
  Cog*   readDestination = nullptr;
  if(!DeserializeNetEvent(bitStream, readNetEvent, readDestination, ourNetPeerId)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Invalid net event?
  if(!ValidateNetEvent(readNetEvent->EventId, readNetEvent, TransmissionDirection::Incoming))
  {
    Assert(false);
    return false;
  }

  // Handle net event received
  HandleNetEventReceived(readNetEvent, destination, ourNetPeerId);

  // Success
  return true;
}

bool NetPeer::DispatchRemoteInternal(StringParam netEventId, Event* netEvent, NetPeerId netPeerId, Cog* destination)
{
  // (Should be open and online)
  Assert(IsOpen() && !mIsOpenOffline);

  // Dispatched on this NetPeer (GameSession)?
  Cog* writeDestination = destination;
  if(writeDestination == GetOwner())
    writeDestination = nullptr; // We represent this as null

  // Not open?
  if(!IsOpen())
  {
    Warn("Unable to net dispatch event - NetPeer is not open");
    return false;
  }

  // Invalid net event?
  if(!ValidateNetEvent(netEventId, netEvent, TransmissionDirection::Outgoing))
    return false;

  // Serialize net event
  netEvent->EventId = netEventId;
  BitStreamExtended bitStream;
  if(!SerializeNetEvent(bitStream, netEvent, writeDestination)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Get link
  PeerLink* link = Replicator::GetLink(netPeerId);
  if(!link) // Unable?
  {
    Warn("Unable to net dispatch event - Unable to get network link");
    return false;
  }

  // Create network event message
  Message netEventMessage(NetPeerMessageType::NetEvent, bitStream);

  // Send network event message
  Status status;
  link->GetPlugin<ReplicatorLink>("ReplicatorLink")->Send(status, netEventMessage);
  if(status.Failed()) // Unable?
  {
    Warn("Unable to net dispatch event - Error sending message (%s)", status.Message.c_str());
    return false;
  }

  // Handle net event sent
  HandleNetEventSent(netEvent, destination, netPeerId);

  // Success
  return true;
}

bool NetPeer::DispatchBroadcastInternal(StringParam netEventId, Event* netEvent, Cog* destination)
{
  // (Should be open and online)
  Assert(IsOpen() && !mIsOpenOffline);

  // Dispatched on this NetPeer (GameSession)?
  Cog* writeDestination = destination;
  if(writeDestination == GetOwner())
    writeDestination = nullptr; // We represent this as null

  // Not open?
  if(!IsOpen())
  {
    Warn("Unable to net dispatch event - NetPeer is not open");
    return false;
  }

  // Invalid net event?
  if(!ValidateNetEvent(netEventId, netEvent, TransmissionDirection::Outgoing))
    return false;

  // Serialize net event
  netEvent->EventId = netEventId;
  BitStreamExtended bitStream;
  if(!SerializeNetEvent(bitStream, netEvent, writeDestination)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Get links
  PeerLinkSet links = Replicator::GetLinks();
  bool result = links.Empty();
  Status status;
  forRange(PeerLink* link, links.All())
  {
    // Get replicator link
    ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

    // Create network event message
    Message netEventMessage(NetPeerMessageType::NetEvent, bitStream);

    // Send network event message
    Status linkSendStatus;
    replicatorLink->Send(status, netEventMessage);
    if(linkSendStatus.Succeeded())
    {
      result = true;

      // Handle net event sent
      HandleNetEventSent(netEvent, destination, replicatorLink->GetReplicatorId().value());
    }
    else
      status.SetFailed(linkSendStatus.Message);
  }
  // At least one send succeeded?
  if(result)
    status.SetSucceeded();

  // Unable?
  if(status.Failed())
  {
    Warn("Unable to net dispatch event - Error sending message (%s)", status.Message.c_str());
    return false;
  }

  // Success
  Assert(result);
  return true;
}

//
// Channel Type Management
//

NetChannelType* NetPeer::GetOrAddReplicaChannelType(const String& netChannelTypeName, NetChannelConfig* netChannelConfig)
{
  // Get net channel type (if it already exists)
  NetChannelType* netChannelType = static_cast<NetChannelType*>(Replicator::GetReplicaChannelType(netChannelTypeName));
  if(!netChannelType) // Doesn't exist?
  {
    // Create net channel type
    netChannelType = new NetChannelType(netChannelTypeName);
    Assert(!netChannelType->IsValid());

    // Configuration provided?
    if(netChannelConfig)
    {
      // Set configuration settings
      netChannelType->SetConfig(netChannelConfig);
    }

    // Add net channel type
    netChannelType = static_cast<NetChannelType*>(Replicator::AddReplicaChannelType(ReplicaChannelTypePtr(netChannelType)));
    Assert(netChannelType->IsValid());
  }

  // Success
  return netChannelType;
}

//
// Property Type Management
//

NetPropertyType* NetPeer::GetOrAddReplicaPropertyType(const String& netPropertyTypeName, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn, NetPropertyConfig* netPropertyConfig)
{
  // (TODO: Replace NativeType's unsafe DebugTypeName with a safe display name)
  // Get targeted net property name ("NetPropertyTypeName_NativeTypeName")
  String targetedNetPropertyTypeName = String::Format("%s_%s", netPropertyTypeName.c_str(), nativeType->mDebugTypeName);

  // Get net property type (if it already exists)
  NetPropertyType* netPropertyType = static_cast<NetPropertyType*>(Replicator::GetReplicaPropertyType(targetedNetPropertyTypeName));
  if(!netPropertyType) // Doesn't exist?
  {
    // Create net property type
    netPropertyType = new NetPropertyType(targetedNetPropertyTypeName, nativeType, serializeValueFn, getValueFn, setValueFn);
    Assert(!netPropertyType->IsValid());

    // Configuration provided?
    if(netPropertyConfig)
    {
      // Set configuration settings
      netPropertyType->SetConfig(netPropertyConfig);
    }

    // Add net property type
    netPropertyType = static_cast<NetPropertyType*>(Replicator::AddReplicaPropertyType(ReplicaPropertyTypePtr(netPropertyType)));
    Assert(netPropertyType->IsValid());
  }

  // Success
  return netPropertyType;
}

//
// Replicator Replica Interface
//

bool NetPeer::SerializeReplicas(const ReplicaArray& replicas, ReplicaStream& replicaStream)
{
  // Determine if this is a clone-from-spawn replica stream
  bool isCloneFromSpawn = false;
  if(replicaStream.GetReplicaStreamMode() == ReplicaStreamMode::Clone)
  {
    Assert(IsServer());

    // For All replicas
    forRange(Replica* replica, replicas.All())
    {
      // Absent replica?
      if(!replica)
        continue; // Skip

      // Was originally spawned?
      if(replica->IsSpawned())
      {
        isCloneFromSpawn = true;
        break;
      }
    }

    // Write 'Is Clone-From-Spawn?' Flag
    if(!replicaStream.GetBitStream().Write(isCloneFromSpawn))
    {
      Assert(false);
      return false;
    }
  }

  // Is a spawn or clone-from-spawn replica stream?
  if(replicaStream.GetReplicaStreamMode() == ReplicaStreamMode::Spawn
  || isCloneFromSpawn)
  {
    Assert(IsServer());

    // Get ancestor's create context and replica type
    CreateContext ancestorCreateContext;
    ReplicaType   ancestorReplicaType;

    // For All replicas
    forRange(Replica* replica, replicas.All())
    {
      // Absent replica?
      if(!replica)
        continue; // Skip

      // (Should be spawned, not emplaced)
      Assert(replica->IsSpawned());

      // Convert to net object
      NetObject* netObject = static_cast<NetObject*>(replica);

      // Get family tree ID
      FamilyTreeId familyTreeId = netObject->GetFamilyTreeId();
      Assert(familyTreeId != 0);

      // Get net object's family tree
      const FamilyTree* familyTree = mFamilyTrees.FindValue(familyTreeId, FamilyTreePtr());
      if(!familyTree) // Unable?
      {
        Assert(false);
        return false;
      }

      // Get ancestor's create context and replica type
      ancestorCreateContext = familyTree->GetAncestorCreateContext();
      ancestorReplicaType   = familyTree->GetAncestorReplicaType();
      break;
    }

    // Unable to get ancestor's create context and replica type?
    if(ancestorCreateContext.IsEmpty() && ancestorReplicaType.IsEmpty())
    {
      Assert(false);
      return false;
    }

    // Write creation info (ancestor's create context and replica type)
    if(!replicaStream.WriteCreationInfo(ancestorCreateContext, ancestorReplicaType)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // For All replicas
  forRange(Replica* replica, replicas.All())
  {
    // Is a reverse replica channels replica stream
    // And this replica doesn't use reverse replica channels?
    if(replicaStream.GetReplicaStreamMode() == ReplicaStreamMode::ReverseReplicaChannels
    && !replica->UsesReverseReplicaChannels())
      continue; // Skip

    // Write identification info
    if(!replicaStream.WriteIdentificationInfo(replica == nullptr, replica)) // Unable?
    {
      Assert(false);
      return false;
    }

    // Present replica?
    if(replica)
    {
      // Write channel data
      if(!replicaStream.WriteChannelData(replica)) // Unable?
      {
        Assert(false);
        return false;
      }
    }
  }

  // Success
  return true;
}
bool NetPeer::DeserializeReplicas(const ReplicaStream& replicaStream, ReplicaArray& replicas)
{
  // Game is quitting?
  if(GetGameSession()->mQuiting)
  {
    // Don't attempt to deserialize replicas (the gamesession is getting cleaned up!)
    return false;
  }

  // Determine if this is a clone-from-spawn replica stream
  bool isCloneFromSpawn = false;
  if(replicaStream.GetReplicaStreamMode() == ReplicaStreamMode::Clone)
  {
    Assert(IsClient());

    // Read 'Is Clone-From-Spawn?' Flag
    if(!replicaStream.GetBitStream().Read(isCloneFromSpawn)) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // Is a spawn or clone-from-spawn replica stream?
  if(replicaStream.GetReplicaStreamMode() == ReplicaStreamMode::Spawn
  || isCloneFromSpawn)
  {
    Assert(IsClient());

    // Get ancestor's create context and replica type
    CreateContext ancestorCreateContext;
    ReplicaType   ancestorReplicaType;

    // Read creation info (ancestor's create context and replica type)
    if(!replicaStream.ReadCreationInfo(ancestorCreateContext, ancestorReplicaType)) // Unable?
    {
      Assert(false);
      return false;
    }

    // Create replicas using the replica stream and creation info (ancestor's create context and replica type)
    if(!CreateReplicas(ancestorCreateContext, ancestorReplicaType, replicaStream, replicas)) // Unable?
    {
      Assert(false);
      return false;
    }
  }
  // Is another type of replica stream?
  else
  {
    // Gather All replicas
    while(replicaStream.GetBitStream().GetBitsUnread())
    {
      // Read identification info
      bool           isAbsent   = false;
      ReplicaId      replicaId  = 0;
      bool           isCloned   = false;
      bool           isEmplaced = false;
      EmplaceContext emplaceContext;
      EmplaceId      emplaceId  = 0;
      if(!replicaStream.ReadIdentificationInfo(isAbsent, replicaId, isCloned, isEmplaced, emplaceContext, emplaceId)) // Unable?
      {
        Assert(false);
        return false;
      }

      // Present replica?
      if(!isAbsent)
      {
        // Find replica
        Replica* replica = nullptr;
        {
          // Is a clone?
          if(isCloned)
          {
            // (Should have a valid replica ID, because the emplaced object needs an assigned live ID)
            Assert(replicaId != 0);
            // (Should be a clone-from-emplacement, because our clone-from-spawn case is handled above)
            Assert(isEmplaced);

            // Find replica by emplace context and ID
            replica = GetReplicaByEmplaceContext(emplaceContext, emplaceId);

            // Unable to find emplaced replica?
            if(!replica)
            {
              // Get the emplace context as a string
              String emplaceContextString = emplaceContext.GetOrError<String>();
              DoNotifyWarning("Error Deserializing Emplaced NetObject",
                              String::Format("The EmplaceContext '%s' appears to be mismatched -"
                                             " Please verify All peers emplace objects in the exact same way inside this context.",
                                           emplaceContextString.c_str()));
              return false;
            }

            // (Should not already have a replica ID)
            Assert(replica->GetReplicaId() == 0);

            // Set replica ID
            replica->SetReplicaId(replicaId);

            // Set "Is cloned?" flag
            replica->SetIsCloned(isCloned);
          }
          // Is not a clone?
          else
          {
            // (Should have a valid replica ID, otherwise we have no way of finding this replica)
            Assert(replicaId != 0);
            // (Should definitely not be emplaced, such a case is not possible)
            Assert(!isEmplaced);

            // Find replica by replica ID
            replica = GetReplica(replicaId);
          }
        }
        if(!replica) // Unable?
        {
          DoNotifyWarning("Error Deserializing NetObject",
                          "The specified NetObject could not be found locally.");
          return false;
        }

        // Read channel data
        if(!replicaStream.ReadChannelData(replica)) // Unable?
        {
          Assert(false);
          return false;
        }

        // Add replica
        replicas.PushBack(replica);
      }
    }
    if(replicas.Empty()) // Unable?
    {
      Assert(false);
      return false;
    }
  }

  // (Replica stream should have been entirely read by now)
  Assert(replicaStream.GetBitStream().GetBitsUnread() == 0);

  // Success
  return true;
}

bool NetPeer::CreateReplicas(const CreateContext& createContext, const ReplicaType& replicaType, const ReplicaStream& replicaStream, ReplicaArray& replicas)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  //
  // Get Create Context (Space)
  //
  Space* space = nullptr;

  // Empty create context?
  if(createContext.IsEmpty())
  {
    DoNotifyError("Unable to create NetObject", "Empty create context");

    // Failure
    return false;
  }

  // Get space net object ID from create context
  NetObjectId spaceNetObjectId = createContext.GetOrError<NetObjectId>();

  // Is a net space being created?
  if(spaceNetObjectId == 0)
  {
    space = nullptr;
  }
  // Is a net object being created?
  else
  {
    // Get net space
    space = GetNetSpace(spaceNetObjectId);
    if(!space) // Unable?
    {
      DoNotifyError("Unable to create NetObject", "Cannot find NetSpace");

      // Failure
      return false;
    }
  }

  //
  // Get Replica Type (Archetype Resource)
  //
  Archetype* archetype = nullptr;

  // Empty replica type?
  if(replicaType.IsEmpty())
  {
    DoNotifyError("Unable to create NetObject", "Empty replica type");

    // Failure
    return false;
  }

// Using Archetype "ResourceId:Name" String as ReplicaType? (Easier to debug)
#ifdef NETOBJECT_USE_RESOURCE_ID_NAME_STRING

  // Get archetype resource ID name from replica type
  String archetypeResourceIdName = replicaType.GetOrError<String>();

  // Has archetype resource ID name?
  if(!archetypeResourceIdName.Empty())
  {
    // Find archetype by resource ID name
    archetype = static_cast<Archetype*>(Z::gResources->GetResourceByName(archetypeResourceIdName));
  }

// Using Archetype ResourceId u64 as ReplicaType? (Much more efficient)
#else

  // Get archetype resource ID from replica type
  u64 archetypeResourceId = replicaType.GetOrError<u64>();

  // Has archetype resource ID?
  if(archetypeResourceId != 0)
  {
    // Find archetype by resource ID
    archetype = static_cast<Archetype*>(Z::gResources->GetResource(archetypeResourceId));
  }

#endif

  // Archetype not found?
  if(!archetype)
  {
    // This seems to indicate we're missing a resource that the other side claims to have
    DoNotifyError("Unable to create NetObject", "Unable to find specified Archetype resource");

    // Failure
    return false;
  }

  //
  // Create Replica (NetObject)
  //
  Cog* cog = nullptr;

  // Set active replica stream
  // (Will be used in the following create call)
  SetActiveReplicaStream(&replicaStream);

  // Is a net space being created?
  if(!space)
  {
    // Create cog of specified archetype (must be a space archetype)
    cog = static_cast<Cog*>(owner->CreateNamedSpace(archetype->Name, archetype));
    if(!cog) // Unable?
    {
      DoNotifyError("Unable to create NetSpace", "Unable to create Cog of specified Archetype");

      // Clear active replica stream
      SetActiveReplicaStream(nullptr);

      // Failure
      return false;
    }

    // Get net space component
    NetSpace* netSpace = cog->has(NetSpace);
    if(!netSpace) // Unable?
    {
      DoNotifyError("Unable to create NetSpace", "Archetype does not have NetSpace Component");

      // Clear active replica stream
      SetActiveReplicaStream(nullptr);

      // Destroy created object
      cog->Destroy();

      // Failure
      return false;
    }
  }
  // Is a net object being created?
  else
  {
    // Create cog of specified archetype in specified space (must be an object archetype)
    cog = space->CreateNamed(archetype->ResourceIdName, archetype->Name);
    if(!cog) // Unable?
    {
      DoNotifyError("Unable to create NetObject", "Unable to create Cog of specified Archetype");

      // Clear active replica stream
      SetActiveReplicaStream(nullptr);

      // Failure
      return false;
    }
  }

  // Clear active replica stream
  SetActiveReplicaStream(nullptr);

  // Get net object component
  NetObject* netObject = cog->has(NetObject);
  if(!netObject) // Unable?
  {
    DoNotifyError("Unable to create NetObject", "Archetype does not have NetObject Component");

    // Destroy created object
    cog->Destroy();

    // Failure
    return false;
  }

  // Get family tree ID
  FamilyTreeId familyTreeId = netObject->GetFamilyTreeId();
  Assert(familyTreeId != 0);

  // Get net object's family tree
  const FamilyTree* familyTree = mFamilyTrees.FindValue(familyTreeId, FamilyTreePtr());
  if(!familyTree) // Unable?
  {
    DoNotifyError("Unable to create NetObject", "Could not find NetObject's Family Tree");

    // Destroy created object
    cog->Destroy();

    // Failure
    return false;
  }

  // Family tree is empty? (All net objects are absent somehow?)
  if(familyTree->IsEmpty())
  {
    DoNotifyError("Unable to create NetObject", "NetObject's Family Tree is empty");

    // Destroy created object
    cog->Destroy();

    // Failure
    return false;
  }

  // Copy created replicas from the net object's family tree
  // (Any net object's that were absent have already been removed)
  replicas = familyTree->GetReplicas();

  // Success
  return true;
}

bool NetPeer::ReleaseReplicas(const ReplicaArray& replicas)
{
  // For All replicas
  forRange(Replica* replica, replicas.All())
  {
    // Absent replica?
    if(!replica)
      continue; // Skip

    // Replica was spawned?
    if(replica->IsSpawned())
    {
      // Convert to net object
      NetObject* netObject = static_cast<NetObject*>(replica);

      // Get cog
      Cog* cog = netObject->GetOwner();

      // Destroy cog
      cog->Destroy();
    }
  }

  // Success
  return true;
}

void NetPeer::OnValidReplica(Replica* replica)
{
  // Get net object
  NetObject* netObject = static_cast<NetObject*>(replica);

  // // Get space net object ID from create context
  // NetObjectId spaceNetObjectId = netObject->GetCreateContext().GetOrDefault<NetObjectId>(0);
  // 
  // // (Sanity check: Verify create context is appropriately set by this point in All cases)
  // if(netObject->IsNetPeer() || netObject->IsNetSpace())
  //   Assert(spaceNetObjectId == 0);
  // else
  //   Assert(spaceNetObjectId != 0);
}
void NetPeer::OnLiveReplica(Replica* replica)
{
  // Get net object
  NetObject* netObject = static_cast<NetObject*>(replica);

  // Handle net object coming online
  netObject->HandleNetObjectOnline();
}
void NetPeer::OnInvalidReplica(Replica* replica, bool isForget)
{
  // Get net object
  NetObject* netObject = static_cast<NetObject*>(replica);

  // Net object still online?
  if(netObject->IsOnline())
  {
    // Is server?
    if(IsServer())
      Assert(isForget);

    // Handle net object going offline
    netObject->HandleNetObjectOffline();
  }
}

void NetPeer::OnReplicaChannelPropertyChange(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, Replica* replica, ReplicaChannel* replicaChannel, ReplicaProperty* replicaProperty, TransmissionDirection::Enum direction)
{
  // (This should only be called on an actual property change)
  Assert(replicaProperty->GetValue() != replicaProperty->GetLastValue());

  // Get net object
  NetObject* netObject = static_cast<NetObject*>(replica);

  // This is net object's net user owner ID property change?
  if(replicaChannel->GetName()  == "NetObject"
  && replicaProperty->GetName() == "NetUserOwnerUserId")
  {
    // (We've configured the channel to only tell us about incoming changes)
    Assert(direction == TransmissionDirection::Incoming);

    // Get previous net user owner ID
    const Variant& lastValue = replicaProperty->GetLastValue();
    NetUserId previousNetUserOwnerUserId = lastValue.GetOrDefault<NetUserId>(0);

    // Has changed?
    if(netObject->GetNetUserOwnerUserId() != previousNetUserOwnerUserId)
    {
      // Handle the net user owner change
      netObject->HandleNetUserOwnerChanged(previousNetUserOwnerUserId);
    }
  }
  // Other net channel change?
  else
  {
    // (We already configured when to notify the user at the replicator layer)

    // Get combined net property name
    StringSplitRange combinedNetPropertyName = replicaProperty->GetName().Split("_");

    // Get component and property names
    String componentName = combinedNetPropertyName.Front();
    combinedNetPropertyName.PopFront();
    String propertyName = combinedNetPropertyName.Front();

    // Get net object owner
    Cog* cog = netObject->GetOwner();

    // Create net property change event
    NetChannelPropertyChange event;
    event.mTimestamp        = TimeMsToFloatSeconds(timestamp);
    event.mReplicationPhase = replicationPhase;
    event.mDirection        = direction;
    event.mObject           = cog;
    event.mChannelName      = replicaChannel->GetName();
    event.mComponentName    = componentName;
    event.mPropertyName     = propertyName;

    // Determine net property change event ID (based on direction and replication phase)
    String eventId;
    switch(direction)
    {
    default:
    case TransmissionDirection::Unspecified:
      Assert(false);
      break;
    case TransmissionDirection::Outgoing:

      switch(replicationPhase)
      {
      default:
        Assert(false);
        break;
      case ReplicationPhase::Initialization:
        eventId = Events::NetChannelOutgoingPropertyInitialized;
        break;
      case ReplicationPhase::Uninitialization:
        eventId = Events::NetChannelOutgoingPropertyUninitialized;
        break;
      case ReplicationPhase::Change:
        eventId = Events::NetChannelOutgoingPropertyChanged;
        break;
      }

      break;
    case TransmissionDirection::Incoming:

      switch(replicationPhase)
      {
      default:
        Assert(false);
        break;
      case ReplicationPhase::Initialization:
        eventId = Events::NetChannelIncomingPropertyInitialized;
        break;
      case ReplicationPhase::Uninitialization:
        eventId = Events::NetChannelIncomingPropertyUninitialized;
        break;
      case ReplicationPhase::Change:
        eventId = Events::NetChannelIncomingPropertyChanged;
        break;
      }

      break;
    }

    // Dispatch net property change event
    cog->DispatchEvent(eventId, &event);
  }
}

//
// Replicator Link Interface
//

void NetPeer::AddingLink(PeerLink* link)
{
  // (PeerLink should not already have user data)
  Assert(!link->GetUserData());

  // Set PeerLink user data
  NetLinkData* netLinkData = new NetLinkData;
  link->SetUserData(netLinkData);
}
void NetPeer::RemovingLink(PeerLink* link)
{
  // (PeerLink should already have user data)
  Assert(link->GetUserData());

  // Clear PeerLink user data
  NetLinkData* netLinkData = link->GetUserData<NetLinkData>();
  if(netLinkData)
    delete netLinkData;
  else
    Assert(false);

  link->SetUserData(nullptr);
}

//
// Replicator Handshake Sequence Interface
//

void NetPeer::ClientOnConnectRequest(ReplicatorLink* link, ConnectRequestData& connectRequestData)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Read network connect request data
  NetConnectRequestData netConnectRequestData;
  if(!connectRequestData.mExtraData.Read(netConnectRequestData)) // Unable?
  {
    DoNotifyError("Client Connect Request Error", "Unable to read connect request data");
    return;
  }

  // Create event
  NetPeerSentConnectRequest event(owner);
  event.mTheirIpAddress                = link->GetLink()->GetTheirIpAddress();
  event.mOurRequestBundle              = ZeroMove(netConnectRequestData.mEventBundleData);
  event.mOurPendingUserAddRequestCount = netConnectRequestData.mAddUserRequestCount;

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerSentConnectRequest, &event);
}
BitStream NetPeer::ClientOnConnectResponse(ReplicatorLink* link, ConnectResponseData& connectResponseData)
{
  // No longer waiting on a connect response, allow another connect request
  mWaitingOnConnectResponse = false;

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Read network connect request data
  NetConnectRequestData netConnectRequestData;
  if(!link->GetLastConnectRequestData().mExtraData.Read(netConnectRequestData)) // Unable?
  {
    DoNotifyError("Client Connect Response Error", "Unable to read last connect request data");
    return BitStream();
  }

  // Create event
  NetPeerReceivedConnectResponse event(owner);
  event.mTheirIpAddress                = link->GetLink()->GetTheirIpAddress();
  event.mOurRequestBundle              = ZeroMove(netConnectRequestData.mEventBundleData);
  event.mOurPendingUserAddRequestCount = netConnectRequestData.mAddUserRequestCount;
  event.mOurIpAddress                  = connectResponseData.mIpAddress;
  event.mTheirConnectResponse          = connectResponseData.mConnectResponse;
  event.mTheirResponseBundle           = connectResponseData.mExtraData;
  event.mOurNetPeerId                  = Replicator::GetReplicatorId().value();

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerReceivedConnectResponse, &event);

  // Return extra data
  return BitStream();
}
void NetPeer::ClientOnConnectConfirmation(ReplicatorLink* link, BitStream& connectConfirmationData)
{
  // Handle net link connected
  HandleNetLinkConnected(link, connectConfirmationData, TransmissionDirection::Outgoing);
}
void NetPeer::ClientOnDisconnectNotice(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction)
{
  // Handle net link disconnected
  HandleNetLinkDisconnected(link, disconnectNoticeData, direction);
}

Pair<bool, BitStream> NetPeer::ServerOnConnectRequest(ReplicatorLink* link, ConnectRequestData& connectRequestData)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Read network connect request data
  NetConnectRequestData netConnectRequestData;
  if(!connectRequestData.mExtraData.Read(netConnectRequestData)) // Unable?
  {
    DoNotifyError("Server Connect Request Error", "Unable to read connect request data");
    return Pair<bool, BitStream>(false);
  }

  // Create event
  NetPeerReceivedConnectRequest event(owner);
  event.mTheirIpAddress                  = link->GetLink()->GetTheirIpAddress();
  event.mTheirRequestBundle              = ZeroMove(netConnectRequestData.mEventBundleData);
  event.mTheirPendingUserAddRequestCount = netConnectRequestData.mAddUserRequestCount;
  event.mOurIpAddress                    = connectRequestData.mIpAddress;
  event.mReturnOurConnectResponse        = true; // Optionally set by the event receiver
  event.mReturnOurResponseBundle;                // Optionally set by the event receiver

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerReceivedConnectRequest, &event);

  // Return accept decision and extra data
  Pair<bool, BitStream> result(event.mReturnOurConnectResponse, ZeroMove(event.mReturnOurResponseBundle.GetBitStream()));
  return ZeroMove(result);
}
void NetPeer::ServerOnConnectResponse(ReplicatorLink* link, ConnectResponseData& connectResponseData)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Read network connect request data
  NetConnectRequestData netConnectRequestData;
  if(!link->GetLastConnectRequestData().mExtraData.Read(netConnectRequestData)) // Unable?
  {
    DoNotifyError("Server Connect Response Error", "Unable to read last connect request data");
    return;
  }

  // Create event
  NetPeerSentConnectResponse event(owner);
  event.mTheirNetPeerId     = link->GetReplicatorId().value();
  event.mTheirIpAddress     = link->GetLink()->GetTheirIpAddress();
  event.mTheirRequestBundle              = ZeroMove(netConnectRequestData.mEventBundleData);
  event.mTheirPendingUserAddRequestCount = netConnectRequestData.mAddUserRequestCount;
  event.mOurIpAddress       = link->GetLink()->GetOurIpAddress();
  event.mOurConnectResponse = connectResponseData.mConnectResponse;
  event.mOurResponseBundle  = connectResponseData.mExtraData;

  // Dispatch event
  owner->DispatchEvent(Events::NetPeerSentConnectResponse, &event);

  // We accepted the client?
  if(connectResponseData.mConnectResponse == ConnectResponse::Accept)
  {
    // Clone entire network game state to client
    if(!CloneNetGame(link->GetReplicatorId().value())) // Unable?
    {
       DoNotifyError("Error Accepting Client",
                    String::Format("Error cloning the network game to the accepted client. Disconnecting the client now."));

      // Disconnect client
      DisconnectLink(link->GetReplicatorId().value());
      return;
    }
  }
}
void NetPeer::ServerOnConnectConfirmation(ReplicatorLink* link, BitStream& connectConfirmationData)
{
  // Handle net link connected
  HandleNetLinkConnected(link, connectConfirmationData, TransmissionDirection::Incoming);
}
void NetPeer::ServerOnDisconnectNotice(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction)
{
  // Handle net link disconnected
  HandleNetLinkDisconnected(link, disconnectNoticeData, direction);
}

//
// Internal
//

void NetPeer::HandleNetLinkConnected(ReplicatorLink* link, BitStream& connectConfirmationData, TransmissionDirection::Enum direction)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetLinkConnected event;
  event.mTheirNetPeerId   = link->GetReplicatorId().value();
  event.mTheirIpAddress   = link->GetLink()->GetTheirIpAddress();
  event.mDirection        = direction;

  // Dispatch event
  owner->DispatchEvent(Events::NetLinkConnected, &event);
}
void NetPeer::HandleNetLinkDisconnected(ReplicatorLink* link, DisconnectNoticeData& disconnectNoticeData, TransmissionDirection::Enum direction)
{
  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(GetOwner());

  // Create event
  NetLinkDisconnected event(owner);
  event.mTheirNetPeerId   = link->GetReplicatorId().value();
  event.mTheirIpAddress   = link->GetLink()->GetTheirIpAddress();
  event.mDisconnectReason = disconnectNoticeData.mDisconnectReason;
  event.mRequestBundle    = disconnectNoticeData.mExtraData;
  event.mDirection        = direction;

  // Dispatch event
  owner->DispatchEvent(Events::NetLinkDisconnected, &event);

  // Is server?
  if(IsServer())
  {
    //
    // Link Scope Clean-up:
    //

    // Remove any remaining users added by the remote peer
    NetUserRange users = GetUsersAddedByPeer(link->GetReplicatorId().value());
    forRange(Cog* cog, users)
      RemoveUser(cog);
  }
}

void NetPeer::ProcessReceivedCustomPacket(Peer* peer, InPacket& packet)
{
  // Get net peer
  NetPeer* netPeer = static_cast<NetPeer*>(peer);

  // For All messages
  forRange(Message& message, packet.GetMessages().All())
  {
    // Handle message

    IpAddress messageSource = packet.GetSourceIpAddress();

    //Check this netpeers ping manager. it may want to respond to a ping with a pong.
    if ( netPeer->mPingManager.ReceivePeerMessage(messageSource, message) ) continue;
    //check this netpeers LanHostDiscovery. It may have pinged, and be looking for a pong.
    else if ( netPeer->mLanHostDiscovery.ReceivePeerMessage(messageSource, message)) continue;
    //check this netpeers InternetHostDiscovery. It may have pinged, and be looking for a pong.
    else if ( netPeer->mInternetHostDiscovery.ReceivePeerMessage(messageSource, message)) continue;
    //else let net peer decide what to do with the message.
    else switch(message.GetType())
    {
    default:
      {
        // Ignore message
        Warn("Unable to process network message - Unknown message type");
      }
      break;

    //master server only handles receiving host publishings.
    case NetPeerMessageType::NetHostPublish:
      {
        // Receive network host pong
        netPeer->ReceiveHostPublish(messageSource, message);
      }
      break;
    }
  }

  // // Not an event message?
  // if(!packet.mContainsEventMessage)
  // {
  //   // Ignore packet
  //   return;
  // }
  // 
  // // Read message
  // Message message;
  // if(!packet.mData.Read(message)) // Unable?
  // {
  //   // Ignore packet
  //   return;
  // }
  // 
  // // Handle message
  // switch(message.GetType())
  // {
  // default:
  //   // Ignore packet
  //   return;
  // 
  // //
  // // Peer Events
  // //
  // case PeerEventMessageType::IncomingLinkCreated:
  //   // Do nothing
  //   return;
  // 
  // case PeerEventMessageType::FatalError:
  //   {
  //     // Read fatal error message data
  //     FatalErrorData fatalErrorData;
  //     if(!message.GetData().Read(fatalErrorData)) // Unable?
  //     {
  //       // Ignore packet
  //       Assert(false);
  //       break;
  //     }
  // 
  //     // Handle fatal error message
  //     Error("Fatal NetPeer error (%s)\n", fatalErrorData.mErrorString.c_str());
  //   }
  //   break;
  // }
}

bool NetPeer::ProcessReceivedCustomMessage(PeerLink* link, Message& message)
{
  // Get net peer
  NetPeer* netPeer = static_cast<NetPeer*>(link->GetPeer());

  // Get replicator link
  ReplicatorLink* replicatorLink = link->GetPlugin<ReplicatorLink>("ReplicatorLink");

  // Get their net peer ID
  NetPeerId theirNetPeerId = replicatorLink->GetReplicatorId().value();

  IpAddress messageSource = link->GetTheirIpAddress();

  //Does the net peers LAN discovery handle this message?
  if (netPeer->mLanHostDiscovery.ReceiveLinkMessage(messageSource, message))
    return true;
  //Does the net peers internet discovery handle this message?
  else if (netPeer->mInternetHostDiscovery.ReceiveLinkMessage(messageSource, message))
    return true;
  //else let net peer decide what to do with the message.
  else switch(message.GetType()) // Handle message
  {
  default:
    {
      // Ignore message
      Warn("Unable to process network message - Unknown message type");
    }
    break;

  //
  // NetPeer Events
  //
  case NetPeerMessageType::NetEvent:
    {
      // Deserialize net event
      Event* netEvent    = nullptr;
      Cog*   destination = nullptr;
      if(!netPeer->DeserializeNetEvent(static_cast<BitStreamExtended&>(message.GetData()), netEvent, destination, theirNetPeerId)) // Unable?
      {
        // Continue
        return true;
      }

      // Invalid net event?
      if(!netPeer->ValidateNetEvent(netEvent->EventId, netEvent, TransmissionDirection::Incoming))
      {
        // Continue
        return true;
      }

      // Handle net event received
      netPeer->HandleNetEventReceived(netEvent, destination, theirNetPeerId);
    }
    break;

  case NetPeerMessageType::NetUserAddRequest:
    {
      // Is server?
      if(netPeer->IsServer())
      {
        // Receive network user add request
        netPeer->ReceiveUserAddRequest(theirNetPeerId, link->GetTheirIpAddress(), message);
      }
      else
      {
        // Ignore message
        Warn("Unable to process network user add request message - NetPeer must be a server");
      }
    }
    break;

  case NetPeerMessageType::NetUserAddResponse:
    {
      // Is client?
      if(netPeer->IsClient())
      {
        // There is a pending outgoing user request?
        if(!netPeer->mPendingUserRequests.Empty())
        {
          // Receive network user add response
          netPeer->ReceiveUserAddResponse(theirNetPeerId, link->GetTheirIpAddress(), message, &netPeer->mPendingUserRequests.Front().mOurRequestBundle);

          // Remove pending request (it's been handled)
          netPeer->mPendingUserRequests.PopFront();
        }
        else
        {
          // Ignore message
          Warn("Unable to process network user add response message - No pending outgoing user add request");
        }
      }
      else
      {
        // Ignore message
        Warn("Unable to process network user add response message - NetPeer must be a client");
      }
    }
    break;

  case NetPeerMessageType::NetUserRemoveRequest:
    {
      // Is server?
      if(netPeer->IsServer())
      {
        // Receive network user remove request
        netPeer->ReceiveUserRemoveRequest(theirNetPeerId, link->GetTheirIpAddress(), message);
      }
      else
      {
        // Ignore message
        Warn("Unable to process network user remove request message - NetPeer must be a server");
      }
    }
    break;

  case NetPeerMessageType::NetLevelLoadStarted:
    {
      // Is client?
      if(netPeer->IsClient())
      {
        // Receive network level load started
        netPeer->ReceiveLevelLoadStarted(theirNetPeerId, link->GetTheirIpAddress(), message);
      }
      else
      {
        // Ignore message
        Warn("Unable to process network level load started message - NetPeer must be a client");
      }
    }
    break;

  case NetPeerMessageType::NetLevelLoadFinished:
    {
      // Is client?
      if(netPeer->IsClient())
      {
        // Receive network level load finished
        netPeer->ReceiveLevelLoadFinished(theirNetPeerId, link->GetTheirIpAddress(), message);
      }
      else
      {
        // Ignore message
        Warn("Unable to process network level load finished message - NetPeer must be a client");
      }
    }
    break;

  case NetPeerMessageType::NetGameLoadStarted:
    {
      // Is client?
      if(netPeer->IsClient())
      {
        // Receive network game load started
        netPeer->ReceiveGameLoadStarted(theirNetPeerId, link->GetTheirIpAddress(), message);
      }
      else
      {
        // Ignore message
        Warn("Unable to process network game load started message - NetPeer must be a client");
      }
    }
    break;

  case NetPeerMessageType::NetGameLoadFinished:
    {
      // Is client?
      if(netPeer->IsClient())
      {
        // Receive network game load finished
        netPeer->ReceiveGameLoadFinished(theirNetPeerId, link->GetTheirIpAddress(), message);
      }
      else
      {
        // Ignore message
        Warn("Unable to process network game load finished message - NetPeer must be a client");
      }
    }
    break;

  //
  // Link Events
  //
  case LinkEventMessageType::ConnectRequested:
  case LinkEventMessageType::ConnectResponded:
  case LinkEventMessageType::DisconnectNoticed:
  case LinkEventMessageType::IncomingChannelOpened:
  case LinkEventMessageType::IncomingChannelClosed:
  case LinkEventMessageType::StateChange:
  case LinkEventMessageType::StatusChange:
  break;

  //Master server receives receipt.
  case LinkEventMessageType::Receipt:
    {

      // Create receipt event message
      ReceiptData receiptData;
      if(!message.GetData().Read(receiptData))
      {
        Warn("Unable to process link message receipt!");
      }

      //At the moment, only master server cares about these receipts...
      if(netPeer->GetRole() != Role::MasterServer)
      {
        //ZPrint("Non-Master Server: Got receipt, but broke out because no one else uses it at the moment.");
        break;
      };

      //figure out who this was a receipt for
      IpAddress clientIp = netPeer->mReceiptRecipients[receiptData.mReceiptId];
      //disconnect them, we are done with their link.
      if(netPeer->HasLink(clientIp))
      {
        //ZPrint("Master server: Disconnecting from peer.\n");
        netPeer->DisconnectLink(clientIp);
      }
      //else if they don't have the link, the client likely already disconnected from the master server on their end first.

    }
    break;
  }

  // Continue
  return true;
}


//Servers every update check to see if they should publish server data to subscribed master servers.
void NetPeer::UpdatePublishInterval(UpdateEvent* event)
{
  //Only servers have publish intervals.
  if(GetRole() != Role::Server || !mInternetDiscoverable) return;

  mPublishElapsedTime += event->RealDt;

  if (mPublishElapsedTime >= mInternetHostPublishInterval)
  {
    mPublishElapsedTime -= mInternetHostPublishInterval;

    // Get owner as game session
    GameSession* owner = static_cast<GameSession*>(GetOwner());
    // Create event
    AcquireNetHostInfo event(owner);
    // AquireHostInfo
    owner->DispatchEvent(Events::AcquireBasicNetHostInfo, &event);
    
    
    // Create network host ping message
    Message netHostPublishMessage(NetPeerMessageType::NetHostPublish);

    NetHostPublishData netHostPublishData;
    netHostPublishData.mProjectGuid = GetOurProjectGuid();
    netHostPublishData.mBasicHostInfo = ZeroMove(event.mReturnHostInfo.GetBitStream());

    netHostPublishMessage.GetData().Write(netHostPublishData);
    
    //for each master server subscription
    forRange(auto ipAddress, mMasterServerSubscriptions.All() )
    {
      Peer::Send(ipAddress, netHostPublishMessage);
      //ZPrint("Publishing my server %s to the master server %s\n", GetIpv4Address().GetString().c_str(), ipAddress.GetString().c_str());
    }
  }
}

bool NetPeer::HandlePing(IpAddress const& theirIpAddress, NetHostPingData& netHostPingData)
{
  //Clients or non-discoverables may get pings. they should just ignore them.
  if(GetRole() == Role::Client || !(GetLanDiscoverable() || GetInternetDiscoverable()) ) return true;
  
  EventBundle pingBundle(static_cast<GameSession*>(GetOwner()));
  pingBundle = ZeroMove(netHostPingData.mEventBundleData);

  switch (GetRole())
  {
    case Role::MasterServer: //A master server finds and sends (if it has it)
    {
      //check ping bundle. did they write to it?
      if (pingBundle.IsEmpty()) return true; // if its empty, this is not a valid ping request for a master server, so ignore it.
                                             //otherwise they must have specified the project from which they want to get the host info for.
                                             //not only the project type, but also the IP address of the server.

      NetRequestHostRefreshData requestRefreshData;
      pingBundle.GetBitStream().Read(requestRefreshData);

      HostRecordsMap& recordMap = GetProjectRecordsMap(requestRefreshData.mProjectGuid);
      NetHostRecord* hostRecord = recordMap.FindValue(requestRefreshData.mHostIp, nullptr);

      //if we have this record, send its host info. Master server wont do anything if it doesn't have the record.
      if (hostRecord != nullptr)
      {
        BitStream masterServerPongResponseBundle;
        NetHostRefreshData refreshData;

        refreshData.mHostIp = hostRecord->mIpAddress;
        refreshData.mBasicHostInfo = hostRecord->mBasicHostInfo.GetBitStream();
        masterServerPongResponseBundle.Write(refreshData);

        mPingManager.SendHostPong(theirIpAddress, netHostPingData.mPingId, netHostPingData.mSendAttemptId, netHostPingData.mManagerId, masterServerPongResponseBundle);
      }

      return true;
    }break;

    case Role::Server: //A server sends its basic host info.
    {
      // Get their project GUID
      Guid theirProjectGuid = netHostPingData.mProjectGuid;

      // Our peer is not configured to be LAN discoverable?
      bool discoverable = IsServer() && (GetLanDiscoverable() || GetInternetDiscoverable());
      if (!discoverable)
      {
        // Do nothing
        return true;
      }

      // Get our project GUID
      Guid ourProjectGuid = GetOurProjectGuid();

      // Ping is from a different project? (Project GUIDs don't match?)
      if (ourProjectGuid != theirProjectGuid)
      {
        // Do nothing. don't respond to other peoples games.
        return true;
      }

      // Get basic project-specific host information
      BitStream basicNetHostInfo = GetBasicNetHostInfo();
      Assert(basicNetHostInfo.GetBytesWritten() <= BasicNetHostInfoMaxSize);

      // Send host pong message reply
      mPingManager.SendHostPong(theirIpAddress, netHostPingData.mPingId, netHostPingData.mSendAttemptId, netHostPingData.mManagerId, basicNetHostInfo);
      return true;
    }

    default:
    {
      return false; //if we get pinged as a client or offline, do not handle the message.
    }break;
  }
}

//Master Servers handle ping.
void NetPeer::MasterServerRecievePublish(const IpAddress& theirIpAddress, EventBundle& basicHostInfo, Guid const& thierProjectGuid )
{
  // Have we received a ping from this server?
  // If yes, update that record.
  // if no, has this IP address exceeded the SameIpHostRecordLimit?
  // if yes, do nothing.
  // if no, increase IpAddressServerCounts, add listing to HostRecords array.

  // Get project record map to which this server record belongs.
  HostRecordsMap& projectRecordsMap = GetProjectRecordsMap( thierProjectGuid );

  // Is their a record from this IP address?
  bool recordExists = projectRecordsMap.ContainsKey(theirIpAddress);

  if (recordExists)
  {
    // Update the record. reset its lifetime. update its basic host info by copying the event bundle.
    NetHostRecord* record = projectRecordsMap[theirIpAddress];
    record->mLifetime = 0;
    record->mBasicHostInfo = ZeroMove(EventBundle(basicHostInfo).GetBitStream());

    //ZPrint("Updated host record for %s.\n", theirIpAddress.GetString().c_str());

    GameSession* owner = static_cast<GameSession*>(GetOwner());
    NetHostRecordEvent newRecordEvent(record);
    owner->DispatchEvent(Events::NetHostRecordUpdate, &newRecordEvent);
  }
  else // Record did not exist.
  {
    Zero::String justIp = theirIpAddress.GetHost();

    //get the number of servers hosted for this IP currently.
    unsigned int count = mIpAddressServerCounts.FindValue(justIp, 0);

    //check if host server will exceeded maximum number of games per server, if so ignore further unique host records from that host IP address.
    if(count >= mInternetSameIpHostRecordLimit) return; 

    // increase count of same IP address servers by one for this new record
    mIpAddressServerCounts[justIp] = count + 1;
    // create a new record and Insert into records array.
    mHostRecords.PushBack(NetHostRecordPtr(new NetHostRecord()));

    // Insert copy of pointer to record into the map of guids to IP addresses to records.
    NetHostRecord* newRecord = mHostRecords.Back().mPointer;
    projectRecordsMap[theirIpAddress] = newRecord;

    // set the information of the host record.
    newRecord->mIpAddress = theirIpAddress;
    newRecord->mBasicHostInfo = ZeroMove( basicHostInfo.GetBitStream() );
    newRecord->mProjectGuid = thierProjectGuid;

    //ZPrint("Added new host record for %s.\n", theirIpAddress.GetString().c_str() );

    GameSession* owner = static_cast<GameSession*>(GetOwner());
    NetHostRecordEvent newRecordEvent(newRecord);
    owner->DispatchEvent(Events::NetHostRecordDiscovered, &newRecordEvent);

  }
}

void NetPeer::UpdateHostRecords(UpdateEvent* event)
{
	if (GetRole() != Role::MasterServer) return; // only master server updates host records.

	for (uint i = 0; i < mHostRecords.Size(); )
	{
    NetHostRecord& record = *mHostRecords[i];
		//increase life of record.
    record.mLifetime += event->RealDt;

		//if the lifetime of the record has expired.
		if (record.mLifetime > mInternetHostRecordLifetime)
		{
      //first dispatch the event.
      GameSession* owner = static_cast<GameSession*>(GetOwner());
      NetHostRecordEvent newRecordEvent(&record);
      owner->DispatchEvent(Events::NetHostRecordExpired, &newRecordEvent);

      // get map of records, and remove record from map.
      HostRecordsMap& projectRecords = GetProjectRecordsMap(record.mProjectGuid);
      projectRecords.Erase(record.mIpAddress);
      // subtract one from the total count of severs on that IP address.
      mIpAddressServerCounts[record.mIpAddress.GetHost()] -= 1;

      // Remove record from array.
			// swap the back to this position
			mHostRecords[i] = mHostRecords.Back();
			// pop it.
			mHostRecords.PopBack();

			continue; // AFTER POPPING FROM THE BACK, CONTINUE (no increment)
		}
		else
		{
			i += 1; // else keep iterating on.
		}
	}
}

void NetPeer::RemoveNetHostRecord(Guid const& projectGuid, IpAddress const & netHostRecordIp)
{
  HostRecordsMap& projectRecords = GetProjectRecordsMap(projectGuid);
  Zero::String justIp = netHostRecordIp.GetHost();
  NetHostRecord* record = projectRecords[netHostRecordIp];

  if( !projectRecords.ContainsKey(netHostRecordIp) ) return;

  // if it does, remove it from the map.
  projectRecords.Erase(netHostRecordIp);
  // subtract one from the total count of severs on that IP address.
  mIpAddressServerCounts[justIp] -= 1;
  // remove it from the array.
  NetHostRecordPtr* ptr = mHostRecords.FindPointer(record);
  mHostRecords.Erase(ptr);

}

HostRecordsMap& NetPeer::GetProjectRecordsMap(Guid const& projectGuid)
{
  // Attempt to find the recordsMap.
  HostRecordsMap* projectRecordsMap = mProjectHostRecordMaps.FindPointer(projectGuid, nullptr);

  // Create this project's record map if it doesn't exist.
  if (projectRecordsMap == nullptr)
  {
    mProjectHostRecordMaps[projectGuid] = HostRecordsMap();
    projectRecordsMap = &mProjectHostRecordMaps[projectGuid];
  }

  return *projectRecordsMap;
}


void NetPeer::OnNetPeerSentConnectResponse(NetPeerSentConnectResponse* event)
{
  if(GetRole() != Role::MasterServer) return; //Master server is the only one currently who internally sends connect responses.

  if(event->mOurConnectResponse != ConnectResponse::Accept ) return; //Master server denied them access to the host list.

  // Create a message
  NetHostRecordListData netHostRecordList;
  // copy host records into structure.
  for (unsigned int i = 0; i < mHostRecords.Size(); i += 1)
  {
    netHostRecordList.mNetHostRecords.PushBack(*(mHostRecords[i].mPointer)); //create a message to send to the client.
  }

  // create host record list message.
  Message hostRecordListMessage(NetPeerMessageType::NetHostRecordList);
  
  // write in hostRecord data. 
  hostRecordListMessage.GetData().Write(netHostRecordList);
  //Get and save the receipt with their IP address.

  PeerLink* link = GetLink(event->mTheirIpAddress);
  //we should have a link to them at this point. If we don't, something odd going on (Or maybe i do have to wait longer for link?)
  Assert(link);

  Status status;
  MessageReceiptId receiptId = link->Send(status, hostRecordListMessage, true, 0, true, 0);
  mReceiptRecipients[receiptId] = event->mTheirIpAddress;

}

void NetPeer::OnNetPeerReceivedConnectResponse(NetPeerReceivedConnectResponse* event)
{
    //don't do anything. consider dropping this event.
}


void NetPeer::ReceiveHostPublish(IpAddress const& theirIpAddress, Message& message)
{
  if( GetRole() != Role::MasterServer) return; //Only master server cares about host publishings.

  // Read network host ping message data
  NetHostPublishData netHostPublishData;
  if (!message.GetData().Read(netHostPublishData)) // Unable?
  {
    Assert(false);
  }

  //move the message data into an event bundle
  EventBundle basicHostInfo(static_cast<GameSession*>(GetOwner()));
  basicHostInfo = ZeroMove(netHostPublishData.mBasicHostInfo);

  // Get their project GUID
  Guid theirProjectGuid = netHostPublishData.mProjectGuid;

  //process this event bundle along with their project and IP address.
  MasterServerRecievePublish(theirIpAddress, basicHostInfo, theirProjectGuid);
}

bool NetPeer::IsSubscribedMasterServer(IpAddress const & ipAddress)
{
  return mMasterServerSubscriptions.Contains(ipAddress);
}

void NetPeer::OnNetPeerSentConnectRequest(NetPeerSentConnectRequest* event)
{
  //consider removing this 
}

void NetPeer::OnNetLinkConnected(NetLinkConnected* event)
{
  //consider removing this.
}

void NetPeer::OnNetLinkDisconnected(NetLinkDisconnected* event)
{
  //consider removing.
}

void NetPeer::SetInternetHostListTimeout(float internetHostListTimeout)
{
  mInternetHostListTimeout = internetHostListTimeout;
}

float NetPeer::GetInternetHostListTimeout() const
{
  return mInternetHostListTimeout;
}

void NetPeer::SetBasicHostInfoTimeout(float basicHostInfoTimeout)
{
  mBasicHostInfoTimeout = basicHostInfoTimeout;
}

float NetPeer::GetBasicHostInfoTimeout() const
{
  return mBasicHostInfoTimeout;
}

void NetPeer::SetExtraHostInfoTimeout(float extraHostInfoTimeout)
{
  mExtraHostInfoTimeout = extraHostInfoTimeout;
}

float NetPeer::GetExtraHostInfoTimeout() const
{
  return mExtraHostInfoTimeout;
}

//---------------------------------------------------------------------------------//
//                                 NetLinkData                                     //
//---------------------------------------------------------------------------------//

NetLinkData::NetLinkData()
{
}

} // namespace Zero
