///////////////////////////////////////////////////////////////////////////////
///
/// \file ErrorContext.hpp
///
/// Authors: Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
namespace Zero
{

// Loading operations are complex with many levels of indirection. Many times 
// errors will occur at a low level which means it is hard to get an idea of
// which levels, objects, or resources contain them. For example, the engine 
// will use default resources or values to prevent crashing while loading 
// the level or resource. With the error context these errors and the owning 
// objects can be tracked down and fixed by a user.

// Error context object interface. Derived classes of this object
// are stored on the stack and a pointer to them is pushed on 
// the ActiveErrorContextStack. This makes the error context efficient
// until the stack needs to be evaluated.
class ErrorContext
{
public:
  /// Constructor/Destructor will push and pop to ActiveErrorContextStack
  ErrorContext();
  virtual ~ErrorContext();
  /// Overridden by derived classes to get a description of
  /// this error context
  virtual String GetDescription()=0;
};

// Using a simple array for the error stack
typedef Array<ErrorContext*> ErrorContextStack;
// Thread local context
ZeroThreadLocal extern ErrorContextStack* ActiveErrorContextStack;
// Do a Notify and print the error context stack
void DoNotifyErrorWithContext(StringParam message, NotifyException::Enum notifyException = NotifyException::Script);
// Cleanup Error Context
void CleanUpErrorContext();

/// Error Context object with simple cstr string
class ErrorContextMessage : public ErrorContext
{
public:
  // Message to be displayed.
  cstr Message;
  ErrorContextMessage(cstr message)
    :Message(message)
  {}
  String GetDescription() override{return Message;}
};

// Push a cstr message for the ErrorContext.
// Do not free the string memory until the stack returns.
#define PushErrorContext(message) ErrorContextMessage __entry(message);

/// Error context object that stores an object to be displayed
/// with a message.
class ErrorContextObject : public ErrorContext
{
public:
  // Message to be displayed.
  cstr Message;
  // Object relevant to this context
  Object* ContextObject;

  //ErrorContext Interface
  String GetDescription() override;

  ErrorContextObject(cstr message, Object* object)
    :Message(message), ContextObject(object)
  {}
};

// Push a cstr message and Object pointer.
// Do not free object or string memory.
// Standard usage is PushErrorContextObject("Loading Level", level)
#define PushErrorContextObject(message, contextObject) ErrorContextObject __entry(message, contextObject);

}
