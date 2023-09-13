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
  ZilchBindGetterSetterProperty(MonitorClientPosition);
  ZilchBindSetter(MinClientSize);
  ZilchBindGetterSetterProperty(ClientSize);
  // Seems to be problematic to expose because they can set some dangerous stuff
  // ZilchBindGetterSetterProperty(Style);
  ZilchBindGetterSetterProperty(Visible);
  ZilchBindSetter(Title);
  ZilchBindGetterSetterProperty(State);

  ZilchBindMethod(HasFocus);
  // Currently behaves weird when called from script when the window doesn't
  // have focus
  // ZilchBindMethod(TakeFocus);
  ZilchBindSetter(MouseCapture);
  ZilchBindGetterSetter(MouseTrap);

  ZilchBindMethod(MonitorToClient);
  ZilchBindMethod(ClientToMonitor);
}

OsWindow::OsWindow(OsShell* shell,
                   StringParam windowName,
                   IntVec2Param clientSize,
                   IntVec2Param monitorClientPos,
                   WindowStyleFlags::Enum flags,
                   WindowState::Enum state) :
    mWindow(&shell->mShell,
            windowName,
            clientSize,
            monitorClientPos,
            flags,
            state),
    mMouseTrapped(false)
{
  ErrorIf(sInstance != nullptr, "We should only have one instance");
  sInstance = this;
  
  mWindow.mUserData = this;

  mWindow.mOnClose = &ShellWindowOnClose;
  mWindow.mOnFocusChanged = &ShellWindowOnFocusChanged;
  mWindow.mOnMouseDropFiles = &ShellWindowOnMouseDropFiles;
  mWindow.mOnClientSizeChanged = &ShellWindowOnClientSizeChanged;
  mWindow.mOnMouseScrollY = &ShellWindowOnMouseScrollY;
  mWindow.mOnMouseScrollX = &ShellWindowOnMouseScrollX;
  mWindow.mOnDevicesChanged = &ShellWindowOnDevicesChanged;
  mWindow.mOnHitTest = &ShellWindowOnHitTest;
  mWindow.mOnInputDeviceChanged = &ShellWindowOnInputDeviceChanged;

  // Since we're creating the main window, do a single scan for input devices
  // (they rely on a main window)
  if (flags & WindowStyleFlags::MainWindow)
    shell->ScanInputDevices();
}

OsWindow::~OsWindow()
{
}

IntVec2 OsWindow::GetMonitorClientPosition()
{
  return mWindow.GetMonitorClientPosition();
}

void OsWindow::SetMonitorClientPosition(IntVec2Param monitorPosition)
{
  return mWindow.SetMonitorClientPosition(monitorPosition);
}

IntVec2 OsWindow::GetBorderedSize()
{
  return mWindow.GetBorderedSize();
}

void OsWindow::SetBorderedSize(IntVec2Param borderedSize)
{
  return mWindow.SetBorderedSize(borderedSize);
}

void OsWindow::SetMinClientSize(IntVec2Param minClientSize)
{
  return mWindow.SetMinClientSize(minClientSize);
}

IntVec2 OsWindow::GetClientSize()
{
  return mWindow.GetClientSize();
}

void OsWindow::SetClientSize(IntVec2Param clientSize)
{
  return mWindow.SetClientSize(clientSize);
}

IntVec2 OsWindow::MonitorToClient(IntVec2Param monitorPosition)
{
  return mWindow.MonitorToClient(monitorPosition);
}

IntVec2 OsWindow::ClientToMonitor(IntVec2Param clientPosition)
{
  return mWindow.ClientToMonitor(clientPosition);
}

WindowStyleFlags::Enum OsWindow::GetStyle()
{
  return mWindow.GetStyle();
}

void OsWindow::SetStyle(WindowStyleFlags::Enum style)
{
  return mWindow.SetStyle(style);
}

bool OsWindow::GetVisible()
{
  return mWindow.GetVisible();
}

void OsWindow::SetVisible(bool visible)
{
  return mWindow.SetVisible(visible);
}

void OsWindow::SetTitle(StringParam title)
{
  return mWindow.SetTitle(title);
}

String OsWindow::GetTitle()
{
  return mWindow.GetTitle();
}

WindowState::Enum OsWindow::GetState()
{
  return mWindow.GetState();
}

void OsWindow::SetState(WindowState::Enum windowState)
{
  return mWindow.SetState(windowState);
}

void OsWindow::TakeFocus()
{
  return mWindow.TakeFocus();
}

bool OsWindow::HasFocus()
{
  return mWindow.HasFocus();
}

void OsWindow::Close()
{
  return mWindow.Close();
}

void OsWindow::Destroy()
{
  return mWindow.Destroy();
}

void OsWindow::SetMouseCapture(bool enabled)
{
  return mWindow.SetMouseCapture(enabled);
}

bool OsWindow::GetMouseTrap()
{
  return mMouseTrapped;
}

void OsWindow::SetMouseTrap(bool mouseTrapped)
{
  mMouseTrapped = mouseTrapped;
  ImportMouseTrap(mouseTrapped);
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
  Shell* shell = mWindow.mShell;
  keyEvent.Key = key;
  keyEvent.State = keyState;
  keyEvent.mKeyboard = Keyboard::GetInstance();
  keyEvent.AltPressed = shell->IsKeyDown(Keys::Alt);
  keyEvent.CtrlPressed = shell->IsKeyDown(Keys::Control);
  keyEvent.ShiftPressed = shell->IsKeyDown(Keys::Shift);
  keyEvent.SpacePressed = shell->IsKeyDown(Keys::Space);
}

void OsWindow::FillMouseEvent(IntVec2Param clientPosition, MouseButtons::Enum mouseButton, OsMouseEvent& mouseEvent)
{
  Shell* shell = mWindow.mShell;
  mouseEvent.ClientPosition = clientPosition;
  mouseEvent.ScrollMovement = Vec2(0, 0);
  mouseEvent.AltPressed = shell->IsKeyDown(Keys::Alt);
  mouseEvent.CtrlPressed = shell->IsKeyDown(Keys::Control);
  mouseEvent.ShiftPressed = shell->IsKeyDown(Keys::Shift);
  mouseEvent.ButtonDown[MouseButtons::Left] = shell->IsMouseDown(MouseButtons::Left);
  mouseEvent.ButtonDown[MouseButtons::Right] = shell->IsMouseDown(MouseButtons::Right);
  mouseEvent.ButtonDown[MouseButtons::Middle] = shell->IsMouseDown(MouseButtons::Middle);
  mouseEvent.ButtonDown[MouseButtons::XOneBack] = shell->IsMouseDown(MouseButtons::XOneBack);
  mouseEvent.ButtonDown[MouseButtons::XTwoForward] = shell->IsMouseDown(MouseButtons::XTwoForward);
  mouseEvent.ButtonDown[MouseButtons::None] = 0;
  mouseEvent.MouseButton = mouseButton;
}

void OsWindow::ShellWindowOnClose(ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;
  OsWindowEvent event;
  self->DispatchEvent(Events::OsClose, &event);
}

void OsWindow::ShellWindowOnFocusChanged(bool activated, ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;
  OsWindowEvent focusEvent;
  if (activated)
  {
    focusEvent.EventId = Events::OsFocusGained;
  }
  else
  {
    Keyboard::Instance->Clear();
    focusEvent.EventId = Events::OsFocusLost;
  }

  self->SendWindowEvent(focusEvent);
}

void OsWindow::ShellWindowOnMouseDropFiles(Math::IntVec2Param clientPosition,
                                           const Array<String>& files,
                                           ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;

  OsMouseDropEvent mouseDrop;
  self->FillMouseEvent(clientPosition, MouseButtons::None, mouseDrop);
  mouseDrop.Files = files;

  mouseDrop.EventId = Events::OsMouseFileDrop;
  self->SendMouseDropEvent(mouseDrop);
}

void OsWindow::ShellWindowOnClientSizeChanged(Math::IntVec2Param clientSize, ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;
  OsWindowEvent sizeEvent;
  sizeEvent.ClientSize = clientSize;
  self->DispatchEvent(Events::OsResized, &sizeEvent);
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

void ZeroExportNamed(ExportMouseButtonChanged)(int32_t x, int32_t y, Zero::MouseButtons::Enum button, Zero::MouseState::Enum state)
{
  Shell::sInstance->mMouseState[button] = (state == MouseState::Down);
  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(IntVec2(x, y), button, mouseEvent);
  mouseEvent.EventId = state == MouseState::Down ? Events::OsMouseDown : Events::OsMouseUp;
  OsWindow::sInstance->SendMouseEvent(mouseEvent);
}

void ZeroExportNamed(ExportMouseMove)(int32_t x, int32_t y, int32_t dx, int32_t dy) {
  IntVec2 clientPosition(x, y);

  OsMouseEvent mouseEvent;
  OsWindow::sInstance->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.EventId = Events::OsMouseMove;

  OsWindow::sInstance->SendMouseEvent(mouseEvent);

  Z::gMouse->mRawMovement += Vec2(dx, dy);
}

void OsWindow::ShellWindowOnMouseScrollY(Math::IntVec2Param clientPosition, float scrollAmount, ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;
  OsMouseEvent mouseEvent;
  self->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.EventId = Events::OsMouseScroll;
  mouseEvent.ScrollMovement.y = scrollAmount;
  self->SendMouseEvent(mouseEvent);
}

void OsWindow::ShellWindowOnMouseScrollX(Math::IntVec2Param clientPosition, float scrollAmount, ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;
  OsMouseEvent mouseEvent;
  self->FillMouseEvent(clientPosition, MouseButtons::None, mouseEvent);
  mouseEvent.EventId = Events::OsMouseScroll;
  mouseEvent.ScrollMovement.x = scrollAmount;
  self->SendMouseEvent(mouseEvent);
}

void OsWindow::ShellWindowOnDevicesChanged(ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;

  // DeactivateAll because joysticks may have been removed in device changed
  Z::gJoysticks->DeactivateAll();

  const Array<PlatformInputDevice>& devices = window->mShell->ScanInputDevices();
  forRange (PlatformInputDevice& device, devices)
  {
    // Tell the Joysticks system that a Joystick is present
    Z::gJoysticks->AddJoystickDevice(device);
  }

  Z::gJoysticks->JoysticksChanged();
}

WindowBorderArea::Enum OsWindow::ShellWindowOnHitTest(Math::IntVec2Param clientPosition, ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;

  OsWindowBorderHitTest event;
  event.Window = self;
  event.ClientPosition = clientPosition;
  self->DispatchEvent(Events::OsWindowBorderHitTest, &event);
  return event.mWindowBorderArea;
}

void OsWindow::ShellWindowOnInputDeviceChanged(
    PlatformInputDevice& device, uint buttons, const Array<uint>& axes, const DataBlock& data, ShellWindow* window)
{
  OsWindow* self = (OsWindow*)window->mUserData;

  Joystick* joystick = Z::gJoysticks->GetJoystickByDevice(device.mDeviceHandle);
  ReturnIf(joystick == nullptr,
           ,
           "Unable to find a joystick by the device handle "
           "(should have been found in ShellWindowOnDevicesChanged)");

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

OsWindowEvent::OsWindowEvent() : Window(nullptr), ClientSize(IntVec2::cZero)
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
    Window(nullptr),
    ClientPosition(IntVec2::cZero),
    mWindowBorderArea(WindowBorderArea::None)
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
