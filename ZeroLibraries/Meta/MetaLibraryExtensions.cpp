////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void AddPropertyRenamedAttribute(Zilch::ParseEvent* e, Property* property, Attribute* attribute);

//-------------------------------------------------------------------------- Zero Library Extensions
//**************************************************************************************************
void MetaLibraryExtensions::AddNativeExtensions(LibraryBuilder& builder)
{
  // Loop through any SendsEvents on every type in this library, and add to MetaDatabase
  // METAREFACTOR this should walk BoundTypes, but they are not filled out yet
  forRange(BoundType* type, builder.BoundTypes.Values())
    ProcessComposition(builder, type);

  // Process all Components now that compositions have been processed
  forRange(BoundType* type, builder.BoundTypes.Values())
    ProcessComponent(builder, type);
}

//**************************************************************************************************
void MetaLibraryExtensions::AddExtensionsPreCompilation(LibraryBuilder& builder)
{

}

//**************************************************************************************************
void MetaLibraryExtensions::AddExtensionsPostCompilation(LibraryBuilder& builder)
{

}

//**************************************************************************************************
void MetaLibraryExtensions::TypeParsedCallback(Zilch::ParseEvent* e, void* userData)
{
  ProcessComponent(*e->Builder, e->Type);

  // Check for renamed properties
  forRange(Property* property, e->Type->GetProperties(Members::Instance))
  {
    if (Attribute* attribute = property->HasAttribute(PropertyAttributes::cRenamedFrom))
      AddPropertyRenamedAttribute(e, property, attribute);
  }
}

//**************************************************************************************************
// Get a component from a Zero Object in Zilch
void GetComponent(Call& call, ExceptionReport& report)
{
  Function* currentFunction = call.GetFunction();
  BoundType* metaType = Type::DebugOnlyDynamicCast<BoundType*>(currentFunction->FunctionType->Return);

  Handle& selfHandle = call.GetHandle(Call::This);
  MetaComposition* composition = selfHandle.StoredType->HasInherited<MetaComposition>();

  Handle componentInstance = composition->GetComponent(selfHandle, metaType);
  call.Set(Call::Return, componentInstance);
}

//**************************************************************************************************
void MetaLibraryExtensions::ProcessComposition(LibraryBuilder& builder, BoundType* type)
{
  if(MetaComposition* composition = type->Has<MetaComposition>())
      MetaDatabase::GetInstance()->mCompositionTypes.PushBack(type);
}

//**************************************************************************************************
void MetaLibraryExtensions::ProcessComponent(LibraryBuilder& builder, BoundType* type)
{
  forRange(BoundType* compositionType, MetaDatabase::GetInstance()->mCompositionTypes.All())
  {
    MetaComposition* composition = compositionType->HasInherited<MetaComposition>();
    BoundType* componentType = composition->mComponentType;

    if(type->IsA(componentType))
      AddCompositionExtension(builder, compositionType, type);
  }
}

//**************************************************************************************************
void MetaLibraryExtensions::AddCompositionExtension(LibraryBuilder& builder, BoundType* compositionType,
                                                    BoundType* componentType)
{
  Property* extensionProperty = builder.AddExtensionGetterSetter(
    compositionType,
    componentType->Name,
    componentType,
    nullptr,
    GetComponent,
    MemberOptions::None);

  ErrorIf(extensionProperty == nullptr, "Failed to bind event extension property");

  if(extensionProperty)
  {
    // Don't bother showing these properties in the debugger if they are null
    extensionProperty->IsHiddenWhenNull = true;

    // Copy over description
    extensionProperty->Description = componentType->Description;
  }
}

//**************************************************************************************************
void AddPropertyRenamedAttribute(Zilch::ParseEvent* e, Property* property, Attribute* attribute)
{
  cstr msg = "RenamedFrom attribute must have a string parameter of the old "
             "property name (i.e. \"[RenamedFrom(\"CurrentHealth\"]\"";

  // We must have one parameter
  if (attribute->Parameters.Empty())
  {
    e->BuildingProject->Raise(property->Location, ErrorCode::GenericError, msg);
    return;
  }

  // It must be a string (the old property name)
  AttributeParameter& param = attribute->Parameters.Front();
  if (param.Type != ConstantType::String)
  {
    e->BuildingProject->Raise(property->Location, ErrorCode::GenericError, msg);
    return;
  }

  // Check for duplicates
  MetaPropertyRename* rename = property->Has<MetaPropertyRename>();
  if (rename)
  {
    cstr duplicateMsg = "You can only have one 'RenamedFrom' attribute";
    e->BuildingProject->Raise(property->Location, ErrorCode::GenericError, duplicateMsg);
    return;
  }

  property->Add(new MetaPropertyRename(param.StringValue));
}

}//namespace Zero
