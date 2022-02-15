// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(KeyUp);
DefineEvent(KeyDown);
DefineEvent(KeyRepeated);
DefineEvent(TextTyped);
} // namespace Events

const String cKeyboardEventsFromState[] = {Events::KeyUp, Events::KeyDown, Events::KeyRepeated};

HashMap<String, Keys::Enum> KeyNameToEnum;
HashMap<String, String> KeyNameToSymbol;

void SetUpKeyNames()
{
  KeyNameToEnum["Unknown"] = Keys::Unknown;
  KeyNameToEnum["LeftBracket"] = Keys::LeftBracket;
  KeyNameToEnum["["] = Keys::LeftBracket;
  KeyNameToEnum["RightBracket"] = Keys::RightBracket;
  KeyNameToEnum["]"] = Keys::RightBracket;
  KeyNameToEnum["Comma"] = Keys::Comma;
  KeyNameToEnum[","] = Keys::Comma;
  KeyNameToEnum["Period"] = Keys::Period;
  KeyNameToEnum["."] = Keys::Period;
  KeyNameToEnum["Semicolon"] = Keys::Semicolon;
  KeyNameToEnum[";"] = Keys::Semicolon;
  KeyNameToEnum["Space"] = Keys::Space;
  KeyNameToEnum["Spacebar"] = Keys::Space;
  KeyNameToEnum["Equal"] = Keys::Equal;
  KeyNameToEnum["="] = Keys::Equal;
  KeyNameToEnum["Minus"] = Keys::Minus;
  KeyNameToEnum["-"] = Keys::Minus;
  KeyNameToEnum["Apostrophe"] = Keys::Apostrophe;
  KeyNameToEnum["'"] = Keys::Apostrophe;

  KeyNameToSymbol["LeftBracket"] = "[";
  KeyNameToSymbol["RightBracket"] = "]";
  KeyNameToSymbol["Comma"] = ",";
  KeyNameToSymbol["Period"] = ".";
  KeyNameToSymbol["Semicolon"] = ";";
  KeyNameToSymbol["Space"] = "Spacebar";
  KeyNameToSymbol["Equal"] = "=";
  KeyNameToSymbol["Minus"] = "-";
  KeyNameToSymbol["Apostrophe"] = "'";

  KeyNameToEnum["Up"] = Keys::Up;
  KeyNameToEnum["Down"] = Keys::Down;
  KeyNameToEnum["Left"] = Keys::Left;
  KeyNameToEnum["Right"] = Keys::Right;

  KeyNameToEnum["F1"] = Keys::F1;
  KeyNameToEnum["F2"] = Keys::F2;
  KeyNameToEnum["F3"] = Keys::F3;
  KeyNameToEnum["F4"] = Keys::F4;
  KeyNameToEnum["F5"] = Keys::F5;
  KeyNameToEnum["F6"] = Keys::F6;
  KeyNameToEnum["F7"] = Keys::F7;
  KeyNameToEnum["F8"] = Keys::F8;
  KeyNameToEnum["F9"] = Keys::F9;
  KeyNameToEnum["F10"] = Keys::F10;
  KeyNameToEnum["F11"] = Keys::F11;
  KeyNameToEnum["F12"] = Keys::F12;

  KeyNameToEnum["Insert"] = Keys::Insert;
  KeyNameToEnum["Delete"] = Keys::Delete;
  KeyNameToEnum["Back"] = Keys::Back;
  KeyNameToEnum["BackSpace"] = Keys::Back;
  KeyNameToEnum["Home"] = Keys::Home;
  KeyNameToEnum["End"] = Keys::End;
  KeyNameToEnum["Tilde"] = Keys::Tilde;
  KeyNameToEnum["~"] = Keys::Tilde;
  KeyNameToEnum["Slash"] = Keys::Slash;
  KeyNameToEnum["/"] = Keys::Slash;
  KeyNameToEnum["Backslash"] = Keys::Backslash;
  KeyNameToEnum["\\"] = Keys::Backslash;
  KeyNameToEnum["Tab"] = Keys::Tab;
  KeyNameToEnum["Shift"] = Keys::Shift;
  KeyNameToEnum["Alt"] = Keys::Alt;
  KeyNameToEnum["Control"] = Keys::Control;
  KeyNameToEnum["Ctrl"] = Keys::Control;
  KeyNameToEnum["Capital"] = Keys::Capital;
  KeyNameToEnum["CapsLock"] = Keys::Capital;
  KeyNameToEnum["Enter"] = Keys::Enter;
  KeyNameToEnum["Escape"] = Keys::Escape;
  KeyNameToEnum["Esc"] = Keys::Escape;
  KeyNameToEnum["PageUp"] = Keys::PageUp;
  KeyNameToEnum["PageDown"] = Keys::PageDown;

  KeyNameToSymbol["Del"] = "Delete";
  KeyNameToSymbol["Back"] = "Backspace";
  KeyNameToSymbol["Tilde"] = "~";
  KeyNameToSymbol["Slash"] = "/";
  KeyNameToSymbol["Backslash"] = "\\";
  KeyNameToSymbol["Control"] = "Ctrl";
  KeyNameToSymbol["Capital"] = "CapsLock";
  KeyNameToSymbol["Escape"] = "Esc";

  KeyNameToEnum["NumPad0"] = Keys::NumPad0;
  KeyNameToEnum["NumPad1"] = Keys::NumPad1;
  KeyNameToEnum["NumPad2"] = Keys::NumPad2;
  KeyNameToEnum["NumPad3"] = Keys::NumPad3;
  KeyNameToEnum["NumPad4"] = Keys::NumPad4;
  KeyNameToEnum["NumPad5"] = Keys::NumPad5;
  KeyNameToEnum["NumPad6"] = Keys::NumPad6;
  KeyNameToEnum["NumPad7"] = Keys::NumPad7;
  KeyNameToEnum["NumPad8"] = Keys::NumPad8;
  KeyNameToEnum["NumPad9"] = Keys::NumPad9;

  KeyNameToEnum["Add"] = Keys::Add;
  KeyNameToEnum["NumPadPlus"] = Keys::Add;
  KeyNameToEnum["Multiply"] = Keys::Multiply;
  KeyNameToEnum["NumPadMultiply"] = Keys::Multiply;
  KeyNameToEnum["Subtract"] = Keys::Subtract;
  KeyNameToEnum["NumPadMinus"] = Keys::Subtract;
  KeyNameToEnum["Divide"] = Keys::Divide;
  KeyNameToEnum["NumPadDivide"] = Keys::Divide;
  KeyNameToEnum["Decimal"] = Keys::Decimal;
  KeyNameToEnum["NumPadDecimal"] = Keys::Decimal;

  KeyNameToSymbol["Add"] = "NumPadPlus";
  KeyNameToSymbol["Multiply"] = "NumPadMultiply";
  KeyNameToSymbol["Subtract"] = "NumPadMinus";
  KeyNameToSymbol["Divide"] = "NumPadDivide";
  KeyNameToSymbol["Decimal"] = "NumPadDecimal";

  KeyNameToEnum["A"] = Keys::A;
  KeyNameToEnum["B"] = Keys::B;
  KeyNameToEnum["C"] = Keys::C;
  KeyNameToEnum["D"] = Keys::D;
  KeyNameToEnum["E"] = Keys::E;
  KeyNameToEnum["F"] = Keys::F;
  KeyNameToEnum["G"] = Keys::G;
  KeyNameToEnum["H"] = Keys::H;
  KeyNameToEnum["I"] = Keys::I;
  KeyNameToEnum["J"] = Keys::J;
  KeyNameToEnum["K"] = Keys::K;
  KeyNameToEnum["L"] = Keys::L;
  KeyNameToEnum["M"] = Keys::M;
  KeyNameToEnum["N"] = Keys::N;
  KeyNameToEnum["O"] = Keys::O;
  KeyNameToEnum["P"] = Keys::P;
  KeyNameToEnum["Q"] = Keys::Q;
  KeyNameToEnum["R"] = Keys::R;
  KeyNameToEnum["S"] = Keys::S;
  KeyNameToEnum["T"] = Keys::T;
  KeyNameToEnum["U"] = Keys::U;
  KeyNameToEnum["V"] = Keys::V;
  KeyNameToEnum["W"] = Keys::W;
  KeyNameToEnum["Y"] = Keys::Y;
  KeyNameToEnum["X"] = Keys::X;
  KeyNameToEnum["Z"] = Keys::Z;

  KeyNameToEnum["0"] = Keys::Num0;
  KeyNameToEnum["1"] = Keys::Num1;
  KeyNameToEnum["2"] = Keys::Num2;
  KeyNameToEnum["3"] = Keys::Num3;
  KeyNameToEnum["4"] = Keys::Num4;
  KeyNameToEnum["5"] = Keys::Num5;
  KeyNameToEnum["6"] = Keys::Num6;
  KeyNameToEnum["7"] = Keys::Num7;
  KeyNameToEnum["8"] = Keys::Num8;
  KeyNameToEnum["9"] = Keys::Num9;

  KeyNameToEnum["Zero"] = Keys::Num0;
  KeyNameToEnum["One"] = Keys::Num1;
  KeyNameToEnum["Two"] = Keys::Num2;
  KeyNameToEnum["Three"] = Keys::Num3;
  KeyNameToEnum["Four"] = Keys::Num4;
  KeyNameToEnum["Five"] = Keys::Num5;
  KeyNameToEnum["Six"] = Keys::Num6;
  KeyNameToEnum["Seven"] = Keys::Num7;
  KeyNameToEnum["Eight"] = Keys::Num8;
  KeyNameToEnum["Nine"] = Keys::Num9;

  KeyNameToEnum["None"] = Keys::None;
}

Keyboard* Keyboard::Instance = nullptr;

ZilchDefineType(Keyboard, builder, type)
{
  ZeroBindDocumented();
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindMethod(IsAnyKeyDown);
  ZilchBindMethod(IsAnyNonModifierDown);
  ZilchBindMethod(KeyIsDown);
  ZilchBindMethod(KeyIsUp);
  ZilchBindMethod(KeyIsPressed);
  ZilchBindMethod(KeyIsReleased);

  ZilchBindMethod(GetKeyName);

  ZilchBindOverloadedMethod(Valid, ZilchInstanceOverload(bool, Keys::Enum));
  ZilchBindOverloadedMethod(Valid, ZilchInstanceOverload(bool, StringParam));

  ZilchBindMethod(ToKey);
  ZilchBindOverloadedMethod(ToSymbol, ZilchInstanceOverload(String, Keys::Enum));
  ZilchBindOverloadedMethod(ToSymbol, ZilchInstanceOverload(String, StringParam));

  ZeroBindEvent(Events::KeyUp, KeyboardEvent);
  ZeroBindEvent(Events::KeyDown, KeyboardEvent);
  ZeroBindEvent(Events::KeyRepeated, KeyboardEvent);
  ZeroBindEvent(Events::TextTyped, KeyboardTextEvent);
}

Keyboard::Keyboard()
{
  Keyboard::Instance = this;
  Clear();
}

bool Keyboard::IsAnyKeyDown()
{
  return mStateDownCount > 0;
}

bool Keyboard::IsAnyNonModifierDown()
{
  uint modifierCount = KeyIsDown(Keys::Control);
  modifierCount += KeyIsDown(Keys::Shift);
  modifierCount += KeyIsDown(Keys::Alt);

  return (mStateDownCount != 0 && modifierCount != mStateDownCount);
}

bool Keyboard::KeyIsDown(Keys::Enum key)
{
  return States[key] == KeyHeld || States[key] == KeyPressed;
}

bool Keyboard::KeyIsUp(Keys::Enum key)
{
  return States[key] == KeyNotHeld || States[key] == KeyReleased;
}

bool Keyboard::KeyIsPressed(Keys::Enum key)
{
  return States[key] == KeyPressed;
}

bool Keyboard::KeyIsReleased(Keys::Enum key)
{
  return States[key] == KeyReleased;
}

String Keyboard::GetKeyName(Keys::Enum key)
{
  if (key < Keys::KeyMax && KeyNames[key])
    return KeyNames[key];
  else
    return String();
}

bool Keyboard::Valid(Keys::Enum key)
{
  bool isValid = (key == Keys::Space) || (key == Keys::LeftBracket) || (key == Keys::RightBracket) ||
                 (key == Keys::Comma) || (key == Keys::Period) || (key == Keys::Semicolon) || (key == Keys::Minus) ||
                 (key == Keys::Apostrophe) || (key == Keys::Slash) || (key == Keys::Backslash) ||
                 (key >= Keys::A && key <= Keys::Z) || (key >= Keys::Num0 && key <= Keys::Num9) ||
                 (key >= Keys::Up && key < Keys::None);

  return isValid;
}

bool Keyboard::Valid(StringParam key)
{
  if (key == "Unknown" || key == "None" || key == "KeyMax" || key == "Size")
    return false;
  else
    return ToKey(key) != Keys::Unknown;
}

String Keyboard::ToSymbol(Keys::Enum key)
{
  if (!Valid(key))
    return "Unknown";

  if (key <= Keys::Backslash)
  {
    StringBuilder buffer;
    buffer.Append(Rune(key));

    return buffer.ToString();
  }

  String symbol = KeyNameToSymbol.FindValue(KeyNames[key], String());
  if (!symbol.Empty())
    return symbol;

  return GetKeyName(key);
}

String Keyboard::ToSymbol(StringParam keyName)
{
  return KeyNameToSymbol.FindValue(keyName, keyName);
}

Keys::Enum Keyboard::ToKey(StringParam key)
{
  return KeyNameToEnum.FindValue(key, Keys::Unknown);
}

void Keyboard::Update()
{
  for (uint i = 0; i < Keys::KeyMax; ++i)
  {
    byte& state = States[i];
    if (state == KeyPressed)
      state = KeyHeld;
    else if (state == KeyReleased)
    {
      state = KeyNotHeld;

      if (i != Keys::Unknown)
        --mStateDownCount;
    }
  }
}

void Keyboard::Clear()
{
  for (uint i = 0; i < Keys::KeyMax; ++i)
  {
    byte& state = States[i];
    state = KeyNotHeld;
  }

  mStateDownCount = 0;
}

void Keyboard::UpdateKeys(KeyboardEvent& event)
{
  // Key is repeated do nothing
  if (event.State == KeyState::Repeated)
    return;

  // Update the internal key state
  byte& state = States[event.Key];
  if (event.State == KeyState::Down)
  {
    switch (state)
    {
    case KeyReleased:
    case KeyNotHeld:
      state = KeyPressed;
      if (event.Key != Keys::Unknown)
        ++mStateDownCount;
      break;
    case KeyPressed:
      state = KeyHeld;
      break;
    case KeyHeld:
      break;
    }
  }
  else
  {
    if (state == KeyReleased)
    {
      state = KeyNotHeld;
    }
    else if (state != KeyNotHeld)
    {
      state = KeyReleased;
    }
  }
}

ZilchDefineType(KeyboardEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(Key);
  ZilchBindFieldProperty(State);
  ZilchBindFieldProperty(ShiftPressed);
  ZilchBindFieldProperty(AltPressed);
  ZilchBindFieldProperty(CtrlPressed);
  ZilchBindFieldProperty(SpacePressed);
  ZilchBindFieldPropertyAs(HandledEventScript, "HandledEvent");
  ZilchBindGetterProperty(Keyboard);
  ZilchBindFieldProperty(OsKey);
  ZilchBindGetterProperty(ModifierPressed);
}

KeyboardEvent::KeyboardEvent()
{
  Handled = false;
  HandledEventScript = false;
  OsKey = 0;
}

void KeyboardEvent::Serialize(Serializer& stream)
{
  SerializeNameDefault(EventId, String());
  SerializeNameDefault(ShiftPressed, false);
  SerializeNameDefault(AltPressed, false);
  SerializeNameDefault(CtrlPressed, false);
  SerializeNameDefault(SpacePressed, false);
  SerializeNameDefault(OsKey, 0U);
  stream.SerializeFieldDefault<int>("Key", *(int*)&Key, (int)Keys::Unknown);

  SerializeEnumNameDefault(KeyState, State, KeyState::Up);
}

Keyboard* KeyboardEvent::GetKeyboard()
{
  return mKeyboard;
}

bool KeyboardEvent::GetModifierPressed()
{
  return CtrlPressed || ShiftPressed || AltPressed;
}

// KeyboardTextEvent
ZilchDefineType(KeyboardTextEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(mRune);
}

KeyboardTextEvent::KeyboardTextEvent() : mRune(Rune::Invalid), mHandled(false)
{
}

void KeyboardTextEvent::Serialize(Serializer& stream)
{
  SerializeNameDefault(EventId, String());
  stream.SerializeFieldDefault("Rune", mRune.value, 0u);
}

} // namespace Zero
