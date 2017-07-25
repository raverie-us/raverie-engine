////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------- Cog Serialization Filter
//**************************************************************************************************
ZilchDefineType(CogSerializationFilter, builder, type)
{
}

//**************************************************************************************************
bool CogSerializationFilter::ShouldSerialize(Object* object)
{
  ReturnIf(!ZilchVirtualTypeId(object)->IsA(ZilchTypeId(Cog)), false, "Invalid object given for filter");

  return CogSerialization::ShouldSave(object);
}

//------------------------------------------------------------------------------ Cog Meta Operations
//**************************************************************************************************
ZilchDefineType(CogMetaOperations, builder, type)
{
}

//**************************************************************************************************
u64 CogMetaOperations::GetUndoHandleId(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();
  return cog->mObjectId.ToUint64();
}

//**************************************************************************************************
Any CogMetaOperations::GetUndoData(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();
  ReturnIf(cog == nullptr, Any(), "Invalid Cog given.");

  if (Space* space = cog->GetSpace())
    return true; // Temporary until we fix issues with how this works
    //return space->GetModified();

  // Doesn't matter what we return because we won't do anything with it when we get it back
  return nullptr;
}

//**************************************************************************************************
void CogMetaOperations::ObjectModified(HandleParam object)
{
  Cog* cog = object.Get<Cog*>(GetOptions::AssertOnNull);
  if (Space* space = cog->GetSpace())
  {
    space->MarkModified();
    space->ChangedObjects();
  }
  MetaOperations::ObjectModified(object);
}

//**************************************************************************************************
void CogMetaOperations::RestoreUndoData(HandleParam object, AnyParam undoData)
{
  Cog* cog = object.Get<Cog*>(GetOptions::AssertOnNull);
  if (Space* space = cog->GetSpace())
  {
    bool wasSpaceModified = undoData.Get<bool>();
    if (wasSpaceModified == false)
      space->MarkNotModified();
  }
}

//**************************************************************************************************
ObjectRestoreState* CogMetaOperations::GetRestoreState(HandleParam object)
{
  Cog* cog = object.Get<Cog*>(GetOptions::AssertOnNull);
  return new CogRestoreState(cog->FindNearestArchetypeContext());
}

//------------------------------------------------------------------------ Cog Meta Data Inheritance
//**************************************************************************************************
ZilchDefineType(CogMetaDataInheritance, builder, type)
{
}

//**************************************************************************************************
String CogMetaDataInheritance::GetInheritId(HandleParam instance, InheritIdContext::Enum context)
{
  ReturnIf(!instance.StoredType->IsA(ZilchTypeId(Cog)), "", "Expected Cog");

  Cog* cog = instance.Get<Cog*>();
  if (Archetype* archetype = cog->GetArchetype())
  {
    if (context == InheritIdContext::Instance)
      return archetype->ResourceIdName;
    else
      return archetype->mBaseResourceIdName;
  }

  return String();
}

//**************************************************************************************************
void CogMetaDataInheritance::SetInheritId(HandleParam instance, StringParam inheritId)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);

  Archetype* archetype = ArchetypeManager::FindOrNull(inheritId);
  cog->SetArchetype(archetype);
}

//**************************************************************************************************
Guid CogMetaDataInheritance::GetUniqueId(HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  return cog->mChildId;
}

//**************************************************************************************************
void CogMetaDataInheritance::Revert(HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  cog->RevertToArchetype();
}

//**************************************************************************************************
bool CogMetaDataInheritance::CanPropertyBeReverted(HandleParam, PropertyPathParam)
{
  return true;
}

//**************************************************************************************************
void CogMetaDataInheritance::RevertProperty(HandleParam instance, PropertyPathParam propertyPath)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);

  MetaDataInheritance::RevertProperty(instance, propertyPath);

  if (Cog* rootArchetype = cog->FindNearestArchetypeContext())
    ArchetypeRebuilder::RebuildCog(rootArchetype);
}

//**************************************************************************************************
void CogMetaDataInheritance::SetPropertyModified(HandleParam instance, PropertyPathParam propertyPath,
  bool state)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);

  MetaDataInheritance::SetPropertyModified(instance, propertyPath, state);

  if (Space* space = cog->GetSpace())
    space->MarkModified();
}

//**************************************************************************************************
void CogMetaDataInheritance::RebuildObject(HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  ArchetypeRebuilder::RebuildCog(cog->FindNearestArchetypeContext());
}

//------------------------------------------------------------------------------- Cog Meta Transform
//**************************************************************************************************
ZilchDefineType(CogMetaTransform, builder, type)
{
}

//**************************************************************************************************
MetaTransformInstance CogMetaTransform::GetInstance(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();
  ReturnIf(cog == nullptr, MetaTransformInstance(), "Invalid object given");

  Transform* transform = cog->has(Transform);
  if (transform == nullptr)
    return MetaTransformInstance();

  BoundType* t = ZilchTypeId(Transform);

  MetaTransformInstance instance(transform);
  instance.mSpace = cog->GetSpace();
  instance.mLocalTranslation = t->GetProperty("Translation");
  instance.mLocalRotation = t->GetProperty("Rotation");
  instance.mLocalScale = t->GetProperty("Scale");

  instance.mWorldTranslation = t->GetProperty("WorldTranslation");
  instance.mWorldRotation = t->GetProperty("WorldRotation");
  instance.mWorldScale = t->GetProperty("WorldScale");

  if (Cog* parent = cog->GetParent())
  {
    if (Transform* parentTransform = parent->has(Transform))
    {
      instance.mParentInstance = parentTransform;
      instance.mParentWorldMatrix = t->GetProperty("WorldMatrix");
    }
  }

  return instance;
}

//-------------------------------------------------------------------------- Cog Archetype Extension
ZilchDefineType(CogArchetypeExtension, builder, type)
{
}

//--------------------------------------------------------------------------------- Cog Meta Display
ZilchDefineType(CogMetaDisplay, builder, type)
{
}

//**************************************************************************************************
String CogMetaDisplay::GetName(HandleParam object)
{
  Cog* cog = (Cog*)object.Dereference();

  if(!cog->GetName().Empty( ))
    return cog->GetName( );

  if(Archetype* archetype = cog->GetArchetype( ))
    return String::Format("(%s)[%d]", archetype->Name.c_str( ), cog->GetId( ).Id);
  else
    return String::Format("(%s)[%d]", ZilchTypeId(Cog)->Name.c_str( ), cog->GetId( ).Id);
}

//**************************************************************************************************
String CogMetaDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

}//namespace Zero
