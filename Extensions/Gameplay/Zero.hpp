////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Editor;

//-------------------------------------------------------------------------------------- Zero Static
/// Global functionality exposed to Zilch script. Bound as "Zero" to script (e.g. Zero.Keyboard)
/// ZeroStatic was used to avoid the conflict with namespace Zero).
class ZeroStatic
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Connection invokes the given delegate when sender dispatches the specified event.
  static void Connect(Object* sender, StringParam eventId, DelegateParam receiverDelegate);
  /// Removes specified event connection, 
  /// if connection delegate was a component method then receiver object is just the component.
  static void Disconnect(Object* sender, StringParam eventId, Object* receiver);
  /// Removes all event connections between sender and receiver, 
  /// if connection delegate was a component method then receiver object is just the component.
  static void DisconnectAllEvents(Object* sender, Object* receiver);
  
  static Keyboard* GetKeyboard();
  static Mouse* GetMouse();
  static Editor* GetEditor();
  static Engine* GetEngine();
  static Environment* GetEnvironment();
  static Gamepads* GetGamepads();
  static Joysticks* GetJoysticks();
  static ObjectStore* GetObjectStore();
  static ResourceSystem* GetResourceSystem();
  static OsShell* GetOsShell();
  static SoundSystem* GetAudio();

  //static Joysticks* GetJoysticks();
  //static MultiTouch* GetMultiTouch();
};

//---------------------------------------------------------------------------- ZilchScriptConnection
/// ZilchScriptConnection enables zilch to connect to any event in the engine.
class ZilchScriptConnection : public EventConnection
{
public:
  ZilchScriptConnection(DelegateParam delagate);

  void RaiseError(StringParam message) override;
  void Invoke(Event* event) override;

  size_t mStatePatchId;
  Delegate mDelegate;
};

}//namespace Zero
