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
// NetPeer Message Interface - Handles messages received on peer link
//

/// NetPeerMessageInterface.
/// This is the interface required for receiving messages from a netpeer.
class NetPeerMessageInterface
{
public:
  /// Handles a message sent directly to the net peer. Returns true if it handled the message, and false if it did not
  virtual bool ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage) = 0;
  /// Handles a message sent through a peer link of a net peer. Returns true if it handled the message, and false if it did not
  virtual bool ReceiveLinkMessage(IpAddress const& theirIpAddress, Message& linkMessage) = 0;
};

} // namespace Zero
