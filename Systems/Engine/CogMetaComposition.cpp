////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \file MetaComposition.cpp
/// Implementation of the Meta Class for composition class.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(CogMetaComposition, builder, type)
{
}

//----------------------------------------------------------------------------- Cog Meta Composition
//**************************************************************************************************
CogMetaComposition::CogMetaComposition() : MetaComposition(ZilchTypeId(Component))
{
}

//**************************************************************************************************
uint CogMetaComposition::GetComponentCount(HandleParam owner)
{
  Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
  return cog->mComponents.Size();
}

//**************************************************************************************************
Handle CogMetaComposition::GetComponent(HandleParam owner, BoundType* typeId)
{
  Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
  if(Component* component = cog->QueryComponentType(typeId))
    return component;

  return Handle();
}

//**************************************************************************************************
Handle CogMetaComposition::GetComponentAt(HandleParam owner, uint index)
{
  Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
  return cog->mComponents[index];
}

//**************************************************************************************************
bool CogMetaComposition::CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info)
{
  // We cannot add a Transform if our parent doesn't have a transform
  if(typeToAdd == ZilchTypeId(Transform))
  {
    Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
    if(cog->GetParent() && !cog->GetParent()->has(Transform))
    {
      if(info)
        info->Reason = "Cannot add Transform to an object who's parent doesn't have a Transform.";
      return false;
    }
  }

  return MetaComposition::CanAddComponent(owner, typeToAdd, info);
}

//**************************************************************************************************
Handle CogMetaComposition::MakeObject(BoundType* typeToCreate)
{
  Component* component = ZilchAllocate(Component, typeToCreate);

  // Call SetDefaults or default serialize the object
  SetUpObject(component);

  return component;
}

//**************************************************************************************************
BoundType* CogMetaComposition::MakeProxy(StringParam typeName, ProxyReason::Enum reason)
{
  return ProxyObject<Component>::CreateProxyMetaFromFile(typeName, reason);
}

//**************************************************************************************************
void CogMetaComposition::AddComponent(HandleParam owner, HandleParam componentToAdd, int index,
                                      bool ignoreDependencies)
{
  Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);

  BoundType* componentType = componentToAdd.StoredType;
  Component* component = componentToAdd.Get<Component*>();

  // If for some reason we still failed to create
  // the component then just return an empty object
  ReturnIf(component == nullptr, , "Invalid Component given");

  // Originally we never set the owner on the component prior to serialization, but certain features
  // such as CogPath require knowing the owning object
  // We need to make sure the user cannot access the owner from script during the serialization phase
  component->mOwner = cog;

  // Add the Component
  if(ignoreDependencies)
    cog->ForceAddComponent(component, index);
  else
    cog->AddComponent(component, index);

  // Initialize the Component
  CogInitializer init(cog->GetSpace(), cog->GetGameSession());
  init.mParent = cog;
  component->ScriptInitialize(init);

  // No objects were created, so instead of calling AllCreated, just send the AllObjectsInitialized
  // event
  init.SendAllObjectsInitialized();

  if(Space* space = cog->GetSpace())
    space->ChangedObjects();
}

//**************************************************************************************************
bool CogMetaComposition::CanRemoveComponent(HandleParam owner, HandleParam component, String& reason)
{
  BoundType* typeToRemove = component.StoredType;

  //The child transform's TransformParent* will not be fixed and be a dangling pointer.
  if(typeToRemove->IsA(ZilchTypeId(Transform)))
  {
    // Can only remove it if we have no children with a Transform
    Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
    forRange(Cog& child, cog->GetChildren())
    {
      if(child.has(Transform))
      {
        reason = "Cannot remove the Transform from an object who's children have a Transform.";
        return false;
      }
    }
  }

  return MetaComposition::CanRemoveComponent(owner, component, reason);
}

//**************************************************************************************************
void CogMetaComposition::RemoveComponent(HandleParam owner, HandleParam component,
                                         bool ignoreDependencies)
{
  String reason;
  ErrorIf(CanRemoveComponent(owner, component, reason) == false, reason.c_str());

  // Remove the component
  Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
  Component* componentToRemove = component.Get<Component*>(GetOptions::AssertOnNull);

  ErrorIf(componentToRemove->GetOwner() != cog, "Component is not on the given Cog");

  if(ignoreDependencies)
    cog->ForceRemoveComponent(componentToRemove);
  else
    cog->RemoveComponent(componentToRemove);
};

//**************************************************************************************************
void CogMetaComposition::MoveComponent(HandleParam owner, HandleParam component, uint destination)
{
  String reason;
  Handle blocking;
  ErrorIf(CanMoveComponent(owner, component, destination, blocking, reason) == false, reason.c_str());

  uint componentIndex = GetComponentIndex(owner, component);
  Cog* cog = owner.Get<Cog*>(GetOptions::AssertOnNull);
  cog->MoveComponentBefore(componentIndex, destination);
}

//METAREFACTOR Take care of this stuff (meta components)
////******************************************************************************
//void MetaSerializeCog(MetaObjectInstance owningInstance, MetaType* propertyMeta, MetaProperty* property, 
//                      Serializer& serializer)
//{
//  ObjPtr instance = owningInstance.ObjectPtr;
//
//  cstr fieldName = property->PropertyName.c_str();
//  if(serializer.GetMode() == SerializerMode::Saving)
//  {
//    // Saving a cog save it as a stream Id uint
//    Variant variant = property->GetValue(instance);
//    
//    // Get the saving context and the stream id.
//    CogSavingContext* context = static_cast<CogSavingContext*>(serializer.GetSerializationContext());
//
//    // By default write an empty id
//    u32 contextId = 0;
//
//    if(variant.Is<uint>())
//    {
//      // Value has not yet been Restore save as id.
//      contextId = variant.As<uint>();
//    }
//    else
//    {
//      // Check to see if it is valid object 
//      if(Cog* object = variant.AsRef<Cog>())
//      {
//        //If there is a saving context convert the id to a Save Id
//        if(context != NULL && context->CurrentContextMode==ContextMode::Saving)
//          contextId = context->ToContextId(object->GetId().Id);
//        else
//          contextId = object->GetId().Id;
//      }
//    }
//
//    //Serialize the id
//    Serialization::Policy<uint>::Serialize(serializer, fieldName, contextId);
//  }
//  else
//  {
//    // Loading reads the stream id and sets it as an int
//    // Reference will be restored in OnAllObjectsCreated
//    TypeIdType typeId = property->TypeId;
//    MetaType* metaType = MetaDatabase::GetInstance()->FindType(typeId);
//    if(metaType)
//    {
//      CogCreationContext* context = static_cast<CogCreationContext*>(serializer.GetSerializationContext());
//
//      u32 contextId = 0;
//      Serialization::Policy<uint>::Serialize(serializer, fieldName, contextId);
//
//      Variant variant = contextId;
//      property->SetValue(instance, &variant);
//    }
//  }
//}
//
//******************************************************************************
//bool PropertyModifiedOnComponent(MetaType* meta, MetaProperty* property, 
//                                 ObjPtr instance)
//{
//  Component* component = (Component*)instance;
//
//  //LocalObjectModifications* modifications = LocalObjectModifications::GetInstance();
//  //PropertyPath path(component, property);
//  return modifications->IsPropertyModified(component->GetOwner(), path);
//}
//
////******************************************************************************
//void MetaSerializeComponentObject(MetaObjectInstance object, 
//                                 Serializer& serializer)
//{
//  Component* component = (Component*)object.ObjectPtr;
//  component->Serialize(serializer);
//}
//
////******************************************************************************
//bool CogShouldStoreLocalModifications(MetaObjectInstance instance)
//{
//  Cog* cog = GetCogFromUndoObject(instance);
//  ReturnIf(cog == nullptr, false, "Invalid object given.");
//  
//  if(cog->GetArchetype() || cog->IsChildOfArchetype())
//    return true;
//  return false;
//}

//void CogSetChildOrderOverride(ObjPtr instance)
//{
//  Cog* cog = (Cog*)instance;
//  LocalModifications* modifications = LocalModifications::GetInstance();
//  modifications->SetChildOrderModified(cog, true);
//}

}//namespace Zero
