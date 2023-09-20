// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
namespace Events
{
DeclareEvent(AllObjectsInitialized);
}

class GameSession;
class Level;
typedef InList<Cog, &Cog::SpaceLink> SpaceCogList;

class CogInitializerEvent : public Event
{
public:
  RaverieDeclareType(CogInitializerEvent, TypeCopyMode::ReferenceType);

  CogInitializer* mCogInitializer;
};

/// Cog initialization data. Structure that is passed in to
/// initialize the component and cog.
class CogInitializer : public SafeId32EventObject
{
public:
  RaverieDeclareType(CogInitializer, TypeCopyMode::ReferenceType);

  CogInitializer(Space* space, GameSession* gameSession = nullptr);
  ~CogInitializer();

  void Initialize(Space* space);

  SpaceCogList::range AllCreated();
  void SendAllObjectsInitialized();

  uint Flags;

  /// Dispatches an event to anyone listening on the CogInitializer
  /// This is useful for controlling initialization order and phases
  void DispatchEvent(StringParam eventId, Event* event);

  Space* GetSpace()
  {
    return mSpace;
  }
  Cog* GetParent()
  {
    return mParent;
  }
  GameSession* GetGameSession()
  {
    return mGameSession;
  }

  // The GameSession the object is created in.
  GameSession* mGameSession;

  // The space the object is created in.
  Space* mSpace;

  // The parent in the Game Object Hierarchy.
  Cog* mParent;

  // Level resource create from.
  String mLevel;

  // Objects being added
  SpaceCogList CreationList;
  uint AddCount;

  friend class Space;

  // Creation Context for loading object
  // relationships.
  CogCreationContext* Context;
};

} // namespace Raverie
