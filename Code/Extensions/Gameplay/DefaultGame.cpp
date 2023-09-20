// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(DefaultGameSetup, builder, type)
{
  RaverieBindComponent();
  RaverieBindDependency(GameSession);
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::CallSetDefaults);

  MetaEditorResource* editorResource = new MetaEditorResource();
  editorResource->FilterTag = RaverieTypeId(Space)->Name;
  RaverieBindGetterSetterProperty(StartingSpace)->Add(editorResource);

  RaverieBindGetterSetterProperty(StartingLevel);

  RaverieBindFieldProperty(mLoadEditingLevel);
}

DefaultGameSetup::DefaultGameSetup()
{
  mLoadEditingLevel = true;
}

void DefaultGameSetup::Serialize(Serializer& stream)
{
  SerializeResourceName(mStartingSpace, ArchetypeManager);
  SerializeResourceName(mStartingLevel, LevelManager);
  SerializeNameDefault(mLoadEditingLevel, true);
}

void DefaultGameSetup::SetDefaults()
{
  mStartingSpace = ArchetypeManager::Find(CoreArchetypes::Space);
  mStartingLevel = LevelManager::GetDefault();
  mLoadEditingLevel = true;
}

void DefaultGameSetup::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::GameSetup, OnSetup);
}

void DefaultGameSetup::OnSetup(GameEvent* event)
{
  GameSession* game = (GameSession*)this->GetOwner();
  Archetype* spaceArchetype = mStartingSpace;

  // Check to see if the starting space archetype is actually a space
  if (mStartingSpace->mStoredType != RaverieTypeId(Space))
  {
    // Space archetype has been removed or an invalid archetype has been
    // selected.
    DoNotifyError("Invalid Space Archetype",
                  "The space archetype that was"
                  " loaded is either missing or does not contain a Space.");
    spaceArchetype = ArchetypeManager::GetDefault();
  }

  // Create the space
  Space* gameSpace = game->CreateSpace(spaceArchetype);

  // just a safeguard in case null got returned somehow
  if (gameSpace == NULL)
    return;

  if (gameSpace->mName.Empty())
    gameSpace->SetName(SpecialCogNames::Main);

  event->mSpace = gameSpace;

  // begin level loading, default to the starting level
  Level* levelToLoad = mStartingLevel;

  // If the game is created inside the editor attempt
  // to load the level that is being edited
  if (Z::gRuntimeEditor && mLoadEditingLevel)
    levelToLoad = Z::gRuntimeEditor->GetEditingLevel();

  // If we have a valid level, load it
  if (levelToLoad)
    gameSpace->LoadLevelAdditive(levelToLoad);
}

} // namespace Raverie
