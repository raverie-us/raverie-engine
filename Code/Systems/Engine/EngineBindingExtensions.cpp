// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Meta Resource
RaverieDefineType(MetaResource, builder, type)
{
}

MetaResource::MetaResource(Resource* resource)
{
  SetResource(resource);
}

void MetaResource::SetResource(Resource* resource)
{
  mResourceId = resource->mResourceId;
}

// MetaEditor Script Object
RaverieDefineType(MetaEditorScriptObject, builder, type)
{
  RaverieBindField(mAutoRegister)->AddAttribute(PropertyAttributes::cOptional);
}

MetaEditorScriptObject::MetaEditorScriptObject() : mAutoRegister(true)
{
}

void MetaEditorScriptObject::PostProcess(Status& status, ReflectionObject* owner)
{
  // If auto register is true, we're creating an empty Cog and adding this
  // Component to it. Therefore, it cannot have any dependencies. We could first
  // add the dependencies to the empty Cog, but that's for a later time
  if (mAutoRegister)
  {
    BoundType* componentType = Type::DebugOnlyDynamicCast<BoundType*>(owner);

    // We would normally check for CogComponentMeta and check its dependencies,
    // however when this post process happens, the property attributes haven't
    // been processed yet. We should add a second pass for post process when all
    // attributes for the class have been processed. Or we could even change
    // this post process to be after everything

    forRange (Property* property, componentType->GetProperties())
    {
      if (property->HasAttribute(PropertyAttributes::cDependency))
      {
        status.SetFailed("Cannot have dependencies with autoRegister:true");
        return;
      }
    }
  }
}

// Meta Dependency
RaverieDefineType(MetaDependency, builder, type)
{
}

void MetaDependency::PostProcess(Status& status, ReflectionObject* owner)
{
  Property* property = Type::DynamicCast<Property*>(owner);

  // The attribute system should stop this from ever being the case, so it's an
  // assert instead of a compilation error
  ReturnIf(property == nullptr, , "Dependency attribute should only ever be on a property");

  BoundType* classType = property->Owner;

  // It's only valid for Components to have dependencies
  if (!classType->IsA(RaverieTypeId(Component)))
  {
    String message = "Dependency properties can only be on Component types";
    status.SetFailed(message);
    return;
  }

  BoundType* propertyType = Type::GetBoundType(property->PropertyType);

  // Cannot be our own type
  if (classType->IsA(propertyType))
  {
    String message = "Cannot have a dependency of our own class type";
    status.SetFailed(message);
    return;
  }

  CogComponentMeta* componentMeta = classType->HasOrAdd<::Raverie::CogComponentMeta>(classType);
  componentMeta->mDependencies.Insert(propertyType);
  componentMeta->mSetupMode = SetupMode::DefaultConstructor;
}

// Meta Interface
RaverieDefineType(MetaInterface, builder, type)
{
}

void MetaInterface::PostProcess(Status& status, ReflectionObject* owner)
{
  BoundType* classType = Type::DynamicCast<BoundType*>(owner);

  // The attribute system should stop this from ever being the case, so it's an
  // assert instead of a compilation error
  ReturnIf(classType == nullptr, , "Interface attribute should only ever be on a class");

  BoundType* baseType = classType->BaseType;

  CogComponentMeta* componentMeta = classType->HasOrAdd<::Raverie::CogComponentMeta>(classType);
  componentMeta->AddInterface(baseType);
  componentMeta->mSetupMode = SetupMode::DefaultConstructor;
}

} // namespace Raverie
