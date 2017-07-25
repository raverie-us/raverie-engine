///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

LinkInbox::LinkInbox(PeerLink* link)
/// Operating Data
  : mLink(link),

  /// Packet Data
  mReceivedPackets(),
  mLastReceiveTime(link->GetLocalTime()),
  mIncomingPacketSequence(),
  mLastOutPacketSequenceHistorySendTime(mLastReceiveTime),
  mLastOutPacketSequenceHistoryNESQ(0),
  mLastInPacketSequenceHistoryNESQ(0),

  /// Channel Data
  mCustomDefaultChannel(0, TransferMode::Immediate),
  mProtocolDefaultChannel(0, TransferMode::Immediate),
  mChannels(),

  /// Message Data
  mReleasedCustomMessages(),
  mReleasedProtocolMessages()
{
}

LinkInbox::LinkInbox(MoveReference<LinkInbox> rhs)
/// Operating Data
  : mLink(rhs->mLink),

  /// Packet Data
  mReceivedPackets(ZeroMove(rhs->mReceivedPackets)),
  mLastReceiveTime(rhs->mLastReceiveTime),
  mIncomingPacketSequence(ZeroMove(rhs->mIncomingPacketSequence)),
  mLastOutPacketSequenceHistorySendTime(rhs->mLastOutPacketSequenceHistorySendTime),
  mLastOutPacketSequenceHistoryNESQ(rhs->mLastOutPacketSequenceHistoryNESQ),
  mLastInPacketSequenceHistoryNESQ(rhs->mLastInPacketSequenceHistoryNESQ),

  /// Channel Data
  mCustomDefaultChannel(ZeroMove(rhs->mCustomDefaultChannel)),
  mProtocolDefaultChannel(ZeroMove(rhs->mProtocolDefaultChannel)),
  mChannels(ZeroMove(rhs->mChannels)),

  /// Message Data
  mReleasedCustomMessages(ZeroMove(rhs->mReleasedCustomMessages)),
  mReleasedProtocolMessages(ZeroMove(rhs->mReleasedProtocolMessages))
{
}

LinkInbox& LinkInbox::operator =(MoveReference<LinkInbox> rhs)
{
  /// Operating Data
  mLink = rhs->mLink;

  /// Packet Data
  mReceivedPackets = ZeroMove(rhs->mReceivedPackets);
  mLastReceiveTime = rhs->mLastReceiveTime;
  mIncomingPacketSequence = ZeroMove(rhs->mIncomingPacketSequence);
  mLastOutPacketSequenceHistorySendTime = rhs->mLastOutPacketSequenceHistorySendTime;
  mLastOutPacketSequenceHistoryNESQ = rhs->mLastOutPacketSequenceHistoryNESQ;
  mLastInPacketSequenceHistoryNESQ = rhs->mLastInPacketSequenceHistoryNESQ;

  /// Channel Data
  mCustomDefaultChannel = ZeroMove(rhs->mCustomDefaultChannel);
  mProtocolDefaultChannel = ZeroMove(rhs->mProtocolDefaultChannel);
  mChannels = ZeroMove(rhs->mChannels);

  /// Message Data
  mReleasedCustomMessages = ZeroMove(rhs->mReleasedCustomMessages);
  mReleasedProtocolMessages = ZeroMove(rhs->mReleasedProtocolMessages);

  return *this;
}

//
// Incoming Message Channel Management
//

InMessageChannel* LinkInbox::GetIncomingChannel(MessageChannelId channelId) const
{
  return mChannels.FindPointer(channelId);
}
ArraySet<InMessageChannel>::range LinkInbox::GetIncomingChannels() const
{
  return mChannels.All();
}
uint LinkInbox::GetIncomingChannelCount() const
{
  return mChannels.Size();
}

//
// Member Functions
//

void LinkInbox::ReceivePacket(MoveReference<InPacket> packet)
{
  // For every message
  forRange(Message& message, packet->GetMessages().All())
  {
    // Message has an accurate remote timestamp?
    TimeMs remoteTimestamp = 0;
    if (message.HasTimestamp())
    {
      // Get accurate remote timestamp specified by their remote peer
      remoteTimestamp = message.GetTimestamp();
    }
    // Message does not have an accurate remote timestamp?
    else
    {
      // Get inaccurate remote timestamp estimated by our local peer
      remoteTimestamp = mLink->GetRemoteTime();
    }

    // Convert to local timestamp
    TimeMs localTimestamp = mLink->RemoteToLocalTime(remoteTimestamp);

    // Set local timestamp (later logic assumes timestamps are always in local time)
    message.SetTimestamp(localTimestamp);
  }

  // Packet has messages to be processed?
  if (!packet->GetMessages().Empty())
  {
    // Process packet next inbox update
    mReceivedPackets.PushBack(ZeroMove(packet));
  }

  // Set last packet receive time
  mLastReceiveTime = mLink->GetLocalTime();
}

void LinkInbox::Update(ACKArray& remoteACKs, NAKArray& remoteNAKs)
{
  //
  // Process Incoming Packets
  //

  // For every packet
  ArraySet<MessageChannelId> updatedChannels;
  forRange(InPacket& packet, mReceivedPackets.All())
  {
    //
    // Process Packet
    //

    // Attempt to add packet to sequence
    if (!mIncomingPacketSequence.Add(packet.GetSequenceId())) // Is network-duplicate?
      continue; // Ignore packet

                //
                // Process Incoming Messages
                //

                // For every message
    forRange(Message& message, packet.GetMessages().All())
    {
      MessageChannelId  channelId = message.GetChannelId();

      //
      // Get Channel
      //

      // Get incoming message channel
      InMessageChannel* channel = nullptr;

      // Default (zero) channel?
      if (channelId == 0)
      {
        // Custom message type?
        if (message.IsCustomType())
          channel = &mCustomDefaultChannel;
        // Protocol message type?
        else
          channel = &mProtocolDefaultChannel;
      }
      // Non-default (non-zero) channel?
      else
      {
        // Get previously opened channel (if it exists)
        channel = mChannels.FindPointer(channelId);
      }
      Assert(channel ? channel->GetChannelId() == channelId : true);

      //
      // Process Message
      //

      // Is resend-duplicate?
      if (channel && channel->IsDuplicate(message))
        continue; // Ignore message

                  // Process message
      switch (message.GetType())
      {
      case ProtocolMessageType::ChannelOpened:
      {
        // Channel already open?
        if (channel)
          break; // Ignore message

                 // [Link Plugin Event] Continue?
        if (mLink->PluginEventOnIncomingChannelOpen(channelId))
        {
          // Read channel opened message data
          ChannelOpenedData channelOpenedData;
          if (!message.GetData().Read(channelOpenedData)) // Unable?
            break; // Ignore message

                   // Add new incoming message channel
          InMessageChannel newChannel(channelId, channelOpenedData.mTransferMode);
          ArraySet<InMessageChannel>::pointer_bool_pair result = mChannels.Insert(ZeroMove(newChannel));
          Assert(result.second); // (Insertion should have succeeded)

                                 // Open new incoming message channel
          result.first->Open();

          // [Link Event]
          mLink->LinkEventIncomingChannelOpened(channelId, channelOpenedData.mTransferMode);
        }
      }
      break;

      case ProtocolMessageType::ChannelClosed:
      {
        // Channel not open or already closed?
        if (!channel || channel->IsClosed())
          break; // Ignore message

                 // [Link Plugin Event]
        mLink->PluginEventOnIncomingChannelClose(channelId);

        // Close incoming message channel
        channel->Close(message.GetSequenceId());
      }
      break;

      case ProtocolMessageType::PacketSequenceHistory:
      {
        // Read packet sequence history protocol message data
        PacketSequenceHistoryData packetSequenceHistoryData;
        if (!message.GetData().Read(packetSequenceHistoryData)) // Unable?
          break; // Ignore message

                 // Does this packet sequence history record contain newer data?
                 // (Is this record more recent than the last record we received?)
        PacketSequenceHistory& packetRecord = packetSequenceHistoryData;
        if (packetRecord.mNext > mLastInPacketSequenceHistoryNESQ)
        {
          // Process packet sequence history record
          PacketSequenceId offset = 0;
          while (packetRecord.mHistory.GetBitsUnread())
          {
            // Determine packet sequence ID based on bit offset
            PacketSequenceId packetSequenceId = packetRecord.mNext - (++offset);

            // Read bit value to determine if it was ACKd or NAKd
            bool packetACKd;
            packetRecord.mHistory.ReadBit(packetACKd);

            // Packet was ACKd?
            if (packetACKd)
              remoteACKs.PushBack(packetSequenceId); // Add to ACKs
                                                      // Packet was NAKd?
            else
              remoteNAKs.PushBack(packetSequenceId); // Add to NAKs
          }
          mLastInPacketSequenceHistoryNESQ = packetRecord.mNext;
        }
      }
      break;

      default:
      {
        // Channel not open?
        if (!channel)
          break; // Ignore message

                 // Attempt to push message into channel for later release
        channel->Push(ZeroMove(message));
      }
      break;
      }

      //
      // Update Channel
      //

      // (Take released messages and erase the channel if it's ready to be deleted)

      // Non-default (non-zero) channel?
      if (channelId != 0)
      {
        // Find updated channel
        ArraySet<InMessageChannel>::iterator updatedChannel = mChannels.FindIterator(channelId);
        if (updatedChannel != mChannels.End()) // Found?
        {
          // Release pending messages
          Array<Message> releasedMessages = updatedChannel->Release();
          ReleaseCustomMessages(releasedMessages);

          // Channel ready to delete?
          if (updatedChannel->ReadyToDelete())
          {
            // [Link Event]
            mLink->LinkEventIncomingChannelClosed(channelId);

            // Erase channel
            mChannels.Erase(updatedChannel);
          }
        }
      }
      // Default (zero) channel?
      else
      {
        // Release pending custom messages
        Array<Message> customReleasedMessages = mCustomDefaultChannel.Release();
        ReleaseCustomMessages(customReleasedMessages);

        // Release pending protocol messages
        Array<Message> protocolReleasedMessages = mProtocolDefaultChannel.Release();
        ReleaseProtocolMessages(protocolReleasedMessages);
      }

    } // (For every message)

  } // (For every packet)
  mReceivedPackets.Clear();

  //
  // Generate Outgoing Messages
  //

  // Time to send a packet sequence history update?
  TimeMs now = mLink->GetLocalTime();
  if ((now - mLastOutPacketSequenceHistorySendTime) > (mLink->GetSendRate() * mLink->GetPacketSequenceHistoryRateFactor()))
  {
    // Get packet sequence history data
    PacketSequenceHistoryData packetSequenceHistoryData = mIncomingPacketSequence.GetSequenceHistory(uint(mLink->GetSendRate() * mLink->GetPacketSequenceHistoryRangeFactor()));

    // Record contains newer data?
    if (packetSequenceHistoryData.mNext > mLastOutPacketSequenceHistoryNESQ)
    {
      // Create packet sequence history message
      Message message(ProtocolMessageType::PacketSequenceHistory);
      message.GetData().Write(packetSequenceHistoryData);

      // Send packet sequence history message
      Status status;
      mLink->SendInternal(status, ZeroMove(message), false);
      mLastOutPacketSequenceHistoryNESQ = packetSequenceHistoryData.mNext;
      mLastOutPacketSequenceHistorySendTime = now;
    }
  }
}

void LinkInbox::ReleaseCustomMessages(Array<Message>& messages)
{
  // For all messages
  forRange(Message& message, messages.All())
  {
    // Should be a custom message
    Assert(message.IsCustomType());

    // [Link Plugin Event] Stop?
    if (!mLink->PluginEventOnMessageReceive(message))
      continue;

    // Release custom message
    mReleasedCustomMessages.PushBack(ZeroMove(message));
  }
  messages.Clear();
}
void LinkInbox::ReleaseProtocolMessages(Array<Message>& messages)
{
  // For all messages
  forRange(Message& message, messages.All())
  {
    // Should be a protocol message
    Assert(!message.IsCustomType());

    // [Link Plugin Event] Stop?
    if (!mLink->PluginEventOnMessageReceive(message))
      continue;

    // Release protocol message
    mReleasedProtocolMessages.PushBack(ZeroMove(message));
  }
  messages.Clear();
}

TimeMs LinkInbox::GetLastReceiveDuration() const
{
  return GetDuration(mLastReceiveTime, mLink->GetLocalTime());
}

void LinkInbox::PushUserEventMessage(MoveReference<Message> message)
{
  // Release event message for the user
  mLink->ProcessReceivedCustomMessage(*message, true);
}
void LinkInbox::PushProtocolEventMessage(MoveReference<Message> message)
{
  // Release event message for the protocol
  mReleasedProtocolMessages.PushBack(ZeroMove(message));
}

} // namespace Zero
