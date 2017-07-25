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
//                                   Message                                       //
//---------------------------------------------------------------------------------//

/// Application data unit
class Message
{
public:
  /// Default Constructor
  Message();

  /// Constructs a message with the specified type ID and data stream
  Message(MessageType type, const BitStream& data);
  Message(MessageType type, MoveReference<BitStream> data);

  /// Constructs a message with the specified type ID
  explicit Message(MessageType type);

  /// Constructs a message with the specified data stream
  explicit Message(const BitStream& data);
  explicit Message(MoveReference<BitStream> data);

  /// Copy Constructors
  Message(const Message& rhs, bool doNotCopyData);
  Message(const Message& rhs);

  /// Move Constructor
  Message(MoveReference<Message> rhs);

  /// Copy Assignment Operator
  Message& operator =(const Message& rhs);
  /// Move Assignment Operator
  Message& operator =(MoveReference<Message> rhs);

  /// Comparison Operators (compares message channel sequence IDs)
  bool operator ==(const Message& rhs) const;
  bool operator !=(const Message& rhs) const;
  bool operator  <(const Message& rhs) const;
  bool operator ==(MessageSequenceId rhs) const;
  bool operator !=(MessageSequenceId rhs) const;
  bool operator  <(MessageSequenceId rhs) const;

  //
  // Member Functions
  //

  /// Message type ID
  void SetType(MessageType type);
  MessageType GetType() const;

  /// Returns true if the message data stream is non-empty, else false
  bool HasData() const;

  /// Message data stream
  void SetData(const BitStream& data);
  void SetData(MoveReference<BitStream> data);
  const BitStream& GetData() const;
  BitStream& GetData();

  /// Returns true if the message has a valid timestamp, else false
  bool HasTimestamp() const;

  /// Message timestamp (in local time), else cInvalidMessageTimestamp
  void SetTimestamp(TimeMs timestamp);
  TimeMs GetTimestamp() const;

  /// Returns true if the message is a part of a message channel (has a non-zero channel ID), else false
  bool IsChanneled() const;

  /// Message channel ID this message was sent with
  MessageChannelId GetChannelId() const;
  /// Message channel sequence ID this message was sent with
  MessageSequenceId GetSequenceId() const;

  /// Returns the message's current header bits as it currently would result in when serialized
  Bits GetHeaderBits(bool asFragment = false) const;

  /// Returns the total unread message bits (current header size + unread message data)
  Bits GetTotalBits() const;

protected:
  //
  // Internal
  //

  /// Returns true if the message type belongs to the user or a plugin (not the protocol), else false
  bool IsCustomType() const;

  /// Returns true if the message is a fragment, else false
  bool IsFragment() const;
  /// Returns the fragment index
  MessageFragmentIndex GetFragmentIndex() const;
  /// Returns true if the message is the final fragment, else false
  bool IsFinalFragment() const;

  /// Message type ID
  MessageType          mType;
  /// Message data stream
  BitStream            mData;
  /// Message channel ID
  MessageChannelId     mChannelId;
  /// Message channel sequence ID
  MessageSequenceId    mSequenceId;
  /// Message timestamp
  TimeMs               mTimestamp;
  /// Message 'Is fragment?' flag
  bool                 mIsFragment;
  /// Message fragment index
  MessageFragmentIndex mFragmentIndex;
  /// Message 'Is final fragment?' flag
  bool                 mIsFinalFragment;

  /// Friends
  friend class PeerLink;
  friend class LinkInbox;
  friend class LinkOutbox;
  friend class InPacket;
  friend class OutPacket;
  friend class OutMessage;
  friend class FragmentedMessage;
  friend class InMessageChannel;
  friend class LinkPlugin;
  friend Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, Message& message);
};

/// Typedefs
typedef UniquePointer<Message> MessagePtr;

/// Message Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<Message>
{
  static inline void MoveWithoutDestruction(Message* dest, Message* source)
  {
    new(dest) Message(ZeroMove(*source));
  }
};

/// Serializes a message
/// Returns the number of bits serialized if successful, else 0
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, Message& message);

//---------------------------------------------------------------------------------//
//                                 OutMessage                                      //
//---------------------------------------------------------------------------------//

/// Outgoing application data unit (message w/ additional send information)
class OutMessage : public Message
{
public:
  /// Constructors
  OutMessage();
  OutMessage(MoveReference<Message> message, bool reliable = false, MessageChannelId channelId = 0, MessageSequenceId sequenceId = 0, TransferMode::Enum transferMode = TransferMode::Immediate,
             MessageReceiptId receiptId = 0, MessagePriority priority = 0, TimeMs lifetime = 0, TimeMs creationTime = 0);

  /// Copy Constructors
  OutMessage(const OutMessage& rhs, MoveReference<Message> takeThisMessageInstead);
  OutMessage(const OutMessage& rhs);

  /// Move Constructor
  OutMessage(MoveReference<OutMessage> rhs);

  /// Move Assignment Operator
  OutMessage& operator =(MoveReference<OutMessage> rhs);

  /// Comparison Operator (compares message type category, then priority)
  bool operator <(const OutMessage& rhs) const;

  //
  // Member Functions
  //

  /// Returns true if the message is reliable, else false
  bool IsReliable() const;

  /// Returns the message channel's transfer mode
  TransferMode::Enum GetTransferMode() const;

  /// Returns true if the message is receipted (a receipt was requested), else false
  bool IsReceipted() const;

  /// Returns the message receipt ID (0 indicates no receipt was requested)
  MessageReceiptId GetReceiptID() const;

  /// Returns the message priority
  MessagePriority GetPriority() const;

  /// Returns true if the message has expired (lifetime duration has elapsed), else false
  bool HasExpired(TimeMs currentTime) const;

  /// Returns the message lifetime duration
  TimeMs GetLifetime() const;

  /// Takes a message fragment of the specified dataSize + header size
  OutMessage TakeFragment(Bits dataSize);

private:
  /// Message 'Is reliable?' flag
  bool               mReliable;
  /// Message channel transfer mode
  TransferMode::Enum mTransferMode;
  /// Message receipt ID
  MessageReceiptId   mReceiptID;
  /// Message priority
  MessagePriority    mPriority;
  /// Message lifetime duration
  TimeMs             mLifetime;
  /// Message creation time
  TimeMs             mCreationTime;
};

/// Typedefs
typedef UniquePointer<OutMessage> OutMessagePtr;

/// OutMessage Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<OutMessage>
{
  static inline void MoveWithoutDestruction(OutMessage* dest, OutMessage* source)
  {
    new(dest) OutMessage(ZeroMove(*source));
  }
};

//---------------------------------------------------------------------------------//
//                              FragmentedMessage                                  //
//---------------------------------------------------------------------------------//

/// Fragmented message collection
class FragmentedMessage
{
  /// Policy for how fragments are sorted
  struct FragmentSortPolicy : public ComparePolicy<Message>
  {
    /// Typedefs
    typedef Message value_type;

    /// Sort using fragment index
    template<typename CompareType>
    bool operator()(const value_type& lhs, const CompareType& rhs) const
    {
      return lhs.mFragmentIndex < rhs;
    }
    bool operator()(const value_type& lhs, const value_type& rhs) const
    {
      return lhs.mFragmentIndex < rhs.mFragmentIndex;
    }

    template<typename CompareType>
    bool Equal(const value_type& lhs, const CompareType& rhs) const
    {
      return lhs.mFragmentIndex == rhs;
    }
    bool Equal(const value_type& lhs, const value_type& rhs) const
    {
      return lhs.mFragmentIndex == rhs.mFragmentIndex;
    }
  };

  /// Typedefs
  typedef ArraySet<Message, FragmentSortPolicy> FragmentSet;

public:
  /// Constructors
  FragmentedMessage();
  explicit FragmentedMessage(MoveReference<Message> fragment);

  /// Move Constructor
  FragmentedMessage(MoveReference<FragmentedMessage> rhs);

  /// Comparison Operators (compares message channel sequence IDs)
  bool operator ==(const FragmentedMessage& rhs) const;
  bool operator !=(const FragmentedMessage& rhs) const;
  bool operator  <(const FragmentedMessage& rhs) const;
  bool operator ==(MessageSequenceId rhs) const;
  bool operator !=(MessageSequenceId rhs) const;
  bool operator  <(MessageSequenceId rhs) const;

  //
  // Member Functions
  //

  /// Returns true if the provided fragment is a duplicate, else false
  bool IsDuplicate(const Message& fragment) const;

  /// Adds a fragment to the collection
  void Add(MoveReference<Message> fragment);

  /// Returns true if the collection is complete (all missing fragments have been added), else false
  bool IsComplete() const;

  /// Reconstructs the collection into a whole message
  Message Reconstruct();

private:
  /// Adds a fragment to the collection
  void AddInternal(MoveReference<Message> fragment);

  /// Fragment message collection
  FragmentSet          mFragments;
  /// Final message fragment index
  MessageFragmentIndex mFinalFragmentIndex;
};

/// FragmentedMessage Move-Without-Destruction Operator
template<>
struct MoveWithoutDestructionOperator<FragmentedMessage>
{
  static inline void MoveWithoutDestruction(FragmentedMessage* dest, FragmentedMessage* source)
  {
    new(dest) FragmentedMessage(ZeroMove(*source));
  }
};

} // namespace Zero
