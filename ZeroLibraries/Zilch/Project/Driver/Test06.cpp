#include "Precompiled.hpp"
using namespace Zilch;
class Animal;

class Brain
{
public:
  int Thoughts;
  float ThinkSpeed;

  Brain()
  {
    this->Thoughts = 1000;
    this->ThinkSpeed = 1.0f;
  }

  void SuperCharge(Animal* parent);

  void ProcessThoughts(float time)
  {
    this->Thoughts += (Integer)(this->ThinkSpeed * time);
    this->ThinkSpeed *= 0.9f;
  }
};

class Animal
{
public:
  int Lives;
  float Health;
  bool IsOnGround;

  Brain LoclMemberBrain;
  Brain* HeapMemberBrain;

  Animal()
  {
    this->Lives = 9;
    this->Health = 100.0f;
    this->IsOnGround = false;
    this->HeapMemberBrain = new Brain();
  }

  ~Animal()
  {
    delete this->HeapMemberBrain;
  }

  Animal* Jump()
  {
    this->IsOnGround = false;
    return this;
  }

  void TakeDamage(float damage)
  {
    this->Health -= damage;

    this->LoclMemberBrain.SuperCharge(this);
    this->HeapMemberBrain->SuperCharge(this);

    if (this->Health < 0.0)
    {
      --this->Lives;
      this->Health = 100.0f;
    }
  }

  void OneUp()
  {
    ++this->Lives;
  }

  float GetTotalHealth()
  {
    float totalHealth = (this->Lives * 100.0f + this->Health);
    return totalHealth;
  }
};

void Brain::SuperCharge(Animal* parent)
{
  this->ThinkSpeed *= 40.0f;
  parent->OneUp();
}

int Test06()
{
  Animal* locl = new Animal();
  Animal* heap = new Animal();

  int val = 3;

  if (locl == heap)
  {
    ++val;
  }

  if (locl != heap)
  {
    val += 9;
  }

  AutoDeclare(takeDamageDelegate, &Animal::TakeDamage);

  for (int i = 0; i < 10; ++i)
  {
    (locl->*takeDamageDelegate)(51.235f);
    (heap->*takeDamageDelegate)(65.653f);
  }

  locl->Jump()->Jump()->Jump();
  heap->Jump()->Jump()->Jump();

  AutoDeclare(processThoughtsDelegate, &Brain::ProcessThoughts);

  (locl->LoclMemberBrain.*processThoughtsDelegate)(0.23f);
  (locl->HeapMemberBrain->*processThoughtsDelegate)(0.20f);
  (heap->LoclMemberBrain.*processThoughtsDelegate)(0.37f);
  (heap->HeapMemberBrain->*processThoughtsDelegate)(0.89f);

  locl->LoclMemberBrain.SuperCharge(heap);
  locl->HeapMemberBrain->SuperCharge(heap);
  heap->LoclMemberBrain.SuperCharge(heap);
  heap->HeapMemberBrain->SuperCharge(heap);

  Integer result = ((Integer)(locl->GetTotalHealth() + heap->GetTotalHealth())) +
    locl->LoclMemberBrain.Thoughts +
    locl->HeapMemberBrain->Thoughts +
    heap->LoclMemberBrain.Thoughts +
    heap->HeapMemberBrain->Thoughts;
  
  delete locl;
  delete heap;

  return result + val;
}
