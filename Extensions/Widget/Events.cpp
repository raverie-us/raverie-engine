///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(MouseEnterPreview);
  DefineEvent(KeyPreview);
  DefineEvent(HoverKeyPreview);
  DefineEvent(HoverKeyDown);
  DefineEvent(HoverKeyUp);
  DefineEvent(HoverKeyRepeated);
  DefineEvent(MouseUpdate);
  DefineEvent(MouseEnter);
  DefineEvent(MouseExit);
  DefineEvent(MouseEnterHierarchy);
  DefineEvent(MouseExitHierarchy);
  DefineEvent(MouseDown);
  DefineEvent(MouseUp);
  DefineEvent(LeftMouseDown);
  DefineEvent(LeftMouseUp);
  DefineEvent(RightMouseDown);
  DefineEvent(RightMouseUp);
  DefineEvent(MiddleMouseDown);
  DefineEvent(MiddleMouseUp);
  DefineEvent(MouseMove);
  DefineEvent(MouseScroll);
  DefineEvent(LeftMouseDrag);
  DefineEvent(RightMouseDrag);
  DefineEvent(MouseHold);
  DefineEvent(MouseHover);
  DefineEvent(LeftClick);
  DefineEvent(RightClick);
  DefineEvent(MiddleClick);
  DefineEvent(DoubleClick);
  DefineEvent(MouseDrop);
  DefineEvent(FocusLost);
  DefineEvent(FocusGained);
  DefineEvent(FocusLostHierarchy);
  DefineEvent(FocusGainedHierarchy);
  DefineEvent(FocusReset);
  DefineEvent(WidgetUpdate);
  DefineEvent(Activated);
  DefineEvent(Deactivated);
  DefineEvent(WidgetShown);
  DefineEvent(OnDestroy);
  DefineEvent(Closing);
}//namespace Events


String NamedMouseDown[] = {Events::LeftMouseDown, Events::RightMouseDown, Events::MiddleMouseDown};
String NamedMouseUp[] = {Events::LeftMouseUp, Events::RightMouseUp, Events::MiddleMouseUp};
String NamedMouseClick[] = {Events::LeftClick, Events::RightClick, Events::MiddleClick};

ZilchDefineType(FocusEvent, builder, type)
{
}

ZilchDefineType(HandleableEvent, builder, type)
{
}

ZilchDefineType(MouseDragEvent, builder, type)
{
}

bool MouseEvent::IsButtonDown(MouseButtons::Enum button)
{
  return mButtonDown[button] != 0;
}

MouseEvent::MouseEvent()
{
  Handled = false;
  Source = NULL;
  Button = MouseButtons::None;
  ButtonDown = false;
  Position = Vec2(0,0);
  Movement = Vec2(0,0);
  Scroll = Vec2(0,0);
  ShiftPressed = false;
  AltPressed = false;
  Handled = false;
  HandledEventScript = false;
}

ZilchDefineType(MouseEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(Button);
  ZilchBindFieldProperty(ButtonDown);
  ZilchBindFieldProperty(Position);
  ZilchBindFieldProperty(Movement);
  ZilchBindFieldProperty(Scroll);
  ZilchBindFieldProperty(ShiftPressed);
  ZilchBindFieldProperty(AltPressed);
  ZilchBindFieldProperty(CtrlPressed);
  ZilchBindGetterProperty(Mouse);
  ZilchBindMethod(IsButtonDown);
  ZilchBindMethod(IsButtonUp);

  ZilchBindFieldPropertyAs(HandledEventScript, "HandledEvent");
}

}
