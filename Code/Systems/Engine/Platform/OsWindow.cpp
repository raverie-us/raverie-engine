// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

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

ZilchDefineType(OsWindow, builder, type)
{
  ZilchBindGetterProperty(ClientSize);
  ZilchBindMethod(HasFocus);
  ZilchBindSetter(MouseCapture);
  ZilchBindGetterSetter(MouseTrap);
}

OsWindow::OsWindow()
{
  ErrorIf(sInstance != nullptr, "We should only have one instance");
  sInstance = this;

  //Shell::sInstance->mOnDevicesChanged = &ShellWOnDevicesChanged;
  //Shell::sInstance->mOnInputDeviceChanged = &ShellWOnInputDeviceChanged;
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

void ZeroExportNamed(ExportQuit)() {
  OsWindowEvent event;
  OsWindow::sInstance->DispatchEvent(Events::OsClose, &event);
}

void ZeroExportNamed(ExportFocusChanged)(bool focused)
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
void ZeroExportNamed(ExportFileDropAdd)(const char* filePath) {
  gDroppedFiles.PushBack(filePath);
}

void ZeroExportNamed(ExportFileDropFinish)(int32_t clientX, int32_t clientY) {
  IntVec2 clientPosition(clientX, clientY);
  OsMouseDropEvent mouseDrop;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseDrop);
  mouseDrop.Files = gDroppedFiles;
  gDroppedFiles.Clear();

  mouseDrop.EventId = Events::OsMouseFileDrop;
  OsWindow::sInstance->SendMouseDropEvent(mouseDrop);
}

void OsWindow::SendSizeChanged(IntVec2Param clientSize) {
  Shell::sInstance->mClientSize = clientSize;
  OsWindowEvent sizeEvent;
  sizeEvent.ClientSize = clientSize;
  DispatchEvent(Events::OsResized, &sizeEvent);
}

void ZeroExportNamed(ExportSizeChanged)(int32_t clientWidth, int32_t clientHeight)
{
  IntVec2 clientSize(clientWidth, clientHeight);
  OsWindow::sInstance->SendSizeChanged(clientSize);
}

void ZeroExportNamed(ExportTextTyped)(uint32_t rune)
{
  KeyboardTextEvent textEvent(rune);
  textEvent.EventId = Events::OsKeyTyped;
  OsWindow::sInstance->SendKeyboardTextEvent(textEvent);
}

void ZeroExportNamed(ExportKeyboardButtonChanged)(Zero::Keys::Enum key, Zero::KeyState::Enum state)
{
  Shell::sInstance->mKeyState[key] = (state == KeyState::Down || state == KeyState::Repeated);
  KeyboardEvent keyEvent;
  OsWindow::sInstance->FillKeyboardEvent(key, state, keyEvent);
  OsWindow::sInstance->SendKeyboardEvent(keyEvent);
}

void ZeroExportNamed(ExportMouseButtonChanged)(int32_t clientX, int32_t clientY, Zero::MouseButtons::Enum button, Zero::MouseState::Enum state)
{
  IntVec2 clientPosition(clientX, clientY);
  Shell::sInstance->mMouseState[button] = (state == MouseState::Down);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, button, mouseEvent);
  mouseEvent.EventId = state == MouseState::Down ? Events::OsMouseDown : Events::OsMouseUp;
  OsWindow::sInstance->SendMouseEvent(mouseEvent);
}

void ZeroExportNamed(ExportMouseMove)(int32_t clientX, int32_t clientY, int32_t dx, int32_t dy) {
  IntVec2 clientPosition(clientX, clientY);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.EventId = Events::OsMouseMove;

  OsWindow::sInstance->SendMouseEvent(mouseEvent);

  Z::gMouse->mRawMovement += Vec2(dx, dy);
}

void ZeroExportNamed(ExportMouseScroll)(int32_t clientX, int32_t clientY, float scrollX, float scrollY)
{
  IntVec2 clientPosition(clientX, clientY);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.EventId = Events::OsMouseScroll;
  mouseEvent.ScrollMovement.x = scrollX;
  mouseEvent.ScrollMovement.y = scrollY;
  OsWindow::sInstance->SendMouseEvent(mouseEvent);
}

void OsWindow::ShellWOnDevicesChanged()
{
  // DeactivateAll because joysticks may have been removed in device changed
  Z::gJoysticks->DeactivateAll();

  const Array<PlatformInputDevice>& devices = Shell::sInstance->ScanInputDevices();
  forRange (PlatformInputDevice& device, devices)
  {
    // Tell the Joysticks system that a Joystick is present
    Z::gJoysticks->AddJoystickDevice(device);
  }

  Z::gJoysticks->JoysticksChanged();
}

void OsWindow::ShellWOnInputDeviceChanged(
    PlatformInputDevice& device, uint buttons, const Array<uint>& axes, const DataBlock& data)
{
  Joystick* joystick = Z::gJoysticks->GetJoystickByDevice(device.mDeviceHandle);
  ReturnIf(joystick == nullptr,
           ,
           "Unable to find a joystick by the device handle "
           "(should have been found in ShellWOnDevicesChanged)");

  joystick->RawSetButtons(buttons);

  for (size_t i = 0; i < axes.Size(); ++i)
  {
    if (i >= device.mAxes.Size())
    {
      Error("We should be getting the same number of axes as was registered "
            "with the device, unless an error occurred");
      break;
    }

    uint value = axes[i];
    if (device.mAxes[i].mCanBeDisabled && value == 0)
      joystick->RawSetAxisDisabled(i);
    else
      joystick->RawSetAxis(i, axes[i]);
  }

  // Tell everyone we updated this joystick
  joystick->SignalUpdated();
}

ZilchDefineType(OsWindowEvent, builder, type)
{
}

OsWindowEvent::OsWindowEvent() : ClientSize(IntVec2::cZero)
{
}

void OsWindowEvent::Serialize(Serializer& stream)
{
  SerializeNameDefault(EventId, String());
}

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

ZilchDefineType(OsWindowBorderHitTest, builder, type)
{
}

OsWindowBorderHitTest::OsWindowBorderHitTest() :
    ClientPosition(IntVec2::cZero)
{
}

ZilchDefineType(OsMouseDropEvent, builder, type)
{
}

void OsMouseDropEvent::Serialize(Serializer& stream)
{
  OsMouseEvent::Serialize(stream);
  SerializeName(Files);
}

} // namespace Zero
