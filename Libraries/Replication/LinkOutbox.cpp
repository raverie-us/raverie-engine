///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                              FragmentedReceipt                                  //
//---------------------------------------------------------------------------------//

FragmentedReceipt::FragmentedReceipt()
  : mReceiptId(0),
    mPacketRecords(),
    mIsComplete(false)
{
}
FragmentedReceipt::FragmentedReceipt(MessageReceiptId receiptId)
  : mReceiptId(receiptId),
    mPacketRecords(),
    mIsComplete(false)
{
}

FragmentedReceipt::FragmentedReceipt(MoveReference<FragmentedReceipt> rhs)
  : mReceiptId(rhs->mReceiptId),
    mPacketRecords(ZeroMove(rhs->mPacketRecords)),
    mIsComplete(rhs->mIsComplete)
{
}

bool FragmentedReceipt::operator ==(const FragmentedReceipt& rhs) const
{
  return mReceiptId == rhs.mReceiptId;
}
bool FragmentedReceipt::operator !=(const FragmentedReceipt& rhs) const
{
  return mReceiptId != rhs.mReceiptId;
}
bool FragmentedReceipt::operator  <(const FragmentedReceipt& rhs) const
{
  return mReceiptId < rhs.mReceiptId;
}
bool FragmentedReceipt::operator ==(MessageReceiptId rhs) const
{
  return mReceiptId == rhs;
}
bool FragmentedReceipt::operator !=(MessageReceiptId rhs) const
{
  return mReceiptId != rhs;
}
bool FragmentedReceipt::operator  <(MessageReceiptId rhs) const
{
  return mReceiptId < rhs;
}

//---------------------------------------------------------------------------------//
//                                  LinkOutbox                                     //
//---------------------------------------------------------------------------------//

LinkOutbox::LinkOutbox(PeerLink* link)
    /// Operating Data
  : mLink(link),

    /// Channel Data
    mDefaultChannel(0, TransferMode::Immediate),
    mChannels(),
    mChannelIdStore(),

    /// Message Data
    mNextReceiptID(1),
    mOutMessages(),

    /// Packet Data
    mNextSequenceId(0),
    mLastSendTime(link->GetLocalTime()),
    mSentPackets(),
    mResendPackets(),
    mFragmentedReceipts()
{
}

LinkOutbox::LinkOutbox(MoveReference<LinkOutbox> rhs)
    /// Operating Data
  : mLink(rhs->mLink),

    /// Channel Data
    mDefaultChannel(rhs->mDefaultChannel),
    mChannels(ZeroMove(rhs->mChannels)),
    mChannelIdStore(ZeroMove(rhs->mChannelIdStore)),

    /// Message Data
    mNextReceiptID(rhs->mNextReceiptID),
    mOutMessages(ZeroMove(rhs->mOutMessages)),

    /// Packet Data
    mNextSequenceId(rhs->mNextSequenceId),
    mLastSendTime(rhs->mLastSendTime),
    mSentPackets(ZeroMove(rhs->mSentPackets)),
    mResendPackets(ZeroMove(rhs->mResendPackets)),
    mFragmentedReceipts(ZeroMove(rhs->mFragmentedReceipts))
{
}

LinkOutbox& LinkOutbox::operator =(MoveReference<LinkOutbox> rhs)
{
  /// Operating Data
  mLink = rhs->mLink;

  /// Channel Data
  mDefaultChannel = rhs->mDefaultChannel;
  mChannels       = ZeroMove(rhs->mChannels);
  mChannelIdStore = ZeroMove(rhs->mChannelIdStore);

  /// Message Data
  mNextReceiptID = rhs->mNextReceiptID;
  mOutMessages   = ZeroMove(rhs->mOutMessages);

  /// Packet Data
  mNextSequenceId     = rhs->mNextSequenceId;
  mLastSendTime       = rhs->mLastSendTime;
  mSentPackets        = ZeroMove(rhs->mSentPackets);
  mResendPackets      = ZeroMove(rhs->mResendPackets);
  mFragmentedReceipts = ZeroMove(rhs->mFragmentedReceipts);

  return *this;
}

//
// Outgoing Message Channel Management
//

OutMessageChannel* LinkOutbox::OpenOutgoingChannel(TransferMode::Enum transferMode)
{
  // Link not connected?
  if(mLink->GetState() != LinkState::Connected)
    return nullptr;

  // Acquire new outgoing message channel ID
  MessageChannelId channelId = mChannelIdStore.AcquireId();
  if(channelId == 0) // Unable?
    return nullptr;

  // Open new outgoing message channel
  OutMessageChannel newChannel(channelId, transferMode);
  ArraySet<OutMessageChannel>::pointer_bool_pair result = mChannels.Insert(newChannel);
  Assert(result.second); // (Insertion should have succeeded)

  // Create channel opened message
  Message message(ProtocolMessageType::ChannelOpened);

  ChannelOpenedData channelOpenedData;
  channelOpenedData.mTransferMode = transferMode;

  message.GetData().Write(channelOpenedData);

  // Send channel opened message
  Status status;
  mLink->SendInternal(status, ZeroMove(message), true, channelId);
  Assert(status.Succeeded()); // (Send should have succeeded)

  // Success
  return result.first;
}

OutMessageChannel* LinkOutbox::GetOutgoingChannel(MessageChannelId channelId) const
{
  return mChannels.FindPointer(channelId);
}
ArraySet<OutMessageChannel>::range LinkOutbox::GetOutgoingChannels() const
{
  return mChannels.All();
}
uint LinkOutbox::GetOutgoingChannelCount() const
{
  return mChannels.Size();
}

void LinkOutbox::CloseOutgoingChannel(MessageChannelId channelId)
{
  // Get outgoing message channel
  ArraySet<OutMessageChannel>::iterator iter = mChannels.FindIterator(channelId);
  if(iter == mChannels.End()) // Channel not found?
  {
    Assert(false); // (Just to make sure we're not doing anything silly, not critical to check this)
    return;
  }

  // Link is connected?
  if(mLink->GetState() == LinkState::Connected)
  {
    // Create channel closed message
    Message message(ProtocolMessageType::ChannelClosed);

    // Send channel closed message
    Status status;
    mLink->SendInternal(status, ZeroMove(message), true, channelId);
    Assert(status.Succeeded()); // (Send should have succeeded)
  }

  // Close outgoing message channel
  mChannels.Erase(iter);

  // Free outgoing message channel ID
  bool idFreeResult = mChannelIdStore.FreeId(channelId);
  Assert(idFreeResult); // (ID free should have succeeded)
}
void LinkOutbox::CloseOutgoingChannels()
{
  while(!mChannels.Empty())
    CloseOutgoingChannel(mChannels.Back().GetChannelId());
}

//
// Member Functions
//

void LinkOutbox::RecordFragmentReceipt(const OutPacket& packet, const OutMessage& message, const OutPacket* prevPacket)
{
  Assert(message.IsFragment());

  // Message doesn't need a receipt?
  if(!message.IsReceipted())
    return;

  // Add new fragmented receipt if one does not already exist
  ArraySet<FragmentedReceipt>::iterator fragmentedReceiptIter = mFragmentedReceipts.FindIterator(message.GetReceiptID());
  if(fragmentedReceiptIter == mFragmentedReceipts.End()) // Doesn't exist?
  {
    // Add new fragmented receipt
    FragmentedReceipt newFragmentedReceipt(message.GetReceiptID());
    ArraySet<FragmentedReceipt>::pointer_bool_pair result = mFragmentedReceipts.Insert(ZeroMove(newFragmentedReceipt));
    Assert(result.second); // (Insert should have succeeded)
    fragmentedReceiptIter = result.first;
  }

  // Add new packet record if one does not already exist (we don't care if this Insert fails)
  fragmentedReceiptIter->mPacketRecords.Insert(packet.GetSequenceId(), ACKState::Undetermined);

  // Is this a resend message?
  if(prevPacket)
  {
    // Remove previous packet record
    ArrayMap<PacketSequenceId, ACKState::Enum>::pointer_bool_pair result = fragmentedReceiptIter->mPacketRecords.EraseValue(prevPacket->GetSequenceId());
    Assert(result.second); // (Erase should have succeeded)
  }
  // Not a resend message?
  else
  {
    // Mark as complete if this is the final fragment message
    Assert(!fragmentedReceiptIter->mIsComplete);
    fragmentedReceiptIter->mIsComplete = message.IsFinalFragment();
  }
}
ACKState::Enum LinkOutbox::UpdateReceiptACKState(const OutPacket& packet, ACKState::Enum packetACKState, const OutMessage& message)
{
  // Typedefs
  typedef ArrayMap<PacketSequenceId, ACKState::Enum>::value_type packet_record_type;

  // Packet NAKd and message is reliable?
  if(packetACKState == ACKState::NAKd && message.IsReliable())
    return ACKState::Undetermined;

  //
  // Whole Message Receipt?
  //
  ArraySet<FragmentedReceipt>::iterator fragmentedReceiptIter = mFragmentedReceipts.FindIterator(message.GetReceiptID());
  if(fragmentedReceiptIter == mFragmentedReceipts.End())
    return packetACKState;

  //
  // Fragment Message Receipt?
  //

  // Update packet record ACK state
  ArrayMap<PacketSequenceId, ACKState::Enum>::iterator packetRecordIter = fragmentedReceiptIter->mPacketRecords.FindIterator(packet.GetSequenceId());
  Assert(packetRecordIter != fragmentedReceiptIter->mPacketRecords.End());
  Assert(packetRecordIter->second == ACKState::Undetermined);
  packetRecordIter->second = packetACKState;

  // Fragmented receipt is incomplete (not all fragments have been sent yet)?
  if(!fragmentedReceiptIter->mIsComplete)
    return ACKState::Undetermined;

  // Determine overall fragmented receipt ACK state based on all contained packet records' current ACK states
  ACKState::Enum result = ACKState::ACKd;
  forRange(const packet_record_type& packetRecord, fragmentedReceiptIter->mPacketRecords.All())
  {
    if(packetRecord.second == ACKState::Undetermined)
      return ACKState::Undetermined;
    else if(packetRecord.second == ACKState::NAKd)
      result = ACKState::NAKd;
  }

  // Clear fragmented receipt record now that it's been ACKd or NAKd
  Assert(result == ACKState::ACKd || result == ACKState::NAKd);
  mFragmentedReceipts.Erase(fragmentedReceiptIter);
  return result;
}
void LinkOutbox::AcknowledgePacket(OutPacket& packet, ACKState::Enum packetACKState)
{
  Assert(packetACKState != ACKState::Undetermined);

  // For all receipted messages in the given packet
  forRange(OutMessage& message, packet.GetMessages().All())
  {
    // Message not receipted?
    if(!message.IsReceipted())
      continue; // Skip message

    //
    // Update Receipt ACKState
    //
    ACKState::Enum result = UpdateReceiptACKState(packet, packetACKState, message);

    // Receipt message based on ACK state
    switch(result)
    {
    default:
      Assert(false);
    case ACKState::Undetermined:
      // Skip message
      break;

    case ACKState::ACKd:
      switch(message.GetTransferMode())
      {
      // ACK
      default:
        Assert(false);
      case TransferMode::Immediate:
      case TransferMode::Ordered:
        ReceiptMessage(ZeroMove(message), Receipt::ACK);
        break;

      // MAYBE
      case TransferMode::Sequenced:
        ReceiptMessage(ZeroMove(message), Receipt::MAYBE);
        break;
      }
      break;

    case ACKState::NAKd:
      // NAK
      Assert(!message.IsReliable());
      ReceiptMessage(ZeroMove(message), Receipt::NAK);
      break;
    } // (Receipt message based on ACK state)
  } // (For all receipted messages in the given packet)
}

void LinkOutbox::ACKSentPacket(const ArraySet<OutPacket>::iterator& sentPacketIter, TimeMs ACKTime)
{
  // Update RTT
  mLink->UpdateRoundTripTime(GetDuration(sentPacketIter->GetSendTime(), ACKTime),
                            mLink->GetFloorRoundTripTime());

  // Acknowledge packet
  AcknowledgePacket(*sentPacketIter, ACKState::ACKd);

  // Remove packet
  mSentPackets.Erase(sentPacketIter);
}
void LinkOutbox::NAKSentPacket(ArraySet<OutPacket>::iterator& sentPacketIter)
{
  // Acknowledge packet
  AcknowledgePacket(*sentPacketIter, ACKState::NAKd);

  // Remove unreliable messages
  RemoveUnreliableMessages(*sentPacketIter);

  // Remove expired messages
  RemoveExpiredMessages(*sentPacketIter);

  // Still has messages?
  if(sentPacketIter->HasMessages())
    mResendPackets.PushBack(ZeroMove(*sentPacketIter)); // Move to resend packets

  // Remove packet
  sentPacketIter = mSentPackets.Erase(sentPacketIter);
}

void LinkOutbox::SendPacket(MoveReference<OutPacket> packet)
{
  // Send packet now
  mLink->SendPacket(*packet);
  mLastSendTime = mLink->GetLocalTime();
  packet->SetSendTime(mLastSendTime);

  // Store sent packet for later acknowledgement
  mSentPackets.Insert(ZeroMove(packet));
}

MessageReceiptId LinkOutbox::AcquireNextReceiptID()
{
  // Avoid invalid receipt ID upon wrap around
  if(mNextReceiptID == 0)
   ++mNextReceiptID;

  return mNextReceiptID++;
}
TimeMs LinkOutbox::GetLastSendDuration() const
{
  return GetDuration(mLastSendTime, mLink->GetLocalTime());
}

MessageReceiptId LinkOutbox::PushMessage(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime, bool isProtocol)
{
  // Not a protocol send?
  if(!isProtocol)
  {
    // Link not connected?
    if(mLink->GetState() != LinkState::Connected)
    {
      // Failure
      status.SetFailed("Link is not connected to the remote peer");
      return 0;
    }
  }

  // Message data too large?
  if(message->GetData().GetBitsWritten() > MaxMessageWholeDataBits)
  {
    // Failure
    status.SetFailed("Message data is too large");
    return 0;
  }

  // Get outgoing message channel
  OutMessageChannel* channel = nullptr;

  // Default (zero) channel?
  if(channelId == 0)
  {
    // Get default channel
    channel = &mDefaultChannel;
  }
  // Non-default (non-zero) channel?
  else
  {
    // Get previously opened channel
    channel = mChannels.FindPointer(channelId);
    if(!channel) // Unable?
    {
      // Failure
      status.SetFailed("Outgoing message channel is not open");
      return 0;
    }
  }

  // Acquire new message sequence ID
  MessageSequenceId sequenceId = channel->AcquireNextSequenceId();

  // Sending message over an ordered channel?
  TransferMode::Enum transferMode = channel->GetTransferMode();
  if(transferMode == TransferMode::Ordered)
    reliable = true; // Force message to be reliable

  // Receipt requested?
  MessageReceiptId receiptId = 0;
  if(receipt)
  {
    // Acquire new message receipt ID
    receiptId = AcquireNextReceiptID();
  }

  // Reset message data read cursor (just in case it was touched)
  message->GetData().ClearBitsRead();

  // Push new outgoing message to be sent later
  mOutMessages.Insert(OutMessagePtr(new OutMessage(ZeroMove(message), reliable, channelId, sequenceId, transferMode, receiptId, priority, lifetime, mLink->GetLocalTime())));

  // Success
  return receiptId;
}
bool LinkOutbox::WriteMessageToPacket(OutPacket& packet, Bits& remBits, OutMessage& message, OutPacket* prevPacket)
{
  TimeMs now = mLink->GetLocalTime();

  // Link not connected and this is a custom type, or message is not a fragment and has expired?
  if(mLink->GetState() != LinkState::Connected && message.IsCustomType()
  || !message.IsFragment() && message.HasExpired(now))
  {
    // Receipt expired message
    if(message.IsReceipted())
      ReceiptMessage(ZeroMove(message), Receipt::EXPIRED);

    // Done with this message
    return true;
  }

  // Attempt to write message to packet
  switch(WriteMessage(packet, message, remBits, prevPacket ? true : false))
  {
  case PacketWriteResult::NotDone_Fragment: // Fragment written?
    RecordFragmentReceipt(packet, message, prevPacket);
  case PacketWriteResult::NotDone_None:     // Nothing written?

    // Not done with this Message
    return false;

  case PacketWriteResult::Done_Fragment: // Final fragment written?
    RecordFragmentReceipt(packet, message, prevPacket);
  case PacketWriteResult::Done_Whole:    // Whole message written?
  case PacketWriteResult::Done_Rejected: // Message rejected?
  default:

    // Done with this message
    return true;
  }
}
PacketWriteResult::Enum LinkOutbox::WriteMessage(OutPacket& packet, OutMessage& message, Bits& remBits, bool isResendMessage)
{
  // Get message size
  Bits messageSize = message.GetTotalBits();

  // Is a resend fragment message?
  if(isResendMessage && message.IsFragment())
  {
    // Whole fragment message fits?
    if(messageSize <= remBits)
    {
      // Update remaining bits
      remBits -= messageSize;

      // Write fragment message
      packet.mMessages.PushBack(ZeroMove(message));
      return PacketWriteResult::Done_Fragment;
    }
    // Whole fragment message does not fit?
    else
    {
      // This is the first custom or protocol message being written (we should force write)?
      if(message.IsCustomType() ? !packet.HasCustomMessages() : !packet.HasProtocolMessages())
      {
        // TODO: Refactor message fragmentation to use segments,
        //       at the moment we write a resend fragment as is,
        //       even if it doesn't fit into the packet. This will
        //       only work until we start using the no-fragment option!

        // Update remaining bits
        Assert(messageSize > remBits);
        remBits = 0; // (Yes, we're exceeding remBits. This is only a temporary solution! See note above.)

        // Write fragment message
        packet.mMessages.PushBack(ZeroMove(message));
        return PacketWriteResult::Done_Fragment;
      }
      else
      {
        // Write nothing
        return PacketWriteResult::NotDone_None;
      }
    }
  }

  // Whole message fits?
  if(messageSize <= remBits)
  {
    // Should not send this message?
    if(!isResendMessage && !ShouldSendMessage(message)) // Stop?
      return PacketWriteResult::Done_Rejected;

    // Update remaining bits
    remBits -= messageSize;

    // Fragment?
    if(message.IsFragment())
    {
      // Make final fragment
      message.mData.TrimFront();
      message.mIsFinalFragment = true;

      // Write final fragment message
      packet.mMessages.PushBack(ZeroMove(message));
      return PacketWriteResult::Done_Fragment;
    }
    else
    {
      // Write whole message
      packet.mMessages.PushBack(ZeroMove(message));
      return PacketWriteResult::Done_Whole;
    }
  }
  // Whole message does not fit?
  else
  {
    // Enough space left for a fragment
    // And either:
    // This message is already a fragment,
    // or This is the first custom or protocol message being written (we should force fragmentation)?
    Bits messageAsFragmentHeaderSize = message.GetHeaderBits(true);
    if((remBits >= (MinMessageFragmentDataBits + messageAsFragmentHeaderSize)) &&
      (message.IsFragment()
    || (message.IsCustomType() ? !packet.HasCustomMessages() : !packet.HasProtocolMessages())))
    {
      // Should not send this message?
      if(!isResendMessage && !ShouldSendMessage(message)) // Stop?
        return PacketWriteResult::Done_Rejected;

      // Take message fragment
      OutMessage fragment     = message.TakeFragment(remBits - messageAsFragmentHeaderSize);
      Bits       fragmentSize = fragment.GetTotalBits();

      // Update remaining bits
      remBits -= fragmentSize;
      Assert(remBits == 0);

      // Write fragment message
      packet.mMessages.PushBack(ZeroMove(fragment));
      return PacketWriteResult::NotDone_Fragment;
    }
    else
    {
      // Write nothing
      return PacketWriteResult::NotDone_None;
    }
  }
}
void LinkOutbox::Update(const ACKArray& remoteACKs, const NAKArray& remoteNAKs)
{
  TimeMs now = mLink->GetLocalTime();

  //
  // Handle Packet Acknowledgements
  //

  // Handle explicit remote ACKs
  forRange(PacketSequenceId remoteACK, remoteACKs.All())
  {
    // Find remoteACK packet
    ArraySet<OutPacket>::iterator iter = mSentPackets.FindIterator(remoteACK);
    if(iter != mSentPackets.End()) // Found?
      ACKSentPacket(iter, now);
  }

  // Handle explicit remote NAKs
  forRange(PacketSequenceId remoteNAK, remoteNAKs.All())
  {
    // Find remoteNAK packet
    ArraySet<OutPacket>::iterator iter = mSentPackets.FindIterator(remoteNAK);
    if(iter != mSentPackets.End()) // Found?
      NAKSentPacket(iter);
  }

  // Handle implicit (assumed) remote NAKs
  for(ArraySet<OutPacket>::iterator iter = mSentPackets.Begin(); iter != mSentPackets.End(); )
  {
    // Packet was sent over RTT * NAKFactor ago?
    if(GetDuration(iter->GetSendTime(), now) > mLink->GetAvgInternalRoundTripTime() * mLink->GetPacketNAKFactor())
      NAKSentPacket(iter); // Assume NAK and advance
    else
      ++iter; // Advance
  }

  // TODO: This is all going to change once AIMD is implemented

  //
  // Generate Outgoing Packets
  //

  // Match the configured send rate
  uint outPacketCount = uint(GetDuration(mLastSendTime, now) / RATE_TO_INTERVAL(mLink->GetSendRate()));

  // Generate outgoing packets
  while(outPacketCount)
  {
    //
    // Create Packet
    //
    OutPacket  newPacket(mLink->GetTheirIpAddress(), false, ++mNextSequenceId);
    const Bits packetDataSizeLimit = BYTES_TO_BITS(mLink->GetPacketDataBytes());
    Bits       remBits             = packetDataSizeLimit;

    //
    // Write Resend Messages
    //

    // For all resend packets
    for(Array<OutPacket>::iterator resendPacketIter = mResendPackets.Begin(); resendPacketIter != mResendPackets.End(); )
    {
      // Get resend packet
      OutPacket& resendPacket = *resendPacketIter;

      // Write resend messages
      Array<OutMessage>& resendMessages = resendPacket.GetMessages();
      for(Array<OutMessage>::iterator resendMessageIter = resendMessages.Begin(); resendMessageIter != resendMessages.End(); )
      {
        // Packet full?
        if(remBits < MinMessageHeaderBits)
          goto LinkOutbox_Update_DoneWritingResendMessages; // Exit routine

        // Get resend message
        OutMessage& message = *resendMessageIter;

        // Write message to packet, done with this message?
        if(WriteMessageToPacket(newPacket, remBits, message, &resendPacket))
          resendMessageIter = resendMessages.Erase(resendMessageIter);  // Erase and advance
        else
          ++resendMessageIter; // Advance

      } // (Write resend messages)

      // Resend packet is empty?
      if(resendMessages.Empty())
        resendPacketIter = mResendPackets.Erase(resendPacketIter); // Erase and advance
      else
        ++resendPacketIter; // Advance

    } // (For all resend packets)
    LinkOutbox_Update_DoneWritingResendMessages:

    //
    // Write New Messages
    //

    // For all new queued outgoing messages
    for(OutMessages::iterator iter = mOutMessages.End(); iter != mOutMessages.Begin(); ) // Note: We are iterating in reverse here
    {
      // Packet full?
      if(remBits < MinMessageHeaderBits)
        break;

      // Get message
      OutMessage& message = **(iter - 1);

      // Write message to packet, done with this message?
      if(WriteMessageToPacket(newPacket, remBits, message))
        iter = mOutMessages.Erase(iter - 1); // Erase and advance
      else
        --iter; // Advance

    } // (For all new queued outgoing messages)

    // Packet has messages or it's time to send a heartbeat packet?
    if(newPacket.HasMessages()
    || GetDuration(mLastSendTime, now) > RATE_TO_INTERVAL(mLink->GetHeartbeatPacketRate()))
      SendPacket(ZeroMove(newPacket)); // Send new Packet
    else
      --mNextSequenceId; // Nothing to send, revert unused sequence ID

    // Advance
    --outPacketCount;

  } // (Generate outgoing packets)
}

void LinkOutbox::RemoveUnreliableMessages(OutPacket& packet)
{
  // For all messages in the packet
  Array<OutMessage>& messages = packet.GetMessages();
  for(Array<OutMessage>::iterator iter = messages.Begin(); iter != messages.End(); )
  {
    // Is unreliable message?
    if(!iter->IsReliable())
      iter = messages.Erase(iter); // Erase and advance
    else
      ++iter; // Advance
  }
}
void LinkOutbox::RemoveExpiredMessages(OutPacket& packet)
{
  TimeMs now = mLink->GetLocalTime();

  // For all messages in the packet
  Array<OutMessage>& messages = packet.GetMessages();
  for(Array<OutMessage>::iterator iter = messages.Begin(); iter != messages.End(); )
  {
    // Link not connected and this is a custom type, or message is not a fragment and has expired?
    if(mLink->GetState() != LinkState::Connected && iter->IsCustomType()
    || !iter->IsFragment() && iter->HasExpired(now))
    {
      // Receipt expired message
      if(iter->IsReceipted())
        ReceiptMessage(ZeroMove(*iter), Receipt::EXPIRED);

      // Erase and advance
      iter = messages.Erase(iter);
    }
    else
      ++iter; // Advance
  }
}

void LinkOutbox::ReceiptMessage(MoveReference<OutMessage> message, Receipt::Enum receipt)
{
  Assert(message->IsReceipted());

  // [Link Plugin Event] Stop?
  if(!mLink->PluginEventOnMessageReceipt(*message, receipt))
    return;

  // Attempt to receipt the message as a plugin message
  if(mLink->AttemptPluginMessageReceipt(ZeroMove(message), receipt)) // Successful?
    return;

  // [Link Event]
  mLink->LinkEventReceipt(message->GetReceiptID(), receipt, message->IsCustomType());
}

bool LinkOutbox::ShouldSendMessage(OutMessage& message)
{
  // [Link Plugin Event] Stop?
  if(!mLink->PluginEventOnMessageSend(message))
    return false;

  // [Link Plugin Event] Stop?
  if(!mLink->PluginEventOnPluginMessageSend(message))
    return false;

  // Continue
  return true;
}

} // namespace Zero
