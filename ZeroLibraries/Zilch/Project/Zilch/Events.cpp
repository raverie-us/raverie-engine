/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  // For the moment we let the user use a special define to disable event safety, because right now events have issues
  #if !defined(ZilchUnsafeEvents)
    #define ZilchUnsafeEvents false
  #endif

  //***************************************************************************
  // When we attempt to 'get an event handler' from a type that is actually an EventHandler
  // then we really don't need to do anything except return ourself
  EventHandler* EventHandlerGetEventHandlerFunction(const BoundType* type, const byte* data)
  {
    return (EventHandler*)data;
  }

  //***************************************************************************
  ZilchDefineType(EventHandler, builder, type)
  {
    ZilchFullBindDestructor(builder, type, EventHandler);
    ZilchFullBindConstructor(builder, type, EventHandler, ZilchNoNames);
    type->GetEventHandlerFunction = EventHandlerGetEventHandlerFunction;
  }

  //***************************************************************************
  ZilchDefineType(EventsClass, builder, type)
  {
    ZilchFullBindMethod(builder, type, &EventsClass::Send, ZilchNoOverload, "Send", "sender, eventName, event");
    ZilchFullBindMethod(builder, type, &EventsClass::Connect, ZilchNoOverload, "Connect", "sender, eventName, callback");
  }

  //***************************************************************************
  ZilchDefineType(EventData, builder, type)
  {
    ZilchFullBindDestructor(builder, type, EventData);
    ZilchFullBindConstructor(builder, type, EventData, ZilchNoNames);
  }

  //***************************************************************************
  // Definition of static members
  EventHandler EventHandler::Global;

  //***************************************************************************
  EventData::~EventData()
  {
  }
  
  //***************************************************************************
  EventDelegate::EventDelegate() :
    Type(nullptr)
  {
  }
  
  //***************************************************************************
  EventDelegate::~EventDelegate()
  {
    // Event delegates are always responsible for unlinking themselves from the intrusive lists
    if (this->IncomingLink.Next != nullptr)
      IncomingList::Unlink(this);
    if (this->OutgoingLink.Next != nullptr)
      OutgoingList::Unlink(this);
  }

  //***************************************************************************
  EventDelegateList::~EventDelegateList()
  {
    // Delete all outgoing connections (using safe iteration)
    OnlyDeleteObjectIn(this->Outgoing);
  }
  
  //***************************************************************************
  int EventDelegateList::Send(EventData* event)
  {
    // If we have no connections then don't do anything
    if (this->Outgoing.Empty())
      return 0;

    // Store the old event name, in case this event is being forwarded
    String lastEventName = event->EventName;
    event->EventName = this->EventName;

    // Get the type of event we're sending
    BoundType* sentEventType = ZilchVirtualTypeId(event);

    // Because we want to ensure complete event safety, we have to make an entire copy of
    // all event delegates (and the container that holds them all)
    // That way if the user destroy objects, disconnect or reconnect new events, etc we won't
    // have issues referencing the original list / connections
    size_t totalOutgoing = 0;
    ZilchForEach(EventDelegate& delegate, this->Outgoing)
    {
      // Count how many outgoing event delegates we have
      ++totalOutgoing;
    }

    // Allocate on the stack an array of pointers to the delegates
    byte* delegatesList = (byte*)alloca(EventDelegate::MaxEventDelegateSize * totalOutgoing);
    
    // Now copy all the event delegates into our stack local list (before we invoke any user code!)
    size_t i = 0;
    ZilchForEach(EventDelegate& delegate, this->Outgoing)
    {
      // Get the memory for the current delegate
      byte* currentDelegate = delegatesList + EventDelegate::MaxEventDelegateSize * i;
      
      // Copy construct (clone) the current delegate into the memory
      delegate.CopyInto(currentDelegate);
      ++i;
    }

    // How many connections were successfully invoked?
    int successfulInvokes = 0;

    // Now that we've made a copy of all delegates, go through and invoke them, erasing them as we go
    for (size_t j = 0; j < totalOutgoing; ++j)
    {
      // Get the memory for the current delegate
      EventDelegate* delegate = (EventDelegate*)(delegatesList + EventDelegate::MaxEventDelegateSize * j);

      // If the event is not of the correct type, then don't deliver it
      // We allow the delegate to accept a more base version of the event too
      if (ZilchUnsafeEvents || Type::BoundIsA(sentEventType, delegate->Type))
      {
        // Invoke the delegate with the event
        successfulInvokes += delegate->Invoke(event);
      }

      // Because the delegate is a temporary copy, we can destruct it using the virtual destructor
      delegate->~EventDelegate();
    }

    // Restore the last event name
    event->EventName = lastEventName;

    // Return how many connections were successfully invoked
    return successfulInvokes;
  }

  //***************************************************************************
  EventDelegateList* EventHandler::GetOrCreateOutgoingDelegateList(StringParam eventName)
  {
    // Get the current list from the map, or Insert a new entry (will be null)
    EventDelegateList*& list = this->OutgoingPerEventName[eventName];

    // If the list was null (just inserted) then allocate it right now
    // Because the list is grabbed as a reference, this will automatically update the list in the map
    if (list == nullptr)
    {
      // Create the new list and set its name
      list = new EventDelegateList();
      list->EventName = eventName;
    }
    
    // Return the list that we found or created
    return list;
  }
  
  //***************************************************************************
  EventHandler::EventHandler()
  {
  }

  //***************************************************************************
  EventHandler::~EventHandler()
  {
    // Destroy all incoming delegates
    OnlyDeleteObjectIn(this->Incoming);

    // Destroy all outgoing delegate lists (which in turn destroys those delegates)
    DeleteObjectsInContainer(this->OutgoingPerEventName);
  }
  
  //***************************************************************************
  int EventSend(EventHandler* sender, StringParam eventName, EventData* event)
  {
    // We don't allow dispatching of null events
    ReturnIf(event == nullptr, 0, "Cannot send a null event");

    // Get the event delegate list for the given event name and send the event to all of them
    EventDelegateList* delegates = sender->OutgoingPerEventName.FindValue(eventName, nullptr);
    if (delegates != nullptr)
    {
      return delegates->Send(event);
    }

    // Otherwise, no events were invoked
    return 0;
  }

  //***************************************************************************
  void EventSwapAll(EventHandler* a, EventHandler* b)
  {
    a->OutgoingPerEventName.Swap(b->OutgoingPerEventName);
    a->Incoming.Swap(b->Incoming);
  }

  //***************************************************************************
  void EventConnect(EventHandler* sender, StringParam eventName, EventDelegate* delegate, EventHandler* receiver)
  {
    // The receiver doesn't need to do anything special, it just directly stores a reference to the delegate
    receiver->Incoming.PushBack(delegate);

    // Grab the list of outgoing delegates from the sender by this event name
    EventDelegateList* outgoingDelegates = sender->GetOrCreateOutgoingDelegateList(eventName);

    // Link in the delegate to this outgoing connection
    outgoingDelegates->Outgoing.PushBack(delegate);
  }
  
  //***************************************************************************
  int EventDisconnect(EventHandler* sender, EventHandler* receiver, StringParam eventName, void* thisPointerOrUniqueId)
  {
    // Grab the list of outgoing delegates from the sender by this event name
    EventDelegateList* outgoingDelegates = sender->OutgoingPerEventName.FindValue(eventName, nullptr);

    // If nobody signed up for this event, then early out (nothing to disconnect)
    if (outgoingDelegates == nullptr)
      return 0;

    // Loop through all the event delegates and remove anyone that has the same unique id/this pointer
    int removeCount = 0;
    EventDelegate* delegate = &outgoingDelegates->Outgoing.Front();
    while (delegate != outgoingDelegates->Outgoing.End())
    {
      // List is intrusive so the pointer must be moved
      // before erasing the current connection.
      EventDelegate* currentDelegate = delegate;
      delegate = OutgoingList::Next(delegate);

      // If the delegate returns a matching this pointer...
      if (currentDelegate->GetThisPointerOrUniqueId() == thisPointerOrUniqueId)
      {
        // We can directly remove the delegate since we do safe iteration here, and we also
        // perform copying when dispatching
        // Note: Technically due to reference counting, if the delegate is keeping an object alive
        // and then we destroy it, that can cause user code to run, which could then delete the 'Next' delegate
        delete currentDelegate;
        ++removeCount;
      }
    }

    // Return how many connections were deleted
    return removeCount;
  }

  //***************************************************************************
  ForwardingEventDelegate::ForwardingEventDelegate(EventHandler* forwardTo) :
    ForwardTo(forwardTo)
  {
    this->Type = ZilchTypeId(EventData);
  }

  //***************************************************************************
  int ForwardingEventDelegate::Invoke(EventData* event)
  {
    // Directly forward the event to the other handler
    return EventSend(this->ForwardTo, event->EventName, event);
  }

  //***************************************************************************
  void* ForwardingEventDelegate::GetThisPointerOrUniqueId()
  {
    return (void*)this->ForwardTo;
  }

  //***************************************************************************
  void EventForward(EventHandler* sender, StringParam eventName, EventHandler* receiver)
  {
    // Create a member function delegate
    ForwardingEventDelegate* eventDelegate = new ForwardingEventDelegate(receiver);
    
    // Connect the event handler up to this newly created member delegate
    EventConnect(sender, eventName, eventDelegate, receiver);
  }
  
  //***************************************************************************
  ZilchEventDelegate::ZilchEventDelegate(const Delegate& delegate, ExecutableState* state) :
    FunctionWithThis(delegate),
    State(state)
  {
    // We assume that the passed in delegate should be validated by this point, so this should not crash or be invalid
    this->Type = (BoundType*)delegate.BoundFunction->FunctionType->Parameters[0].ParameterType;
  }

  //***************************************************************************
  int ZilchEventDelegate::Invoke(EventData* event)
  {
    // If we have no executable state, then the event handler was most likely invalidated
    if (this->State == nullptr)
      return 0;

    // If we the this handle is null, then we also skip this event handler
    // In general this should never happen (because this event handler would be removed)
    // However it is possible if we copied all the event handlers that we were going to call
    // and then the object was destroyed, then we would still traverse this
    if (this->FunctionWithThis.ThisHandle.IsNull())
      return 0;

    // Now we want to call the event handler
    // At the moment, we catch all exceptions from event handlers
    ExceptionReport report;
    Call call(this->FunctionWithThis, this->State);

    // For now we have a special toggle that allows the users to receive events unsafely (as a stop gap...)
    if (ZilchUnsafeEvents)
      call.DisableParameterChecks();

    // Set the event on the call and invoke it!
    call.Set(0, event);
    call.Invoke(report);

    // If the function failed to completely run... then return that this was not a successful invoke
    if (report.HasThrownExceptions())
      return 0;

    // Otherwise, we successfully invoked a Zilch callback!
    return 1;
  }

  //***************************************************************************
  void* ZilchEventDelegate::GetThisPointerOrUniqueId()
  {
    return (void*)this->FunctionWithThis.ThisHandle.Dereference();
  }
  
  //***************************************************************************
  int EventsClass::Send(const Handle& sender, StringParam eventName, EventData* event)
  {
    // Get the state that called the function (this is thread local and therefore safe)
    ExecutableState* state = ExecutableState::CallingState;
    ExceptionReport& report = state->GetCallingReport();

    // Make sure the event being sent is not null
    if (event == nullptr)
    {
      state->ThrowException(report, "The event being sent must not be null");
      return 0;
    }

    // Make sure the sender is not null
    byte* senderMemory = sender.Dereference();
    if (senderMemory == nullptr)
    {
      state->ThrowException(report, "The sender must not be null");
      return 0;
    }

    // Attempt to get an event handler from the memory of the sender (calls into user code for user provided types)
    EventHandler* senderHandler = sender.StoredType->GetEventHandler(senderMemory);
    if (senderHandler == nullptr)
    {
      state->ThrowException(report, "The sender was not a valid event handler and cannot send or receive events");
      return 0;
    }

    // Send the event out and return the number of successful invokes
    return EventSend(senderHandler, eventName, event);
  }
  
  //***************************************************************************
  void EventsClass::Connect(const Handle& sender, StringParam eventName, const Delegate& callback)
  {
    // Get the state that called the function (this is thread local and therefore safe)
    ExecutableState* state = ExecutableState::CallingState;
    ExceptionReport& report = state->GetCallingReport();

    // If the function is null, then throw an exception
    if (callback.BoundFunction == nullptr)
    {
      state->ThrowException(report, "The callback must not be null");
      return;
    }
    
    // Make sure the delegate's return type is void
    DelegateType* signature = callback.BoundFunction->FunctionType;
    if (Type::IsSame(signature->Return, ZilchTypeId(void)) == false)
    {
      state->ThrowException(report, "The callback must not return a value (the return type must be Void)");
      return;
    }
    
    // Make sure the delegate only takes one argument
    ParameterArray& parameters = signature->Parameters;
    if (parameters.Size() != 1)
    {
      state->ThrowException(report, "The callback must take only one parameter, the EventData");
      return;
    }
    
    // Make sure the delegate's parameter derives from EventData (it can also be just EventData itself)
    if (Type::GenericIsA(parameters[0].ParameterType, ZilchTypeId(EventData)) == false)
    {
      state->ThrowException(report, "The callback's parameter must be an EventData or inherit from it");
      return;
    }
    
    // If the this handle is null and its not a static function, then throw an exception
    const Handle& receiver = callback.ThisHandle;
    byte* receiverMemory = receiver.Dereference();
    if (receiverMemory == nullptr && callback.BoundFunction->This != nullptr)
    {
      state->ThrowException(report, "The callback's object (the receiver) was null, and the function was not a static function");
      return;
    }

    // Attempt to get an event handler from the memory of the receiver (calls into user code for user provided types)
    EventHandler* receiverHandler = receiver.StoredType->GetEventHandler(receiverMemory);
    if (receiverHandler == nullptr)
    {
      state->ThrowException(report, "The receiver was not a valid event handler and cannot send or receive events");
      return;
    }

    // Make sure the sender is not null
    byte* senderMemory = sender.Dereference();
    if (senderMemory == nullptr)
    {
      state->ThrowException(report, "The sender must not be null");
      return;
    }

    // Attempt to get an event handler from the memory of the sender (calls into user code for user provided types)
    EventHandler* senderHandler = sender.StoredType->GetEventHandler(senderMemory);
    if (senderHandler == nullptr)
    {
      state->ThrowException(report, "The sender was not a valid event handler and cannot send or receive events");
      return;
    }

    // Create a the event connection that will invoke the Zilch delegate
    ZilchEventDelegate* eventDelegate = new ZilchEventDelegate(callback, state);
    
    // Connect the event handler up to this newly created member delegate
    EventConnect(senderHandler, eventName, eventDelegate, receiverHandler);
  }
}
