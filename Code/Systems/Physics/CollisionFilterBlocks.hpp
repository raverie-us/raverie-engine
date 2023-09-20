// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// What kind of events this collision block should out.
DeclareBitField3(CollisionBlockStates, SendEventsToA, SendEventsToB, SendEventsToSpace);
/// What kind of filter block this is. These blocks are used to send
/// out/override collision group events of certain types (collision started,
/// etc...)
DeclareEnum4(
    CollisionFilterBlockType, CollisionStartedBlock, CollisionPersistedBlock, CollisionEndedBlock, PreSolveBlock);

/// Used to specify which collision group events should be sent out for a
/// CollisionFilter. Allows customizing who gets events (in the filter pair) and
/// what event name is sent out.
struct CollisionFilterBlock : public SafeId32Object
{
  RaverieDeclareType(CollisionFilterBlock, TypeCopyMode::ReferenceType);

  CollisionFilterBlock();

  void Serialize(Serializer& stream) override;

  /// Does the first object in the filter get this event type sent to it?
  bool GetSendEventsToA();
  void SetSendEventsToA(bool state);
  /// Does the second object in the filter get this event type sent to it?
  bool GetSendEventsToB();
  void SetSendEventsToB(bool state);
  /// Does the active space of the objects get this event type sent to it?
  bool GetSendEventsToSpace();
  void SetSendEventsToSpace(bool state);

  /// What type of collision filter block is this?
  CollisionFilterBlockType::Enum GetBlockType() const;

  /// What event name to send out when this block triggers. If left empty the
  /// default name will be used (e.g. GroupCollisionStarted).
  String mEventOverride;
  BitField<CollisionBlockStates::Enum> mStates;

  // Defined by the FilterFlags state in CollisionFilter.hpp
  FilterFlags::Enum mBlockType;
};

/// CollisionFilterBlock for CollisionStarted events.
struct CollisionStartBlock : public CollisionFilterBlock
{
  RaverieDeclareType(CollisionStartBlock, TypeCopyMode::ReferenceType);

  CollisionStartBlock();
};

/// CollisionFilterBlock for CollisionPersisted events.
struct CollisionPersistedBlock : public CollisionFilterBlock
{
  RaverieDeclareType(CollisionPersistedBlock, TypeCopyMode::ReferenceType);

  CollisionPersistedBlock();
};

/// CollisionFilterBlock for CollisionEnded events.
struct CollisionEndBlock : public CollisionFilterBlock
{
  RaverieDeclareType(CollisionEndBlock, TypeCopyMode::ReferenceType);

  CollisionEndBlock();
};

/// CollisionFilterBlock for sending out an event before collision is solved.
/// Allows modifying object state before collision responses have been
/// calculated.
struct PreSolveBlock : public CollisionFilterBlock
{
  RaverieDeclareType(PreSolveBlock, TypeCopyMode::ReferenceType);

  PreSolveBlock();
};

typedef SimpleResourceFactory<CollisionFilter, CollisionFilterBlock> CollisionFilterMetaComposition;

} // namespace Raverie
