///////////////////////////////////////////////////////////////////////////////
///
/// \file GamePadSystem.cpp
/// Implementation of the GamePadSystem classes.
/// 
/// Authors: Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#ifdef _MSC_VER
  #include "Platform/Windows/Windows.hpp"
  #include <Xinput.h>

  typedef DWORD (WINAPI* XInputGetState_T)(DWORD dwUserIndex, XINPUT_STATE* pState);
  typedef DWORD (WINAPI* XInputSetState_T)(DWORD dwUserIndex,  XINPUT_VIBRATION* pVibration);
  XInputGetState_T XInputGetState_P = NULL;
  XInputSetState_T XInputSetState_P = NULL;
  HMODULE XInputModule = NULL;
  const float MaxRumble = 65535.0f;

  // Use XInput 9.1.0 for compatibility with Windows 7 and Windows Vista
  #define XINPUT_DLL_VER_9_1_0 "xinput9_1_0.dll"

  void InitializeGamepad()
  {
    // Load the library instead of linked in case XInput is not installed
    XInputModule = LoadLibrary(XINPUT_DLL_VER_9_1_0);
    if(XInputModule)
    {
      ZPrint("Using XINPUT for gamepads.\n");
      XInputGetState_P = (XInputGetState_T)GetProcAddress(XInputModule, "XInputGetState");
      XInputSetState_P = (XInputSetState_T)GetProcAddress(XInputModule, "XInputSetState");
    }
    else
    {
      ZPrint("Failed to load XINPUT. Gamepads will not be available.\n");
    }
  }

  void SetGamepadVibration(uint gamepadIndex, float LeftSpeed , float RightSpeed)
  {
    if(XInputModule)
    {
      XINPUT_VIBRATION zerovibration;
      zerovibration.wLeftMotorSpeed  = (WORD)(LeftSpeed * MaxRumble);
      zerovibration.wRightMotorSpeed = (WORD)(RightSpeed * MaxRumble);
      (*XInputSetState_P)(gamepadIndex, &zerovibration);
    }
  }

  HRESULT GetGamepadState(uint gamepadIndex, XINPUT_STATE* pState)
  {
    if(XInputModule)
      return (*XInputGetState_P)(gamepadIndex, pState);
    else
      return E_FAIL;
  }

#else
  struct XINPUT_STATE{};
  void InitializeGamepad(){};
  void SetGamepadVibration(uint gamepadIndex, float LeftSpeed , float RightSpeed){};
  bool GetGamepadState(uint gamepadIndex, XINPUT_STATE* pState){return false;}
#endif

#include "Time.hpp"

namespace Zero
{

namespace Z
{
  Gamepads * gGamepads = NULL;
}

namespace Events
{
  DefineEvent(ButtonDown);
  DefineEvent(ButtonUp);
  DefineEvent(GamepadStickFlicked);
  DefineEvent(GamepadsUpdated);
  DefineEvent(GamepadUpdated);
}

//----------------------------------------------------------------- GamepadEvent
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

//----------------------------------------------------------------------- Button
void Button::Clear()
{
  State = Released;
  TimePressed = 0.0f;
}

void Button::Update(Gamepad* pad, uint value, float elasped, bool& anyDown)
{
  bool buttonIsDown = value!=0;
  if(buttonIsDown)
  {
    switch(State)
    {
    case Pressed:State = Held; break;
    case Released: 
    case NotHeld:
      {
        State = Pressed; 
        GamepadEvent e(pad, ButtonIndex);
        pad->DispatchEvent(Events::ButtonDown, &e);
      }
      break;
    case Held: break;
    }
    TimePressed += elasped;
    anyDown = true;
  }
  else
  {
    if(State == Released)
    {
      State = NotHeld;
    }
    else if(State != NotHeld)
    {
      GamepadEvent e(pad, ButtonIndex);
      pad->DispatchEvent(Events::ButtonUp, &e);
      State = Released;
      TimePressed = 0.0f;
    }
  }
}


//--------------------------------------------------------------------- Gamepads
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
  for(uint i = 0; i < cMaxUsers; ++i)
  {
    SetGamepadVibration(i, 0, 0);
    delete mGamePads[i];
  }
}

void Gamepad::Initialize(Gamepads* gamepads, uint index)
{
  mGamePads = gamepads;

  mGamepadIndex = index;
  //Start as active to first update will check all game pads
  mIsActive = true;

  Clear();
}

void Gamepad::Clear()
{
  //Clear all buttons
  for(uint i=0;i<Buttons::Size;++i)
  {
    Buttons[i].ButtonIndex = i;
    Buttons[i].Clear();
  }

  //Clear sticks
  mLeftStick = Vec2(0, 0);
  mLeftStickDelta = Vec2(0, 0);
  mRightStick = Vec2(0, 0);
  mRightStickDelta = Vec2(0, 0);
  mLeftStickFlicked = false;
  mRightStickFlicked = false;
  mWasAnyDpadDown = false;

  mLeftTrigger = 0;
  mRightTrigger = 0;

  //Clear vibration
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
  if(index < 0 || index > Buttons::Size)
    return false;

  return Buttons[index].State == Button::Pressed;
}

bool Gamepad::IsButtonHeld(int index)
{
  if(index < 0 || index > Buttons::Size)
    return false;

  return Buttons[index].State == Button::Held;
}

bool Gamepad::IsButtonReleased(int index)
{
  if(index < 0 || index > Buttons::Size)
    return false;

  return Buttons[index].State == Button::Released;
}

float Gamepad::TimeButtonHeld(int index)
{
  if(index < 0 || index > Buttons::Size)
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
  if(!mIsActive)
    return;

#ifdef _MSC_VER
  // If XInputModule did not load disable the
  // gamepad
  if(XInputModule == NULL)
  {
    mIsActive = false;
    return;
  }

  XINPUT_STATE mInputState;
  HRESULT result = GetGamepadState(mGamepadIndex, &mInputState);

  if(result != ERROR_SUCCESS)
  {
    Clear();
    mIsActive = false;
  }
  else
  {
    mIsActive = true;
    bool anyDown = false;

    //Face Buttons
    Buttons[Buttons::A].Update(this, XINPUT_GAMEPAD_A & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::B].Update(this, XINPUT_GAMEPAD_B & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::X].Update(this, XINPUT_GAMEPAD_X & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::Y].Update(this, XINPUT_GAMEPAD_Y & mInputState.Gamepad.wButtons, elasped, anyDown);

    //Dpad Buttons
    Buttons[Buttons::DpadUp].Update(this, XINPUT_GAMEPAD_DPAD_UP & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::DpadDown].Update(this, XINPUT_GAMEPAD_DPAD_DOWN & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::DpadLeft].Update(this, XINPUT_GAMEPAD_DPAD_LEFT & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::DpadRight].Update(this, XINPUT_GAMEPAD_DPAD_RIGHT & mInputState.Gamepad.wButtons, elasped, anyDown);

    bool anyDpadDown = 
      IsButtonHeld(Buttons::DpadUp)       ||
      IsButtonHeld(Buttons::DpadDown)     ||
      IsButtonHeld(Buttons::DpadLeft)     ||
      IsButtonHeld(Buttons::DpadRight)    ||
      IsButtonPressed(Buttons::DpadUp)    ||
      IsButtonPressed(Buttons::DpadDown)  ||
      IsButtonPressed(Buttons::DpadLeft)  ||
      IsButtonPressed(Buttons::DpadRight);

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
        Buttons[Buttons::DpadUpFiltered].Update(this, Buttons[Buttons::DpadUpFiltered].State != Button::Released, elasped, anyDown);
        Buttons[Buttons::DpadDownFiltered].Update(this, Buttons[Buttons::DpadDownFiltered].State != Button::Released, elasped, anyDown);
        Buttons[Buttons::DpadLeftFiltered].Update(this, Buttons[Buttons::DpadLeftFiltered].State != Button::Released, elasped, anyDown);
        Buttons[Buttons::DpadRightFiltered].Update(this, Buttons[Buttons::DpadRightFiltered].State != Button::Released, elasped, anyDown);
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

    //Special Buttons
    Buttons[Buttons::Start].Update(this, XINPUT_GAMEPAD_START & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::Back].Update(this, XINPUT_GAMEPAD_BACK & mInputState.Gamepad.wButtons, elasped, anyDown);

    //Thumbstick Buttons
    Buttons[Buttons::LeftThumb].Update(this, XINPUT_GAMEPAD_LEFT_THUMB & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::RightThumb].Update(this, XINPUT_GAMEPAD_RIGHT_THUMB & mInputState.Gamepad.wButtons, elasped, anyDown);

    //Shoulder Buttons
    Buttons[Buttons::LeftShoulder].Update(this, XINPUT_GAMEPAD_LEFT_SHOULDER & mInputState.Gamepad.wButtons, elasped, anyDown);
    Buttons[Buttons::RightShoulder].Update(this, XINPUT_GAMEPAD_RIGHT_SHOULDER & mInputState.Gamepad.wButtons, elasped, anyDown);
    
    //The any button
    Buttons[Buttons::AnyButton].Update(this, anyDown, elasped, anyDown);

    //Divide by the max value of short
    const float fshortmax = (float)SHRT_MAX;

    float LX = mInputState.Gamepad.sThumbLX;
    float LY = mInputState.Gamepad.sThumbLY;
    float RX = mInputState.Gamepad.sThumbRX;
    float RY = mInputState.Gamepad.sThumbRY;

    //Check for dead zone
    if(Math::Sqrt(LX*LX+LY*LY) < float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
    {
      LX = 0.0f;
      LY = 0.0f;
    }

    if(Math::Sqrt(RX*RX+RY*RY) < float(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
    {
      RX = 0.0f;
      RY = 0.0f;
    }

    //Update stick buttons
    Buttons[Buttons::StickUp].Update(this, LY > 0 , elasped, anyDown);
    Buttons[Buttons::StickDown].Update(this, LY < 0 , elasped, anyDown);
    Buttons[Buttons::StickLeft].Update(this, LX < 0 , elasped, anyDown);
    Buttons[Buttons::StickRight].Update(this, LX > 0 , elasped, anyDown);

    //Normalize stick values
    //There is 1 more negative value in a unsigned short
    //than positive so clamp to -1 to 1
    float lx = Math::Clamp(LX / fshortmax, -1.0f, 1.0f);
    float ly = Math::Clamp(LY / fshortmax, -1.0f, 1.0f);
    float rx = Math::Clamp(RX / fshortmax, -1.0f, 1.0f);
    float ry = Math::Clamp(RY / fshortmax, -1.0f, 1.0f);

    //Update values
    mLeftStickDelta = Vec2(lx - mLeftStick.x, ly - mLeftStick.y);
    mRightStickDelta = Vec2(rx - mRightStick.x, ry - mRightStick.y);
    mLeftStick = Vec2(lx, ly);
    mRightStick = Vec2(rx, ry);

    const float cFlickMagnitude = 0.8f;

    //Stick flicking
    bool leftStickFlicked = (Math::Length(mLeftStick) >= cFlickMagnitude);
    bool rightStickFlicked = (Math::Length(mRightStick) >= cFlickMagnitude);
    
    if(leftStickFlicked && !mLeftStickFlicked)
    {
      GamepadEvent toSend(this);
      toSend.mFlickedStick = FlickedStick::Left;
      toSend.mFlickDirection = Math::Normalized(mLeftStick);
      DispatchEvent(Events::GamepadStickFlicked, &toSend);
    }
    
    if(rightStickFlicked && !mRightStickFlicked)
    {
      GamepadEvent toSend(this);
      toSend.mFlickedStick = FlickedStick::Right;
      toSend.mFlickDirection = Math::Normalized(mRightStick);
      DispatchEvent(Events::GamepadStickFlicked, &toSend);
    }

    mLeftStickFlicked = leftStickFlicked;
    mRightStickFlicked = rightStickFlicked;

    //Update Triggers
    mLeftTrigger = mInputState.Gamepad.bLeftTrigger / 255.0f;
    mRightTrigger = mInputState.Gamepad.bRightTrigger / 255.0f;

    //Animate vibration.
    if(mIsVibrating)
    {
      mVibrationTime -= elasped;
      if(mVibrationTime < 0.0f)
      {
        mVibrationTime = 0.0f;
        mIsVibrating = false;
        SetGamepadVibration(mGamepadIndex, 0.0f, 0.0f);
      }
    }
  }
#endif
}


//------------------------------------------------------------ Gamepads
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
  for(uint i = 0; i < cMaxUsers; ++i)
  {
    SetGamepadVibration(i, 0, 0);
    mGamePads[i]->mIsVibrating = false;
  }
}

void Gamepads::ResumeVibration()
{
  mVibrationIsPaused = false;
  for(uint i = 0; i < cMaxUsers; ++i)
  {
    SetGamepadVibration(i, mGamePads[i]->mLeftSpeed, mGamePads[i]->mRightSpeed);
    mGamePads[i]->mIsVibrating = true;
  }
}

void Gamepads::CheckGamepads()
{
  //Set mIsActive next update will test and reset value
  for(uint i=0;i<cMaxUsers;++i)
    mGamePads[i]->mIsActive = true;
}

Gamepad* Gamepads::GetGamePad(uint i)
{
  if (i >= cMaxUsers)
  {
    DoNotifyError("Invalid Controller", String::Format("A controller with index '%d' was requested, but does not exist!", i));
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

  InitializeGamepad();

  for(uint i = 0; i < cMaxUsers; ++i)
  {
    mGamePads[i] = new Gamepad();
    mGamePads[i]->Initialize(this, i);
  }

  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnUpdate);
  ConnectThisTo(Z::gEngine, Events::OsDeviceAdded, OnDeviceChanged);

  Update();
}

void Gamepads::OnUpdate(UpdateEvent* event)
{
  for(uint i = 0; i < cMaxUsers; ++i)
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
  CheckGamepads();
}

void Gamepads::Update()
{
  for(uint i = 0; i < cMaxUsers; ++i)
    mGamePads[i]->Update(0.0f);
}

}
