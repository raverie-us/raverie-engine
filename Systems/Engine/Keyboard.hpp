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
extern cstr KeyNames[];

namespace Keys
{
  // KeyCodes for keys A - Z same as capital ASCII characters 'A' - 'Z'
  // KeyCodes for keys 0 - 9 same as ASCII number characters '0' - '9'

  enum Enum
  {
  // Key not mapped
    Unknown = 0,

  // Letters 
    A = 'A',
    B = 'B',
    C = 'C',
    D = 'D',
    E = 'E',
    F = 'F',
    G = 'G',
    H = 'H',
    I = 'I',
    J = 'J',
    K = 'K',
    L = 'L',
    M = 'M',
    N = 'N',
    O = 'O',
    P = 'P',
    Q = 'Q',
    R = 'R',
    S = 'S',
    T = 'T',
    U = 'U',
    V = 'V',
    W = 'W',
    Y = 'Y',
    X = 'X',
    Z = 'Z',

    Space = ' ',

  //Numbers
    Num0 = '0',
    Num1 = '1',
    Num2 = '2',
    Num3 = '3',
    Num4 = '4',
    Num5 = '5',
    Num6 = '6',
    Num7 = '7',
    Num8 = '8',
    Num9 = '9',

  //Symbols
    LeftBracket = '[',
    RightBracket = ']',
    Comma = ',',
    Period = '.',
    Semicolon = ';',
    Minus = '-',
    Apostrophe = '\'',
    Slash = '/',
    Backslash = '\\',

  //Arrow Keys
    Up = 128,
    Down,
    Left,
    Right,

  //Function Keys
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

  //Special Keys
    Delete,
    Back,
    Home,
    End,
    Tilde,
    Tab,
    Shift,
    Alt,
    Control,
    Capital,
    Enter,
    Escape,
    PageUp,
    PageDown,
    Equal,

  //Num Pad
    NumPad0,
    NumPad1,
    NumPad2,
    NumPad3,
    NumPad4,
    NumPad5,
    NumPad6,
    NumPad7,
    NumPad8,
    NumPad9,
    Add,
    Multiply,
    Subtract,
    Divide,
    Decimal,

    None,

    KeyMax,
    Size
  };
}//namespace KeyCodes

/// Set all of the default key names. Should be called once before the Keys enum is bound
void SetUpKeyNames();

//-------------------------------------------------------------------Keyboard
/// Keyboard representing the physical keyboard.
class Keyboard : public ExplicitSingleton<Keyboard, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  KeyboardEvent();

  void Serialize(Serializer& stream);

  /// Get the keyboard that generated this event.
  Keyboard* GetKeyboard();

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

  /// Is they key handled
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
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  KeyboardTextEvent();

  KeyboardTextEvent(uint runeCode)
    : mRune(runeCode), mHandled(false) {};

  Rune mRune;
  bool mHandled;
};

}//namespace Zero
