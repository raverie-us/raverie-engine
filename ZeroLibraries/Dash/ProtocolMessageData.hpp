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
// Protocol Message Types
//

//---------------------------------------------------------------------------------//
//                               ConnectRequestData                                //
//---------------------------------------------------------------------------------//

/// Connect request protocol message data
struct ConnectRequestData
{
  /// Out: Their IP address (as seen from our perspective)
  /// In:  Our IP address (as seen from their perspective)
  IpAddress mIpAddress;
  /// Out: Our local time
  /// In:  Their remote time
  TimeMs    mTimestamp;
  /// Optional, user provided extra data
  BitStream mExtraData;
};

/// Serializes connect request data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectRequestData& connectRequestData);

//---------------------------------------------------------------------------------//
//                              ConnectResponseData                                //
//---------------------------------------------------------------------------------//

/// Connect response protocol message data
struct ConnectResponseData
{
  /// Out: Their IP address (as seen from our perspective)
  /// In:  Our IP address (as seen from their perspective)
  IpAddress             mIpAddress;
  /// Out: Our local time
  /// In:  Their remote time
  TimeMs                mTimestamp;
  /// Link connect response
  ConnectResponse::Enum mConnectResponse;
  /// Optional, user provided extra data
  BitStream             mExtraData;
};

/// Serializes connect response data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectResponseData& connectResponseData);

//---------------------------------------------------------------------------------//
//                           PacketSequenceHistoryData                             //
//---------------------------------------------------------------------------------//

/// Packet sequence history protocol message data
typedef PacketSequenceHistory PacketSequenceHistoryData;

/// Serializes packet sequence history protocol message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, PacketSequenceHistoryData& connectResponseData);

//---------------------------------------------------------------------------------//
//                               ChannelOpenedData                                 //
//---------------------------------------------------------------------------------//

/// Channel opened protocol message data
struct ChannelOpenedData
{
  /// Message channel transfer mode
  TransferMode::Enum mTransferMode;
};

/// Serializes channel opened data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ChannelOpenedData& channelOpenedData);

//---------------------------------------------------------------------------------//
//                              DisconnectNoticeData                               //
//---------------------------------------------------------------------------------//

/// Disconnect notice protocol message data
struct DisconnectNoticeData
{
  /// Link disconnect reason
  DisconnectReason::Enum mDisconnectReason;
  /// Optional, user provided extra data
  BitStream              mExtraData;
};

/// Serializes disconnect notice data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, DisconnectNoticeData& disconnectNoticeData);

//
// Peer Event Message Types
//

//---------------------------------------------------------------------------------//
//                           IncomingLinkCreatedData                               //
//---------------------------------------------------------------------------------//

/// Peer incoming link created event message data
struct IncomingLinkCreatedData
{
  /// Their IP address (as seen from our perspective)
  IpAddress mIpAddress;
};

/// Serializes peer incoming link created event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IncomingLinkCreatedData& incomingLinkCreatedData);

//---------------------------------------------------------------------------------//
//                               FatalErrorData                                    //
//---------------------------------------------------------------------------------//

/// Peer fatal error event message data
struct FatalErrorData
{
  /// Error description string
  String mErrorString;
};

/// Serializes peer fatal error event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, FatalErrorData& fatalErrorData);

//
// Link Event Message Types
//

//---------------------------------------------------------------------------------//
//                              ConnectRequestedData                               //
//---------------------------------------------------------------------------------//

/// Link connect requested event message data
struct ConnectRequestedData
{
  /// Direction the event was triggered from
  TransmissionDirection::Enum mDirection;
  /// Connect request data
  ConnectRequestData          mConnectRequestData;
};

/// Serializes link connect requested event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectRequestedData& connectRequestedData);

//---------------------------------------------------------------------------------//
//                              ConnectRespondedData                               //
//---------------------------------------------------------------------------------//

/// Link connect responded event message data
struct ConnectRespondedData
{
  /// Direction the event was triggered from
  TransmissionDirection::Enum mDirection;
  /// Connect response data
  ConnectResponseData         mConnectResponseData;
};

/// Serializes link connect responded event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectRespondedData& connectRespondedData);

//---------------------------------------------------------------------------------//
//                              DisconnectNoticedData                              //
//---------------------------------------------------------------------------------//

/// Link disconnect noticed event message data
struct DisconnectNoticedData
{
  /// Direction the event was triggered from
  TransmissionDirection::Enum mDirection;
  /// Disconnect notice data
  DisconnectNoticeData        mDisconnectNoticeData;
};

/// Serializes link disconnect noticed event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, DisconnectNoticedData& disconnectNoticedData);

//---------------------------------------------------------------------------------//
//                           IncomingChannelOpenedData                             //
//---------------------------------------------------------------------------------//

/// Link incoming channel opened event message data
struct IncomingChannelOpenedData
{
  /// Message channel ID
  MessageChannelId   mChannelId;
  /// Message channel transfer mode
  TransferMode::Enum mTransferMode;
};

/// Serializes link incoming channel opened event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IncomingChannelOpenedData& incomingChannelOpenedData);

//---------------------------------------------------------------------------------//
//                           IncomingChannelClosedData                             //
//---------------------------------------------------------------------------------//

/// Link incoming channel closed event message data
struct IncomingChannelClosedData
{
  /// Message channel ID
  MessageChannelId mChannelId;
};

/// Serializes link incoming channel closed event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IncomingChannelClosedData& incomingChannelClosedData);

//---------------------------------------------------------------------------------//
//                                StateChangeData                                  //
//---------------------------------------------------------------------------------//

/// Link state change event message data
struct StateChangeData
{
  /// New link state
  LinkState::Enum mNewState;
};

/// Serializes link state change event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, StateChangeData& stateChangeData);

//---------------------------------------------------------------------------------//
//                                StatusChangeData                                 //
//---------------------------------------------------------------------------------//

/// Link status change event message data
struct StatusChangeData
{
  /// New link status
  LinkStatus::Enum mNewStatus;
};

/// Serializes link status change event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, StatusChangeData& statusChangeData);

//---------------------------------------------------------------------------------//
//                                  ReceiptData                                    //
//---------------------------------------------------------------------------------//

/// Receipt event message data
struct ReceiptData
{
  /// Message receipt ID
  MessageReceiptId mReceiptId;
  /// Message receipt result
  Receipt::Enum    mReceipt;
};

/// Serializes receipt event message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ReceiptData& receiptData);

} // namespace Zero
