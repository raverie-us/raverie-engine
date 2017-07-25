#include "Wallaby.hpp"

LivingBeing::LivingBeing()
{
  printf("Constructed LivingBeing\n");
}

LivingBeing::~LivingBeing()
{
  printf("Destructed LivingBeing\n");
}

void LivingBeing::Speak() const
{
  printf("Hello! I have %d lives...\n", this->Lives);
}

void LivingBeing::Yell(float volume) const
{
  if (volume > 80.0f)
    printf("HEY YOU!");
  else if (volume > 50.0f)
    printf("Hey You!");
  else
    printf("Hey you");
}

void LivingBeing::Yell() const
{
  this->Yell(100.0f);
}

int LivingBeing::ComputeLives(float mana, int level)
{
  return (int)(mana * level);
}

float LivingBeing::GetHealth() const
{
  return this->InternalHealth;
}

void LivingBeing::SetHealth(float value)
{
  if (value > 100.0f)
    value = 100.0f;
  if (value < 0.0f)
    value = 0.0f;

  this->InternalHealth = value;
}

Player::Player(const String& name, float startingHealth) :
  Name(name)
{
  printf("Constructed Player\n");
  this->SetHealth(startingHealth);
}

Player::~Player()
{
  printf("Destructed Player\n");
}

void Player::Speak() const
{
  printf("I am %s, hear me roar!\n", this->Name.c_str());
}
