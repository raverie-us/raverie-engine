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
//                                  RawPacket                                      //
//---------------------------------------------------------------------------------//

RawPacket::RawPacket()
  : mContainsEventMessage(false),
    mIpAddress(),
    mData()
{
}

RawPacket::RawPacket(const RawPacket& rhs)
  : mContainsEventMessage(rhs.mContainsEventMessage),
    mIpAddress(rhs.mIpAddress),
    mData(rhs.mData)
{
}

RawPacket::RawPacket(MoveReference<RawPacket> rhs)
  : mContainsEventMessage(rhs->mContainsEventMessage),
    mIpAddress(rhs->mIpAddress),
    mData(ZeroMove(rhs->mData))
{
}

RawPacket& RawPacket::operator =(const RawPacket& rhs)
{
  mContainsEventMessage = rhs.mContainsEventMessage;
  mIpAddress            = rhs.mIpAddress;
  mData                 = rhs.mData;

  return *this;
}

RawPacket& RawPacket::operator =(MoveReference<RawPacket> rhs)
{
  mContainsEventMessage = rhs->mContainsEventMessage;
  mIpAddress            = rhs->mIpAddress;
  mData                 = ZeroMove(rhs->mData);

  return *this;
}

//---------------------------------------------------------------------------------//
//                                    Packet                                       //
//---------------------------------------------------------------------------------//

Packet::Packet(const IpAddress& ipAddress, bool isStandalone, PacketSequenceId sequenceId)
  : mIpAddress(ipAddress),
    mIsStandalone(isStandalone),
    mSequenceId(sequenceId)
{
  if(mIsStandalone)
    Assert(mSequenceId == 0);
}

Packet::Packet(const Packet& rhs)
  : mIpAddress(rhs.mIpAddress),
    mIsStandalone(rhs.mIsStandalone),
    mSequenceId(rhs.mSequenceId)
{
}

Packet& Packet::operator =(const Packet& rhs)
{
  mIpAddress    = rhs.mIpAddress;
  mIsStandalone = rhs.mIsStandalone;
  mSequenceId   = rhs.mSequenceId;

  return *this;
}

bool Packet::operator ==(const Packet& rhs) const
{
  return GetSequenceId() == rhs.GetSequenceId();
}
bool Packet::operator !=(const Packet& rhs) const
{
  return GetSequenceId() != rhs.GetSequenceId();
}
bool Packet::operator  <(const Packet& rhs) const
{
  return GetSequenceId() < rhs.GetSequenceId();
}
bool Packet::operator ==(PacketSequenceId rhs) const
{
  return GetSequenceId() == rhs;
}
bool Packet::operator !=(PacketSequenceId rhs) const
{
  return GetSequenceId() != rhs;
}
bool Packet::operator  <(PacketSequenceId rhs) const
{
  return GetSequenceId() < rhs;
}

//
// Member Functions
//

bool Packet::IsStandalone() const
{
  return mIsStandalone;
}

PacketSequenceId Packet::GetSequenceId() const
{
  return mSequenceId;
}

Bits Packet::GetHeaderBits() const
{
  return IsStandalone() ? MinPacketHeaderBits
                        : MaxPacketHeaderBits;
}

bool operator ==(PacketSequenceId lhs, const Packet& rhs)
{
  return lhs == rhs.GetSequenceId();
}
bool operator  <(PacketSequenceId lhs, const Packet& rhs)
{
  return lhs < rhs.GetSequenceId();
}

//---------------------------------------------------------------------------------//
//                                  OutPacket                                      //
//---------------------------------------------------------------------------------//

OutPacket::OutPacket(const IpAddress& destination, bool isStandalone, PacketSequenceId sequenceId)
  : Packet(destination, isStandalone, sequenceId),
    mMessages(),
    mSendTime(0)
{
}

OutPacket::OutPacket(const OutPacket& rhs)
  : Packet(rhs),
    mMessages(rhs.mMessages),
    mSendTime(rhs.mSendTime)
{
}

OutPacket::OutPacket(MoveReference<OutPacket> rhs)
  : Packet(*rhs),
    mMessages(ZeroMove(rhs->mMessages)),
    mSendTime(rhs->mSendTime)
{
}

OutPacket& OutPacket::operator =(const OutPacket& rhs)
{
  Packet::operator=(rhs);
  mMessages       = rhs.mMessages;
  mSendTime       = rhs.mSendTime;

  return *this;
}

OutPacket& OutPacket::operator =(MoveReference<OutPacket> rhs)
{
  Packet::operator=(*rhs);
  mMessages       = ZeroMove(rhs->mMessages);
  mSendTime       = rhs->mSendTime;

  return *this;
}

//
// Member Functions
//

const IpAddress& OutPacket::GetDestinationIpAddress() const
{
  return mIpAddress;
}

bool OutPacket::HasMessages() const
{
  return !mMessages.Empty();
}
bool OutPacket::HasCustomMessages() const
{
  forRange(Message& message, mMessages.All())
    if(message.IsCustomType())
      return true;
  return false;
}
bool OutPacket::HasProtocolMessages() const
{
  forRange(Message& message, mMessages.All())
    if(!message.IsCustomType())
      return true;
  return false;
}

void OutPacket::SetSendTime(TimeMs sendTime)
{
  mSendTime = sendTime;
}
TimeMs OutPacket::GetSendTime() const
{
  return mSendTime;
}

Array<OutMessage>& OutPacket::GetMessages()
{
  return mMessages;
}

Bits OutPacket::GetTotalBits() const
{
  // Packet header size
  Bits result = GetHeaderBits();

  // Message sizes
  forRange(Message& message, mMessages.All())
    result += message.GetTotalBits();

  return result;
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, OutPacket& outPacket)
{
  // Write operation only
  Assert(direction == SerializeDirection::Write);

  const Bits bitsWrittenStart = bitStream.GetBitsWritten();

  //
  // Write Packet Header
  //

  // Write protocol ID
  ProtocolId protocolId = Peer::GetProtocolId();
  bitStream.Write(protocolId);

  // Write 'Is standalone?' flag
  bitStream.Write(outPacket.mIsStandalone);

  // Not a standalone packet?
  if(!outPacket.mIsStandalone)
  {
    // Write packet sequence ID
    bitStream.Write(outPacket.mSequenceId);
  }

  // (Packet header size should be correct)
  Assert((bitStream.GetBitsWritten() - bitsWrittenStart) == outPacket.GetHeaderBits());

  //
  // Write Messages
  //
  forRange(Message& message, outPacket.mMessages.All())
    bitStream.Write(message);

  // Success
  return bitStream.GetBitsWritten() - bitsWrittenStart;
}

//---------------------------------------------------------------------------------//
//                                   InPacket                                      //
//---------------------------------------------------------------------------------//

InPacket::InPacket(const IpAddress& source)
  : Packet(source),
    mMessages()
{
}

InPacket::InPacket(const InPacket& rhs)
  : Packet(rhs),
    mMessages(rhs.mMessages)
{
}

InPacket::InPacket(MoveReference<InPacket> rhs)
  : Packet(*rhs),
    mMessages(ZeroMove(rhs->mMessages))
{
}

InPacket& InPacket::operator =(const InPacket& rhs)
{
  Packet::operator=(rhs);
  mMessages       = rhs.mMessages;

  return *this;
}

InPacket& InPacket::operator =(MoveReference<InPacket> rhs)
{
  Packet::operator=(*rhs);
  mMessages       = ZeroMove(rhs->mMessages);

  return *this;
}

//
// Member Functions
//

const IpAddress& InPacket::GetSourceIpAddress() const
{
  return mIpAddress;
}

Array<Message>& InPacket::GetMessages()
{
  return mMessages;
}

Bits InPacket::GetTotalBits() const
{
  // Packet header size
  Bits result = GetHeaderBits();

  // Message sizes
  forRange(Message& message, mMessages.All())
    result += message.GetTotalBits();

  return result;
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, InPacket& inPacket)
{
  // Read operation only
  Assert(direction == SerializeDirection::Read);

  const Bits bitsReadStart = bitStream.GetBitsRead();

  //
  // Read Packet Header
  //

  // Read protocol ID
  ProtocolId protocolId;
  ReturnIf(!bitStream.Read(protocolId), 0);

  // (Their protocol ID should match ours, as we discard invalid packets at the raw packet receive stage)
  Assert(protocolId == Peer::GetProtocolId());

  // Read 'Is standalone?' flag
  ReturnIf(!bitStream.Read(inPacket.mIsStandalone), 0);

  // Not a standalone packet?
  if(!inPacket.mIsStandalone)
  {
    // Read packet sequence ID
    ReturnIf(!bitStream.Read(inPacket.mSequenceId), 0);
  }

  // (Packet header size should be correct)
  Assert((bitStream.GetBitsRead() - bitsReadStart) == inPacket.GetHeaderBits());

  //
  // Read Messages
  //

  // Enough bits left to possibly read another message?
  while(bitStream.GetBitsUnread() >= MinMessageHeaderBits)
  {
    // Read a message
    Message message(ProtocolMessageType::Invalid);
    if(!bitStream.Read(message)) // Unable?
    {
      Assert(false);
      break;
    }

    // Push message
    inPacket.mMessages.PushBack(ZeroMove(message));
  }

  // Success
  return bitStream.GetBitsRead() - bitsReadStart;
}

} // namespace Zero
