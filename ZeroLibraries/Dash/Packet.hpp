///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  RawPacket                                      //
//---------------------------------------------------------------------------------//

/// Unprocessed network data unit
class RawPacket
{
public:
  /// Default Constructor
  RawPacket();

  /// Copy Constructor
  RawPacket(const RawPacket& rhs);

  /// Move Constructor
  RawPacket(MoveReference<RawPacket> rhs);

  /// Copy Assignment Operator
  RawPacket& operator =(const RawPacket& rhs);

  /// Move Assignment Operator
  RawPacket& operator =(MoveReference<RawPacket> rhs);

  /// Contains a peer event message?
  bool      mContainsEventMessage;
  /// Source/Destination IP address
  IpAddress mIpAddress;
  /// Packet data stream
  BitStream mData;
};

/// RawPacket Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<RawPacket>
{
  static inline void MoveWithoutDestruction(RawPacket* dest, RawPacket* source)
  {
    new(dest) RawPacket(ZeroMove(*source));
  }
};

//---------------------------------------------------------------------------------//
//                                    Packet                                       //
//---------------------------------------------------------------------------------//

/// Network data unit
class Packet
{
public:
  /// Constructor
  Packet(const IpAddress& ipAddress, bool isStandalone = true, PacketSequenceId sequenceId = 0);

  /// Copy Constructor
  Packet(const Packet& rhs);

  /// Copy Assignment Operator
  Packet& operator =(const Packet& rhs);

  /// Comparison Operators (compares packet sequence IDs)
  bool operator ==(const Packet& rhs) const;
  bool operator !=(const Packet& rhs) const;
  bool operator  <(const Packet& rhs) const;
  bool operator ==(PacketSequenceId rhs) const;
  bool operator !=(PacketSequenceId rhs) const;
  bool operator  <(PacketSequenceId rhs) const;

  //
  // Member Functions
  //

  /// Returns true if this is a standalone packet (not sent by a link), else false
  bool IsStandalone() const;

  /// Returns the packet's sequence ID
  PacketSequenceId GetSequenceId() const;

  /// Returns the packet's current header bits as it currently would result in when serialized
  Bits GetHeaderBits() const;

  /// Source/Destination IP address
  IpAddress        mIpAddress;
  /// Is a standalone packet?
  bool             mIsStandalone;
  /// Packet sequence ID
  PacketSequenceId mSequenceId;
};

/// Global Comparison Operators
bool operator ==(PacketSequenceId lhs, const Packet& rhs);
bool operator  <(PacketSequenceId lhs, const Packet& rhs);

/// Typedefs
typedef Array<PacketSequenceId> ACKArray;
typedef Array<PacketSequenceId> NAKArray;

//---------------------------------------------------------------------------------//
//                                  OutPacket                                      //
//---------------------------------------------------------------------------------//

/// Outgoing network data unit (packet w/ additional send information)
class OutPacket : public Packet
{
public:
  /// Constructor
  OutPacket(const IpAddress& destination = IpAddress(), bool isStandalone = true, PacketSequenceId sequenceId = 0);

  /// Copy Constructor
  OutPacket(const OutPacket& rhs);

  /// Move Constructor
  OutPacket(MoveReference<OutPacket> rhs);

  /// Copy Assignment Operator
  OutPacket& operator =(const OutPacket& rhs);

  /// Move Assignment Operator
  OutPacket& operator =(MoveReference<OutPacket> rhs);

  //
  // Member Functions
  //

  /// Returns the destination IP address
  const IpAddress& GetDestinationIpAddress() const;

  /// Returns true if the packet has messages, else false
  bool HasMessages() const;
  /// Returns true if the packet has custom messages, else false
  bool HasCustomMessages() const;
  /// Returns true if the packet has protocol messages, else false
  bool HasProtocolMessages() const;

  /// Sets the time the packet was queued for sending
  void SetSendTime(TimeMs sendTime);
  /// Returns the time the packet was queued for sending
  TimeMs GetSendTime() const;

  /// Returns the packet's contained messages
  Array<OutMessage>& GetMessages();

  /// Returns the total packet size (header size + all unread message sizes)
  Bits GetTotalBits() const;

  /// Contained messages
  Array<OutMessage> mMessages;
  /// Time the packet was queued for sending
  TimeMs            mSendTime;

  /// Friends
  friend class LinkOutbox;
  friend Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, OutPacket& outPacket);
};

/// OutPacket Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<OutPacket>
{
  static inline void MoveWithoutDestruction(OutPacket* dest, OutPacket* source)
  {
    new(dest) OutPacket(ZeroMove(*source));
  }
};

/// Serializes an outgoing packet (write only)
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, OutPacket& outPacket);

//---------------------------------------------------------------------------------//
//                                   InPacket                                      //
//---------------------------------------------------------------------------------//

/// Incoming network data unit (packet w/ additional receive information)
class InPacket : public Packet
{
public:
  /// Constructor
  InPacket(const IpAddress& source = IpAddress());

  /// Copy Constructor
  InPacket(const InPacket& rhs);

  /// Move Constructor
  InPacket(MoveReference<InPacket> rhs);

  /// Copy Assignment Operator
  InPacket& operator =(const InPacket& rhs);

  /// Move Assignment Operator
  InPacket& operator =(MoveReference<InPacket> rhs);

  //
  // Member Functions
  //

  /// Returns the source IP address
  const IpAddress& GetSourceIpAddress() const;

  /// Returns the packet's contained messages
  Array<Message>& GetMessages();

  /// Returns the total packet size (header size + all message sizes)
  Bits GetTotalBits() const;

  /// Contained messages
  Array<Message> mMessages;

  /// Friends
  friend Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, InPacket& inPacket);
};

/// InPacket Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<InPacket>
{
  static inline void MoveWithoutDestruction(InPacket* dest, InPacket* source)
  {
    new(dest) InPacket(ZeroMove(*source));
  }
};

/// Serializes an incoming packet (read only)
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, InPacket& inPacket);

//---------------------------------------------------------------------------------//
//                               Packet Constants                                  //
//---------------------------------------------------------------------------------//

/// Maximum packet size (packet header size + data)
/// Might work but it's likely the IPv4/IPv6 + UDP header will contain more options than the minimum
static const Bits  MaxPacketBits      = BYTES_TO_BITS(EthernetMtuBytes - Ipv4MinHeaderBytes - UdpHeaderBytes);
static const Bytes MaxPacketBytes     = BITS_TO_BYTES(MaxPacketBits);
/// Typical packet size (packet header size + data)
/// Should be safe over the internet
static const Bits  TypicalPacketBits  = BYTES_TO_BITS(Ipv4MinMtuBytes - Ipv4MaxHeaderBytes - UdpHeaderBytes);
static const Bytes TypicalPacketBytes = BITS_TO_BYTES(TypicalPacketBits);
/// Minimum packet size (packet header size + data)
/// Our peer protocol requires at least one minimum sized message fragment be able to fit into every packet
static const Bits  MinPacketBits      = MaxPacketHeaderBits + MaxMessageHeaderBits + MinMessageFragmentDataBits;
static const Bytes MinPacketBytes     = BITS_TO_BYTES(MinPacketBits);

/// Maximum packet data size left over for messages
static const Bits  MaxPacketDataBits      = MaxPacketBits     - MaxPacketHeaderBits;
static const Bytes MaxPacketDataBytes     = BITS_TO_BYTES(MaxPacketDataBits);
/// Typical packet data size left over for messages
static const Bits  TypicalPacketDataBits  = TypicalPacketBits - MaxPacketHeaderBits;
static const Bytes TypicalPacketDataBytes = BITS_TO_BYTES(TypicalPacketDataBits);
/// Minimum packet data size left over for messages
static const Bits  MinPacketDataBits      = MinPacketBits     - MaxPacketHeaderBits;
static const Bytes MinPacketDataBytes     = BITS_TO_BYTES(MinPacketDataBits);

} // namespace Zero
