///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//
// Enumerations
//

/// Net Discovery Mode.
/// Represents what a NetDiscovery object is currently doing. (it can be refreshing, a list, single hosts, or idle)
DeclareEnum3(NetDiscoveryMode,
  RefreshList,
  Refresh,
  Idle);

/// Net Discovery Stage.
/// What stage of discovery is a host request in? The stages are represented here.
DeclareEnum4(NetDiscoveryStage,
  Unresponding,
  BasicHostInfo,
  BasicHostInfoDirect,
  ExtraHostInfo);

//---------------------------------------------------------------------------------//
//                                   NetDiscovery                                  //
//---------------------------------------------------------------------------------//

/// Responding Host Data.
/// This stores the most up-to-date host data of a host request. It could be
/// that no data was found on a host, and so no response is returned.
class RespondingHostData
{
public:
  /// Constructors.
  RespondingHostData();
  RespondingHostData(RespondingHostData const& rhs);
  RespondingHostData(MoveReference<RespondingHostData> rhs);

  /// Updates the host data state from no response to basic hose info (taking into account if it was direct or indirect info).
  void UpdateToBasic(bool isIndirect);

  TimeMs                 mRoundTripTime = 0;                            ///< Round trip time (from our peer to this host).
  BitStream              mBasicHostInfo;                                ///< Basic host info (limited to 480 bytes).
  BitStream              mExtraHostInfo;                                ///< Extra host info (requires that connection is established).
  NetRefreshResult::Enum mRefreshResult = NetRefreshResult::NoResponse; ///< How far in the process did this host get?
};

/// Open Host Request.
/// This class stores the state of a host request.
/// A host request is put out by a peer in order to learn more information about other hosts.
/// Host requests can either be for on specific host, or for many unknown hosts.
/// Different methods for finding hosts exist depending on if your operating on the LAN or Internet.
class OpenHostRequest
{
public:
  /// Called in order to prematurely shutdown a host request. Dispatches events, expects that this host request cleans up anything it created
  virtual void FlushHostRequest(NetPeer& netPeer, NetDiscoveryInterface& netDisoveryInstance) = 0;
  /// It will test the passed in IpAddress, and if it was a newly discovered host it will return true.
  virtual bool IsNewHost(IpAddress const& hostIpAddress) = 0;
  /// Function which returns if the host this request is about has gone stale.
  virtual bool IsStaleHost(IpAddress const& hostIpAddress) = 0;

  /// Currently unused. Was to be used in implementing the start of getting extra host info
  virtual void BeginExtraHostInfo() = 0;

  /// Function which handles a pong response from an IpAddress we pinged. Return true if first response, else false
  virtual bool GetIsFirstResponseFrom(IpAddress const& pingedHostIp) = 0;
  /// Sets if this was the first response from a specific IpAddress.
  virtual void SetIsFirstResponseFrom(IpAddress const& pingedHostIp, bool isFirstResponse) = 0;

  /// Helper function to cancel a single host.
  void FlushHost(NetPeer& netPeer, NetDiscoveryInterface& netDiscoveryInstance, IpAddress const& ipAddress);

  //
  // Data
  //
  Network::Enum           mNetwork              = Network::LAN;                     ///< Network request is on.
  bool                    mAllowDiscovery       = false;                            ///< Should refresh allow new discoveries?
  NetDiscoveryStage::Enum mDiscoveryStage       = NetDiscoveryStage::Unresponding;  ///< How far along is this host request?
  bool                    mRemoveStaleHosts     = true;                             ///< Should hosts that did not respond be removed?
  bool                    mAquireExtraHostInfo  = false;                            ///< Does this request want extra host info.
};

typedef UniquePointer<OpenHostRequest> OpenHostRequestPtr;

/// Specific host request.
class SingleHostRequest : public OpenHostRequest
{
public:

  //
  // Request Interface
  //
  void FlushHostRequest(NetPeer& netPeer, NetDiscoveryInterface& netDiscoveryInstance) override;
  bool IsNewHost(IpAddress const& hostIpAddress) override;
  bool IsStaleHost(IpAddress const& hostIpAddress) override;
  void BeginExtraHostInfo() override;
  bool GetIsFirstResponseFrom(IpAddress const& pingedHostIp) override;
  void SetIsFirstResponseFrom(IpAddress const& pingedHostIp, bool isFirstResponse) override;
  //
  // Data
  //
  IpAddress mIpAddress;             ///< Specific IP that was refreshed was requested for.
  bool      mPreviouslyKnown;       ///< Did we know about this host before the request?
  bool      mReceivedFirstResponse; ///< Has this request received its first request?
};

/// Non-specific host request.
class MultiHostRequest : public OpenHostRequest
{
public:
  //
  // Request Interface
  //

  void FlushHostRequest(NetPeer& netPeer, NetDiscoveryInterface& netDiscoveryInstance) override;
  bool IsNewHost(IpAddress const& hostIpAddress) override;
  bool IsStaleHost(IpAddress const& hostIpAddress) override;
  void BeginExtraHostInfo() override;
  bool GetIsFirstResponseFrom(IpAddress const& pingedHostIp) override;
  void SetIsFirstResponseFrom(IpAddress const& pingedHostIp, bool isFirstResponse) override;

  //
  // Data
  //
  ArraySet<IpAddress>         mRespondingHosts; // An array containing the IpAddress of All hosts who responded.
  ArraySet<IpAddress>         mExpectedHosts;   // An array containing a list of IPs who we expect a response from.
};

/// NetDiscoveryInterface.
/// Works under the net peer to manage discovering other net peers.
/// Manages network object state and event replication.
class NetDiscoveryInterface : public NetPeerConnectionInterface, public NetPeerMessageInterface
{
public:
  NetDiscoveryInterface( NetPeer* netPeer );

  //
  // NetDiscoveryInterface
  //

  /// Refresh all hosts that it possibly can.
  virtual void RefreshAll(bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts) = 0;
  /// Refresh just one host.
  virtual void SingleHostRefresh(IpAddress const& thierIp, bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts) = 0;

  virtual void HandleCancelSingleHostRequest(SingleHostRequest& singleHostRequest) = 0;
  virtual void HandleCancelMultiHostRequest(MultiHostRequest& multiHostRequest) = 0;

  /// Handle different ping callbacks.
  virtual bool HandlePing(IpAddress const& theirIpAddress, NetHostPingData& netHostPingData) = 0;
  virtual void HandlePingCancelled(PendingHostPing& pendingHostPing) = 0;
  virtual void HandlePingTimeout(PendingHostPing& pendingHostPing) = 0;

  /// Its expected users use the pong helper. Then in handle pong they just have to decide how they want to handle the pong in terms
  /// of doing additional host discovery or not. (LAN will call it good, internet will likely want to do another more direct search)
  virtual void HandlePong(IpAddress const& theirIpAddress, NetHostPongData& netHostPongdata, PendingHostPing& pendingHostPing ) = 0;

  virtual void OnEngineUpdate(UpdateEvent* event) = 0;

  /// A function which is called when the net peer closes. If a net host discovery instance did something, this is where it can cleans it up.
  virtual void CleanUp() = 0;

  //
  // Standard NetDiscovery functionality.
  //

  /// Updates the network discovery process.
  void Update(UpdateEvent* event);

  /// A function which cancels All refreshes.
  void CancelRefreshes();

  /// Will cancel if a refresh is already going.
  void CancelIfNotIdle();

  /// Will cancel if a list refresh is in progress.
  void CancelIfRefreshingList();

  /// A helper function which connects our callbacks to the ping manager.
  void SetPingManagerCallbacks();

  /// Initialized must be called to set callbacks and to listen to events of the net peer. (because net discovery often causes events, and must handle them itself)
  void Initialize();

  /// Helper function which looks to net peer to see what hosts we expect a response from.
  void GetExpectedHosts(Network::Enum network, ArraySet<IpAddress>& outIpAddressList);

  /// Helper to create and get a pointer to a new open host request.
  SingleHostRequest* CreateSingleHostRequest(Network::Enum network, bool allowDiscovery, IpAddress const& theirIpAddress, bool removeStaleHosts, bool extraHostInfo);
  /// Helper to create and get a pointer to a new open host request.
  MultiHostRequest* CreateMultiHostRequest(Network::Enum network, bool allowDiscovery, bool removeStaleHosts, bool extraHostInfo);

  /// Helper for dispatching NetEvents.
  /// Dispatches a responding host event. Creates NetHosts as appropriate.
  void DispatchHost(IpAddress const & hostIp, OpenHostRequest& hostRequest);

  /// Will attempt to get the multi host request. will throw assert if not possible.
  MultiHostRequest* GetMultiHostRequest();
  /// Will attempt to find single host request. if there isn't one, it returns null.
  SingleHostRequest* GetSingleHostRequest(IpAddress const& thierIpAddress);

  /// A function which terminates an events propagation if the event was caused by something internal, and we don't really want the user to see or deal with it.
  /// Example: Master server communication and exchanging extra host info.
  void TerminateInternalEvent(IpAddress const& eventIp, Event* event);

  /// Returns the IP of the host that was pinged. (it is not theirIpAddress in the case of a master server pong)
  /// Pong helper extracts data from the pong and creates responding host data (or updates it).
  IpAddress PongHelper(IpAddress const& theirIpAddress, NetHostPongData& netHostPongData, PendingHostPing& pendingHostPing);

  bool PongIsForThisProject(NetHostPongData const& netHostPongData, PendingHostPing const& pendingHostPing);

  /// Called on net discovery interface in order to dispatch events, and clean up all other information related to the host request.
  void EndSingleRefresh(SingleHostRequest* hostRequest);

  //
  //  NetPeerMessageInterface
  //

  /// Default implementation receives peer messages for our PingManager. (be sure to call this in overloaded functions)
  virtual bool ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage) override;

  //
  //  Data
  //
  NetDiscoveryMode::Enum                  mDiscoveryMode;       // Some modes cancel others.
  PingManager                             mPingManager;         // Tool for NetHostDiscovery to use to communicate with others.
  Array<OpenHostRequestPtr>               mOpenHostRequests;    // Open host request list.
  ArrayMap<IpAddress, RespondingHostData> mRespondingHostData;  // Array of host data for those who responded. this is for both multi and single host requests to use.
  HashMap<IpAddress, SingleHostRequest*>  mSingleHostRequests;  // Map of single host requests for easy access.
};

} // namespace Zero
