// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Meta Data Inheritance
RaverieDefineType(ComponentMetaDataInheritance, builder, type)
{
}

void ComponentMetaDataInheritance::Revert(HandleParam object)
{
  Component* component = object.Get<Component*>();
  Cog* owner = component->GetOwner();

  // Only retain override properties on the root
  bool retainOverrideProperties = false;
  if (owner == owner->FindRootArchetype())
    retainOverrideProperties = true;

  LocalModifications::GetInstance()->ClearModifications(object, true, retainOverrideProperties);
}

bool ComponentMetaDataInheritance::CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath)
{
  return true;
}

void ComponentMetaDataInheritance::RevertProperty(HandleParam object, PropertyPathParam propertyPath)
{
  MetaDataInheritance::RevertProperty(object, propertyPath);

  Component* component = object.Get<Component*>();
  Cog* owner = component->GetOwner();
  if (Cog* root = owner->FindRootArchetype())
  {
    if (root->InArchetypeDefinitionMode())
    {
      root->UploadToArchetype();
      ArchetypeRebuilder::RebuildArchetypes(root->GetArchetype());
    }
    else
    {
      // We only need to rebuild the nearest context, not the actual root
      Cog* rootContext = owner->FindNearestArchetypeContext();
      ArchetypeRebuilder::RebuildCog(rootContext);
    }
  }
}

void ComponentMetaDataInheritance::SetPropertyModified(HandleParam object, PropertyPathParam propertyPath, bool state)
{
  MetaDataInheritance::SetPropertyModified(object, propertyPath, state);

  Component* component = object.Get<Component*>();
  Space* space = component->GetSpace();
  space->MarkModified();
  space->ChangedObjects();
}

void ComponentMetaDataInheritance::RebuildObject(HandleParam object)
{
  Component* component = object.Get<Component*>();
  Cog* cog = component->GetOwner();
  RaverieTypeId(Cog)->HasInherited<MetaDataInheritance>()->RebuildObject(cog);
}

// Component Meta Operations
RaverieDefineType(ComponentMetaOperations, builder, type)
{
}

u64 ComponentMetaOperations::GetUndoHandleId(HandleParam object)
{
  Component* component = object.Get<Component*>();
  u64 cogId = component->GetOwner()->mObjectId.Id;
  u64 componentHash = RaverieVirtualTypeId(component)->Name.Hash();
  return (cogId << 32) | componentHash;
}

Any ComponentMetaOperations::GetUndoData(HandleParam object)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, Any(), "Invalid Component given.");

  MetaOperations* cogOperations = RaverieTypeId(Cog)->HasInherited<MetaOperations>();
  return cogOperations->GetUndoData(component->GetOwner());
}

void ComponentMetaOperations::ObjectModified(HandleParam object, bool intermediateChange)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, , "Invalid Component given");

  // This could be a component on the game session
  if (Space* space = component->GetSpace())
  {
    space->MarkModified();
    space->ChangedObjects();
  }

  if (!intermediateChange)
  {
    Cog* root = component->GetOwner()->FindRootArchetype();
    if (root && root->InArchetypeDefinitionMode())
    {
      if (!Archetype::sRebuilding)
      {
        root->UploadToArchetype();
        ArchetypeRebuilder::RebuildArchetypes(root->GetArchetype(), root);
      }
    }
  }

  MetaOperations::ObjectModified(object, intermediateChange);
}

void ComponentMetaOperations::RestoreUndoData(HandleParam object, AnyParam undoData)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, , "Invalid Component given.");

  MetaOperations* cogOperations = RaverieTypeId(Cog)->HasInherited<MetaOperations>();
  cogOperations->RestoreUndoData(component->GetOwner(), undoData);
}

ObjectRestoreState* ComponentMetaOperations::GetRestoreState(HandleParam object)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, nullptr, "Invalid Component given.");

  MetaOperations* cogOperations = RaverieTypeId(Cog)->HasInherited<MetaOperations>();
  return cogOperations->GetRestoreState(component->GetOwner());
}

} // namespace Raverie
