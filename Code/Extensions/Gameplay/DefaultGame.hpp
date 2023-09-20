// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Create the starting space and load the starting level into that space
class DefaultGameSetup : public Component
{
public:
  RaverieDeclareType(DefaultGameSetup, TypeCopyMode::ReferenceType);

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

  // If set to true StartingLevel will be overridden by the currently edited
  // level
  bool mLoadEditingLevel;
};

} // namespace Raverie
