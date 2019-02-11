// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class KeyboardEvent;

namespace Events
{
DeclareEvent(KeyUp);
DeclareEvent(KeyDown);
DeclareEvent(KeyRepeated);
DeclareEvent(TextTyped);
} // namespace Events

extern const String cKeyboardEventsFromState[3];

/// Set all of the default key names. Should be called once before the Keys enum
/// is bound
void SetUpKeyNames();

/// Keyboard representing the physical keyboard.
class Keyboard : public ExplicitSingleton<Keyboard, EventObject>
{
public:
  ZilchDeclareType(Keyboard, TypeCopyMode::ReferenceType);

  Keyboard();

  enum InternalKeyState
  {
    KeyReleased,
    KeyPressed,
    KeyHeld,
    KeyNotHeld
  };

  /// Is any key in the 'Keys' enum down (not including 'Keys::Unknown', e.g.
  /// PrintScreen).
  bool IsAnyKeyDown();

  /// Excluding Ctrl, Shift, and Alt - is any key in the 'Keys' enum down
  /// (not including 'Keys::Unknown', e.g. PrintScreen).
  bool IsAnyNonModifierDown();

  /// Is the particular currently down.
  bool KeyIsDown(Keys::Enum key);

  /// Is the particular currently up.
  bool KeyIsUp(Keys::Enum key);

  /// Was the key pressed this frame.
  bool KeyIsPressed(Keys::Enum key);

  /// Was the key released this frame.
  bool KeyIsReleased(Keys::Enum key);

  /// Gets a string name of a particular key.
  String GetKeyName(Keys::Enum key);

  /// Validate that the key is a Keys::Enum that is not 'Unknown', or 'None', or
  /// an integer value that doesn't map to a known Keys::Enum value.
  bool Valid(Keys::Enum key);

  /// Validate that the input string can be mapped back to an enum.
  bool Valid(StringParam key);

  /// Convert key value to it's actual name or keyboard symbol, if it has one.
  /// Returns "Unknown" String if key is not found.
  String ToSymbol(Keys::Enum key);

  /// Convert a key name to it's keyboard symbol, if it has one.
  /// Returns input String if key is not found.
  String ToSymbol(StringParam keyName);

  /// Counterpart to 'ToSymbol'.  Converts a key's name or symbol to the key
  /// value. Returns Keys::Unknown if key is not found.
  Keys::Enum ToKey(StringParam key);

  void Update();
  void Clear();
  void UpdateKeys(KeyboardEvent& event);

  static Keyboard* Instance;
  uint mStateDownCount;
  byte States[Keys::KeyMax];
};

DeclareEnum3(KeyState, Up, Down, Repeated);

/// Represents information about key state changes.
class KeyboardEvent : public Event
{
public:
  ZilchDeclareType(KeyboardEvent, TypeCopyMode::ReferenceType);

  KeyboardEvent();

  void Serialize(Serializer& stream);

  /// Get the keyboard that generated this event.
  Keyboard* GetKeyboard();

  bool GetModifierPressed();

  /// Key that was changed.
  Keys::Enum Key;
  /// State of the key Up,Down,Repeated.
  KeyState::Enum State;
  /// Is Shift pressed with this key?
  bool ShiftPressed;
  /// Is Alt pressed with this key?
  bool AltPressed;
  /// Is Ctrl pressed with this key?
  bool CtrlPressed;
  /// Is Space pressed with this key?
  bool SpacePressed;

  /// Is the key handled
  bool Handled;

  /// Not for use in native code:
  /// Whether this message was handled by script
  bool HandledEventScript;

  /// The original OS key that was pressed (platform specific)
  uint OsKey;

  /// Keyboard for this event.
  Keyboard* mKeyboard;
};

// KeyboardTextEvent
/// Gives the actual key value being typed. For example, holding Shift + 'a'
/// will give 'A'.
class KeyboardTextEvent : public Event
{
public:
  ZilchDeclareType(KeyboardTextEvent, TypeCopyMode::ReferenceType);

  KeyboardTextEvent();

  KeyboardTextEvent(uint runeCode) : mRune(runeCode), mHandled(false){};

  void Serialize(Serializer& stream);

  Rune mRune;
  bool mHandled;
};

} // namespace Zero
