///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
// #include "String/String.hpp"
#include "EnumDeclaration.hpp"
// #include "String/StringBuilder.hpp"

namespace Zero
{
DeclareEnum2(StatusState, Success, Failure);

template <typename T>
class StatusContext
{
public:
  static void ZilchDoNotBind();

  typedef void(*StatusFn)(StringParam message, const T& context, void* userData);

  StatusContext() :
    State(StatusState::Success),
    Context(T()),
    IgnoreMessage(false),
    CallbackOnFailure(nullptr),
    UserDataOnFailure(nullptr)
  {
  }

  StatusContext(StatusState::Enum state, StringParam message) :
    State(state),
    Message(message),
    IgnoreMessage(false),
    CallbackOnFailure(nullptr),
    UserDataOnFailure(nullptr)
  {
  }

  bool Failed()
  {
    return State == StatusState::Failure;
  }

  bool Succeeded()
  {
    return State == StatusState::Success;
  }

  void AssertOnFailure()
  {
    CallbackOnFailure = AssertCallback;
  }

  // Lets the status know that it failed
  // Returns true if this is the first time it failed
  bool SetFailed(StringParam message)
  {
    // Don't fail more than once
    if (State == StatusState::Failure)
      return false;

    State = StatusState::Failure;
    Message = message;

    if (CallbackOnFailure)
      CallbackOnFailure(message, Context, this->UserDataOnFailure);
    return true;
  }

  // Lets the status know that it failed
  // Returns true if this is the first time it failed
  bool SetFailed(StringParam message, const T& context)
  {
    // Don't fail more than once
    if (State == StatusState::Failure)
      return false;

    Context = context;
    return this->SetFailed(message);
  }

  // Lets the status know that it succeeded
  void SetSucceeded(StringParam message = String())
  {
    if (State == StatusState::Success)
      return;
    State = StatusState::Success;
    Message = message;
  }

  // Lets the status know that it succeeded
  void SetSucceeded(StringParam message, const T& context)
  {
    // Don't fail more than once
    if (State == StatusState::Failure)
      return false;

    Context = context;
    State = StatusState::Success;
    Message = message;
  }

  operator bool()
  {
    return Succeeded();
  }

  static void AssertCallback(StringParam message, const T& context, void* userData)
  {
    Error("%s", message.c_str());
  }

  void Reset()
  {
    State = StatusState::Success;
    Context = T();
  }

  // Whether or not the status failed
  StatusState::Enum State;
  // A custom error message (generally only set when the State is Failure)
  String Message;
  // Some functions want to return more detail on why they failed or information about the failure
  // This is usually an enumeration, operating specific error code, etc
  T Context;
  // As an optimization, we can suggest that we don't care about the message string
  // This is only a suggestion and may not be used (but should be heeded in high performance areas)
  bool IgnoreMessage;
  // A function pointer that is automatically invoked when the status fails
  // This is useful for triggering asserts or global callbacks (must use SetFailed, AppendFailed, or PrependFailed)
  StatusFn CallbackOnFailure;
  void* UserDataOnFailure;
};

typedef StatusContext<u32> Status;

#define StatusReturnIfFailed(Status, ...) ReturnIf((Status).Failed(), __VA_ARGS__, "%s", (Status).Message.c_str())

}
