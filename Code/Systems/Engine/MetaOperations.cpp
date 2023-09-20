// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

String GetNameFromHandle(HandleParam object)
{
  if (MetaDisplay* display = object.StoredType->HasInherited<MetaDisplay>())
    return display->GetName(object);
  else
    return object.StoredType->Name;
}

void ChangeAndQueueProperty(OperationQueue* queue, HandleParam object, PropertyPathParam property, const Any& newValue)
{
  OperationQueue::StartListeningForSideEffects();

  Any curValue = property.GetValue(object);
  PropertyOperation* propertyOperation = new PropertyOperation(object, property, curValue, newValue);

  ErrorIf(object == nullptr, "Invalid objects given to ChangeAndQueueProperty");

  queue->BeginBatch();
  queue->SetActiveBatchName(BuildString(propertyOperation->mName, " Batch"));

  // Remove the property from the path, so we have a sub object
  // Turns CameraViewport's "CameraPath.Cog" to "CameraPath". This allows the
  // other CogPath to also change the "Path" property on CogPath.
  PropertyPath localPath = property;
  localPath.PopEntry();
  OperationQueue::PushSubPropertyContext(object, localPath);

  propertyOperation->Redo();
  queue->Queue(propertyOperation);

  queue->QueueRegisteredSideEffects();
  queue->EndBatch();
}

bool QueueRemoveComponent(OperationQueue* queue, HandleParam object, BoundType* componentType, bool ignoreDependencies)
{
  MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();

  if (!ignoreDependencies)
  {
    // First check to see if there's anything dependent on this component
    String reason;
    if (composition->CanRemoveComponent(object, componentType, reason) == false)
    {
      DoNotifyWarning("Can't remove component.", reason);
      return false;
    }
  }

  // Create the operation
  AddRemoveComponentOperation* op = new AddRemoveComponentOperation(object, componentType, ComponentOperation::Remove);
  op->Redo();
  queue->Queue(op);
  return true;
}

void QueueRemoveComponent(OperationQueue* queue, HandleParam object, BoundType* componentMeta, StringParam componentData, uint componentIndex)
{
  // Create the operation
  AddRemoveComponentOperation* op = new AddRemoveComponentOperation(object, componentMeta, ComponentOperation::Remove, componentData, componentIndex);
  queue->Queue(op);
  op->ComponentRemoved(object);
}

bool QueueAddComponent(OperationQueue* queue, HandleParam object, BoundType* componentType)
{
  // We need to look up the meta of the given component type
  MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();

  // We must check to see if adding this component is a valid operation
  // before queuing the operation
  if (composition->CanAddComponent(object, componentType) == false)
    return false;

  // Queue the operation
  AddRemoveComponentOperation* op = new AddRemoveComponentOperation(object, componentType, ComponentOperation::Add);
  queue->Queue(op);
  op->Redo();
  queue->mCreationContexts.Finalize();
  return true;
}

void QueueAddComponent(OperationQueue* queue, HandleParam object, HandleParam component)
{
  AddRemoveComponentOperation* op = new AddRemoveComponentOperation(object, component.StoredType, ComponentOperation::Add);
  queue->Queue(op);
  op->ComponentAdded(object);
}

void QueueMoveComponent(OperationQueue* queue, HandleParam object, HandleParam component, uint index)
{
  Operation* op = new MoveComponentOperation(object, component, index);
  op->Redo();
  queue->Queue(op);
}

void MarkPropertyAsModified(OperationQueue* queue, HandleParam object, PropertyPathParam propertyPath)
{
  Operation* op = new MarkPropertyModifiedOperation(object, propertyPath);
  op->Redo();
  queue->Queue(op);
}

void RevertProperty(OperationQueue* queue, HandleParam object, PropertyPathParam propertyPath)
{
  Operation* op = new RevertPropertyOperation(object, propertyPath);
  op->Redo();
  queue->Queue(op);
}

void RestoreLocallyRemovedChild(OperationQueue* queue, HandleParam parent, ObjectState::ChildId& childId)
{
  ReturnIf(parent.StoredType == nullptr, , "We should always be given a valid parent handle");

  // First check dependencies before re-adding the child (e.g. if Transform and
  // Model were both locally removed, we cannot restore Model until Transform
  // has been restored)
  if (BoundType* childType = MetaDatabase::FindType(childId.mTypeName))
  {
    MetaComposition* composition = parent.StoredType->HasInherited<MetaComposition>();
    ReturnIf(composition == nullptr, , "Object has no MetaComposition.");

    AddInfo addInfo;
    if (!composition->CanAddComponent(parent, childType, &addInfo))
    {
      DoNotifyWarning("Cannot Restore", addInfo.Reason);
      return;
    }
  }

  Operation* op = new RestoreChildOperation(parent, childId);
  op->Redo();
  queue->Queue(op);
}

void RestoreChildOrder(OperationQueue* queue, HandleParam object)
{
  Operation* op = new RestoreChildOrderOperation(object);
  op->Redo();
  queue->Queue(op);
}

MetaOperation::MetaOperation(HandleParam object) : mUndoHandle(object)
{
  if (MetaOperations* metaOp = object.StoredType->HasInherited<MetaOperations>())
    mUndoClientData = metaOp->GetUndoData(object);
}

void MetaOperation::Undo()
{
  Handle object = GetUndoObject();

  ReturnIf(object.IsNull(), , "Cannot undo operation");

  if (MetaOperations* metaOp = object.StoredType->HasInherited<MetaOperations>())
    metaOp->RestoreUndoData(object, mUndoClientData);
}

void MetaOperation::Redo()
{
  Handle object = GetUndoObject();

  if (MetaOperations* metaOp = object.StoredType->HasInherited<MetaOperations>())
    metaOp->ObjectModified(object, false);
}

Handle MetaOperation::GetUndoObject()
{
  return mUndoHandle.GetHandle();
}

RaverieDefineType(PropertyOperation, builder, type)
{
  RaverieBindFieldGetter(mValueBefore);
  RaverieBindFieldGetter(mValueAfter);
}

PropertyOperation::PropertyOperation(HandleParam object, PropertyPathParam property, AnyParam before, AnyParam after) : MetaOperation(object), mPropertyPath(property)
{
  MetaOwner* owner = object.StoredType->HasInherited<MetaOwner>();
  if (owner && owner->GetOwner(object).IsNotNull())
  {
    mName = BuildString(GetNameFromHandle(owner->GetOwner(object)), ".", GetNameFromHandle(object), ".", property.GetStringPath());
  }
  else
  {
    mName = BuildString(GetNameFromHandle(object), ".", property.GetStringPath());
  }

  mValueBefore = before;
  mValueAfter = after;

  // Store whether or not the property was modified so that when we undo, we
  // can restore that state
  LocalModifications* modifications = LocalModifications::GetInstance();
  mPropertyWasModified = modifications->IsPropertyModified(object, property);

  ConnectThisTo(MetaDatabase::GetInstance(), Events::MetaRemoved, OnMetaRemoved);
  ConnectThisTo(RaverieManager::GetInstance(), Events::ScriptsCompiledPrePatch, OnScriptsCompiled);
}

PropertyOperation::~PropertyOperation()
{
}

void PropertyOperation::Undo()
{
  Handle instance = MetaOperation::GetUndoObject();

  // Object may be gone
  ReturnIf(instance.IsNull(), , "Failed to find undo object.");

  // Attempt to set the value
  bool succeeded = mPropertyPath.SetValue(instance, mValueBefore);
  if (!succeeded)
    return;

  // Notify Meta that the property has changed
  MetaOperations::NotifyPropertyModified(instance, mPropertyPath, mValueBefore, mValueAfter, false);

  // Mark the property as modified
  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->SetPropertyModified(instance, mPropertyPath, mPropertyWasModified);

  MetaOperation::Undo();
}

void PropertyOperation::Redo()
{
  Handle instance = MetaOperation::GetUndoObject();

  // Object may be gone
  ReturnIf(instance.IsNull(), , "Failed to find undo object.");

  // Attempt to set the value
  bool succeeded = mPropertyPath.SetValue(instance, mValueAfter);
  if (!succeeded)
    return;

  // Mark the property as modified
  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->SetPropertyModified(instance, mPropertyPath, true);

  // Should we disable property side effects for the notification?

  // Notify Meta that the property has changed
  MetaOperations::NotifyPropertyModified(instance, mPropertyPath, mValueBefore, mValueAfter, false);
}

void PropertyOperation::UpdateValueAfter()
{
  Handle instance = MetaOperation::GetUndoObject();
  mValueAfter = mPropertyPath.GetValue(instance);
}

void PropertyOperation::OnScriptsCompiled(RaverieCompileEvent* e)
{
  if (mValueBefore.IsNull() || mValueAfter.IsNull())
    return;

  BoundType* propertyType = Type::GetBoundType(mValueBefore.StoredType);
  if (BoundType* newType = e->GetReplacingType(propertyType))
  {
    MetaSerialization* metaSerialize = newType->HasInherited<MetaSerialization>();
    if (metaSerialize)
    {
      String stringBefore = metaSerialize->ConvertToString(mValueBefore);
      String stringAfter = metaSerialize->ConvertToString(mValueAfter);

      bool succeeded = true;
      succeeded &= metaSerialize->ConvertFromString(stringBefore, mValueBefore);
      succeeded &= metaSerialize->ConvertFromString(stringAfter, mValueAfter);

      if (succeeded)
        return;
    }

    mInvalidReason = "Could not convert enum value to updated enum type";
  }
  else
  {
    mInvalidReason = String::Format("The property type '%s' was removed", propertyType->Name.c_str());
  }

  // We can't update the values, so invalidate them
  mValueBefore = Any();
  mValueAfter = Any();
}

void PropertyOperation::OnMetaRemoved(MetaLibraryEvent* e)
{
  if (mValueBefore.IsNull() || mValueAfter.IsNull())
    return;

  // If the property type was built in the library being removed,
  BoundType* propertyType = Type::GetBoundType(mValueBefore.StoredType);
  if (propertyType->SourceLibrary == e->mLibrary)
  {
    mValueBefore = Any();
    mValueAfter = Any();
    mInvalidReason = String::Format("The property type '%s' was removed", propertyType->Name.c_str());
  }
}

AddRemoveComponentOperation::AddRemoveComponentOperation(HandleParam object, BoundType* componentType, ComponentOperation::Enum mode, StringParam componentData, uint componentIndex) :
    MetaOperation(object), mComposition(object.StoredType), mComponentType(componentType), mRemovedObjectState(nullptr)
{
  mNotifyModified = true;

  if (mode == ComponentOperation::Add)
    mName = BuildString("Add '", componentType->Name, "' to '", GetNameFromHandle(object), "'");
  else
    mName = BuildString("Remove '", componentType->Name, "' from '", GetNameFromHandle(object), "'");

  mMode = mode;
  mSerializationBuffer = componentData;

  MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();

  // Get the component index if it wasn't specified
  mComponentIndex = componentIndex;
  if (mComponentIndex == uint(-1))
    mComponentIndex = composition->GetComponentIndex(object, componentType);

  // Take the objects global modified state so we can apply the same
  // modifications it when we re-create it
  if (mode == ComponentOperation::Remove)
  {
    Handle component = composition->GetComponent(object, componentType);
    mComponentHandle.SetObject(component);
    mRemovedObjectState = LocalModifications::GetInstance()->TakeObjectState(component);
  }
}

AddRemoveComponentOperation::~AddRemoveComponentOperation()
{
}

void AddRemoveComponentOperation::Undo()
{
  if (mMode == ComponentOperation::Remove)
    AddComponentFromBuffer();
  else
    SaveComponentToBuffer();

  if (mNotifyModified)
    MetaOperation::Undo();
}

void AddRemoveComponentOperation::Redo()
{
  if (mMode == ComponentOperation::Remove)
    SaveComponentToBuffer();
  else
    AddComponentFromBuffer();

  if (mNotifyModified)
    MetaOperation::Redo();
}

void AddRemoveComponentOperation::AddComponentFromBuffer()
{
  // Attempt to grab the object from the undo map
  Handle object = MetaOperation::GetUndoObject();
  ReturnIf(object == nullptr, , "Invalid undo object handle.");

  // Create the component
  BoundType* componentType = mComponentType;
  MetaComposition* composition = mComposition;

  LocalModifications* modifications = LocalModifications::GetInstance();

  // Lets say you have an Archetype with the Model Component. The Model's
  // Material property is set to the Dirt Material. If you locally remove the
  // Model Component and then revert it, it should come back with the Dirt
  // Material still set. If instead of reverting we add a new Model from the
  // 'Add Component' button, it would have the default property values (not the
  // Dirt Material). Unless you re-upload to Archetype, when you run the game,
  // the instance in the game will have the Dirt Material again, instead of the
  // default it shows in the editor. Because of this, instead of actually adding
  // a new Model, we're going to revert the locally removed one.
  if (mMode == ComponentOperation::Add && componentType != nullptr && modifications->IsChildLocallyRemoved(object, componentType))
  {
    modifications->ChildAdded(object, componentType);

    // Rebuild the object to reflect the changes
    MetaDataInheritance* inheritance = object.StoredType->HasInherited<MetaDataInheritance>();
    inheritance->RebuildObject(object);

    return;
  }

  // Create the object
  Handle componentInstance;
  if (componentType)
    componentInstance = composition->MakeObject(componentType);

  // If the component type doesn't exist, it was likely a script
  // component, and the script was removed. We still want to be able
  // to restore the data, so lets create a proxy component
  if (componentType == nullptr || componentInstance == nullptr)
  {
    // If the type existed, but the component was null, it was likely due to an
    // exception when allocating the object
    ProxyReason::Enum reasonToProxy = ProxyReason::TypeDidntExist;
    if (componentType)
      reasonToProxy = ProxyReason::AllocationException;

    String typeName;

    if (componentType)
      typeName = componentType->Name;
    else
    {
      ErrorIf(mSerializationBuffer.Empty(), "Cannot proxy without any data");

      // Load the buffer into a data tree
      Status status;
      DataTreeLoader loader;
      loader.OpenBuffer(status, mSerializationBuffer);

      // We want to grab the first polymorphic node off the top
      // because the CreateProxyType function expects the Serializer
      // to have already entered the object
      PolymorphicNode componentNode;
      loader.GetPolymorphic(componentNode);

      typeName = componentNode.TypeName;
    }

    componentType = composition->MakeProxy(typeName, reasonToProxy);

    // Attempt to recreate the object
    componentInstance = composition->MakeObject(componentType);

    // Nothing else we can do at this point
    ReturnIf(componentInstance.IsNull(), , "Cannot create object, cannot undo.");
  }

  if (!mSerializationBuffer.Empty())
  {
    // Load the saved component buffer into a serializer
    Status status;
    DataTreeLoader loader;
    loader.OpenBuffer(status, mSerializationBuffer);

    // Serialize the component with the saved data
    PolymorphicNode node;
    loader.GetPolymorphic(node);
    ErrorIf(String(node.TypeName) != componentType->Name, "Invalid node in file");
    MetaSerialization* metaSerialize = componentType->HasInherited<MetaSerialization>();
    ErrorIf(metaSerialize == nullptr, "Component did not have meta serialization");

    if (metaSerialize)
      metaSerialize->SerializeObject(componentInstance, loader);
    loader.EndPolymorphic();
  }

  MetaCreationContext* creationContext = mQueue->mCreationContexts.GetContext(composition);

  // Add the component
  bool ignoreDependencies = true;
  composition->AddComponent(object, componentInstance, (int)mComponentIndex, ignoreDependencies, creationContext);

  Handle component = composition->GetComponent(object, componentType);

  // Re-add the local modifications
  if (mRemovedObjectState)
  {
    // In the case of Cogs, the Components handles contain the CogId in it. This
    // component handle was created before being added to the Cog, so it has a
    // raw pointer in the handle data, not a valid CogId. Now that it has been
    // added to the Cog, lets get a new, proper handle to it. This should change
    // once we change Component handles.
    modifications->RestoreObjectState(component, mRemovedObjectState);
  }

  mComponentHandle.UpdateObject(component);

  ComponentAdded(object);
}

void AddRemoveComponentOperation::SaveComponentToBuffer()
{
  // Attempt to grab the object from the undo map
  Handle object = MetaOperation::GetUndoObject();
  ReturnIf(object.IsNull(), , "Invalid undo object handle.");
  BoundType* componentType = mComponentType;
  MetaComposition* composition = mComposition; // componentType->HasInherited<MetaComposition>();

  ReturnIf(composition == nullptr, , "Cannot get MetaComposition.");

  // Attempt to get the component on the object
  Handle componentInstance = composition->GetComponent(object, componentType);
  ReturnIf(componentInstance.IsNull(), , "Invalid component handle.");

  MetaSerialization* metaSerialize = componentType->HasInherited<MetaSerialization>();
  ReturnIf(metaSerialize == nullptr, , "Cannot serialize component for Undo/Redo.");

  // Serialize the component to our buffer
  TextSaver saver;
  saver.OpenBuffer();

  saver.StartPolymorphic(componentInstance.StoredType);
  metaSerialize->SerializeObject(componentInstance, saver);
  saver.EndPolymorphic();

  // Store the saved data in out buffer
  mSerializationBuffer = saver.GetString();

  // Store the index of the component before we remove it so we can
  // put it back in the same spot
  BoundType* componentTypeId = componentInstance.StoredType;
  mComponentIndex = composition->GetComponentIndex(object, componentTypeId);

  bool ignoreDependencies = true;
  composition->RemoveComponent(object, componentInstance, ignoreDependencies);

  ComponentRemoved(object);
}

void AddRemoveComponentOperation::ComponentAdded(HandleParam object)
{
  ReturnIf(object.IsNull(), , "Invalid undo object handle.");

  LocalModifications* modifications = LocalModifications::GetInstance();

  // If the component ever has a unique child id, this will not work. Should
  // be fixed later
  ObjectState::ChildId childId(mComponentType->Name);
  modifications->ChildAdded(object, childId);

  if (mNotifyModified)
    MetaOperations::NotifyComponentsModified(object);
}

void AddRemoveComponentOperation::ComponentRemoved(HandleParam object)
{
  ReturnIf(object.IsNull(), , "Invalid undo object handle.");

  LocalModifications* modifications = LocalModifications::GetInstance();
  // If the component ever has a unique child id, this will not work. Should
  // be fixed later
  ObjectState::ChildId childId(mComponentType->Name);
  modifications->ChildRemoved(object, childId);

  if (mNotifyModified)
    MetaOperations::NotifyComponentsModified(object);
}

MoveComponentOperation::MoveComponentOperation(HandleParam object, HandleParam componentToMove, uint destinationIndex) : MetaOperation(object)
{
  mName = BuildString("Move '", GetNameFromHandle(componentToMove), "' on '", GetNameFromHandle(object), "'");

  mWasOrderLocallyModified = false;
  mComposition = object.StoredType;

  mStartIndex = mComposition->GetComponentIndex(object, componentToMove);
  mEndIndex = destinationIndex;

  // Store whether or not the component order was already overridden so that
  // when we undo, we can properly restore that state
  LocalModifications* modifications = LocalModifications::GetInstance();
  mWasOrderLocallyModified = modifications->IsChildOrderModified(object);
}

MoveComponentOperation::~MoveComponentOperation()
{
}

void MoveComponentOperation::Undo()
{
  if (mStartIndex < mEndIndex)
    MoveComponent(mEndIndex - 1, mStartIndex);
  else
    MoveComponent(mEndIndex, mStartIndex + 1);

  // Attempt to grab the object from the undo map

  Handle instance = MetaOperation::GetUndoObject();
  ReturnIf(instance.IsNull(), , "Invalid undo object handle.");

  // Restore the overridden state
  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->SetChildOrderModified(instance, mWasOrderLocallyModified);

  MetaOperation::Undo();
}

void MoveComponentOperation::Redo()
{
  MoveComponent(mStartIndex, mEndIndex);

  // Attempt to grab the object from the undo map
  Handle instance = MetaOperation::GetUndoObject();
  ReturnIf(instance.IsNull(), , "Invalid undo object handle.");

  // The component order is now overridden
  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->SetChildOrderModified(instance, true);

  MetaOperation::Redo();
}

void MoveComponentOperation::MoveComponent(uint from, uint to)
{
  // Attempt to grab the object from the undo map
  Handle object = MetaOperation::GetUndoObject();
  ReturnIf(object.IsNull(), , "Invalid undo object handle.");

  Handle componentToMove = mComposition->GetComponentAt(object, from);
  mComposition->MoveComponent(object, componentToMove, to);
}

MarkPropertyModifiedOperation::MarkPropertyModifiedOperation(HandleParam object, PropertyPathParam propertyPath) : MetaOperation(object)
{
  mName = BuildString("'", propertyPath.GetStringPath(), "' marked modified on '", GetNameFromHandle(object), "'");

  mPropertyPath = propertyPath;
}

void MarkPropertyModifiedOperation::Undo()
{
  SetModifiedState(false);
  MetaOperation::Undo();
}

void MarkPropertyModifiedOperation::Redo()
{
  SetModifiedState(true);
  MetaOperation::Redo();
}

void MarkPropertyModifiedOperation::SetModifiedState(bool state)
{
  // Grab the object from the operation queue
  Handle instance = MetaOperation::GetUndoObject();
  if (instance.IsNull())
    return;

  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->SetPropertyModified(instance, mPropertyPath, state);

  MetaDataInheritance* inheritance = instance.StoredType->HasInherited<MetaDataInheritance>();
  ReturnIf(inheritance == nullptr, , "Must have data inheritance for this operation.");
  inheritance->SetPropertyModified(instance, mPropertyPath, state);
}

RevertPropertyOperation::RevertPropertyOperation(HandleParam object, PropertyPathParam propertyPath) : MetaOperation(object)
{
  mName = BuildString("Reverted '", propertyPath.GetStringPath(), "' value on '", GetNameFromHandle(object), "'");

  mPropertyToRevert = propertyPath;
  mOldValue = propertyPath.GetValue(object);
}

void RevertPropertyOperation::Undo()
{
  // Grab the object from the operation queue
  Handle instance = MetaOperation::GetUndoObject();
  if (instance.IsNull())
    return;

  Any currentValue = mPropertyToRevert.GetValue(instance);

  // Set the property to the old value
  PropertyOperation op(instance, mPropertyToRevert, currentValue, mOldValue);
  op.Redo();

  MetaOperation::Undo();
}

void RevertPropertyOperation::Redo()
{
  // Grab the object from the operation queue
  Handle instance = MetaOperation::GetUndoObject();
  if (instance.IsNull())
    return;

  // Revert the property
  BoundType* instanceType = instance.StoredType;
  if (MetaDataInheritance* inheritance = instanceType->HasInherited<MetaDataInheritance>())
    inheritance->RevertProperty(instance, mPropertyToRevert);

  MetaOperation::Redo();
}

RestoreChildOperation::RestoreChildOperation(HandleParam parent, ObjectState::ChildId& childId) : MetaOperation(parent), mChildId(childId), mInheritance(parent.StoredType)
{
  mName = BuildString("RestoreChild on '", GetNameFromHandle(parent), "'");
}

void RestoreChildOperation::Undo()
{
  Handle parent = MetaOperation::GetUndoObject();

  if (!parent.IsNull())
  {
    LocalModifications::GetInstance()->ChildRemoved(parent, mChildId);
    mInheritance->RebuildObject(parent);
  }

  MetaOperation::Undo();
}

void RestoreChildOperation::Redo()
{
  Handle parent = MetaOperation::GetUndoObject();

  if (!parent.IsNull())
  {

    BoundType* parentType = parent.StoredType;
    if (MetaDataInheritance* inheritance = parentType->HasInherited<MetaDataInheritance>())
      inheritance->RestoreRemovedChild(parent, mChildId);
  }

  MetaOperation::Redo();
}

RestoreChildOrderOperation::RestoreChildOrderOperation(HandleParam object) : MetaOperation(object), mMetaOperations(object.StoredType), mInheritance(object.StoredType)
{
  mName = BuildString("RestoreChildOrder on '", GetNameFromHandle(object), "'");

  MetaOperations* metaOp = mMetaOperations;
  ReturnIf(metaOp == nullptr, , "Must have MetaOperations interface.");
  mRestoreState = metaOp->GetRestoreState(object);
}

RestoreChildOrderOperation::~RestoreChildOrderOperation()
{
  SafeDelete(mRestoreState);
}

void RestoreChildOrderOperation::Undo()
{
  mRestoreState->RestoreObject();
  MetaOperation::Undo();
}

void RestoreChildOrderOperation::Redo()
{
  Handle instance = MetaOperation::GetUndoObject();
  if (!instance.IsNull())
  {
    LocalModifications::GetInstance()->SetChildOrderModified(instance, false);

    // Rebuild the object to reflect the changes
    mInheritance->RebuildObject(instance);
  }

  MetaOperation::Redo();
}

} // namespace Raverie
