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
// Peer Connection Interface
//

/// NetPeerConnectionInterface.
/// This adds the interface to a class which allows it to handle connection
/// related events that the net peer receives.
class NetPeerConnectionInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  NetPeerConnectionInterface(NetPeer* netPeer);

  /// Net peer whose events we are connecting to.
  void InitializeEventConnections(NetPeer* netPeer);

  /// Handles the event where we sent a connect response to someone. Return true if we handled it, false if we did not
  virtual void HandleNetPeerSentConnectResponse(NetPeerSentConnectResponse* event) = 0;
  /// Handles a connection response received from another net peer. Return true if we handled it, false if we did not
  virtual void HandleNetPeerReceivedConnectResponse(NetPeerReceivedConnectResponse* event) = 0;
  /// Handles the event where we sent a connect request. Return true if we handled it, false if we did not
  virtual void HandleNetPeerSentConnectRequest(NetPeerSentConnectRequest* event) = 0;
  /// Handles the event where we received a connect response. Return true if we handled, false if we did not
  virtual void HandleNetPeerReceivedConnectRequest(NetPeerReceivedConnectRequest* event) = 0;

  /// Handles the event where we established a connection. Return true if we handled, false if we did not
  virtual void HandleNetLinkConnected(NetLinkConnected* event) = 0;
  /// Handles the event where we established a connection. Return true if we handled, false if we did not
  virtual void HandleNetLinkDisconnected(NetLinkDisconnected* event) = 0;

  /// Returns the event receiver of this instance.
  EventReceiver* GetReceiver();

  // Data
  NetPeer*      mNetPeer;
  EventReceiver mReceiver;
};

} // namespace Zero
