///////////////////////////////////////////////////////////////////////////////
///
/// \file Particle.hpp
/// Declaration of the Particle classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Tags
{
  DeclareTag(Particle);
}

/// The particle Contains the position, size, color,
/// and other properties of any individual particle.
class Particle
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Particle* Next;
  float Time;
  float Lifetime;
  float Size;
  float Rotation;
  float RotationalVelocity;
  Vec3 Position;
  Vec3 Velocity;
  Vec4 Color;
  float WanderAngle;
};

/// This class manages a linked-list of particles.
class ParticleList
{
public:
  Particle* AllocateParticle();
  void FreeParticle(Particle* particle);

  void DestroyParticle(Particle* particle);
  void ClearDestroyed();

  void AddParticle(Particle* particle);
  void FreeParticles();

  struct range
  {
    typedef Particle* value_type;
    typedef Particle*& FrontResult;

    range() : mCurrentParticle(nullptr), mEndParticle(nullptr) {}
    range(Particle* curr, Particle* endParticle = nullptr)
    {
      mCurrentParticle = curr;
      mEndParticle = endParticle;
    }

    void PopFront(){mCurrentParticle = mCurrentParticle->Next;}
    FrontResult Front(){return mCurrentParticle;}
    bool Empty(){return mCurrentParticle == mEndParticle;}
    Particle* mCurrentParticle;
    Particle* mEndParticle;
  };

  range All()
  {
    return range(Particles);
  }

  static Memory::Pool* Memory;
  Particle* Particles;
  Particle* Destroyed;

  uint mActiveParticles;
  void Initialize();
};

typedef ParticleList::range ParticleListRange;

} // namespace Zero
