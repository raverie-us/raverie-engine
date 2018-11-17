///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class LanHostDiscovery : public NetDiscoveryInterface
{
public:
  /// Constructor.
  LanHostDiscovery(NetPeer* netPeer);

  //
  // NetPeer Message Interface
  //

  /// Handles a message sent directly to the net peer. Returns true if it handled the message, and false if it did not
  virtual bool ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage) override;
  // Handles a message sent through a peer link of a net peer. Returns true if it handled the message, and false if it did not
  virtual bool ReceiveLinkMessage(IpAddress const& theirIpAddress, Message& linkMessage) override;

  //
  // NetPeer Connection Interface
  //

  /// Handles the event where we sent a connect response to someone. Returns true if we handled it, false if we did not
  virtual void HandleNetPeerSentConnectResponse(NetPeerSentConnectResponse* event) override;
  /// Handles a connection response received from another net peer. Returns true if we handled it, false if we did not
  virtual void HandleNetPeerReceivedConnectResponse(NetPeerReceivedConnectResponse* event) override;
  /// Handles the event where we sent a connect request. Returns true if we handled it, false if we did not
  virtual void HandleNetPeerSentConnectRequest(NetPeerSentConnectRequest* event) override;
  /// Handles the event where we received a connect response. Returns true if we handled, false if we did not
  virtual void HandleNetPeerReceivedConnectRequest(NetPeerReceivedConnectRequest* event) override;

  /// Handles the event where we established a connection. Returns true if we handled, false if we did not
  virtual void HandleNetLinkConnected(NetLinkConnected* event) override;
  /// Handles the event where we established a connection. Returns true if we handled, false if we did not
  virtual void HandleNetLinkDisconnected(NetLinkDisconnected* event) override;

  //
  // Net Discovery Interface
  //

  /// Refresh all hosts that it possibly can.
  virtual void RefreshAll(bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts) override;
  /// Refresh just one host.
  virtual void SingleHostRefresh(IpAddress const& thierIp, bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts) override;

  virtual void HandleCancelSingleHostRequest(SingleHostRequest& singleHostRequest);
  virtual void HandleCancelMultiHostRequest(MultiHostRequest& multiHostRequest);

  /// Handle different ping callbacks.
  virtual bool HandlePing(IpAddress const& theirIpAddress, NetHostPingData& netHostPingData) override;
  virtual void HandlePingCancelled(PendingHostPing& pendingHostPing) override;
  virtual void HandlePingTimeout(PendingHostPing& pendingHostPing) override;
  virtual void HandlePong(IpAddress const& theirIpAddress, NetHostPongData& netHostPongdata, PendingHostPing& pendingHostPing) override;

  virtual void OnEngineUpdate(UpdateEvent* event);

  virtual void CleanUp();

};

} // namespace Zero
