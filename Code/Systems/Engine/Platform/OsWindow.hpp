// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Window Events
namespace Events
{
DeclareEvent(OsMouseUp);
DeclareEvent(OsMouseDown);
DeclareEvent(OsMouseMove);
DeclareEvent(OsMouseScroll);
DeclareEvent(OsKeyDown);
DeclareEvent(OsKeyUp);
DeclareEvent(OsKeyRepeated);
DeclareEvent(OsKeyTyped);
DeclareEvent(OsDeviceChanged);
DeclareEvent(OsResized);
DeclareEvent(OsMoved);
DeclareEvent(OsClose);
DeclareEvent(OsFocusGained);
DeclareEvent(OsFocusLost);
DeclareEvent(OsPaint);
DeclareEvent(OsMouseFileDrop);

// Checking if we're over one of the grip borders of a window.
DeclareEvent(OsWindowBorderHitTest);

} // namespace Events

extern const String cOsKeyboardEventsFromState[3];

class KeyboardEvent;
class KeyboardTextEvent;
class OsMouseEvent;
class OsMouseDropEvent;
class OsWindowEvent;
class OsShell;

/// Represents an window in the operating system.
class OsWindow : public ThreadSafeId32EventObject
{
public:
  ZilchDeclareType(OsWindow, TypeCopyMode::ReferenceType);

  OsWindow();
  virtual ~OsWindow();

  /// The current client size of the window
  IntVec2 GetClientSize();
  /// Does this window have focus?
  bool HasFocus();

  /// Capturing the mouse prevents messages from being sent to other windows and
  /// getting mouse movements outside the window (e.g. dragging).
  void SetMouseCapture(bool enabled);

  /// Locks the mouse to prevent it from moving.
  bool GetMouseTrap();
  void SetMouseTrap(bool trapped);

  void SendKeyboardEvent(KeyboardEvent& event);
  void SendKeyboardTextEvent(KeyboardTextEvent& event);
  void SendMouseEvent(OsMouseEvent& event);
  void SendMouseDropEvent(OsMouseDropEvent& event);
  void SendWindowEvent(OsWindowEvent& event);

  // Internal

  void FillKeyboardEvent(Keys::Enum key, KeyState::Enum keyState, KeyboardEvent& keyEvent);
  void FillMouseEvent(IntVec2Param clientPosition, MouseButtons::Enum mouseButton, OsMouseEvent& mouseEvent);

  static void ShellWOnFocusChanged(bool activated);
  static void ShellWOnClientSizeChanged(Math::IntVec2Param clientSize);
  static void ShellWOnDevicesChanged();
  static WindowBorderArea::Enum ShellWOnHitTest(Math::IntVec2Param clientPosition);
  static void ShellWOnInputDeviceChanged(
      PlatformInputDevice& device, uint buttons, const Array<uint>& axes, const DataBlock& data);

  static OsWindow* sInstance;
};

// General windows events.
class OsWindowEvent : public Event
{
public:
  ZilchDeclareType(OsWindowEvent, TypeCopyMode::ReferenceType);
  OsWindowEvent();

  void Serialize(Serializer& stream);

  IntVec2 ClientSize;
};

/// Mouse Events on the main window.
class OsMouseEvent : public Event
{
public:
  ZilchDeclareType(OsMouseEvent, TypeCopyMode::ReferenceType);

  OsMouseEvent();
  void Clear();

  bool ShiftPressed;
  bool AltPressed;
  bool CtrlPressed;
  IntVec2 ClientPosition;
  Vec2 ScrollMovement;

  // Button For this Event
  MouseButtons::Enum MouseButton;
  byte ButtonDown[MouseButtons::Size];

  void Serialize(Serializer& stream);
};

/// An even that we send when we want to check if we're over a window border
/// gripper such as the title bar for dragging or an edge/corner for resizing.
class OsWindowBorderHitTest : public Event
{
public:
  ZilchDeclareType(OsWindowBorderHitTest, TypeCopyMode::ReferenceType);

  OsWindowBorderHitTest();

  IntVec2 ClientPosition;

  // This should be set by the receiver of the event.
  WindowBorderArea::Enum mWindowBorderArea;
};

/// Files have been dropped on a OsShellWindow
class OsMouseDropEvent : public OsMouseEvent
{
public:
  ZilchDeclareType(OsMouseDropEvent, TypeCopyMode::ReferenceType);
  OsMouseDropEvent()
  {
  }

  void Serialize(Serializer& stream);

  Array<String> Files;
};

} // namespace Zero
