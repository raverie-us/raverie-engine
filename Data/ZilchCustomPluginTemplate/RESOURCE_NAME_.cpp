#include "RESOURCE_NAME_Precompiled.hpp"

//***************************************************************************
ZilchDefineType(RESOURCE_NAME_, builder, type)
{
  // This is required for component binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  ZilchBindMethod(Initialize);
  
  // Note: All event connection methods must be bound
  ZilchBindMethod(OnLogicUpdate);

  // Using Property at the end is the same as the [Property] attribute
  // You could also use ->AddAttribute after the bind macro
  ZilchBindMethod(Speak);
  ZilchBindFieldProperty(mLives);
  ZilchBindGetterSetterProperty(Health);
}

//***************************************************************************
RESOURCE_NAME_::RESOURCE_NAME_()
{
  Zilch::Console::WriteLine("RESOURCE_NAME_::RESOURCE_NAME_ (Constructor)");
  // Initialize our default values here (we automatically zero the memory first)
  // In the future we'll support a newer compiler with member initialization
  mHealth = 100.0f;
  mLives = 9;
}

//***************************************************************************
RESOURCE_NAME_::~RESOURCE_NAME_()
{
  Zilch::Console::WriteLine("RESOURCE_NAME_::~RESOURCE_NAME_ (Destructor)");
  // Always check for null if you are intending
  // to destroy any cogs that you 'own'
}

//***************************************************************************
void RESOURCE_NAME_::Initialize(ZeroEngine::CogInitializer* initializer)
{
  Zilch::Console::WriteLine("RESOURCE_NAME_::Initialize");
  
  ZeroConnectThisTo(this->GetSpace(), "LogicUpdate", "OnLogicUpdate");
}

//***************************************************************************
void RESOURCE_NAME_::OnLogicUpdate(ZeroEngine::UpdateEvent* event)
{
  // Do we have a Model component?
  ZeroEngine::Model* model = this->GetOwner()->has(ZeroEngine::Model);
  if (model != nullptr)
    Zilch::Console::WriteLine("We have a Model!");
  
  // Send our own update event
  // We could also replace this with ZilchEvent to send basic events
  // Note: ZilchAllocate should be used for any type that is
  // typically allocated within Zilch, such as a CastFilter
  Zilch::HandleOf<RESOURCE_NAME_Event> toSend = ZilchAllocate(RESOURCE_NAME_Event);
  toSend->mLives = mLives;
  this->GetOwner()->DispatchEvent("RESOURCE_NAME_Update", toSend);
}

//***************************************************************************
Zilch::String RESOURCE_NAME_::Speak()
{
  Zilch::String text("Hello World");
  Zilch::Console::WriteLine(text);
  return text;
}

//***************************************************************************
float RESOURCE_NAME_::GetHealth()
{
  return mHealth;
}

//***************************************************************************
void RESOURCE_NAME_::SetHealth(float value)
{
  if (value < 0)
    value = 0;
  else if (value > 100)
    value = 100;
  
  mHealth = value;
}

//***************************************************************************
ZilchDefineType(RESOURCE_NAME_Event, builder, type)
{
  // This is required for event binding
  ZilchBindDestructor();
  ZilchBindConstructor();
  
  ZilchBindFieldProperty(mLives);
}
