// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
// Forward declarations
class Joystick;
class Joysticks;
class UpdateEvent;
class RawControlMapping;

namespace Events
{
DeclareEvent(JoystickUpdated);
DeclareEvent(JoystickButtonDown);
DeclareEvent(JoystickButtonUp);

DeclareEvent(JoystickFound);
DeclareEvent(JoystickLost);
DeclareEvent(JoysticksChanged);
} // namespace Events

class RawAxis
{
public:
  void Serialize(Serializer& stream);

  String Name;
  uint Offset;
  uint Size;
  uint Min;
  uint Mid;
  uint Max;
  float DeadZonePercent;
  bool CanCalibrate;
  bool UseMid;
  bool Reversed;
  bool CanBeDisabled;
};

class RawButton
{
public:
  void Serialize(Serializer& stream);

  String Name;
  uint Offset;
  uint Bit;
};

class RawControlMapping
{
public:
  ZilchDeclareType(RawControlMapping, TypeCopyMode::ReferenceType);

  RawControlMapping();
  RawControlMapping(const PlatformInputDevice& device);
  void Serialize(Serializer& stream);

  bool IsSame(RawControlMapping* map);

  String mName;
  Array<RawAxis> mAxes;
  Array<RawButton> mButtons;
};

/// Joystick events are sent when a game pad button state is changed
class JoystickEvent : public Event
{
public:
  ZilchDeclareType(JoystickEvent, TypeCopyMode::ReferenceType);

  JoystickEvent(Joystick* joystick, int button);

  /// The button that was just pressed or released
  int mButton;

  /// The joystick that generated this event
  Joystick* mJoystick;
};

/// A Joystick is associated with a hardware joystick,
/// and provides the ability to query axes and buttons
class Joystick : public EventObject
{
public:
  ZilchDeclareType(Joystick, TypeCopyMode::ReferenceType);

  friend class Joysticks;

  /// Constructs a cleared out joystick
  Joystick();

  /// Destroy any allocated resources
  ~Joystick();

  /// Get the name of the Joystick
  String GetName();

  /// Queries an axes and returns a value between [-1, 1].
  /// The valid range of axes is between 0 and 'GetMaxAxes'.
  /// If the axis is not valid, then the value returned is 0.
  /// If the axis is disabled, then the value returned is
  /// Joystick.DisabledValue.
  float GetAxisValue(int index);

  /// A value that means a joystick axis is invalid. For example
  /// when a HAT-switch is not pressed down, it will return this value.
  static float GetDisabledValue();

  String GetAxisName(int index);
  int GetAxisIndex(StringParam name);
  float GetAxisValueByName(StringParam name);

  /// Get the number of buttons or axes
  uint GetButtonCount();
  uint GetAxisCount();

  /// Queries a button and returns true if it is down, false if it is up
  /// The valid range of buttons is between 0 and 'GetMaxButtons'
  /// If the button is not valid, then the value returned is false
  bool GetButtonValue(uint index);

  /// Gets whether or not the joystick is active.
  bool GetIsActive();

  /// Load an input mapping.
  void LoadInputMapping(StringParam name);
  void SaveInputMapping(StringParam name);
  RawControlMapping* GetInputMapping();
  void InternalSetInputMapping(RawControlMapping* map);
  void InternalSetInputMappingIfDifferent(RawControlMapping* map);

  void StartCalibration(void);
  void EndCalibration(void);
  bool Calibrating(void);

  bool IsParsedInput();

  void SignalUpdated();
  void RawProcess(DataBlock data);
  void RawSetAxis(uint index, uint rawValue);
  void RawSetAxisDisabled(uint index);
  void RawSetButtons(uint newStates);

private:
  /// Clears out the joystick the joystick state when the controller is inactive
  void InactiveClear();

private:
  /// The control mapping tells us which axes are which and holds the min/max of
  /// the raw values. If we're directly reading from a raw buffer, the control
  /// mapping also tells us the offset/size/bits that we read from the buffer
  RawControlMapping* mRawMapping;

  /// The OS device handle that's sending input
  OsHandle mDeviceHandle;

  /// The guid allows us to mostly uniquely identify joysticks, so that when a
  /// joystick is unplugged and then plugged back in, we can map it back to the
  /// same joystick
  Guid mHardwareGuid;

  /// The name of the joystick
  String mName;

  /// Tells us if the controller is turned on and plugged in
  bool mIsActive;

  /// If set, this means we're currently calibrating the controller and updating
  /// the control mapping
  bool mAutoCalibrate;

  /// All the values of the axes for this joystick [-1, 1] (includes hats)
  Array<float> mAxes;

  /// All the states of the buttons for this joystick
  /// Every bit represents a single button
  uint mButtons;
};

typedef HashMap<Guid, Joystick*> JoystickGuidMap;
typedef JoystickGuidMap::valuerange JoystickGuidRange;
typedef HashMap<OsHandle, Joystick*> JoystickDeviceMap;
typedef JoystickDeviceMap::valuerange JoystickDeviceRange;

/// Joysticks is a collection of all joysticks available
class Joysticks : public ExplicitSingleton<Joysticks, EventObject>
{
public:
  ZilchDeclareType(Joysticks, TypeCopyMode::ReferenceType);

  /// Get the number of joysticks
  uint GetJoystickCount();

  /// Get the joystick for a given hardware id
  Joystick* GetJoystickByDevice(OsHandle handle);

  // Get a range of joystick objects
  JoystickDeviceRange GetJoysticks();

  /// Creates the joystick system and attempts to acquire any available
  /// joysticks
  Joysticks();

  /// Shutdown the system and free any joystick captures
  ~Joysticks();

  // Signal that a joystick has been added
  void DeactivateAll();
  void AddJoystickDevice(const PlatformInputDevice& device);
  void JoysticksChanged();

private:
  JoystickDeviceMap mDeviceToJoystick;
  JoystickGuidMap mGuidToJoystick;
};

namespace Z
{
extern Joysticks* gJoysticks;
}

} // namespace Zero
