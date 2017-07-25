///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
DeclareEvent(OsDeviceAdded);
DeclareEvent(OsResized);
DeclareEvent(OsMoved);
DeclareEvent(OsClose);
DeclareEvent(OsFocusGained);
DeclareEvent(OsFocusLost);
DeclareEvent(OsPaint);
DeclareEvent(OsMouseFileDrop);
}//namespace Events

extern const String cOsKeyboardEventsFromState[3];

/// Flags used to control the behavior of an OsWindow
DeclareBitField7(WindowStyleFlags,
  // Is the window visible. This is 'NotVisible' instead of visible so that the
  // default is visible.
  NotVisible,
  // Main window
  MainWindow,
  // Does the window appear on the task bar
  OnTaskBar, 
  // Does the window have a title bar area
  TitleBar,
  // Does this window have resizable borders
  Resizable, 
  // Does this window have a close button
  Close,
  // Window has client area only
  ClientOnly);

/// The state of the window for minimizing / maximizing
DeclareEnum5(WindowState,
  // Shrink the window to the taskbar
  Minimized, 
  // Expand the window to fill the current desktop
  Maximized,
  // Arbitrarily sized window
  Windowed,
  // Window covers everything including the taskbar and is minimized without focus
  Fullscreen,
  // Restore window to previous state before being minimizing
  Restore);

/// Standard Mouse Cursors
DeclareEnum11(Cursor,
  Arrow,
  Wait,
  Cross,
  SizeNWSE,
  SizeNESW,
  SizeWE,
  SizeNS,
  SizeAll,
  TextBeam,
  Hand,
  Invisible);

/// Border of the window for manipulation
DeclareEnum9(WindowBorderArea, 
  Title, 
  TopLeft, Top, TopRight,
  Left, Right, 
  BottomLeft, Bottom, BottomRight);

/// As the extra mouse buttons are typically Back and Forward they have been named accordingly.
DeclareEnum6(MouseButtons, Left, Right, Middle, XOneBack, XTwoForward, None);

//--------------------------------------------------------------------- OsWindow
/// Represents an window in the operating system.
class OsWindow : public ThreadSafeId32EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  OsWindow() {};
  virtual ~OsWindow() {};

  /// Position of the window in screen coordinates
  virtual IntVec2 GetPosition() = 0;
  virtual void SetPosition(IntVec2Param newScreenPosition) = 0;

  /// The size of the window including the border
  virtual IntVec2 GetSize() = 0;
  virtual void SetSize(IntVec2Param newSize) = 0;

  // Set the minimum size of the window
  virtual void SetMinSize(IntVec2Param minSize) = 0;

  /// The current client size of the window
  virtual IntVec2 GetClientSize() = 0;
  virtual void SetClientSize(IntVec2Param newClientSize) = 0;

  /// Parent window of this window. Null for Desktop windows.
  virtual OsWindow* GetParent() = 0;

  /// Convert screen coordinates to client coordinates
  virtual IntVec2 ScreenToClient(IntVec2Param screenPoint) = 0;

  /// Convert client coordinates to screen coordinates
  virtual IntVec2 ClientToScreen(IntVec2Param clientPoint) = 0;

  /// Style flags control border style, title bar, and other features.
  virtual WindowStyleFlags::Enum GetStyle() = 0;
  virtual void SetStyle(WindowStyleFlags::Enum style) = 0;

  /// Is the window visible on the desktop?
  virtual bool GetVisible() = 0;
  virtual void SetVisible(bool visible) = 0;

  /// Set the title of window displayed in the title bar.
  virtual void SetTitle(StringParam title) = 0;

  /// State of the Window, Set state to Minimize, Maximize, or Restore the window
  virtual WindowState::Enum GetState() = 0;
  virtual void SetState(WindowState::Enum windowState) = 0;

  /// Try to take Focus and bring the window to the foreground.
  /// The OS may prevent this from working if this app is not the foreground app.
  virtual void TakeFocus() = 0;
  /// Does this window have focus?
  virtual bool HasFocus() = 0;

  /// Try to close the window. The OsClose event is sent and can be canceled
  virtual void Close() = 0;

  /// Destroy the window
  virtual void Destroy() = 0;

  /// Resize or move the window using the default OS method
  virtual void ManipulateWindow(WindowBorderArea::Enum borderArea) = 0;

  /// Capturing the mouse prevents messages from being sent to other windows and
  /// getting mouse movements outside the window (e.g. dragging).
  virtual void SetMouseCapture(bool enabled) = 0;

  /// Locks the mouse to prevent it from moving.
  virtual bool GetMouseTrap() = 0;
  virtual void SetMouseTrap(bool trapped) = 0;

  /// Set the cursor for the mouse.
  virtual void SetMouseCursor(Cursor::Enum cursorId) = 0;

  virtual OsHandle GetWindowHandle() = 0;

  // Set application progress
  virtual void SendProgress(ProgressType::Enum progressType, float progress) {}
};

//-------------------------------------------------------------------OsWindowEvent
//General windows events.
class OsWindowEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  OsWindowEvent() {}

  OsWindow* Window;
  IntVec2 ClientSize;
  IntVec2 WindowSize;
};

//-------------------------------------------------------------------OsMouseDropEvent
/// Mouse Events on the main window.
class OsMouseEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  OsMouseEvent();
  void Clear();

  OsWindow* Window;
  bool ShiftPressed;
  bool AltPressed;
  bool CtrlPressed;
  IntVec2 ClientPosition;
  Vec2 ScrollMovement;
  bool IsTrapMoveBack;

  //Button For this Event
  MouseButtons::Enum MouseButton;
  byte ButtonDown[MouseButtons::Size];

  void Serialize(Serializer& stream);
};

//-------------------------------------------------------------------OsMouseDropEvent
/// Files have been dropped on a OsShellWindow
class OsMouseDropEvent : public OsMouseEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  OsMouseDropEvent() {}
  Array<String> Files;
};

}//namespace Zero
