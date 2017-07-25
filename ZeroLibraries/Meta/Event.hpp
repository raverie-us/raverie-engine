///////////////////////////////////////////////////////////////////////////////
///
/// \file Event.hpp
/// Object Event / Messaging System used by engine for cross game object and 
/// system communication.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class EventReceiver;
class EventDispatcher;

//------------------------------------------------------------------------ Event

///Base event class. All events types inherit from this class.
class Event : public ThreadSafeId<u32, Object>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Efficient memory pooling
  OverloadedNew();

  // Constructor
  Event();

  Event(const Event& rhs);
  Event& operator=(const Event& rhs);

  // Make sure the destructor is virtual so we'll clean up resources properly
  virtual ~Event();

  /// Stops the event from being sent to any other connections.
  void Terminate();

  /// The event-ID that this event was dispatched under
  String EventId;
  bool mTerminated;
};

//----------------------------------------------------------------- Object Event
///Simple event for general signals.
class ObjectEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Object* GetSource();

  Object* Source;
  ObjectEvent() : Source(nullptr) {}
  ObjectEvent(Object* source)
    :Source(source)
  {
  }
};


DeclareBitField3(ConnectionFlags, Invalid, DoNotDisconnect, Script);

/// Makes sure a given event string matches a given event type.
/// This should ALWAYS be called before attaching to a receiver and a dispatcher
/// If it returns false, meaning it did not validate, it should not be attached to either!
bool ValidateEvent(StringParam eventId, BoundType* typeSent);

//------------------------------------------------------------- Event Connection
/// A event connection between two objects.
class EventConnection
{
public:
  OverloadedNew();

  EventConnection();
  virtual ~EventConnection();

  // Prints details about the location and name of an event connection
  virtual void RaiseError(StringParam message);

  BitField<ConnectionFlags::Enum> Flags;

  /// Invoke the event
  virtual void Invoke(Event* event)=0;
  virtual void DebugDraw(){};

  /// A helper that connects this connection to both receiver (tests for validation too)
  void ConnectToReceiverAndDispatcher(
    StringParam eventId, EventReceiver* receiver, EventDispatcher* dispatcher);

  /// The ThisObject is the object listening to the event
  /// if the connection is a member function, not the object
  /// that dispatches the event. It normally is the owner
  /// of the EventReceiver but can be different when the object is 
  /// a component listening to a composition which owns the Receiver.
  ObjPtr ThisObject;
  /// Link for all connections on a receiver
  /// contained inside of a object.
  Link<EventConnection> ReceiverLink;
  /// Link for all connection on a signal
  Link<EventConnection> DispatcherLink;
  /// The type that the event is registered for (the parameter type)
  BoundType* EventType;
  /// Name identifier of the event, used by receiver since its connections aren't mapped
  String mEventId;
};

typedef InList<EventConnection, &EventConnection::DispatcherLink> DispatchList;
typedef InList<EventConnection, &EventConnection::ReceiverLink> ReceiverList;

//--------------------------------------------------------------- Event Receiver
/// Object needed to receive events from other objects. Cleans up connections
/// on destruction.
class EventReceiver
{
public:
  OverloadedNew();
  EventReceiver();
  ~EventReceiver();

  /// Add new connection to this Receiver
  void Connect(EventConnection* connection);

  /// Remove all connections with provided 'this' object.
  /// See EventConnection::ThisObject
  void Disconnect(ObjPtr thisObject);
  /// Remove all connections of the given event id.
  void Disconnect(StringParam eventId);

  /// Remove all connections
  void DestroyConnections();

  /// All connections
  ReceiverList::range GetConnections();

private:
  ReceiverList mConnections;
};

//---------------------------------------------------------- Event Dispatch List
/// Object that stores a list of event connections to invoke when Dispatched.
class EventDispatchList
{
public:
  OverloadedNew();
  EventDispatchList();
  ~EventDispatchList();

  /// Dispatch event to all connections
  void Dispatch(Event* event);

  /// Add a new connection to this list
  void Connect(EventConnection* connection);

  /// Remove all connections with given 'this' object
  /// See EventConnection::ThisObject
  void Disconnect(ObjPtr thisObject);

  /// Is the 'this' object on one of the connections in the list
  /// See EventConnection::ThisObject
  bool IsConnected(ObjPtr thisObject);
private:
  DispatchList mConnections;
};

//------------------------------------------------------------- Event Dispatcher
/// Object that enables the dispatching of events. This class allows other 
/// objects to connect to events using if they have an EventReceiver. 
/// Cleans up connections on destruction.
class EventDispatcher
{
public:
  OverloadedNew();
  EventDispatcher();
  ~EventDispatcher();

  /// Dispatch event to all connections
  void Dispatch(StringParam eventId, Event* event);

  /// Add a new EventConnection to this Dispatcher
  void Connect(StringParam eventId, EventConnection* connect);

  /// Remove all connections with the given 'this' 
  /// object from all event lists
  /// See EventConnection::ThisObject
  void Disconnect(ObjPtr thisObject);

  /// Remove all connections with the given 'this' object
  /// from event list with given id
  /// See EventConnection::ThisObject
  void DisconnectEvent(StringParam eventId, ObjPtr thisObject);

  /// Is the 'this' object on one of the connections in the list?
  /// See EventConnection::ThisObject
  bool IsConnected(StringParam eventId, ObjPtr thisObject);

  /// Is anything connected to this event?
  bool IsAnyConnected(StringParam eventId);

private:
  typedef HashMap<String, EventDispatchList*> EventMapType;
  EventMapType mEvents;
};

//--------------------------------------------------- Member Function Connection
template<typename classType, typename eventType>
struct MemberFunctionConnection : public EventConnection
{
  typedef void (classType::*FuncType)(eventType*);
  FuncType MyFunction;
  classType* MyObject;

  MemberFunctionConnection(classType* instance, FuncType func)
  {
    MyObject = instance;
    MyFunction = func;
    ThisObject = MyObject;
  }

  void Invoke(Event* event) override
  {
    (MyObject->*MyFunction)((eventType*)event);
  }
};

///Create an event connection
template<typename targetType, typename classType, typename eventType>
inline void Connect(targetType* dispatcher, StringParam eventId,
                    classType* receiver, void (classType::*function)(eventType*))
{
  ReturnIf(dispatcher==NULL,, "Dispatcher is NULL");

  MemberFunctionConnection<classType, eventType>* connection = 
          new MemberFunctionConnection<classType, eventType>(receiver, function);

  // For safety, we store the event's type on the connection so we can validate it
  BoundType* type = ZilchTypeId(eventType);
  connection->EventType = type;
  ErrorIf(type->IsInitialized() == false,
    "The event type was never initialized using ZilchDeclareType / ZilchDefineType / ZilchInitializeType");

  connection->ConnectToReceiverAndDispatcher(eventId,
    receiver->GetReceiver(), dispatcher->GetDispatcher());
}

#define DeclareEvent(name) extern const String name

#define DefineEvent(name) const String name = #name

#define ConnectThisTo(target, eventname, handle) \
  do { Zero::Connect(target, eventname, this, &ZilchSelf::handle); } while (false)

#define DisconnectAll(sender, receiver) sender->GetDispatcher()->Disconnect(receiver);

#define SignalObjectEvent(event)                                 \
   do                                                            \
   {                                                             \
     ObjectEvent objectEvent(this);                              \
     GetOwner()->GetDispatcher()->Dispatch(event, &objectEvent); \
   } while(false)

//--------------------------------------------------- Static Function Connection
template<typename eventType>
struct StaticFunctionConnection : public EventConnection
{
  typedef void (*FuncType)(eventType*);
  FuncType MyFunction;
  StaticFunctionConnection(FuncType func)
  {
    MyFunction = func;
  }
  virtual void Invoke(Event* event)
  {
    (*MyFunction)((eventType*)event);
  }
};

namespace Events
{
  DeclareEvent(ObjectDestroyed);
}//namespace Events

namespace Tags
{
  DeclareTag(Event);
}

//----------------------------------------------------------------- Event Object
///Helper class for simple adding of Event Dispatching and Receiving
class EventObject : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  //Object Interface
  EventDispatcher* GetDispatcherObject() override { return GetDispatcher(); }
  EventReceiver* GetReceiverObject() override { return GetReceiver(); }

  void DispatchEvent(StringParam eventId, Event* event);
  EventDispatcher* GetDispatcher() { return &mDispatcher; }
  EventReceiver* GetReceiver() { return &mTracker; }

protected:
  EventReceiver mTracker;
  EventDispatcher mDispatcher;
};

}//namespace Zero
