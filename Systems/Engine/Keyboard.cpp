///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(KeyUp);
  DefineEvent(KeyDown);
  DefineEvent(KeyRepeated);
  DefineEvent(TextTyped);
}

const String cKeyboardEventsFromState[] = 
{
  Events::KeyUp,
  Events::KeyDown, 
  Events::KeyRepeated 
};

cstr KeyNames[Keys::KeyMax + 1] = {0};

#define SetKeyName(value) KeyNames[Keys::value] = #value;
#define SetKeyNameLiteral(value, name) KeyNames[value] = name;

void SetUpKeyNames()
{
  SetKeyName(Unknown);
  SetKeyName(LeftBracket);
  SetKeyName(RightBracket);
  SetKeyName(Comma);
  SetKeyName(Period);
  SetKeyName(Semicolon);
  SetKeyName(Space);
  SetKeyName(Equal);
  SetKeyName(Minus);

  SetKeyName(Apostrophe);

  SetKeyName(Up);
  SetKeyName(Down);
  SetKeyName(Left);
  SetKeyName(Right);

  SetKeyName(F1);
  SetKeyName(F2);
  SetKeyName(F3);
  SetKeyName(F4);
  SetKeyName(F5);
  SetKeyName(F6);
  SetKeyName(F7);
  SetKeyName(F8);
  SetKeyName(F9);
  SetKeyName(F10);
  SetKeyName(F11);
  SetKeyName(F12);
  SetKeyName(Delete);
  SetKeyName(Back);
  SetKeyName(Home);
  SetKeyName(End);
  SetKeyName(Tilde);
  SetKeyName(Slash);
  SetKeyName(Backslash);
  SetKeyName(Tab);
  SetKeyName(Shift);
  SetKeyName(Alt);
  SetKeyName(Control);
  SetKeyName(Capital);
  SetKeyName(Enter);
  SetKeyName(Escape);
  SetKeyName(PageUp);
  SetKeyName(PageDown);

  SetKeyName(NumPad0);
  SetKeyName(NumPad1);
  SetKeyName(NumPad2);
  SetKeyName(NumPad3);
  SetKeyName(NumPad4);
  SetKeyName(NumPad5);
  SetKeyName(NumPad6);
  SetKeyName(NumPad7);
  SetKeyName(NumPad8);
  SetKeyName(NumPad9);

  SetKeyName(Add);
  SetKeyName(Multiply);
  SetKeyName(Subtract);
  SetKeyName(Divide);
  SetKeyName(Decimal);

  SetKeyName(A);
  SetKeyName(B);
  SetKeyName(C);
  SetKeyName(D);
  SetKeyName(E);
  SetKeyName(F);
  SetKeyName(G);
  SetKeyName(H);
  SetKeyName(I);
  SetKeyName(J);
  SetKeyName(K);
  SetKeyName(L);
  SetKeyName(M);
  SetKeyName(N);
  SetKeyName(O);
  SetKeyName(P);
  SetKeyName(Q);
  SetKeyName(R);
  SetKeyName(S);
  SetKeyName(T);
  SetKeyName(U);
  SetKeyName(V);
  SetKeyName(W);
  SetKeyName(Y);
  SetKeyName(X);
  SetKeyName(Z);

  SetKeyNameLiteral('0', "Zero");
  SetKeyNameLiteral('1', "One");
  SetKeyNameLiteral('2', "Two");
  SetKeyNameLiteral('3', "Three");
  SetKeyNameLiteral('4', "Four");
  SetKeyNameLiteral('5', "Five");
  SetKeyNameLiteral('6', "Six");
  SetKeyNameLiteral('7', "Seven");
  SetKeyNameLiteral('8', "Eight");
  SetKeyNameLiteral('9', "Nine");

  SetKeyName(None);
}


//-------------------------------------------------------------------Keyboard
Keyboard* Keyboard::Instance = nullptr;

ZilchDefineType(Keyboard, builder, type)
{
  ZeroBindDocumented();
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindMethod(KeyIsDown);
  ZilchBindMethod(KeyIsUp);
  ZilchBindMethod(KeyIsPressed);
  ZilchBindMethod(KeyIsReleased);
  ZilchBindMethod(GetKeyName);
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
  if(key < Keys::KeyMax && KeyNames[key])
    return KeyNames[key];
  else
    return String();
}

void Keyboard::Update()
{
  for(uint i = 0; i < Keys::KeyMax; ++i)
  {
    byte& state = States[i];
    if(state == KeyPressed)
      state = KeyHeld;
    else if(state == KeyReleased)
      state = KeyNotHeld;
  }
}

void Keyboard::Clear()
{
  for(uint i = 0; i < Keys::KeyMax; ++i)
  {
    byte& state = States[i];
    state = KeyNotHeld;
  }
}

void Keyboard::UpdateKeys(KeyboardEvent& event)
{
  // Key is repeated do nothing
  if(event.State == KeyState::Repeated)
    return;

  // Update the internal key state
  byte& state = States[event.Key];
  if(event.State == KeyState::Down)
  {
    switch(state)
    {
    case KeyReleased:
    case KeyNotHeld:
      state = KeyPressed;
      break;
    case KeyPressed: state = KeyHeld; break;
    case KeyHeld: break;
    }
  }
  else
  {
    if(state == KeyReleased)
      state = KeyNotHeld;
    else if(state != KeyNotHeld)
      state = KeyReleased;
  }
}

//--------------------------------------------------------------- KeyboardEvent
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
}

KeyboardEvent::KeyboardEvent()
{
  Handled = false;
  HandledEventScript = false;
  OsKey = 0;
}

void KeyboardEvent::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    stream.StartPolymorphic(ZilchTypeId(KeyboardEvent));

    SerializeNameDefault(ShiftPressed, false);
    SerializeNameDefault(AltPressed, false);
    SerializeNameDefault(CtrlPressed, false);
    SerializeNameDefault(SpacePressed, false);
    SerializeNameDefault(OsKey, 0U);
    stream.SerializeFieldDefault<int>("Key", *(int*)&Key, (int)Keys::Unknown);

    SerializeEnumNameDefault(KeyState, State, (KeyState::Enum)KeyState::Size);

    stream.EndPolymorphic();
  }
  else
  {
    SerializeNameDefault(ShiftPressed, false);
    SerializeNameDefault(AltPressed, false);
    SerializeNameDefault(CtrlPressed, false);
    SerializeNameDefault(SpacePressed, false);
    SerializeNameDefault(OsKey, 0U);
    stream.SerializeFieldDefault<int>("Key", *(int*)&Key, (int)Keys::Unknown);

    SerializeEnumNameDefault(KeyState, State, (KeyState::Enum)KeyState::Size);
  }
}

Keyboard* KeyboardEvent::GetKeyboard()
{
  return mKeyboard;
}

//--------------------------------------------------------------- KeyboardTextEvent
ZilchDefineType(KeyboardTextEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(mRune);
}

KeyboardTextEvent::KeyboardTextEvent()
  : mRune(Rune::Invalid), mHandled(false)
{
}

}//namespace Zero
