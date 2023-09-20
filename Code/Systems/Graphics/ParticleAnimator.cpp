// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ParticleAnimator, builder, type)
{
  RaverieBindDocumented();
  RaverieBindTag(Tags::Particle);
}

ParticleAnimator::ParticleAnimator() : mGraphicsSpace(nullptr)
{
}

void ParticleAnimator::Initialize(CogInitializer& initializer)
{
  mGraphicsSpace = GetSpace()->has(GraphicsSpace);
}

} // namespace Raverie
