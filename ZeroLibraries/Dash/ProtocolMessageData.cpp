///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//
// Protocol Message Types
//

//---------------------------------------------------------------------------------//
//                               ConnectRequestData                                //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectRequestData& connectRequestData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(connectRequestData.mIpAddress), 0);

    // Read timestamp
    ReturnIf(!bitStream.ReadQuantized(connectRequestData.mTimestamp, MessageTimestampMin, MessageTimestampMax), 0);

    // Read extra data (if any)
    connectRequestData.mExtraData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              ConnectResponseData                                //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectResponseData& connectResponseData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(connectResponseData.mIpAddress), 0);

    // Read timestamp
    ReturnIf(!bitStream.ReadQuantized(connectResponseData.mTimestamp, MessageTimestampMin, MessageTimestampMax), 0);

    // Read connect response
    ReturnIf(!bitStream.ReadQuantized(connectResponseData.mConnectResponse, ConnectResponseMin, ConnectResponseMax), 0);

    // Read extra data (if any)
    connectResponseData.mExtraData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                           PacketSequenceHistoryData                             //
//---------------------------------------------------------------------------------//

/// Serializes packet sequence history protocol message data
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, PacketSequenceHistoryData& packetSequenceHistoryData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(packetSequenceHistoryData.mNext), 0);

    // Read history
    packetSequenceHistoryData.mHistory.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                               ChannelOpenedData                                 //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ChannelOpenedData& channelOpenedData)
{
  return bitStream.SerializeQuantized(direction, channelOpenedData.mTransferMode, TransferModeMin, TransferModeMax);
}

//---------------------------------------------------------------------------------//
//                              DisconnectNoticeData                               //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, DisconnectNoticeData& disconnectNoticeData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.ReadQuantized(disconnectNoticeData.mDisconnectReason, DisconnectReasonMin, DisconnectReasonMax), 0);

    // Read extra data (if any)
    disconnectNoticeData.mExtraData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//
// Peer Event Message Types
//

//---------------------------------------------------------------------------------//
//                           IncomingLinkCreatedData                               //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IncomingLinkCreatedData& incomingLinkCreatedData)
{
  return bitStream.Serialize(direction, incomingLinkCreatedData.mIpAddress);
}

//---------------------------------------------------------------------------------//
//                               FatalErrorData                                    //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, FatalErrorData& fatalErrorData)
{
  return bitStream.Serialize(direction, fatalErrorData.mErrorString);
}

//
// Link Event Message Types
//

//---------------------------------------------------------------------------------//
//                              ConnectRequestedData                               //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectRequestedData& connectRequestedData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(connectRequestedData.mDirection), 0);

    // Read connect request data
    ReturnIf(!bitStream.Read(connectRequestedData.mConnectRequestData), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              ConnectRespondedData                               //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ConnectRespondedData& connectRespondedData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(connectRespondedData.mDirection), 0);

    // Read connect response data
    ReturnIf(!bitStream.Read(connectRespondedData.mConnectResponseData), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              DisconnectNoticedData                              //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, DisconnectNoticedData& disconnectNoticedData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(disconnectNoticedData.mDirection), 0);

    // Read disconnect notice data
    ReturnIf(!bitStream.Read(disconnectNoticedData.mDisconnectNoticeData), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                           IncomingChannelOpenedData                             //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IncomingChannelOpenedData& incomingChannelOpenedData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(incomingChannelOpenedData.mChannelId), 0);

    // Read message channel transfer mode
    ReturnIf(!bitStream.Read(incomingChannelOpenedData.mTransferMode), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                           IncomingChannelClosedData                             //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IncomingChannelClosedData& incomingChannelClosedData)
{
  return bitStream.Serialize(direction, incomingChannelClosedData.mChannelId);
}

//---------------------------------------------------------------------------------//
//                                StateChangeData                                  //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, StateChangeData& stateChangeData)
{
  return bitStream.Serialize(direction, stateChangeData.mNewState);
}

//---------------------------------------------------------------------------------//
//                                StatusChangeData                                 //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, StatusChangeData& statusChangeData)
{
  return bitStream.Serialize(direction, statusChangeData.mNewStatus);
}

//---------------------------------------------------------------------------------//
//                                  ReceiptData                                    //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, ReceiptData& receiptData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(receiptData.mReceiptId), 0);

    // Read receipt result
    ReturnIf(!bitStream.Read(receiptData.mReceipt), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

} // namespace Zero
