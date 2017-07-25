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
//                                 MessageChannel                                  //
//---------------------------------------------------------------------------------//

MessageChannel::MessageChannel()
  : mChannelId(0),
    mTransferMode(TransferMode::Immediate)
{
}
MessageChannel::MessageChannel(MessageChannelId channelId, TransferMode::Enum transferMode)
  : mChannelId(channelId),
    mTransferMode(transferMode)
{
}

bool MessageChannel::operator ==(const MessageChannel& rhs) const
{
  return GetChannelId() == rhs.GetChannelId();
}
bool MessageChannel::operator !=(const MessageChannel& rhs) const
{
  return GetChannelId() != rhs.GetChannelId();
}
bool MessageChannel::operator  <(const MessageChannel& rhs) const
{
  return GetChannelId() < rhs.GetChannelId();
}
bool MessageChannel::operator ==(MessageChannelId rhs) const
{
  return GetChannelId() == rhs;
}
bool MessageChannel::operator !=(MessageChannelId rhs) const
{
  return GetChannelId() != rhs;
}
bool MessageChannel::operator  <(MessageChannelId rhs) const
{
  return GetChannelId() < rhs;
}

//
// Member Functions
//

MessageChannelId MessageChannel::GetChannelId() const
{
  return mChannelId;
}

TransferMode::Enum MessageChannel::GetTransferMode() const
{
  return mTransferMode;
}

//---------------------------------------------------------------------------------//
//                                OutMessageChannel                                //
//---------------------------------------------------------------------------------//

OutMessageChannel::OutMessageChannel()
  : MessageChannel()
{
}
OutMessageChannel::OutMessageChannel(MessageChannelId channelId, TransferMode::Enum transferMode)
  : MessageChannel(channelId, transferMode)
{
}

//
// Internal
//

MessageSequenceId OutMessageChannel::AcquireNextSequenceId()
{
  return ++mNextSequenceId;
}

//---------------------------------------------------------------------------------//
//                                InMessageChannel                                 //
//---------------------------------------------------------------------------------//

InMessageChannel::InMessageChannel()
  : MessageChannel(),
    mLastSequenceId(0),
    mFragmentedMessages(),
    mMessages(),
    mMessageSequence(),
    mClosed(false),
    mFinalSequenceId(0)
{
}
InMessageChannel::InMessageChannel(MessageChannelId channelId, TransferMode::Enum transferMode)
  : MessageChannel(channelId, transferMode),
    mLastSequenceId(0),
    mFragmentedMessages(),
    mMessages(),
    mMessageSequence(),
    mClosed(false),
    mFinalSequenceId(0)
{
}

InMessageChannel::InMessageChannel(const InMessageChannel& rhs)
  : MessageChannel(rhs),
    mLastSequenceId(rhs.mLastSequenceId),
    mFragmentedMessages(rhs.mFragmentedMessages),
    mMessages(rhs.mMessages),
    mMessageSequence(rhs.mMessageSequence),
    mClosed(rhs.mClosed),
    mFinalSequenceId(rhs.mFinalSequenceId)
{
}

InMessageChannel::InMessageChannel(MoveReference<InMessageChannel> rhs)
  : MessageChannel(*rhs),
    mLastSequenceId(rhs->mLastSequenceId),
    mFragmentedMessages(ZeroMove(rhs->mFragmentedMessages)),
    mMessages(ZeroMove(rhs->mMessages)),
    mMessageSequence(ZeroMove(rhs->mMessageSequence)),
    mClosed(rhs->mClosed),
    mFinalSequenceId(rhs->mFinalSequenceId)
{
}

InMessageChannel& InMessageChannel::operator =(const InMessageChannel& rhs)
{
  MessageChannel::operator=(rhs);
  mLastSequenceId         = rhs.mLastSequenceId;
  mFragmentedMessages     = rhs.mFragmentedMessages;
  mMessages               = rhs.mMessages;
  mMessageSequence        = rhs.mMessageSequence;
  mClosed                 = rhs.mClosed;
  mFinalSequenceId        = rhs.mFinalSequenceId;

  return *this;
}

InMessageChannel& InMessageChannel::operator =(MoveReference<InMessageChannel> rhs)
{
  MessageChannel::operator=(*rhs);
  mLastSequenceId         = rhs->mLastSequenceId;
  mFragmentedMessages     = ZeroMove(rhs->mFragmentedMessages);
  mMessages               = ZeroMove(rhs->mMessages);
  mMessageSequence        = ZeroMove(rhs->mMessageSequence);
  mClosed                 = rhs->mClosed;
  mFinalSequenceId        = rhs->mFinalSequenceId;

  return *this;
}

//
// Internal
//

bool InMessageChannel::IsDuplicate(const Message& message) const
{
  // Whole message already acknowledged? (Whether this message is whole or a fragment doesn't matter)
  if(mMessageSequence.IsDuplicate(message.GetSequenceId()))
    return true;

  // Fragment message already acknowledged? (This specific fragment has already been acknowledged?)
  if(message.IsFragment())
  {
    ArraySet<FragmentedMessage>::iterator iter = mFragmentedMessages.FindPointer(message.GetSequenceId());
    if(iter != mFragmentedMessages.End())
      if(iter->IsDuplicate(message))
        return true;
  }

  return false;
}

bool InMessageChannel::Push(MoveReference<Message> message)
{
  Assert(!IsDuplicate(*message));

  // Fragment message already exists as a fragmented message?
  if(message->IsFragment())
  {
    ArraySet<FragmentedMessage>::iterator iter = mFragmentedMessages.FindPointer(message->GetSequenceId());
    if(iter != mFragmentedMessages.End())
    {
      // Add message fragment
      iter->Add(ZeroMove(message));

      // Fragmented message now complete?
      if(iter->IsComplete())
      {
        // Reconstruct whole message
        Message wholeMessage = iter->Reconstruct();
        Assert(!wholeMessage.IsFragment());

        // Erase from fragmented messages
        mFragmentedMessages.Erase(iter);

        // Push whole message
        bool result = Push(ZeroMove(wholeMessage));
        Assert(result);
      }

      // Success
      return true;
    }
  }

  // Channel closed? (And if channel is ordered, allow any outstanding messages to proceed as normal)
  if(mClosed && (GetTransferMode() != TransferMode::Ordered || message->GetSequenceId() > mFinalSequenceId))
    return false; // Discard message

  // New message?
  switch(GetTransferMode())
  {
  default:
    Assert(false);
    return false; // Discard message

  case TransferMode::Sequenced:
    // Out-of-order?
    if(message->GetSequenceId() < mLastSequenceId)
      return false; // Discard message

  case TransferMode::Immediate:
  case TransferMode::Ordered:
    //
    // Keep Message
    //

    // Fragmented?
    if(message->IsFragment())
    {
      // Keep message fragment
      FragmentedMessage fragmentedMessage(ZeroMove(message));
      ArraySet<FragmentedMessage>::pointer_bool_pair insertResult = mFragmentedMessages.Insert(ZeroMove(fragmentedMessage));
      Assert(insertResult.second); // (Insertion should have succeeded)
    }
    // Whole?
    else
    {
      // Update message sequence
      mMessageSequence.Add(message->GetSequenceId());

      // Update latest sequence ID
      mLastSequenceId = message->GetSequenceId();

      // Keep message
      mMessages.Insert(ZeroMove(message));
    }

    // Success
    return true;
  }
}
Array<Message> InMessageChannel::Release()
{
  Array<Message> result;
  switch(GetTransferMode())
  {
  default:
    Assert(false);
  case TransferMode::Immediate:
  case TransferMode::Sequenced:
    // Release all whole Messages
    result = ZeroMove(static_cast<Array<Message>&>(mMessages));
    mMessages.Clear();
    break;

  case TransferMode::Ordered:
    // Release all verified whole Messages
    for(ArraySet<Message>::iterator iter = mMessages.Begin(); iter != mMessages.End(); )
      if(mMessageSequence.IsVerified(iter->GetSequenceId()))
      {
        result.PushBack(ZeroMove(*iter));
        iter = mMessages.Erase(iter);
      }
      else
        ++iter;
    break;
  }
  return result;
}

void InMessageChannel::Open()
{
  Assert(!mClosed);

  mMessageSequence.Add(1);
  mLastSequenceId = 1;
}
void InMessageChannel::Close(MessageSequenceId finalSequenceId)
{
  Assert(!mClosed);

  mMessageSequence.Add(finalSequenceId);
  mFinalSequenceId = finalSequenceId;
  mClosed          = true;
}
bool InMessageChannel::IsClosed()
{
  return mClosed;
}

bool InMessageChannel::ReadyToDelete() const
{
  return mClosed
      && (GetTransferMode() == TransferMode::Ordered ? mMessageSequence.IsVerified(mFinalSequenceId) : true)
      && mMessages.Empty()
      && mFragmentedMessages.Empty();
}

} // namespace Zero
