////////////////////////////////////////////////////////////////////////////////
///
/// \file MultiPropertyInterface.cpp
/// Implementation of the property interface for meta selections.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
Handle GetActingObject(HandleParam componentOrSelection, HandleParam object)
{
  if(object.IsNull())
    return Handle();

  if(componentOrSelection.Get<MetaSelection*>() != nullptr)
    return object;

  BoundType* objectType = componentOrSelection.StoredType;

  MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();
  return composition->GetComponent(object, objectType);
}

//---------------------------------------------------------------- MultiProperty
//******************************************************************************
MultiPropertyInterface::MultiPropertyInterface(OperationQueue* queue, MetaSelection* selection)
{
  mOperationQueue = queue;
  mSelection = selection;
}

//******************************************************************************
void ChangeRotationValue(OperationQueue* queue, HandleParam actingObject, 
                         PropertyPathParam property, PropertyState& state, 
                         PropertyAction::Type action)
{
  Quat oldQuat = property.GetValue(actingObject).Get<Quat>();
  Vec3 oldEuler = Math::QuatToEulerDegrees(oldQuat);

  Vec3 newEuler = state.Value.Get<Vec3>();

  // We only want to use valid values from the new vector, so if they're
  // invalid, grab the values from our old state
  for(uint i = 0; i < 3; ++i)
  {
    if(state.PartialState[i] == PropertyState::Invalid)
      newEuler[i] = oldEuler[i];
  }

  Any newRotation(Math::EulerDegreesToQuat(newEuler));

  // If it's immediate, just set the value directly
  if(action == PropertyAction::Preview)
    property.SetValue(actingObject, newRotation);
  // Otherwise, queue an operation
  else
    ChangeAndQueueProperty(queue, actingObject, property, newRotation);
}

//******************************************************************************
template <typename VectorType, uint Dimensions>
void ChangeVectorValue(OperationQueue* queue, HandleParam actingObject,
                       PropertyPathParam property, PropertyState& state, 
                       PropertyAction::Type action)
{

  VectorType oldVec = property.GetValue(actingObject).Get<VectorType>();
  VectorType newVec = state.Value.Get<VectorType>();

  // We only want to use valid values from the new vector, so if they're
  // invalid, grab the values from our old state
  for(uint i = 0; i < Dimensions; ++i)
  {
    if(state.PartialState[i] == PropertyState::Invalid)
      newVec[i] = oldVec[i];
  }

  Any newValue(newVec);

  // If it's immediate, just set the value directly
  if(action == PropertyAction::Preview)
    property.SetValue(actingObject, newValue);
  // Otherwise, queue an operation
  else
    ChangeAndQueueProperty(queue, actingObject, property, newValue);
}

//******************************************************************************
void MultiPropertyInterface::ChangeProperty(HandleParam object, 
                                   PropertyPathParam property, PropertyState& state,
                                   PropertyAction::Enum action)
{
  // We want all the property changes to be part of the same operation group
  if(action == PropertyAction::Commit)
  {
    mOperationQueue->BeginBatch( );
    mOperationQueue->SetActiveBatchName("MultiPropertyInterface_ChangeProperty");
  }

  // Change the property on each object in the selection
  forRange(Object* instance, mSelection->AllOfType<Object>())
  {
    // Get the acting object (either cog or component)
    Handle actingObject = GetActingObject(object, instance);

    Property* metaProp = property.GetPropertyFromRoot(actingObject);

    // Rotations may be given to use as a Vec4 (in Euler Angles format), so we
    // have to check the HashId of the property. If the property is expecting
    // a quaternion, we have to resolve conflicts, and convert back to a quat
    if(metaProp->PropertyType == ZilchTypeId(Quat))
      ChangeRotationValue(mOperationQueue, actingObject, property, state, action);
    else if(state.Value.Is<Vec2>())
      ChangeVectorValue<Vec2, 2>(mOperationQueue, actingObject, property, state, action);
    else if(state.Value.Is<Vec3>())
      ChangeVectorValue<Vec3, 3>(mOperationQueue, actingObject, property, state, action);
    else if(state.Value.Is<Vec4>())
      ChangeVectorValue<Vec4, 4>(mOperationQueue, actingObject, property, state, action);
    // If it's immediate, just set the value
    else if(action == PropertyAction::Preview)
      property.SetValue(actingObject, state.Value);
    // Otherwise, queue an operation
    else
      ChangeAndQueueProperty(mOperationQueue, actingObject, property, state.Value);

    Handle leaf = property.GetLeafInstance(actingObject);
    Any oldValue = metaProp->GetValue(leaf);
    SendPropertyModifiedOnGrid(actingObject, property, oldValue, state.Value, action);
  }

  if(action == PropertyAction::Commit)
    mOperationQueue->EndBatch();
}

//******************************************************************************
void FinalizeVectorState(PropertyState& state, uint dimensions)
{
  // Count how many invalid properties we have
  uint invalidCount = 0;
  for(uint i = 0; i < dimensions; ++i)
  {
    if(state.PartialState[i] == PropertyState::Invalid)
      ++invalidCount;
  }

  // If they're all invalid, just make the state invalid
  if(invalidCount == dimensions)
  {
    state.State = PropertyState::Invalid;
    return;
  }

  // It's only partially valid if there's at least one invalid value
  if(invalidCount != 0)
    state.State = PropertyState::PartiallyValid;
}

//******************************************************************************
template <typename VectorType, uint Dimensions>
PropertyState GetVectorValue(MetaSelection* selection, AnyParam firstValue, 
                             HandleParam object, PropertyPathParam property)
{
  // Compare each value with the value of the first object
  VectorType firstVec = firstValue.Get<VectorType>();

  PropertyState state(firstValue);

  forRange(Object* instance, selection->AllOfType<Object>())
  {
    Handle actingObject = GetActingObject(object, instance);

    // Get the value of the current object
    Any currValue = property.GetValue(actingObject);
    VectorType currVec = currValue.Get<VectorType>();

    // Check each value and mark it as invalid if it isn't the same
    for(uint i = 0; i < Dimensions; ++i)
    {
      if(firstVec[i] != currVec[i])
        state.PartialState[i] = PropertyState::Invalid;
    }
  }

  // Finalize the state and partial states of this property before returning it
  FinalizeVectorState(state, Dimensions);

  return state;
}

//******************************************************************************
PropertyState GetRotationValue(MetaSelection* selection,Any& firstValue, 
                               HandleParam object, PropertyPathParam property)
{
  /// For rotations, we can only resolve partial conflicts as Euler Angles,
  /// not as quaternions, so we must convert
  Vec3 firstEuler = Math::QuatToEulerDegrees(firstValue.Get<Quat>());

  PropertyState state(firstEuler);

  forRange(Object* instance, selection->AllOfType<Object>())
  {
    Handle actingObject = GetActingObject(object, instance);

    // Get the value of the current object
    Any currValue = property.GetValue(actingObject);
    Vec3 currEuler = Math::QuatToEulerDegrees(currValue.Get<Quat>());

    // Check each value and mark it as invalid if it isn't the same
    for(uint i = 0; i < 3; ++i)
    {
      if(firstEuler[i] != currEuler[i])
        state.PartialState[i] = PropertyState::Invalid;
    }
  }

  // Finalize the state and partial states of this property before returning it
  FinalizeVectorState(state, 3);

  return state;
}

//******************************************************************************
PropertyState MultiPropertyInterface::GetValue(HandleParam multiObject,  PropertyPathParam property)
{
  // Default case
  if(mSelection->Count() == 0)
    return PropertyState();

  // Get the acting object (either cog or component)
  Handle actingObject = GetActingObject(multiObject, mSelection->GetPrimaryAs<Object>());

  // Do nothing if it's invalid
  if(actingObject.IsNull())
    return PropertyState();

  // Compare each value with the value of the first object
  // If they're different, there's a conflict
  Any firstValue = property.GetValue(actingObject);

  /// If it's a vector type, we want to check for partial conflicts
  if(firstValue.Is<Vec2>())
    return GetVectorValue<Vec2, 2>(mSelection, firstValue, multiObject, property);
  if(firstValue.Is<Vec3>())
    return GetVectorValue<Vec3, 3>(mSelection, firstValue, multiObject, property);
  if(firstValue.Is<Vec4>())
    return GetVectorValue<Vec4, 4>(mSelection, firstValue, multiObject, property);
  if(firstValue.Is<Quat>())
    return GetRotationValue(mSelection, firstValue, multiObject, property);

  forRange(Object* subObject, mSelection->AllOfType<Object>())
  {
    actingObject = GetActingObject(multiObject, subObject);

    // Get the value of the current object
    Any currValue = property.GetValue(actingObject);

    // If it's different than the value of the first object, there's a conflict
    if(currValue != firstValue)
    {
      PropertyState state;
      state.Value = firstValue;
      state.State = PropertyState::Invalid;
      return state;
    }
  }

  // There are no conflicts, so return the value
  return PropertyState(firstValue);
}

//******************************************************************************
void MultiPropertyInterface::InvokeFunction(HandleParam object, Function* function)
{
  // We want all add operations to be part of the same operation group
  mOperationQueue->BeginBatch();

  String name;
  if (MetaDisplay* display = object.StoredType->HasInherited<MetaDisplay>())
    name = display->GetName(object);
  else
    name = object.StoredType->Name;

  mOperationQueue->SetActiveBatchName(BuildString("Invoke '", function->Name, "' on '", name, "'"));

  forRange(Object* instance, mSelection->AllOfType<Object>())
  {
    Handle actingObject = GetActingObject(object, instance);

    Any returnValue = function->Invoke(actingObject, nullptr);
  }

  mOperationQueue->EndBatch();
}

//******************************************************************************
HandleOf<MetaComposition> MultiPropertyInterface::GetMetaComposition(BoundType* objectType)
{
  // If the object itself doesn't have a meta composition, we don't want to return our custom one
  if(MetaComposition* objectComposition = objectType->HasInherited<MetaComposition>())
    return new MultiMetaComposition(this, objectType, mOperationQueue);
  return nullptr;
}

//******************************************************************************
ObjectPropertyNode* MultiPropertyInterface::BuildObjectTree(ObjectPropertyNode* parent, 
                                                            HandleParam object, Property* objectProperty)
{
  // We only want to special case this if it's the selection object
  if(!object.Get<MetaSelection*>())
    return PropertyInterface::BuildObjectTree(parent, object, objectProperty);

  BoundType* targetType = GetTargetType();
  if(targetType == NULL)
    return NULL;

  // Create a new node for this object
  ObjectPropertyNode* node = new ObjectPropertyNode(parent, object, objectProperty);

  // Set the custom composition (if it exists)
  node->mComposition = GetMetaComposition(targetType);
  node->mMetaArray = GetMetaArray(targetType);

  // Check for Editable attribute as well as Property attribute (Property implies Editable)
  forRange(Property* property, targetType->GetProperties())
  {
    if(property->HasAttribute(Zilch::PropertyAttribute) || property->HasAttribute(PropertyAttributes::cEditable))
    {
      if(EditorPropertyExtension* extension = property->HasInherited<EditorPropertyExtension>())
        node->mProperties.PushBack(new ObjectPropertyNode(node, property));
      else if(property->PropertyType->HasInherited<MetaPropertyEditor>())
        node->mProperties.PushBack(new ObjectPropertyNode(node, property));
    }
  }

  // Add Methods with no parameters to this node
  forRange(Function* function, targetType->GetFunctions())
  {
    // Don't want to add hidden methods
    if (function->HasAttribute(FunctionAttributes::cProperty))
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
      // Don't show hidden sub objects
      if (component.StoredType->HasAttribute(ObjectAttributes::cHidden))
        continue;

      // Create a new node for this sub object
      ObjectPropertyNode* subNode = BuildObjectTree(node, component);
      node->mContainedObjects.PushBack(subNode);
    }
  }

  return node;
}

//******************************************************************************
void MultiPropertyInterface::GetObjects(HandleParam instance, Array<Handle>& objects)
{
  objects.Reserve(mSelection->Count());
  forRange(Object* instance, mSelection->AllOfType<Object>())
    objects.PushBack(instance);
}

//******************************************************************************
BoundType* MultiPropertyInterface::GetTargetType()
{
  // Find meta that's shared between all objects in the selection
  Handle primary = mSelection->GetPrimaryAs<Object>();
  BoundType* targetMeta = primary.StoredType;

  forRange(Object* object, mSelection->AllOfType<Object>())
  {
    if(targetMeta != ZilchVirtualTypeId(object))
      return NULL;
  }

  return targetMeta;
}

//******************************************************************************
void MultiPropertyInterface::Undo()
{
  mOperationQueue->Undo();
}

//******************************************************************************
void MultiPropertyInterface::Redo()
{
  mOperationQueue->Redo();
}

//******************************************************************************
void MultiPropertyInterface::CaptureState(PropertyStateCapture& capture, HandleParam multiObject,
                                          Property* property)
{
  // For every selected object capture the state
  forRange(Object* objectInstance, mSelection->AllOfType<Object>())
  {
    // Get the component selected
    Handle actingObject = GetActingObject(multiObject, objectInstance);

    // Add state to the capture
    PropertyInterface::CaptureState(capture, actingObject, property);
  }
}

//--------------------------------------------------------------------------- Multi Meta Composition
//******************************************************************************
MultiMetaComposition::MultiMetaComposition(PropertyInterface* propertyInterface,
                                           BoundType* objectType, OperationQueue* opQueue) :
  UndoMetaComposition(propertyInterface, objectType, opQueue)
{
  mSupportsComponentReorder = false;
}

//******************************************************************************
uint MultiMetaComposition::GetComponentCount(HandleParam object)
{
  MetaSelection* selection = object.Get<MetaSelection*>();

  // The count needed is the shared component count
  Array<BoundType*> sharedComponents;
  GetSharedComponents(selection, sharedComponents);

  return sharedComponents.Size();
}

//******************************************************************************
Handle MultiMetaComposition::GetComponentAt(HandleParam object, uint index)
{
  MetaSelection* selection = object.Get<MetaSelection*>();

  // It's the index into the shared component list
  Array<BoundType*> sharedComponents;
  GetSharedComponents(selection, sharedComponents);

  // We need to return an instance, so just return the instance of the primary
  // object in the selection. Nothing should be done directly to this instance
  // as any operation should come through this property interface
  BoundType* componentType = sharedComponents[index];
  Handle primary = selection->GetPrimaryAs<Object>();

  return mContainedComposition->GetComponent(primary, componentType);
}

//******************************************************************************
bool MultiMetaComposition::CanAddComponent(HandleParam object, BoundType* typeToAdd, AddInfo* info)
{
  MetaSelection* selection = object.Get<MetaSelection*>();

  // Check if it can be added to each object in the selection
  forRange(Handle instance, selection->All())
  {
    if(mContainedComposition->CanAddComponent(instance, typeToAdd, info) == false)
      return false;
  }

  return true;
}

//******************************************************************************
void MultiMetaComposition::AddComponent(HandleParam object, BoundType* typeToAdd, int index, bool ignoreDependencies)
{
  ErrorIf(index != -1, "Adding Components at an index through multi-select is not supported");

  // We want all add operations to be part of the same operation group
  mOperationQueue->BeginBatch();

  mOperationQueue->SetActiveBatchName(BuildString("Add '", typeToAdd->Name, "to objects"));

  // Add the component to each object in the selection
  MetaSelection* selection = object.Get<MetaSelection*>();
  forRange(Handle currentObject, selection->All())
    UndoMetaComposition::AddComponent(currentObject, typeToAdd, index, ignoreDependencies);

  mOperationQueue->EndBatch();
}

//******************************************************************************
bool MultiMetaComposition::CanRemoveComponent(HandleParam object, HandleParam component, String& reason)
{
  BoundType* componentType = component.StoredType;

  // Check if we can remove the component type from each object in the selection
  MetaSelection* selection = object.Get<MetaSelection*>();
  forRange(Handle currentObject, selection->All())
  {
    // If we cannot remove the component on this object, we can't remove any
    if(mContainedComposition->CanRemoveComponent(currentObject, componentType, reason) == false)
      return false;
  }

  return true;
}

//******************************************************************************
void MultiMetaComposition::RemoveComponent(HandleParam object, HandleParam component, bool ignoreDependencies)
{
  // We want all remove operations to be part of the same operation group
  mOperationQueue->BeginBatch();

  BoundType* componentType = component.StoredType;

  mOperationQueue->SetActiveBatchName(BuildString("Remove '", componentType->Name, "from objects"));

  // Remove the component type from each object in the selection
  MetaSelection* selection = object.Get<MetaSelection*>();
  forRange(Handle currentObject, selection->All())
  {
    Handle component = GetComponent(currentObject, componentType);
    UndoMetaComposition::RemoveComponent(currentObject, component, ignoreDependencies);
  }

  mOperationQueue->EndBatch();
}

//******************************************************************************
void MultiMetaComposition::Enumerate(Array<BoundType*>& addTypes, EnumerateAction::Enum action, HandleParam object)
{
  MetaSelection* selection = object.Get<MetaSelection*>();

  // Only show the types that all compositions can add / enumerate
  HashMap<BoundType*, uint> filteredList;
  forRange(Object* instance, selection->AllOfType<Object>())
  {
    // Get the addable types for the current object
    Array<BoundType*> currentAddTypes;
    mContainedComposition->Enumerate(currentAddTypes, action, instance);

    // Add each type into the hash map and increment its count
    forRange(BoundType* metaType, currentAddTypes.All())
      filteredList[metaType]++;
  }

  // Add each entry if it's addable for each object
  forRange(auto& entry, filteredList.All())
  {
    if (entry.second == selection->Count())
      addTypes.PushBack(entry.first);
  }
}

//******************************************************************************
void MultiMetaComposition::GetSharedComponents(MetaSelection* selection,
                                               Array<BoundType*>& sharedComponents)
{
  Handle primary = selection->GetPrimaryAs<Object>();
  if (primary.IsNull())
    return;

  // We're using this to filter out any unshared components
  // The array stores all component instances of the key meta type
  // If the array size matches the amount of objects (in mObjects),
  // then you know that all objects have that component
  HashMap<BoundType*, uint> sharedComponentMap;

  forRange(Handle object, selection->All())
  {
    // Add each component
    uint componentCount = mContainedComposition->GetComponentCount(object);
    for (uint i = 0; i < componentCount; ++i)
    {
      Handle component = mContainedComposition->GetComponentAt(object, i);
      sharedComponentMap[component.StoredType]++;
    }
  }

  // Walk the components of the first object (we're using the order of the primary object's components)
  // and check to see if all objects had the component
  uint componentCount = mContainedComposition->GetComponentCount(primary);
  for (uint i = 0; i < componentCount; ++i)
  {
    // Get the component meta
    Handle component = mContainedComposition->GetComponentAt(primary, i);
    BoundType* componentMeta = component.StoredType;

    // If all objects in the selection have the component,
    // add it to be displayed
    uint count = sharedComponentMap.FindValue(componentMeta, uint(-1));
    if (count == selection->Count())
      sharedComponents.PushBack(componentMeta);
  }
}

}//namespace Zero
