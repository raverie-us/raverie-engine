// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

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
  for (uint i = 0; i < cMaxUsers; ++i)
  {
    SetGamepadVibration(i, 0, 0);
    delete mGamePads[i];
  }
}

void Gamepad::Initialize(Gamepads* gamepads, uint index)
{
  mGamePads = gamepads;

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

  // Clear vibration
  mVibrationTime = 0.0f;
  mIsVibrating = false;
  mLeftSpeed = 0.0f;
  mRightSpeed = 0.0f;
  SetGamepadVibration(mGamepadIndex, 0, 0);
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

void Gamepad::Vibrate(float time, float leftSpeed, float rightSpeed)
{
  mIsVibrating = true;
  mVibrationTime = time;
  mLeftSpeed = leftSpeed;
  mRightSpeed = rightSpeed;
  SetGamepadVibration(mGamepadIndex, mLeftSpeed, mRightSpeed);
}

void Gamepad::Update(float elasped)
{
  if (!mIsActive)
    return;

  // If XInputModule did not load disable the gamepad
  if (!AreGamepadsEnabled())
  {
    mIsActive = false;
    return;
  }

  GamepadState inputState;
  bool result = GetGamepadState((size_t)mGamepadIndex, &inputState);

  if (!result)
  {
    Clear();
    mIsActive = false;
  }
  else
  {
    mIsActive = true;
    bool anyDown = false;

    // Face Buttons
    Buttons[Buttons::A].Update(this, GamepadButtonFlag::A & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::B].Update(this, GamepadButtonFlag::B & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::X].Update(this, GamepadButtonFlag::X & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::Y].Update(this, GamepadButtonFlag::Y & inputState.mButtons, elasped, anyDown);

    // Dpad Buttons
    Buttons[Buttons::DpadUp].Update(this, GamepadButtonFlag::DpadUp & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::DpadDown].Update(this, GamepadButtonFlag::DpadDown & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::DpadLeft].Update(this, GamepadButtonFlag::DpadLeft & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::DpadRight].Update(this, GamepadButtonFlag::DpadRight & inputState.mButtons, elasped, anyDown);

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
    Buttons[Buttons::Start].Update(this, GamepadButtonFlag::Start & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::Back].Update(this, GamepadButtonFlag::Back & inputState.mButtons, elasped, anyDown);

    // Thumbstick Buttons
    Buttons[Buttons::LeftThumb].Update(this, GamepadButtonFlag::LThumb & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::RightThumb].Update(this, GamepadButtonFlag::RThumb & inputState.mButtons, elasped, anyDown);

    // Shoulder Buttons
    Buttons[Buttons::LeftShoulder].Update(this, GamepadButtonFlag::LShoulder & inputState.mButtons, elasped, anyDown);
    Buttons[Buttons::RightShoulder].Update(this, GamepadButtonFlag::RShoulder & inputState.mButtons, elasped, anyDown);

    // The any button
    Buttons[Buttons::AnyButton].Update(this, anyDown, elasped, anyDown);

    // Divide by the max value of short
    const float fshortmax = (float)SHRT_MAX;

    float LX = inputState.mThumbLX;
    float LY = inputState.mThumbLY;
    float RX = inputState.mThumbRX;
    float RY = inputState.mThumbRY;

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
    mLeftTrigger = inputState.mLTrigger / 255.0f;
    mRightTrigger = inputState.mRTrigger / 255.0f;

    // Animate vibration.
    if (mIsVibrating)
    {
      mVibrationTime -= elasped;
      if (mVibrationTime < 0.0f)
      {
        mVibrationTime = 0.0f;
        mIsVibrating = false;
        SetGamepadVibration(mGamepadIndex, 0.0f, 0.0f);
      }
    }
  }
}

ZilchDefineType(Gamepads, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZeroBindDocumented();
  ZilchBindMethod(PauseVibration);
  ZilchBindMethod(ResumeVibration);
  ZilchBindMethod(GetGamePad);
  ZilchBindGetterProperty(MaxGamepadCount);
  ZeroBindEvent(Events::GamepadsUpdated, ObjectEvent);
}

Gamepads::Gamepads()
{
  Startup();
}

void Gamepads::PauseVibration()
{
  mVibrationIsPaused = true;
  for (uint i = 0; i < cMaxUsers; ++i)
  {
    SetGamepadVibration(i, 0, 0);
    mGamePads[i]->mIsVibrating = false;
  }
}

void Gamepads::ResumeVibration()
{
  mVibrationIsPaused = false;
  for (uint i = 0; i < cMaxUsers; ++i)
  {
    SetGamepadVibration(i, mGamePads[i]->mLeftSpeed, mGamePads[i]->mRightSpeed);
    mGamePads[i]->mIsVibrating = true;
  }
}

void Gamepads::UpdateGamepadsActiveState()
{
  // Set mIsActive next update will test and reset value
  for (uint i = 0; i < cMaxUsers; ++i)
  {
    GamepadState inputState;
    bool result = GetGamepadState(i, &inputState);

    if (!result)
    {
      mGamePads[i]->Clear();
      mGamePads[i]->mIsActive = false;
    }
    else
    {
      mGamePads[i]->mIsActive = true;
    }
  }
}

Gamepad* Gamepads::GetGamePad(uint i)
{
  if (i >= cMaxUsers)
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
  return cMaxUsers;
}

void Gamepads::Startup()
{
  Z::gGamepads = this;

  mVibrationIsPaused = false;

  for (uint i = 0; i < cMaxUsers; ++i)
  {
    mGamePads[i] = new Gamepad();
    mGamePads[i]->Initialize(this, i);
  }

  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnUpdate);
  ConnectThisTo(Z::gEngine, Events::OsDeviceChanged, OnDeviceChanged);

  Update();
}

void Gamepads::OnUpdate(UpdateEvent* event)
{
  for (uint i = 0; i < cMaxUsers; ++i)
  {
    Gamepad* gamepad = mGamePads[i];
    gamepad->Update(event->Dt);

    ObjectEvent toSend;
    gamepad->GetDispatcher()->Dispatch(Events::GamepadUpdated, &toSend);
  }

  ObjectEvent toSend;
  GetDispatcher()->Dispatch(Events::GamepadsUpdated, &toSend);
}

void Gamepads::OnDeviceChanged(Event* event)
{
  UpdateGamepadsActiveState();
}

void Gamepads::Update()
{
  for (uint i = 0; i < cMaxUsers; ++i)
    mGamePads[i]->Update(0.0f);
}

} // namespace Zero
