// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ParticleAnimator, builder, type)
{
  ZeroBindDocumented();
  ZeroBindTag(Tags::Particle);
}

ParticleAnimator::ParticleAnimator() : mGraphicsSpace(nullptr)
{
}

void ParticleAnimator::Initialize(CogInitializer& initializer)
{
  mGraphicsSpace = GetSpace()->has(GraphicsSpace);
}

} // namespace Zero
