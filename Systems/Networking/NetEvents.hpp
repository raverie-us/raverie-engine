///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  //
  // NetHost Events
  //

  // Host Info:
  // [Client/Server/MasterServer] (Dispatched on GameSession)
  // Generated when the NetPeer host is acquiring project-specific host information.
  DeclareEvent(AcquireBasicNetHostInfo);
  DeclareEvent(AcquireExtraNetHostInfo);

  // Host Discovery:
  // [Client/Server/MasterServer] (Dispatched on GameSession)
  // Generated after a new NetHost is discovered.
  DeclareEvent(NetHostDiscovered);
  // Generated after a NetHost list discovery request completes or times out.
  DeclareEvent(NetHostListDiscovered);
  // Generated after a known NetHost is refreshed or the refresh request times out.
  DeclareEvent(NetHostRefreshed);
  // Generated after a NetHost list refresh request completes or times out.
  DeclareEvent(NetHostListRefreshed);

  //
  // NetPeer Events
  //

  // Peer Status:
  // [Client/Server/Offline] (Dispatched on GameSession)
  // Generated when the NetPeer is opened or closed, for the LOCAL peer only.
  DeclareEvent(NetPeerOpened);
  DeclareEvent(NetPeerClosed);

  // Game Scope:
  // [Client/Server/Offline] (Dispatched on GameSession)
  // Generated when the GameSession is brought online or taken offline, for ALL peers in the network graph.
  DeclareEvent(NetGameOnline);
  DeclareEvent(NetGameOffline);

  // Game State:
  // [Client/Server/Offline] (Dispatched on GameSession and ALL Spaces)
  // Generated after fully joining or hosting a network game, for the LOCAL peer only.
  DeclareEvent(NetGameStarted);

  //
  // NetLink Events
  //

  // Link Handshake Sequence:
  // [Client/Server] (Dispatched on GameSession)
  // Generated as a result of a NetLink connect request, for the LOCAL peer and REMOTE peer.
  DeclareEvent(NetPeerSentConnectRequest);
  DeclareEvent(NetPeerReceivedConnectRequest);
  DeclareEvent(NetPeerSentConnectResponse);
  DeclareEvent(NetPeerReceivedConnectResponse);

  // Link Status:
  // [Client/Server] (Dispatched on GameSession)
  // Generated when the NetLink is connected or disconnected, for the LOCAL peer and REMOTE peer.
  DeclareEvent(NetLinkConnected);
  DeclareEvent(NetLinkDisconnected);

  //
  // NetSpace Events
  //

  // Space Scope:
  // [Client/Server/Offline] (Dispatched on GameSession and Space)
  // Generated when the Space is brought online or taken offline, for ALL peers in the network graph.
  DeclareEvent(NetSpaceOnline);
  DeclareEvent(NetSpaceOffline);

  // Level State:
  // [Client/Server/Offline] (Dispatched on GameSession and Space)
  // Generated after fully loading and synchronizing a level in a NetSpace, for ALL RELEVANT peers in the network graph.
  DeclareEvent(NetLevelStarted);

  //
  // NetUser Events
  //

  // User Add Handshake Sequence:
  // [Client/Server/Offline] (Dispatched on GameSession)
  // Generated as a result of a NetUser add request, for the LOCAL peer and REMOTE peer.
  DeclareEvent(NetPeerSentUserAddRequest);
  DeclareEvent(NetPeerReceivedUserAddRequest);
  DeclareEvent(NetPeerSentUserAddResponse);
  DeclareEvent(NetPeerReceivedUserAddResponse);

  // User Scope:
  // [Client/Server/Offline] (Dispatched on GameSession, Space, and Cog)
  // Generated when the User is brought online or taken offline, for ALL peers in the network graph.
  DeclareEvent(NetUserOnline);
  DeclareEvent(NetUserOffline);

  // Network Ownership:
  // [Client/Server/Offline] (Dispatched on User Cog)
  // Generated as a result of changing NetObject ownership, for ALL RELEVANT peers in the network graph.
  DeclareEvent(NetUserLostObjectOwnership);
  DeclareEvent(NetUserAcquiredObjectOwnership);

  //
  // NetObject Events
  //

  // Object Initialization:
  // [Client/Server/Offline] (Dispatched on Cog)
  // Generated while adding C++ component net properties to a NetObject, for the LOCAL peer only.
  // Gives C++ components the opportunity to add their net properties to a NetObject at the appropriate time.
  DeclareEvent(RegisterCppNetProperties);

  // Object Scope:
  // [Client/Server/Offline] (Dispatched on GameSession, Space, and Cog)
  // Generated when the Object is brought online or taken offline, for ALL RELEVANT peers in the network graph.
  DeclareEvent(NetObjectOnline);
  DeclareEvent(NetObjectOffline);

  // Network Ownership:
  // [Client/Server/Offline] (Dispatched on Object Cog)
  // Generated as a result of changing NetObject ownership, for ALL RELEVANT peers in the network graph.
  DeclareEvent(NetUserOwnerChanged);

  // Network Channel Property Change:
  // [Client/Server] (Dispatched on Cog)
  // Generated after an outgoing/incoming net property change is detected, for ALL RELEVANT peers in the network graph.
  DeclareEvent(NetChannelOutgoingPropertyInitialized);
  DeclareEvent(NetChannelIncomingPropertyInitialized);
  DeclareEvent(NetChannelOutgoingPropertyUninitialized);
  DeclareEvent(NetChannelIncomingPropertyUninitialized);
  DeclareEvent(NetChannelOutgoingPropertyChanged);
  DeclareEvent(NetChannelIncomingPropertyChanged);

  // Master Server Records:
  // [MasterServer] (Dispatched on GameSession)
  // Generated after a new NetHostRecord is discovered.
  DeclareEvent(NetHostRecordDiscovered);
  // Generated after an already discovered NetHostRecord is updated from receiving new information.
  DeclareEvent(NetHostRecordUpdate);
  // Generated after a NetHostRecord's lifetime has exceeded the max record lifetime.
  DeclareEvent(NetHostRecordExpired);

  //
  // NetEvent Events
  //

  // Event Handling:
  // [Client/Server/Offline] (Dispatched on GameSession)
  // Generated as a result of sending/receiving Events over the network, for the LOCAL peer and REMOTE peer(s).
  DeclareEvent(NetEventSent);
  DeclareEvent(NetEventReceived);
}

/// Binds all Net API events.
void BindNetEvents(LibraryBuilder& builder, BoundType* type);

//---------------------------------------------------------------------------------//
//                                  NetRequest                                     //
//---------------------------------------------------------------------------------//

// NetPeer protocol request type
DeclareEnum3(NetRequestType,
  Unspecified, /// Unspecified request.
  Connect,     /// Connect link request.
  AddUser);    ///< Add user request.

/// Contains a pending network request (used internally to buffer requests).
class NetRequest
{
public:
  /// Constructors.
  NetRequest(NetRequestType::Enum netRequestType = NetRequestType::Unspecified,
             const IpAddress& ipAddress = IpAddress(),
             const EventBundle& requestBundle = EventBundle());

  // Data
  NetRequestType::Enum mNetRequestType;   ///< Network request type.
  IpAddress            mTheirIpAddress;   ///< Their IP address (as seen from our perspective).
  EventBundle          mOurRequestBundle; ///< Our bundled request event data.
};

/////////////////////////////////////////////////////////////////////////////////////
//                                NetHost Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

/////////////////
//  Host Info  //
/////////////////

//---------------------------------------------------------------------------------//
//                              AcquireNetHostInfo                                 //
//---------------------------------------------------------------------------------//

/// Dispatched when the net peer host is acquiring project-specific host information.
class AcquireNetHostInfo : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  AcquireNetHostInfo(GameSession* gameSession);

  // Data
  EventBundle mReturnHostInfo; ///< Return: Our bundled host info event data.
};

//////////////////////
//  Host Discovery  //
//////////////////////

//---------------------------------------------------------------------------------//
//                                 NetHostUpdate                                   //
//---------------------------------------------------------------------------------//

/// Dispatched when a host discovery operation update occurs.
class NetHostUpdate : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetHostUpdate();

  // Data
  NetRefreshResult::Enum mRefreshResult; ///< Whether or not the operation completed successfully.
  float                  mResponseTime;  ///< Operation response time (from request to completion).
  Network::Enum          mNetwork;       ///< Operation target network.
  NetHost*               mHost;          ///< Host discovered or refreshed (will contain the first host updated if this is a list update).
};

//---------------------------------------------------------------------------------//
//                                 NetHostListUpdate                               //
//---------------------------------------------------------------------------------//

/// Dispatched when a host discovery operation update occurs.
class NetHostListUpdate : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetHostListUpdate();

  Network::Enum mNetwork; ///< Operation target network.
};

/////////////////////////////////////////////////////////////////////////////////////
//                                NetPeer Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

////////////////
// Peer Scope //
////////////////

//---------------------------------------------------------------------------------//
//                                NetPeerOpened                                    //
//---------------------------------------------------------------------------------//

/// Dispatched after successfully opening the net peer.
class NetPeerOpened : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

//---------------------------------------------------------------------------------//
//                                 NetPeerClosed                                   //
//---------------------------------------------------------------------------------//

/// Dispatched before gracefully closing the net peer.
class NetPeerClosed : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

////////////////
// Game State //
////////////////

//---------------------------------------------------------------------------------//
//                                 NetGameStarted                                  //
//---------------------------------------------------------------------------------//

/// Dispatched after fully joining or hosting a network game.
class NetGameStarted : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetGameStarted();

  // Data
  GameSession* mGameSession; ///< Network game session.
};

/////////////////////////////////////////////////////////////////////////////////////
//                                NetLink Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////
// Link Handshake Sequence //
/////////////////////////////

//---------------------------------------------------------------------------------//
//                           NetPeerSentConnectRequest                             //
//---------------------------------------------------------------------------------//

/// Dispatched after sending a connect request.
class NetPeerSentConnectRequest : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerSentConnectRequest(GameSession* gameSession);

  // Data
  IpAddress   mTheirIpAddress;                ///< Their IP address (as seen from our perspective).
  EventBundle mOurRequestBundle;              ///< Our bundled request event data.
  uint        mOurPendingUserAddRequestCount; ///< Our pending user add requests following this connect request (within the same frame).
};

//---------------------------------------------------------------------------------//
//                         NetPeerReceivedConnectRequest                           //
//---------------------------------------------------------------------------------//

/// Dispatched after receiving a connect request.
/// If accepted, their net peer ID is assigned immediately after this.
/// Return true to accept the connect request, else false.
class NetPeerReceivedConnectRequest : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerReceivedConnectRequest(GameSession* gameSession);

  // Data
  IpAddress   mTheirIpAddress;                  ///< Their IP address (as seen from our perspective).
  EventBundle mTheirRequestBundle;              ///< Their bundled request event data.
  uint        mTheirPendingUserAddRequestCount; ///< Their pending user add requests following this connect request (within the same frame).
  IpAddress   mOurIpAddress;                    ///< Our IP address (as seen from their perspective).
  bool        mReturnOurConnectResponse;        ///< Return: Our connect response (accept the connect request?).
  EventBundle mReturnOurResponseBundle;         ///< Return: Our bundled response event data.
};

//---------------------------------------------------------------------------------//
//                           NetPeerSentConnectResponse                            //
//---------------------------------------------------------------------------------//

/// Dispatched after sending a connect response.
/// If denied, their net peer ID is released and link is destroyed immediately after this.
class NetPeerSentConnectResponse : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerSentConnectResponse(GameSession* gameSession);

  // Data
  NetPeerId             mTheirNetPeerId;                  ///< Their net peer ID (set only if accepted).
  IpAddress             mTheirIpAddress;                  ///< Their IP address (as seen from our perspective).
  EventBundle           mTheirRequestBundle;              ///< Their bundled request event data.
  uint                  mTheirPendingUserAddRequestCount; ///< Their pending user add requests following this connect request (within the same frame).
  IpAddress             mOurIpAddress;                    ///< Our IP address (as seen from their perspective).
  ConnectResponse::Enum mOurConnectResponse;              ///< Our connect response.
  EventBundle           mOurResponseBundle;               ///< Our bundled response event data.
};

//---------------------------------------------------------------------------------//
//                        NetPeerReceivedConnectResponse                           //
//---------------------------------------------------------------------------------//

/// Dispatched after receiving a connect response.
/// If accepted, our net peer ID is set immediately before this and a connect confirmation is sent after this.
/// If denied, our net peer ID is cleared and link is destroyed immediately after this.
class NetPeerReceivedConnectResponse : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerReceivedConnectResponse(GameSession* gameSession);

  // Data
  IpAddress             mTheirIpAddress;                ///< Their IP address (as seen from our perspective).
  EventBundle           mOurRequestBundle;              ///< Our bundled request event data.
  uint                  mOurPendingUserAddRequestCount; ///< Our pending user add requests following this connect request (within the same frame).
  IpAddress             mOurIpAddress;                  ///< Our IP address (as seen from their perspective).
  ConnectResponse::Enum mTheirConnectResponse;          ///< Their connect response.
  EventBundle           mTheirResponseBundle;           ///< Their bundled response event data.
  NetPeerId             mOurNetPeerId;                  ///< Our net peer ID (set only if accepted).
};

////////////////
// Link Scope //
////////////////

//---------------------------------------------------------------------------------//
//                                NetLinkConnected                                 //
//---------------------------------------------------------------------------------//

/// Dispatched after sending or receiving a connect confirmation.
class NetLinkConnected : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetLinkConnected();

  // Data
  NetPeerId                   mTheirNetPeerId; ///< Their net peer ID.
  IpAddress                   mTheirIpAddress; ///< Their IP address (as seen from our perspective).
  TransmissionDirection::Enum mDirection;      ///< Transmission direction.
};

//---------------------------------------------------------------------------------//
//                              NetLinkDisconnected                                //
//---------------------------------------------------------------------------------//

/// Dispatched after sending or receiving a disconnect notice.
/// Their net peer ID is released and link is destroyed immediately after this.
class NetLinkDisconnected : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetLinkDisconnected(GameSession* gameSession);

  // Data
  NetPeerId                   mTheirNetPeerId;   ///< Their net peer ID.
  IpAddress                   mTheirIpAddress;   ///< Their IP address (as seen from our perspective).
  DisconnectReason::Enum      mDisconnectReason; ///< Disconnect reason.
  EventBundle                 mRequestBundle;    ///< Bundled request event data.
  TransmissionDirection::Enum mDirection;        ///< Transmission direction.
};

/////////////////////////////////////////////////////////////////////////////////////
//                               NetSpace Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

/////////////////
// Level State //
/////////////////

//---------------------------------------------------------------------------------//
//                                NetLevelStarted                                  //
//---------------------------------------------------------------------------------//

/// Dispatched after fully loading and synchronizing a level in a net space.
class NetLevelStarted : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetLevelStarted();

  // Data
  GameSession* mGameSession; ///< Network game session.
  Space*       mSpace;       ///< Network space.
  String       mLevelName;   ///< Current level name.
};

/////////////////////////////////////////////////////////////////////////////////////
//                                NetUser Events                                   //
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////
//  User Add Handshake Sequence  //
///////////////////////////////////

//---------------------------------------------------------------------------------//
//                          NetPeerSentUserAddRequest                              //
//---------------------------------------------------------------------------------//

/// Dispatched after sending a net user add request.
class NetPeerSentUserAddRequest : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerSentUserAddRequest(GameSession* gameSession);

  // Data
  NetPeerId   mTheirNetPeerId;   ///< Their net peer ID.
  IpAddress   mTheirIpAddress;   ///< Their IP address (as seen from our perspective).
  EventBundle mOurRequestBundle; ///< Our bundled request event data.
};

//---------------------------------------------------------------------------------//
//                        NetPeerReceivedUserAddRequest                            //
//---------------------------------------------------------------------------------//

/// Dispatched after receiving a net user add request.
class NetPeerReceivedUserAddRequest : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerReceivedUserAddRequest(GameSession* gameSession);

  // Data
  NetPeerId   mTheirNetPeerId;          ///< Their net peer ID.
  IpAddress   mTheirIpAddress;          ///< Their IP address (as seen from our perspective).
  EventBundle mTheirRequestBundle;      ///< Their bundled request event data.
  bool        mReturnOurAddResponse;    ///< Return: Our add response (accept the add request?).
  EventBundle mReturnOurResponseBundle; ///< Return: Our bundled response event data.
  Cog*        mReturnTheirNetUser;      ///< Return: Their network user object (must have a NetUser component).
  NetUserId   mTheirNetUserId;          ///< Their net user ID (released back to the store if not accepted).
};

//---------------------------------------------------------------------------------//
//                          NetPeerSentUserAddResponse                             //
//---------------------------------------------------------------------------------//

/// Dispatched after sending a net user add response.
class NetPeerSentUserAddResponse : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerSentUserAddResponse(GameSession* gameSession);

  // Data
  NetPeerId                mTheirNetPeerId;     ///< Their net peer ID.
  IpAddress                mTheirIpAddress;     ///< Their IP address (as seen from our perspective).
  EventBundle              mTheirRequestBundle; ///< Their bundled request event data.
  NetUserAddResponse::Enum mOurAddResponse;     ///< Our add response.
  EventBundle              mOurResponseBundle;  ///< Our bundled response event data.
  NetUserId                mTheirNetUserId;     ///< Their net user ID (set only if accepted).
  Cog*                     mTheirNetUser;       ///< Their net user object about to be added (set only if accepted).
};

//---------------------------------------------------------------------------------//
//                        NetPeerReceivedUserAddResponse                           //
//---------------------------------------------------------------------------------//

/// Dispatched after receiving a net user add response.
class NetPeerReceivedUserAddResponse : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPeerReceivedUserAddResponse(GameSession* gameSession);

  // Data
  NetPeerId                mTheirNetPeerId;      ///< Their net peer ID.
  IpAddress                mTheirIpAddress;      ///< Their IP address (as seen from our perspective).
  EventBundle              mOurRequestBundle;    ///< Our bundled request event data.
  NetUserAddResponse::Enum mTheirAddResponse;    ///< Their add response.
  EventBundle              mTheirResponseBundle; ///< Their bundled response event data.
  NetUserId                mOurNetUserId;        ///< Our net user ID (set only if accepted).
};

///////////////////////
// Network Ownership //
///////////////////////

//---------------------------------------------------------------------------------//
//                          NetUserLostObjectOwnership                             //
//---------------------------------------------------------------------------------//

/// Dispatched after the net user loses network ownership of a net object.
class NetUserLostObjectOwnership : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  Cog* mLostObject;           ///< The object this user just lost network ownership of.
  Cog* mCurrentNetUserOwner;  ///< The object's current network user owner.
};

//---------------------------------------------------------------------------------//
//                        NetUserAcquiredObjectOwnership                           //
//---------------------------------------------------------------------------------//

/// Dispatched after the net user acquires network ownership of a net object.
class NetUserAcquiredObjectOwnership : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  Cog* mAcquiredObject;       ///< The object this user just acquired network ownership of.
  Cog* mPreviousNetUserOwner; ///< The object's previous network user owner.
};

/////////////////////////////////////////////////////////////////////////////////////
//                               NetObject Events                                  //
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////
// Object Initialization //
///////////////////////////

//---------------------------------------------------------------------------------//
//                           RegisterCppNetProperties                              //
//---------------------------------------------------------------------------------//

/// Dispatched while adding C++ component net properties to a NetObject.
class RegisterCppNetProperties : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

//////////////////
// Object Scope //
//////////////////

//---------------------------------------------------------------------------------//
//                                 NetObjectOnline                                 //
//---------------------------------------------------------------------------------//

/// Dispatched after the net object is brought online.
class NetObjectOnline : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  GameSession* mGameSession;       ///< Network game session.
  Space*       mSpace;             ///< Network space.
  Cog*         mObject;            ///< Network object.
  bool         mIsStartOfLifespan; ///< Is this the start of the object's lifespan?
};

//---------------------------------------------------------------------------------//
//                               NetObjectOffline                                  //
//---------------------------------------------------------------------------------//

/// Dispatched before the net object is taken offline.
class NetObjectOffline : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  GameSession* mGameSession;     ///< Network game session.
  Space*       mSpace;           ///< Network space.
  Cog*         mObject;          ///< Network object.
  bool         mIsEndOfLifespan; ///< Is this the end of the object's lifespan?
};

///////////////////////
// Network Ownership //
///////////////////////

//---------------------------------------------------------------------------------//
//                             NetUserOwnerChanged                                 //
//---------------------------------------------------------------------------------//

/// Dispatched after the net object changes network ownership.
class NetUserOwnerChanged : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  Cog* mPreviousNetUserOwner; ///< The object's previous network user owner.
  Cog* mCurrentNetUserOwner;  ///< The object's current network user owner.
};

/////////////////////////////////////
// Network Channel Property Change //
/////////////////////////////////////

//---------------------------------------------------------------------------------//
//                           NetChannelPropertyChange                              //
//---------------------------------------------------------------------------------//

/// Dispatched after an outgoing/incoming net channel property change is detected during a particular replication phase.
class NetChannelPropertyChange : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  float                       mTimestamp;        ///< The time this change occurred.
  ReplicationPhase::Enum      mReplicationPhase; ///< The replication phase.
  TransmissionDirection::Enum mDirection;        ///< The change direction.
  Cog*                        mObject;           ///< The changed net object.
  String                      mChannelName;      ///< The changed net channel.
  String                      mComponentName;    ///< The component which declared the changed net property.
  String                      mPropertyName;     ///< The changed net property.
};

/////////////////////////////////////////////////////////////////////////////////////
//                                NetEvent Events                                  //
/////////////////////////////////////////////////////////////////////////////////////

////////////////////
// Event Handling //
////////////////////

//---------------------------------------------------------------------------------//
//                                 NetEventSent                                    //
//---------------------------------------------------------------------------------//

/// Dispatched after a dispatched net event is sent.
class NetEventSent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  NetPeerId mTheirNetPeerId; ///< Their net peer ID.
  Event*    mNetEvent;       ///< Network event sent.
  Cog*      mDestination;    ///< Dispatch destination object.
};

//---------------------------------------------------------------------------------//
//                               NetEventReceived                                  //
//---------------------------------------------------------------------------------//

/// Dispatched before a received net event is dispatched.
class NetEventReceived : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Data
  NetPeerId mTheirNetPeerId; ///< Their net peer ID.
  Event*    mNetEvent;       ///< Network event received.
  Cog*      mDestination;    ///< Dispatch destination object (null if the net object could not be found locally).
  bool      mReturnAllow;    ///< Return: Allow the received network event to be dispatched on the destination object?
};

//---------------------------------------------------------------------------------//
//                                 NetHostRecord                                   //
//---------------------------------------------------------------------------------//

/// Dispatched when a NetHostRecord is discovered, Updated, or Expired.
class NetHostRecordEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetHostRecordEvent();
  NetHostRecordEvent(NetHostRecord*);

  NetHostRecord* mHostRecord; ///< Host discovered or refreshed (will contain the first host updated if this is a list update).
};

} // namespace Zero
