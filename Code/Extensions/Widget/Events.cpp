// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(MouseEnterPreview);
DefineEvent(KeyPreview);
DefineEvent(HoverKeyPreview);
DefineEvent(HoverKeyDown);
DefineEvent(HoverKeyUp);
DefineEvent(HoverKeyRepeated);
DefineEvent(MouseFileDrop);
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
} // namespace Events

String NamedMouseDown[] = {Events::LeftMouseDown, Events::RightMouseDown, Events::MiddleMouseDown};
String NamedMouseUp[] = {Events::LeftMouseUp, Events::RightMouseUp, Events::MiddleMouseUp};
String NamedMouseClick[] = {Events::LeftClick, Events::RightClick, Events::MiddleClick};

RaverieDefineType(FocusEvent, builder, type)
{
}

RaverieDefineType(HandleableEvent, builder, type)
{
  RaverieBindFieldProperty(Handled);
  RaverieBindDocumented();
}

RaverieDefineType(MouseDragEvent, builder, type)
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
  Position = Vec2(0, 0);
  Movement = Vec2(0, 0);
  Scroll = Vec2(0, 0);
  ShiftPressed = false;
  AltPressed = false;
  Handled = false;
  HandledEventScript = false;
}

RaverieDefineType(MouseEvent, builder, type)
{
  RaverieBindDocumented();
  RaverieBindFieldProperty(Button);
  RaverieBindFieldProperty(ButtonDown);
  RaverieBindFieldProperty(Position);
  RaverieBindFieldProperty(Movement);
  RaverieBindFieldProperty(Scroll);
  RaverieBindFieldProperty(ShiftPressed);
  RaverieBindFieldProperty(AltPressed);
  RaverieBindFieldProperty(CtrlPressed);
  RaverieBindGetterProperty(Mouse);
  RaverieBindMethod(IsButtonDown);
  RaverieBindMethod(IsButtonUp);

  RaverieBindFieldPropertyAs(HandledEventScript, "HandledEvent");
}

RaverieDefineType(MouseFileDropEvent, builder, type)
{
  RaverieBindMember(Files);
}

MouseFileDropEvent::MouseFileDropEvent() : MouseEvent()
{
  Files = RaverieAllocate(ArrayString);
}

MouseFileDropEvent::MouseFileDropEvent(const MouseEvent& rhs) : MouseEvent(rhs)
{
  Files = RaverieAllocate(ArrayString);
}

void MouseFileDropEvent::Copy(const OsMouseDropEvent& rhs)
{
  Array<HandleOfString>& files = Files->NativeArray;
  files.Insert(files.Begin(), rhs.Files.Begin(), rhs.Files.End());
}

} // namespace Raverie
