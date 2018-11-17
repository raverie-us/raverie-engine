///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Typedefs
typedef SortedArray<OutMessagePtr, PointerSortPolicy<OutMessagePtr> > OutMessages;

//---------------------------------------------------------------------------------//
//                              FragmentedReceipt                                  //
//---------------------------------------------------------------------------------//

/// Fragmented message receipt record
/// Tracks ACK state for all the packets a receipted, fragmented message is split across
struct FragmentedReceipt
{
  /// Constructors
  FragmentedReceipt();
  FragmentedReceipt(MessageReceiptId receiptId);

  /// Move Constructor
  FragmentedReceipt(MoveReference<FragmentedReceipt> rhs);

  /// Comparison Operators (compares message receipt IDs)
  bool operator ==(const FragmentedReceipt& rhs) const;
  bool operator !=(const FragmentedReceipt& rhs) const;
  bool operator  <(const FragmentedReceipt& rhs) const;
  bool operator ==(MessageReceiptId rhs) const;
  bool operator !=(MessageReceiptId rhs) const;
  bool operator  <(MessageReceiptId rhs) const;

  /// Fragmented message receipt ID
  MessageReceiptId                           mReceiptId;
  /// Packet ACK state records
  ArrayMap<PacketSequenceId, ACKState::Enum> mPacketRecords;
  /// Completed receipt status flag
  bool                                       mIsComplete;
};

/// FragmentedReceipt Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<FragmentedReceipt>
{
  static inline void MoveWithoutDestruction(FragmentedReceipt* dest, FragmentedReceipt* source)
  {
    new(dest) FragmentedReceipt(ZeroMove(*source));
  }
};

//---------------------------------------------------------------------------------//
//                                  LinkOutbox                                     //
//---------------------------------------------------------------------------------//

/// PeerLink helper class
/// Processes outgoing messages into packets
class LinkOutbox
{
  /// Constructor
  LinkOutbox(PeerLink* link);

  /// Move Constructor
  LinkOutbox(MoveReference<LinkOutbox> rhs);

  /// Move Assignment Operator
  LinkOutbox& operator =(MoveReference<LinkOutbox> rhs);

  //
  // Outgoing Message Channel Management
  //

  /// Opens an outgoing message channel with the specified transfer mode if there are available outgoing message channel IDs on this connected link
  /// Returns the new outgoing message channel if successful, else nullptr
  OutMessageChannel* OpenOutgoingChannel(TransferMode::Enum transferMode);

  /// Returns the outgoing message channel corresponding to the specified message channel ID open on this link, else nullptr
  OutMessageChannel* GetOutgoingChannel(MessageChannelId channelId) const;
  /// Returns all outgoing message channels open on this link
  ArraySet<OutMessageChannel>::range GetOutgoingChannels() const;
  /// Returns the number of outgoing message channels open on this link
  uint GetOutgoingChannelCount() const;

  /// Closes an outgoing message channel open on this link
  void CloseOutgoingChannel(MessageChannelId channelId);
  /// Closes all outgoing message channels open on this link
  void CloseOutgoingChannels();

  //
  // Member Functions
  //

  /// Records a fragmented receipt and adds the corresponding packet record
  void RecordFragmentReceipt(const OutPacket& packet, const OutMessage& message, const OutPacket* prevPacket);
  /// Updates and returns the receipt ID's overall ACK state
  ACKState::Enum UpdateReceiptACKState(const OutPacket& packet, ACKState::Enum packetACKState, const OutMessage& message);
  /// Acknowledges the packet (generates receipt events for all receipted messages accordingly)
  void AcknowledgePacket(OutPacket& packet, ACKState::Enum packetACKState);

  /// ACKs a sent packet
  void ACKSentPacket(const ArraySet<OutPacket>::iterator& sentPacketIter, TimeMs ACKTime);
  /// NAKs a sent packet
  void NAKSentPacket(ArraySet<OutPacket>::iterator& sentPacketIter);

  /// Sends an outgoing packet now
  void SendPacket(MoveReference<OutPacket> packet);

  /// Increments and returns the next receipt ID
  MessageReceiptId AcquireNextReceiptID();
  /// Returns the duration since the last packet was sent
  TimeMs GetLastSendDuration() const;

  /// Pushes an outgoing message to be sent later
  MessageReceiptId PushMessage(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime, bool isProtocol);
  /// Writes a message to the packet
  /// Returns true if done with the message, else false
  bool WriteMessageToPacket(OutPacket& packet, Bits& remBits, OutMessage& message, OutPacket* prevPacket = nullptr);
  /// Attempts to write a message to the packet
  /// Returns the result of the operation
  PacketWriteResult::Enum WriteMessage(OutPacket& packet, OutMessage& message, Bits& remBits, bool isResendMessage);
  /// Updates the link outbox
  void Update(const ACKArray& remoteACKs, const NAKArray& remoteNAKs);

  /// Removes unreliable messages from the packet
  void RemoveUnreliableMessages(OutPacket& packet);
  /// Removes expired messages from the packet and generates receipt events as necessary
  void RemoveExpiredMessages(OutPacket& packet);

   /// Receipts a message at it's intended destination
  void ReceiptMessage(MoveReference<OutMessage> message, Receipt::Enum receipt);

  /// Called before a message is sent
  /// Returns true to continue sending the message, else false
  bool ShouldSendMessage(OutMessage& message);

  /// Operating Data
  PeerLink* mLink; /// Operating link

  /// Channel Data
  OutMessageChannel           mDefaultChannel; /// Outgoing default (zero) message channel
  ArraySet<OutMessageChannel> mChannels;       /// Outgoing (non-zero) message channels
  IdStore<MessageChannelId>   mChannelIdStore; /// Outgoing message channel ID store

  /// Message Data
  MessageReceiptId mNextReceiptID; /// Next outgoing message receipt ID
  OutMessages      mOutMessages;   /// Queued outgoing messages

  /// Packet Data
  PacketSequenceId            mNextSequenceId;     /// Next packet sequence ID
  TimeMs                      mLastSendTime;       /// Last packet send time
  ArraySet<OutPacket>         mSentPackets;        /// Sent packets awaiting acknowledgement
  Array<OutPacket>            mResendPackets;      /// NAKd packets containing messages that need to be resent
  ArraySet<FragmentedReceipt> mFragmentedReceipts; /// Fragmented receipt records

  /// Friends
  friend class PeerLink;
};

} // namespace Zero
