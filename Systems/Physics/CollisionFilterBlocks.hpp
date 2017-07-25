///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2013-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// What kind of events this collision block should out.
DeclareBitField3(CollisionBlockStates, SendEventsToA, SendEventsToB, SendEventsToSpace);
/// What kind of filter block this is. These blocks are used to send out/override
/// collision group events of certain types (collision started, etc...)
DeclareEnum4(CollisionFilterBlockType, CollisionStartedBlock, CollisionPersistedBlock, CollisionEndedBlock, PreSolveBlock);

//-------------------------------------------------------------------CollisionFilterBlock
/// Used to specify which collision group events should be sent out for a CollisionFilter.
/// Allows customizing who gets events (in the filter pair) and what event name is sent out.
struct CollisionFilterBlock : public SafeId32Object
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

//-------------------------------------------------------------------CollisionStartBlock
/// CollisionFilterBlock for CollisionStarted events.
struct CollisionStartBlock : public CollisionFilterBlock
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CollisionStartBlock();
};

//-------------------------------------------------------------------CollisionPersistedBlock
/// CollisionFilterBlock for CollisionPersisted events.
struct CollisionPersistedBlock : public CollisionFilterBlock
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CollisionPersistedBlock();
};

//-------------------------------------------------------------------CollisionEndBlock
/// CollisionFilterBlock for CollisionEnded events.
struct CollisionEndBlock : public CollisionFilterBlock
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CollisionEndBlock();
};

//-------------------------------------------------------------------PreSolveBlock
/// CollisionFilterBlock for sending out an event before collision is solved.
/// Allows modifying object state before collision responses have been calculated.
struct PreSolveBlock : public CollisionFilterBlock
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PreSolveBlock();
};

typedef SimpleResourceFactory<CollisionFilter, CollisionFilterBlock> CollisionFilterMetaComposition;

}//namespace Zero
