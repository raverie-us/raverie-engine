///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(OsMouseUp);
  DefineEvent(OsMouseDown);
  DefineEvent(OsMouseMove);
  DefineEvent(OsMouseScroll);
  DefineEvent(OsKeyDown);
  DefineEvent(OsKeyUp);
  DefineEvent(OsKeyRepeated);
  DefineEvent(OsKeyTyped);
  DefineEvent(OsDeviceAdded);
  DefineEvent(OsResized);
  DefineEvent(OsMoved);
  DefineEvent(OsClose);
  DefineEvent(OsFocusGained);
  DefineEvent(OsFocusLost);
  DefineEvent(OsPaint);
  DefineEvent(OsMouseFileDrop);
}//namespace Events

const String cOsKeyboardEventsFromState[] =
{
  Events::OsKeyUp,
  Events::OsKeyDown,
  Events::OsKeyRepeated
};

//-------------------------------------------------------------------OsWindow
ZilchDefineType(OsWindow, builder, type)
{
  ZilchBindGetterSetterProperty(Position);
  ZilchBindGetterSetterProperty(Size);
  ZilchBindSetter(MinSize);
  ZilchBindGetterSetterProperty(ClientSize);
  ZilchBindGetterProperty(Parent);
  // Seems to be problematic to expose because they can set some dangerous stuff
  //ZilchBindGetterSetterProperty(Style);
  ZilchBindGetterSetterProperty(Visible);
  ZilchBindSetter(Title);
  ZilchBindGetterSetterProperty(State);

  ZilchBindMethod(HasFocus);
  // Currently behaves weird when called from script when the window doesn't have focus
  //ZilchBindMethod(TakeFocus);
  ZilchBindSetter(MouseCapture);
  ZilchBindGetterSetter(MouseTrap);
  ZilchBindSetter(MouseCursor);

  ZilchBindMethod(ScreenToClient);
  ZilchBindMethod(ClientToScreen);
}

//-------------------------------------------------------------------OsWindowEvent
ZilchDefineType(OsWindowEvent, builder, type)
{
}

//-------------------------------------------------------------------OsMouseEvent
ZilchDefineType(OsMouseEvent, builder, type)
{
}

OsMouseEvent::OsMouseEvent()
{
  Clear();
}

void OsMouseEvent::Clear()
{
  ClientPosition = IntVec2(0, 0);
  ScrollMovement = Vec2(0, 0);
  ShiftPressed = false;
  AltPressed = false;
  CtrlPressed = false;
  MouseButton = MouseButton;
  IsTrapMoveBack = false;
  for(uint i = 0; i < MouseButtons::Size; ++i)
    ButtonDown[i] = false;
}

void OsMouseEvent::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Loading)
  {
    PolymorphicNode eventNode;

    stream.GetPolymorphic(eventNode);
  }
  else
  {
    stream.StartPolymorphic(ZilchTypeId(OsMouseEvent));
  }

  SerializeNameDefault(ShiftPressed, false);
  SerializeNameDefault(AltPressed, false);
  SerializeNameDefault(CtrlPressed, false);
  SerializeNameDefault(ClientPosition, IntVec2::cZero);
  SerializeNameDefault(ScrollMovement, Vec2::cZero);
  SerializeNameDefault(IsTrapMoveBack, false);

  SerializeEnumNameDefault(MouseButtons, MouseButton, MouseButtons::None);

  stream.EndPolymorphic();
}

//-------------------------------------------------------------------OsMouseDropEvent
ZilchDefineType(OsMouseDropEvent, builder, type)
{
}

}//namespace Zero
