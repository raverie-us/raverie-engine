///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  EventRange                                     //
//---------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------//
//                           EventBundleMetaComposition                            //
//---------------------------------------------------------------------------------//

ZilchDefineType(EventBundleMetaComposition, builder, type)
{
}

//
// MetaComposition Interface
//

EventBundleMetaComposition::EventBundleMetaComposition() :
  MetaComposition(ZilchTypeId(Event))
{

}

/// "Component" Instance Management
uint EventBundleMetaComposition::GetComponentCount(HandleParam instance)
{
  // Get event bundle instance
  EventBundle* eventBundle = instance.Get<EventBundle*>();

  // Return event count
  return eventBundle->GetEventCount();
}

Handle EventBundleMetaComposition::GetComponentAt(HandleParam instance, uint index)
{
  // Get event bundle instance
  EventBundle* eventBundle = instance.Get<EventBundle*>();

  // Return event by index
  return Handle(eventBundle->GetEventByIndex(index));
}

Handle EventBundleMetaComposition::GetComponent(HandleParam instance, BoundType* boundType)
{
  // *** Note: At the moment this is our only MetaComposition function actually being used ***

  // Get event bundle instance
  EventBundle* eventBundle = instance.Get<EventBundle*>();

  // Return event by type ID
  return Handle(eventBundle->GetEventByType(boundType));
}

uint EventBundleMetaComposition::GetComponentIndex(HandleParam instance, BoundType* boundType)
{
  // Get event bundle instance
  EventBundle* eventBundle = instance.Get<EventBundle*>();

  // Return event index by type ID
  return eventBundle->GetEventIndexByType(boundType);
}

void EventBundleMetaComposition::AddComponent(HandleParam instance, HandleParam subObject, int index, bool ignoreDependencies)
{
  // Get event bundle instance
  EventBundle* eventBundle = instance.Get<EventBundle*>();

  // Get event instance
  Event* event = subObject.Get<Event*>();

  // Add event to event bundle
  eventBundle->AddEvent(event);
}

void EventBundleMetaComposition::RemoveComponent(HandleParam instance, HandleParam subObject,
                                                 bool ignoreDependencies)
{
  BoundType* typeToRemove = subObject.StoredType;

  // Get event bundle instance
  EventBundle* eventBundle = instance.Get<EventBundle*>();

  // Get event instance
  Event* event = subObject.Get<Event*>();
  Assert(ZilchVirtualTypeId(event) == typeToRemove);

  // Remove event by type ID from event bundle
  bool success = eventBundle->RemoveEventByType(typeToRemove);
  ErrorIf(success == false, "Unable to remove event by type ID from event bundle - Event not found");
}

//---------------------------------------------------------------------------------//
//                                 EventBundle                                     //
//---------------------------------------------------------------------------------//

ZilchDefineType(EventBundle, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // Bind destructor
  ZilchBindDestructor();

  // Bind constructor
  ZilchBindDefaultConstructor();
  ZilchBindConstructor(Event*);
  ZilchBindConstructor(GameSession*);
  ZilchBindConstructor(GameSession*, Event*);
  ZilchBindConstructor(const EventBundle&);

  // Bind interface
  ZilchBindCustomGetterProperty(IsEmpty);
  ZilchBindMethod(AddEvent);
  ZilchBindGetterSetter(GameSession);
  // ZilchBindMethod(GetEventByTypeName);
  // ZilchBindMethod(GetEventByType);
  // ZilchBindMethod(GetEventByIndex);
  // ZilchBindMethod(GetEventIndexByType);
  ZilchBindMethod(GetEvents);
  // ZilchBindMethod(GetEventCount);
  ZilchBindMethod(RemoveEvent);
  ZilchBindMethodAs(RemoveEventByTypeName, "RemoveEvent");
  // ZilchBindMethod(RemoveEventByType);
  // ZilchBindMethod(RemoveEventByIndex);
  ZilchBindMethod(Clear);

  // Make constructible in script
  type->CreatableInScript = true;

  // Set meta composition
  type->Add(new EventBundleMetaComposition);
}

EventBundle::EventBundle()
  : mGameSession(nullptr),
    mBitStream(),
    mEvents(),
    mNeedToSerialize(false),
    mNeedToDeserialize(false)
{
}

EventBundle::EventBundle(Event* event)
  : mGameSession(nullptr),
    mBitStream(),
    mEvents(),
    mNeedToSerialize(false),
    mNeedToDeserialize(false)
{
  // Add specified event
  AddEvent(event);
}

EventBundle::EventBundle(GameSession* gameSession)
  : mGameSession(gameSession),
    mBitStream(),
    mEvents(),
    mNeedToSerialize(false),
    mNeedToDeserialize(false)
{
  Assert(mGameSession);
}

EventBundle::EventBundle(GameSession* gameSession, Event* event)
  : mGameSession(gameSession),
    mBitStream(),
    mEvents(),
    mNeedToSerialize(false),
    mNeedToDeserialize(false)
{
  Assert(mGameSession);

  // Add specified event
  AddEvent(event);
}

EventBundle::EventBundle(const EventBundle& rhs)
  : mGameSession(rhs.mGameSession),
    mBitStream(),
    mEvents(),
    mNeedToSerialize(false),
    mNeedToDeserialize(false)
{
  //Assert(mGameSession);

  // Serialize rhs events if needed
  if(rhs.mNeedToSerialize)
    const_cast<EventBundle&>(rhs).SerializeEventsToBitStream();

  // Copy bitstream
  static_cast<BitStream&>(mBitStream) = static_cast<const BitStream&>(rhs.mBitStream);
  mNeedToDeserialize = true; // (Our bitstream has been modified)
}

EventBundle::~EventBundle()
{
  // Clear our event bundle
  Clear();
}

EventBundle& EventBundle::operator =(const EventBundle& rhs)
{
  // Copy game session
  mGameSession = rhs.mGameSession;

  // Clear our event bundle
  Clear();

  // Serialize rhs events if needed
  if(rhs.mNeedToSerialize)
    const_cast<EventBundle&>(rhs).SerializeEventsToBitStream();

  // Copy bitstream
  static_cast<BitStream&>(mBitStream) = static_cast<const BitStream&>(rhs.mBitStream);
  mNeedToDeserialize = true; // (Our bitstream has been modified)

  return *this;
}

EventBundle& EventBundle::operator =(const BitStream& rhs)
{
  // Clear our event bundle
  Clear();

  // Copy bitstream
  mBitStream = rhs;
  mNeedToDeserialize = true; // (Our bitstream has been modified)

  return *this;
}

EventBundle& EventBundle::operator =(MoveReference<BitStream> rhs)
{
  // Clear our event bundle
  Clear();

  // Move bitstream
  mBitStream = ZeroMove(rhs);
  mNeedToDeserialize = true; // (Our bitstream has been modified)

  return *this;
}

//
// Interface
//

void EventBundle::SetGameSession(GameSession* gameSession)
{
  mGameSession = gameSession;
}

GameSession* EventBundle::GetGameSession()
{
  return mGameSession;
}

bool EventBundle::IsEmpty()
{
  return mBitStream.IsEmpty()
      && mEvents.Empty();
}

bool EventBundle::AddEvent(Event* event)
{
  // Null event?
  if(!event)
  {
    Assert(false);
    return false;
  }

  // Get event type
  BoundType* eventType = ZilchVirtualTypeId(event);
  if(!eventType)
  {
    Assert(false);
    return false;
  }

  // Serialize events to bitstream as needed
  if(mNeedToSerialize)
    SerializeEventsToBitStream();

  // An event with that event ID has already been added?
  if(GetEventByTypeName(eventType->Name))
    return false;

  // // Set event ID on event
  // event->EventId = meta->TypeName;

  // Write event to bitstream
  mBitStream.WriteEvent(event);
  mNeedToDeserialize = true; // (Our bitstream has been modified)
  return true;
}

Event* EventBundle::GetEventByTypeName(StringParam eventTypeName)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Find event by type name
  for(uint i = 0; i < mEvents.Size(); ++i)
    if(ZilchVirtualTypeId(static_cast<Event*>(mEvents[i]))->Name == eventTypeName) // Found?
    {
      // Return event by index
      return GetEventByIndex(i);
    }

  // Not found
  return nullptr;
}

Event* EventBundle::GetEventByType(BoundType* eventType)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Find event by type ID
  for(uint i = 0; i < mEvents.Size(); ++i)
    if(ZilchVirtualTypeId(static_cast<Event*>(mEvents[i])) == eventType) // Found?
    {
      // Return event by index
      return GetEventByIndex(i);
    }

  // Not found
  return nullptr;
}

Event* EventBundle::GetEventByIndex(uint index)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Invalid index?
  if(mEvents.Size() < (index + 1))
    return nullptr;

  // Return event
  mNeedToSerialize = true; // (Our events may be modified by the user)
  return mEvents[index];
}

uint EventBundle::GetEventIndexByType(BoundType* eventType)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Find event by type ID
  for(uint i = 0; i < mEvents.Size(); ++i)
    if(ZilchVirtualTypeId(static_cast<Event*>(mEvents[i])) == eventType)
      return i; // Return index

  // Not found
  return -1;
}

EventRange EventBundle::GetEvents()
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Return events
  mNeedToSerialize = true; // (Our events may be modified by the user)
  return mEvents.All();
}

uint EventBundle::GetEventCount()
{
  return GetEvents().Size();
}

bool EventBundle::RemoveEvent(Event* event)
{
  // Null event?
  if(!event)
  {
    Assert(false);
    return false;
  }

  // Get event type
  BoundType* eventType = ZilchVirtualTypeId(event);
  if(!eventType)
  {
    Assert(false);
    return false;
  }

  // Remove event by type name
  return RemoveEventByTypeName(eventType->Name);
}

bool EventBundle::RemoveEventByTypeName(StringParam eventTypeName)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Find event by type name
  for(uint i = 0; i < mEvents.Size(); ++i)
    if(ZilchVirtualTypeId(static_cast<Event*>(mEvents[i]))->Name == eventTypeName) // Found?
    {
      // Remove event by index
      return RemoveEventByIndex(i);
    }

  // Not found
  return false;
}

bool EventBundle::RemoveEventByType(BoundType* eventType)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Find event by type ID
  for(uint i = 0; i < mEvents.Size(); ++i)
    if(ZilchVirtualTypeId(static_cast<Event*>(mEvents[i])) == eventType) // Found?
    {
      // Remove event by index
      return RemoveEventByIndex(i);
    }

  // Not found
  return false;
}

bool EventBundle::RemoveEventByIndex(uint index)
{
  // Deserialize bitstream to events as needed
  if(mNeedToDeserialize)
    DeserializeBitStreamToEvents();

  // Invalid index?
  if(mEvents.Size() < (index + 1))
    return false;

  // Remove event
  mEvents.EraseAt(index);
  mNeedToSerialize = true; // (Our events have been modified)
  return true;
}

void EventBundle::Clear()
{
  // Clear bitstream
  mBitStream.Clear(false);

  // Clear events
  mEvents.Clear();

  // Reset dirty flags
  mNeedToSerialize = false;
  mNeedToDeserialize = false;
}

BitStream& EventBundle::GetBitStream()
{
  // Serialize events to bitstream as needed
  if(mNeedToSerialize)
    SerializeEventsToBitStream();

  // Return bitstream
  return mBitStream;
}

//
// Internal
//

bool EventBundle::SerializeEventsToBitStream()
{
  // Clear bitstream
  // (Don't free memory because we're likely to write a similar size)
  // (This is admittedly inefficient but EventBundle isn't intended for performance critical situations)
  mBitStream.Clear(false);

  // Write all events to bitstream
  forRange(Event* event, mEvents.All())
    mBitStream.WriteEvent(event);

  // Success
  return true;
}

bool EventBundle::DeserializeBitStreamToEvents()
{
  // Gamesession not set?
  if(!mGameSession)
  {
    DoNotifyException("Invalid EventBundle Operation", "EventBundle needs a GameSession in order to deserialize Events");
    return false;
  }

  // Clear events
  // (These are outdated and we are going to replace them here)
  // (This is admittedly inefficient but EventBundle isn't intended for performance critical situations)
  mEvents.Clear();

  // Reset read cursor to start of bitstream
  mBitStream.ClearBitsRead();

  // Bitstream is able to read an event?
  while(mBitStream.CanReadEvent())
  {
    // Read event from bitstream
    Event* event = mBitStream.ReadEvent(mGameSession);
    if(!event) // Unable?
    {
      DoNotifyError("Failed EventBundle Operation", "Error deserializing an Event contained in the EventBundle");
      return false;
    }

    // Add to deserialized events
    mEvents.PushBack(event);
  }

  // (If we were unable to deserialize any events, bitstream should be empty)
  if(mEvents.Empty())
    Assert(mBitStream.IsEmpty());

  // Success
  return true;
}

} // namespace Zero
