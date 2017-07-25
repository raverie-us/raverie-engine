///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectSaver.cpp
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------- Object Saver
//**************************************************************************************************
ObjectSaver::ObjectSaver()
{
  //mInheritanceDepth = 0;
  mSerializeStart = nullptr;
}

//**************************************************************************************************
void ObjectSaver::SaveInstance(Object* object)
{
  PropertyPath path;
  SaveObject(object, object, path, false, InheritIdContext::Instance);
}

//**************************************************************************************************
void ObjectSaver::SaveDefinition(Object* object)
{
  PropertyPath path;
  SaveObject(object, object, path, false, InheritIdContext::Definition);
}

//**************************************************************************************************
void ObjectSaver::SaveObject(Object* object, Object* propertyPathParent, PropertyPath& path,
                             bool patching, InheritIdContext::Enum context)
{
  // Check to see if we should serialize this object
  BoundType* objectType = ZilchVirtualTypeId(object);

  // Check to see if we should serialize this object
  if(SerializationFilter* filter = objectType->HasInherited<SerializationFilter>())
  {
    if(filter->ShouldSerialize(object) == false)
      return;
  }

  // Store if this was the first object
  bool rootObject = false;
  if(mSerializeStart == nullptr)
  {
    mSerializeStart = object;
    rootObject = true;
  }
  
  // Start a new property path if this object stores local modifications
  PropertyPath localPath;
  if(objectType->HasAttributeInherited(ObjectAttributes::cStoreLocalModifications))
    path = localPath;

  // If we're patching, continue to only save out modified properties
  if(patching)
  {
    SaveModifications(object, propertyPathParent, path, context);
    return;
  }

  // If the given object inherits from another object, we want to save the object out
  // as a data patch
  String inheritId;
  if(MetaDataInheritanceRoot* inheritance = objectType->HasInherited<MetaDataInheritanceRoot>())
    inheritId = inheritance->GetInheritId(object, context);

  if(inheritId.Empty())
    SaveFullObject(object);
  else
    SaveModifications(object, propertyPathParent, path, context);

  // Reset data
  if(rootObject)
    mSerializeStart = nullptr;
}

//**************************************************************************************************
void ObjectSaver::SaveFullObject(Object* object)
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  // The 'InheritIdContext' here doesn't matter because this object is not being saved as inherited
  // data. We're just passing in 'InheritIdContext::Instance' because we need to pass in something
  PolymorphicInfo info;
  BuildPolymorphicInfo(info, object, InheritIdContext::Instance, false);

  // We're saving the full object, so clear out the inheritance id if it exists
  info.mInheritanceId = String();

  StartPolymorphicInternal(info);
  object->Serialize(*this);

  // We're not doing full meta serialization yet
  //SaveProperties(object, IsPatching());
  //SaveChildren(object, IsPatching());

  EndPolymorphic();
}

//**************************************************************************************************
void ObjectSaver::SaveModifications(Object* object, Object* propertyPathParent, PropertyPath& path,
                                    InheritIdContext::Enum context)
{
  PolymorphicInfo info;
  BuildPolymorphicInfo(info, object, context, true);

  StartPolymorphicInternal(info);

  // Save out modified properties
  SaveProperties(object, propertyPathParent, path, true);

  // Save out modified children
  SaveChildren(object, propertyPathParent, path, true);

  EndPolymorphic();
}

//**************************************************************************************************
void ObjectSaver::SaveProperties(Object* object, Object* propertyPathParent, PropertyPath& path,
                                 bool onlyModifiedProperties)
{
  if(onlyModifiedProperties)
  {
    LocalModifications* modifications = LocalModifications::GetInstance();

    // Only save out properties if the object had modifications
    if(ObjectState* objectState = modifications->GetObjectState(propertyPathParent))
    {
      BoundType* objectType = ZilchVirtualTypeId(object);
      forRange(Property* metaProperty, objectType->GetProperties())
      {
        // Skip properties that aren't serialized (disabled until meta refactor)
        //if(!metaProperty->Flags.IsSet(PropertyFlags::Serialized))
          //continue;

        BoundType* propertyType = Type::GetBoundType(metaProperty->PropertyType);

        // If it's not a value type, check to see if 
        if(propertyType &&
           propertyType->HasAttribute(PropertyAttributes::cAsPropertyUseCustomSerialization) &&
           !propertyType->GetProperties().Empty())
        {
          path.AddPropertyToPath(metaProperty);

          Any objectValue = metaProperty->GetValue(object);
          Object* subObject = objectValue.Get<Object*>();

          forRange(Property* subProperty, propertyType->GetProperties())
          {
            path.AddPropertyToPath(subProperty);

            if(objectState->IsPropertyModified(path))
            {
              InnerStart(propertyType->Name.c_str(), metaProperty->Name.c_str(), 0);
              InnerStart(propertyType->Name.c_str(), nullptr, StructureType::Object, true);

              path.PopEntry();

              SaveProperties(subObject, object, path, onlyModifiedProperties);

              InnerEnd(nullptr, StructureType::Object);

              break;
            }

            path.PopEntry();
          }

          path.PopEntry();
        }
        else
        {
          path.AddPropertyToPath(metaProperty);
          if(objectState->IsPropertyModified(path))
            SerializeProperty(object, metaProperty, *this);
          path.PopEntry();
        }
      }
    }
  }
  // Save out all properties
  else
  {
    MetaSerializeProperties(object, *this);
  }
}

//**************************************************************************************************
void ObjectSaver::SaveChildren(Object* object, Object* propertyPathParent, PropertyPath& path,
                               bool onlyModifiedChildren)
{
  // We need a MetaComposition to get access to the object's children
  BoundType* objectMeta = ZilchVirtualTypeId(object);

  MetaComposition* composition = objectMeta->Has<MetaComposition>();
  if(composition == nullptr)
    return;

  LocalModifications* modifications = LocalModifications::GetInstance();
  ObjectState* objectState = modifications->GetObjectState(object);

  forRange(Handle childHandle, composition->AllComponents(object))
  {
    Object* child = childHandle.Get<Object*>();

    // Only save objects that are modified if specified
    if(onlyModifiedChildren)
    {
      // If the object has modifications, we only want to save out modifications
      if(modifications->IsModified(child, true, false))
        SaveObject(child, child, path, true, InheritIdContext::Instance);
      // If the object was locally added, we want to save the object normally
      else if(objectState && objectState->IsChildLocallyAdded(Handle(child)))
        SaveObject(child, child, path, false, InheritIdContext::Instance);
      else if(objectState && objectState->IsChildOrderModified())
      {
        PolymorphicInfo info;
        BuildPolymorphicInfo(info, child, InheritIdContext::Instance, true);

        // We didn't actually have any modifications to the child's data, so just create and close
        // a polymorphic node to get its order correct in the file
        StartPolymorphicInternal(info);
        EndPolymorphic();
      }
    }
    else
    {
      SaveObject(child, propertyPathParent, path, onlyModifiedChildren, InheritIdContext::Instance);
    }
  }

  // Save out removed children
  if(objectState)
  {
    forRange(ObjectState::ChildId removedChild, objectState->GetRemovedChildren())
      AddSubtractivePolymorphicNode(removedChild.mTypeName.c_str(), removedChild.mId);
  }
}

//**************************************************************************************************
void ObjectSaver::BuildPolymorphicInfo(PolymorphicInfo& info, Object* object,
                                       InheritIdContext::Enum context, bool patching)
{
  LocalModifications* modifications = LocalModifications::GetInstance();
  BoundType* objectType = ZilchVirtualTypeId(object);

  // Set up serialization info
  info.mTypeName = objectType->Name.c_str();
  info.mRuntimeType = objectType;

  //if(patching)
  {
    // Get the inheritance data
    if(MetaDataInheritanceRoot* inheritance = objectType->HasInherited<MetaDataInheritanceRoot>())
      info.mInheritanceId = inheritance->GetInheritId(object, context);

    // Get the unique node id
    if(MetaDataInheritance* inheritance = objectType->HasInherited<MetaDataInheritance>())
    {
      // We don't need the unique node id if it's the root object being saved because it's only
      // used for resolving child nodes
      if(object != mSerializeStart)
        info.mUniqueNodeId = inheritance->GetUniqueId(object);
    }

    // Check our parent to see if we were locally added
    if(MetaOwner* metaOwner = objectType->HasInherited<MetaOwner>())
    {
      if(Object* owner = metaOwner->GetOwner(Handle(object)).Get<Object*>())
      {
        if(ObjectState* ownerState = modifications->GetObjectState(owner))
        {
          ObjectState::ChildId childId(objectType->Name, info.mUniqueNodeId);

          if(ownerState->IsChildLocallyAdded(childId))
            info.mFlags.SetFlag(PolymorphicSaveFlags::LocallyAdded);
        }
      }
    }

    // If we're patching, we don't want to add child inherit id's because don't want the inherit
    // id's to be resolved twice. It will be resolved when our parents inherited data is resolved,
    // and we only want to show modifications here.
    // This is true unless we're locally added, in that case we do want to include the inherit id,
    // because it's not in our parents inherited data
    //if(!info.mFlags.IsSet(PolymorphicSaveFlags::LocallyAdded))
      //info.mInheritanceId = "";

    // Check for child order override
    if(modifications->IsChildOrderModified(object))
      info.mFlags.SetFlag(PolymorphicSaveFlags::ChildOrderOverride);
  }
}

//**************************************************************************************************
//bool ObjectSaver::IsPatching()
//{
//  return mInheritanceDepth > 0;
//}

}//namespace Zero
