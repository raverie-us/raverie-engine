///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
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
}

void BindNetEvents(LibraryBuilder& builder, BoundType* type)
{
  //
  // NetHost Events
  //

  // Host Info:
  ZeroBindEvent(Events::AcquireBasicNetHostInfo, AcquireNetHostInfo);
  ZeroBindEvent(Events::AcquireExtraNetHostInfo, AcquireNetHostInfo);

  // Host Discovery:
  ZeroBindEvent(Events::NetHostDiscovered,     NetHostUpdate);
  ZeroBindEvent(Events::NetHostListDiscovered, NetHostListUpdate);
  ZeroBindEvent(Events::NetHostRefreshed,      NetHostUpdate);
  ZeroBindEvent(Events::NetHostListRefreshed,  NetHostListUpdate);

  //
  // NetPeer Events
  //

  // Peer Status:
  ZeroBindEvent(Events::NetPeerOpened, NetPeerOpened);
  ZeroBindEvent(Events::NetPeerClosed, NetPeerClosed);

  // Game Scope:
  ZeroBindEvent(Events::NetGameOnline,  NetObjectOnline);
  ZeroBindEvent(Events::NetGameOffline, NetObjectOffline);

  // Game State:
  ZeroBindEvent(Events::NetGameStarted, NetGameStarted);

  //
  // NetLink Events
  //

  // Link Handshake Sequence:
  ZeroBindEvent(Events::NetPeerSentConnectRequest,      NetPeerSentConnectRequest);
  ZeroBindEvent(Events::NetPeerReceivedConnectRequest,  NetPeerReceivedConnectRequest);
  ZeroBindEvent(Events::NetPeerSentConnectResponse,     NetPeerSentConnectResponse);
  ZeroBindEvent(Events::NetPeerReceivedConnectResponse, NetPeerReceivedConnectResponse);

  // Link Status:
  ZeroBindEvent(Events::NetLinkConnected,    NetLinkConnected);
  ZeroBindEvent(Events::NetLinkDisconnected, NetLinkDisconnected);

  //
  // NetSpace Events
  //

  // Space Scope:
  ZeroBindEvent(Events::NetSpaceOnline,  NetObjectOnline);
  ZeroBindEvent(Events::NetSpaceOffline, NetObjectOffline);

  // Level State:
  ZeroBindEvent(Events::NetLevelStarted, NetLevelStarted);

  //
  // NetUser Events
  //

  // User Add Handshake Sequence:
  ZeroBindEvent(Events::NetPeerSentUserAddRequest,      NetPeerSentUserAddRequest);
  ZeroBindEvent(Events::NetPeerReceivedUserAddRequest,  NetPeerReceivedUserAddRequest);
  ZeroBindEvent(Events::NetPeerSentUserAddResponse,     NetPeerSentUserAddResponse);
  ZeroBindEvent(Events::NetPeerReceivedUserAddResponse, NetPeerReceivedUserAddResponse);

  // User Scope:
  ZeroBindEvent(Events::NetUserOnline,  NetObjectOnline);
  ZeroBindEvent(Events::NetUserOffline, NetObjectOffline);

  // Network Ownership:
  ZeroBindEvent(Events::NetUserLostObjectOwnership,     NetUserLostObjectOwnership);
  ZeroBindEvent(Events::NetUserAcquiredObjectOwnership, NetUserAcquiredObjectOwnership);

  //
  // NetObject Events
  //

  // Object Scope:
  ZeroBindEvent(Events::NetObjectOnline,  NetObjectOnline);
  ZeroBindEvent(Events::NetObjectOffline, NetObjectOffline);

  // Network Ownership:
  ZeroBindEvent(Events::NetUserOwnerChanged, NetUserOwnerChanged);

  // Network Channel Property Change:
  ZeroBindEvent(Events::NetChannelOutgoingPropertyInitialized,   NetChannelPropertyChange);
  ZeroBindEvent(Events::NetChannelIncomingPropertyInitialized,   NetChannelPropertyChange);
  ZeroBindEvent(Events::NetChannelOutgoingPropertyUninitialized, NetChannelPropertyChange);
  ZeroBindEvent(Events::NetChannelIncomingPropertyUninitialized, NetChannelPropertyChange);
  ZeroBindEvent(Events::NetChannelOutgoingPropertyChanged,       NetChannelPropertyChange);
  ZeroBindEvent(Events::NetChannelIncomingPropertyChanged,       NetChannelPropertyChange);

  //
  // NetEvent Events
  //

  // Event Handling:
  ZeroBindEvent(Events::NetEventSent,     NetEventSent);
  ZeroBindEvent(Events::NetEventReceived, NetEventReceived);

  //
  // MasterServer Events
  //

  ZeroBindEvent(Events::NetHostRecordDiscovered, NetHostRecordEvent);
  ZeroBindEvent(Events::NetHostRecordUpdate,     NetHostRecordEvent);
  ZeroBindEvent(Events::NetHostRecordExpired,    NetHostRecordEvent);
}

//---------------------------------------------------------------------------------//
//                                  NetRequest                                     //
//---------------------------------------------------------------------------------//

NetRequest::NetRequest(NetRequestType::Enum netRequestType, const IpAddress& ipAddress, const EventBundle& requestBundle)
  : mNetRequestType(netRequestType),
    mTheirIpAddress(ipAddress),
    mOurRequestBundle(requestBundle)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                                NetHost Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

/////////////////
//  Host Info  //
/////////////////

//---------------------------------------------------------------------------------//
//                              AcquireNetHostInfo                                 //
//---------------------------------------------------------------------------------//

ZilchDefineType(AcquireNetHostInfo, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldProperty(mReturnHostInfo);
}

AcquireNetHostInfo::AcquireNetHostInfo(GameSession* gameSession)
  : mReturnHostInfo(gameSession)
{
}

//////////////////////
//  Host Discovery  //
//////////////////////

//---------------------------------------------------------------------------------//
//                                 NetHostUpdate                                   //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetHostUpdate, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mRefreshResult);
  ZilchBindFieldGetterProperty(mResponseTime);
  ZilchBindFieldGetterProperty(mNetwork);
  ZilchBindFieldGetterProperty(mHost);
}

NetHostUpdate::NetHostUpdate()
  : mRefreshResult(NetRefreshResult::NoResponse),
    mResponseTime(0),
    mNetwork(Network::LAN),
    mHost(nullptr)
{
}

//---------------------------------------------------------------------------------//
//                                 NetHostListUpdate                               //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetHostListUpdate, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mNetwork);
}

NetHostListUpdate::NetHostListUpdate()
  : mNetwork(Network::LAN)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                                NetPeer Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

////////////////
// Peer Scope //
////////////////

//---------------------------------------------------------------------------------//
//                                NetPeerOpened                                    //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerOpened, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();
}

//---------------------------------------------------------------------------------//
//                                 NetPeerClosed                                   //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerClosed, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();
}

////////////////
// Game State //
////////////////

//---------------------------------------------------------------------------------//
//                                 NetGameStarted                                  //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetGameStarted, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mGameSession);
}

NetGameStarted::NetGameStarted()
  : mGameSession(nullptr)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                                NetLink Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////
// Link Handshake Sequence //
/////////////////////////////

//---------------------------------------------------------------------------------//
//                           NetPeerSentConnectRequest                             //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerSentConnectRequest, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mOurRequestBundle);
  ZilchBindFieldGetterProperty(mOurPendingUserAddRequestCount);
}

NetPeerSentConnectRequest::NetPeerSentConnectRequest(GameSession* gameSession)
  : mTheirIpAddress(),
    mOurRequestBundle(gameSession),
    mOurPendingUserAddRequestCount(0)
{
}

//---------------------------------------------------------------------------------//
//                         NetPeerReceivedConnectRequest                           //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerReceivedConnectRequest, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mTheirRequestBundle);
  ZilchBindFieldGetterProperty(mTheirPendingUserAddRequestCount);
  ZilchBindFieldGetterProperty(mOurIpAddress);
  ZilchBindFieldProperty(mReturnOurConnectResponse);
  ZilchBindFieldProperty(mReturnOurResponseBundle);
}

NetPeerReceivedConnectRequest::NetPeerReceivedConnectRequest(GameSession* gameSession)
  : mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mTheirPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mReturnOurConnectResponse(false),
    mReturnOurResponseBundle(gameSession)
{
}

//---------------------------------------------------------------------------------//
//                           NetPeerSentConnectResponse                            //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerSentConnectResponse, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mTheirRequestBundle);
  ZilchBindFieldGetterProperty(mTheirPendingUserAddRequestCount);
  ZilchBindFieldGetterProperty(mOurIpAddress);
  ZilchBindFieldGetterProperty(mOurConnectResponse);
  ZilchBindFieldGetterProperty(mOurResponseBundle);
}

NetPeerSentConnectResponse::NetPeerSentConnectResponse(GameSession* gameSession)
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mTheirPendingUserAddRequestCount(0),
    mOurIpAddress(),
    mOurConnectResponse(ConnectResponse::Deny),
    mOurResponseBundle(gameSession)
{
}

//---------------------------------------------------------------------------------//
//                        NetPeerReceivedConnectResponse                           //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerReceivedConnectResponse, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mOurRequestBundle);
  ZilchBindFieldGetterProperty(mOurPendingUserAddRequestCount);
  ZilchBindFieldGetterProperty(mOurIpAddress);
  ZilchBindFieldGetterProperty(mTheirConnectResponse);
  ZilchBindFieldGetterProperty(mTheirResponseBundle);
  ZilchBindFieldGetterProperty(mOurNetPeerId);
}

NetPeerReceivedConnectResponse::NetPeerReceivedConnectResponse(GameSession* gameSession)
  : mTheirIpAddress(),
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

//---------------------------------------------------------------------------------//
//                                NetLinkConnected                                 //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetLinkConnected, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mDirection);
}

NetLinkConnected::NetLinkConnected()
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mDirection(TransmissionDirection::Unspecified)
{
}

//---------------------------------------------------------------------------------//
//                              NetLinkDisconnected                                //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetLinkDisconnected, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mDisconnectReason);
  ZilchBindFieldGetterProperty(mRequestBundle);
  ZilchBindFieldGetterProperty(mDirection);
}

NetLinkDisconnected::NetLinkDisconnected(GameSession* gameSession)
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mDisconnectReason(DisconnectReason::Request),
    mRequestBundle(gameSession),
    mDirection(TransmissionDirection::Unspecified)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                               NetSpace Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

/////////////////
// Level State //
/////////////////

//---------------------------------------------------------------------------------//
//                                NetLevelStarted                                  //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetLevelStarted, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mGameSession);
  ZilchBindFieldGetterProperty(mSpace);
  ZilchBindFieldGetterProperty(mLevelName);
}

NetLevelStarted::NetLevelStarted()
  : mGameSession(nullptr),
    mSpace(nullptr),
    mLevelName()
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                                NetUser Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////
//  User Add Handshake Sequence  //
///////////////////////////////////

//---------------------------------------------------------------------------------//
//                          NetPeerSentUserAddRequest                              //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerSentUserAddRequest, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mOurRequestBundle);
}

NetPeerSentUserAddRequest::NetPeerSentUserAddRequest(GameSession* gameSession)
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mOurRequestBundle(gameSession)
{
}

//---------------------------------------------------------------------------------//
//                        NetPeerReceivedUserAddRequest                            //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerReceivedUserAddRequest, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mTheirRequestBundle);
  ZilchBindFieldProperty(mReturnOurAddResponse);
  ZilchBindFieldProperty(mReturnOurResponseBundle);
  ZilchBindFieldProperty(mReturnTheirNetUser);
  ZilchBindFieldGetterProperty(mTheirNetUserId);
}

NetPeerReceivedUserAddRequest::NetPeerReceivedUserAddRequest(GameSession* gameSession)
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mReturnOurAddResponse(false),
    mReturnOurResponseBundle(gameSession),
    mReturnTheirNetUser(nullptr),
    mTheirNetUserId(0)
{
}

//---------------------------------------------------------------------------------//
//                          NetPeerSentUserAddResponse                             //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerSentUserAddResponse, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mTheirRequestBundle);
  ZilchBindFieldGetterProperty(mOurAddResponse);
  ZilchBindFieldGetterProperty(mOurResponseBundle);
  ZilchBindFieldGetterProperty(mTheirNetUserId);
  ZilchBindFieldGetterProperty(mTheirNetUser);
}

/// Constructor.
NetPeerSentUserAddResponse::NetPeerSentUserAddResponse(GameSession* gameSession)
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mTheirRequestBundle(gameSession),
    mOurAddResponse(NetUserAddResponse::Deny),
    mOurResponseBundle(gameSession),
    mTheirNetUserId(0),
    mTheirNetUser(nullptr)
{
}

//---------------------------------------------------------------------------------//
//                        NetPeerReceivedUserAddResponse                           //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPeerReceivedUserAddResponse, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mTheirIpAddress);
  ZilchBindFieldGetterProperty(mOurRequestBundle);
  ZilchBindFieldGetterProperty(mTheirAddResponse);
  ZilchBindFieldGetterProperty(mTheirResponseBundle);
  ZilchBindFieldGetterProperty(mOurNetUserId);
}

NetPeerReceivedUserAddResponse::NetPeerReceivedUserAddResponse(GameSession* gameSession)
  : mTheirNetPeerId(0),
    mTheirIpAddress(),
    mOurRequestBundle(gameSession),
    mTheirAddResponse(NetUserAddResponse::Deny),
    mTheirResponseBundle(gameSession),
    mOurNetUserId(0)
{
}

///////////////////////
// Network Ownership //
///////////////////////

//---------------------------------------------------------------------------------//
//                          NetUserLostObjectOwnership                             //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetUserLostObjectOwnership, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mLostObject);
  ZilchBindFieldGetterProperty(mCurrentNetUserOwner);
}

//---------------------------------------------------------------------------------//
//                        NetUserAcquiredObjectOwnership                           //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetUserAcquiredObjectOwnership, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mAcquiredObject);
  ZilchBindFieldGetterProperty(mPreviousNetUserOwner);
}

/////////////////////////////////////////////////////////////////////////////////////
//                               NetObject Events                                  //
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////
// Object Initialization //
///////////////////////////

//---------------------------------------------------------------------------------//
//                           RegisterCppNetProperties                              //
//---------------------------------------------------------------------------------//

ZilchDefineType(RegisterCppNetProperties, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();
}

//////////////////
// Object Scope //
//////////////////

//---------------------------------------------------------------------------------//
//                                 NetObjectOnline                                 //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetObjectOnline, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mGameSession);
  ZilchBindFieldGetterProperty(mSpace);
  ZilchBindFieldGetterProperty(mObject);
  ZilchBindFieldGetterProperty(mIsStartOfLifespan);
}

//---------------------------------------------------------------------------------//
//                               NetObjectOffline                                  //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetObjectOffline, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mGameSession);
  ZilchBindFieldGetterProperty(mSpace);
  ZilchBindFieldGetterProperty(mObject);
  ZilchBindFieldGetterProperty(mIsEndOfLifespan);
}

///////////////////////
// Network Ownership //
///////////////////////

//---------------------------------------------------------------------------------//
//                             NetUserOwnerChanged                                 //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetUserOwnerChanged, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mPreviousNetUserOwner);
  ZilchBindFieldGetterProperty(mCurrentNetUserOwner);
}

/////////////////////////////////////
// Network Channel Property Change //
/////////////////////////////////////

//---------------------------------------------------------------------------------//
//                           NetChannelPropertyChange                              //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetChannelPropertyChange, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTimestamp);
  ZilchBindFieldGetterProperty(mReplicationPhase);
  ZilchBindFieldGetterProperty(mDirection);
  ZilchBindFieldGetterProperty(mObject);
  ZilchBindFieldGetterProperty(mChannelName);
  ZilchBindFieldGetterProperty(mComponentName);
  ZilchBindFieldGetterProperty(mPropertyName);
}

/////////////////////////////////////////////////////////////////////////////////////
//                                NetEvent Events                                  //
/////////////////////////////////////////////////////////////////////////////////////

////////////////////
// Event Handling //
////////////////////

//---------------------------------------------------------------------------------//
//                                 NetEventSent                                    //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetEventSent, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mNetEvent);
  ZilchBindFieldGetterProperty(mDestination);
}

//---------------------------------------------------------------------------------//
//                               NetEventReceived                                  //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetEventReceived, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mTheirNetPeerId);
  ZilchBindFieldGetterProperty(mNetEvent);
  ZilchBindFieldGetterProperty(mDestination);
  ZilchBindFieldProperty(mReturnAllow);
}

//---------------------------------------------------------------------------------//
//                                 NetHostUpdate                                   //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetHostRecordEvent, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind properties
  ZilchBindFieldGetterProperty(mHostRecord);
}

NetHostRecordEvent::NetHostRecordEvent()
  : mHostRecord(nullptr)
{
}

NetHostRecordEvent::NetHostRecordEvent(NetHostRecord* netHostRecord)
  : mHostRecord(netHostRecord)
{
}

} // namespace Zero
