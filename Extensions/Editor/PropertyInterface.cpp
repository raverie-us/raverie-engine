////////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyInterface.hpp
/// Declaration of the property interface.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2013, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------- Object Property Node
//******************************************************************************
ObjectPropertyNode::ObjectPropertyNode(ObjectPropertyNode* parent, HandleParam object, Property* objectProperty) :
  mParent(parent),
  mProperty(objectProperty),
  mObject(object)
{
  
}

//******************************************************************************
ObjectPropertyNode::ObjectPropertyNode(ObjectPropertyNode* parent, Property* property) :
  mParent(parent),
  mProperty(property),
  mObject(nullptr)
{

}

//******************************************************************************
ObjectPropertyNode::~ObjectPropertyNode()
{
  DeleteObjectsInContainer(mProperties);
  DeleteObjectsInContainer(mContainedObjects);
}

//--------------------------------------------------------------- Property State
//******************************************************************************
PropertyState::PropertyState()
{
  // By default, everything is invalid
  State = PropertyState::Invalid;
  PartialState[0] = Invalid;
  PartialState[1] = Invalid;
  PartialState[2] = Invalid;
  PartialState[3] = Invalid;
}

//******************************************************************************
PropertyState::PropertyState(AnyParam value, Enum state)
{
  Value = value;
  State = state;
  PartialState[0] = state;
  PartialState[1] = state;
  PartialState[2] = state;
  PartialState[3] = state;
}

//******************************************************************************
bool PropertyState::IsValid()
{
  return State == PropertyState::Valid;
}

//******************************************************************************
bool PropertyState::IsPartiallyValid()
{
  return State == PropertyState::PartiallyValid;
}

//----------------------------------------------------------- Property Interface
//******************************************************************************
PropertyInterface::PropertyInterface()
{
  mPropertyGrid = nullptr;
}

//******************************************************************************
void PropertyInterface::ChangeProperty(HandleParam object, PropertyPathParam propertyPath,
                                       PropertyState& state, PropertyAction::Enum action)
{
  Any oldValue = propertyPath.GetValue(object);

  propertyPath.SetValue(object, state.Value);

  // Specific types of objects need to do extra logic when properties are modified (e.g. modifications
  // to Cogs need to mark the Space as modified.
  bool intermediate = (action == PropertyAction::Preview);
  MetaOperations::NotifyPropertyModified(object, propertyPath, oldValue, state.Value, intermediate);

  // Send the event on the property grid notifying of the change
  SendPropertyModifiedOnGrid(object, propertyPath, oldValue, state.Value, action);
}

//******************************************************************************
PropertyState PropertyInterface::GetValue(HandleParam object, PropertyPathParam property)
{
  Any currValue = property.GetValue(object);
  return PropertyState(currValue);
}

//******************************************************************************
void PropertyInterface::InvokeFunction(HandleParam object, Function* method)
{
  method->Invoke(object, nullptr);
}

//******************************************************************************
HandleOf<MetaComposition> PropertyInterface::GetMetaComposition(BoundType* objectType)
{
  // If the object itself doesn't have a meta composition, we don't want to return our custom one
  if(objectType->HasInherited<MetaComposition>())
    return new EventMetaComposition(this, objectType);
  return nullptr;
}

//******************************************************************************
HandleOf<MetaArray> PropertyInterface::GetMetaArray(BoundType* objectType)
{
  // If the object itself doesn't have an array composition, we don't want to return our custom one
  if(objectType->HasInherited<MetaArray>())
    return new EventMetaArray(objectType, this);
  
  return nullptr;
}

//******************************************************************************
ObjectPropertyNode* PropertyInterface::BuildObjectTree(ObjectPropertyNode* parent, HandleParam object, Property* objectProperty)
{
  ReturnIf(object.IsNull(), nullptr, "Invalid object.");

  BoundType* objectType = object.StoredType;

  // Create a new node for this object
  ObjectPropertyNode* node = new ObjectPropertyNode(parent, object, objectProperty);

  // Set the custom composition (if it exists)
  node->mComposition = GetMetaComposition(objectType);
  node->mMetaArray = GetMetaArray(objectType);

  // Add properties to this node
  forRange(Property* property, objectType->GetProperties())
  {
    if(BoundType* boundType = Type::GetBoundType(property->PropertyType))
      ErrorIf(boundType->Name.Empty(), "Forget to bind an enum?");

    // Check for Editable attribute as well as Property attribute (Property implies Editable)
    if(property->HasAttribute(Zilch::PropertyAttribute) || property->HasAttribute(PropertyAttributes::cEditable))
    {
      if(EditorPropertyExtension* extension = property->HasInherited<EditorPropertyExtension>())
      {
        node->mProperties.PushBack(new ObjectPropertyNode(node, property));
      }
      else if(property->PropertyType->HasInherited<MetaPropertyEditor>())
      {
        node->mProperties.PushBack(new ObjectPropertyNode(node, property));
      }
      else
      {
        Handle propertyObject = property->GetValue(object).ToHandle();
        if(propertyObject.IsNotNull())
          node->mProperties.PushBack(BuildObjectTree(node, propertyObject, property));
      }
    }
  }

  // Add Methods with no parameters to this node
  forRange(Function* function, objectType->GetFunctions())
  {
    // Don't want to add hidden methods
    if(function->HasAttribute(FunctionAttributes::cProperty))
    {
      // METAREFACTOR - 0 param
      // We can only display methods with 0 parameters
      //if(function->HasOverloadWithNoParams)
        node->mFunctions.PushBack(function);
    }
  }

  // Add dynamically contained objects to this node
  if(MetaComposition* composition = node->mComposition)
  {
    forRange(Handle component, composition->AllComponents(object))
    {

      if (component.StoredType == nullptr)
      {
        Error("Contained object does not have meta initialized.");
        continue;
      }

      // Don't show hidden sub objects
      if(component.StoredType->HasAttribute(ObjectAttributes::cHidden))
        continue;

      // Create a new node for this sub object
      ObjectPropertyNode* subNode = BuildObjectTree(node, component);
      node->mContainedObjects.PushBack(subNode);
    }
  }

  // Add array objects
  if(MetaArray* metaArray = node->mMetaArray)
  {
    forRange(Any arrayValue, metaArray->All(object))
    {
      // METAREFACTOR - for now, we only support objects that can be handles in MetaArray. This will need
      // to change once that is no longer true
      Handle arrayObject = arrayValue.ToHandle();

      // Create a new node for this sub object
      ObjectPropertyNode* subNode = BuildObjectTree(node, arrayObject);
      node->mContainedObjects.PushBack(subNode);
    }
  }

  return node;
}

//******************************************************************************
void PropertyInterface::GetObjects(HandleParam instance,  Array<Handle>& objects)
{
  objects.PushBack(instance);
}

//******************************************************************************
void PropertyInterface::CaptureState(PropertyStateCapture& capture, HandleParam object,
                                     Property* property)
{
  // Capture the property's value for this object
  Any currValue = property->GetValue(object);
  PropertyStateCapture::CapturedProperty& captured = capture.Properties.PushBack();
  captured.Object = object;
  captured.Value = currValue;
  captured.Property = property;
}

//******************************************************************************
void PropertyInterface::RestoreState(PropertyStateCapture& capture)
{
  // Restore all objects the state
  forRange(PropertyStateCapture::CapturedProperty& captured, capture.Properties.All())
    captured.Property->SetValue(captured.Object, captured.Value);
}

//******************************************************************************
void PropertyInterface::SendPropertyModifiedOnGrid(HandleParam object, PropertyPathParam property,
                                                   AnyParam oldValue, AnyParam newValue,
                                                   PropertyAction::Enum action)
{
  // Dispatch an event on the property grid
  PropertyEvent eventToSend(object, property, oldValue, newValue);

  if (action == PropertyAction::Commit)
    mPropertyGrid->GetDispatcher()->Dispatch(Events::PropertyModified, &eventToSend);
  else if (action == PropertyAction::Preview)
    mPropertyGrid->GetDispatcher()->Dispatch(Events::PropertyModifiedIntermediate, &eventToSend);

  ObjectEvent e;
  e.Source = object.Get<Object*>();
  mPropertyGrid->GetDispatcher()->Dispatch(Events::ObjectModified, &e);
}

//******************************************************************************
void PropertyInterface::SendComponentsModifiedOnGrid(HandleParam object)
{
  ObjectEvent e;
  e.Source = object.Get<Object*>();
  mPropertyGrid->GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
  mPropertyGrid->GetDispatcher()->Dispatch(Events::ObjectModified, &e);
}

//--------------------------------------------------------------------------- Event Meta Composition
//**************************************************************************************************
EventMetaComposition::EventMetaComposition(PropertyInterface* propertyInterface, BoundType* typeToWrap) :
  MetaCompositionWrapper(typeToWrap),
  mPropertyInterface(propertyInterface)
{

}

//**************************************************************************************************
void EventMetaComposition::AddComponent(HandleParam owner, BoundType* typeToAdd, int index,
                                        bool ignoreDependencies)
{
  MetaComposition* composition = owner.StoredType->HasInherited<MetaComposition>();
  composition->AddComponent(owner, typeToAdd, index, ignoreDependencies);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(owner);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

//**************************************************************************************************
void EventMetaComposition::RemoveComponent(HandleParam owner, HandleParam component,
                                           bool ignoreDependencies)
{
  MetaComposition* composition = owner.StoredType->HasInherited<MetaComposition>();
  composition->RemoveComponent(owner, component, ignoreDependencies);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(owner);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

//**************************************************************************************************
void EventMetaComposition::MoveComponent(HandleParam owner, HandleParam component, uint destination)
{
  MetaComposition* composition = owner.StoredType->HasInherited<MetaComposition>();
  composition->MoveComponent(owner, component, destination);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(owner);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

//--------------------------------------------------------------------------------- Event Meta Array
//**************************************************************************************************
EventMetaArray::EventMetaArray(BoundType* containedType, PropertyInterface* propertyInterface) :
  MetaArrayWrapper(containedType),
  mPropertyInterface(propertyInterface)
{

}

//**************************************************************************************************
void EventMetaArray::Add(HandleParam container, AnyParam value)
{
  MetaArray* metaArray = container.StoredType->HasInherited<MetaArray>();
  metaArray->Add(container, value);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(container);
  mPropertyInterface->SendComponentsModifiedOnGrid(container);
}

//**************************************************************************************************
void EventMetaArray::EraseIndex(HandleParam container, uint index)
{
  MetaArray* metaArray = container.StoredType->HasInherited<MetaArray>();
  metaArray->EraseIndex(container, index);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(container);
  mPropertyInterface->SendComponentsModifiedOnGrid(container);
}

}//namespace Zero
