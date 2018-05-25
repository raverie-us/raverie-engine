///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
}

extern const String cKeyboardEventsFromState[3];

//-------------------------------------------------------------------Keyboard
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

  void Update();
  void Clear();
  void UpdateKeys(KeyboardEvent& event);
  static Keyboard* Instance;

private:
  byte States[Keys::KeyMax];
};


//--------------------------------------------------------------- Keyboard Event

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

  bool GetModififierPressed();

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

//------------------------------------------------------------------- KeyboardTextEvent
/// Gives the actual key value being typed. For example, holding Shift + 'a' will give 'A'.
class KeyboardTextEvent : public Event
{
public:
  ZilchDeclareType(KeyboardTextEvent, TypeCopyMode::ReferenceType);

  KeyboardTextEvent();

  KeyboardTextEvent(uint runeCode)
    : mRune(runeCode), mHandled(false) {};

  void Serialize(Serializer& stream);

  Rune mRune;
  bool mHandled;
};

}//namespace Zero
