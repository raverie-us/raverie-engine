// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Particle Animator Interface. Particle Animators effect particles in the
/// system.
class ParticleAnimator : public Component
{
public:
  RaverieDeclareType(ParticleAnimator, TypeCopyMode::ReferenceType);

  ParticleAnimator();

  void Initialize(CogInitializer& initializer) override;

  // Particle Animator Interface
  virtual void Animate(ParticleList* particleList, float dt, Mat4Ref transform) = 0;

  Link<ParticleAnimator> link;
  GraphicsSpace* mGraphicsSpace;
};

typedef InList<ParticleAnimator> AnimatorList;

} // namespace Raverie
