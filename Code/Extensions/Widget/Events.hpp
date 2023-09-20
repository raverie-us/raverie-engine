// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(MouseEnterPreview);
DeclareEvent(KeyPreview);
DeclareEvent(HoverKeyPreview);
DeclareEvent(HoverKeyDown);
DeclareEvent(HoverKeyUp);
DeclareEvent(HoverKeyRepeated);

// Update when mouse is over the object
DeclareEvent(MouseUpdate);

// Mouse dropped files on a viewport
DeclareEvent(MouseFileDrop);

// Mouse move from over an object to any other object including a child
DeclareEvent(MouseEnter);
DeclareEvent(MouseExit);

// Mouse has moved out from the object and all children of object
DeclareEvent(MouseEnterHierarchy);
DeclareEvent(MouseExitHierarchy);

// Mouse Down or Up on object or child
DeclareEvent(MouseDown);
DeclareEvent(MouseUp);

// Named Button Events
DeclareEvent(LeftMouseDown);
DeclareEvent(LeftMouseUp);
DeclareEvent(RightMouseDown);
DeclareEvent(RightMouseUp);
DeclareEvent(MiddleMouseDown);
DeclareEvent(MiddleMouseUp);

// Mouse moved on object or child
DeclareEvent(MouseMove);

// Mouse scroll
DeclareEvent(MouseScroll);

// Mouse is down and move for the drag distance
DeclareEvent(LeftMouseDrag);
DeclareEvent(RightMouseDrag);

// Mouse is down for the hold time
DeclareEvent(MouseHold);

// Mouse is over for the hover time
DeclareEvent(MouseHover);

// Mouse button was pressed down and up on a object
DeclareEvent(LeftClick);
DeclareEvent(RightClick);
DeclareEvent(MiddleClick);

// Mouse Double clicked
DeclareEvent(DoubleClick);

DeclareEvent(MouseDrop);

// Focus on Object
DeclareEvent(FocusLost);
DeclareEvent(FocusGained);

DeclareEvent(FocusLostHierarchy);
DeclareEvent(FocusGainedHierarchy);

// Any focus operation should be canceled
DeclareEvent(FocusReset);

DeclareEvent(WidgetUpdate);

DeclareEvent(Activated);
DeclareEvent(Deactivated);

DeclareEvent(WidgetShown);

DeclareEvent(OnDestroy);

// Test if this widget should be closed if this event is not handled
// the window/widget will close
DeclareEvent(Closing);
} // namespace Events

const uint NamedButtonEvents = 3;
extern String NamedMouseDown[3];
extern String NamedMouseUp[3];
extern String NamedMouseClick[3];

/// Basic event that can be "handled" to override default engine behavior.
class HandleableEvent : public Event
{
public:
  RaverieDeclareType(HandleableEvent, TypeCopyMode::ReferenceType);
  HandleableEvent() : Handled(false){};
  /// Set to true to signify that you have responded to this event, and that
  /// other event responders should do nothing.
  bool Handled;
};

class Widget;

class FocusEvent : public Event
{
public:
  RaverieDeclareType(FocusEvent, TypeCopyMode::ReferenceType);

  FocusEvent(Widget* focusGained, Widget* focusLost) : ReceivedFocus(focusGained), LostFocus(focusLost)
  {
  }

  Widget* ReceivedFocus;
  Widget* LostFocus;
};

class DispatchAtParams
{
public:
  DispatchAtParams() : ObjectHit(false), Ignore(NULL), BubbleEvent(true)
  {
  }
  bool ObjectHit;
  Vec2 Position;
  String EventId;
  Event* EventObject;
  Widget* Ignore;
  bool BubbleEvent;
};

/// Mouse events for actions concerning the mouse.
class MouseEvent : public Event
{
public:
  RaverieDeclareType(MouseEvent, TypeCopyMode::ReferenceType);

  MouseEvent();

  /// If this is a MouseDown event which button was changed
  MouseButtons::Enum Button;
  bool ButtonDown;

  /// State of all the mouse buttons
  byte mButtonDown[MouseButtons::Size];
  /// Position of the Mouse.
  Vec2 Position;
  /// Movement since last mouse move.
  Vec2 Movement;
  /// Movement of scroll wheel.
  Vec2 Scroll;
  /// Is Shift held down on the keyboard?
  bool ShiftPressed;
  /// Is Alt held down on the keyboard?
  bool AltPressed;
  /// Is Ctrl held down on the keyboard?
  bool CtrlPressed;
  /// Prevent more than one control from processing the event
  bool Handled;

  // Not for use in native code:
  /// Whether this message was handled by script
  bool HandledEventScript;

  bool IsButtonDown(MouseButtons::Enum button);
  bool IsButtonUp(MouseButtons::Enum button)
  {
    return !IsButtonDown(button);
  }

  Mouse* GetMouse()
  {
    return EventMouse;
  }

  /// The mouse object
  Mouse* EventMouse;
  Widget* Source;
  Event* OsEvent;
};

class MouseDragEvent : public MouseEvent
{
public:
  RaverieDeclareType(MouseDragEvent, TypeCopyMode::ReferenceType);
  Vec2 StartPosition;
};

/// Files have been dropped on a viewport.
class MouseFileDropEvent : public MouseEvent
{
public:
  RaverieDeclareType(MouseFileDropEvent, TypeCopyMode::ReferenceType);

  MouseFileDropEvent();
  MouseFileDropEvent(const MouseEvent& rhs);

  void Copy(const OsMouseDropEvent& rhs);

public:
  HandleOf<ArrayString> Files;
};

} // namespace Raverie
