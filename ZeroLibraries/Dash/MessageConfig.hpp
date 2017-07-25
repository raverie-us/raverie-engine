///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Because windows...
#undef min
#undef max

//---------------------------------------------------------------------------------//
//                           Message Configuration                                 //
//---------------------------------------------------------------------------------//

/// Minimum message fragment data size
/// Determines the threshold at which fragmentation may be preferred and fragment index count
#define MESSAGE_MIN_FRAGMENT_DATA_BYTES POW2(5)
StaticAssertWithinRange(Range1, MESSAGE_MIN_FRAGMENT_DATA_BYTES, 1, BITSTREAM_MAX_BYTES);

/// Maximum message whole data size (original data size before any fragmentation occurs)
/// Determines the maximum data size and fragment index count
#define MESSAGE_MAX_WHOLE_DATA_BYTES POW2(24)
StaticAssertWithinRange(Range2, MESSAGE_MAX_WHOLE_DATA_BYTES, MESSAGE_MIN_FRAGMENT_DATA_BYTES, BITSTREAM_MAX_BYTES);

/// Message type bits
/// Determines the maximum number of message types
#define MESSAGE_TYPE_BITS 7
StaticAssertWithinRange(Range3, MESSAGE_TYPE_BITS, 1, UINTMAX_BITS);

/// Message timestamp bits
/// Determines the signed timestamp range (in milliseconds) that can be represented on a message
#define MESSAGE_TIMESTAMP_BITS 40
StaticAssertWithinRange(Range16, MESSAGE_TIMESTAMP_BITS, 1, UINTMAX_BITS);

/// Message channel ID bits
/// Determines the maximum number of outgoing and incoming message channels (separately, not combined)
#define MESSAGE_CHANNEL_ID_BITS 16
StaticAssertWithinRange(Range4, MESSAGE_CHANNEL_ID_BITS, 1, UINTMAX_BITS);

/// Message channel sequence message ID bits
/// Determines the maximum number of sequence message IDs before wrap-around occurs
#define MESSAGE_SEQUENCE_ID_BITS 16
StaticAssertWithinRange(Range5, MESSAGE_SEQUENCE_ID_BITS, 1, UINTMAX_BITS);

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                Message Type                                     //
//---------------------------------------------------------------------------------//

/// Message type ID bit size
static const Bits MessageTypeBits = MESSAGE_TYPE_BITS;

/// Message type ID
typedef make_type<FASTEST_UINT(MessageTypeBits)>::type MessageType;

/// Message type ID range
static const MessageType MessageTypeMin = MessageType(0);
static const MessageType MessageTypeMax = MessageType(POW2(MessageTypeBits) - 1);

/// Protocol absolute message type ID range
static const MessageType ProtocolMessageTypeStart = MessageType(ProtocolMessageType::Invalid);
static const MessageType ProtocolMessageTypeEnd   = MessageType(ProtocolMessageType::Reserved3);

/// Custom (user or plugin) absolute message type ID range
static const MessageType CustomMessageTypeStart = MessageType(ProtocolMessageTypeEnd + 1);
static const MessageType CustomMessageTypeEnd   = MessageTypeMax;

/// Peer event message type ID range (used locally only)
namespace PeerEventMessageType
{
  /// Note: This enum must count down, not up
  enum Enum
  {
    IncomingLinkCreated = MessageType(MessageTypeMax - 0), // An incoming link was created
    FatalError          = MessageType(MessageTypeMax - 1)  // A fatal error occurred, invalidating the peer

    /// Note: This enum must not exceed the number of protocol messages in use
  };
  typedef uint Type;
}

/// Link event message type ID range (used locally only)
namespace LinkEventMessageType
{
  /// Note: This enum must count down, not up
  enum Enum
  {
    ConnectRequested      = MessageType(MessageTypeMax - 0), // A connect request was sent or received
    ConnectResponded      = MessageType(MessageTypeMax - 1), // A connect response was sent or received
    DisconnectNoticed     = MessageType(MessageTypeMax - 2), // A disconnect notice was sent or received
    IncomingChannelOpened = MessageType(MessageTypeMax - 3), // The remote peer opened an incoming channel
    IncomingChannelClosed = MessageType(MessageTypeMax - 4), // The remote peer closed an incoming channel
    StateChange           = MessageType(MessageTypeMax - 5), // The link's state has changed
    StatusChange          = MessageType(MessageTypeMax - 6), // The link's status has changed
    Receipt               = MessageType(MessageTypeMax - 7)  // A previously sent message was receipted

    /// Note: This enum must not exceed the number of protocol messages in use
  };
  typedef uint Type;
}

//---------------------------------------------------------------------------------//
//                                Message Data                                     //
//---------------------------------------------------------------------------------//

/// Message data stream range
static const Bits MinMessageDataBits = Bits(0);
static const Bits MaxMessageDataBits = BYTES_TO_BITS(EthernetMtuBytes);

/// Message data stream size bit size
static const Bits MessageDataSizeBits = BITS_NEEDED_TO_REPRESENT(MaxMessageDataBits);

//---------------------------------------------------------------------------------//
//                             Message Channel ID                                  //
//---------------------------------------------------------------------------------//

/// Message channel ID bits
static const Bits MessageChannelIdBits = MESSAGE_CHANNEL_ID_BITS;

/// Message channel ID
typedef UintN<MessageChannelIdBits> MessageChannelId;

//---------------------------------------------------------------------------------//
//                             Message Sequence ID                                 //
//---------------------------------------------------------------------------------//

/// Message sequence ID bits
static const Bits MessageSequenceIdBits = MESSAGE_SEQUENCE_ID_BITS;

/// Message sequence ID
typedef UintN<MessageSequenceIdBits, true> MessageSequenceId;

//---------------------------------------------------------------------------------//
//                              Message Timestamp                                  //
//---------------------------------------------------------------------------------//

/// Message timestamp bits
static const Bits MessageTimestampBits = MESSAGE_TIMESTAMP_BITS;

/// Message timestamp range
static const TimeMs MessageTimestampMin   = -TimeMs(POW2(MessageTimestampBits - 1));
static const TimeMs MessageTimestampMax   = +TimeMs(POW2(MessageTimestampBits - 1)) - 1;
static const TimeMs MessageTimestampRange = (MessageTimestampMax - MessageTimestampMin);

/// Invalid message timestamp
static const TimeMs cInvalidMessageTimestamp = std::numeric_limits<TimeMs>::min();

// (Sanity check)
static const Bits MessageTimestampBitsNeeded = BITS_NEEDED_TO_REPRESENT(MessageTimestampRange);
StaticAssert(MessageTimestampSize, MessageTimestampBits == MessageTimestampBitsNeeded,
            "Declared message timestamp bits do not match the actual bits needed to represent the declared message timestamp range");

//---------------------------------------------------------------------------------//
//                            Message Fragment Index                               //
//---------------------------------------------------------------------------------//

/// Maximum message whole data size
static const Bytes MaxMessageWholeDataBytes = MESSAGE_MAX_WHOLE_DATA_BYTES;
static const Bits  MaxMessageWholeDataBits  = BYTES_TO_BITS(MaxMessageWholeDataBytes);

/// Minimum message fragment data size
static const Bytes MinMessageFragmentDataBytes = MESSAGE_MIN_FRAGMENT_DATA_BYTES;
static const Bits  MinMessageFragmentDataBits  = BYTES_TO_BITS(MinMessageFragmentDataBytes);

/// Resulting maximum message fragment index count
static const uint  MaxMessageFragmentCount  = MaxMessageWholeDataBits / MinMessageFragmentDataBits;
static const uint  MessageFragmentIndexMax  = MaxMessageFragmentCount - 1;
static const Bits  MessageFragmentIndexBits = BITS_NEEDED_TO_REPRESENT(MessageFragmentIndexMax);

/// Message fragment index
typedef UintN<MessageFragmentIndexBits> MessageFragmentIndex;

//---------------------------------------------------------------------------------//
//                             Message Receipt ID                                  //
//---------------------------------------------------------------------------------//

/// Message receipt ID
typedef uint MessageReceiptId;

//---------------------------------------------------------------------------------//
//                              Message Priority                                   //
//---------------------------------------------------------------------------------//

/// Message priority (larger values indicate greater priority)
typedef uint8 MessagePriority;

//---------------------------------------------------------------------------------//
//                            Message Header Size                                  //
//---------------------------------------------------------------------------------//

/// Minimum message header size
static const Bits MinMessageHeaderBits = MessageTypeBits       /// Message type
                                       + MessageSequenceIdBits /// Message sequence ID
                                       + MessageDataSizeBits   /// Message data size
                                       + 1                     /// Message 'Is channeled?' flag
                                       + 1;                    /// Message 'Has accurate timestamp?' flag

/// Message header options
static const Bits MessageT = MessageTimestampBits;    /// Message timestamp
static const Bits MessageC = MessageChannelIdBits;    /// Message channel ID
static const Bits MessageD = 1;                       /// Message with data ('Is fragment?' flag)
static const Bits MessageF = MessageFragmentIndexBits /// Message fragment index
                           + 1;                       /// Message 'Is final fragment?' flag

/// Possible message header configurations
static const Bits MessageHeaderBitsC    = MinMessageHeaderBits + MessageC;
static const Bits MessageHeaderBitsD    = MinMessageHeaderBits + MessageD;
static const Bits MessageHeaderBitsDF   = MinMessageHeaderBits + MessageD + MessageF;
static const Bits MessageHeaderBitsCD   = MinMessageHeaderBits + MessageC + MessageD;
static const Bits MessageHeaderBitsCDF  = MinMessageHeaderBits + MessageC + MessageD + MessageF;
static const Bits MessageHeaderBitsTC   = MinMessageHeaderBits + MessageT + MessageC;
static const Bits MessageHeaderBitsTD   = MinMessageHeaderBits + MessageT + MessageD;
static const Bits MessageHeaderBitsTDF  = MinMessageHeaderBits + MessageT + MessageD + MessageF;
static const Bits MessageHeaderBitsTCD  = MinMessageHeaderBits + MessageT + MessageC + MessageD;
static const Bits MessageHeaderBitsTCDF = MinMessageHeaderBits + MessageT + MessageC + MessageD + MessageF;

/// Maximum message header size
static const Bits MaxMessageHeaderBits = MessageHeaderBitsTCDF;

} // namespace Zero
