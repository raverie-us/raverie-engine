///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyHandle.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------- Property Path
ZilchDefineType(PropertyPath, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//******************************************************************************
PropertyPath::PropertyPath(Property* prop)
{
  AddPropertyToPath(prop);
}

//******************************************************************************
PropertyPath::PropertyPath(cstr prop)
{
  AddPropertyToPath(prop);
}

//******************************************************************************
PropertyPath::PropertyPath(StringParam prop)
{
  AddPropertyToPath(prop);
}

//******************************************************************************
PropertyPath::PropertyPath(Object* component, Property* prop)
{ 
  AddComponentToPath(component);
  AddPropertyToPath(prop);
}

//******************************************************************************
PropertyPath::PropertyPath(Object* component, StringParam propertyName)
{
  AddComponentToPath(component);
  AddPropertyToPath(propertyName);
}

//******************************************************************************
PropertyPath::PropertyPath(BoundType* componentType, Property* prop)
{
  AddComponentToPath(componentType);
  AddPropertyToPath(prop);
}

//******************************************************************************
PropertyPath::PropertyPath(BoundType* componentType, StringParam propertyName)
{
  AddComponentToPath(componentType);
  AddPropertyToPath(propertyName);
}

//******************************************************************************
PropertyPath::PropertyPath(StringParam componentName, Property* prop)
{
  AddComponentToPath(componentName);
  AddPropertyToPath(prop);
}

//******************************************************************************
PropertyPath::PropertyPath(StringParam componentName, StringParam propertyName)
{
  AddComponentToPath(componentName);
  AddPropertyToPath(propertyName);
}

//******************************************************************************
bool PropertyPath::operator==(const PropertyPath& rhs) const
{
  return Hash() == rhs.Hash();
}

//******************************************************************************
String PropertyPath::GetStringPath() const
{
  StringBuilder builder;
  bool first = true;
  forRange(Entry& entry, mPath.All())
  {
    if(entry.mType == PropertyPathType::Index)
    {
      builder.Append('[');
      builder.Append(entry.mIndex);
      builder.Append(']');
    }
    else
    {
      if (!first)
        builder.Append('.');
      builder.Append(entry.mName);
    }

    first = false;
  }
  return builder.ToString();
}

//******************************************************************************
Any PropertyPath::GetValue(HandleParam rootInstance) const
{
  Handle leaf = GetLeafInstance(rootInstance);
  if(Property* prop = GetPropertyFromLeaf(leaf))
    return prop->GetValue(leaf);
  return Any();
}

//******************************************************************************
bool PropertyPath::SetValue(HandleParam rootInstance, AnyParam newValue) const
{
  Handle leaf = GetLeafInstance(rootInstance);
  if(Property* prop = GetPropertyFromLeaf(leaf))
  {
    prop->SetValue(leaf, newValue);
    return true;
  }
  return false;
}

//******************************************************************************
Handle PropertyPath::GetLeafInstance(HandleParam rootInstance) const
{
  return GetLeafInstanceInternal(rootInstance);
}

//******************************************************************************
Property* PropertyPath::GetPropertyFromLeaf(HandleParam leafInstance) const
{
  String leafPropertyName = GetLeafPropertyName();
  return leafInstance.StoredType->GetProperty(leafPropertyName, Members::InheritedInstanceStatic);
}

//******************************************************************************
Property* PropertyPath::GetPropertyFromRoot(HandleParam rootInstance) const
{
  Handle leaf = GetLeafInstance(rootInstance);

  // The path may be invalid if a property was renamed or removed
  if(leaf.IsNull())
    return nullptr;

  return GetPropertyFromLeaf(leaf);
}

//******************************************************************************
String PropertyPath::GetLeafPropertyName() const
{
  ReturnIf(mPath.Empty(), String(), "Empty path");
  return mPath.Back().mName;
}

//******************************************************************************
void PropertyPath::AddComponentToPath(HandleParam component)
{
  AddComponentToPath(component.StoredType);
}

//******************************************************************************
void PropertyPath::AddComponentToPath(BoundType* componentType)
{
  AddComponentToPath(componentType->Name);
}

//******************************************************************************
void PropertyPath::AddComponentToPath(StringParam componentName)
{
  mPath.PushBack(Entry(componentName, PropertyPathType::Component));
}

//******************************************************************************
void PropertyPath::AddComponentIndexToPath(uint index)
{
  mPath.PushBack(Entry("", PropertyPathType::Index, index));
}

//******************************************************************************
void PropertyPath::AddPropertyToPath(Property* prop)
{
  AddPropertyToPath(prop->Name);
}

//******************************************************************************
void PropertyPath::AddPropertyToPath(StringParam propertyName)
{
  mPath.PushBack(Entry(propertyName, PropertyPathType::Property));
}

//******************************************************************************
void PropertyPath::GetInstanceHierarchy(HandleParam rootInstance,
                                        Array<Handle>* objects) const
{
  GetLeafInstanceInternal(rootInstance, objects);
}

//******************************************************************************
Handle GetComponent(HandleParam parent, StringParam componentName)
{
  BoundType* parentMeta = parent.StoredType;
  MetaComposition* composition = parentMeta->HasInherited<MetaComposition>();
  
  if (composition)
  {
    // Look up the Component type
    BoundType* componentMeta = MetaDatabase::GetInstance()->FindType(componentName);
    ReturnIf(componentMeta == nullptr, nullptr, "Invalid Component type in property path.");
  
    // Query the composition for that component
    Handle componentInstance = composition->GetComponent(parent, componentMeta);
    return componentInstance;
  }
  
  return Handle();
}

//******************************************************************************
Handle PropertyPath::GetLeafInstanceInternal(HandleParam instance,
                                             Array<Handle>* objects) const
{
  Handle currentInstance = instance;

  // We're looking for the leaf instance, and the last in the path is the
  // property, so we don't want to evaluate that (hence the size - 1)
  for(int i = 0; i < (int)mPath.Size() - 1; ++i)
  {
    BoundType* boundType = Type::GetBoundType(currentInstance.StoredType);

    const Entry& entry = mPath[i];
    if(entry.mType == PropertyPathType::Component)
    {
      currentInstance = GetComponent(currentInstance, entry.mName);
    }
    else if(entry.mType == PropertyPathType::Property)
    {
      Property* property = boundType->GetProperty(entry.mName);

      // The property may no longer exist
      if(property == nullptr)
        return Handle();

      Any result = property->GetValue(currentInstance);
      currentInstance = Handle(result);
    }
    else // entry.mType == PropertyPathType::Index
    {
      // We can only get the value from an index with a composition
      if(MetaArray* array = currentInstance.StoredType->HasInherited<MetaArray>())
      {
        Any value = array->GetValue(currentInstance, entry.mIndex);

        Handle valueHandle = value.ToHandle();
        ErrorIf(valueHandle.IsNull(), "Values in MetaArray must be able to be put into a handle."
          "Read the METAREFACTOR comment above the PropertyPath class "
          "definition for more info.");
        currentInstance = valueHandle;
      }
      else
      {
        return nullptr;
      }
    }

    ReturnIf(currentInstance.StoredType == nullptr, Handle(),
      "Object hierarchy does not match property path.");

    if(objects)
      objects->PushBack(currentInstance);
  }

  return currentInstance;
}

//******************************************************************************
void PropertyPath::PopEntry()
{
  mPath.PopBack();
}

//******************************************************************************
size_t PropertyPath::Hash() const
{
  size_t hash = 51243989;
  for(uint i = 0; i < mPath.Size(); ++i)
    hash ^= mPath[i].mName.Hash() * (i + 23520983);
  return hash;
}

//-------------------------------------------------------------- Property Handle
//******************************************************************************
PropertPathHandle::PropertPathHandle(HandleParam rootObject, PropertyPathParam path) :
  mRootObject(rootObject), mPath(path)
{
}

//******************************************************************************
Any PropertPathHandle::GetValue()
{
  return mPath.GetValue(mRootObject);
}

//******************************************************************************
bool PropertPathHandle::SetValue(AnyParam newValue)
{
  return mPath.SetValue(mRootObject, newValue);
}

}//namespace Zero
