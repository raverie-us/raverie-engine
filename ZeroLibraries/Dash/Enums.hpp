///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//
// Peer Enums
//

/// Peer transport layer protocol
DeclareEnum3(TransportProtocol,
  Unspecified, /// Unspecified transport layer protocol
  Tcp,         /// Transmission control protocol
  Udp);        /// User datagram protocol

/// Peer incoming connect response mode
DeclareEnum3(ConnectResponseMode,
  Accept,  /// Accepts all appropriate incoming connection requests
  Deny,    /// Denies all appropriate incoming connection requests
  Custom); /// Notifies the user of all appropriate incoming connection requests,
           /// call PeerLink::RespondToConnectRequest() ASAP to accept or deny them

/// Transmission direction
DeclareEnum3(TransmissionDirection,
  Unspecified, /// Unspecified transmission direction
  Incoming,    /// Incoming, from a remote peer
  Outgoing);   /// Outgoing, to a remote peer

//
// Link Enums
//

/// Overall link status
DeclareEnum4(LinkStatus,
  Unspecified,          /// Unspecified overall link status
  Disconnected,         /// Disconnected from the remote peer
  AttemptingConnection, /// Attempting connection with the remote peer
  Connected);           /// Connected to the remote peer

/// Specific link state
DeclareEnum9(LinkState,
  Unspecified, /// Unspecified specific link state

  /// LinkStatus::Disconnected
  Disconnected,                    /// Disconnected from the remote peer (default state)
  SentDisconnectNotice,            /// Sent a disconnect notice, awaiting their ACK or ((RTT/2) * disconnect attempt factor) timeout before being disconnected
  ReceivedDisconnectNotice,        /// Received a disconnect notice, waiting ((RTT/2) * disconnect attempt factor) for probable ACKing before being disconnected
  SentNegativeConnectResponse,     /// Sent a negative connect response, awaiting their ACK or ((RTT/2) * connect attempt factor) timeout before being disconnected
  ReceivedNegativeConnectResponse, /// Received a negative connect response, waiting ((RTT/2) * connect attempt factor) for probable ACKing before being disconnected

  /// LinkStatus::AttemptingConnection
  SentConnectRequest,     /// Sent a connect request, awaiting their connect response or ((RTT/2) * connect attempt factor) timeout before being disconnected
  ReceivedConnectRequest, /// Received an appropriate connect request, they're awaiting our connect response or ((RTT/2) * connect attempt factor) timeout before being disconnected

  /// LinkStatus::Connected
  Connected); /// Connected to the remote peer, may send or receive a disconnect notice by request, latency (latency limit), or timeout ((RTT/2) * timeout factor)

/// Link connect response
DeclareEnum4(ConnectResponse,
  Accept,       /// Connection request accepted
  Deny,         /// Connection request denied
  DenyFull,     /// Connection request denied, remote peer connection limit has been reached (Peer::GetConnectionLimit)
  DenyTimeout); /// Connection request timed out, remote peer did not respond

/// Link connect response range
static const ConnectResponse::Type ConnectResponseMin = ConnectResponse::Accept;
static const ConnectResponse::Type ConnectResponseMax = ConnectResponse::DenyTimeout;

/// Link connect response bits
static const Bits ConnectResponseBits = BITS_NEEDED_TO_REPRESENT(ConnectResponseMax);

/// Link disconnect reason
DeclareEnum3(DisconnectReason,
  Request,  /// Disconnected by request
  Timeout,  /// Disconnected by timeout
  Latency); /// Disconnected by latency

/// Link disconnect reason range
static const DisconnectReason::Type DisconnectReasonMin = DisconnectReason::Request;
static const DisconnectReason::Type DisconnectReasonMax = DisconnectReason::Latency;

/// Link disconnect reason bits
static const Bits DisconnectReasonBits = BITS_NEEDED_TO_REPRESENT(DisconnectReasonMax);

/// Link user connect response (used internally)
DeclareEnum3(UserConnectResponse,
  Pending, /// Pending user response
  Accept,  /// Accepted by the user
  Deny);   /// Denied by the user

//
// Packet Enums
//

/// Packet write message result (used internally)
DeclareEnum5(PacketWriteResult,
  NotDone_None,     /// Not done with this message, nothing was written
  NotDone_Fragment, /// Not done with this message, a fragment was written
  Done_Fragment,    /// Done with this message, the final fragment was written
  Done_Whole,       /// Done with this message, the whole message was written
  Done_Rejected);   /// Done with this message, the whole message was rejected (nothing was written)

/// Packet acknowledgement state (used internally)
DeclareEnum3(ACKState,
  Undetermined, /// Undetermined ACK state
  ACKd,         /// Acknowledged
  NAKd)         /// Not acknowledged

//
// Message Enums
//

/// Message channel transfer mode
/// All transfer modes include duplicate protection
DeclareEnum3(TransferMode,
  Immediate, /// Messages released immediately on receive, including late messages
  Sequenced, /// Messages released immediately on receive, discarding late messages
  Ordered);  /// Messages released immediately, in sent order, once preceding late messages have been received; forces all messages to be reliable

/// Message channel transfer mode range
static const TransferMode::Type TransferModeMin = TransferMode::Immediate;
static const TransferMode::Type TransferModeMax = TransferMode::Ordered;

/// Message channel transfer mode bits
static const Bits TransferModeBits = BITS_NEEDED_TO_REPRESENT(TransferModeMax);

// Transfer Example:
// Sent 1 2 3 4 5; 2 was 'lost', 3 was *late*, and 5 was 'lost'
// ------------------------------------------------------------
// Immediate (Unreliable): 1 ' '  4  *3* ' ' Released upon arrival, even if out-of-order
// Immediate (Reliable)  : 1  4  *3* '2' '5' Released upon arrival, even if out-of-order, and retransmits lost messages
// Sequenced (Unreliable): 1 ' ' *X*  4  ' ' Released upon arrival, but discarded if out-of-order
// Sequenced (Reliable)  : 1  4  *X* 'X' '5' Released upon arrival, but discarded if out-of-order, and retransmits lost messages
// Ordered   (Reliable)  : 1 '2' *3*  4  '5' Released upon arrival when prior messages have been received, re-orders messages, and retransmits lost messages
// ------------------------------------------------------------
// Note: With Sequenced (Reliable), retransmitted messages are dropped unless they are most recent;
// This can be wasteful of bandwidth if many packets are lost, so keep this in mind.

/// Message receipt result
DeclareEnum4(Receipt,
  ACK,      /// Message was sent and ACKnowledged
  NAK,      /// Message was sent and Not AcKnowledged
  MAYBE,    /// Message was sent and MAYBE acknowledged (message was received but may have been discarded)
  EXPIRED); /// Message was not sent (either due to an elapsed lifetime or link disconnection)

// Receipt Possibilities:
// ----------------------
// Immediate (Unreliable): ACK, NAK, EXPIRED
// Immediate (Reliable)  : ACK, EXPIRED
// Sequenced (Unreliable): MAYBE, NAK, EXPIRED
// Sequenced (Reliable)  : MAYBE, EXPIRED
// Ordered   (Reliable)  : ACK, EXPIRED
// ----------------------
// Note: MAYBE indicates that the message arrived but might have been discarded if it arrived late.
// Even so, MAYBE will often indicate an probable ACK.

/// Protocol message types (used internally)
DeclareEnum10(ProtocolMessageType,
  Invalid,               /// Invalid message type
  ConnectRequest,        /// Connection request
  ConnectResponse,       /// Connection response
  PacketSequenceHistory, /// Packet sequence history
  ChannelOpened,         /// Channel opened notice
  ChannelClosed,         /// Channel closed notice
  DisconnectNotice,      /// Graceful disconnection notice
  Reserved1,             /// Reserved for later use
  Reserved2,             /// Reserved for later use
  Reserved3);            /// Reserved for later use

} // namespace Zero
