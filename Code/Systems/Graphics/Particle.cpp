// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Tags
{
DefineTag(Particle);
}

//////////Helper functions///////////////
void PushFront(Particle*& front, Particle* particle)
{
  particle->Next = front;
  front = particle;
}

RaverieDefineType(Particle, builder, type)
{
  type->HandleManager = RaverieManagerId(PointerManager);
  RaverieBindFieldProperty(Time);
  RaverieBindFieldProperty(Lifetime);
  RaverieBindFieldProperty(Size);
  RaverieBindFieldProperty(Rotation);
  RaverieBindFieldProperty(RotationalVelocity);
  RaverieBindFieldProperty(Position);
  RaverieBindFieldProperty(Velocity);
  RaverieBindFieldProperty(Color);
  RaverieBindFieldProperty(WanderAngle);
}

Memory::Pool* ParticleList::Memory = NULL;

void ParticleList::Initialize()
{
  Particles = NULL;
  Destroyed = NULL;
  mActiveParticles = 0;
}

void ParticleList::AddParticle(Particle* particle)
{
  ++mActiveParticles;
  PushFront(Particles, particle);
}

Particle* ParticleList::AllocateParticle()
{
  return (Particle*)Memory->Allocate(sizeof(Particle));
}

void ParticleList::FreeParticle(Particle* particle)
{
  --mActiveParticles;
  Memory->Deallocate(particle, sizeof(Particle));
}

void ParticleList::DestroyParticle(Particle* particle)
{
  PushFront(Destroyed, particle);
}

void ParticleList::ClearDestroyed()
{
  Particle* curParticle = Destroyed;
  while (curParticle != NULL)
  {
    Particle* nextParticle = curParticle->Next;
    FreeParticle(curParticle);
    curParticle = nextParticle;
  }
  Destroyed = NULL;
}

void ParticleList::FreeParticles()
{
  Particle* curParticle = Particles;
  while (curParticle != NULL)
  {
    Particle* nextParticle = curParticle->Next;
    FreeParticle(curParticle);
    curParticle = nextParticle;
  }
  Particles = NULL;
  ErrorIf(Destroyed != 0, "Particle on destroy.");
  ErrorIf(mActiveParticles != 0, "Particle system memory error.");
}

} // namespace Raverie
