///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String cInvalidTypeName = "void";

//-------------------------------------------------------------------------------- Object Attributes
namespace ObjectAttributes
{

const String cHidden("Hidden");
const String cDocumented("Documented");
const String cExpanded("Expanded");
const String cCore("Core");
const String cStoreLocalModifications("StoreLocalModifications");
const String cProxy("Proxy");
const String cExceptionProxy("ExceptionProxy");
const String cRunInEditor("RunInEditor");
const String cResourceInterface("ResourceInterface");
const String cComponentInterface("ComponentInterface");
const String cGizmo("Gizmo");

}//namespace ObjectFlags

 //------------------------------------------------------------------------------ Property Attributes
namespace PropertyAttributes
{

const String cHidden("Hidden");
const String cInvalidatesObject("InvalidatesObject");
const String cShaderInput("ShaderInput");
const String cLocalModificationOverride("LocalModificationOverride");
const String cAsPropertyUseCustomSerialization("AsPropertyUseCustomSerialization");
const String cSerialized("Serialized");
const String cDependency("Dependency");
const String cEditable("Editable");
const String cResourceProperty("ResourceProperty");
const String cRenamedFrom("RenamedFrom");

}//namespace PropertyFlags

//------------------------------------------------------------------------------ Function Attributes
namespace FunctionAttributes
{

const String cProperty("Property");
const String cInvalidatesObject("InvalidatesObject");

}//namespace FunctionAttributes


 //------------------------------------------------------------------------------------------- Events
namespace Events
{
DefineEvent(PropertyModified);
DefineEvent(PropertyModifiedIntermediate);
DefineEvent(ComponentsModified);
DefineEvent(ObjectModified);
}//namespace Events


//**************************************************************************************************
void PropertyModifiedDefault(HandleParam object, PropertyPathParam property, AnyParam oldValue,
                             AnyParam newValue, bool intermediateChange)
{
  if (Object* zeroObject = object.Get<Object*>())
  {
    // Get the correct event
    String eventId = Events::PropertyModified;
    if (intermediateChange)
      eventId = Events::PropertyModifiedIntermediate;

    // Send the event to the object
    PropertyEvent eventToSend(object, property, oldValue, newValue);
    if(EventDispatcher* dispatcher = zeroObject->GetDispatcher())
      dispatcher->Dispatch(Events::PropertyModified, &eventToSend);
  }
}

//**************************************************************************************************
void ComponentsChangedDefault(HandleParam object)
{
  if (Object* zeroObject = object.Get<Object*>())
  {
    ObjectEvent e(zeroObject);
    if (EventDispatcher* dispatcher = zeroObject->GetDispatcher())
      dispatcher->Dispatch(Events::ComponentsModified, &e);
  }
}

//**************************************************************************************************
void ObjectModifiedDefault(HandleParam object)
{
  if (Object* zeroObject = object.Get<Object*>())
  {
    ObjectEvent e(zeroObject);
    if (EventDispatcher* dispatcher = zeroObject->GetDispatcher())
      dispatcher->Dispatch(Events::ObjectModified, &e);
  }
}

//---------------------------------------------------------------------------------- Meta Operations
//**************************************************************************************************
void MetaOperations::NotifyPropertyModified(HandleParam object, PropertyPathParam property,
                                          AnyParam oldValue, AnyParam newValue,
                                          bool intermediateChange)
{
  Array<Handle> instances;

  // It's most likely that we have Cog/Component/Property, so reserve 3
  // We could impose a depth limit and avoid more heap allocations
  instances.Reserve(3);
  instances.PushBack(object);

  property.GetInstanceHierarchy(object, &instances);

  // The leaf will be at the end
  Handle& leafVariant = instances.Back();
  ReturnIf(leafVariant.IsNull(), , "Instances should always be valid if in undo/redo.");

  // Make sure the property path is always relative to the current object we're sending an event on
  PropertyPath localPath = property;
  forRange(Handle& localObject, instances.All())
  {
    BoundType* objectType = localObject.StoredType;
    if (MetaOperations* metaOps = objectType->HasInherited<MetaOperations>())
      metaOps->PropertyModified(localObject, localPath, oldValue, newValue, intermediateChange);
    else
      PropertyModifiedDefault(localObject, localPath, oldValue, newValue, intermediateChange);

    NotifyObjectModified(localObject);

    localPath.mPath.PopFront();
  }
}

//**************************************************************************************************
void MetaOperations::NotifyComponentsModified(HandleParam object)
{
  BoundType* objectType = object.StoredType;
  if (MetaOperations* metaOps = objectType->HasInherited<MetaOperations>())
  {
    metaOps->ComponentsModified(object);
    metaOps->ObjectModified(object);
  }
  else
  {
    ComponentsChangedDefault(object);
    ObjectModifiedDefault(object);
  }
}

//**************************************************************************************************
void MetaOperations::NotifyObjectModified(HandleParam object)
{
  BoundType* objectType = object.StoredType;
  if (MetaOperations* metaOps = objectType->HasInherited<MetaOperations>())
    metaOps->ObjectModified(object);
  else
    ObjectModifiedDefault(object);
}

//**************************************************************************************************
u64 MetaOperations::GetUndoHandleId(HandleParam object)
{
  return (u64)-1;
}

//**************************************************************************************************
Any MetaOperations::GetUndoData(HandleParam object)
{
  return Any();
}

//**************************************************************************************************
void MetaOperations::PropertyModified(HandleParam object, PropertyPathParam property, 
                                      AnyParam oldValue, AnyParam newValue, bool intermediateChange)
{
  PropertyModifiedDefault(object, property, oldValue, newValue, intermediateChange);
}

//**************************************************************************************************
void MetaOperations::ComponentsModified(HandleParam object)
{
  ComponentsChangedDefault(object);
}

//**************************************************************************************************
void MetaOperations::ObjectModified(HandleParam object)
{
  ObjectModifiedDefault(object);
}

//----------------------------------------------------------------------------------- Property Event
//**************************************************************************************************
ZilchDefineType(PropertyEvent, builder, Type)
{
}

//**************************************************************************************************
PropertyEvent::PropertyEvent(HandleParam object, PropertyPathParam property,
                             AnyParam oldValue, AnyParam newValue) : 
  mObject(object),
  mProperty(property),
  mOldValue(oldValue),
  mNewValue(newValue)
{

}

//--------------------------------------------------------------------------------------- Type Event
//**************************************************************************************************
ZilchDefineType(TypeEvent, builder, Type)
{
}

//**************************************************************************************************
// Getter for bound events extensions e.g. Events.LogicUpdate
void GetEventNameProperty(Call& call, ExceptionReport& report)
{
  Function* currentFunction = call.GetFunction();
  String& eventName = currentFunction->ComplexUserData.ReadObject<String>(0);

  // We have no parameters, no this pointer,
  // so the only thing on the stack is the return value
  call.Set<String>(Call::Return, eventName);
}

//**************************************************************************************************
void BindEventSent(LibraryBuilder& builder, BoundType* boundType, StringParam eventName, BoundType* eventType)
{
  ErrorIf(eventType == nullptr, "Event type must be provided");

  ErrorIf(!eventType->IsA(ZilchTypeId(Event)),
    "Attempting to bind '%s' as an event that isn't an Event type: BindBase(Event)",
    eventType->Name.c_str());

  builder.AddSendsEvent(boundType, eventName, eventType);
}

ZilchDefineType(MetaCustomUi, builder, type)
{
}

ZilchDefineType(MetaOperations, builder, type)
{
}

ZilchDefineType(MetaPropertyFilter, builder, type)
{
}

ZilchDefineType(MetaPropertyBasicFilter, builder, type)
{
}

ZilchDefineType(MetaEditorGizmo, builder, type)
{
}

ZilchDefineType(MetaDisplay, builder, type)
{
}

ZilchDefineType(TypeNameDisplay, builder, type)
{
}

ZilchDefineType(StringNameDisplay, builder, type)
{
}

ZilchDefineType(MetaTransform, builder, type)
{
}

//------------------------------------------------------------------------ TypeNameDisplay
String TypeNameDisplay::GetName(HandleParam object)
{
  return object.StoredType->Name;
}

String TypeNameDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

//------------------------------------------------------------------------ StringNameDisplay
StringNameDisplay::StringNameDisplay(StringParam string)
 : mString(string)
{
}

String StringNameDisplay::GetName(HandleParam)
{
  return mString;
}

String StringNameDisplay::GetDebugText(HandleParam)
{
  return mString;
}

//----------------------------------------------------------------------------------- Meta Transform
//**************************************************************************************************
bool MetaTransformInstance::IsNull()
{
  return mInstance.IsNull();
}

//**************************************************************************************************
bool MetaTransformInstance::IsNotNull()
{
  return !IsNull();
}

//**************************************************************************************************
Vec3 MetaTransformInstance::GetLocalTranslation()
{
  if(mLocalTranslation)
    return mLocalTranslation->GetValue(mInstance).Get<Vec3>();
  return Vec3::cZero;
}

//**************************************************************************************************
Quat MetaTransformInstance::GetLocalRotation()
{
  if(mLocalRotation)
    return mLocalRotation->GetValue(mInstance).Get<Quat>();
  return Quat::cIdentity;
}

//**************************************************************************************************
Vec3 MetaTransformInstance::GetLocalScale()
{
  if(mLocalScale)
    return mLocalScale->GetValue(mInstance).Get<Vec3>();
  return Vec3::cZero;
}

//**************************************************************************************************
void MetaTransformInstance::SetLocalTranslation(Vec3Param value)
{
  if(mLocalTranslation)
    mLocalTranslation->SetValue(mInstance, value);
}

//**************************************************************************************************
void MetaTransformInstance::SetLocalRotation(QuatParam value)
{
  if(mLocalRotation)
    mLocalRotation->SetValue(mInstance, value);
}

//**************************************************************************************************
void MetaTransformInstance::SetLocalScale(Vec3Param value)
{
  if(mLocalScale)
    mLocalScale->SetValue(mInstance, value);
}

//**************************************************************************************************
Vec3 MetaTransformInstance::GetWorldTranslation()
{
  if(mWorldTranslation)
    return mWorldTranslation->GetValue(mInstance).Get<Vec3>();
  return Vec3::cZero;
}

//**************************************************************************************************
Quat MetaTransformInstance::GetWorldRotation()
{
  if(mWorldRotation)
    return mWorldRotation->GetValue(mInstance).Get<Quat>();
  return Quat::cIdentity;
}

//**************************************************************************************************
Vec3 MetaTransformInstance::GetWorldScale()
{
  if(mWorldScale)
    return mWorldScale->GetValue(mInstance).Get<Vec3>();
  return Vec3::cZero;
}

//**************************************************************************************************
Mat4 MetaTransformInstance::GetParentWorldMatrix()
{
  if(mParentWorldMatrix)
    return mParentWorldMatrix->GetValue(mParentInstance).Get<Mat4>();
  return Mat4::cIdentity;
}


//---------------------------------------------------------------------------------- Property Rename
//**************************************************************************************************
ZilchDefineType(MetaPropertyRename, builder, type)
{
}

//**************************************************************************************************
MetaPropertyRename::MetaPropertyRename(StringParam oldName)
  : mOldName(oldName)
{

}

}//namespace Zero
