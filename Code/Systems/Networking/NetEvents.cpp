// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
//
// NetHost Events
//

// Host Info:
DefineEvent(AcquireBasicNetHostInfo);
DefineEvent(AcquireExtraNetHostInfo);

// Host Discovery:
DefineEvent(NetHostDiscovered);
DefineEvent(NetHostListDiscovered);
DefineEvent(NetHostRefreshed);
DefineEvent(NetHostListRefreshed);

//
// NetPeer Events
//

// Peer Status:
DefineEvent(NetPeerOpened);
DefineEvent(NetPeerClosed);

// Game Scope:
DefineEvent(NetGameOnline);
DefineEvent(NetGameOffline);

// Game State:
DefineEvent(NetGameStarted);

//
// NetLink Events
//

// Link Handshake Sequence:
DefineEvent(NetPeerSentConnectRequest);
DefineEvent(NetPeerReceivedConnectRequest);
DefineEvent(NetPeerSentConnectResponse);
DefineEvent(NetPeerReceivedConnectResponse);

// Link Status:
DefineEvent(NetLinkConnected);
DefineEvent(NetLinkDisconnected);

//
// NetSpace Events
//

// Space Scope:
DefineEvent(NetSpaceOnline);
DefineEvent(NetSpaceOffline);

// Level State:
DefineEvent(NetLevelStarted);

//
// NetUser Events
//

// User Add Handshake Sequence:
DefineEvent(NetPeerSentUserAddRequest);
DefineEvent(NetPeerReceivedUserAddRequest);
DefineEvent(NetPeerSentUserAddResponse);
DefineEvent(NetPeerReceivedUserAddResponse);

// User Scope:
DefineEvent(NetUserOnline);
DefineEvent(NetUserOffline);

// Network Ownership:
DefineEvent(NetUserLostObjectOwnership);
DefineEvent(NetUserAcquiredObjectOwnership);

//
// NetObject Events
//

// Object Initialization:
DefineEvent(RegisterCppNetProperties);

// Object Scope:
DefineEvent(NetObjectOnline);
DefineEvent(NetObjectOffline);

// Network Ownership:
DefineEvent(NetUserOwnerChanged);

// Network Channel Property Change:
DefineEvent(NetChannelOutgoingPropertyInitialized);
DefineEvent(NetChannelIncomingPropertyInitialized);
DefineEvent(NetChannelOutgoingPropertyUninitialized);
DefineEvent(NetChannelIncomingPropertyUninitialized);
DefineEvent(NetChannelOutgoingPropertyChanged);
DefineEvent(NetChannelIncomingPropertyChanged);

//
// NetEvent Events
//

// Event Handling:
DefineEvent(NetEventSent);
DefineEvent(NetEventReceived);

//
// Master Server
//

DefineEvent(NetHostRecordDiscovered);
DefineEvent(NetHostRecordUpdate);
DefineEvent(NetHostRecordExpired);
} // namespace Events

void BindNetEvents(LibraryBuilder& builder, BoundType* type)
{
  //
  // NetHost Events
  //

  // Host Info:
  RaverieBindEvent(Events::AcquireBasicNetHostInfo, AcquireNetHostInfo);
  RaverieBindEvent(Events::AcquireExtraNetHostInfo, AcquireNetHostInfo);

  // Host Discovery:
  RaverieBindEvent(Events::NetHostDiscovered, NetHostUpdate);
  RaverieBindEvent(Events::NetHostListDiscovered, NetHostListUpdate);
  RaverieBindEvent(Events::NetHostRefreshed, NetHostUpdate);
  RaverieBindEvent(Events::NetHostListRefreshed, NetHostListUpdate);

  //
  // NetPeer Events
  //

  // Peer Status:
  RaverieBindEvent(Events::NetPeerOpened, NetPeerOpened);
  RaverieBindEvent(Events::NetPeerClosed, NetPeerClosed);

  // Game Scope:
  RaverieBindEvent(Events::NetGameOnline, NetObjectOnline);
  RaverieBindEvent(Events::NetGameOffline, NetObjectOffline);

  // Game State:
  RaverieBindEvent(Events::NetGameStarted, NetGameStarted);

  //
  // NetLink Events
  //

  // Link Handshake Sequence:
  RaverieBindEvent(Events::NetPeerSentConnectRequest, NetPeerSentConnectRequest);
  RaverieBindEvent(Events::NetPeerReceivedConnectRequest, NetPeerReceivedConnectRequest);
  RaverieBindEvent(Events::NetPeerSentConnectResponse, NetPeerSentConnectResponse);
  RaverieBindEvent(Events::NetPeerReceivedConnectResponse, NetPeerReceivedConnectResponse);

  // Link Status:
  RaverieBindEvent(Events::NetLinkConnected, NetLinkConnected);
  RaverieBindEvent(Events::NetLinkDisconnected, NetLinkDisconnected);

  //
  // NetSpace Events
  //

  // Space Scope:
  RaverieBindEvent(Events::NetSpaceOnline, NetObjectOnline);
  RaverieBindEvent(Events::NetSpaceOffline, NetObjectOffline);

  // Level State:
  RaverieBindEvent(Events::NetLevelStarted, NetLevelStarted);

  //
  // NetUser Events
  //

  // User Add Handshake Sequence:
  RaverieBindEvent(Events::NetPeerSentUserAddRequest, NetPeerSentUserAddRequest);
  RaverieBindEvent(Events::NetPeerReceivedUserAddRequest, NetPeerReceivedUserAddRequest);
  RaverieBindEvent(Events::NetPeerSentUserAddResponse, NetPeerSentUserAddResponse);
  RaverieBindEvent(Events::NetPeerReceivedUserAddResponse, NetPeerReceivedUserAddResponse);

  // User Scope:
  RaverieBindEvent(Events::NetUserOnline, NetObjectOnline);
  RaverieBindEvent(Events::NetUserOffline, NetObjectOffline);

  // Network Ownership:
  RaverieBindEvent(Events::NetUserLostObjectOwnership, NetUserLostObjectOwnership);
  RaverieBindEvent(Events::NetUserAcquiredObjectOwnership, NetUserAcquiredObjectOwnership);

  //
  // NetObject Events
  //

  // Object Scope:
  RaverieBindEvent(Events::NetObjectOnline, NetObjectOnline);
  RaverieBindEvent(Events::NetObjectOffline, NetObjectOffline);

  // Network Ownership:
  RaverieBindEvent(Events::NetUserOwnerChanged, NetUserOwnerChanged);

  // Network Channel Property Change:
  RaverieBindEvent(Events::NetChannelOutgoingPropertyInitialized, NetChannelPropertyChange);
  RaverieBindEvent(Events::NetChannelIncomingPropertyInitialized, NetChannelPropertyChange);
  RaverieBindEvent(Events::NetChannelOutgoingPropertyUninitialized, NetChannelPropertyChange);
  RaverieBindEvent(Events::NetChannelIncomingPropertyUninitialized, NetChannelPropertyChange);
  RaverieBindEvent(Events::NetChannelOutgoingPropertyChanged, NetChannelPropertyChange);
  RaverieBindEvent(Events::NetChannelIncomingPropertyChanged, NetChannelPropertyChange);

  //
  // NetEvent Events
  //

  // Event Handling:
  RaverieBindEvent(Events::NetEventSent, NetEventSent);
  RaverieBindEvent(Events::NetEventReceived, NetEventReceived);

  //
  // MasterServer Events
  //

  RaverieBindEvent(Events::NetHostRecordDiscovered, NetHostRecordEvent);
  RaverieBindEvent(Events::NetHostRecordUpdate, NetHostRecordEvent);
  RaverieBindEvent(Events::NetHostRecordExpired, NetHostRecordEvent);
}

//                                  NetRequest //

NetRequest::NetRequest(NetRequestType::Enum netRequestType, const IpAddress& ipAddress, const EventBundle& requestBundle) :
    mNetRequestType(netRequestType), mTheirIpAddress(ipAddress), mOurRequestBundle(requestBundle)
{
}

//                                NetHost Events //

/////////////////
//  Host Info  //
/////////////////

//                              AcquireNetHostInfo //

RaverieDefineType(AcquireNetHostInfo, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldProperty(mReturnHostInfo);
}

AcquireNetHostInfo::AcquireNetHostInfo(GameSession* gameSession) : mReturnHostInfo(gameSession)
{
}

//////////////////////
//  Host Discovery  //
//////////////////////

//                                 NetHostUpdate //

RaverieDefineType(NetHostUpdate, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mRefreshResult);
  RaverieBindFieldGetterProperty(mResponseTime);
  RaverieBindFieldGetterProperty(mNetwork);
  RaverieBindFieldGetterProperty(mHost);
}

NetHostUpdate::NetHostUpdate() : mRefreshResult(NetRefreshResult::NoResponse), mResponseTime(0), mNetwork(Network::LAN), mHost(nullptr)
{
}

//                                 NetHostListUpdate //

RaverieDefineType(NetHostListUpdate, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mNetwork);
}

NetHostListUpdate::NetHostListUpdate() : mNetwork(Network::LAN)
{
}

//                                NetPeer Events //

////////////////
// Peer Scope //
////////////////

//                                NetPeerOpened //

RaverieDefineType(NetPeerOpened, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();
}

//                                 NetPeerClosed //

RaverieDefineType(NetPeerClosed, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();
}

////////////////
// Game State //
////////////////

//                                 NetGameStarted //

RaverieDefineType(NetGameStarted, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mGameSession);
}

NetGameStarted::NetGameStarted() : mGameSession(nullptr)
{
}

//                                NetLink Events //

/////////////////////////////
// Link Handshake Sequence //
/////////////////////////////

//                           NetPeerSentConnectRequest //

RaverieDefineType(NetPeerSentConnectRequest, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mOurRequestBundle);
  RaverieBindFieldGetterProperty(mOurPendingUserAddRequestCount);
}

NetPeerSentConnectRequest::NetPeerSentConnectRequest(GameSession* gameSession) : mTheirIpAddress(), mOurRequestBundle(gameSession), mOurPendingUserAddRequestCount(0)
{
}

//                         NetPeerReceivedConnectRequest //

RaverieDefineType(NetPeerReceivedConnectRequest, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mTheirRequestBundle);
  RaverieBindFieldGetterProperty(mTheirPendingUserAddRequestCount);
  RaverieBindFieldGetterProperty(mOurIpAddress);
  RaverieBindFieldProperty(mReturnOurConnectResponse);
  RaverieBindFieldProperty(mReturnOurResponseBundle);
}

NetPeerReceivedConnectRequest::NetPeerReceivedConnectRequest(GameSession* gameSession) :
    mTheirIpAddress(), mTheirRequestBundle(gameSession), mTheirPendingUserAddRequestCount(0), mOurIpAddress(), mReturnOurConnectResponse(false), mReturnOurResponseBundle(gameSession)
{
}

//                           NetPeerSentConnectResponse //

RaverieDefineType(NetPeerSentConnectResponse, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mTheirRequestBundle);
  RaverieBindFieldGetterProperty(mTheirPendingUserAddRequestCount);
  RaverieBindFieldGetterProperty(mOurIpAddress);
  RaverieBindFieldGetterProperty(mOurConnectResponse);
  RaverieBindFieldGetterProperty(mOurResponseBundle);
}

NetPeerSentConnectResponse::NetPeerSentConnectResponse(GameSession* gameSession) :
    mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mTheirPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mOurConnectResponse(ConnectResponse::Deny),
    mOurResponseBundle(gameSession)
{
}

//                        NetPeerReceivedConnectResponse //

RaverieDefineType(NetPeerReceivedConnectResponse, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mOurRequestBundle);
  RaverieBindFieldGetterProperty(mOurPendingUserAddRequestCount);
  RaverieBindFieldGetterProperty(mOurIpAddress);
  RaverieBindFieldGetterProperty(mTheirConnectResponse);
  RaverieBindFieldGetterProperty(mTheirResponseBundle);
  RaverieBindFieldGetterProperty(mOurNetPeerId);
}

NetPeerReceivedConnectResponse::NetPeerReceivedConnectResponse(GameSession* gameSession) :
    mTheirIpAddress(),
    mOurRequestBundle(gameSession),
    mOurPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mTheirConnectResponse(ConnectResponse::Deny),
    mTheirResponseBundle(gameSession),
    mOurNetPeerId(0)
{
}

////////////////
// Link Scope //
////////////////

//                                NetLinkConnected //

RaverieDefineType(NetLinkConnected, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mDirection);
}

NetLinkConnected::NetLinkConnected() : mTheirNetPeerId(0), mTheirIpAddress(), mDirection(TransmissionDirection::Unspecified)
{
}

//                              NetLinkDisconnected //

RaverieDefineType(NetLinkDisconnected, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mDisconnectReason);
  RaverieBindFieldGetterProperty(mRequestBundle);
  RaverieBindFieldGetterProperty(mDirection);
}

NetLinkDisconnected::NetLinkDisconnected(GameSession* gameSession) :
    mTheirNetPeerId(0), mTheirIpAddress(), mDisconnectReason(DisconnectReason::Request), mRequestBundle(gameSession), mDirection(TransmissionDirection::Unspecified)
{
}

//                               NetSpace Events //

/////////////////
// Level State //
/////////////////

//                                NetLevelStarted //

RaverieDefineType(NetLevelStarted, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mGameSession);
  RaverieBindFieldGetterProperty(mSpace);
  RaverieBindFieldGetterProperty(mLevelName);
}

NetLevelStarted::NetLevelStarted() : mGameSession(nullptr), mSpace(nullptr), mLevelName()
{
}

//                                NetUser Events //

///////////////////////////////////
//  User Add Handshake Sequence  //
///////////////////////////////////

//                          NetPeerSentUserAddRequest //

RaverieDefineType(NetPeerSentUserAddRequest, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mOurRequestBundle);
}

NetPeerSentUserAddRequest::NetPeerSentUserAddRequest(GameSession* gameSession) : mTheirNetPeerId(0), mTheirIpAddress(), mOurRequestBundle(gameSession)
{
}

//                        NetPeerReceivedUserAddRequest //

RaverieDefineType(NetPeerReceivedUserAddRequest, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mTheirRequestBundle);
  RaverieBindFieldProperty(mReturnOurAddResponse);
  RaverieBindFieldProperty(mReturnOurResponseBundle);
  RaverieBindFieldProperty(mReturnTheirNetUser);
  RaverieBindFieldGetterProperty(mTheirNetUserId);
}

NetPeerReceivedUserAddRequest::NetPeerReceivedUserAddRequest(GameSession* gameSession) :
    mTheirNetPeerId(0), mTheirIpAddress(), mTheirRequestBundle(gameSession), mReturnOurAddResponse(false), mReturnOurResponseBundle(gameSession), mReturnTheirNetUser(nullptr), mTheirNetUserId(0)
{
}

//                          NetPeerSentUserAddResponse //

RaverieDefineType(NetPeerSentUserAddResponse, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mTheirRequestBundle);
  RaverieBindFieldGetterProperty(mOurAddResponse);
  RaverieBindFieldGetterProperty(mOurResponseBundle);
  RaverieBindFieldGetterProperty(mTheirNetUserId);
  RaverieBindFieldGetterProperty(mTheirNetUser);
}

/// Constructor.
NetPeerSentUserAddResponse::NetPeerSentUserAddResponse(GameSession* gameSession) :
    mTheirNetPeerId(0), mTheirIpAddress(), mTheirRequestBundle(gameSession), mOurAddResponse(NetUserAddResponse::Deny), mOurResponseBundle(gameSession), mTheirNetUserId(0), mTheirNetUser(nullptr)
{
}

//                        NetPeerReceivedUserAddResponse //

RaverieDefineType(NetPeerReceivedUserAddResponse, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mTheirIpAddress);
  RaverieBindFieldGetterProperty(mOurRequestBundle);
  RaverieBindFieldGetterProperty(mTheirAddResponse);
  RaverieBindFieldGetterProperty(mTheirResponseBundle);
  RaverieBindFieldGetterProperty(mOurNetUserId);
}

NetPeerReceivedUserAddResponse::NetPeerReceivedUserAddResponse(GameSession* gameSession) :
    mTheirNetPeerId(0), mTheirIpAddress(), mOurRequestBundle(gameSession), mTheirAddResponse(NetUserAddResponse::Deny), mTheirResponseBundle(gameSession), mOurNetUserId(0)
{
}

///////////////////////
// Network Ownership //
///////////////////////

//                          NetUserLostObjectOwnership //

RaverieDefineType(NetUserLostObjectOwnership, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mLostObject);
  RaverieBindFieldGetterProperty(mCurrentNetUserOwner);
}

//                        NetUserAcquiredObjectOwnership //

RaverieDefineType(NetUserAcquiredObjectOwnership, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mAcquiredObject);
  RaverieBindFieldGetterProperty(mPreviousNetUserOwner);
}

//                               NetObject Events //

///////////////////////////
// Object Initialization //
///////////////////////////

//                           RegisterCppNetProperties //

RaverieDefineType(RegisterCppNetProperties, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();
}

//////////////////
// Object Scope //
//////////////////

//                                 NetObjectOnline //

RaverieDefineType(NetObjectOnline, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mGameSession);
  RaverieBindFieldGetterProperty(mSpace);
  RaverieBindFieldGetterProperty(mObject);
  RaverieBindFieldGetterProperty(mIsStartOfLifespan);
}

//                               NetObjectOffline //

RaverieDefineType(NetObjectOffline, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mGameSession);
  RaverieBindFieldGetterProperty(mSpace);
  RaverieBindFieldGetterProperty(mObject);
  RaverieBindFieldGetterProperty(mIsEndOfLifespan);
}

///////////////////////
// Network Ownership //
///////////////////////

//                             NetUserOwnerChanged //

RaverieDefineType(NetUserOwnerChanged, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mPreviousNetUserOwner);
  RaverieBindFieldGetterProperty(mCurrentNetUserOwner);
}

/////////////////////////////////////
// Network Channel Property Change //
/////////////////////////////////////

//                           NetChannelPropertyChange //

RaverieDefineType(NetChannelPropertyChange, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTimestamp);
  RaverieBindFieldGetterProperty(mReplicationPhase);
  RaverieBindFieldGetterProperty(mDirection);
  RaverieBindFieldGetterProperty(mObject);
  RaverieBindFieldGetterProperty(mChannelName);
  RaverieBindFieldGetterProperty(mComponentName);
  RaverieBindFieldGetterProperty(mPropertyName);
}

//                                NetEvent Events //

////////////////////
// Event Handling //
////////////////////

//                                 NetEventSent //

RaverieDefineType(NetEventSent, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mNetEvent);
  RaverieBindFieldGetterProperty(mDestination);
}

//                               NetEventReceived //

RaverieDefineType(NetEventReceived, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mTheirNetPeerId);
  RaverieBindFieldGetterProperty(mNetEvent);
  RaverieBindFieldGetterProperty(mDestination);
  RaverieBindFieldProperty(mReturnAllow);
}

//                                 NetHostUpdate //

RaverieDefineType(NetHostRecordEvent, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind properties
  RaverieBindFieldGetterProperty(mHostRecord);
}

NetHostRecordEvent::NetHostRecordEvent() : mHostRecord(nullptr)
{
}

NetHostRecordEvent::NetHostRecordEvent(NetHostRecord* netHostRecord) : mHostRecord(netHostRecord)
{
}

} // namespace Raverie
