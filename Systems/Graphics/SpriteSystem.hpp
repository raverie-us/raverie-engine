///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteSystem.hpp
/// Declaration of the Particle Rendering component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Sprite Particle System Enums
DeclareEnum5(SpriteParticleGeometryMode, Billboarded, Beam, Outward, FaceVelocity, Flat);
DeclareEnum9(SpriteParticleSortMode, None, BackToFrontView, FrontToBackView, NegativeToPositiveX, PositiveToNegativeX, NegativeToPositiveY, PositiveToNegativeY, NegativeToPositiveZ, PositiveToNegativeZ);
DeclareEnum2(SpriteParticleAnimationMode, Single, Looping);

/// A particle system that uses sprites to represent each particle.
class SpriteParticleSystem : public ParticleSystem
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Graphical Interface

  String GetDefaultMaterialName() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;

  // Properties

  /// Color attribute of the generated vertices accessible in the vertex shader, value is multiplied with the particle color.
  Vec4 mVertexColor;

  /// How the geometry of the particles are generated.
  SpriteParticleGeometryMode::Enum mGeometryMode;

  /// The sprite definition to use for each particle.
  HandleOf<SpriteSource> mSpriteSource;

  /// How the sprite's animation should be used.
  SpriteParticleAnimationMode::Enum mParticleAnimation;

  /// How particles should be sorted with each other, determines draw order between particles.
  SpriteParticleSortMode::Enum mParticleSort;

  /// How much to scale particles along their direction of movement.
  float mBeamBaseScale;

  /// How much additional scale to add to particles by how fast they are moving.
  float mBeamVelocityScale;

  // Internal

  void CheckSort(ViewBlock& viewBlock);
};

} // namespace Zero
