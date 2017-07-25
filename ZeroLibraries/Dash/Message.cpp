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
//                                   Message                                       //
//---------------------------------------------------------------------------------//

Message::Message()
  : mType(0),
    mData(),
    mChannelId(0),
    mSequenceId(0),
    mTimestamp(cInvalidMessageTimestamp),
    mIsFragment(false),
    mFragmentIndex(0),
    mIsFinalFragment(false)
{
}

Message::Message(MessageType type, const BitStream& data)
  : mType(type),
    mData(data),
    mChannelId(0),
    mSequenceId(0),
    mTimestamp(cInvalidMessageTimestamp),
    mIsFragment(false),
    mFragmentIndex(0),
    mIsFinalFragment(false)
{
}
Message::Message(MessageType type, MoveReference<BitStream> data)
  : mType(type),
    mData(ZeroMove(data)),
    mChannelId(0),
    mSequenceId(0),
    mTimestamp(cInvalidMessageTimestamp),
    mIsFragment(false),
    mFragmentIndex(0),
    mIsFinalFragment(false)
{
}

Message::Message(MessageType type)
  : mType(type),
    mData(),
    mChannelId(0),
    mSequenceId(0),
    mTimestamp(cInvalidMessageTimestamp),
    mIsFragment(false),
    mFragmentIndex(0),
    mIsFinalFragment(false)
{
}

Message::Message(const BitStream& data)
  : mType(0),
    mData(data),
    mChannelId(0),
    mSequenceId(0),
    mTimestamp(cInvalidMessageTimestamp),
    mIsFragment(false),
    mFragmentIndex(0),
    mIsFinalFragment(false)
{
}
Message::Message(MoveReference<BitStream> data)
  : mType(0),
    mData(ZeroMove(data)),
    mChannelId(0),
    mSequenceId(0),
    mTimestamp(cInvalidMessageTimestamp),
    mIsFragment(false),
    mFragmentIndex(0),
    mIsFinalFragment(false)
{
}

Message::Message(const Message& rhs, bool doNotCopyData)
  : mType(rhs.mType),
    mData(),
    mChannelId(rhs.mChannelId),
    mSequenceId(rhs.mSequenceId),
    mTimestamp(rhs.mTimestamp),
    mIsFragment(rhs.mIsFragment),
    mFragmentIndex(rhs.mFragmentIndex),
    mIsFinalFragment(rhs.mIsFinalFragment)
{
  UnusedParameter(doNotCopyData);
}
Message::Message(const Message& rhs)
  : mType(rhs.mType),
    mData(rhs.mData),
    mChannelId(rhs.mChannelId),
    mSequenceId(rhs.mSequenceId),
    mTimestamp(rhs.mTimestamp),
    mIsFragment(rhs.mIsFragment),
    mFragmentIndex(rhs.mFragmentIndex),
    mIsFinalFragment(rhs.mIsFinalFragment)
{
}

Message::Message(MoveReference<Message> rhs)
  : mType(rhs->mType),
    mData(ZeroMove(rhs->mData)),
    mChannelId(rhs->mChannelId),
    mSequenceId(rhs->mSequenceId),
    mTimestamp(rhs->mTimestamp),
    mIsFragment(rhs->mIsFragment),
    mFragmentIndex(rhs->mFragmentIndex),
    mIsFinalFragment(rhs->mIsFinalFragment)
{
}

Message& Message::operator =(const Message& rhs)
{
  mType            = rhs.mType;
  mData            = rhs.mData;
  mChannelId       = rhs.mChannelId;
  mSequenceId      = rhs.mSequenceId;
  mTimestamp       = rhs.mTimestamp;
  mIsFragment      = rhs.mIsFragment;
  mFragmentIndex   = rhs.mFragmentIndex;
  mIsFinalFragment = rhs.mIsFinalFragment;

  return *this;
}
Message& Message::operator =(MoveReference<Message> rhs)
{
  mType            = rhs->mType;
  mData            = ZeroMove(rhs->mData);
  mChannelId       = rhs->mChannelId;
  mSequenceId      = rhs->mSequenceId;
  mTimestamp       = rhs->mTimestamp;
  mIsFragment      = rhs->mIsFragment;
  mFragmentIndex   = rhs->mFragmentIndex;
  mIsFinalFragment = rhs->mIsFinalFragment;

  return *this;
}

bool Message::operator ==(const Message& rhs) const
{
  return GetSequenceId() == rhs.GetSequenceId();
}
bool Message::operator !=(const Message& rhs) const
{
  return GetSequenceId() != rhs.GetSequenceId();
}
bool Message::operator  <(const Message& rhs) const
{
  return GetSequenceId() < rhs.GetSequenceId();
}
bool Message::operator ==(MessageSequenceId rhs) const
{
  return GetSequenceId() == rhs;
}
bool Message::operator !=(MessageSequenceId rhs) const
{
  return GetSequenceId() != rhs;
}
bool Message::operator  <(MessageSequenceId rhs) const
{
  return GetSequenceId() < rhs;
}

//
// Member Functions
//

void Message::SetType(MessageType type)
{
  mType = type;
}
MessageType Message::GetType() const
{
  return mType;
}

bool Message::HasData() const
{
  return !mData.IsEmpty();
}

void Message::SetData(const BitStream& data)
{
  mData = data;
}
void Message::SetData(MoveReference<BitStream> data)
{
  mData = ZeroMove(data);
}
const BitStream& Message::GetData() const
{
  return mData;
}
BitStream& Message::GetData()
{
  return mData;
}

bool Message::HasTimestamp() const
{
  return mTimestamp != cInvalidMessageTimestamp;
}

void Message::SetTimestamp(TimeMs timestamp)
{
  mTimestamp = timestamp;
}
TimeMs Message::GetTimestamp() const
{
  return mTimestamp;
}

bool Message::IsChanneled() const
{
  return mChannelId != 0;
}

MessageChannelId Message::GetChannelId() const
{
  return mChannelId;
}
MessageSequenceId Message::GetSequenceId() const
{
  return mSequenceId;
}

Bits Message::GetHeaderBits(bool asFragment) const
{
  // Minimum header size
  Bits result = MinMessageHeaderBits;

  // Timestamped?
  if(HasTimestamp())
    result += MessageT;

  // Channeled?
  if(IsChanneled())
    result += MessageC;

  // Data?
  if(HasData())
  {
    result += MessageD;

    // Fragmented?
    if(IsFragment() || asFragment)
      result += MessageF;
  }

  return result;
}

Bits Message::GetTotalBits() const
{
  return GetHeaderBits() + mData.GetBitsUnread();
}

//
// Internal
//

bool Message::IsCustomType() const
{
  return mType >= CustomMessageTypeStart;
}

bool Message::IsFragment() const
{
  return mIsFragment;
}
MessageFragmentIndex Message::GetFragmentIndex() const
{
  return mFragmentIndex;
}
bool Message::IsFinalFragment() const
{
  return mIsFinalFragment;
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, Message& message)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
#if ZeroDebug
    // Get predicted write size
    const Bits predictedBitsWritten = message.GetTotalBits();
#endif

    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    //
    // Write Message Header
    //

    // Write message type
    Bits bits1 = bitStream.WriteQuantized(message.mType, MessageTypeMin, MessageTypeMax);

    // Write message sequence ID
    Bits bits2 = bitStream.Write(message.mSequenceId);

    // Write message data size
    Bits bits3 = bitStream.WriteQuantized(message.mData.GetBitsWritten(), MinMessageDataBits, MaxMessageDataBits);

    // Write 'Has timestamp?' flag
    bool hasTimestamp = message.HasTimestamp();
    Bits bits4 = bitStream.Write(hasTimestamp);

    // Timestamped?
    Bits bits5 = 0;
    if(hasTimestamp)
    {
      // Write timestamp
      bits5 = bitStream.WriteQuantized(message.mTimestamp, MessageTimestampMin, MessageTimestampMax);
    }

    // Write 'Is channeled?' flag
    bool isChanneled = message.IsChanneled();
    Bits bits6 = bitStream.Write(isChanneled);

    // Channeled?
    Bits bits7 = 0;
    if(isChanneled)
    {
      // Write channel ID
      bits7 = bitStream.Write(message.mChannelId);
    }

    // Has Data?
    Bits bits8 = 0;
    Bits bits9 = 0;
    Bits bits10 = 0;
    Bits bitsAppended = 0;
    if(message.HasData())
    {
      // Write 'Is fragment?' flag
      bits8 = bitStream.Write(message.mIsFragment);

      // Fragmented?
      if(message.mIsFragment)
      {
        // Write FragmentIndex
        bits9 = bitStream.Write(message.mFragmentIndex);

        // Write 'Is final fragment?' flag
        bits10 = bitStream.Write(message.mIsFinalFragment);
      }

      //
      // Write Message Data
      //
      bitsAppended = bitStream.AppendAll(message.mData);
      Assert(bitsAppended == message.mData.GetBitsWritten());
    }

#if ZeroDebug
    // Verify predicted write size was correct
    const Bits bitsWrittenEnd = bitStream.GetBitsWritten();
    Assert((bitsWrittenEnd - bitsWrittenStart) == predictedBitsWritten);
#endif

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    //
    // Read Message Header
    //

    // Read message type
    ReturnIf(!bitStream.ReadQuantized(message.mType, MessageTypeMin, MessageTypeMax), 0);

    // Read message sequence ID
    ReturnIf(!bitStream.Read(message.mSequenceId), 0);

    // Read message data size
    Bits dataSize = 0;
    ReturnIf(!bitStream.ReadQuantized(dataSize, MinMessageDataBits, MaxMessageDataBits), 0);

    // Read 'Has timestamp?' flag
    bool hasTimestamp;
    ReturnIf(!bitStream.Read(hasTimestamp), 0);

    // Timestamped?
    if(hasTimestamp)
    {
      // Read timestamp
      ReturnIf(!bitStream.ReadQuantized(message.mTimestamp, MessageTimestampMin, MessageTimestampMax), 0);
    }

    // Read 'Is channeled?' flag
    bool isChanneled;
    ReturnIf(!bitStream.Read(isChanneled), 0);

    // Channeled?
    if(isChanneled)
    {
      // Read channel ID
      ReturnIf(!bitStream.Read(message.mChannelId), 0);
    }

    // Has Data?
    if(dataSize)
    {
      // Read 'Is fragment?' flag
      ReturnIf(!bitStream.Read(message.mIsFragment), 0);

      // Fragmented?
      if(message.mIsFragment)
      {
        // Read FragmentIndex
        ReturnIf(!bitStream.Read(message.mFragmentIndex), 0);

        // Read 'Is final fragment?' flag
        ReturnIf(!bitStream.Read(message.mIsFinalFragment), 0);
      }

      //
      // Read Message Data
      //
      Assert(message.mData.GetBitsWritten() == 0);
      ReturnIf(message.mData.Append(bitStream, dataSize) != dataSize, 0);
    }

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                                 OutMessage                                      //
//---------------------------------------------------------------------------------//

OutMessage::OutMessage()
  : Message(),
    mReliable(false),
    mTransferMode(TransferMode::Immediate),
    mReceiptID(0),
    mPriority(0),
    mLifetime(0),
    mCreationTime(0)
{
}
OutMessage::OutMessage(MoveReference<Message> message, bool reliable, MessageChannelId channelId, MessageSequenceId sequenceId, TransferMode::Enum transferMode,
                       MessageReceiptId receiptId, MessagePriority priority, TimeMs lifetime, TimeMs creationTime)
  : Message(ZeroMove(message)),
    mReliable(reliable),
    mTransferMode(transferMode),
    mReceiptID(receiptId),
    mPriority(priority),
    mLifetime(lifetime),
    mCreationTime(creationTime)
{
  mChannelId  = channelId;
  mSequenceId = sequenceId;
}

OutMessage::OutMessage(const OutMessage& rhs, MoveReference<Message> takeThisMessageInstead)
  : Message(ZeroMove(takeThisMessageInstead)),
    mReliable(rhs.mReliable),
    mTransferMode(rhs.mTransferMode),
    mReceiptID(rhs.mReceiptID),
    mPriority(rhs.mPriority),
    mLifetime(rhs.mLifetime),
    mCreationTime(rhs.mCreationTime)
{
}
OutMessage::OutMessage(const OutMessage& rhs)
  : Message(rhs),
    mReliable(rhs.mReliable),
    mTransferMode(rhs.mTransferMode),
    mReceiptID(rhs.mReceiptID),
    mPriority(rhs.mPriority),
    mLifetime(rhs.mLifetime),
    mCreationTime(rhs.mCreationTime)
{
}

OutMessage::OutMessage(MoveReference<OutMessage> rhs)
  : Message(ZeroMove(static_cast<Message&>(*rhs))),
    mReliable(rhs->mReliable),
    mTransferMode(rhs->mTransferMode),
    mReceiptID(rhs->mReceiptID),
    mPriority(rhs->mPriority),
    mLifetime(rhs->mLifetime),
    mCreationTime(rhs->mCreationTime)
{
}

OutMessage& OutMessage::operator =(MoveReference<OutMessage> rhs)
{
  Message::operator=(ZeroMove(static_cast<Message&>(*rhs)));
  mReliable        = rhs->mReliable;
  mTransferMode    = rhs->mTransferMode;
  mReceiptID       = rhs->mReceiptID;
  mPriority        = rhs->mPriority;
  mLifetime        = rhs->mLifetime;
  mCreationTime    = rhs->mCreationTime;

  return *this;
}

bool OutMessage::operator <(const OutMessage& rhs) const
{
  //
  // Sorted by message type category (protocol > custom), then priority.
  // Relative chronological ordering is expected to be maintained implicitly by the container.
  //

  // Message type category: protocol vs custom?
  if(!IsCustomType() && rhs.IsCustomType())
    return false;
  // Message type category: custom vs protocol?
  else if(IsCustomType() && !rhs.IsCustomType())
    return true;
  // Message type category: same? Compare priority
  else
    return mPriority < rhs.mPriority;
}

//
// Member Functions
//

bool OutMessage::IsReliable() const
{
  return mReliable;
}

TransferMode::Enum OutMessage::GetTransferMode() const
{
  return mTransferMode;
}

bool OutMessage::IsReceipted() const
{
  return mReceiptID != 0;
}

MessageReceiptId OutMessage::GetReceiptID() const
{
  return mReceiptID;
}

MessagePriority OutMessage::GetPriority() const
{
  return mPriority;
}

bool OutMessage::HasExpired(TimeMs currentTime) const
{
  return (currentTime - mCreationTime) > mLifetime;
}

TimeMs OutMessage::GetLifetime() const
{
  return mLifetime;
}

OutMessage OutMessage::TakeFragment(Bits dataSize)
{
  // Not already a fragment?
  if(!mIsFragment)
  {
    // Make it a fragment
    mReliable   = true;
    mIsFragment = true;
  }

  //
  // Extract desired message fragment
  //
  
  // Copy this message without it's data
  Message fragment(*this, true);

  // Write desired data size to fragment,
  // using Append which updates the Read cursor
  // for the next fragment to continue reading where it left off
  Bits bitsWritten = fragment.mData.Append(mData, dataSize);
  ErrorIf(bitsWritten != dataSize);

  // Update fragment index for the next fragment
  ++mFragmentIndex;

  // Return the new fragment
  return OutMessage(*this, ZeroMove(fragment));
}

//---------------------------------------------------------------------------------//
//                              FragmentedMessage                                  //
//---------------------------------------------------------------------------------//

FragmentedMessage::FragmentedMessage()
  : mFragments(),
    mFinalFragmentIndex(0)
{
}
FragmentedMessage::FragmentedMessage(MoveReference<Message> fragment)
  : mFragments(),
    mFinalFragmentIndex(0)
{
  // Add first fragment
  AddInternal(ZeroMove(fragment));
}

FragmentedMessage::FragmentedMessage(MoveReference<FragmentedMessage> rhs)
  : mFragments(ZeroMove(rhs->mFragments)),
    mFinalFragmentIndex(rhs->mFinalFragmentIndex)
{
}

bool FragmentedMessage::operator ==(const FragmentedMessage& rhs) const
{
  return mFragments.Front().GetSequenceId() == rhs.mFragments.Front().GetSequenceId();
}
bool FragmentedMessage::operator !=(const FragmentedMessage& rhs) const
{
  return mFragments.Front().GetSequenceId() != rhs.mFragments.Front().GetSequenceId();
}
bool FragmentedMessage::operator  <(const FragmentedMessage& rhs) const
{
  return mFragments.Front().GetSequenceId() < rhs.mFragments.Front().GetSequenceId();
}
bool FragmentedMessage::operator ==(MessageSequenceId rhs) const
{
  return mFragments.Front().GetSequenceId() == rhs;
}
bool FragmentedMessage::operator !=(MessageSequenceId rhs) const
{
  return mFragments.Front().GetSequenceId() != rhs;
}
bool FragmentedMessage::operator  <(MessageSequenceId rhs) const
{
  return mFragments.Front().GetSequenceId() < rhs;
}

//
// Member Functions
//

bool FragmentedMessage::IsDuplicate(const Message& fragment) const
{
  return mFragments.Contains(fragment.GetFragmentIndex());
}

void FragmentedMessage::Add(MoveReference<Message> fragment)
{
  //     Should be a fragment,
  // and Whole Message SequenceNumber should match,
  // and Should not be a duplicate fragment,
  // and If we have a final fragment, this fragment neither claims to be the final fragment and it's index is less than the final fragment index.
  Assert(fragment->IsFragment()
      && fragment->GetSequenceId() == mFragments.Front().GetSequenceId()
      && !IsDuplicate(*fragment)
      && (mFinalFragmentIndex != 0 ? (!fragment->IsFinalFragment() && fragment->GetFragmentIndex() < mFinalFragmentIndex) : true));

  // Add fragment
  AddInternal(ZeroMove(fragment));
}

bool FragmentedMessage::IsComplete() const
{
  return (mFinalFragmentIndex != 0
      && (mFragments.Size() == (mFinalFragmentIndex + 1).value()));
}

Message FragmentedMessage::Reconstruct()
{
  Assert(IsComplete());

  // Copy front fragment without it's data
  Message result(mFragments.Front(), true);

  // Append all fragment data in order
  for(FragmentSet::iterator iter = mFragments.Begin(); iter != mFragments.End(); ++iter)
  {
    Bits bitsWritten = result.mData.AppendAll(iter->mData);
    ErrorIf(bitsWritten != iter->mData.GetBitsWritten());
  }

  // Clear fragments, no longer needed
  mFragments.Clear();

  // Clear any fragment information on the reconstructed message
  result.mIsFragment      = false;
  result.mFragmentIndex   = 0;
  result.mIsFinalFragment = false;

  // Return reconstructed message
  return result;
}

void FragmentedMessage::AddInternal(MoveReference<Message> fragment)
{
  // Add fragment at it's sort (fragment index) position
  if(fragment->IsFinalFragment())
    mFinalFragmentIndex = fragment->GetFragmentIndex();
  mFragments.Insert(ZeroMove(fragment));
}

} // namespace Zero
