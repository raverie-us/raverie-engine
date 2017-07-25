///////////////////////////////////////////////////////////////////////////////
///
/// \file GamePadSystem.hpp
/// Declaration of the GamePadSystem classes.
/// 
/// Authors: Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Gamepad;
class Gamepads;
class UpdateEvent;

struct Button
{
  enum ButtonState
  {
    Pressed,
    Held,
    // Was the button just released?
    Released,
    // Is the button not down
    NotHeld,
  };
  void Clear();
  float TimePressed;
  uint ButtonIndex;
  ButtonState State;
  void Update(Gamepad* gamepad, uint value, float elasped, bool& anyDown);
};

DeclareEnum23(Buttons,
  A,
  B,
  X,
  Y,
  Start,
  Back,
  LeftThumb,
  RightThumb,
  LeftShoulder,
  RightShoulder,
  DpadUp,
  DpadDown,
  DpadLeft,
  DpadRight,
  DpadUpFiltered,
  DpadDownFiltered,
  DpadLeftFiltered,
  DpadRightFiltered,
  StickUp,
  StickDown,
  StickLeft,
  StickRight,
  AnyButton);

namespace Events
{
  DeclareEvent(ButtonDown);
  DeclareEvent(ButtonUp);
  DeclareEvent(GamepadStickFlicked);
  DeclareEvent(GamepadsUpdated);
  DeclareEvent(GamepadUpdated);
}

DeclareEnum3(FlickedStick, None, Left, Right);

/// Gamepad events are send when a game pad button state is changed.
class GamepadEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GamepadEvent(Gamepad* gamepad, int buttonIndex = -1);
  /// Button that was just pressed down or released up.
  int mButton;
  /// Gamepad that generated this event.
  Gamepad* mGamepad;

  /// When responding to the 'GamepadStickFlicked' event, this will be set to the stick that was flicked
  FlickedStick::Enum mFlickedStick;

  /// The direction of the stick that was flicked (normalized)
  Vec2 mFlickDirection;
};


/// Game pad is a object for getting game pad input.
class Gamepad : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Gamepad();

  /// Current offset [-1,1] from the center of the left stick.
  Vec2 mLeftStick;
  /// Current offset [-1,1] from the center of the right stick.
  Vec2 mRightStick;
  /// Change in the left stick this frame.
  Vec2 mLeftStickDelta;
  /// Change in the right stick this frame.
  Vec2 mRightStickDelta;
  /// Is this gamepad currently vibrating.
  bool mIsVibrating;

  /// Value of how much the Left Trigger is down. Range [0,1].
  float GetLeftTrigger();
  float mLeftTrigger;

  /// Value of how much the Right Trigger is down. Range [0,1].
  float GetRightTrigger();
  float mRightTrigger;

  /// Has the button just been pressed this frame.
  bool IsButtonPressed(int index);
  /// Is the button currently being held down.
  bool IsButtonHeld(int index);
  /// Is the button just been released.
  bool IsButtonReleased(int index);
  /// How long has this button been held down.
  float TimeButtonHeld(int index);

  /// Vibrate this controller for a given time. 
  /// Speed is a value between zero and one.
  void Vibrate(float time , float LeftSpeed , float RightSpeed);

  Button Buttons[Buttons::Size];

  // Internals
  /// Is this controller turned on and plugged in.
  bool mIsActive;
  /// Index of this gamepad.
  int mGamepadIndex;

private:

  // If the user moves the stick from its base position to the edge, the stick is considered flicked
  // This property generally should not be read by the user (rather event based)
  bool mLeftStickFlicked;
  bool mRightStickFlicked;

  // This is used for dpad filtering
  bool mWasAnyDpadDown;

  float mVibrationTime;
  float mLeftSpeed;
  float mRightSpeed;
  void Clear();
  void Initialize(Gamepads* gamepads, uint index);
  void Update(float elasped);
  Gamepads* mGamePads;
  friend class Gamepads;
};

const uint cMaxUsers = 4;

/// Gamepads is a collection of gamepads.
class Gamepads : public ExplicitSingleton<Gamepads, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Gamepads();
  ~Gamepads();

  /// Pause Vibration on all gamepads.
  void PauseVibration();
  /// Resume vibration on all gamepads.
  void ResumeVibration();
  /// Get the gamepad for a given index from [0, GamepadCount]
  Gamepad* GetGamePad(uint gamepadIndex);
  /// Gets the maximum number of supported gamepads
  uint GetMaxGamepadCount();

  void Startup();
  void Update();
  void CheckGamepads();

  void OnUpdate(UpdateEvent* event);
  void OnDeviceChanged(Event* event);
private:
  Gamepad* mGamePads[4];
  bool mVibrationIsPaused;
};

namespace Z
{
extern Gamepads * gGamepads;
}

}
