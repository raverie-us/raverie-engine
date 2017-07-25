///////////////////////////////////////////////////////////////////////////////
///
/// \file Event.cpp
/// Implementation of the Event class and support.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(EventObject, builder, type)
{
}

//Right now use Direct memory allocation because events on allocated on different threads.
Memory::Heap* sEventHeap = new Memory::Heap("Events", Memory::GetRoot());

#define UseEventMemoryPool(type) \
  void* type::operator new(size_t size) { return sEventHeap->Allocate(size); }; \
  void type::operator delete(void* pMem, size_t size) { sEventHeap->Deallocate(pMem, size); }


UseEventMemoryPool(Event);
UseEventMemoryPool(EventConnection);
UseEventMemoryPool(EventDispatchList);
UseEventMemoryPool(EventReceiver);
UseEventMemoryPool(EventDispatcher);

namespace Events
{
  DefineEvent(ObjectDestroyed);
}//namespace Events

namespace Tags
{
  DefineTag(Event);
}

//------------------------------------------------------------------------ Event
ZilchDefineType(Event, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldGetterProperty(EventId);
  ZeroBindTag(Tags::Event);
  ZilchBindMethod(Terminate);
}

volatile int gEventCount = 0;

Event::Event()
{
  mTerminated = false;
  AtomicPreIncrement(&gEventCount);
}

Event::Event(const Event& rhs) :
  EventId(rhs.EventId),
  mTerminated(rhs.mTerminated)
{
}

Event& Event::operator=(const Event& rhs)
{
  EventId = rhs.EventId;
  mTerminated = rhs.mTerminated;
  return *this;
}

Event::~Event()
{
  AtomicPreDecrement(&gEventCount);
};

void Event::Terminate()
{
  mTerminated = true;
}

//------------------------------------------------------------------------ ObjectEvent
ZilchDefineType(ObjectEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldGetterProperty(Source);
}

Object* ObjectEvent::GetSource()
{
  return Source;
}

//------------------------------------------------------------- Event Connection
EventConnection::EventConnection()
  :ThisObject(NULL),
   EventType(NULL)
{
}

EventConnection::~EventConnection()
{
  if(!Flags.IsSet(ConnectionFlags::DoNotDisconnect))
  {
    DispatchList::Unlink(this);
    ReceiverList::Unlink(this);
  }
}

// Disabled until all issues are fixed
static const bool CheckAllEventsBound = false;
static const bool CheckEventConnectAsBoundType = false;
static const bool CheckEventDispatchAsBoundType = false;
static const bool CheckEventReceiveAsConnectedType = true;

bool ValidateEvent(StringParam eventId, BoundType* typeSent)
{
  ReturnIf(typeSent == nullptr, false, "Null Event Type");

  // It is possible that this event was bound to a specific type, make sure our event matches the type it sends
  BoundType* boundEventType = MetaDatabase::GetInstance()->mEventMap.FindValue(eventId, nullptr);

  if(CheckAllEventsBound)
  {
    ErrorIf(boundEventType == nullptr, 
      "Attempting to connect to an event that is not bound as sent. Expected type '%s' sent with id '%s'", 
      typeSent->Name.c_str(), eventId.c_str());
  }

  if(CheckEventConnectAsBoundType)
  {
    if(boundEventType)
    {
      // Validate that we are connecting to the correct event type!
      if(!boundEventType->IsA(typeSent))
      {
        String message = String::Format(
          "The event was bound as a %s but you attempted to register an event connection that takes %s",
          boundEventType->Name.c_str(), typeSent->Name.c_str());
        DoNotifyException("Events", message);
        return false;
      }
    }
  }

  return true;
}

void EventConnection::ConnectToReceiverAndDispatcher(
  StringParam eventId, EventReceiver* receiver, EventDispatcher* dispatcher)
{
  ErrorIf(EventType == nullptr, "The event connection should always register it's event parameter type");

  if(!ValidateEvent(eventId, EventType))
    return;

  mEventId = eventId;
  dispatcher->Connect(eventId, this);
  receiver->Connect(this);
}

//----------------------------------------------------------------- Event Signal
EventDispatchList::EventDispatchList()
{

}

EventDispatchList::~EventDispatchList()
{
  OnlyDeleteObjectIn(mConnections);
}

void EventConnection::RaiseError(StringParam message)
{
  DoNotifyExceptionAssert("Event Connection", message);
}

void EventDispatchList::Dispatch(Event* event)
{
  BoundType* sentEventType = ZilchVirtualTypeId(event);

  //if we have no connections then don't do anything
  if(mConnections.Empty())
    return;

  //dispatch to all connections for this event
  EventConnection* connection = &mConnections.Front();
  //we don't want to iterate over any newly added nodes so we iterate to the last
  //item currently in the list (not to the end since that is a sentinel node)
  // WARNING: Caching the back technically makes the iteration less safe because that EventConnection could be deleted
  // In the case that it is deleted, this continues walking forever until it hits th sentinel and crashes
  // To fix this we also check against the end sentinel (similar to previous behavior, see file history)
  // This technically means that if the back EventConnection is deleted, we will walk newly added event connections
  // and dispatch to those too (could have undesired behavior, but at least it isn't crashing)
  EventConnection* back = &mConnections.Back();
  EventConnection* end = mConnections.End();
  EventConnection* current = NULL;

  do
  {
    // To make iteration entirely safe when 'back' gets deleted,
    // we also check if connection hits the end (sentinel), see the warning above
    if (connection == end)
      break;

    current = connection;
    //safe iterate
    connection = mConnections.Next(connection);

    if(CheckEventReceiveAsConnectedType)
    {
      // We should only ever dispatch an event that is either more derived or the exact same as the received event type
      if(!sentEventType->IsA(current->EventType))
      {
        String message = String::Format(
          "Expected a %s, but the event type sent for event %s was %s",
          current->EventType->Name.c_str(), event->EventId.c_str(), sentEventType->Name.c_str());

        current->RaiseError(message);

        // If this is a script connection, we want to skip it (don't want to run invalid code)
        if (current->Flags.IsSet(ConnectionFlags::Script))
          current->Flags.SetFlag(ConnectionFlags::Invalid);
      }
    }

    if(current->Flags.IsSet(ConnectionFlags::Invalid))
    {
      //delete invalid events only during iteration
      //to prevent removed connections from breaking
      //iteration.
      delete current;
    }
    else
      current->Invoke(event);

    if(event->mTerminated)
      break;
  }
  while(current != back);

}

template<typename type>
void RemoveReceiver(type& mConnections, ObjPtr thisObject)
{
  // Walk through all the connections to this list and
  // removed them if they are the receiving object.

  EventConnection* event = &mConnections.Front();
  while(event != mConnections.End())
  {
    // List is intrusive so the pointer must be moved
    // before erasing the current connection.
    EventConnection* current = event;
    event = type::Next(event);

    if(current->ThisObject == thisObject)
    {
      // Mark connection as invalid but do not remove it
      // since the dispatcher may be iterating through the 
      // connections list. The dispatcher will removed all invalid
      // connections during invoking.
      current->Flags.SetFlag(ConnectionFlags::Invalid);
    }
  }
}


void EventDispatchList::Disconnect(ObjPtr thisObject)
{
  RemoveReceiver(mConnections, thisObject);
}

bool EventDispatchList::IsConnected(ObjPtr thisObject)
{
  forRange(EventConnection& connection, mConnections.All())
  {
    if(connection.ThisObject == thisObject)
      return true;
  }
  return false;
}

void EventDispatchList::Connect(EventConnection* connection)
{
  mConnections.PushBack(connection);
}

//--------------------------------------------------------------- Event Receiver
void EventReceiver::Connect(EventConnection* connection)
{
  //Added connect to the intrusive list
  mConnections.PushBack(connection);
}

EventReceiver::EventReceiver()
{

}

void EventReceiver::DestroyConnections()
{
  // Inform all connections that the receiving object has been destroyed.
  // They will remove themselves automatically in the destructor.
  OnlyDeleteObjectIn(mConnections);
}

EventReceiver::~EventReceiver()
{
  DestroyConnections();
}

ReceiverList::range EventReceiver::GetConnections()
{
  return mConnections.All();
}


void EventReceiver::Disconnect(ObjPtr thisObject)
{
  RemoveReceiver(mConnections, thisObject);
}

void EventReceiver::Disconnect(StringParam eventId)
{
  forRange (EventConnection& connection, mConnections.All())
  {
    // Mark all matching connections as invalid so they get removed
    if (connection.mEventId == eventId)
      connection.Flags.SetFlag(ConnectionFlags::Invalid);
  }
}

//------------------------------------------------------------- Event Dispatcher
EventDispatcher::EventDispatcher()
{

}

EventDispatcher::~EventDispatcher()
{
  //Detach all listening objects
  DeleteObjectsInContainer(mEvents);
}

void EventDispatcher::DisconnectEvent(StringParam eventId, ObjPtr thisObject)
{
  EventMapType::range r = mEvents.Find(eventId);
  if(!r.Empty())
  {
    r.Front().second->Disconnect(thisObject);
  }
}

bool EventDispatcher::IsConnected(StringParam eventId, ObjPtr thisObject)
{
  EventMapType::range r = mEvents.Find(eventId);
  if(!r.Empty())
  {
    return r.Front().second->IsConnected(thisObject);
  }
  return false;
}

bool EventDispatcher::IsAnyConnected(StringParam eventId)
{
  EventMapType::range r = mEvents.Find(eventId);
  return !r.Empty();
}

void EventDispatcher::Disconnect(ObjPtr thisObject)
{
  EventMapType::range r = mEvents.All();
  for(;!r.Empty();r.PopFront())
  {
    r.Front().second->Disconnect(thisObject);
  }
}

void EventDispatcher::Dispatch(StringParam eventId, Event* event)
{
  if(event == NULL)
  {
    DoNotifyException("Invalid event", "Cannot dispatch a null event");
    return;
  }

  if(event->mTerminated)
    return;

  BoundType* sentEventType = ZilchVirtualTypeId(event);

  // Validate that, if this event is bound, we're actually sending the proper event!
  BoundType* boundEventType = MetaDatabase::GetInstance()->mEventMap.FindValue(eventId, nullptr);

  if (CheckAllEventsBound)
  {
    // Comment this out if there are valid cases!
    //ErrorIf(boundEventType == nullptr, "Attempting to dispatch an event that is not bound anywhere, we should bind this!");
  }

  if (CheckEventDispatchAsBoundType)
  {
    if(boundEventType)
    {
      // The event type that we're sending should be either more derived or the same type
      if(!sentEventType->IsA(boundEventType))
      {
        String message = String::Format("The event was bound as a %s but you attempted to send a %s",
          boundEventType->Name.c_str(), sentEventType->Name.c_str());
        DoNotifyException("Events", message);
        return;
      }
    }
  }

  // Store the event Id so we can restore it after
  String previousEventId = event->EventId;

  event->EventId = eventId;

  EventMapType::range r = mEvents.Find(eventId);
  if(!r.Empty())
  {
    //Object is listening to this signal.
    //Signal all objects in the signal chain.
    r.Front().second->Dispatch(event);
  }
  else
  {
    //do nothing
  }

  event->EventId = previousEventId;
}

void EventDispatcher::Connect(StringParam eventId, EventConnection* connection)
{
  //Check to see if the signal has been mapped
  EventMapType::range r = mEvents.Find(eventId);
  EventDispatchList* list = nullptr;
  if(!r.Empty())
  {
    list = r.Front().second;
  }
  else
  {
    //Event with that eventId not yet mapped. Make a new list and map the event id
    list = new EventDispatchList();
    mEvents.Insert(eventId, list);
  }

  //Bind the connection to the event list
  list->Connect(connection);
}

void EventObject::DispatchEvent(StringParam eventId, Event* event)
{
  this->GetDispatcher()->Dispatch(eventId, event);
}

}//namespace Zero
