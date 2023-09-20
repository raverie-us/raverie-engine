// MIT Licensed (see LICENSE.md).
#pragma once
#include "DataSource.hpp"

namespace Raverie
{

// Forward Declarations.
class MetaSelection;
class CogRestoreState;

// Cog Replace Event
namespace Events
{
// Sent on the space when an Archetype object is rebuilt
DeclareEvent(CogReplaced);
} // namespace Events

/// Sent on the Space when an object is rebuilt.
class CogReplaceEvent : public DataReplaceEvent
{
public:
  RaverieDeclareType(CogReplaceEvent, TypeCopyMode::ReferenceType);
  CogReplaceEvent(Cog* oldCog, Cog* newCog);
  Cog* mOldCog;
  Cog* mNewCog;
};

// Archetype Rebuilder
/// To rebuild an Archetype, we serialize the object to a string, create a new
/// object from that string, then destroy the old object.
/// The new object is then updated in all Selections and is updated in the
/// operation map.
class ArchetypeRebuilder
{
public:
  // Rebuild all archetypes in all GameSessions.
  static void RebuildArchetypes(Archetype* modifiedArchetype, Cog* ignore = nullptr, Array<CogRestoreState*>* restoreStates = nullptr);
  static Cog* RebuildCog(Cog* cog);
  static Cog* RebuildCog(Cog* cog, HashSet<MetaSelection*>* modifiedSelections);
};

} // namespace Raverie
