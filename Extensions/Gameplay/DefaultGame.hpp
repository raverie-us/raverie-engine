///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Create the starting space and load the starting level into that space
class DefaultGameSetup : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DefaultGameSetup();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void SetDefaults() override;

  // Space to create for the game.
  ResourceProperty(Archetype, StartingSpace);

  // Level to load into the game.
  ResourceProperty(Level, StartingLevel);

  void OnSetup(GameEvent* event);

  // If set to true StartingLevel will be overridden by the currently edited level
  bool mLoadEditingLevel;
};

} // namespace Zero
