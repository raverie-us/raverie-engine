////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void FindResource(Call& call, ExceptionReport& report);
void FindResourceOrNull(Call& call, ExceptionReport& report);
void FindResourceOrError(Call& call, ExceptionReport& report);
void AddResourceFind(LibraryBuilder& builder, BoundType* resourceMeta);
void FindNamedResource(Call& call, ExceptionReport& report);
void ProcessResourceProperties(BoundType* type);
void ProcessComponentInterfaces(BoundType* type);

//-------------------------------------------------------------------------- Zero Library Extensions
//**************************************************************************************************
void EngineLibraryExtensions::AddNativeExtensions(LibraryBuilder& builder)
{
  return AddNativeExtensions(builder, builder.BoundTypes);
}

//**************************************************************************************************
void EngineLibraryExtensions::AddNativeExtensions(LibraryBuilder& builder, BoundTypeMap& boundTypes)
{
  BoundType* resourceType = ZilchTypeId(Resource);

  forRange(BoundType* type, boundTypes.Values())
  {
    // Add Resource.Find functions
    if(type->IsA(resourceType) && !type->HasAttribute(ObjectAttributes::cResourceInterface))
      AddResourceFind(builder, type);

    // The only reason we need to check if 'EnumMetaSerialization' already exists is because
    // we call this for a plugin library (more than once unfortunately).
    if (type->IsEnumOrFlags() && type->has(EnumMetaSerialization) == nullptr)
      type->Add(new EnumMetaSerialization(type));
  }

  MetaLibraryExtensions::AddNativeExtensions(builder, boundTypes);
}

//**************************************************************************************************
void EngineLibraryExtensions::AddExtensionsPreCompilation(LibraryBuilder& builder,
                                                          ResourceLibrary* resources)
{
  AddResourceExtensions(builder, resources);

  MetaLibraryExtensions::AddExtensionsPreCompilation(builder);
}

//**************************************************************************************************
void EngineLibraryExtensions::AddExtensionsPostCompilation(LibraryBuilder& builder)
{
  MetaLibraryExtensions::AddExtensionsPostCompilation(builder);
}

//**************************************************************************************************
void EngineLibraryExtensions::TypeParsedCallback(Zilch::ParseEvent* e, void* userData)
{
  MetaLibraryExtensions::TypeParsedCallback(e, userData);

  BoundType* componentType = ZilchTypeId(Component);

  BoundType* boundType = e->Type;

  // @TrevorS: Is this necessary anymore? I believe HandleManagers are inherited.
  if(boundType->IsA(componentType))
    boundType->HandleManager = componentType->HandleManager;

  if (boundType->IsEnumOrFlags())
    boundType->Add(new EnumMetaSerialization(boundType));

  // Get the location of the resource so we can take the user to where the type is defined
  ZilchDocumentResource* resource = (ZilchDocumentResource*)e->Location->CodeUserData;
  ReturnIf(resource == nullptr, , "Type parsed not from a Resource?");
  boundType->Add(new MetaResource(resource));

  Status nameStatus;
  resource->GetManager()->ValidateRawName(nameStatus, boundType->TemplateBaseName, boundType);

  if (nameStatus.Failed())
    e->BuildingProject->Raise(boundType->NameLocation, ErrorCode::GenericError, nameStatus.Message.c_str());

  ProcessComponentInterfaces(boundType);
}

//**************************************************************************************************
void EngineLibraryExtensions::FindProxiedTypeOrigin(BoundType* proxiedType)
{
  // This should probably look through only ZilchScripts for "class" and ZilchFragments for "struct"
  // This can possibly point a proxied fragment to a ZilchScript
  String regexString = String::Format("(class|struct)\\s+%s", proxiedType->Name.c_str());

  Regex regex(regexString);

  forRange(ResourceId textResourceId, Z::gResources->TextResources.Values())
  {
    DocumentResource* textResource = (DocumentResource*)Z::gResources->GetResource(textResourceId);
    if(textResource == nullptr)
      continue;

    Matches matches;
    regex.Search(textResource->LoadTextData(), matches);

    if (matches.Empty())
      continue;

    // Remove it if it's already there
    MetaResource* metaResource = proxiedType->HasOrAdd<MetaResource>();
    metaResource->SetResource(textResource);
    return;
  }

  // Remove the component if it wasn't found this time
  if (proxiedType->Has<MetaResource>())
    proxiedType->Remove<MetaResource>();
}

//**************************************************************************************************
void EngineLibraryExtensions::AddResourceExtensions(LibraryBuilder& builder, ResourceLibrary* resources)
{
  // Add a property extension for all resources so they can be accessed on the resource type
  // e.g. SpriteSource.Fireball
  forRange(HandleOf<Resource> resourceHandle, resources->Resources.All())
  {
    Resource* resource = resourceHandle;
    BoundType* resourceType = ZilchVirtualTypeId(resource);

    String resourceName = resource->Name;

    Property* property = builder.AddExtensionGetterSetter(
      resourceType,
      resourceName,
      resourceType,
      nullptr,
      FindNamedResource,
      MemberOptions::Static);

    property->Get->ComplexUserData.WriteObject(resourceName);
    property->Get->UserData = resource->GetManager();

    // By setting all of these, we ensure that Go-To-Definition will
    // select our resource (see EditorUtility/DisplayCodeDefinition)
    u64 resourceId = (u64)resource->mResourceId;
    property->Location.CodeUserDataU64 = resourceId;
    property->NameLocation.CodeUserDataU64 = resourceId;
    property->Get->Location.CodeUserDataU64 = resourceId;
    property->Get->NameLocation.CodeUserDataU64 = resourceId;
  }
}

//**************************************************************************************************
bool FindResourceMode(Call& call, ExceptionReport& report, ResourceNotFound::Enum mode)
{
  Function* currentFunction = call.GetFunction();
  String* name = call.Get<String*>(0);

  // Guard against the user passing in null
  if(name == nullptr)
  {
    call.GetState()->ThrowNullReferenceException(report, "The string that was passed in was null. Please provide a name.");
    return false;
  }

  ResourceManager* manager = Z::gResources->Managers.FindValue(currentFunction->Owner->Name, nullptr);
  ReturnIf(manager == nullptr, false, "Couldn't find resource manager");

  if(Resource* resource = manager->GetResource(*name, mode))
  {
    call.Set(Call::Return, resource);
    return true;
  }

  Handle nullHandle;

  call.Set(Call::Return, nullHandle);
  return false;
}

//**************************************************************************************************
void FindResourceOrDefault(Call& call, ExceptionReport& report)
{
  FindResourceMode(call, report, ResourceNotFound::ReturnDefault);
}

//**************************************************************************************************
void FindResourceOrNull(Call& call, ExceptionReport& report)
{
  FindResourceMode(call, report, ResourceNotFound::ReturnNull);
}

//**************************************************************************************************
void FindResourceOrError(Call& call, ExceptionReport& report)
{
  if(FindResourceMode(call, report, ResourceNotFound::ReturnNull) == false)
  {
    // If the first parameter wasn't a string, an exception may have already been thrown
    if (report.HasThrownExceptions() == false)
    {
      String* name = call.Get<String*>(0);
      String message = String::Format("Failed to find resource by the name '%s'", name->c_str());
      call.GetState()->ThrowException(report, message);
    }
  }
}

//**************************************************************************************************
void AddResourceFind(LibraryBuilder& builder, BoundType* resourceType)
{
  // We're binding a static function on the type called 'Find', which finds a resource of that type by name
  ParameterArray params;
  DelegateParameter& p0 = params.PushBack();
  p0.ParameterType = ZilchTypeId(String);
  p0.Name = "name";

  // Find
  Function* function = builder.AddExtensionFunction(
    resourceType,
    "FindOrDefault",
    FindResourceOrDefault,
    params,
    resourceType,
    FunctionOptions::Static);

  // Find or null
  function = builder.AddExtensionFunction(
    resourceType,
    "FindOrNull",
    FindResourceOrNull,
    params,
    resourceType,
    FunctionOptions::Static);

  // Find or error
  function = builder.AddExtensionFunction(
    resourceType,
    "FindOrError",
    FindResourceOrError,
    params,
    resourceType,
    FunctionOptions::Static);
}

//**************************************************************************************************
void FindNamedResource(Call& call, ExceptionReport& report)
{
  Function* currentFunction = call.GetFunction();
  String& name = currentFunction->ComplexUserData.ReadObject<String>(0);
  ResourceManager* manager = (ResourceManager*)currentFunction->UserData;

  Resource* resource = manager->GetResource(name, ResourceNotFound::ErrorFallback);

  call.Set(Call::Return, resource);
}

//**************************************************************************************************
void ProcessComponentInterfaces(BoundType* type)
{
  // Check for this having a base that has component interface
  Attribute* componentInterfaceAttribute = type->HasAttributeInherited(ObjectAttributes::cComponentInterface);
  if (componentInterfaceAttribute)
  {
    BoundType* baseType = type->BaseType;
    // Keep walking up the hierarchy to find what base type had the attribute. Make
    // sure we skip the base type itself though (i.e. Collider can't add Collider as an interface)
    while (baseType != nullptr && type != baseType)
    {
      if (baseType->HasAttribute(ObjectAttributes::cComponentInterface))
      {
        CogComponentMeta* componentMeta = type->HasOrAdd<CogComponentMeta>(type);
        // The default constructor will set this type to FromDataOnly so we have to manually set this for now
        componentMeta->mSetupMode = SetupMode::DefaultSerialization;
        componentMeta->AddInterface(baseType);
        break;
      }

      baseType = baseType->BaseType;
    }
  }
}

}//namespace Zero
