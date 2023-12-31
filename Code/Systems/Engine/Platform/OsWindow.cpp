// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Raverie
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
DefineEvent(OsDeviceChanged);
DefineEvent(OsResized);
DefineEvent(OsMoved);
DefineEvent(OsClose);
DefineEvent(OsFocusGained);
DefineEvent(OsFocusLost);
DefineEvent(OsPaint);
DefineEvent(OsMouseFileDrop);
DefineEvent(OsWindowBorderHitTest);
} // namespace Events

const String cOsKeyboardEventsFromState[] = {Events::OsKeyUp, Events::OsKeyDown, Events::OsKeyRepeated};
OsWindow* OsWindow::sInstance = nullptr;

RaverieDefineType(OsWindow, builder, type)
{
  RaverieBindGetterProperty(ClientSize);
  RaverieBindMethod(HasFocus);
  RaverieBindSetter(MouseCapture);
  RaverieBindGetterSetter(MouseTrap);
}

OsWindow::OsWindow()
{
  ErrorIf(sInstance != nullptr, "We should only have one instance");
  sInstance = this;
}

OsWindow::~OsWindow()
{
}

IntVec2 OsWindow::GetClientSize()
{
  return Shell::sInstance->GetClientSize();
}

bool OsWindow::HasFocus()
{
  return Shell::sInstance->mHasFocus;
}

void OsWindow::SetMouseCapture(bool enabled)
{
  return Shell::sInstance->SetMouseCapture(enabled);
}

bool OsWindow::GetMouseTrap()
{
  return Shell::sInstance->GetMouseTrap();
}

void OsWindow::SetMouseTrap(bool mouseTrapped)
{
  Shell::sInstance->SetMouseTrap(mouseTrapped);
}

void OsWindow::SendKeyboardEvent(KeyboardEvent& event)
{
  Keyboard* keyboard = Keyboard::GetInstance();
  keyboard->UpdateKeys(event);

  DispatchEvent(cOsKeyboardEventsFromState[event.State], &event);

  keyboard->DispatchEvent(cKeyboardEventsFromState[event.State], &event);
}

void OsWindow::SendKeyboardTextEvent(KeyboardTextEvent& event)
{
  DispatchEvent(event.EventId, &event);
  Keyboard::GetInstance()->DispatchEvent(Events::TextTyped, &event);
}

void OsWindow::SendMouseEvent(OsMouseEvent& event)
{
  DispatchEvent(event.EventId, &event);
}

void OsWindow::SendMouseDropEvent(OsMouseDropEvent& event)
{
  DispatchEvent(event.EventId, &event);
}

void OsWindow::SendWindowEvent(OsWindowEvent& event)
{
  DispatchEvent(event.EventId, &event);
}

void OsWindow::FillKeyboardEvent(Keys::Enum key, KeyState::Enum keyState, KeyboardEvent& keyEvent)
{
  keyEvent.Key = key;
  keyEvent.State = keyState;
  keyEvent.mKeyboard = Keyboard::GetInstance();
  keyEvent.AltPressed = Shell::sInstance->IsKeyDown(Keys::Alt);
  keyEvent.CtrlPressed = Shell::sInstance->IsKeyDown(Keys::Control);
  keyEvent.ShiftPressed = Shell::sInstance->IsKeyDown(Keys::Shift);
  keyEvent.SpacePressed = Shell::sInstance->IsKeyDown(Keys::Space);
}

void OsWindow::FillMouseEvent(IntVec2Param clientPosition, MouseButtons::Enum mouseButton, OsMouseEvent& mouseEvent)
{
  mouseEvent.ClientPosition = clientPosition;
  mouseEvent.ScrollMovement = Vec2(0, 0);
  mouseEvent.AltPressed = Shell::sInstance->IsKeyDown(Keys::Alt);
  mouseEvent.CtrlPressed = Shell::sInstance->IsKeyDown(Keys::Control);
  mouseEvent.ShiftPressed = Shell::sInstance->IsKeyDown(Keys::Shift);
  mouseEvent.ButtonDown[MouseButtons::Left] = Shell::sInstance->IsMouseDown(MouseButtons::Left);
  mouseEvent.ButtonDown[MouseButtons::Right] = Shell::sInstance->IsMouseDown(MouseButtons::Right);
  mouseEvent.ButtonDown[MouseButtons::Middle] = Shell::sInstance->IsMouseDown(MouseButtons::Middle);
  mouseEvent.ButtonDown[MouseButtons::XOneBack] = Shell::sInstance->IsMouseDown(MouseButtons::XOneBack);
  mouseEvent.ButtonDown[MouseButtons::XTwoForward] = Shell::sInstance->IsMouseDown(MouseButtons::XTwoForward);
  mouseEvent.ButtonDown[MouseButtons::None] = 0;
  mouseEvent.MouseButton = mouseButton;
}

void RaverieExportNamed(ExportQuit)()
{
  OsWindowEvent event;
  OsWindow::sInstance->DispatchEvent(Events::OsClose, &event);
}

void RaverieExportNamed(ExportFocusChanged)(bool focused)
{
  Shell::sInstance->mHasFocus = focused;
  OsWindowEvent focusEvent;
  if (focused)
  {
    focusEvent.EventId = Events::OsFocusGained;
  }
  else
  {
    Keyboard::Instance->Clear();
    focusEvent.EventId = Events::OsFocusLost;
  }

  OsWindow::sInstance->SendWindowEvent(focusEvent);
}

Array<String> gDroppedFiles;
void RaverieExportNamed(ExportFileDropAdd)(const char* filePath)
{
  gDroppedFiles.PushBack(filePath);
}

void RaverieExportNamed(ExportFileDropFinish)(int32_t clientX, int32_t clientY)
{
  IntVec2 clientPosition(clientX, clientY);
  OsMouseDropEvent mouseDrop;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseDrop);
  mouseDrop.Files = gDroppedFiles;
  gDroppedFiles.Clear();

  mouseDrop.EventId = Events::OsMouseFileDrop;
  OsWindow::sInstance->SendMouseDropEvent(mouseDrop);
}

void RaverieExportNamed(ExportSizeChanged)(int32_t clientWidth, int32_t clientHeight)
{
  IntVec2 clientSize(clientWidth, clientHeight);
  Shell::sInstance->mClientSize = clientSize;
  OsWindowEvent sizeEvent;
  sizeEvent.ClientSize = clientSize;
  OsWindow::sInstance->DispatchEvent(Events::OsResized, &sizeEvent);
}

void RaverieExportNamed(ExportTextTyped)(uint32_t rune)
{
  KeyboardTextEvent textEvent(rune);
  textEvent.EventId = Events::OsKeyTyped;
  OsWindow::sInstance->SendKeyboardTextEvent(textEvent);
}

void RaverieExportNamed(ExportKeyboardButtonChanged)(Raverie::Keys::Enum key, Raverie::KeyState::Enum state)
{
  Shell::sInstance->mKeyState[key] = (state == KeyState::Down || state == KeyState::Repeated);
  KeyboardEvent keyEvent;
  OsWindow::sInstance->FillKeyboardEvent(key, state, keyEvent);
  OsWindow::sInstance->SendKeyboardEvent(keyEvent);
}

void RaverieExportNamed(ExportMouseButtonChanged)(int32_t clientX, int32_t clientY, Raverie::MouseButtons::Enum button, Raverie::MouseState::Enum state)
{
  IntVec2 clientPosition(clientX, clientY);
  Shell::sInstance->mMouseState[button] = (state == MouseState::Down);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, button, mouseEvent);
  mouseEvent.EventId = state == MouseState::Down ? Events::OsMouseDown : Events::OsMouseUp;
  OsWindow::sInstance->SendMouseEvent(mouseEvent);
}

void RaverieExportNamed(ExportMouseMove)(int32_t clientX, int32_t clientY, int32_t dx, int32_t dy)
{
  IntVec2 clientPosition(clientX, clientY);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.Movement = IntVec2(dx, dy);
  mouseEvent.EventId = Events::OsMouseMove;

  OsWindow::sInstance->SendMouseEvent(mouseEvent);

  Z::gMouse->mRawMovement += Vec2(dx, dy);
}

void RaverieExportNamed(ExportMouseScroll)(int32_t clientX, int32_t clientY, float scrollX, float scrollY)
{
  IntVec2 clientPosition(clientX, clientY);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.EventId = Events::OsMouseScroll;
  mouseEvent.ScrollMovement.x = scrollX;
  mouseEvent.ScrollMovement.y = scrollY;
  OsWindow::sInstance->SendMouseEvent(mouseEvent);
}

void RaverieExportNamed(ExportGamepadConnectionChanged)(uint32_t gamepadIndex, const char* id, bool connected)
{
  auto& gamepad = Shell::sInstance->GetOrCreateGamepad(gamepadIndex);
  gamepad.mConnected = connected;

  if (connected)
  {
    gamepad.mId = id;
  }
  else
  {
    gamepad.mId.Clear();
    gamepad.mButtons.Clear();
    gamepad.mAxes.Clear();
  }

  Event event;
  OsWindow::sInstance->DispatchEvent(Events::OsDeviceChanged, &event);
}

void RaverieExportNamed(ExportGamepadButtonChanged)(uint32_t gamepadIndex, uint32_t buttonIndex, bool pressed, bool touched, float value)
{
  auto& gamepad = Shell::sInstance->GetOrCreateGamepad(gamepadIndex);
  auto& button = gamepad.GetOrCreateButton(buttonIndex);

  button.mPressed = pressed;
  button.mTouched = touched;
  button.mValue = value;
}

void RaverieExportNamed(ExportGamepadAxisChanged)(uint32_t gamepadIndex, uint32_t axisIndex, float value)
{
  auto& gamepad = Shell::sInstance->GetOrCreateGamepad(gamepadIndex);
  gamepad.GetOrCreateAxis(axisIndex).mValue = value;
}

RaverieDefineType(OsWindowEvent, builder, type)
{
}

OsWindowEvent::OsWindowEvent() : ClientSize(IntVec2::cZero)
{
}

void OsWindowEvent::Serialize(Serializer& stream)
{
  SerializeNameDefault(EventId, String());
}

RaverieDefineType(OsMouseEvent, builder, type)
{
}

OsMouseEvent::OsMouseEvent()
{
  Clear();
}

void OsMouseEvent::Clear()
{
  ClientPosition = IntVec2::cZero;
  Movement = IntVec2::cZero;
  ScrollMovement = Vec2::cZero;
  ShiftPressed = false;
  AltPressed = false;
  CtrlPressed = false;
  MouseButton = MouseButtons::None;
  for (uint i = 0; i < MouseButtons::Size; ++i)
    ButtonDown[i] = false;
}

void OsMouseEvent::Serialize(Serializer& stream)
{
  SerializeNameDefault(EventId, String());
  SerializeNameDefault(ShiftPressed, false);
  SerializeNameDefault(AltPressed, false);
  SerializeNameDefault(CtrlPressed, false);
  SerializeNameDefault(ClientPosition, IntVec2::cZero);
  SerializeNameDefault(ScrollMovement, Vec2::cZero);

  SerializeEnumNameDefault(MouseButtons, MouseButton, MouseButtons::None);

  static_assert(sizeof(bool) == sizeof(byte), "For this trick work the size must be the same");
  bool& LeftButton = (bool&)ButtonDown[MouseButtons::Left];
  bool& RightButton = (bool&)ButtonDown[MouseButtons::Right];
  bool& MiddleButton = (bool&)ButtonDown[MouseButtons::Middle];
  bool& XOneBackButton = (bool&)ButtonDown[MouseButtons::XOneBack];
  bool& XTwoForwardButton = (bool&)ButtonDown[MouseButtons::XTwoForward];

  SerializeNameDefault(LeftButton, false);
  SerializeNameDefault(RightButton, false);
  SerializeNameDefault(MiddleButton, false);
  SerializeNameDefault(XOneBackButton, false);
  SerializeNameDefault(XTwoForwardButton, false);
}

RaverieDefineType(OsWindowBorderHitTest, builder, type)
{
}

OsWindowBorderHitTest::OsWindowBorderHitTest() : ClientPosition(IntVec2::cZero)
{
}

RaverieDefineType(OsMouseDropEvent, builder, type)
{
}

void OsMouseDropEvent::Serialize(Serializer& stream)
{
  OsMouseEvent::Serialize(stream);
  SerializeName(Files);
}

} // namespace Raverie
