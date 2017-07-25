////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------- Zilch Component
//**************************************************************************************************
ZilchDefineType(ZilchComponent, builder, type)
{
  ZeroBindComponent();
  type->Add(new MetaSerialization());

  // Temporary solution so that ZilchComponent cannot be added on Cog in the property grid
  type->HasOrAdd<CogComponentMeta>(type)->mSetupMode = SetupMode::FromDataOnly;
}

//**************************************************************************************************
void RestoreCogPathLinks(HandleParam object, Cog* owner, CogInitializer& initializer)
{
  BoundType* cogPathType = ZilchTypeId(CogPath);
  BoundType* objectType = object.StoredType;

  // Cog paths and some certain properties need to have a special resolve phase called on them
  forRange (Property* property, objectType->GetProperties())
  {
    if(property->HasAttribute(PropertyAttribute) ||
       property->HasAttribute(PropertyAttributes::cEditable) ||
       property->HasAttribute(PropertyAttributes::cSerialized))
    {
      Any value = property->GetValue(object);
      if(property->PropertyType == cogPathType)
      {
        CogPath* cogPath = value.Get<CogPath*>( );
        if(cogPath != nullptr)
          cogPath->RestoreLink(initializer, owner, property->Name);
      }
      else
      {
        Handle subObject = value.ToHandle( );
        if(subObject.IsNotNull( ))
          RestoreCogPathLinks(subObject, owner, initializer);
      }
    }
  }
}

//**************************************************************************************************
void ZilchComponent::OnAllObjectsCreated(CogInitializer& initializer)
{
  RestoreCogPathLinks(this, GetOwner(), initializer);
}

//**************************************************************************************************
void PopulateDependencies(Component* component, Cog* owner)
{
  BoundType* componentType = ZilchTypeId(Component);
  BoundType* virtualBoundType = ZilchVirtualTypeId(component);
  // Walk through all properties on this Component 
  forRange(Property* property, virtualBoundType->GetProperties())
  {
    // This property isn't marked as a dependency so don't set it to anything.
    // This is likely an internal property that the user will set to something later.
    if(!property->HasAttribute(PropertyAttributes::cDependency))
      continue;

    if(BoundType* propertyType = Type::GetBoundType(property->PropertyType))
    {
      if(propertyType->IsA(componentType))
      {
        Component* componentDependency = owner->QueryComponentType(propertyType);

        if(componentDependency)
        {
          // Set the dependency
          property->SetValue(component, componentDependency);
        }
        else
        {
          String message = String::Format("The '%s' Component depends on the '%s' Component, "
                                          "but the object '%s' does not have that Component.",
                                          componentType->Name.c_str(),
                                          propertyType->Name.c_str(),
                                          owner->GetDescription().c_str());
          DoNotifyError("Cog missing dependencies", message);
        }
      }
    }
  }
}

//**************************************************************************************************
void ZilchComponent::ScriptInitialize(CogInitializer& initializer)
{
  BoundType* thisType = ZilchVirtualTypeId(this);

  // Set the value for all dependencies
  PopulateDependencies(this, this->GetOwner());

  static const String cRuntimeClone = "RuntimeClone";

  // Walk all attributes and search for any RuntimeClones
  forRange(Property* prop, thisType->GetProperties())
  {
    bool isResource = prop->PropertyType->IsA(ZilchTypeId(Resource));
    if(isResource && prop->HasAttribute(cRuntimeClone))
    {
      Any val = prop->GetValue(this);
      
      if(Resource* resource = val.Get<Resource*>())
      {
        // Clone the resource and assign it to the property
        Handle runtimeClone = resource->Clone();
        prop->SetValue(this, runtimeClone);
      }
    }
  }

  // Only run Initialize function if not in editor mode or if has RunInEditor attribute
  bool editorMode = false;
  if (initializer.mGameSession != nullptr)
    editorMode = initializer.mGameSession->IsEditorMode();
  if(initializer.mSpace)
    editorMode |= initializer.mSpace->IsEditorMode();
  
  if(!editorMode || thisType->HasAttributeInherited(ObjectAttributes::cRunInEditor))
  {
    Core& core = Core::GetInstance();

    BoundType* cogInit = ZilchTypeId(CogInitializer);
    ErrorIf(cogInit == nullptr,
      "Could not get the cog initializer type!");

    // This should be cached possibly on the BoundType userdata, or as a meta component... but that might be slower than just looking it up
    Array<Type*> params;
    params.PushBack(cogInit);
    static String FunctionName("Initialize");
    Function* function = thisType->FindFunction(FunctionName, params, core.VoidType, FindMemberOptions::None);

    if (function != nullptr)
    {
      ExceptionReport report;
      Call call(function);

      call.SetHandle(Call::This, this);
      call.SetHandle(0, &initializer);
      call.Invoke(report);
    }
  }
}

//**************************************************************************************************
void ZilchComponent::OnDestroy(uint flags)
{
  BoundType* thisType = ZilchVirtualTypeId(this);

  Core& core = Core::GetInstance();
  static String FunctionName("Destroyed");
  Function* function = thisType->FindFunction(FunctionName, Array<Type*>(), core.VoidType,
                                              FindMemberOptions::None);
  
  if(function != nullptr)
  {
    ExceptionReport report;
    Call call(function);
    call.SetHandle(Call::This, this);
    call.Invoke(report);
  }
}

//**************************************************************************************************
void ZilchComponent::Serialize(Serializer& stream)
{
  Component::Serialize(stream);
  MetaSerializeProperties(this, stream);
}

//**************************************************************************************************
void ZilchComponent::DebugDraw()
{
  BoundType* thisType = ZilchVirtualTypeId(this);

  Zilch::Core& core = Zilch::Core::GetInstance();
  static String FunctionName("DebugDraw");
  Zilch::Function* function = thisType->FindFunction(FunctionName, Array<Zilch::Type*>(),
                                                     core.VoidType, Zilch::FindMemberOptions::None);

  // Do not want to re-invoke Component's DebugDraw, will not find ZilchComponent's DebugDraw because it is not bound
  // Still want find to look for base class methods so that it invokes correctly with inheritance within script.
  if(function != nullptr && function->Owner != ZilchTypeId(Component))
  {
    Zilch::ExceptionReport report;
    Zilch::Call call(function);
    call.SetHandle(Zilch::Call::This, this);
    call.Invoke(report);
  }
}

//**************************************************************************************************
ObjPtr ZilchComponent::GetEventThisObject()
{
  return this;
}

//**************************************************************************************************
void ZilchComponent::Delete()
{
  // Component handles use CogId to dereference the handle. In this case, the Cog is being deleted,
  // so the CogId won't resolve, and attempting to delete the handle will fail.
  // We set the owner to null here to tell the handle manager to store it as a raw pointer, that
  // way it can properly delete it.
  mOwner = nullptr;
  Handle handle(this);
  handle.Delete();
}

//--------------------------------------------------------------------------------------- ZilchEvent
//**************************************************************************************************
ZilchDefineType(ZilchEvent, builder, type)
{
  // If ZilchEvent's created in Zilch were using the same handle manager as C++ Events, they
  // would leak. So, we want just ZilchEvents to be reference counted. We're using HeapManager
  // over ReferenceCountedHandleManager because manually deleting it could be useful.
  type->HandleManager = ZilchManagerId(HeapManager);
  type->CreatableInScript = true;
  ZilchBindConstructor();
  // Do not bind copy constructor. The only time it would be need is if this went from
  // C++ to Zilch (because of HeapManager), and this should never be constructed in C++.
  ZilchBindDestructor();
}

//**************************************************************************************************
void ZilchEvent::Serialize(Serializer& stream)
{
  Event::Serialize(stream);
  MetaSerializeProperties(this, stream);
}

//**************************************************************************************************
void ZilchEvent::Delete()
{
  Handle thisHandle(this);
  thisHandle.Delete();
}

//-------------------------------------------------------------------------------------- ZilchObject
//**************************************************************************************************
ZilchDefineType(ZilchObject, builder, type)
{
  type->CreatableInScript = true;

  ZilchBindMethod(DispatchEvent);
}

//**************************************************************************************************
void ZilchObject::Serialize(Serializer& stream)
{
  EventObject::Serialize(stream);
  MetaSerializeProperties(this, stream);
}

//**************************************************************************************************
void ZilchObject::Delete()
{
  Handle thisHandle(this);
  thisHandle.Delete();
}

//**************************************************************************************************
void ZilchObject::DispatchEvent(StringParam eventId, Event* event)
{
  this->GetDispatcher()->Dispatch(eventId, event);
}

}
