// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Editor;

// Raverie Static
/// Global functionality exposed to Raverie script. Bound as "Raverie" to script
/// (e.g. Raverie.Keyboard) RaverieStatic was used to avoid the conflict with
/// namespace Raverie).
class RaverieStatic
{
public:
  RaverieDeclareType(RaverieStatic, TypeCopyMode::ReferenceType);

  /// Connection invokes the given delegate when sender dispatches the specified
  /// event.
  static void Connect(Object* sender, StringParam eventId, DelegateParam receiverDelegate);
  /// Removes specified event connection,
  /// if connection delegate was a component method then receiver object is just
  /// the component.
  static void Disconnect(Object* sender, StringParam eventId, Object* receiver);
  /// Removes all event connections between sender and receiver,
  /// if connection delegate was a component method then receiver object is just
  /// the component.
  static void DisconnectAllEvents(Object* sender, Object* receiver);

  static Keyboard* GetKeyboard();
  static Mouse* GetMouse();
  static Editor* GetEditor();
  static Engine* GetEngine();
  static Environment* GetEnvironment();
  static Gamepads* GetGamepads();
  static ObjectStore* GetObjectStore();
  static ResourceSystem* GetResourceSystem();
  static OsShell* GetOsShell();
  static SoundSystem* GetAudio();
};

// RaverieScriptConnection
/// RaverieScriptConnection enables raverie to connect to any event in the engine.
class RaverieScriptConnection : public EventConnection
{
public:
  RaverieScriptConnection(EventDispatcher* dispatcher, StringParam eventId, DelegateParam delagate);
  ~RaverieScriptConnection();

  void RaiseError(StringParam message) override;
  void Invoke(Event* event) override;
  DataBlock GetFunctionPointer() override;

  size_t mStatePatchId;
  Delegate mDelegate;
};

} // namespace Raverie
