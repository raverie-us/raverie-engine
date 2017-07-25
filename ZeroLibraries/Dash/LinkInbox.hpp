///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// PeerLink helper class
/// Processes incoming packets into messages
class LinkInbox
{
  /// Constructor
  LinkInbox(PeerLink* link);

  /// Move Constructor
  LinkInbox(MoveReference<LinkInbox> rhs);

  /// Move Assignment Operator
  LinkInbox& operator =(MoveReference<LinkInbox> rhs);

  //
  // Incoming Message Channel Management
  //

  /// Returns the incoming message channel corresponding to the specified message channel ID open on this link, else nullptr
  InMessageChannel* GetIncomingChannel(MessageChannelId channelId) const;
  /// Returns all incoming message channels open on this link
  ArraySet<InMessageChannel>::range GetIncomingChannels() const;
  /// Returns the number of incoming message channels open on this link
  uint GetIncomingChannelCount() const;

  //
  // Member Functions
  //

  /// Receives an incoming packet to be processed later
  void ReceivePacket(MoveReference<InPacket> packet);

  /// Updates the link inbox
  /// Releases custom and protocol messages as appropriate
  void Update(ACKArray& remoteACKs, NAKArray& remoteNAKs);

  /// Releases the specified custom messages
  void ReleaseCustomMessages(Array<Message>& messages);
  /// Releases the specified protocol messages
  void ReleaseProtocolMessages(Array<Message>& messages);

  /// Returns the duration since the last packet was received
  TimeMs GetLastReceiveDuration() const;

  /// Pushes an event message to be received later by the user
  void PushUserEventMessage(MoveReference<Message> message);
  /// Pushes an event message to be received later by the protocol
  void PushProtocolEventMessage(MoveReference<Message> message);

  /// Operating Data
  PeerLink* mLink; /// Operating link

  /// Packet Data
  Array<InPacket>  mReceivedPackets;                      /// Received packets
  TimeMs           mLastReceiveTime;                      /// Last packet receive time
  PacketSequence   mIncomingPacketSequence;               /// Incoming packet sequence record
  TimeMs           mLastOutPacketSequenceHistorySendTime; /// Last outgoing packet sequence history send time
  PacketSequenceId mLastOutPacketSequenceHistoryNESQ;     /// Last outgoing packet sequence history next expected sequence ID
  PacketSequenceId mLastInPacketSequenceHistoryNESQ;      /// Last incoming packet sequence history next expected sequence ID

  /// Channel Data
  InMessageChannel           mCustomDefaultChannel;   /// Incoming default (zero) custom message channel
  InMessageChannel           mProtocolDefaultChannel; /// Incoming default (zero) protocol message channel
  ArraySet<InMessageChannel> mChannels;               /// Incoming (non-zero) message channels

  /// Message Data
  Array<Message> mReleasedCustomMessages;   /// Released custom messages
  Array<Message> mReleasedProtocolMessages; /// Released protocol messages

  /// Friends
  friend class PeerLink;
};

} // namespace Zero
