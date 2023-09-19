// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Zero
{

namespace Z
{
Gamepads* gGamepads = NULL;
}

namespace Events
{
DefineEvent(ButtonDown);
DefineEvent(ButtonUp);
DefineEvent(GamepadStickFlicked);
DefineEvent(GamepadsUpdated);
DefineEvent(GamepadUpdated);
} // namespace Events

ZilchDefineType(GamepadEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindFieldProperty(mGamepad);
  ZilchBindFieldProperty(mButton);
  ZilchBindFieldProperty(mFlickDirection);
  ZilchBindFieldProperty(mFlickedStick);
}

GamepadEvent::GamepadEvent(Gamepad* gamepad, int buttonIndex)
{
  mFlickDirection = Vec2::cZero;
  mGamepad = gamepad;
  mButton = buttonIndex;
  mFlickedStick = FlickedStick::None;
}

void Button::Clear()
{
  State = Released;
  TimePressed = 0.0f;
}

void Button::Update(Gamepad* pad, uint value, float elasped, bool& anyDown)
{
  bool buttonIsDown = value != 0;
  if (buttonIsDown)
  {
    switch (State)
    {
    case Pressed:
      State = Held;
      break;
    case Released:
    case NotHeld:
    {
      State = Pressed;
      GamepadEvent e(pad, ButtonIndex);
      pad->DispatchEvent(Events::ButtonDown, &e);
    }
    break;
    case Held:
      break;
    }
    TimePressed += elasped;
    anyDown = true;
  }
  else
  {
    if (State == Released)
    {
      State = NotHeld;
    }
    else if (State != NotHeld)
    {
      GamepadEvent e(pad, ButtonIndex);
      pad->DispatchEvent(Events::ButtonUp, &e);
      State = Released;
      TimePressed = 0.0f;
    }
  }
}

ZilchDefineType(Gamepad, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mIsActive);
  ZilchBindFieldProperty(mGamepadIndex);

  ZilchBindFieldProperty(mLeftStick);
  ZilchBindFieldProperty(mRightStick);
  ZilchBindFieldProperty(mLeftStickDelta);
  ZilchBindFieldProperty(mRightStickDelta);

  ZilchBindFieldProperty(mLeftTrigger);
  ZilchBindFieldProperty(mRightTrigger);

  ZilchBindMethod(IsButtonPressed);
  ZilchBindMethod(IsButtonHeld);
  ZilchBindMethod(IsButtonReleased);
  ZilchBindMethod(TimeButtonHeld);

  ZilchBindMethod(Vibrate);

  ZeroBindEvent(Events::ButtonDown, GamepadEvent);
  ZeroBindEvent(Events::ButtonUp, GamepadEvent);
  ZeroBindEvent(Events::GamepadUpdated, ObjectEvent);
  ZeroBindEvent(Events::GamepadStickFlicked, GamepadEvent);
}

Gamepad::Gamepad()
{
}

Gamepads::~Gamepads()
{
  for (uint i = 0; i < cMaxGamepads; ++i)
  {
    delete mGamePads[i];
  }
}

void Gamepad::Initialize(uint index)
{
  mGamepadIndex = index;
  // Start as active to first update will check all game pads
  mIsActive = true;

  Clear();
}

void Gamepad::Clear()
{
  // Clear all buttons
  for (uint i = 0; i < Buttons::Size; ++i)
  {
    Buttons[i].ButtonIndex = i;
    Buttons[i].Clear();
  }

  // Clear sticks
  mLeftStick = Vec2(0, 0);
  mLeftStickDelta = Vec2(0, 0);
  mRightStick = Vec2(0, 0);
  mRightStickDelta = Vec2(0, 0);
  mLeftStickFlicked = false;
  mRightStickFlicked = false;
  mWasAnyDpadDown = false;

  mLeftTrigger = 0;
  mRightTrigger = 0;
}

float Gamepad::GetLeftTrigger()
{
  return mLeftTrigger;
}

float Gamepad::GetRightTrigger()
{
  return mRightTrigger;
}

bool Gamepad::IsButtonPressed(int index)
{
  if (index < 0 || index > Buttons::Size)
    return false;

  return Buttons[index].State == Button::Pressed;
}

bool Gamepad::IsButtonHeld(int index)
{
  if (index < 0 || index > Buttons::Size)
    return false;

  return Buttons[index].State == Button::Held;
}

bool Gamepad::IsButtonReleased(int index)
{
  if (index < 0 || index > Buttons::Size)
    return false;

  return Buttons[index].State == Button::Released;
}

float Gamepad::TimeButtonHeld(int index)
{
  if (index < 0 || index > Buttons::Size)
    return 0;

  return Buttons[index].TimePressed;
}

void Gamepad::Vibrate(float duration, float intensity)
{
  ImportGamepadVibrate(mGamepadIndex, Math::Clamp(duration, 0.0f, 1.0f), Math::Clamp(intensity, 0.0f, 1.0f));
}

void Gamepad::Update(float elasped)
{
  GamepadRawState& gamepadState = Shell::sInstance->GetOrCreateGamepad(mGamepadIndex);

  mIsActive = gamepadState.mConnected;

  if (mIsActive)
  {
    bool anyDown = false;

    // Face Buttons
    Buttons[Buttons::A].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::A).mPressed, elasped, anyDown);
    Buttons[Buttons::B].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::B).mPressed, elasped, anyDown);
    Buttons[Buttons::X].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::X).mPressed, elasped, anyDown);
    Buttons[Buttons::Y].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::Y).mPressed, elasped, anyDown);

    // Dpad Buttons
    Buttons[Buttons::DpadUp].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::DpadUp).mPressed, elasped, anyDown);
    Buttons[Buttons::DpadDown].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::DpadDown).mPressed, elasped, anyDown);
    Buttons[Buttons::DpadLeft].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::DpadLeft).mPressed, elasped, anyDown);
    Buttons[Buttons::DpadRight].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::DpadRight).mPressed, elasped, anyDown);

    bool anyDpadDown = IsButtonHeld(Buttons::DpadUp) || IsButtonHeld(Buttons::DpadDown) ||
                       IsButtonHeld(Buttons::DpadLeft) || IsButtonHeld(Buttons::DpadRight) ||
                       IsButtonPressed(Buttons::DpadUp) || IsButtonPressed(Buttons::DpadDown) ||
                       IsButtonPressed(Buttons::DpadLeft) || IsButtonPressed(Buttons::DpadRight);

    if (anyDpadDown)
    {
      if (mWasAnyDpadDown == false)
      {
        if (IsButtonHeld(Buttons::DpadUp) || IsButtonPressed(Buttons::DpadUp))
          Buttons[Buttons::DpadUpFiltered].Update(this, 1, elasped, anyDown);
        if (IsButtonHeld(Buttons::DpadDown) || IsButtonPressed(Buttons::DpadDown))
          Buttons[Buttons::DpadDownFiltered].Update(this, 1, elasped, anyDown);
        if (IsButtonHeld(Buttons::DpadLeft) || IsButtonPressed(Buttons::DpadLeft))
          Buttons[Buttons::DpadLeftFiltered].Update(this, 1, elasped, anyDown);
        if (IsButtonHeld(Buttons::DpadRight) || IsButtonPressed(Buttons::DpadRight))
          Buttons[Buttons::DpadRightFiltered].Update(this, 1, elasped, anyDown);
      }
      else
      {
        Buttons[Buttons::DpadUpFiltered].Update(
            this, Buttons[Buttons::DpadUpFiltered].State != Button::Released, elasped, anyDown);
        Buttons[Buttons::DpadDownFiltered].Update(
            this, Buttons[Buttons::DpadDownFiltered].State != Button::Released, elasped, anyDown);
        Buttons[Buttons::DpadLeftFiltered].Update(
            this, Buttons[Buttons::DpadLeftFiltered].State != Button::Released, elasped, anyDown);
        Buttons[Buttons::DpadRightFiltered].Update(
            this, Buttons[Buttons::DpadRightFiltered].State != Button::Released, elasped, anyDown);
      }
    }
    else
    {
      Buttons[Buttons::DpadUpFiltered].Update(this, 0, elasped, anyDown);
      Buttons[Buttons::DpadDownFiltered].Update(this, 0, elasped, anyDown);
      Buttons[Buttons::DpadLeftFiltered].Update(this, 0, elasped, anyDown);
      Buttons[Buttons::DpadRightFiltered].Update(this, 0, elasped, anyDown);
    }

    mWasAnyDpadDown = anyDpadDown;

    // Special Buttons
    Buttons[Buttons::Start].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::Start).mPressed, elasped, anyDown);
    Buttons[Buttons::Back].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::Back).mPressed, elasped, anyDown);

    // Thumbstick Buttons
    Buttons[Buttons::LeftThumb].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::LeftThumb).mPressed, elasped, anyDown);
    Buttons[Buttons::RightThumb].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::RightThumb).mPressed, elasped, anyDown);

    // Shoulder Buttons
    Buttons[Buttons::LeftShoulder].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::LeftShoulder).mPressed, elasped, anyDown);
    Buttons[Buttons::RightShoulder].Update(this, gamepadState.GetOrCreateButton(GamepadRawButton::RightShoulder).mPressed, elasped, anyDown);

    // The any button
    Buttons[Buttons::AnyButton].Update(this, anyDown, elasped, anyDown);

    // Divide by the max value of short
    const float fshortmax = (float)SHRT_MAX;

    float LX = gamepadState.GetOrCreateAxis(GamepadRawAxis::LeftThumbX).mValue;
    float LY = gamepadState.GetOrCreateAxis(GamepadRawAxis::LeftThumbY).mValue;
    float RX = gamepadState.GetOrCreateAxis(GamepadRawAxis::RightThumbX).mValue;
    float RY = gamepadState.GetOrCreateAxis(GamepadRawAxis::RightThumbY).mValue;

    // Update stick buttons
    Buttons[Buttons::StickUp].Update(this, LY > 0, elasped, anyDown);
    Buttons[Buttons::StickDown].Update(this, LY < 0, elasped, anyDown);
    Buttons[Buttons::StickLeft].Update(this, LX < 0, elasped, anyDown);
    Buttons[Buttons::StickRight].Update(this, LX > 0, elasped, anyDown);

    // Normalize stick values
    // There is 1 more negative value in a unsigned short
    // than positive so clamp to -1 to 1
    float lx = Math::Clamp(LX / fshortmax, -1.0f, 1.0f);
    float ly = Math::Clamp(LY / fshortmax, -1.0f, 1.0f);
    float rx = Math::Clamp(RX / fshortmax, -1.0f, 1.0f);
    float ry = Math::Clamp(RY / fshortmax, -1.0f, 1.0f);

    // Update values
    mLeftStickDelta = Vec2(lx - mLeftStick.x, ly - mLeftStick.y);
    mRightStickDelta = Vec2(rx - mRightStick.x, ry - mRightStick.y);
    mLeftStick = Vec2(lx, ly);
    mRightStick = Vec2(rx, ry);

    const float cFlickMagnitude = 0.8f;

    // Stick flicking
    bool leftStickFlicked = (Math::Length(mLeftStick) >= cFlickMagnitude);
    bool rightStickFlicked = (Math::Length(mRightStick) >= cFlickMagnitude);

    if (leftStickFlicked && !mLeftStickFlicked)
    {
      GamepadEvent toSend(this);
      toSend.mFlickedStick = FlickedStick::Left;
      toSend.mFlickDirection = Math::Normalized(mLeftStick);
      DispatchEvent(Events::GamepadStickFlicked, &toSend);
    }

    if (rightStickFlicked && !mRightStickFlicked)
    {
      GamepadEvent toSend(this);
      toSend.mFlickedStick = FlickedStick::Right;
      toSend.mFlickDirection = Math::Normalized(mRightStick);
      DispatchEvent(Events::GamepadStickFlicked, &toSend);
    }

    mLeftStickFlicked = leftStickFlicked;
    mRightStickFlicked = rightStickFlicked;

    // Update Triggers
    mLeftTrigger = gamepadState.GetOrCreateButton(GamepadRawButton::LeftTrigger).mValue;
    mRightTrigger = gamepadState.GetOrCreateButton(GamepadRawButton::RightTrigger).mValue;
  } else {
    Clear();
  }
}

ZilchDefineType(Gamepads, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZeroBindDocumented();
  ZilchBindMethod(GetGamePad);
  ZilchBindGetterProperty(MaxGamepadCount);
  ZeroBindEvent(Events::GamepadsUpdated, ObjectEvent);
}

Gamepads::Gamepads()
{
  Startup();
}

Gamepad* Gamepads::GetGamePad(uint i)
{
  if (i >= cMaxGamepads)
  {
    DoNotifyError("Invalid Controller",
                  String::Format("A controller with index '%d' was requested, but does not exist!", i));
    i = 0;
  }
  Gamepad* gamepad = mGamePads[i];
  return gamepad;
}

uint Gamepads::GetMaxGamepadCount()
{
  return cMaxGamepads;
}

void Gamepads::Startup()
{
  Z::gGamepads = this;

  for (uint i = 0; i < cMaxGamepads; ++i)
  {
    mGamePads[i] = new Gamepad();
    mGamePads[i]->Initialize(i);
  }

  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnUpdate);

  Update();
}

void Gamepads::OnUpdate(UpdateEvent* event)
{
  for (uint i = 0; i < cMaxGamepads; ++i)
  {
    Gamepad* gamepad = mGamePads[i];
    gamepad->Update(event->Dt);

    ObjectEvent toSend;
    gamepad->GetDispatcher()->Dispatch(Events::GamepadUpdated, &toSend);
  }

  ObjectEvent toSend;
  GetDispatcher()->Dispatch(Events::GamepadsUpdated, &toSend);
}

void Gamepads::Update()
{
  for (uint i = 0; i < cMaxGamepads; ++i)
    mGamePads[i]->Update(0.0f);
}

} // namespace Zero
