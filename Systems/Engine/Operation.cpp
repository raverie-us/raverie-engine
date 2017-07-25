///////////////////////////////////////////////////////////////////////////////
///
/// \file Operation.cpp
/// Implementation of the Operation classes.
/// 
/// Authors: Joshua Claeys, Chris Peters, Ryan Edgemon
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool OperationQueue::sListeningForSideEffects = false;
Array<PropertyOperation*> OperationQueue::sSideEffects;
Array<OperationQueue::SideEffectContext> OperationQueue::sSideEffectContextStack;

namespace Z
{
  UndoMap* gUndoMap = nullptr;
}

//----------------------------------------------------------------------- Events
namespace Events
{
DefineEvent(OperationQueued);
DefineEvent(OperationUndo);
DefineEvent(OperationRedo);
}

ZilchDefineType(OperationQueueEvent, builder, type)
{
  ZilchBindFieldProperty(mOperation);
}

//-------------------------------------------------------------------- Operation
ZilchDefineType(Operation, builder, type)
{
  ZilchBindFieldProperty(mName);
  ZilchBindFieldProperty(mDescription);

  ZilchBindGetterProperty(Children);

  ZeroBindEvent(Events::OperationQueued, OperationQueueEvent);
  ZeroBindEvent(Events::OperationUndo, OperationQueueEvent);
  ZeroBindEvent(Events::OperationRedo, OperationQueueEvent);
}

Memory::Heap* Operation::sHeap = new Memory::Heap("Operations", NULL);

//******************************************************************************
void* Operation::operator new(size_t size)
{
  return sHeap->Allocate(size);
}

//******************************************************************************
void Operation::operator delete(void* pMem, size_t size)
{
  return sHeap->Deallocate(pMem, size);
}

//------------------------------------------------------------------- Meta Proxy
// Use the Operations heap for allocations
//******************************************************************************
void* MetaProxy::operator new(size_t size)
{
  return Operation::sHeap->Allocate(size);
}

//******************************************************************************
void MetaProxy::operator delete(void* pMem, size_t size)
{
  return Operation::sHeap->Deallocate(pMem, size);
}

//******************************************************************************
MetaProxy::~MetaProxy()
{
  DeleteObjectsInContainer(mComponents);
  DeleteObjectsInContainer(mChildren);
}

//-------------------------------------------------------------- Operation Batch
ZilchDefineType(OperationBatch, builder, type)
{
}

//******************************************************************************
OperationBatch::~OperationBatch()
{
  DeleteObjectsIn(BatchedCommands);
  DeleteObjectsInContainer(mObjectProxies);
}

//******************************************************************************
void OperationBatch::Undo()
{
  //Undo the commands in the reverse order
  OperationListType::iterator cur = BatchedCommands.ReverseBegin();
  OperationListType::iterator end = BatchedCommands.ReverseEnd();
  while(cur!=end)
  {
    cur->Undo();
    --cur;
  }
}

//******************************************************************************
void OperationBatch::Redo()
{
  OperationListType::iterator cur = BatchedCommands.Begin();
  OperationListType::iterator end = BatchedCommands.End();
  while(cur!=end)
  {
    cur->Redo();
    ++cur;
  }
}

//-------------------------------------------------------------- Operation Queue
//******************************************************************************
ZilchDefineType(OperationQueue, builder, type)
{
  ZilchBindOverloadedMethod(Undo, ZilchInstanceOverload(void));
  ZilchBindOverloadedMethod(Undo, ZilchInstanceOverload(bool, Operation*));
  ZilchBindOverloadedMethod(Redo, ZilchInstanceOverload(void));
  ZilchBindOverloadedMethod(Redo, ZilchInstanceOverload(bool, Operation*));
  ZilchBindMethod(ClearUndo);
  ZilchBindMethod(ClearRedo);
  ZilchBindMethod(ClearAll);

  ZilchBindGetterProperty(Commands);
  ZilchBindGetterProperty(RedoCommands);

  ZilchBindMethod(BeginBatch);
  ZilchBindMethod(EndBatch);

  ZilchBindMethod(SaveObjectState);
  ZilchBindMethod(ObjectCreated);
  ZilchBindMethod(DestroyObject);

  ZilchBindDefaultDestructor();

  type->CreatableInScript = true;

  ZilchBindMethod(StartListeningForSideEffects);
  ZilchBindMethod(IsListeningForSideEffects);
  ZilchBindMethod(RegisterSideEffect);
  ZilchBindMethod(QueueRegisteredSideEffects);
  ZilchBindMethod(PopSubPropertyContext);
  ZilchBindMethod(MarkPropertyAsModified);
}

//******************************************************************************
OperationQueue::OperationQueue()
{
  ActiveBatch = NULL;
}

//******************************************************************************
OperationQueue::~OperationQueue()
{
  ClearUndo();
  ClearRedo();
}

//******************************************************************************
void OperationQueue::Undo( )
{
  if(Commands.Empty( ))
    return;

  Operation* last = &Commands.Back( );
  last->Undo( );
  Commands.Erase(last);
  RedoCommands.PushFront(last);

  OperationQueueEvent event(last);
  DispatchEvent(Events::OperationUndo, &event);
}

//******************************************************************************
bool OperationQueue::Undo(Operation* allbeforeThis)
{
  if(allbeforeThis == nullptr || Commands.Empty( ))
    return false;

  bool operationFound = false;
  Array<Operation*> toErase;

  OperationListType::reverse_range rRange(Commands.Begin( ), Commands.End( ));
  forRange(Operation& operation, rRange.All( ))
  {
    if(&operation == allbeforeThis)
    {
      operationFound = true;
      break;
    }

    toErase.PushBack(&operation);
  }

  if(!operationFound)
  {
    Warn("Supplied operation does not exist in Undo Queue.  Undo will not occur.");
    return operationFound;
  }

  int size = toErase.Size();
  for(int i = 0; i < size; ++i)
  {
    toErase[i]->Undo( );
    Commands.Erase(toErase[i]);
    RedoCommands.PushFront(toErase[i]);
  }

  return operationFound;
}

//******************************************************************************
void OperationQueue::Redo( )
{
  if(!RedoCommands.Empty( ))
  {
    Operation* first = &RedoCommands.Front( );
    first->Redo( );
    RedoCommands.Erase(first);
    Commands.PushBack(first);

    OperationQueueEvent event(first);
    DispatchEvent(Events::OperationRedo, &event);
  }

}

//******************************************************************************
bool OperationQueue::Redo(Operation* upToAndThis)
{
    // Call most likely came from script.
  if(upToAndThis == nullptr || RedoCommands.Empty())
    return false;

  bool operationFound = false;
  Array<Operation*> toErase;

  forRange(Operation& operation, RedoCommands.All( ))
  {
    toErase.PushBack(&operation);

    if(&operation == upToAndThis)
    {
      operationFound = true;
      break;
    }

  }

  if(!operationFound)
  {
    Warn("Supplied operation does not exist in Redo Queue.  Redo will not occur.");
    return operationFound;
  }

  int size = toErase.Size( );
  for(int i = 0; i < size; ++i)
  {
    toErase[i]->Redo();
    RedoCommands.Erase(toErase[i]);
    Commands.PushBack(toErase[i]);
  }

  return operationFound;
}

template<typename type, Link<type> type::* PtrToMember>
void DeleteObjectsInTest(InList<type, PtrToMember>& container)
{
  container.SafeForEach(container.Begin( ), container.End( ), EraseAndDelete<type, PtrToMember>);
}
template<typename type, typename baseLinkType>
void DeleteObjectsInTest(BaseInList<baseLinkType, type>& container)
{
  container.SafeForEach(container.Begin( ), container.End( ), EraseAndDeleteBase<type, baseLinkType>);
}

//******************************************************************************
void OperationQueue::ClearUndo()
{
  DeleteObjectsIn(Commands);
}

//******************************************************************************
void OperationQueue::ClearRedo()
{
  DeleteObjectsIn(RedoCommands);
}

//******************************************************************************
void OperationQueue::ClearAll()
{
  ClearUndo();
  ClearRedo();
}

//******************************************************************************
OperationListRange OperationQueue::GetCommands( )
{
  return Commands.All();
}

//******************************************************************************
OperationListRange OperationQueue::GetRedoCommands( )
{
  return RedoCommands.All( );
}

//******************************************************************************
void OperationQueue::Queue(Operation* command)
{
  if(ActiveBatch)
  {
    ActiveBatch->BatchedCommands.PushBack(command);
  }
  else
  {
    ClearRedo();
    Commands.PushBack(command);

    OperationQueueEvent event(command);
    DispatchEvent(Events::OperationQueued, &event);
  }

}

//******************************************************************************
const String& OperationQueue::GetActiveBatchName(StringParam batchName)
{
  return ActiveBatch->mName;
}

//******************************************************************************
void OperationQueue::SetActiveBatchName(StringParam batchName)
{
  if(ActiveBatch != nullptr)
    ActiveBatch->mName = batchName;
}

//******************************************************************************
void OperationQueue::SetActiveBatchDescription(StringParam description)
{
  if(ActiveBatch != nullptr)
    ActiveBatch->mDescription = description;
}

//******************************************************************************
void OperationQueue::BeginBatch()
{
  if(ActiveBatch)
  {
    BatchStack.PushBack(ActiveBatch);
    ActiveBatch = new OperationBatch();
  }
  else
  {
    ActiveBatch = new OperationBatch();
  }
}

//******************************************************************************
void OperationQueue::EndBatch()
{
  if(ActiveBatch == nullptr)
  {
    DoNotifyException("No Operation Batch Active", "Call BeginBatch before EndBatch.");
    return;
  }

  int operationCount = 0;
  forRange(Operation& operation, ActiveBatch->GetChildren( ))
  {
    ++operationCount;
  }

  // Only error if operations exist in the batch.  If there are no operations
  // in the batch, it'll get cleaned up below.
  ErrorIf((ActiveBatch->mName.Empty() && operationCount != 0),
    "ActiveBatch must have a name before EndBatch.");

  // Queue all changes
  forRange(MetaProxy* proxy, ActiveBatch->mObjectProxies.All())
  {
    QueueChanges(proxy);
  }

  // Cleanup
  mDestroyedObjects.Clear();

  if(!BatchStack.Empty())
  {
    OperationBatch* previous = (OperationBatch*)&BatchStack.Back();

    // Don't queue an empty batch.
    if(ActiveBatch->BatchedCommands.Empty( ))
    {
      SafeDelete(ActiveBatch);
    }
    // Promote Operation if there is only one in the batch
    else if(operationCount == 1)
    {
      Operation* operation = &ActiveBatch->GetChildren( ).Front( );
      ActiveBatch->BatchedCommands.Erase(operation);
      previous->BatchedCommands.PushBack(operation);

      SafeDelete(ActiveBatch);
    }
    // Normal batch.
    else if(operationCount != 0)
    {
      previous->BatchedCommands.PushBack(ActiveBatch);
    }

    BatchStack.Erase(previous);
    ActiveBatch = previous;
  }
  else
  {
    // Don't queue an empty batch.
    if(ActiveBatch->BatchedCommands.Empty())
    {
      SafeDelete(ActiveBatch);
    }
    else
    {
      // Clear the redo stack
      ClearRedo();

      // Promote Operation if there is only one in the batch
      if(operationCount == 1)
      {
        Operation* operation = &ActiveBatch->GetChildren( ).Front( );
        ActiveBatch->BatchedCommands.Erase(operation);
        Commands.PushBack(operation);

        SafeDelete(ActiveBatch);
      }
      else if(operationCount != 0)
      {
        Commands.PushBack(ActiveBatch);
      }

      // ONLY send out the queue event when the root-batch in the stack has ended.
      OperationQueueEvent event(&Commands.Back());
      DispatchEvent(Events::OperationQueued, &event);
    }

    ActiveBatch = NULL;
  }

}

//******************************************************************************
SerializeCheck::Enum HierarchyFilter(Cog* composition, Component* component)
{
  Hierarchy* hierarchy = Type::DynamicCast<Hierarchy*>(component);
  if(hierarchy != nullptr)
    return SerializeCheck::NotSerialized;
  return SerializeCheck::Serialized;
}

//******************************************************************************
void OperationQueue::SaveObjectState(Cog* object)
{
  // Make sure the object is valid
  if(object == nullptr)
  {
    DoNotifyException("Null object given", String());
    return;
  }

  if(ActiveBatch == nullptr)
  {
    DoNotifyException("No Active Batch", "Call 'BeginBatch' before making modifications.");
    return;
  }

  // Record the state of this object
  MetaProxy* proxy = new MetaProxy();
  BuildMetaProxy(proxy, object);
  ActiveBatch->mObjectProxies.PushBack(proxy);
}

//******************************************************************************
void OperationQueue::MarkPropertyAsModified(Component* component, StringParam propertyName)
{
  if(component == nullptr)
  {
    DoNotifyException("Null component", "Cannot mark a property of a null component as modified");
    return;
  }

  BoundType* componentType = ZilchVirtualTypeId(component);
  Property* property = componentType->GetProperty(propertyName);
  if(property == nullptr)
  {
    String msg = String::Format("Component '%s' doesn't have a property by the name of '%s'", componentType->Name.c_str(), propertyName.c_str());
    DoNotifyException("Invalid property", msg);
    return;
  }
  
  Any val = property->GetValue(component);
  ChangeAndQueueProperty(this, component, propertyName, val);
}

//******************************************************************************
void OperationQueue::ObjectCreated(Cog* object)
{
  if(object == nullptr)
  {
    DoNotifyException("null object given", String());
    return;
  }

  if(ActiveBatch == nullptr)
  {
    DoNotifyException("No Active Batch", "Call 'BeginBatch' before making modifications.");
    return;
  }

  Zero::ObjectCreated(this, object);
}

//******************************************************************************
void RecordDestroyedObjects(Cog* object, HashSet<Cog*>& objectSet)
{
  objectSet.Insert(object);
  forRange(Cog& child, object->GetChildren())
  {
    RecordDestroyedObjects(&child, objectSet);
  }
}

//******************************************************************************
void OperationQueue::DestroyObject(Cog* object)
{
  if(ActiveBatch == nullptr)
  {
    DoNotifyException("No Active Batch", "Call 'BeginBatch' before making modifications.");
    return;
  }

  RecordDestroyedObjects(object, mDestroyedObjects);

  Zero::DestroyObject(this, object);
}

//******************************************************************************
void OperationQueue::DetachObject(Cog* object, bool relative)
{
  Zero::DetachObject(this, object, relative);
}

//******************************************************************************
void OperationQueue::AttachObject(Cog* object, Cog* parent, bool relative)
{
  Zero::AttachObject(this, object, parent, relative);
}

//******************************************************************************
void AddProperties(MetaProxy* proxy, Object* object)
{
  forRange(Property* prop, ZilchVirtualTypeId(object)->GetProperties())
  {
    if(prop->HasAttribute(Zilch::PropertyAttribute) == false || prop->Set == nullptr)
      continue;

    MetaProxy::PropertyState propertyState;
    propertyState.mName = prop->Name;
    propertyState.mValue = prop->GetValue(object);
    proxy->mProperties.PushBack(propertyState);
  }
}

//******************************************************************************
String SaveObjectToString(Object* object)
{
  // Serialize the object to our buffer
  TextSaver saver;
  saver.OpenBuffer(); 

  saver.StartPolymorphic(ZilchVirtualTypeId(object));
  object->Serialize(saver);
  saver.EndPolymorphic();

  // Return the serialized data
  return saver.GetString();
}

//******************************************************************************
void OperationQueue::BuildMetaProxy(MetaProxy* proxy, Cog* object)
{
  proxy->mType = ZilchVirtualTypeId(object);
  proxy->mObject = object;
  proxy->mParent = object->GetParent();
  proxy->mHierarchyIndex = object->GetHierarchyIndex();
  proxy->mChildId = object->mChildId;

  // Add all Properties
  AddProperties(proxy, object);

  // Add all Components
  forRange(Component* component, object->GetComponents())
  {
    MetaProxy* componentProxy = new MetaProxy();
    componentProxy->mType = ZilchVirtualTypeId(component);
    componentProxy->mSerializedObject = SaveObjectToString(component);

    // Add all properties
    AddProperties(componentProxy, component);

    proxy->mComponents.PushBack(componentProxy);
  }

  // Record changes to all children
  forRange(Cog& child, object->GetChildren())
  {
    MetaProxy* childProxy = new MetaProxy();
    BuildMetaProxy(childProxy, &child);
    proxy->mChildren.PushBack(childProxy);
  }
}

//******************************************************************************
void OperationQueue::QueueChanges(MetaProxy* proxy)
{
  Cog* newCog = proxy->mObject.Get<Cog*>();

  // Check to see if the object was destroyed
  if(newCog == nullptr || newCog->GetMarkedForDestruction())
  {
    // No need to throw an exception if we are aware of the deletion
    if(mDestroyedObjects.Contains(newCog))
      return;

    // Destroyed
    DoNotifyException("An object marked for modification was destroyed",
                      "Use the 'DestroyObject' function to destroy objects");
    return;
  }

  // Check for the cog name
  MetaProxy::PropertyState& nameState = proxy->mProperties.Front();
  String oldName = nameState.mValue.Get<String>();
  String newName = newCog->mName;
  if(oldName != newName)
  {
    newCog->mName = oldName;
    PropertyPath propertyPath(nameState.mName);
    ChangeAndQueueProperty(this, newCog, propertyPath, newName);
    newCog->GetSpace()->MarkModified();
  }

  // Check old Components
  for(uint i = 0; i < proxy->mComponents.Size(); ++i)
  {
    MetaProxy* componentProxy = proxy->mComponents[i];

    // Check the new object to see if it still has the component
    BoundType* componentType = componentProxy->mType;
    Component* component = newCog->QueryComponentType(componentType);

    // Queue an operation if the component was removed
    if(component == nullptr)
    {
      String serializedData = componentProxy->mSerializedObject;
      QueueRemoveComponent(this, newCog, componentType, serializedData, i);
      continue;
    }

    // Check properties
    forRange(MetaProxy::PropertyState& propertyState, componentProxy->mProperties.All())
    {
      Property* prop = ZilchVirtualTypeId(component)->GetProperty(propertyState.mName);

      // Property removed / renamed?
      if(prop == nullptr)
        continue;

      Any oldValue = propertyState.mValue;
      Any newValue = prop->GetValue(component);

      // If they're different, queue a change
      if(newValue != oldValue)
      {
        prop->SetValue(component, oldValue);

        PropertyPath propertyPath(component, prop);
        ChangeAndQueueProperty(this, newCog, propertyPath, newValue);
        newCog->GetSpace()->MarkModified();
      }
    }
  }

  // Check to see if the object was detached
  Cog* oldParent = proxy->mParent.Get<Cog*>();
  Cog* newParent = newCog->GetParent();
  if(newParent != oldParent)
  {
    // We aren't doing a relative attach / detach because the if there was
    // a change in translation caused by the attach / detach, it will be
    // handled by the above property change to translation
    bool relative = false;

    if(newParent != nullptr)
    {
      DoNotifyException("Invalid Operation", "Object was attached to another object. "
                                             "Use the 'AttachObject' function on OperationQueue.");
    }
    else
    {
      DoNotifyException("Invalid Operation", "Object was detached. Use the 'DetachObject' function "
                                             "on OperationQueue.");
    }
  }

  // Check for any newly added components on the live object
  forRange(Component* component, newCog->GetComponents())
  {
    // Look to see if the old object had the component
    bool found = false;
    forRange(MetaProxy* proxyComponent, proxy->mComponents.All())
    {
      if(proxyComponent->mType == ZilchVirtualTypeId(component))
      {
        found = true;
        break;
      }
    }

    // If the component wasn't found, it was added and should be queued
    if(!found)
      QueueAddComponent(this, newCog, component);
  }

  // Check changes on all children
  forRange(MetaProxy* childProxy, proxy->mChildren.All())
  {
    QueueChanges(childProxy);
  }
}

//------------------------------------------------------------------------------------- Side Effects
//******************************************************************************
void OperationQueue::StartListeningForSideEffects()
{
  ErrorIf(sListeningForSideEffects == true, "Someone started listening and didn't "
    "queue registered side affects");
  sListeningForSideEffects = true;
}

//******************************************************************************
bool OperationQueue::IsListeningForSideEffects()
{
  return sListeningForSideEffects;
}

//******************************************************************************
void OperationQueue::RegisterSideEffect(HandleParam object, PropertyPathParam propertyPath,
                                        const Any& oldValue)
{
  BoundType* objectType = object.StoredType;
  if(objectType->HasInherited<MetaDataInheritance>() != nullptr)
  {
    Any dummyNewValue;
    PropertyOperation* op = new PropertyOperation(object, propertyPath, oldValue, dummyNewValue);
    sSideEffects.PushBack(op);

    PropertyPath contextPath = propertyPath;
    contextPath.PopEntry();
    PushSubPropertyContext(object, contextPath);
  }
  else
  {
    ReturnIf(sSideEffectContextStack.Empty(), , "No sub context on stack.");

    SideEffectContext& context = sSideEffectContextStack.Back();

    // Combine the paths
    PropertyPath finalPath = context.mRootPath;
    forRange(PropertyPath::Entry& entry, propertyPath.mPath.All())
      finalPath.mPath.PushBack(entry);

    // Create an operation
    Any dummyNewValue;
    PropertyOperation* op = new PropertyOperation(context.mContext, finalPath, oldValue, dummyNewValue);
    sSideEffects.PushBack(op);
  }
}

//******************************************************************************
void OperationQueue::QueueRegisteredSideEffects()
{
  sListeningForSideEffects = false;

  // Start an operation batch if we have multiple operations to queue
  size_t count = sSideEffects.Size();
  if(count > 1)
    BeginBatch();

  // Queue all property operations
  forRange(PropertyOperation* op, sSideEffects.All())
  {
    // Query for the new value
    op->UpdateValueAfter();

    // Even though the value is already set, we want to notify of the property change
    // and properly mark it as modified if it's part of an Archetype
    op->Redo();

    Queue(op);
  }

  if(count > 1)
    EndBatch();

  sSideEffects.Clear();
  sSideEffectContextStack.Clear();
}

//******************************************************************************
void OperationQueue::PushSubPropertyContext(HandleParam object, PropertyPathParam contextPath)
{
  sSideEffectContextStack.PushBack(SideEffectContext(object, contextPath));
}

//******************************************************************************
void OperationQueue::PopSubPropertyContext()
{
  if(!sSideEffectContextStack.Empty())
    sSideEffectContextStack.PopBack();
}

//******************************************************************************
OperationQueue::SideEffectContext::SideEffectContext(HandleParam context, PropertyPathParam path)
{
  mContext = context;
  mRootPath = path;
}

//-------------------------------------------------------------------------------------- Undo Handle
//**************************************************************************************************
UndoHandle::UndoHandle()
{
  mUndoId = cInvalidUndoObjectId;
}

//**************************************************************************************************
UndoHandle::UndoHandle(HandleParam object)
{
  SetObject(object);
}

//**************************************************************************************************
void UndoHandle::operator=(HandleParam rhs)
{
  SetObject(rhs);
}

//**************************************************************************************************
void UndoHandle::SetObject(HandleParam object)
{
  mUndoId = Z::gUndoMap->GetUndoId(object);

  if (mUndoId == cInvalidUndoObjectId)
    mRawObject = object;
}

//**************************************************************************************************
Handle UndoHandle::GetHandle() const
{
  if (mUndoId != cInvalidUndoObjectId)
    return Z::gUndoMap->FindObject(mUndoId);
  return mRawObject;
}

//**************************************************************************************************
void UndoHandle::UpdateObject(HandleParam newObject)
{
  mUndoId = Z::gUndoMap->UpdateUndoId(mUndoId, newObject);
}

//----------------------------------------------------------------------------------------- Undo Map
//**************************************************************************************************
void UndoMap::Initialize()
{
  Z::gUndoMap = new UndoMap();
}

//******************************************************************************
void UndoMap::Shutdown()
{
  delete Z::gUndoMap;
  Z::gUndoMap = nullptr;
}

//******************************************************************************
UndoMap::UndoMap()
{
  UndoIdNum = 0;
}

//******************************************************************************
UndoObjectId UndoMap::UpdateUndoId(UndoObjectId id, HandleParam newObject)
{
  // If the handle did not refer to anything, create a new one
  if(id == 0 )
  {
    ++UndoIdNum;
    id = UndoIdNum;
  }

  // Map it in both directions
  UndoIdToObject[id] = newObject;

  u64 objectId = GetObjectId(newObject);
  ObjectToUndoId[objectId] = id;

  // Return the updated handle (if handle was 0), otherwise it's the same
  return id;
}

//******************************************************************************
void UndoMap::UpdateHandleIfExists(HandleParam oldObject, HandleParam newObject)
{
  UndoObjectId undoId = GetUndoId(oldObject, false);
  if(undoId == 0)
    return;

  UpdateUndoId(undoId, newObject);
}

//******************************************************************************
Handle UndoMap::FindObject(UndoObjectId id)
{
   // Find the primary object id
   return UndoIdToObject.FindValue(id, Handle());
}

//******************************************************************************
UndoObjectId UndoMap::GetUndoId(HandleParam object, bool createEntry)
{
  u64 objectId = GetObjectId(object);

  if (objectId == (u64)-1)
    return cInvalidUndoObjectId;

  // Find the Id
  UndoObjectId undoId = ObjectToUndoId.FindValue(objectId, 0);

  // If it doesn't exist, Insert it
  if(undoId == 0 && createEntry)
  {
    ++UndoIdNum;
    undoId = UndoIdNum;
    UndoIdToObject[ undoId ] = object;
    ObjectToUndoId[objectId] = undoId;
  }
  else
  {
    // Update the live object in case the old one was released
    UndoIdToObject[undoId] = object;
  }

  // Return a new handle
  return undoId;
}

//******************************************************************************
u64 UndoMap::GetObjectId(HandleParam object)
{
  if(MetaOperations* metaOp = object.StoredType->HasInherited<MetaOperations>())
    return metaOp->GetUndoHandleId(object);
  return (u64)-1;
}

}//namespace Zero
