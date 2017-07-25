/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_EVENTS_HPP
#define ZILCH_EVENTS_HPP

namespace Zilch
{
  // Declares an event to be sent by an EventHandler
  // The typical pattern in C++ is to declare these within the Events namespace
  #define ZilchDeclareEvent(EventName, EventType) ZeroShared extern const String EventName;

  // Defines the event so only cpp uint allocates the string 
  #define ZilchDefineEvent(EventName) ZeroShared const String EventName = #EventName;

  // All events that are sent must be derived from this type
  class ZeroShared EventData : public IZilchObject
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Mark the destructor as virtual so we can properly clean up resources
    virtual ~EventData();

    // The name that we sent the event under
    String EventName;
  };

  // The virtual base class that represents a callback
  // This class is specialized for C++ static and member and Zilch delegates (can be made for other languages too)
  class ZeroShared EventDelegate
  {
  public:

    // The size of the event delegate must be less than or equal to this size
    static const size_t MaxEventDelegateSize = 128;

    // Default constructor
    EventDelegate();

    // Mark the destructor as virtual so we can properly clean up resources
    virtual ~EventDelegate();

    // Invokes the delegate (with whatever representation we use under the hood)
    // Returns how many event connections this in turn invoked (forwarding will return how many it invoked)
    // In general most handlers will return 1 if it succeeded, or 0 if it failed to invoke the delegate
    virtual int Invoke(EventData* event) = 0;

    // To support safe iteration through events (and modification of event handlers while sending)
    // All event delegates need to be copyable
    // This is achieved by knowing the size of the derived event delegate, and implementing an in place clone
    // Use the macro 'ZilchDefineEventDelegateHelpers' to automatically implement these virtual methods

    // Copy the event delegate in place
    virtual void CopyInto(byte* destination) = 0;

    // Get a unique id or pointer that lets us identify the owner of the delegate (who the delegate is bound to)
    virtual void* GetThisPointerOrUniqueId() = 0;

    // The type of event we accept
    // Note: We'll also accept more derived versions of this type
    BoundType* Type;

    // Every delegate is connected to the list of dispatched events per message type (MessageDelegateList)
    Link<EventDelegate> OutgoingLink;

    // Delegates are also connected to the message handler that will receive them (MessageHandler)
    Link<EventDelegate> IncomingLink;
  };

  // When we create new event delegates, we put this at the top to automatically implement 'GetSize' and 'CopyInto'
  // Our implementation of 'CopyInto' just invokes the copy constructor via placement new
  #define ZilchDefineEventDelegateHelpers(SelfType)                                     \
    void CopyInto(byte* destination) override                                           \
    {                                                                                   \
      ZilchStaticAssert(sizeof(SelfType) <= MaxEventDelegateSize,                       \
        "The size of the event delegate must not exceed MaxEventDelegateSize",          \
        EventDelegateExceedsMaxEventDelegateSize);                                      \
      SelfType* copy = new (destination) SelfType(*this);                               \
      copy->OutgoingLink.Next = nullptr;                                                \
      copy->IncomingLink.Next = nullptr;                                                \
    }

  // We use dual intrusively linked lists with the delegate to ensure that
  // events get destroyed when either the sender or receiver dies
  typedef InList<EventDelegate, &EventDelegate::OutgoingLink> OutgoingList;
  typedef InList<EventDelegate, &EventDelegate::IncomingLink> IncomingList;

  // Stores all outbound connections for a particular event name
  // As an optimization, this list can be pulled out and stored next to an EventHandler (but will be destroyed along with the handler)
  class ZeroShared EventDelegateList
  {
  public:

    // The destructor deletes all outgoing connections
    ~EventDelegateList();

    // Send an event to all outgoing delegates
    int Send(EventData* event);
    
    //******** Internal ********//

    // Store the name of the events associated with this list
    String EventName;

    // The intrusive list of all outgoing connections
    // A connection can be marked as invalid, in which it will be erased
    OutgoingList Outgoing;
  };

  // Stores all outgoing connections
  class ZeroShared EventHandler : public IZilchObject
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Default constructor
    EventHandler();

    // When we get destructed we mark all outgoing and incoming events as destroyed
    ~EventHandler();

    // Get or create a delegate list for the particular event name
    // As an optimization, this list can be pulled out and stored next to an EventHandler (but will be destroyed along with the handler)
    EventDelegateList* GetOrCreateOutgoingDelegateList(StringParam eventName);

    //******** Internal ********//

    // Whenever we send an event, we look if anyone will respond by looking in this map
    // The delegate list stores an intrusive list of all event delegates that need to receive the event
    HashMap<String, EventDelegateList*> OutgoingPerEventName;

    // We keep track of all delegates that would send events to us
    // If we get destroyed, we'll mark all these delegates as destroyed
    IncomingList Incoming;

    // A global event handler (used for registering static functions and such)
    static EventHandler Global;
    ZilchNoCopy(EventHandler);
  };

  // Swap all the events from one handler to another (generally used when we want to disable all events, but save their states)
  ZeroShared void EventSwapAll(EventHandler* a, EventHandler* b);

  // Connects a sender and receiver event handler for a particular event, given an event connection
  ZeroShared void EventConnect(EventHandler* sender, StringParam eventName, EventDelegate* delegate, EventHandler* receiver);

  // Disconnect an event that we previously connected to
  // Disconnecting can also be done by storing the event delegate and deleting it
  // Returns the number of connections that were disconnected
  // Note: There can be more than one disconnected, but only if someone connected to the same event twice on the same object
  ZeroShared int EventDisconnect(EventHandler* sender, EventHandler* receiver, StringParam eventName, void* thisPointerOrUniqueId);

  // Invokes the event handler for anyone listening to this event name on our object
  // Returns how many receiver callbacks were successfully invoked
  ZeroShared int EventSend(EventHandler* sender, StringParam eventName, EventData* event);

  // When we want to connect up member functions, we use this template
  template <typename ClassType, typename EventType>
  class ZeroSharedTemplate MemberFunctionEventDelegate : public EventDelegate
  {
  public:
    ZilchDefineEventDelegateHelpers(MemberFunctionEventDelegate);

    // This is the signature of method that we accept
    typedef void (ClassType::*FunctionType)(EventType* event);

    // We store the member function pointer and the 'this' pointer for the class
    FunctionType FunctionPointer;
    ClassType* ThisPointer;

    // Construct a member function delegate from a class instance and member function pointer
    MemberFunctionEventDelegate(FunctionType function, ClassType* instance) :
      FunctionPointer(function),
      ThisPointer(instance)
    {
      this->Type = ZilchTypeId(EventType);
    }

    // Invoking the delegate just invokes the member function pointer (casts the event type too)
    int Invoke(EventData* event) override
    {
      // Technically the event type cast is unsafe, however we validate that it is safe using our reflection
      (this->ThisPointer->*this->FunctionPointer)((EventType*)event);
      return 1;
    }

    // We can just directly return the this pointer for the member function connection
    void* GetThisPointerOrUniqueId() override
    {
      return (void*)this->ThisPointer;
    }
  };

  // A special template helper that can infer template arguments to make member function connecting easier
  template <typename ClassType, typename EventType>
  ZeroSharedTemplate void EventConnect(EventHandler* sender, StringParam eventName, void (ClassType::*function)(EventType*), ClassType* receiver)
  {
    // Create a member function delegate
    typedef MemberFunctionEventDelegate<ClassType, EventType> EventDelegateType;
    EventDelegateType* eventDelegate = new EventDelegateType(function, receiver);
    
    // Connect the event handler up to this newly created member delegate
    EventConnect(sender, eventName, eventDelegate, receiver);
  }

  // A special template helper that can infer template arguments to make member function connecting easier (receiver is different frmo the class)
  template <typename ClassType, typename EventType>
  ZeroSharedTemplate void EventConnect(EventHandler* sender, StringParam eventName, void (ClassType::*function)(EventType*), ClassType* selfPointer, EventHandler* receiver)
  {
    // Create a member function delegate
    typedef MemberFunctionEventDelegate<ClassType, EventType> EventDelegateType;
    EventDelegateType* eventDelegate = new EventDelegateType(function, selfPointer);
    
    // Connect the event handler up to this newly created member delegate
    EventConnect(sender, eventName, eventDelegate, receiver);
  }

  // When we want to connect up static functions, we use this template
  template <typename EventType>
  class ZeroSharedTemplate StaticFunctionEventDelegate : public EventDelegate
  {
  public:
    ZilchDefineEventDelegateHelpers(StaticFunctionEventDelegate);

    // This is the signature of function that we accept
    typedef void (*FunctionType)(EventType* event);

    // We store the static function pointer to call
    FunctionType FunctionPointer;

    // Construct a member function delegate from a class instance and member function pointer
    StaticFunctionEventDelegate(FunctionType function) :
      FunctionPointer(function)
    {
      this->Type = ZilchTypeId(EventType);
    }

    // Invoking the delegate just invokes the static function pointer (casts the event type too)
    int Invoke(EventData* event) override
    {
      // Technically the event type cast is unsafe, however we validate that it is safe using our reflection
      this->FunctionPointer((EventType*)event);
      return 1;
    }
    
    // Since we have no real 'this' pointer, we're just going to return the function's address
    void* GetThisPointerOrUniqueId() override
    {
      return (void*)this->FunctionPointer;
    }
  };

  // A special template helper that can infer template arguments to make static function connecting easier
  template <typename EventType>
  ZeroSharedTemplate void EventConnect(EventHandler* sender, StringParam eventName, void (*function)(EventType*), EventHandler* receiver = nullptr)
  {
    // We use the global event handler here (if there is no receiver...)
    if (receiver == nullptr)
      receiver = &EventHandler::Global;

    // Create a member function delegate
    typedef StaticFunctionEventDelegate<EventType> EventDelegateType;
    EventDelegateType* eventDelegate = new EventDelegateType(function);
    
    // Connect the event handler up to this newly created static function delegate
    EventConnect(sender, eventName, eventDelegate, receiver);
  }

  // When we want to connect up static functions with userdata, we use this template
  template <typename EventType>
  class ZeroSharedTemplate StaticFunctionUserDataEventDelegate : public EventDelegate
  {
  public:
    ZilchDefineEventDelegateHelpers(StaticFunctionUserDataEventDelegate);

    // This is the signature of function that we accept
    typedef void (*FunctionType)(EventType* event, void* userData);

    // We store the static function pointer to call and the user-data
    FunctionType FunctionPointer;
    void* UserData;

    // Construct a member function delegate from a class instance and member function pointer
    StaticFunctionUserDataEventDelegate(FunctionType function, void* userData) :
      FunctionPointer(function),
      UserData(userData)
    {
      this->Type = ZilchTypeId(EventType);
    }

    // Invoking the delegate just invokes the static function pointer (casts the event type too)
    int Invoke(EventData* event) override
    {
      // Technically the event type cast is unsafe, however we validate that it is safe using our reflection
      this->FunctionPointer((EventType*)event, this->UserData);
      return 1;
    }
    
    // Return whatever the user gave us as user-data
    void* GetThisPointerOrUniqueId() override
    {
      return (void*)this->UserData;
    }
  };

  // A special template helper that can infer template arguments to make static function connecting easier (with user-data)
  template <typename EventType>
  ZeroSharedTemplate void EventConnect(EventHandler* sender, StringParam eventName, void (*function)(EventType*, void*), void* userData = nullptr, EventHandler* receiver = nullptr)
  {
    // We use the global event handler here (if there is no receiver...)
    if (receiver == nullptr)
      receiver = &EventHandler::Global;

    // Create a member function delegate
    typedef StaticFunctionUserDataEventDelegate<EventType> EventDelegateType;
    EventDelegateType* eventDelegate = new EventDelegateType(function, userData);
    
    // Connect the event handler up to this newly created static function delegate
    EventConnect(sender, eventName, eventDelegate, receiver);
  }

  // A simple event delegate that just forwards events to another EventHandler
  class ZeroShared ForwardingEventDelegate : public EventDelegate
  {
  public:
    ZilchDefineEventDelegateHelpers(ForwardingEventDelegate);

    // The event handler we forward received events to
    // Note: This is safe to store as a pointer because we only allow forwarding to
    // whoever this event delegate is connected to (OutgoingLink)
    // If that object gets deleted, this delegate will automatically be deleted
    EventHandler* ForwardTo;

    // Construct a member function delegate from a class instance and member function pointer
    ForwardingEventDelegate(EventHandler* forwardTo);

    // EventDelegate interface
    int Invoke(EventData* event);
    void* GetThisPointerOrUniqueId();
  };

  // An event delegate that can call Zilch code
  class ZeroShared ZilchEventDelegate : public EventDelegate
  {
  public:
    ZilchDefineEventDelegateHelpers(ZilchEventDelegate);

    // Constructs the event from the Zilch delegate
    // NOTE: Must be validated by 'ValidateEventConnection prior to constructing!
    ZilchEventDelegate(const Delegate& delegate, ExecutableState* state);

    // The delegate *safely* stores the handle to the object as well as the function to call
    Delegate FunctionWithThis;

    // The state that we want to call this event in
    // The state should always be valid because if we destroy the state,
    // all objects get destroyed, and all events disconnected
    ExecutableState* State;

    // EventDelegate interface
    int Invoke(EventData* event);
    void* GetThisPointerOrUniqueId();
  };

  // Automatically forwards all events of a type of event name to another receiver
  ZeroShared void EventForward(EventHandler* sender, StringParam eventName, EventHandler* receiver);

  // A class that represents the 'Events' class within Zilch code (for binding purposes)
  class ZeroShared EventsClass
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Sends an event out to anyone listening
    // Events are only accepted if their type is either the same or more derived then the callback's event type
    // Returns the number of listeners that received the event
    static int Send(const Handle& sender, StringParam eventName, EventData* event);

    // Listen for a given event sent by the 'sender'; invoke the callback when it is received
    // Event connections form a two way binding between the sender and the receiver
    // If either the sender or receiver is deleted, then the connection is broken
    // Here, the receiver is implicitly grabbed from the 'this' handle on the callback delegate
    // Returns an id that can be used to manually 'Disconnect' the event
    // When using a static function as a callback, the user must manually disconnect the event
    // Connect validates that a delegate is in proper form (and will throw an exception if it is not)
    // This validates that we did not get a null delegate, the this pointer was not null (static functions are ok!),
    // and that the delegate has no return and only one argument that inherits from the EventData type
    static void Connect(const Handle& sender, StringParam eventName, const Delegate& callback);
  };
}

#endif
