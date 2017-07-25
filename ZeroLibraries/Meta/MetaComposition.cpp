///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineType(MetaOwner, builder, type)
{
}

//----------------------------------------------------------------------------------- Meta Component
//**************************************************************************************************
ZilchDefineType(CogComponentMeta, builder, type)
{
}

//**************************************************************************************************
CogComponentMeta::CogComponentMeta(BoundType* owner) :
  mSetupMode(SetupMode::FromDataOnly),
  mOwner(owner)
{
  // Inherit our base set up mode by default
  if(owner->BaseType)
  {
    if (CogComponentMeta* base = owner->BaseType->HasInherited<CogComponentMeta>())
      mSetupMode = base->mSetupMode;
  }
}

//**************************************************************************************************
void CogComponentMeta::AddInterface(BoundType* interfaceType)
{
  CogComponentMeta* interfaceMetaComponent = interfaceType->HasOrAdd<CogComponentMeta>(interfaceType);
  interfaceMetaComponent->mInterfaceDerivedTypes.PushBack(mOwner);
  mInterfaces.PushBack(interfaceType);
}

//**************************************************************************************************
bool CogComponentMeta::SupportsInterface(BoundType* interfaceMeta)
{
  // If our meta is the interface, then we support this interface
  if (mOwner == interfaceMeta)
    return true;

  // Otherwise, see if any of our interfaces support the passed in meta
  forRange(BoundType* componentInterface, mInterfaces.All())
  {
    if (componentInterface == interfaceMeta)
      return true;
  }
  return false;
}

//**************************************************************************************************
bool CogComponentMeta::IsDependentOn(BoundType* componentType, BoundType* dependentType)
{
  // Each type may have its own 'CogComponentMeta', so we have to walk all base types
  forRange(CogComponentMeta* metaComponent, componentType->HasAll<CogComponentMeta>())
  {
    // Walk the dependencies of the component we're checking
    forRange(BoundType* dependentComponent, metaComponent->mDependencies.All())
    {
      // The reason we're not checking if the dependenType is a dependentComponent (dynamic cast)
      // is because if it dependentComponent isn't an [Interface], we couldn't call 
      // 'has' on the object with dependentType and get dependentComponent. 

      // Check the base type of the component
      if (Type::IsSame(dependentType, dependentComponent))
        return true;

      // Check all interfaces provided by the component for a dependency
      forRange(CogComponentMeta* dependentMetaComponent, dependentType->HasAll<CogComponentMeta>())
      {
        forRange(BoundType* interfacesToCheck, dependentMetaComponent->mInterfaces.All())
        {
          if (Type::IsSame(interfacesToCheck, dependentComponent))
            return true;
        }
      }
    }
  }

  // Each type may have its own 'CogComponentMeta', so we have to walk all base types
  forRange(CogComponentMeta* metaComponent, componentType->HasAll<CogComponentMeta>())
  {
    // Walk the dependencies of the component we're checking
    forRange(BoundType* dependentComponent, metaComponent->mDependencies.All())
    {
      // Check the base type of the component
      if (dependentType->IsA(dependentComponent))
        return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------- Meta Composition
//**************************************************************************************************
ZilchDefineType(MetaComposition, builder, type)
{
  
}

//**************************************************************************************************
MetaComposition::MetaComposition(BoundType* componentType) :
  mComponentType(componentType),
  mSupportsComponentRemoval(true),
  mSupportsComponentReorder(true)
{
}

//**************************************************************************************************
bool MetaComposition::HasComponent(HandleParam owner, BoundType* componentType)
{
  Handle component = GetComponent(owner, componentType);
  return component.IsNotNull();
}

//**************************************************************************************************
Handle MetaComposition::GetComponent(HandleParam owner, BoundType* componentType)
{
  // Check each component to see if it's the given type
  forRange(Handle component, AllComponents(owner))
  {
    // METAREFACTOR Should this be IsA or IsRawCastable? What is the difference?
    if (component.StoredType->IsA(componentType))
      return component;
  }

  return Handle();
}

//**************************************************************************************************
Handle MetaComposition::GetComponentAt(HandleParam owner, uint index)
{
  Error("Not Implemented");
  return Handle();
}

//**************************************************************************************************
Handle MetaComposition::GetComponentUniqueId(HandleParam owner, u64 uniqueId)
{
  if (uniqueId == cInvalidUniqueId.mValue)
    return Handle();

  // All components are assumed to have the same MetaDataInheritance so we don't have to
  // look it up for each component. If this is ever not the case, this function needs to be changed
  MetaDataInheritance* inheritance = mComponentType->HasInherited<MetaDataInheritance>();

  ReturnIf(inheritance == nullptr, Handle(), "Cannot get component by unique id on this type.");

  forRange(Handle component, AllComponents(owner))
  {
    if (inheritance->GetUniqueId(component) == uniqueId)
      return component;
  }

  return Handle();
}

//**************************************************************************************************
uint MetaComposition::GetComponentIndex(HandleParam owner, BoundType* componentType)
{
  uint componentCount = GetComponentCount(owner);
  for(uint i = 0; i < componentCount; ++i)
  {
    Handle currentComponent = GetComponentAt(owner, i);
    if(currentComponent.StoredType == componentType)
      return i;
  }

  return uint(-1);
}

//**************************************************************************************************
uint MetaComposition::GetComponentIndex(HandleParam owner, HandleParam component)
{
  return GetComponentIndex(owner, component.StoredType);
}

//**************************************************************************************************
void MetaComposition::Enumerate(Array<BoundType*>& addTypes, EnumerateAction::Enum action,
                                HandleParam owner)
{
  ReturnIf(action == EnumerateAction::AllAddableToObject && owner.IsNull(), ,
           "Must give instance to get only addable types");

  forRange(BoundType* componentType, mComponentTypes.Values())
  {
    // Cannot add proxies
    if (componentType->HasAttribute(ObjectAttributes::cProxy))
      continue;

    // Cannot add core components
    if (componentType->HasAttribute(ObjectAttributes::cCore))
      continue;

    // If we aren't return all types, we need to check if it can even be added 
    if(action == EnumerateAction::AllAddableToObject)
    {
      if(CanAddComponent(owner, componentType))
        addTypes.PushBack(componentType);
    }
    else
    {
      // Cannot add types if they can only be constructed from data
      CogComponentMeta* metaComponent = componentType->has(CogComponentMeta);
      if(metaComponent != nullptr && metaComponent->mSetupMode == SetupMode::FromDataOnly)
        continue;

      // Cannot add types without a default constructor bound
      if(componentType->GetDefaultConstructor() != nullptr)
        addTypes.PushBack(componentType);
    }
  }
}

//**************************************************************************************************
void MetaComposition::Enumerate(Array<String>& addTypes, EnumerateAction::Enum action,
                                HandleParam owner)
{
  Array<BoundType*> types;
  Enumerate(types, action, owner);

  forRange(BoundType* type, types.All())
    addTypes.PushBack(type->Name);
}

//**************************************************************************************************
bool MetaComposition::CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info)
{
  // If it doesn't have a bound default constructor, it cannot be created, so we cannot add it
  if(typeToAdd->GetDefaultConstructor() == nullptr)
  {
    if(info)
      info->Reason = "Type doesn't have a default constructor, so we cannot create it";
    return false;
  }

  // Is there an component that implements an interface that the adding
  // component provides? Only one component with a given interface may exist.
  forRange(Handle component, AllComponents(owner))
  {
    //Is the component already present?
    if(Type::IsSame(component.StoredType, typeToAdd))
    {
      // Generate info
      if (info)
      {
        info->BlockingComponent = component;
        info->Reason = String::Format("Component %s already present on object",
                                      typeToAdd->ToString().c_str());
      }
      return false;
    }
    
    // METAREFACTOR Could this be done differently? There's usually either 0 or 1 in the 
    // Interfaces, so maybe it's not a big deal..?
    forRange(CogComponentMeta* addingComponentMeta, typeToAdd->HasAll<CogComponentMeta>())
    {
      // Check each interface on the existing component
      forRange(CogComponentMeta* componentMeta, component.StoredType->HasAll<CogComponentMeta>())
      {
        forRange(BoundType* currentInterface, componentMeta->mInterfaces.All())
        {
          // For any new interfaces
          forRange(BoundType* newInterface, addingComponentMeta->mInterfaces.All())
          {
            if (currentInterface == newInterface)
            {
              // Generate info
              if (info)
              {
                // We're forcing the type to be its base class interface
                info->BlockingComponent = component;
                info->BlockingComponent.StoredType = currentInterface;

                String blockingName = component.StoredType->ToString();
                info->Reason = String::Format("Component with same interface already "
                  "exists on object (%s)", blockingName.c_str());
              }
              return false;
            }
          }
        }
      }
    }
  }

  forRange(CogComponentMeta* addingComponentMeta, typeToAdd->HasAll<CogComponentMeta>())
  {
    // Are the needed dependencies present?
    forRange(BoundType* neededDependency, addingComponentMeta->mDependencies.All())
    {
      // Check for object needing this owner type
      // This allows for components to be dependent on certain composition types
      // e.g. the TimeSpace Component has a dependency on Space (it can only be
      // added to the Space).
      if (owner.StoredType->IsA(neededDependency))
        continue;

      // Check for the dependency
      Handle dependency = GetComponent(owner, neededDependency);

      // If we didn't find the dependency...
      if(dependency.StoredType == nullptr)
      {
        // Generate info
        if(info)
        {
          info->MissingDependency = dependency.StoredType;

          // Short reason
          String dependencyName = neededDependency->ToString();
          info->Reason = String::Format("Missing dependency %s", dependencyName.c_str());

          // Long reason
          String addingName = typeToAdd->ToString();
          info->Reason = String::Format("Component %s can't be added due to a missing dependency "
                                        "on %s.", addingName.c_str(), dependencyName.c_str());
        }
        return false;
      }
    }
  }

  return true;
}

//**************************************************************************************************
Handle MetaComposition::MakeObject(BoundType* typeToCreate)
{
  Error("Not implemented");
  return Handle();
}

//**************************************************************************************************
BoundType* MetaComposition::MakeProxy(StringParam typeName, ProxyReason::Enum reason)
{
  Error("Not implemented");
  return nullptr;
}

//**************************************************************************************************
void MetaComposition::AddComponent(HandleParam owner, HandleParam component, int index,
                                   bool ignoreDependencies)
{
  Error("Not implemented");
}

//**************************************************************************************************
void MetaComposition::AddComponent(HandleParam owner, BoundType* typeToAdd, int index,
                                   bool ignoreDependencies)
{
  Handle component = MakeObject(typeToAdd);
  AddComponent(owner, component, index, ignoreDependencies);
}

//**************************************************************************************************
bool MetaComposition::CanRemoveComponent(HandleParam owner, HandleParam component, String& reason)
{
  BoundType* typeToRemove = component.StoredType;

  // Cannot remove components that are marked as not removable
  if (typeToRemove->HasAttribute(ObjectAttributes::cCore))
  {
    cstr typeName = typeToRemove->Name.c_str();
    reason = String::Format("Component %s can't be removed. Critical component.", typeName);

    return false;
  }

  // Go through all the components checking to see if they have a
  // dependency on the component we're trying to remove
  forRange(Handle component, AllComponents(owner))
  {
    BoundType* componentType = component.StoredType;

    // Check to see if any of the dependencies this component requires
    // are on the component being checked.
    if(CogComponentMeta::IsDependentOn(componentType, typeToRemove))
    {
      cstr toRemoveName = typeToRemove->Name.c_str();
      cstr blockingComponentName = componentType->Name.c_str();
      reason = String::Format("Component %s can't be removed due to a dependency from %s.",
                              toRemoveName, blockingComponentName);

      return false;
    }
  }

  return true;  
}

//**************************************************************************************************
bool MetaComposition::CanRemoveComponent(HandleParam owner, BoundType* componentType, String& reason)
{
  Handle component = GetComponent(owner, componentType);
  if(component.IsNotNull())
    return CanRemoveComponent(owner, component, reason);
  reason = String::Format("Component %s doesn't exist on object", componentType->Name.c_str());
  return false;
}

//**************************************************************************************************
void MetaComposition::RemoveComponent(HandleParam owner, HandleParam component,
                                      bool ignoreDependencies)
{
  Error("Not Implemented.");
}

//**************************************************************************************************
bool MetaComposition::CanMoveComponent(HandleParam owner, HandleParam component, uint destination,
                                       Handle& blockingComponent, String& reason)
{
  String componentName = component.StoredType->Name;
  uint currentIndex = GetComponentIndex(owner, component);

  if(destination > currentIndex)
  {
    // We're moving the component closer to the end of the component list,
    // so we need to check to see if anything behind is dependent on us
    for(uint i = currentIndex + 1; i < destination; ++i)
    {
      // Check to see if this component is dependent on us
      Handle currComponent = GetComponentAt(owner, i);

      if(CogComponentMeta::IsDependentOn(currComponent.StoredType, component.StoredType))
      {
        // Say what component is dependent on what
        String currComponentName = currComponent.StoredType->Name;
        reason = String::Format("%s is dependent on %s",
                                currComponentName.c_str(), componentName.c_str());
        blockingComponent = currComponent;
        return false;
      }
    }
  }
  else
  {
    // We're moving the component closer to the beginning of the component
    // list, so we need to check to see if we're dependent on
    // anything before us
    for(int i = (int)currentIndex - 1; i >= (int)destination; --i)
    {
      // Check to see if we're dependent on this component
      Handle currComponent = GetComponentAt(owner, (uint)i);

      if(CogComponentMeta::IsDependentOn(component.StoredType, currComponent.StoredType))
      {
        String currComponentName = currComponent.StoredType->Name;
        reason = String::Format("%s is dependent on %s",
                                componentName.c_str(), currComponentName.c_str());
        blockingComponent = currComponent;
        return false;
      }
    }
  }

  return true;
}

//**************************************************************************************************
void MetaComposition::MoveComponent(HandleParam owner, HandleParam component,
                                    uint destination)
{
  Error("Not Implemented.");
}

//**************************************************************************************************
bool MetaComposition::ComponentRange::Empty()
{
  return (mIndex >= mComposition->GetComponentCount(mOwner));
}

//**************************************************************************************************
Handle MetaComposition::ComponentRange::Front()
{
  return mComposition->GetComponentAt(mOwner, mIndex);
}

//**************************************************************************************************
void MetaComposition::ComponentRange::PopFront()
{
  ++mIndex;
}

//**************************************************************************************************
MetaComposition::ComponentRange MetaComposition::AllComponents(HandleParam owner)
{
  ComponentRange r;
  r.mOwner = owner;
  r.mIndex = 0;
  r.mComposition = this;
  return r;
}

//**************************************************************************************************
String MetaComposition::GetAddName()
{
  return mComponentType->Name;
}

//--------------------------------------------------------------------------------------- Meta Owner
//**************************************************************************************************
Handle MetaOwner::GetOwner(HandleParam object)
{
  return mGetOwner(object);
}

}//namespace Zero
