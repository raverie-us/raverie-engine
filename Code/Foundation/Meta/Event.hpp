// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
class EventReceiver;
class EventDispatcher;

/// Base event class. All events types inherit from this class.
class Event : public ThreadSafeId<u32, Object>
{
public:
  RaverieDeclareType(Event, TypeCopyMode::ReferenceType);

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

/// Simple event for general signals.
class ObjectEvent : public Event
{
public:
  RaverieDeclareType(ObjectEvent, TypeCopyMode::ReferenceType);

  Object* GetSource();

  Object* Source;
  ObjectEvent() : Source(nullptr)
  {
  }
  ObjectEvent(Object* source) : Source(source)
  {
  }
};

DeclareBitField3(ConnectionFlags, Invalid, DoNotDisconnect, Script);

/// Makes sure a given event string matches a given event type.
/// This should ALWAYS be called before attaching to a receiver and a dispatcher
/// If it returns false, meaning it did not validate, it should not be attached
/// to either!
bool ValidateEvent(StringParam eventId, BoundType* typeSent);

/// A event connection between two objects.
class EventConnection
{
public:
  OverloadedNew();

  EventConnection(EventDispatcher* dispatcher, StringParam eventId);
  virtual ~EventConnection();

  // Prints details about the location and name of an event connection
  virtual void RaiseError(StringParam message);

  BitField<ConnectionFlags::Enum> Flags;

  /// Invoke the event
  virtual void Invoke(Event* event) = 0;
  virtual void DebugDraw(){};

  virtual DataBlock GetFunctionPointer() = 0;
  size_t Hash();
  bool operator==(EventConnection& lhs);

  /// A helper that connects this connection to both receiver (tests for
  /// validation too)
  void ConnectToReceiverAndDispatcher(StringParam eventId, EventReceiver* receiver, EventDispatcher* dispatcher);

  /// Removes this specific event connection from the dispatcher it is attached
  /// to
  void DisconnectSelf();

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
  /// Link for all queued event disconnects
  Link<EventConnection> DisconnectLink;
  /// This dispatcher for this event connection
  EventDispatcher* mDispatcher;
  /// The type that the event is registered for (the parameter type)
  BoundType* EventType;
  /// Name identifier of the event, used by receiver since its connections
  /// aren't mapped
  String mEventId;

  // Keeps handles alive until a safe time for destruction.
  static Array<Delegate> sDelayDestructDelegates;
  // Clears static array of delegates.
  static void DelayDestructDelegates();
};

typedef InList<EventConnection, &EventConnection::DispatcherLink> DispatchList;
typedef InList<EventConnection, &EventConnection::ReceiverLink> ReceiverList;
typedef InList<EventConnection, &EventConnection::DisconnectLink> DisconnectList;

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

// Hash Policy
struct ConnectionPointerHashPolicy
{
  // Hashing operator
  size_t operator()(EventConnection* value)
  {
    return value->Hash();
  }

  // Comparison operator
  bool Equal(EventConnection* a, EventConnection* b)
  {
    return (*a == *b);
  }
};

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

  /// Check if anyone has signed up for a particular event.
  bool HasReceivers(StringParam eventId);

  /// Add a new EventConnection to this Dispatcher
  void Connect(StringParam eventId, EventConnection* connect);

  bool IsUniqueConnection(EventConnection* connection);

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
  friend class EventConnection;
  typedef HashMap<String, EventDispatchList*> EventMapType;
  EventMapType mEvents;

public:
  HashSet<EventConnection*, ConnectionPointerHashPolicy> mUniqueConnections;
};

template <typename classType, typename eventType>
struct MemberFunctionConnection : public EventConnection
{
  typedef void (classType::*FuncType)(eventType*);
  typedef void (classType::** FuncPointer)(eventType*);
  FuncType MyFunction;
  classType* MyObject;

  MemberFunctionConnection(EventDispatcher* dispatcher, StringParam eventId, classType* instance, FuncType func) : EventConnection(dispatcher, eventId)
  {
    MyObject = instance;
    MyFunction = func;
    ThisObject = MyObject;
  }

  void Invoke(Event* event) override
  {
    (MyObject->*MyFunction)((eventType*)event);
  }

  DataBlock GetFunctionPointer() override
  {
    FuncPointer functionPointer = &MyFunction;
    return DataBlock((byte*)functionPointer, sizeof(FuncType));
  }
};

/// Create an event connection
template <typename targetType, typename classType, typename eventType>
inline void Connect(targetType* dispatcherObject, StringParam eventId, classType* receiver, void (classType::*function)(eventType*))
{
  ReturnIf(dispatcherObject == nullptr, , "Dispatcher object is null");
  ReturnIf(receiver == nullptr || !receiver->GetReceiver(), , "Receiver is null");
  EventDispatcher* dispatcher = dispatcherObject->GetDispatcher();
  ReturnIf(dispatcher == nullptr, , "Dispatcher is null");

  MemberFunctionConnection<classType, eventType>* connection = new MemberFunctionConnection<classType, eventType>(dispatcher, eventId, receiver, function);

  if (!dispatcher->IsUniqueConnection(connection))
  {
    connection->Flags.SetFlag(ConnectionFlags::DoNotDisconnect);
    delete connection;
    return;
  }

  // For safety, we store the event's type on the connection so we can validate
  // it
  BoundType* type = RaverieTypeId(eventType);
  connection->EventType = type;
  ErrorIf(type->IsInitialized() == false,
          "The event type was never initialized using RaverieDeclareType / "
          "RaverieDefineType / RaverieInitializeType");

  connection->ConnectToReceiverAndDispatcher(eventId, receiver->GetReceiver(), dispatcher);
}

#define DeclareEvent(name) extern const String name

#define DefineEvent(name) const String name = #name

#define ConnectThisTo(target, eventname, handle)                                                                                                                                                       \
  do                                                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    ::Raverie::Connect(target, eventname, this, &RaverieSelf::handle);                                                                                                                                 \
  } while (false)

#define DisconnectAll(sender, receiver) sender->GetDispatcher()->Disconnect(receiver);

#define SignalObjectEvent(event)                                                                                                                                                                       \
  do                                                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    ObjectEvent objectEvent(this);                                                                                                                                                                     \
    GetOwner()->GetDispatcher()->Dispatch(event, &objectEvent);                                                                                                                                        \
  } while (false)

template <typename eventType>
struct StaticFunctionConnection : public EventConnection
{
  typedef void (*FuncType)(eventType*);
  typedef void (**FuncPointer)(eventType*);
  FuncType MyFunction;
  StaticFunctionConnection(EventDispatcher* dispatcher, StringParam eventId, FuncType func) : EventConnection(dispatcher, eventId)
  {
    MyFunction = func;
  }
  virtual void Invoke(Event* event)
  {
    (*MyFunction)((eventType*)event);
  }

  DataBlock GetFunctionPointer() override
  {
    FuncPointer functionPointer = &MyFunction;
    return DataBlock((byte*)functionPointer, sizeof(FuncType));
  }
};

namespace Events
{
DeclareEvent(ObjectDestroyed);
} // namespace Events

namespace Tags
{
DeclareTag(Event);
}

/// Helper class for simple adding of Event Dispatching and Receiving
class EventObject : public Object
{
public:
  RaverieDeclareType(EventObject, TypeCopyMode::ReferenceType);

  // Object Interface
  EventDispatcher* GetDispatcherObject() override
  {
    return GetDispatcher();
  }
  EventReceiver* GetReceiverObject() override
  {
    return GetReceiver();
  }

  void DispatchEvent(StringParam eventId, Event* event);
  EventDispatcher* GetDispatcher()
  {
    return &mDispatcher;
  }
  EventReceiver* GetReceiver()
  {
    return &mTracker;
  }

  /// Check if anyone has signed up for a particular event.
  bool HasReceivers(StringParam eventId);

protected:
  EventReceiver mTracker;
  EventDispatcher mDispatcher;
};

} // namespace Raverie
