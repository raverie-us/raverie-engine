// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ObjectPropertyNode::ObjectPropertyNode(ObjectPropertyNode* parent, HandleParam object, Property* objectProperty) :
    mParent(parent),
    mObject(object),
    mProperty(objectProperty)
{
}

ObjectPropertyNode::ObjectPropertyNode(ObjectPropertyNode* parent, Property* property) :
    mParent(parent),
    mObject(nullptr),
    mProperty(property)
{
}

ObjectPropertyNode::~ObjectPropertyNode()
{
  DeleteObjectsInContainer(mProperties);
  DeleteObjectsInContainer(mContainedObjects);
}

void ObjectPropertyNode::ReleaseHandles()
{
  mObject = Handle();
  forRange (ObjectPropertyNode* childNode, mProperties)
    childNode->ReleaseHandles();
  forRange (ObjectPropertyNode* childNode, mContainedObjects)
    childNode->ReleaseHandles();
}

bool ObjectPropertyNode::IsPropertyGroup()
{
  return !mPropertyGroupName.Empty();
}

size_t ObjectPropertyNode::GetDepth()
{
  size_t depth = 0;
  ObjectPropertyNode* parent = mParent;

  while (parent)
  {
    ++depth;
    parent = parent->mParent;
  }

  return depth;
}

ObjectPropertyNode* ObjectPropertyNode::FindChildGroup(StringRange groupName)
{
  forRange (ObjectPropertyNode* child, mContainedObjects.All())
  {
    if (child->mPropertyGroupName == groupName)
      return child;
  }

  return nullptr;
}

PropertyState::PropertyState()
{
  // By default, everything is invalid
  State = PropertyState::Invalid;
  PartialState[0] = Invalid;
  PartialState[1] = Invalid;
  PartialState[2] = Invalid;
  PartialState[3] = Invalid;
}

PropertyState::PropertyState(AnyParam value, Enum state)
{
  Value = value;
  State = state;
  PartialState[0] = state;
  PartialState[1] = state;
  PartialState[2] = state;
  PartialState[3] = state;
}

bool PropertyState::IsValid()
{
  return State == PropertyState::Valid;
}

bool PropertyState::IsPartiallyValid()
{
  return State == PropertyState::PartiallyValid;
}

PropertyInterface::PropertyInterface()
{
  mPropertyGrid = nullptr;
}

void PropertyInterface::ChangeProperty(HandleParam object,
                                       PropertyPathParam propertyPath,
                                       PropertyState& state,
                                       PropertyAction::Enum action)
{
  Any oldValue = propertyPath.GetValue(object);

  propertyPath.SetValue(object, state.Value);

  // Specific types of objects need to do extra logic when properties are
  // modified (e.g. modifications to Cogs need to mark the Space as modified.
  bool intermediate = (action == PropertyAction::Preview);
  MetaOperations::NotifyPropertyModified(object, propertyPath, oldValue, state.Value, intermediate);

  // Send the event on the property grid notifying of the change
  SendPropertyModifiedOnGrid(object, propertyPath, oldValue, state.Value, action);
}

void PropertyInterface::MarkPropertyModified(HandleParam object, PropertyPathParam property)
{
  BoundType* objectType = object.StoredType;
  if (MetaDataInheritance* inheritance = objectType->HasInherited<MetaDataInheritance>())
    inheritance->SetPropertyModified(object, property, true);
}

void PropertyInterface::RevertProperty(HandleParam object, PropertyPathParam property)
{
  BoundType* objectType = object.StoredType;
  if (MetaDataInheritance* inheritance = objectType->HasInherited<MetaDataInheritance>())
    inheritance->RevertProperty(object, property);
}

PropertyState PropertyInterface::GetValue(HandleParam object, PropertyPathParam property)
{
  Any currValue = property.GetValue(object);
  return PropertyState(currValue);
}

void PropertyInterface::InvokeFunction(HandleParam object, Function* method)
{
  method->Invoke(object, nullptr);
}

HandleOf<MetaComposition> PropertyInterface::GetMetaComposition(BoundType* objectType)
{
  // If the object itself doesn't have a meta composition, we don't want to
  // return our custom one
  if (objectType->HasInherited<MetaComposition>())
    return new EventMetaComposition(this, objectType);
  return nullptr;
}

HandleOf<MetaArray> PropertyInterface::GetMetaArray(BoundType* objectType)
{
  // If the object itself doesn't have an array composition, we don't want to
  // return our custom one
  if (objectType->HasInherited<MetaArray>())
    return new EventMetaArray(objectType, this);

  return nullptr;
}

void AddProperty(Property* property,
                 ObjectPropertyNode* parent,
                 HandleParam object,
                 PropertyInterface* propertyInterface)
{
  if (EditorPropertyExtension* extension = property->HasInherited<EditorPropertyExtension>())
  {
    parent->mProperties.PushBack(new ObjectPropertyNode(parent, property));
  }
  else if (property->PropertyType->HasInherited<MetaPropertyEditor>())
  {
    parent->mProperties.PushBack(new ObjectPropertyNode(parent, property));
  }
  else
  {
    Handle propertyObject = property->GetValue(object).ToHandle();
    if (propertyObject.IsNotNull())
      parent->mProperties.PushBack(propertyInterface->BuildObjectTree(parent, propertyObject, property));
  }
}

void AddGroup(ObjectPropertyNode* parentNode,
              StringRange groupPath,
              Property* property,
              HandleParam object,
              Property* objectProperty,
              PropertyInterface* propertyInterface)
{
  StringTokenRange r = StringTokenRange(groupPath, '/');
  String groupName = r.Front();

  ObjectPropertyNode* groupNode = parentNode->FindChildGroup(groupName);
  if (groupNode == nullptr)
  {
    groupNode = new ObjectPropertyNode(parentNode, object, objectProperty);
    parentNode->mContainedObjects.PushBack(groupNode);
    groupNode->mPropertyGroupName = groupName;
  }

  if (!r.internalRange.Empty())
    AddGroup(groupNode, r.internalRange, property, object, objectProperty, propertyInterface);
  else
    AddProperty(property, groupNode, object, propertyInterface);
}

ObjectPropertyNode* PropertyInterface::BuildObjectTree(ObjectPropertyNode* parent,
                                                       HandleParam object,
                                                       Property* objectProperty)
{
  ReturnIf(object.IsNull(), nullptr, "Invalid object.");

  BoundType* objectType = object.StoredType;

  // Create a new node for this object
  ObjectPropertyNode* node = new ObjectPropertyNode(parent, object, objectProperty);

  // Set the custom composition (if it exists)
  node->mComposition = GetMetaComposition(objectType);
  node->mMetaArray = GetMetaArray(objectType);

  // Add properties to this node
  forRange (Property* property, objectType->GetProperties())
  {
    if (BoundType* boundType = Type::GetBoundType(property->PropertyType))
      ErrorIf(boundType->Name.Empty(), "Forget to bind an enum?");

    // Check for Editable attribute as well as Property attribute (Property
    // implies Editable)
    if (property->HasAttribute(PropertyAttributes::cProperty) || property->HasAttribute(PropertyAttributes::cDisplay) ||
        property->HasAttribute(PropertyAttributes::cDeprecatedEditable))
    {
      if (MetaGroup* group = property->Has<MetaGroup>())
      {
        AddGroup(node, group->mName, property, object, objectProperty, this);
        continue;
      }

      AddProperty(property, node, object, this);
    }
  }

  // Add Methods with no parameters to this node
  forRange (Function* function, objectType->GetFunctions())
  {
    // Don't want to add hidden methods
    if (function->HasAttribute(FunctionAttributes::cProperty) || function->HasAttribute(FunctionAttributes::cDisplay))
    {
      // We can only display methods with 0 parameters
      if (function->FunctionType->Parameters.Empty())
        node->mFunctions.PushBack(function);
    }
  }

  // Add dynamically contained objects to this node
  if (MetaComposition* composition = node->mComposition)
  {
    forRange (Handle component, composition->AllComponents(object))
    {

      if (component.StoredType == nullptr)
      {
        Error("Contained object does not have meta initialized.");
        continue;
      }

      // Don't show hidden sub objects
      if (component.StoredType->HasAttribute(ObjectAttributes::cHidden))
        continue;

      // Create a new node for this sub object
      ObjectPropertyNode* subNode = BuildObjectTree(node, component);
      node->mContainedObjects.PushBack(subNode);
    }
  }

  // Add array objects
  if (MetaArray* metaArray = node->mMetaArray)
  {
    forRange (Any arrayValue, metaArray->All(object))
    {
      // METAREFACTOR - for now, we only support objects that can be handles in
      // MetaArray. This will need to change once that is no longer true
      Handle arrayObject = arrayValue.ToHandle();

      // Create a new node for this sub object
      ObjectPropertyNode* subNode = BuildObjectTree(node, arrayObject);
      node->mContainedObjects.PushBack(subNode);
    }
  }

  return node;
}

void PropertyInterface::GetObjects(HandleParam instance, Array<Handle>& objects)
{
  objects.PushBack(instance);
}

void PropertyInterface::CaptureState(PropertyStateCapture& capture, HandleParam object, PropertyPathParam property)
{
  // Capture the property's value for this object
  Any currValue = property.GetValue(object);
  PropertyStateCapture::CapturedProperty& captured = capture.Properties.PushBack();
  captured.Object = object;
  captured.Value = currValue;
  captured.Property = property;
}

void PropertyInterface::RestoreState(PropertyStateCapture& capture)
{
  // Restore all objects the state
  forRange (PropertyStateCapture::CapturedProperty& captured, capture.Properties.All())
  {
    Handle object = captured.Object.GetHandle();
    captured.Property.SetValue(object, captured.Value);
  }
}

void PropertyInterface::SendPropertyModifiedOnGrid(
    HandleParam object, PropertyPathParam property, AnyParam oldValue, AnyParam newValue, PropertyAction::Enum action)
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

void PropertyInterface::SendComponentsModifiedOnGrid(HandleParam object)
{
  ObjectEvent e;
  e.Source = object.Get<Object*>();
  mPropertyGrid->GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
  mPropertyGrid->GetDispatcher()->Dispatch(Events::ObjectModified, &e);
}

// Event Meta Composition
EventMetaComposition::EventMetaComposition(PropertyInterface* propertyInterface, BoundType* typeToWrap) :
    MetaCompositionWrapper(typeToWrap),
    mPropertyInterface(propertyInterface)
{
}

void EventMetaComposition::AddComponent(
    HandleParam owner, BoundType* typeToAdd, int index, bool ignoreDependencies, MetaCreationContext* creationContext)
{
  MetaComposition* composition = owner.StoredType->HasInherited<MetaComposition>();
  composition->AddComponent(owner, typeToAdd, index, ignoreDependencies, creationContext);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(owner);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

void EventMetaComposition::RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies)
{
  MetaComposition* composition = owner.StoredType->HasInherited<MetaComposition>();
  composition->RemoveComponent(owner, component, ignoreDependencies);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(owner);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

void EventMetaComposition::MoveComponent(HandleParam owner, HandleParam component, uint destination)
{
  MetaComposition* composition = owner.StoredType->HasInherited<MetaComposition>();
  composition->MoveComponent(owner, component, destination);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(owner);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

// Event Meta Array
EventMetaArray::EventMetaArray(BoundType* containedType, PropertyInterface* propertyInterface) :
    MetaArrayWrapper(containedType),
    mPropertyInterface(propertyInterface)
{
}

void EventMetaArray::Add(HandleParam container, AnyParam value)
{
  MetaArray* metaArray = container.StoredType->HasInherited<MetaArray>();
  metaArray->Add(container, value);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(container);
  mPropertyInterface->SendComponentsModifiedOnGrid(container);
}

void EventMetaArray::EraseIndex(HandleParam container, uint index)
{
  MetaArray* metaArray = container.StoredType->HasInherited<MetaArray>();
  metaArray->EraseIndex(container, index);

  // Send events on both the object and the property grid
  MetaOperations::NotifyComponentsModified(container);
  mPropertyInterface->SendComponentsModifiedOnGrid(container);
}

} // namespace Zero
