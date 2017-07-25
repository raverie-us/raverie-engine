///////////////////////////////////////////////////////////////////////////////
///
/// \file Interactive.hpp
/// Definition of the interactive display object class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
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
}//namespace Events


const uint NamedButtonEvents = 3;
extern String NamedMouseDown[3];
extern String NamedMouseUp[3];
extern String NamedMouseClick[3];

class HandleableEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  HandleableEvent()
    :Handled(false){};
  bool Handled;
};

class Widget;

//------------------------------------------------------------------ Focus Event
class FocusEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  FocusEvent(Widget* focusGained, Widget* focusLost)
    : ReceivedFocus(focusGained), LostFocus(focusLost)
  {
  }

  Widget* ReceivedFocus;
  Widget* LostFocus;
};

//----------------------------------------------------------- Dispatch At Params
class DispatchAtParams
{
public:
  DispatchAtParams() : ObjectHit(false) , Ignore(NULL), BubbleEvent(true) {}
  bool ObjectHit;
  Vec2 Position;
  String EventId;
  Event* EventObject;
  Widget* Ignore;
  bool BubbleEvent;
};


//------------------------------------------------------------------ Mouse Event
/// Mouse events for any every with the mouse.
class MouseEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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
  ///Whether this message was handled by script
  bool HandledEventScript;

  bool IsButtonDown(MouseButtons::Enum button);
  bool IsButtonUp(MouseButtons::Enum button) { return !IsButtonDown(button); }

  Mouse* GetMouse(){return EventMouse;}

  /// The mouse object
  Mouse* EventMouse;
  Widget* Source;
  Event* OsEvent;
};

class MouseDragEvent : public MouseEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  Vec2 StartPosition;
};

}//namespace Zero
