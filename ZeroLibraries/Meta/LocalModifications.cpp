////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \file LocalModifications.cpp
///
/// Authors: Joshua Claeys
/// Copyright 2015-2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ObjectContext
{
DefineStringConstant(Instance);
}

ZilchDefineType(MetaDataInheritance, builder, type)
{
}

ZilchDefineType(MetaDataInheritanceRoot, builder, type)
{
}

//**************************************************************************************************
bool IsValidForStorage(HandleParam object)
{
  BoundType* type = object.StoredType;
  if(type == nullptr)
    return false;

  if(!type->HasAttributeInherited(ObjectAttributes::cStoreLocalModifications))
    return false;
  if(MetaDataInheritance* inheritance = type->HasInherited<MetaDataInheritance>())
    return inheritance->ShouldStoreLocalModifications(object);
  return false;
}

//----------------------------------------------------------------------------------------- Child Id
//**************************************************************************************************
ObjectState::ChildId::ChildId(StringParam typeName, Guid id) :
  mTypeName(typeName),
  mId(id)
{

}

//**************************************************************************************************
ObjectState::ChildId::ChildId(BoundType* type, Guid id) :
  mTypeName(type->Name),
  mId(id)
{
  
}

//**************************************************************************************************
ObjectState::ChildId::ChildId(HandleParam object) :
  mId(cInvalidUniqueId)
{
  BoundType* type = object.StoredType;
  mTypeName = type->Name;

  if(MetaDataInheritance* inheritance = type->HasInherited<MetaDataInheritance>())
    mId = inheritance->GetUniqueId(object);
}

//**************************************************************************************************
ObjectState::ChildId::ChildId(Object* object) :
  mId(cInvalidUniqueId)
{
  BoundType* type = ZilchVirtualTypeId(object);
  mTypeName = type->Name;

  if(MetaDataInheritance* inheritance = type->HasInherited<MetaDataInheritance>())
    mId = inheritance->GetUniqueId(object);
}

//**************************************************************************************************
size_t ObjectState::ChildId::Hash() const
{
  return mId.Hash() ^ mTypeName.Hash();
}

//**************************************************************************************************
bool ObjectState::ChildId::operator==(const ChildId& rhs) const
{
  return mTypeName == rhs.mTypeName && mId == rhs.mId;
}

//------------------------------------------------------------------------------------- Object State
//**************************************************************************************************
ObjectState::ObjectState() :
  mChildOrderModified(false)
{

}

//**************************************************************************************************
ObjectState* ObjectState::Clone()
{
  return new ObjectState(*this);
}

//**************************************************************************************************
void ObjectState::Combine(ObjectState* state)
{
  // Copy properties
  forRange(PropertyPath& propertyPath, state->GetModifiedProperties())
    mModifiedProperties.Insert(propertyPath);

  // Copy Child modifications
  forRange(ChildId& addedChild, state->GetAddedChildren())
    mAddedChildren.Insert(addedChild);

  forRange(ChildId& removedChild, state->GetRemovedChildren())
    mRemovedChildren.Insert(removedChild);

  // Child order
  mChildOrderModified |= state->mChildOrderModified;
}

//**************************************************************************************************
bool ObjectState::IsModified()
{
  return IsSelfModified() || AreChildrenModified();
}

//**************************************************************************************************
bool ObjectState::IsModified(HandleParam object, bool ignoreOverrideProperties)
{
  return IsSelfModified(object, ignoreOverrideProperties) || AreChildrenModified();
}

//**************************************************************************************************
bool ObjectState::IsSelfModified()
{
  return !mModifiedProperties.Empty();
}

//**************************************************************************************************
bool ObjectState::IsSelfModified(HandleParam object, bool ignoreOverrideProperties)
{
  // Call the default if we're not ignoring override properties
  if(!ignoreOverrideProperties)
    return IsSelfModified();

  // Walk through and try to find any non-override properties
  forRange(PropertyPath& propertyPath, mModifiedProperties.All())
  {
    Property* property = propertyPath.GetPropertyFromRoot(object);
    if(property && !property->HasAttribute(PropertyAttributes::cLocalModificationOverride))
      return true;
  }

  return false;
}

//**************************************************************************************************
bool ObjectState::AreChildrenModified()
{
  return (!mAddedChildren.Empty() || !mRemovedChildren.Empty() || mChildOrderModified);
}

//**************************************************************************************************
void ObjectState::ClearModifications()
{
  mModifiedProperties.Clear();
  mAddedChildren.Clear();
  mRemovedChildren.Clear();
  mChildOrderModified = false;
}

//**************************************************************************************************
void ObjectState::ClearModifications(HandleParam object, bool retainOverrideProperties)
{
  ModifiedProperties cachedMemory;
  ClearModifications(retainOverrideProperties, object, cachedMemory);
}

//**************************************************************************************************
void ObjectState::ClearModifications(bool retainOverrideProperties, HandleParam object,
                                     ModifiedProperties& cachedMemory)
{
  if(retainOverrideProperties)
  {
    // We need the instance to get the meta properties to check if they are set to override
    ReturnIf(object.IsNull(), , "We must have an instance to retain overridden properties");

    // Store all properties that are set to override so we can restore them after clearing
    // all other modified properties
    ModifiedProperties& savedProperties = cachedMemory;
    forRange(PropertyPath& propertyPath, mModifiedProperties.All())
    {
      Property* property = propertyPath.GetPropertyFromRoot(object);
      if(property && property->HasAttribute(PropertyAttributes::cLocalModificationOverride))
        savedProperties.Insert(propertyPath);
    }

    // Clear all properties
    mModifiedProperties.Clear();

    // Restore all saved override properties
    forRange(PropertyPath& propertyPath, savedProperties.All())
      mModifiedProperties.Insert(propertyPath);

    // Clear the cached memory now that we're done with it
    cachedMemory.Clear();

    mAddedChildren.Clear();
    mRemovedChildren.Clear();
    mChildOrderModified = false;
  }
  else
  {
    ClearModifications();
  }
}

//**************************************************************************************************
bool ObjectState::IsPropertyModified(PropertyPathParam property)
{
  return mModifiedProperties.Contains(property);
}

//**************************************************************************************************
bool ObjectState::HasModifiedProperties()
{
  return !mModifiedProperties.Empty();
}

//**************************************************************************************************
void ObjectState::SetPropertyModified(PropertyPathParam property, bool state)
{
  if(state)
    mModifiedProperties.Insert(property);
  else
    mModifiedProperties.Erase(property);
}

//**************************************************************************************************
ObjectState::ModifiedProperties::range ObjectState::GetModifiedProperties()
{
  return mModifiedProperties.All();
}

//**************************************************************************************************
void ObjectState::ChildAdded(ChildIdParam childId)
{
  if(mRemovedChildren.Contains(childId))
    mRemovedChildren.Erase(childId);
  else
    mAddedChildren.Insert(childId);
}

//**************************************************************************************************
void ObjectState::ChildRemoved(ChildIdParam childId)
{
  if(mAddedChildren.Contains(childId))
    mAddedChildren.Erase(childId);
  else
    mRemovedChildren.Insert(childId);
}

//**************************************************************************************************
bool ObjectState::IsChildLocallyAdded(ChildIdParam childId)
{
  return mAddedChildren.Contains(childId);
}

//**************************************************************************************************
bool ObjectState::IsChildLocallyRemoved(ChildIdParam childId)
{
  return mRemovedChildren.Contains(childId);
}

//**************************************************************************************************
bool ObjectState::IsChildOrderModified()
{
  return mChildOrderModified;
}

//**************************************************************************************************
void ObjectState::SetChildOrderModified(bool state)
{
  mChildOrderModified = state;
}

//**************************************************************************************************
ObjectState::ChildrenMap::range ObjectState::GetAddedChildren()
{
  return mAddedChildren.All();
}

//**************************************************************************************************
ObjectState::ChildrenMap::range ObjectState::GetRemovedChildren()
{
  return mRemovedChildren.All();
}

//----------------------------------------------------------------------- Local Object Modifications
//**************************************************************************************************
ObjectState* LocalModifications::GetObjectState(HandleParam object, bool createNew, bool validateStorage)
{
  if(object.IsNull())
    return nullptr;

  // Make sure we can store this type of object
  if(validateStorage && !IsValidForStorage(object))
    return nullptr;

  ObjectState* state = mObjectStates.FindValue(object, nullptr);
  if(state == nullptr && createNew)
  {
    state = new ObjectState();
    mObjectStates.Insert(object, state);
  }

  return state;
}

//**************************************************************************************************
bool LocalModifications::IsModified(HandleParam object, bool checkHierarchy, bool ignoreOverrideProperties)
{
  if(ObjectState* state = GetObjectState(object, false, false))
  {
    if(state->IsModified(object, ignoreOverrideProperties))
      return true;
  }

  MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();
  if(checkHierarchy && composition)
  {
    forRange(Handle child, composition->AllComponents(object))
    {
      if(child.StoredType->HasAttributeInherited(ObjectAttributes::cStoreLocalModifications))
      {
        if(IsModified(child, true, ignoreOverrideProperties))
          return true;
      }
      else
      {
        if(IsModified(child, object, true, ignoreOverrideProperties))
          return true;
      }
    }
  }

  return false;
}

//**************************************************************************************************
Handle LocalModifications::GetClosestInheritedObject(HandleParam object, bool checkParents)
{
  BoundType* objectType = object.StoredType;
  if(MetaDataInheritanceRoot* inheritance = objectType->HasInherited<MetaDataInheritanceRoot>())
  {
    // Check 
    String inheritId = inheritance->GetInheritId(object, InheritIdContext::Instance);
    if(!inheritId.Empty())
      return object;

    inheritId = inheritance->GetInheritId(object, InheritIdContext::Definition);
    if(!inheritId.Empty())
      return object;
  }

  // If it doesn't inherit from anything, continue checking parents if specified
  if(checkParents)
  {
    if(MetaOwner* metaOwner = objectType->HasInherited<MetaOwner>())
    {
      Handle owner = metaOwner->mGetOwner(object);
      if(!owner.IsNull())
        return GetClosestInheritedObject(owner, checkParents);
    }
  }

  return Handle();
}

//**************************************************************************************************
void LocalModifications::ClearModifications(HandleParam object, bool clearChildren,
                                            bool retainOverrideProperties)
{
  ObjectState::ModifiedProperties cachedMemory;
  ClearModifications(object, clearChildren, retainOverrideProperties, cachedMemory);
}

//**************************************************************************************************
void LocalModifications::ClearModifications(HandleParam object, bool clearChildren,
                                            bool retainOverrideProperties,
                                            ObjectState::ModifiedProperties& cachedMemory)
{
  if(object == nullptr)
    return;

  // We don't need to check for validation of the object because if it has modifications,
  // it has already been validated
  ObjectState* state = GetObjectState(object, false, false);
  if(state)
  {
    // Clear all non-override properties if we're retaining overridden properties
    if(retainOverrideProperties)
    {
      state->ClearModifications(true, object, cachedMemory);

      // If it's not modified (it didn't have overridden properties), then delete the state
      if(!state->IsModified())
      {
        mObjectStates.Erase(object);
        SafeDelete(state);
      }
    }
    // If we're not retaining overridden properties, we can delete the entire object state
    else
    {
      mObjectStates.Erase(object);
      SafeDelete(state);
    }
  }

  // Clear all children
  MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();
  if(clearChildren && composition)
  {
    forRange(Handle child, composition->AllComponents(object))
    {
      // If we're not retaining override properties, recursively delete all child
      // states as there's nothing we want to save
      if(!retainOverrideProperties)
      {
        ClearModifications(child, true, false, cachedMemory);
      }
      else
      {
        // If we are retaining override properties, we still don't want to retain
        // properties on locally added children (they should be removed anyways)
        BoundType* childType = child.StoredType;

        // Check to see if the child was locally added
        bool isLocallyAdded = false;
        if(state)
        {
          String childTypeName = childType->Name;
          ObjectState::ChildId childId(childTypeName);

          // If we have a guid for the child, use that. Otherwise use the type name
          if(MetaDataInheritance* inheritance = childType->HasInherited<MetaDataInheritance>())
            childId.mId = inheritance->GetUniqueId(child);

          isLocallyAdded = state->IsChildLocallyAdded(childId);
        }

        // Only retain if it wasn't locally added
        bool childRetainOverrideProperties = !isLocallyAdded;

        ClearModifications(child, true, retainOverrideProperties, cachedMemory);
      }
    }
  }
}

//**************************************************************************************************
bool LocalModifications::IsPropertyModified(HandleParam object, PropertyPathParam property)
{
  if(ObjectState* objectState = GetObjectState(object, false, false))
    return objectState->IsPropertyModified(property);
  return false;
}

//**************************************************************************************************
void LocalModifications::SetPropertyModified(HandleParam object, PropertyPathParam property, bool state)
{
  bool shouldCreateNew = state;
  if(ObjectState* objectState = GetObjectState(object, shouldCreateNew))
    objectState->SetPropertyModified(property, state);
}

//**************************************************************************************************
void LocalModifications::ChildAdded(HandleParam object, ObjectState::ChildIdParam childId)
{
  if(ObjectState* objectState = GetObjectState(object, true))
    objectState->ChildAdded(childId);
}

//**************************************************************************************************
void LocalModifications::ChildRemoved(HandleParam object, ObjectState::ChildIdParam childId)
{
  if(ObjectState* objectState = GetObjectState(object, true))
    objectState->ChildRemoved(childId);
}

//**************************************************************************************************
bool LocalModifications::IsChildLocallyAdded(HandleParam object, ObjectState::ChildIdParam childId)
{
  if(ObjectState* objectState = GetObjectState(object, false, false))
    return objectState->IsChildLocallyAdded(childId);
  return false;
}

//**************************************************************************************************
bool LocalModifications::IsObjectLocallyAdded(HandleParam object, bool recursivelyCheckParents)
{
  BoundType* objectType = object.StoredType;
  MetaOwner* metaOwner = objectType->HasInherited<MetaOwner>();
  ReturnIf(metaOwner == nullptr, false, "Object must have a way to get its owner to "
                                        "generically check if it was locally added.");

  // Check to see if we were locally added to our owner
  Handle owner = metaOwner->mGetOwner(object);
  if(!owner.IsNull())
  {
    if(IsChildLocallyAdded(owner, object))
      return true;

    // If we hit something that inherits from other data, we are not locally added
    if(MetaDataInheritanceRoot* inheritance = owner.StoredType->HasInherited<MetaDataInheritanceRoot>())
    {
      String ownerInheritId = inheritance->GetInheritId(owner, InheritIdContext::Instance);
      if(!ownerInheritId.Empty())
        return false;
    }

    // Walk up to see if our owner was locally added
    if(recursivelyCheckParents)
      return IsObjectLocallyAdded(owner, recursivelyCheckParents);
  }

  return false;
}

//**************************************************************************************************
bool LocalModifications::IsChildOrderModified(HandleParam object)
{
  if(ObjectState* objectState = GetObjectState(object))
    return objectState->IsChildOrderModified();
  return false;
}

//**************************************************************************************************
void LocalModifications::SetChildOrderModified(HandleParam object, bool state)
{
  if(ObjectState* objectState = GetObjectState(object, state))
    objectState->SetChildOrderModified(state);
}

//**************************************************************************************************
ObjectState* LocalModifications::TakeObjectState(HandleParam object)
{
  if(ObjectState* state = GetObjectState(object, false))
  {
    mObjectStates.Erase(object);
    return state;
  }
  return nullptr;
}

//**************************************************************************************************
void LocalModifications::RestoreObjectState(HandleParam object, ObjectState* objectState)
{
  if(object == nullptr)
    return;

  if(ObjectState* currentState = mObjectStates.FindValue(object, nullptr))
    delete currentState;

  if(objectState)
    mObjectStates.Insert(object, objectState);
  else
    mObjectStates.Erase(object);
}

//**************************************************************************************************
void LocalModifications::CombineObjectState(HandleParam object, ObjectState* objectState)
{
  ReturnIf(object == nullptr || objectState == nullptr, , "Invalid data given.");
  ObjectState* currentState = mObjectStates.FindValue(object, nullptr);
  if(currentState)
    currentState->Combine(objectState);
  else
    mObjectStates.Insert(object, objectState->Clone());
}

//**************************************************************************************************
bool ObjectContainsProperty(HandleParam object, Property* property)
{
  // Check the objects properties
  BoundType* objectType = object.StoredType;

  if(objectType->AllProperties.Contains(property))
    return true;

  // Walk composition
  if(MetaComposition* composition = objectType->HasInherited<MetaComposition>())
  {
    forRange(Handle child, composition->AllComponents(object))
    {
      if(ObjectContainsProperty(child, property))
        return true;
    }
  }

  // Walk child property objects
  forRange(Property* property, objectType->GetProperties())
  {
    BoundType* propertyType = Zilch::BoundType::GetBoundType(property->PropertyType);

    // If the contained property has its own properties, check them
    if(propertyType && !propertyType->GetProperties().Empty())
    {
      Any value = property->GetValue(object);
      Handle child(value);
      if(!child.IsNull())
      {
        if(ObjectContainsProperty(child, property))
          return true;
      }
    }
  }

  return false;
}

//**************************************************************************************************
bool LocalModifications::IsModified(HandleParam object, HandleParam propertyPathParent, bool checkHierarchy,
                                    bool ignoreOverrideProperties)
{
  if(object == propertyPathParent)
    return IsModified(object, checkHierarchy, ignoreOverrideProperties);

  if(ObjectState* parentState = GetObjectState(propertyPathParent))
  {
    // Walk each modified property of our parent
    forRange(PropertyPath& modifiedProperty, parentState->GetModifiedProperties())
    {
      // Check if 
      Property* property = modifiedProperty.GetPropertyFromRoot(propertyPathParent);

      if(ObjectContainsProperty(object, property))
        return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------- Meta Data Inheritance
//**************************************************************************************************
Guid MetaDataInheritance::GetUniqueId(HandleParam object)
{
  return cInvalidUniqueId;
}

//**************************************************************************************************
bool MetaDataInheritance::ShouldStoreLocalModifications(HandleParam object)
{
  /*
     Which objects we want to store local modifications:
     (example is using Cogs)

     Cog [Archetype]                       // Store
         ChildA                            // Store
         ChildB [LocallyAdded]             // Do not store
             ChildA                        // Do not store
             ChildB [Archetype]            // Store
                 ChildA                    // Store (relative to direct parent)
         ChildC [Archetype]                // Store
             ChildA                        // Store
             ChildB [LocallyAdded]         // Do not store
         ChildD [Archetype][LocallyAdded]  // Store
             ChildA                        // Store
             ChildB [LocallyAdded]         // Do not store

     Observations:
     1. If it's an Archetype, we always store modifications
     2. If you're a child of an Archetype, store modifications
       3. unless: you are locally added or your parent is locally added
  */
  LocalModifications* modifications = LocalModifications::GetInstance();

  Handle closestInheritedObject = modifications->GetClosestInheritedObject(object, true);

  // If nothing above us inherits from other data, we don't want to store modifications
  if(closestInheritedObject == nullptr)
    return false;

  // Always store modifications if we inherit from other data
  if(closestInheritedObject == object)
    return true;

  // If we don't directly inherit from data, we want to store modifications 
  // unless we are locally added
  return !modifications->IsObjectLocallyAdded(object, true);
}

//**************************************************************************************************
void MetaDataInheritance::Revert(HandleParam object)
{
  LocalModifications::GetInstance()->ClearModifications(object, true, false);
}

//**************************************************************************************************
bool MetaDataInheritance::CanPropertyBeReverted(HandleParam object, PropertyPathParam propertyPath)
{
  return false;
}

//**************************************************************************************************
void MetaDataInheritance::RevertProperty(HandleParam object, PropertyPathParam propertyPath)
{
  LocalModifications::GetInstance()->SetPropertyModified(object, propertyPath, false);
}

//**************************************************************************************************
void MetaDataInheritance::SetPropertyModified(HandleParam object, PropertyPathParam propertyPath,
                                              bool state)
{
  LocalModifications::GetInstance()->SetPropertyModified(object, propertyPath, state);
}

}//namespace Zero
