///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Typedefs
typedef IdSequence<PacketSequenceId>       PacketSequence;
typedef PacketSequence::IdSequenceHistory  PacketSequenceHistory;
typedef IdSequence<MessageSequenceId>      MessageSequence;
typedef MessageSequence::IdSequenceHistory MessageSequenceHistory;
typedef IdStore<MessageChannelId>          MessageChannelIdStore;

//---------------------------------------------------------------------------------//
//                                 MessageChannel                                  //
//---------------------------------------------------------------------------------//

/// Message sequence channel
class MessageChannel
{
public:
  /// Constructors
  MessageChannel();
  MessageChannel(MessageChannelId channelId, TransferMode::Enum transferMode);

  /// Comparison Operators (compares message channel IDs)
  bool operator ==(const MessageChannel& rhs) const;
  bool operator !=(const MessageChannel& rhs) const;
  bool operator  <(const MessageChannel& rhs) const;
  bool operator ==(MessageChannelId rhs) const;
  bool operator !=(MessageChannelId rhs) const;
  bool operator  <(MessageChannelId rhs) const;

  //
  // Member Functions
  //

  /// Returns the message channel's permanent ID
  MessageChannelId GetChannelId() const;

  /// Returns the message channel's permanent transfer mode
  TransferMode::Enum GetTransferMode() const;

private:
  /// Message channel ID
  MessageChannelId   mChannelId;
  /// Message channel transfer mode
  TransferMode::Enum mTransferMode;
};

//---------------------------------------------------------------------------------//
//                                OutMessageChannel                                //
//---------------------------------------------------------------------------------//

/// Outgoing message sequence channel
class OutMessageChannel : public MessageChannel
{
public:
  /// Constructors
  OutMessageChannel();
  OutMessageChannel(MessageChannelId channelId, TransferMode::Enum transferMode);

private:
  //
  // Internal
  //

  /// Increments and returns the next message channel sequence ID
  MessageSequenceId AcquireNextSequenceId();

  /// Next message channel sequence ID
  MessageSequenceId mNextSequenceId;

  /// Friends
  friend class LinkOutbox;
};

//---------------------------------------------------------------------------------//
//                                InMessageChannel                                 //
//---------------------------------------------------------------------------------//

/// Incoming message sequence channel
class InMessageChannel : public MessageChannel
{
public:
  /// Constructors
  InMessageChannel();
  InMessageChannel(MessageChannelId channelId, TransferMode::Enum transferMode);

  /// Copy Constructor
  InMessageChannel(const InMessageChannel& rhs);

  /// Move Constructor
  InMessageChannel(MoveReference<InMessageChannel> rhs);

  /// Copy Assignment Operator
  InMessageChannel& operator =(const InMessageChannel& rhs);

  /// Move Assignment Operator
  InMessageChannel& operator =(MoveReference<InMessageChannel> rhs);

private:
  //
  // Internal
  //

  /// Returns true if the specified message is a duplicate, else false
  bool IsDuplicate(const Message& message) const;

  /// Pushes a message for later release as appropriate
  /// Returns true if successful, else false
  bool Push(MoveReference<Message> message);
  /// Returns all appropriate messages ready for release
  Array<Message> Release();

  /// Opens the channel
  void Open();
  /// Closes the channel
  void Close(MessageSequenceId finalSequenceId);
  /// Returns true if the channel is already closed (may not be ready for deletion, but has been closed)
  bool IsClosed();

  /// Returns true if the channel is ready to be deleted, else false
  bool ReadyToDelete() const;

  /// Last message sequence ID
  MessageSequenceId           mLastSequenceId;
  /// Fragmented messages sorted by sequence ID
  ArraySet<FragmentedMessage> mFragmentedMessages;
  /// Whole messages sorted by sequence ID
  ArraySet<Message>           mMessages;
  /// Whole messages sequence record
  MessageSequence             mMessageSequence;
  /// Channel 'Is closed?' flag
  bool                        mClosed;
  /// Final message sequence ID
  MessageSequenceId           mFinalSequenceId;

  /// Friends
  friend class LinkInbox;
};

/// InMessageChannel Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<InMessageChannel>
{
  static inline void MoveWithoutDestruction(InMessageChannel* dest, InMessageChannel* source)
  {
    new(dest) InMessageChannel(ZeroMove(*source));
  }
};

} // namespace Zero
