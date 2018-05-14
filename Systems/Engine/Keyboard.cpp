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
  ZilchBindGetterProperty(ModififierPressed);
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

bool KeyboardEvent::GetModififierPressed()
{
  return CtrlPressed || ShiftPressed || AltPressed;
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

void KeyboardTextEvent::Serialize(Serializer& stream)
{
  SerializeNameDefault(EventId, String());
  stream.SerializeFieldDefault("Rune", mRune.value, 0u);
}

}//namespace Zero
