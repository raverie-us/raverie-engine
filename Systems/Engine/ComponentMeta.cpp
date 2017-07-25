////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ Component Meta Data Inheritance
//**************************************************************************************************
ZilchDefineType(ComponentMetaDataInheritance, builder, type)
{
}

//**************************************************************************************************
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

//**************************************************************************************************
bool ComponentMetaDataInheritance::CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath)
{
  return true;
}

//**************************************************************************************************
void ComponentMetaDataInheritance::RevertProperty(HandleParam object, PropertyPathParam propertyPath)
{
  MetaDataInheritance::RevertProperty(object, propertyPath);

  Component* component = object.Get<Component*>();
  Cog* owner = component->GetOwner();
  if (Cog* rootArchetype = owner->FindRootArchetype())
    ArchetypeRebuilder::RebuildCog(rootArchetype);
}

//**************************************************************************************************
void ComponentMetaDataInheritance::SetPropertyModified(HandleParam object, PropertyPathParam propertyPath, bool state)
{
  MetaDataInheritance::SetPropertyModified(object, propertyPath, state);

  Component* component = object.Get<Component*>();
  Space* space = component->GetSpace();
  space->MarkModified();
  space->ChangedObjects();
}

//**************************************************************************************************
void ComponentMetaDataInheritance::RebuildObject(HandleParam object)
{
  Component* component = object.Get<Component*>();
  Cog* cog = component->GetOwner();
  ZilchTypeId(Cog)->HasInherited<MetaDataInheritance>()->RebuildObject(cog);
}

//------------------------------------------------------------------------ Component Meta Operations
//**************************************************************************************************
ZilchDefineType(ComponentMetaOperations, builder, type)
{
}

//**************************************************************************************************
u64 ComponentMetaOperations::GetUndoHandleId(HandleParam object)
{
  Component* component = object.Get<Component*>();
  u64 cogId = component->GetOwner()->mObjectId.Id;
  u64 componentHash = ZilchVirtualTypeId(component)->Name.Hash();
  return (cogId << 32) | componentHash;
}

//**************************************************************************************************
Any ComponentMetaOperations::GetUndoData(HandleParam object)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, Any(), "Invalid Component given.");

  // This could be a component on the game session
  if (Space* space = component->GetSpace())
  {
    bool isModified = space->GetModified();
    //return isModified;
    return true; // Temporary until we fix issues with how this works
  }

  // Doesn't matter what we return because we won't do anything with it when we get it back
  return nullptr;
}

//**************************************************************************************************
void ComponentMetaOperations::ObjectModified(HandleParam object)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, , "Invalid Component given");

  // This could be a component on the game session
  if (Space* space = component->GetSpace())
  {
    space->MarkModified();
    space->ChangedObjects();
  }
  MetaOperations::ObjectModified(object);
}

//**************************************************************************************************
void ComponentMetaOperations::RestoreUndoData(HandleParam object, AnyParam undoData)
{
  Component* component = object.Get<Component*>();
  ReturnIf(component == nullptr, , "Invalid Component given.");

  // This could be a component on the game session
  if (Space* space = component->GetSpace())
  {
    bool wasSpaceModified = undoData.Get<bool>();
    if (wasSpaceModified)
      space->MarkModified();
    else
      space->MarkNotModified();
  }
}

//**************************************************************************************************
ObjectRestoreState* ComponentMetaOperations::GetRestoreState(HandleParam object)
{
  Component* component = object.Get<Component*>();
  Cog* cog = component->GetOwner();
  MetaOperations* cogOperations = ZilchTypeId(Cog)->HasInherited<MetaOperations>();
  return cogOperations->GetRestoreState(cog);
}

}//namespace Zero
