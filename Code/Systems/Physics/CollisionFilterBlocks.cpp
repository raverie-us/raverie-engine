// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(CollisionFilterBlock, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetter(BlockType);
  RaverieBindGetterSetterProperty(SendEventsToA);
  RaverieBindGetterSetterProperty(SendEventsToB);
  RaverieBindGetterSetterProperty(SendEventsToSpace);
  RaverieBindFieldProperty(mEventOverride);
}

CollisionFilterBlock::CollisionFilterBlock()
{
  mStates.Clear();
}

void CollisionFilterBlock::Serialize(Serializer& stream)
{
  uint defaultFlags = CollisionBlockStates::SendEventsToA | CollisionBlockStates::SendEventsToB;
  SerializeBits(stream, mStates, CollisionBlockStates::Names, 0, defaultFlags);
  SerializeNameDefault(mEventOverride, String());
}

bool CollisionFilterBlock::GetSendEventsToA()
{
  return mStates.IsSet(CollisionBlockStates::SendEventsToA);
}

void CollisionFilterBlock::SetSendEventsToA(bool state)
{
  mStates.SetState(CollisionBlockStates::SendEventsToA, state);
}

bool CollisionFilterBlock::GetSendEventsToB()
{
  return mStates.IsSet(CollisionBlockStates::SendEventsToB);
}

void CollisionFilterBlock::SetSendEventsToB(bool state)
{
  mStates.SetState(CollisionBlockStates::SendEventsToB, state);
}

bool CollisionFilterBlock::GetSendEventsToSpace()
{
  return mStates.IsSet(CollisionBlockStates::SendEventsToSpace);
}

void CollisionFilterBlock::SetSendEventsToSpace(bool state)
{
  mStates.SetState(CollisionBlockStates::SendEventsToSpace, state);
}

CollisionFilterBlockType::Enum CollisionFilterBlock::GetBlockType() const
{
  if (mBlockType & FilterFlags::StartEvent)
    return CollisionFilterBlockType::CollisionStartedBlock;
  if (mBlockType & FilterFlags::PersistedEvent)
    return CollisionFilterBlockType::CollisionPersistedBlock;
  if (mBlockType & FilterFlags::EndEvent)
    return CollisionFilterBlockType::CollisionEndedBlock;
  if (mBlockType & FilterFlags::PreSolveEvent)
    return CollisionFilterBlockType::PreSolveBlock;
  return CollisionFilterBlockType::CollisionStartedBlock;
}

RaverieDefineType(CollisionStartBlock, builder, type)
{
  RaverieBindComponent();
  type->HasOrAdd<::Raverie::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
}

CollisionStartBlock::CollisionStartBlock()
{
  mBlockType = FilterFlags::StartEvent;
}

RaverieDefineType(CollisionPersistedBlock, builder, type)
{
  RaverieBindComponent();
  type->HasOrAdd<::Raverie::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
}

CollisionPersistedBlock::CollisionPersistedBlock()
{
  mBlockType = FilterFlags::PersistedEvent;
}

RaverieDefineType(CollisionEndBlock, builder, type)
{
  RaverieBindComponent();
  type->HasOrAdd<::Raverie::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
}

CollisionEndBlock::CollisionEndBlock()
{
  mBlockType = FilterFlags::EndEvent;
}

RaverieDefineType(PreSolveBlock, builder, type)
{
  RaverieBindComponent();
  type->HasOrAdd<::Raverie::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
}

PreSolveBlock::PreSolveBlock()
{
  mBlockType = FilterFlags::PreSolveEvent;
}

RaverieDefineTemplateType(CollisionFilterMetaComposition, builder, type)
{
}

} // namespace Raverie
