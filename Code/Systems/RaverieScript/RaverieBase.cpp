// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Raverie Component
RaverieDefineType(RaverieComponent, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  type->Sealed = false;

  type->Add(new MetaSerialization());

  // Temporary solution so that RaverieComponent cannot be added on Cog in the
  // property grid
  type->HasOrAdd<CogComponentMeta>(type)->mSetupMode = SetupMode::FromDataOnly;
}

void RestoreCogPathLinks(HandleParam object, Cog* owner, CogInitializer& initializer)
{
  BoundType* cogPathType = RaverieTypeId(CogPath);
  BoundType* objectType = object.StoredType;

  // Cog paths and some certain properties need to have a special resolve phase
  // called on them. We only loop over fields because properties could have side
  // effects (get/set) and generally always have a backing field, unless they
  // are generating a temporary, in which case a temporary CogPath doesn't make
  // sense to restore links on.
  forRange (Field* field, objectType->GetFields())
  {
    BoundType* propertyType = Type::DirectDynamicCast<BoundType*>(field->PropertyType);

    if (propertyType == nullptr || propertyType->CopyMode != TypeCopyMode::ReferenceType)
      continue;

    if (field->HasAttribute(PropertyAttributes::cProperty) || field->HasAttribute(PropertyAttributes::cDisplay) || field->HasAttribute(PropertyAttributes::cDeprecatedEditable) ||
        field->HasAttribute(PropertyAttributes::cSerialize) || field->HasAttribute(PropertyAttributes::cDeprecatedSerialized))
    {
      Any value = field->GetValue(object);
      if (propertyType == cogPathType)
      {
        CogPath* cogPath = value.Get<CogPath*>();
        if (cogPath != nullptr)
        {
          cogPath->RestoreLink(initializer, owner, field->Name);
        }
        else
        {
          // We set the cog path here because we always expect cog paths to be
          // non-null.
          CogPath path;
          path.SetRelativeTo(owner);
          field->SetValue(object, path);
        }
      }
      else
      {
        Handle subObject = value.ToHandle();

        // Native types must handle this manually
        bool isNative = (subObject.StoredType && subObject.StoredType->Native);
        if (!isNative && subObject.IsNotNull())
          RestoreCogPathLinks(subObject, owner, initializer);
      }
    }
  }
}

void RaverieComponent::OnAllObjectsCreated(CogInitializer& initializer)
{
  RestoreCogPathLinks(this, GetOwner(), initializer);
}

void PopulateDependencies(Component* component, Cog* owner)
{
  BoundType* componentType = RaverieTypeId(Component);
  BoundType* virtualBoundType = RaverieVirtualTypeId(component);
  // Walk through all properties on this Component
  forRange (Property* property, virtualBoundType->GetProperties())
  {
    // This property isn't marked as a dependency so don't set it to anything.
    // This is likely an internal property that the user will set to something
    // later.
    if (!property->HasAttribute(PropertyAttributes::cDependency))
      continue;

    if (BoundType* propertyType = Type::GetBoundType(property->PropertyType))
    {
      if (propertyType->IsA(componentType))
      {
        Component* componentDependency = owner->QueryComponentType(propertyType);

        if (componentDependency)
        {
          // Set the dependency
          property->SetValue(component, componentDependency);
        }
        else
        {
          String message = String::Format("The '%s' Component depends on the '%s' Component, "
                                          "but the object '%s' does not have that Component.",
                                          virtualBoundType->Name.c_str(),
                                          propertyType->Name.c_str(),
                                          owner->GetDescription().c_str());
          DoNotifyWarning("Cog missing dependencies", message);
        }
      }
    }
  }
}

void RaverieComponent::ScriptInitialize(CogInitializer& initializer)
{
  BoundType* thisType = RaverieVirtualTypeId(this);

  // Set the value for all dependencies
  PopulateDependencies(this, this->GetOwner());

  // Walk all attributes and search for any RuntimeClones
  forRange (Property* prop, thisType->GetProperties())
  {
    Type* propertyType = prop->PropertyType;
    bool isResource = propertyType->IsA(RaverieTypeId(Resource));
    if (isResource && prop->HasAttribute(PropertyAttributes::cRuntimeClone) && prop->Set != nullptr)
    {
      Any val = prop->GetValue(this);

      if (Resource* resource = val.Get<Resource*>())
      {
        // Clone the resource and assign it to the property
        Handle runtimeClone = resource->Clone();
        prop->SetValue(this, runtimeClone);
      }
    }

    // If this is any generic handle to an object that has a default constructor
    // and a get/set and the object is currently null, then create it (CogPath,
    // custom objects, etc)
    BoundType* boundPropertyType = Type::GetBoundType(propertyType);
    if (boundPropertyType)
    {
      bool isProperty = prop->HasAttribute(PropertyAttributes::cProperty) || prop->HasAttribute(PropertyAttributes::cDisplay) || prop->HasAttribute(PropertyAttributes::cDeprecatedEditable) ||
                        prop->HasAttribute(PropertyAttributes::cSerialize) || prop->HasAttribute(PropertyAttributes::cDeprecatedSerialized);

      bool isGetSetProperty = isProperty && prop->Get && prop->Set;

      bool isHandleWithDefaultConstructor = boundPropertyType->CopyMode == TypeCopyMode::ReferenceType && boundPropertyType->CreatableInScript && boundPropertyType->IsDefaultConstructable();

      if (isGetSetProperty && isHandleWithDefaultConstructor)
      {
        Any currentObject = prop->GetValue(this);
        if (currentObject.IsNull())
        {
          Handle newObject = RaverieAllocateUntyped(boundPropertyType);
          prop->SetValue(this, newObject);
        }
      }
    }
  }

  // Only run Initialize function if not in editor mode or if has RunInEditor
  // attribute
  bool editorMode = false;
  if (initializer.mSpace)
    editorMode = initializer.mSpace->IsEditorMode();
  else if (initializer.mGameSession != nullptr)
    editorMode = initializer.mGameSession->IsEditorMode();

  if (!editorMode || thisType->HasAttributeInherited(ObjectAttributes::cRunInEditor))
  {
    Core& core = Core::GetInstance();

    BoundType* cogInit = RaverieTypeId(CogInitializer);
    ErrorIf(cogInit == nullptr, "Could not get the cog initializer type!");

    // This should be cached possibly on the BoundType userdata, or as a meta
    // component... but that might be slower than just looking it up
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

void RaverieComponent::OnDestroy(uint flags)
{
  BoundType* thisType = RaverieVirtualTypeId(this);

  Core& core = Core::GetInstance();
  static String FunctionName("Destroyed");
  Function* function = thisType->FindFunction(FunctionName, Array<Type*>(), core.VoidType, FindMemberOptions::None);

  if (function != nullptr)
  {
    ExceptionReport report;
    Call call(function);
    call.SetHandle(Call::This, this);
    call.Invoke(report);
  }
}

void RaverieComponent::Serialize(Serializer& stream)
{
  Component::Serialize(stream);
  MetaSerializeProperties(this, stream);
}

void RaverieComponent::DebugDraw()
{
  BoundType* thisType = RaverieVirtualTypeId(this);

  Raverie::Core& core = Raverie::Core::GetInstance();
  static String FunctionName("DebugDraw");
  Raverie::Function* function = thisType->FindFunction(FunctionName, Array<Raverie::Type*>(), core.VoidType, Raverie::FindMemberOptions::None);

  // Do not want to re-invoke Component's DebugDraw, will not find
  // RaverieComponent's DebugDraw because it is not bound Still want find to look
  // for base class methods so that it invokes correctly with inheritance within
  // script.
  if (function != nullptr && function->Owner != RaverieTypeId(Component))
  {
    Raverie::ExceptionReport report;
    Raverie::Call call(function);
    call.SetHandle(Raverie::Call::This, this);
    call.Invoke(report);
  }
}

ObjPtr RaverieComponent::GetEventThisObject()
{
  return this;
}

void RaverieComponent::Delete()
{
  // Component handles use CogId to dereference the handle. In this case, the
  // Cog is being deleted, so the CogId won't resolve, and attempting to delete
  // the handle will fail. We set the owner to null here to tell the handle
  // manager to store it as a raw pointer, that way it can properly delete it.
  mOwner = nullptr;
  Handle handle(this);
  handle.Delete();
}

// RaverieEvent
RaverieDefineType(RaverieEvent, builder, type)
{
  type->Sealed = false;

  // If RaverieEvent's created in Raverie were using the same handle manager as C++
  // Events, they would leak. So, we want just RaverieEvents to be reference
  // counted. We're using HeapManager over ReferenceCountedHandleManager because
  // manually deleting it could be useful.
  type->HandleManager = RaverieManagerId(HeapManager);
  type->CreatableInScript = true;
  RaverieBindConstructor();
  // Do not bind copy constructor. The only time it would be need is if this
  // went from C++ to Raverie (because of HeapManager), and this should never be
  // constructed in C++.
  RaverieBindDestructor();
}

void RaverieEvent::Serialize(Serializer& stream)
{
  Event::Serialize(stream);
  MetaSerializeProperties(this, stream);
}

void RaverieEvent::Delete()
{
  Handle thisHandle(this);
  thisHandle.Delete();
}

// RaverieObject
RaverieDefineType(RaverieObject, builder, type)
{
  type->HandleManager = RaverieManagerId(HeapManager);
  type->Sealed = false;

  type->CreatableInScript = true;

  RaverieBindMethod(DispatchEvent);

  RaverieBindConstructor();
  // Do not bind copy constructor. The only time it would be need is if this
  // went from C++ to Raverie (because of HeapManager), and this should never be
  // constructed in C++.
  RaverieBindDestructor();
}

void RaverieObject::Serialize(Serializer& stream)
{
  EventObject::Serialize(stream);
  MetaSerializeProperties(this, stream);
}

void RaverieObject::Delete()
{
  Handle thisHandle(this);
  thisHandle.Delete();
}

void RaverieObject::DispatchEvent(StringParam eventId, Event* event)
{
  this->GetDispatcher()->Dispatch(eventId, event);
}

} // namespace Raverie
