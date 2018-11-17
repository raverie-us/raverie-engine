///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Ping Manager.
/// Used to manage pinging other NetPeers.
/// Pings from a ping manager will only receive pongs that are meant for it.
class PingManager : public NetPeerMessageInterface
{
public:
  /// Constructor.
  PingManager(NetPeer* netPeer);

  /// Handles pending host ping requests.
  void UpdatePendingPings();
  /// Acquires the next random incremental identifier (used to prevent predictive message spam attacks).
  uint AcquireNextRandomIncrementalId();

  /// Create a host ping request and sends a host ping message to the specified IP address.
  /// Specify port 0 to indicate we should send a ping message on all ports within our inclusive host range (configured with SetHostPortRangeStart/End).
  /// Returns true if successful, else false.
  bool PingHost(Network::Enum network, const IpAddress& theirIpAddress, HostPingType::Enum hostPingType, TimeMs timeout, const EventBundle& pingBundle);
  bool PingHost(Network::Enum network, const Array<IpAddress>& theirIpAddress, HostPingType::Enum hostPingType, TimeMs timeout, const EventBundle& pingBundle);

  /// Sends a host ping message.
  /// Returns true if successful, else false.
  bool SendHostPing(PendingHostPing& pendingHostPing, TimeMs now);
  /// Receives a host ping message.
  /// Returns true if successful, else false.
  bool ReceiveHostPing(const IpAddress& theirIpAddress, const Message& message);

  /// Sends a host pong message.
  /// Returns true if successful, else false.
  bool SendHostPong(const IpAddress& theirIpAddress, uint pingId, uint sendAttemptId, uint theirManagerId, BitStream& pongData);
  /// Receives a host pong message.
  /// Returns true if successful, else false.
  bool ReceiveHostPong(const IpAddress& theirIpAddress, const Message& message);

  //
  // NetPeerMessageInterface
  //

  /// Handles receiving a message directly to the peer.
  bool ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage) override;
  /// Handles a message sent through a peer link of a net peer. Returns true if it handled the message, and false if it did not
  bool ReceiveLinkMessage(IpAddress const& thierIpAddress, Message& linkMessage) override;

  //
  // Callback accessors
  //

  // Callback typedef interface (How users handle pings)
  typedef UniquePointer< Callback1<void, PendingHostPing&> > PendingPingCallback;
  typedef UniquePointer< Callback2<bool, IpAddress const&, NetHostPingData&> > PingReceivedCallback;
  typedef UniquePointer< Callback3<void, IpAddress const&, NetHostPongData&, PendingHostPing&> > PongReceivedCallback;

  /// Sets the PingTimeout callback. This happens when a ping request has expired by living longer than its lifetime.
  void SetPingTimeoutCallback(PendingPingCallback callback);
  /// Sets the CancelledCallback. This happens when one ping overwrite another and is canceled before starting the next one.
  void SetPingCancelledCallback(PendingPingCallback callback);
  /// Sets the PingCallback. This happens when a ping manager receives a generic ping from any ping manager.
  void SetPingCallback(PingReceivedCallback callback);
  /// Sets the PongCallback. This happens when the ping manager receives a pong that is specifically for this manager.
  void SetPongCallback(PongReceivedCallback callback);

  //
  // Data
  //

  NetPeer*             mNetPeer;               ///< Net peer used to communicate.
  PendingHostPingSet   mPendingHostPings;      ///< Pending host ping requests.
  uint                 mManagerId;             ///< The id of this ping manager. Used to associate pings with this
  uint                 mNextRandomId;          ///< Next random incremental identifier.
  PendingPingCallback  mPingTimeoutCallback;   ///< Callback for when a pending host ping expires.
  PendingPingCallback  mPingCancelledCallback; ///< Callback for when a pending ping is canceled.
  PingReceivedCallback mPingCallback;          ///< Callback for when we get a ping.
  PongReceivedCallback mPongCallback;          ///< Callback for when we get a pong.
  Timer                mTimer;                 ///< Timer used to time pings.
  float                mPingInterval;          ///< Controls how often the peer sends a host ping message when discovering or refreshing hosts via ping messages.
};

} // namespace Zero
