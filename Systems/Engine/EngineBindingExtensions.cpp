////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------ Meta Resource
//**************************************************************************************************
ZilchDefineType(MetaResource, builder, type)
{
}

//**************************************************************************************************
MetaResource::MetaResource(Resource* resource)
{
  SetResource(resource);
}

//**************************************************************************************************
void MetaResource::SetResource(Resource* resource)
{
  mResourceId = resource->mResourceId;
}

//------------------------------------------------------------------------- MetaEditor Script Object
//**************************************************************************************************
ZilchDefineType(MetaEditorScriptObject, builder, type)
{
  ZilchBindField(mAutoRegister)->AddAttribute(PropertyAttributes::cOptional);
}

//**************************************************************************************************
MetaEditorScriptObject::MetaEditorScriptObject()
  : mAutoRegister(true)
{
  
}

//---------------------------------------------------------------------------------- Meta Dependency
//**************************************************************************************************
ZilchDefineType(MetaDependency, builder, type)
{
}

//**************************************************************************************************
void MetaDependency::PostProcess(Status& status, ReflectionObject* owner)
{
  Property* property = Type::DynamicCast<Property*>(owner);

  // The attribute system should stop this from ever being the case, so it's an assert instead
  // of a compilation error
  ReturnIf(property == nullptr, , "Dependency attribute should only ever be on a property");

  BoundType* classType = property->Owner;

  // It's only valid for Components to have dependencies
  if (!classType->IsA(ZilchTypeId(Component)))
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

  CogComponentMeta* componentMeta = classType->HasOrAdd<::Zero::CogComponentMeta>(classType);
  componentMeta->mDependencies.Insert(propertyType);
  componentMeta->mSetupMode = SetupMode::DefaultConstructor;
}

//----------------------------------------------------------------------------------- Meta Interface
//**************************************************************************************************
ZilchDefineType(MetaInterface, builder, type)
{
}

//**************************************************************************************************
void MetaInterface::PostProcess(Status& status, ReflectionObject* owner)
{
  BoundType* classType = Type::DynamicCast<BoundType*>(owner);

  // The attribute system should stop this from ever being the case, so it's an assert instead
  // of a compilation error
  ReturnIf(classType == nullptr, , "Interface attribute should only ever be on a class");

  BoundType* baseType = classType->BaseType;

  CogComponentMeta* componentMeta = classType->HasOrAdd<::Zero::CogComponentMeta>(classType);
  componentMeta->AddInterface(baseType);
  componentMeta->mSetupMode = SetupMode::DefaultConstructor;
}

}//namespace Zero
