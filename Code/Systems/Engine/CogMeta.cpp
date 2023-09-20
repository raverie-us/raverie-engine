// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Serialization Filter
RaverieDefineType(CogSerializationFilter, builder, type)
{
}

bool CogSerializationFilter::ShouldSerialize(Object* object)
{
  bool isCog = !RaverieVirtualTypeId(object)->IsA(RaverieTypeId(Cog));
  ReturnIf(isCog, false, "Invalid object given for filter");

  return CogSerialization::ShouldSave(object);
}

// Cog Meta Operations
RaverieDefineType(CogMetaOperations, builder, type)
{
}

u64 CogMetaOperations::GetUndoHandleId(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();
  return cog->mObjectId.ToUint64();
}

Any CogMetaOperations::GetUndoData(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();
  ReturnIf(cog == nullptr, Any(), "Invalid Cog given.");

  // This could be a component on the game session, so we can't assume it has a
  // Space
  if (Space* space = cog->GetSpace())
  {
    return true; // Temporary until we fix issues with how this works
    // return space->GetModified();
  }

  // Doesn't matter what we return because we won't do anything with it when we
  // get it back
  return nullptr;
}

void CogMetaOperations::ObjectModified(HandleParam object, bool intermediateChange)
{
  Cog* cog = object.Get<Cog*>(GetOptions::AssertOnNull);
  if (Space* space = cog->GetSpace())
  {
    space->MarkModified();
    space->ChangedObjects();
  }

  if (!intermediateChange)
  {
    Cog* root = cog->FindRootArchetype();
    if (root && root->InArchetypeDefinitionMode())
    {
      if (!Archetype::sRebuilding)
      {
        root->UploadToArchetype();
        ArchetypeRebuilder::RebuildArchetypes(root->GetArchetype());
      }
    }
  }

  MetaOperations::ObjectModified(object, intermediateChange);
}

void CogMetaOperations::RestoreUndoData(HandleParam object, AnyParam undoData)
{
  Cog* cog = object.Get<Cog*>(GetOptions::AssertOnNull);

  // This could be a component on the game session, so we can't assume it has a
  // Space
  if (Space* space = cog->GetSpace())
  {
    bool wasSpaceModified = undoData.Get<bool>();
    if (wasSpaceModified == false)
      space->MarkNotModified();
    else
      space->MarkModified();
  }
}

ObjectRestoreState* CogMetaOperations::GetRestoreState(HandleParam object)
{
  Cog* cog = object.Get<Cog*>(GetOptions::AssertOnNull);
  return new CogRestoreState(cog->FindNearestArchetypeContext());
}

// Meta Data Inheritance
RaverieDefineType(CogMetaDataInheritance, builder, type)
{
}

String CogMetaDataInheritance::GetInheritId(HandleParam instance, InheritIdContext::Enum context)
{
  ReturnIf(!instance.StoredType->IsA(RaverieTypeId(Cog)), "", "Expected Cog");

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

void CogMetaDataInheritance::SetInheritId(HandleParam instance, StringParam inheritId)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);

  Archetype* archetype = ArchetypeManager::FindOrNull(inheritId);
  cog->SetArchetype(archetype);
}

Guid CogMetaDataInheritance::GetUniqueId(HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  Guid childId = cog->mChildId;
  if (childId == PolymorphicNode::cInvalidUniqueNodeId)
  {
    childId = GenerateUniqueId64();
    cog->mChildId = childId;
  }
  return childId;
}

void CogMetaDataInheritance::Revert(HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  cog->RevertToArchetype();
}

bool CogMetaDataInheritance::CanPropertyBeReverted(HandleParam, PropertyPathParam)
{
  return true;
}

void CogMetaDataInheritance::RevertProperty(HandleParam instance, PropertyPathParam propertyPath)
{
  MetaDataInheritance::RevertProperty(instance, propertyPath);

  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  if (Cog* root = cog->FindRootArchetype())
  {
    if (root->InArchetypeDefinitionMode())
    {
      root->UploadToArchetype();
      ArchetypeRebuilder::RebuildArchetypes(root->GetArchetype());
    }
    else
    {
      Cog* rootArchetype = cog->FindNearestArchetypeContext();
      ArchetypeRebuilder::RebuildCog(rootArchetype);
    }
  }
}

void CogMetaDataInheritance::RestoreRemovedChild(HandleParam parent, ObjectState::ChildId childId)
{
  MetaDataInheritance::RestoreRemovedChild(parent, childId);

  Cog* cog = parent.Get<Cog*>(GetOptions::AssertOnNull);
  if (Cog* root = cog->FindRootArchetype())
  {
    if (root->InArchetypeDefinitionMode())
    {
      root->UploadToArchetype();
      ArchetypeRebuilder::RebuildArchetypes(root->GetArchetype());
    }
    else
    {
      Cog* rootArchetype = cog->FindNearestArchetypeContext();
      ArchetypeRebuilder::RebuildCog(rootArchetype);
    }
  }
}

void CogMetaDataInheritance::SetPropertyModified(HandleParam instance, PropertyPathParam propertyPath, bool state)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);

  MetaDataInheritance::SetPropertyModified(instance, propertyPath, state);

  if (Space* space = cog->GetSpace())
    space->MarkModified();
}

void CogMetaDataInheritance::RebuildObject(HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>(GetOptions::AssertOnNull);
  ArchetypeRebuilder::RebuildCog(cog->FindNearestArchetypeContext());
}

// Cog Meta Transform
RaverieDefineType(CogMetaTransform, builder, type)
{
}

MetaTransformInstance CogMetaTransform::GetInstance(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();
  ReturnIf(cog == nullptr, MetaTransformInstance(), "Invalid object given");

  Handle instanceHandle = nullptr;
  BoundType* type = nullptr;

  // Try to get a transform or object link
  Transform* transform = cog->has(Transform);
  if (transform != nullptr)
  {
    instanceHandle = transform;
    type = RaverieTypeId(Transform);
  }
  else
  {
    ObjectLink* objectLink = cog->has(ObjectLink);
    if (objectLink != nullptr)
    {
      instanceHandle = objectLink;
      type = RaverieTypeId(ObjectLink);
    }
  }

  if (type == nullptr)
    return MetaTransformInstance();

  MetaTransformInstance instance(instanceHandle);
  instance.mSpace = cog->GetSpace();
  instance.mLocalTranslation = type->GetProperty("Translation");
  instance.mLocalRotation = type->GetProperty("Rotation");
  instance.mLocalScale = type->GetProperty("Scale");

  instance.mWorldTranslation = type->GetProperty("WorldTranslation");
  instance.mWorldRotation = type->GetProperty("WorldRotation");
  instance.mWorldScale = type->GetProperty("WorldScale");

  if (Cog* parent = cog->GetParent())
  {
    if (Transform* parentTransform = parent->has(Transform))
    {
      instance.mParentInstance = parentTransform;
      BoundType* t = RaverieTypeId(Transform);
      instance.mParentWorldMatrix = t->GetProperty("WorldMatrix");
      instance.mParentLocalMatrix = t->GetProperty("LocalMatrix");
    }
  }
  return instance;
}

// Archetype Extension
RaverieDefineType(CogArchetypeExtension, builder, type)
{
}

// Cog Meta Display
RaverieDefineType(CogMetaDisplay, builder, type)
{
}

String CogMetaDisplay::GetName(HandleParam object)
{
  Cog* cog = (Cog*)object.Dereference();

  if (!cog->GetName().Empty())
    return cog->GetName();

  if (Archetype* archetype = cog->GetArchetype())
    return String::Format("(%s)[%d]", archetype->Name.c_str(), cog->GetId().Id);
  else
    return String::Format("(%s)[%d]", RaverieTypeId(Cog)->Name.c_str(), cog->GetId().Id);
}

String CogMetaDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

// Cog Meta Display
bool CogMetaSerialization::sSaveContextIds = true;

RaverieDefineType(CogMetaSerialization, builder, type)
{
}

bool CogMetaSerialization::SerializeReferenceProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer)
{
  Cog* cog = value.Get<Cog*>();
  ReturnIf(cog == nullptr, false, "Cog should never be null here");

  CogId id = cog->GetId();
  serializer.SerializeFieldDefault(fieldName, id, CogId());
  return true;
}

void CogMetaSerialization::AddCustomAttributes(HandleParam object, TextSaver* saver)
{
  if (sSaveContextIds == false)
    return;

  static String sContextId = "ContextId";

  if (Cog* cog = object.Get<Cog*>())
  {
    CogSavingContext* context = static_cast<CogSavingContext*>(saver->GetSerializationContext());
    CogId id = cog->GetId();

    if (context && id != cInvalidCogId)
    {
      uint linkId = context->ToContextId(id.Id);
      saver->SaveAttribute(sContextId, ToString(linkId));
    }
  }
}

RaverieDefineType(CogArchetypePropertyFilter, builder, type)
{
}

bool CogArchetypePropertyFilter::Filter(Member* prop, HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>();
  if (cog->InArchetypeDefinitionMode())
    return false;
  return true;
}

} // namespace Raverie
