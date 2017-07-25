////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------------- Zero Static
//**************************************************************************************************
ZilchDefineType(ZeroStatic, builder, type)
{
  ZilchBindMethod(Connect);
  ZilchBindMethod(Disconnect);
  ZilchBindMethodAs(DisconnectAllEvents, "DisconnectAll");

  ZilchBindGetter(Keyboard);
  ZilchBindGetter(Mouse);
  ZilchBindGetter(Editor);
  ZilchBindGetter(Engine);
  ZilchBindGetter(Environment);
  ZilchBindGetter(Gamepads);
  ZilchBindGetter(Joysticks);
  ZilchBindGetter(ObjectStore);
  ZilchBindGetter(ResourceSystem);
  ZilchBindGetter(OsShell);
}

//**************************************************************************************************
void ErrorOnConnect(Function* func)
{
  DoNotifyException("Zilch Error", String::Format(
    "Zero.Connect must take a delegate that has only one "
    "parameter (a reference / class Event type). Function %s has the signature: %s",
    func->Name.c_str(),
    func->FunctionType->ToString().c_str()));
}

//**************************************************************************************************
void ZeroStatic::Connect(Object* target, StringParam eventId, DelegateParam delegate)
{
  if(target == nullptr)
  {
    DoNotifyException("The target object is null", "Cannot connect to a null object");
    return;
  }

  if(delegate.IsNull())
  {
    DoNotifyException("The delegate is null", "Cannot connect a null delegate");
    return;
  }

  Function* callbackFunction = delegate.BoundFunction;
  const ParameterArray& params = callbackFunction->FunctionType->Parameters;
  
  // The function should only take an event
  if(params.Size() != 1)
  {
    ErrorOnConnect(callbackFunction);
    return;
  }

  // Confirm that the parameter type is an event
  Type* type = params[0].ParameterType;
  BoundType* eventType = Type::DynamicCast<BoundType*>(type);
  if(eventType == nullptr || !eventType->IsA(ZilchTypeId(Event)))
  {
    ErrorOnConnect(callbackFunction);
    return;
  }

  // Get the event support object (Dispatcher/Receiver)
  EventDispatcher* dispatcher = target->GetDispatcherObject();
  Object* receiverObject = delegate.ThisHandle.Get<Object*>();
  EventReceiver* receiver = receiverObject->GetReceiverObject();

  // Create the connection
  ZilchScriptConnection* connection = new ZilchScriptConnection(delegate);
  connection->EventType = eventType;

  // Connect
  connection->ConnectToReceiverAndDispatcher(eventId, receiver, dispatcher);
}

//**************************************************************************************************
void ZeroStatic::Disconnect(Object* sender, StringParam eventId, Object* receiver)
{
  if(sender == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Sender object is null");
    return;
  }

  if(receiver == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Receiver object is null");
    return;
  }

  EventDispatcher* dispatcher = sender->GetDispatcherObject();
  dispatcher->DisconnectEvent(eventId, receiver);
}

//**************************************************************************************************
void ZeroStatic::DisconnectAllEvents(Object* sender, Object* receiver)
{
  if(sender == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Sender object is null");
    return;
  }

  if(receiver == nullptr)
  {
    DoNotifyException("Cannot disconnect", "Receiver object is null");
    return;
  }

  EventDispatcher* dispatcher = sender->GetDispatcherObject();
  dispatcher->Disconnect(receiver);
}

//**************************************************************************************************

//**************************************************************************************************
Keyboard* ZeroStatic::GetKeyboard()
{
  return Keyboard::Instance;
}

//**************************************************************************************************
Mouse* ZeroStatic::GetMouse()
{
  return Z::gMouse;
}

//**************************************************************************************************
Editor* ZeroStatic::GetEditor()
{
  return Z::gEditor;
}

//**************************************************************************************************
Engine* ZeroStatic::GetEngine()
{
  return Z::gEngine;
}

//**************************************************************************************************
Environment* ZeroStatic::GetEnvironment()
{
  return Environment::GetInstance();
}

//**************************************************************************************************
Gamepads* ZeroStatic::GetGamepads()
{
  return Z::gGamepads;
}

//**************************************************************************************************
Zero::Joysticks* ZeroStatic::GetJoysticks()
{
  return Z::gJoysticks;
}

//**************************************************************************************************
ObjectStore* ZeroStatic::GetObjectStore()
{
  return ObjectStore::GetInstance();
}

//**************************************************************************************************
ResourceSystem* ZeroStatic::GetResourceSystem()
{
  return Z::gResources;
}

//**************************************************************************************************
OsShell* ZeroStatic::GetOsShell()
{
  return GetEngine()->has(OsShell);
}

//---------------------------------------------------------------------------- ZilchScriptConnection
//**************************************************************************************************
ZilchScriptConnection::ZilchScriptConnection(DelegateParam delagate)
{
  Flags.SetFlag(ConnectionFlags::Script);
  mDelegate = delagate;
  mStatePatchId = (size_t)-1;
  ThisObject = mDelegate.ThisHandle.Dereference();
}

//**************************************************************************************************
void ZilchScriptConnection::RaiseError(StringParam message)
{
  Function* function = mDelegate.BoundFunction;
  if(function == nullptr)
  {
    DoNotifyException("Null Event Connection", message);
    return;
  }

  // Get the location of the first opcode
  const CodeLocation* location = function->OpcodeLocationToCodeLocation.FindPointer(0);
  if(location)
  {
    String namedMessage = BuildString(function->Name, ": ", message);
    String fullMessage = location->GetFormattedStringWithMessage(MessageFormat::Python, namedMessage);
    ZilchScriptManager::GetInstance()->DispatchScriptError(Events::UnhandledException, namedMessage, fullMessage, *location);
    return;
  }

  DoNotifyException("Event Connection", String::Format("%s\nFunction: ", function->ToString().c_str()));
}

//**************************************************************************************************
void ZilchScriptConnection::Invoke(Event* e)
{
  // If the patch id matches the one on the state, then it means we disabled this event handler
  // Once the state gets patched, the event handler will auto resume (it may throw again, and that will re-disable it)
  if(mStatePatchId == ExecutableState::CallingState->PatchId)
    return;

  ExceptionReport report;
  Call call(mDelegate);

  // We have to disable debug checks because technically we are doing something unsafe here
  // The Zilch binding will yell at us for trying to pass in an 'Event' where it takes a more derived type
  call.DisableParameterChecks();

  call.SetHandle(0, e);
  call.Invoke(report);

  // If an exception has occurred kill this connection
  // to prevent callbacks like 'KeyDown' from constantly throwing exceptions
  if(report.HasThrownExceptions())
    mStatePatchId = ExecutableState::CallingState->PatchId;
}

}//namespace Zero
