// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

//
// Protocol Message Types
//

//                               ConnectRequestData //

/// Connect request protocol message data
struct ConnectRequestData
{
  /// Out: Their IP address (as seen from our perspective)
  /// In:  Our IP address (as seen from their perspective)
  IpAddress mIpAddress;
  /// Out: Our local time
  /// In:  Their remote time
  TimeMs mTimestamp;
  /// Optional, user provided extra data
  BitStream mExtraData;
};

//                              ConnectResponseData //

/// Connect response protocol message data
struct ConnectResponseData
{
  /// Out: Their IP address (as seen from our perspective)
  /// In:  Our IP address (as seen from their perspective)
  IpAddress mIpAddress;
  /// Out: Our local time
  /// In:  Their remote time
  TimeMs mTimestamp;
  /// Link connect response
  ConnectResponse::Enum mConnectResponse;
  /// Optional, user provided extra data
  BitStream mExtraData;
};

//                           PacketSequenceHistoryData //

/// Packet sequence history protocol message data
typedef PacketSequenceHistory PacketSequenceHistoryData;

//                               ChannelOpenedData //

/// Channel opened protocol message data
struct ChannelOpenedData
{
  /// Message channel transfer mode
  TransferMode::Enum mTransferMode;
};

//                              DisconnectNoticeData //

/// Disconnect notice protocol message data
struct DisconnectNoticeData
{
  /// Link disconnect reason
  DisconnectReason::Enum mDisconnectReason;
  /// Optional, user provided extra data
  BitStream mExtraData;
};

//
// Peer Event Message Types
//

//                           IncomingLinkCreatedData //

/// Peer incoming link created event message data
struct IncomingLinkCreatedData
{
  /// Their IP address (as seen from our perspective)
  IpAddress mIpAddress;
};

//                               FatalErrorData //

/// Peer fatal error event message data
struct FatalErrorData
{
  /// Error description string
  String mErrorString;
};

//
// Link Event Message Types
//

//                              ConnectRequestedData //

/// Link connect requested event message data
struct ConnectRequestedData
{
  /// Direction the event was triggered from
  TransmissionDirection::Enum mDirection;
  /// Connect request data
  ConnectRequestData mConnectRequestData;
};

//                              ConnectRespondedData //

/// Link connect responded event message data
struct ConnectRespondedData
{
  /// Direction the event was triggered from
  TransmissionDirection::Enum mDirection;
  /// Connect response data
  ConnectResponseData mConnectResponseData;
};

//                              DisconnectNoticedData //

/// Link disconnect noticed event message data
struct DisconnectNoticedData
{
  /// Direction the event was triggered from
  TransmissionDirection::Enum mDirection;
  /// Disconnect notice data
  DisconnectNoticeData mDisconnectNoticeData;
};

//                           IncomingChannelOpenedData //

/// Link incoming channel opened event message data
struct IncomingChannelOpenedData
{
  /// Message channel ID
  MessageChannelId mChannelId;
  /// Message channel transfer mode
  TransferMode::Enum mTransferMode;
};

//                           IncomingChannelClosedData //

/// Link incoming channel closed event message data
struct IncomingChannelClosedData
{
  /// Message channel ID
  MessageChannelId mChannelId;
};

//                                StateChangeData //

/// Link state change event message data
struct StateChangeData
{
  /// New link state
  LinkState::Enum mNewState;
};

//                                StatusChangeData //

/// Link status change event message data
struct StatusChangeData
{
  /// New link status
  LinkStatus::Enum mNewStatus;
};

//                                  ReceiptData //

/// Receipt event message data
struct ReceiptData
{
  /// Message receipt ID
  MessageReceiptId mReceiptId;
  /// Message receipt result
  Receipt::Enum mReceipt;
};

//
// Protocol Message Types
//

//                               ConnectRequestData //
template <>
inline Bits Serialize<ConnectRequestData>(SerializeDirection::Enum direction,
                                          BitStream& bitStream,
                                          ConnectRequestData& connectRequestData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write IP address
    bitStream.Write(connectRequestData.mIpAddress);

    // Write timestamp
    bitStream.WriteQuantized(connectRequestData.mTimestamp, MessageTimestampMin, MessageTimestampMax);

    // Write extra data (if any)
    bitStream.AppendAll(connectRequestData.mExtraData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read IP address
    ReturnIf(!bitStream.Read(connectRequestData.mIpAddress), 0, "");

    // Read timestamp
    ReturnIf(!bitStream.ReadQuantized(connectRequestData.mTimestamp, MessageTimestampMin, MessageTimestampMax), 0, "");

    // Read extra data (if any)
    connectRequestData.mExtraData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              ConnectResponseData //
template <>
inline Bits Serialize<ConnectResponseData>(SerializeDirection::Enum direction,
                                           BitStream& bitStream,
                                           ConnectResponseData& connectResponseData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write IP address
    bitStream.Write(connectResponseData.mIpAddress);

    // Write timestamp
    bitStream.WriteQuantized(connectResponseData.mTimestamp, MessageTimestampMin, MessageTimestampMax);

    // Write connect response
    bitStream.WriteQuantized(connectResponseData.mConnectResponse, ConnectResponseMin, ConnectResponseMax);

    // Write extra data (if any)
    bitStream.AppendAll(connectResponseData.mExtraData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read IP address
    ReturnIf(!bitStream.Read(connectResponseData.mIpAddress), 0, "");

    // Read timestamp
    ReturnIf(!bitStream.ReadQuantized(connectResponseData.mTimestamp, MessageTimestampMin, MessageTimestampMax), 0, "");

    // Read connect response
    ReturnIf(
        !bitStream.ReadQuantized(connectResponseData.mConnectResponse, ConnectResponseMin, ConnectResponseMax), 0, "");

    // Read extra data (if any)
    connectResponseData.mExtraData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                           PacketSequenceHistoryData //

/// Serializes packet sequence history protocol message data
/// Returns the number of bits serialized if successful, else 0
template <>
inline Bits Serialize<PacketSequenceHistoryData>(SerializeDirection::Enum direction,
                                                 BitStream& bitStream,
                                                 PacketSequenceHistoryData& packetSequenceHistoryData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write next
    bitStream.Write(packetSequenceHistoryData.mNext);

    // Write history
    bitStream.AppendAll(packetSequenceHistoryData.mHistory);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read next
    ReturnIf(!bitStream.Read(packetSequenceHistoryData.mNext), 0, "");

    // Read history
    packetSequenceHistoryData.mHistory.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                               ChannelOpenedData //
template <>
inline Bits Serialize<ChannelOpenedData>(SerializeDirection::Enum direction,
                                         BitStream& bitStream,
                                         ChannelOpenedData& channelOpenedData)
{
  return bitStream.SerializeQuantized(direction, channelOpenedData.mTransferMode, TransferModeMin, TransferModeMax);
};

//                              DisconnectNoticeData //
template <>
inline Bits Serialize<DisconnectNoticeData>(SerializeDirection::Enum direction,
                                            BitStream& bitStream,
                                            DisconnectNoticeData& disconnectNoticeData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write disconnect reason
    bitStream.WriteQuantized(disconnectNoticeData.mDisconnectReason, DisconnectReasonMin, DisconnectReasonMax);

    // Write extra data (if any)
    bitStream.AppendAll(disconnectNoticeData.mExtraData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read disconnect reason
    ReturnIf(!bitStream.ReadQuantized(disconnectNoticeData.mDisconnectReason, DisconnectReasonMin, DisconnectReasonMax),
             0,
             "");

    // Read extra data (if any)
    disconnectNoticeData.mExtraData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//
// Peer Event Message Types
//

//                           IncomingLinkCreatedData //
template <>
inline Bits Serialize<IncomingLinkCreatedData>(SerializeDirection::Enum direction,
                                               BitStream& bitStream,
                                               IncomingLinkCreatedData& incomingLinkCreatedData)
{
  return bitStream.Serialize(direction, incomingLinkCreatedData.mIpAddress);
};

//                               FatalErrorData //
template <>
inline Bits Serialize<FatalErrorData>(SerializeDirection::Enum direction,
                                      BitStream& bitStream,
                                      FatalErrorData& fatalErrorData)
{
  return bitStream.Serialize(direction, fatalErrorData.mErrorString);
};

//
// Link Event Message Types
//

//                              ConnectRequestedData //
template <>
inline Bits Serialize<ConnectRequestedData>(SerializeDirection::Enum direction,
                                            BitStream& bitStream,
                                            ConnectRequestedData& connectRequestedData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write direction
    bitStream.Write(connectRequestedData.mDirection);

    // Write connect request data
    bitStream.Write(connectRequestedData.mConnectRequestData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read direction
    ReturnIf(!bitStream.Read(connectRequestedData.mDirection), 0, "");

    // Read connect request data
    ReturnIf(!bitStream.Read(connectRequestedData.mConnectRequestData), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              ConnectRespondedData //
template <>
inline Bits Serialize<ConnectRespondedData>(SerializeDirection::Enum direction,
                                            BitStream& bitStream,
                                            ConnectRespondedData& connectRespondedData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write direction
    bitStream.Write(connectRespondedData.mDirection);

    // Write connect response data
    bitStream.Write(connectRespondedData.mConnectResponseData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read direction
    ReturnIf(!bitStream.Read(connectRespondedData.mDirection), 0, "");

    // Read connect response data
    ReturnIf(!bitStream.Read(connectRespondedData.mConnectResponseData), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              DisconnectNoticedData //
template <>
inline Bits Serialize<DisconnectNoticedData>(SerializeDirection::Enum direction,
                                             BitStream& bitStream,
                                             DisconnectNoticedData& disconnectNoticedData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write direction
    bitStream.Write(disconnectNoticedData.mDirection);

    // Write disconnect notice data
    bitStream.Write(disconnectNoticedData.mDisconnectNoticeData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read direction
    ReturnIf(!bitStream.Read(disconnectNoticedData.mDirection), 0, "");

    // Read disconnect notice data
    ReturnIf(!bitStream.Read(disconnectNoticedData.mDisconnectNoticeData), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                           IncomingChannelOpenedData //
template <>
inline Bits Serialize<IncomingChannelOpenedData>(SerializeDirection::Enum direction,
                                                 BitStream& bitStream,
                                                 IncomingChannelOpenedData& incomingChannelOpenedData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write message channel ID
    bitStream.Write(incomingChannelOpenedData.mChannelId);

    // Write message channel transfer mode
    bitStream.Write(incomingChannelOpenedData.mTransferMode);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read message channel ID
    ReturnIf(!bitStream.Read(incomingChannelOpenedData.mChannelId), 0, "");

    // Read message channel transfer mode
    ReturnIf(!bitStream.Read(incomingChannelOpenedData.mTransferMode), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                           IncomingChannelClosedData //
template <>
inline Bits Serialize<IncomingChannelClosedData>(SerializeDirection::Enum direction,
                                                 BitStream& bitStream,
                                                 IncomingChannelClosedData& incomingChannelClosedData)
{
  return bitStream.Serialize(direction, incomingChannelClosedData.mChannelId);
};

//                                StateChangeData //
template <>
inline Bits Serialize<StateChangeData>(SerializeDirection::Enum direction,
                                       BitStream& bitStream,
                                       StateChangeData& stateChangeData)
{
  return bitStream.Serialize(direction, stateChangeData.mNewState);
};

//                                StatusChangeData //
template <>
inline Bits Serialize<StatusChangeData>(SerializeDirection::Enum direction,
                                        BitStream& bitStream,
                                        StatusChangeData& statusChangeData)
{
  return bitStream.Serialize(direction, statusChangeData.mNewStatus);
};

//                                  ReceiptData //
template <>
inline Bits Serialize<ReceiptData>(SerializeDirection::Enum direction, BitStream& bitStream, ReceiptData& receiptData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write receipt ID
    bitStream.Write(receiptData.mReceiptId);

    // Write receipt result
    bitStream.Write(receiptData.mReceipt);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read receipt ID
    ReturnIf(!bitStream.Read(receiptData.mReceiptId), 0, "");

    // Read receipt result
    ReturnIf(!bitStream.Read(receiptData.mReceipt), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

} // namespace Zero
