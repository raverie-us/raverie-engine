// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Operation;
class OperationBatch;
class OperationQueue;
class PropertyOperation;
class RaverieCompileEvent;

namespace Events
{
DeclareEvent(OperationQueued);
DeclareEvent(OperationUndo);
DeclareEvent(OperationRedo);
} // namespace Events

/// Notification about most recent Operation/OperationBatch added to the
/// OperationQueue.
class OperationQueueEvent : public Event
{
public:
  RaverieDeclareType(OperationQueueEvent, TypeCopyMode::ReferenceType);

  OperationQueueEvent() : mOperation(nullptr)
  {
  }
  OperationQueueEvent(Operation* op) : mOperation(op)
  {
  }

  Operation* mOperation;
};

class OperationCreationContext
{
public:
  ~OperationCreationContext();

  /// If the context for the given composition doesn't exist, it will be created
  /// for you.
  MetaCreationContext* GetContext(MetaComposition* composition);
  void Finalize();

  typedef HashMap<MetaComposition*, MetaCreationContext*> ContextMap;
  ContextMap mContexts;
};

class Operation;

class OperationLink
{
public:
  Link<OperationLink> link;
};

class Operation : public SafeId32EventObject, public OperationLink
{
public:
  RaverieDeclareDerivedTypeExplicit(Operation, SafeId32EventObject, TypeCopyMode::ReferenceType);

  typedef BaseInList<OperationLink, Operation, &OperationLink::link> OperationList;
  typedef OperationList::range OperationRange;

  Operation() : mParent(nullptr), mQueue(nullptr)
  {
  }

  /// Operation memory management.
  static Memory::Heap* sHeap;
  OverloadedNew();

  /// Standard Interface.
  virtual void Undo() = 0;
  virtual void Redo() = 0;

  virtual OperationRange GetChildren()
  {
    return OperationRange();
  }

  /// Find the top most ancestor in an Operation hierarchy, if there is one.
  Operation* FindRoot();

  bool GetInvalid();

  Operation* mParent;

  String mName;
  String mInvalidReason;
  OperationQueue* mQueue;

private:
  Operation(const Operation& rhs)
  {
  }
};

typedef Operation::OperationList OperationListType;
typedef Operation::OperationList::range OperationListRange;

/// Objects state before modifications
class MetaProxy
{
public:
  OverloadedNew();
  ~MetaProxy();

  BoundType* mType;

  /// Handle to the object.
  Handle mObject;

  /// Handle to the parent.
  Handle mParent;

  uint mHierarchyIndex;
  Guid mChildId;

  struct PropertyState
  {
    PropertyState()
    {
    }
    PropertyState(StringParam name, AnyParam val) : mName(name), mValue(val)
    {
    }

    String mName;
    Any mValue;
  };

  Array<PropertyState> mProperties;
  Array<MetaProxy*> mComponents;
  Array<MetaProxy*> mChildren;

  /// We want to store the serialized object in case it was deleted.
  /// Only currently used for Components.
  String mSerializedObject;
};

class OperationBatch : public Operation
{
public:
  RaverieDeclareType(OperationBatch, TypeCopyMode::ReferenceType);

  OperationBatch() : Operation()
  {
    mName = "UnnamedBatch";
  }
  ~OperationBatch();

  /// Operation Interface.
  void Undo() override;
  void Redo() override;

  OperationRange GetChildren() override
  {
    return BatchedCommands.All();
  }

  OperationListType BatchedCommands;
  Array<MetaProxy*> mObjectProxies;

private:
  OperationBatch(const OperationBatch& rhs)
  {
  }
};

class UndoMap;

class OperationQueue : public ReferenceCountedEventObject
{
public:
  RaverieDeclareType(OperationQueue, TypeCopyMode::ReferenceType);

  OperationQueue();
  ~OperationQueue();

  OperationListType mCommands;
  OperationListType mRedoCommands;

  void Undo();
  /// If the operation passed in has ancestors, then the Undo will be done on
  /// all operations before the ancestor root of the hierarchy containing
  /// 'allBeforeThis'.
  bool Undo(Operation* allBeforeThis);
  void Redo();
  /// If the operation passed in has ancestors, then the Redo will be done on
  /// all operations up to and including the ancestor root of the hierarchy
  /// containing 'upToAndThis'.
  bool Redo(Operation* upToAndThis);

  void ClearUndo();
  void ClearRedo();

  OperationListRange GetCommands();
  OperationListRange GetRedoCommands();

  OperationBatch* ActiveBatch;
  OperationListType BatchStack;

  void Queue(Operation* command);
  String GetActiveBatchName();
  void SetActiveBatchName(StringParam batchName);
  void SetActiveBatchDescription(StringParam description);
  void BeginBatch(StringParam batchName);
  void BeginBatch();
  void EndBatch();
  void ClearAll();

  /// Will store the state of the given object. Any modifications to this object
  /// will be recorded and operations will be properly created when EndBatch
  /// is called on the OperationQueue.
  void SaveObjectState(Cog* object);
  /// Marks a property on the given component as being modified. This will set
  /// the property to it's current value.
  void MarkPropertyAsModified(Component* component, StringParam propertyName);

  /// Records that the object was created. This will also mark the object
  /// for any further modifications until EndBatch is called on the
  /// OperationQueue.
  void ObjectCreated(Cog* object);

  /// Records that the object was destroyed.
  void DestroyObject(Cog* object);
  void DetachObject(Cog* object, bool relative);
  void AttachObject(Cog* object, Cog* parent, bool relative);

  void BuildMetaProxy(MetaProxy* proxy, Cog* object);
  void QueueChanges(MetaProxy* proxy);

  OperationCreationContext mCreationContexts;
  HashSet<Cog*> mDestroyedObjects;

  // Side Effects
  static void StartListeningForSideEffects();
  static bool IsListeningForSideEffects();
  static void RegisterSideEffect(HandleParam object, PropertyPathParam propertyPath, const Any& oldValue);
  static void RegisterSideEffectProperty(HandleParam object, StringParam propertyName, const Any& oldValue);
  void QueueRegisteredSideEffects();

  /// The ObjectLink Component has the property 'ObjectAPath' of type CogPath.
  /// When you modify the 'ResolvedCog' property on CogPath, it also modifies
  /// the Path property. We want to make a side effect operation to also modify
  /// the Path property. However, CogPath cannot store local modifications
  /// because we cannot get a safe handle to it (it doesn't know what contains
  /// it).
  ///
  /// When ResolvedCog on CogPath is modified in the editor, the instance is the
  /// ObjectLink Component, and the value changed is "ObjectAPath/ResolvedCog".
  /// When the setter marks the 'Path' property as a side effect, we want it be
  /// the same instance with "ObjectAPath/Path" as the modified property.
  /// Unfortunately, that information is not available within the
  /// CogPath::SetResolvedCog setter.
  ///
  /// SideEffectContext was added for this reason. When the
  /// "ObjectAPath/ResolvedCog" is modified, it is pushed onto a context stack
  /// so that if there is a side effect, it can read from that stack and have
  /// the correct instance and build the correct path. In this case, what is
  /// pushed onto the stack is the instance of ObjectLink and "ObjectAPath",
  /// allowing the 'Path' side effect to be added to the end of the stored path
  /// on the stack.
  static void PushSubPropertyContext(HandleParam object, PropertyPathParam contextPath);
  static void PopSubPropertyContext();

  struct SideEffectContext
  {
    SideEffectContext()
    {
    }
    SideEffectContext(HandleParam context, PropertyPathParam path);

    Handle mContext;
    PropertyPath mRootPath;
  };

  static bool sListeningForSideEffects;
  static Array<PropertyOperation*> sSideEffects;
  static Array<SideEffectContext> sSideEffectContextStack;
};

typedef u64 UndoObjectId;
const UndoObjectId cInvalidUndoObjectId = 0;

// Undo Handle
class UndoHandle
{
public:
  UndoHandle();
  UndoHandle(HandleParam object);

  void operator=(HandleParam rhs);

  void SetObject(HandleParam object);

  /// Returns the object this handle is referencing.
  Handle GetHandle() const;

  template <typename T>
  T Get() const;

  /// If an object was re-created (destroy Cog and undo), call this to update
  /// the live object in the undo map.
  void UpdateObject(HandleParam newObject);

private:
  /// If we were able to
  UndoObjectId mUndoId;

  /// If the object type doesn't support being put into the undo map, we can
  /// do operations on it until the handle goes null. Once it goes null, all
  /// operations on it are invalid.
  Handle mRawObject;
};

template <typename T>
T UndoHandle::Get() const
{
  return GetHandle().Get<T>();
}

// Undo Handle Of
template <typename T>
class UndoHandleOf : public UndoHandle
{
public:
  UndoHandleOf()
  {
  }

  UndoHandleOf(T* object) : UndoHandle(object)
  {
  }

  UndoHandleOf(const HandleOf<T>& object) : UndoHandle(object)
  {
  }

  operator T*() const
  {
    return Get<T*>();
  }
};

// Undo Map
class UndoMap
{
public:
  static void Initialize();
  static void Shutdown();

  UndoMap();

  friend class UndoHandle;

  /// Object has been created under a new Id or created for the first time.
  UndoObjectId UpdateUndoId(UndoObjectId undoId, HandleParam object);

  /// Updates the object undo handle only if it was already in the undo map.
  void UpdateHandleIfExists(HandleParam oldObject, HandleParam newObject);

  Handle FindObject(UndoObjectId undoId);

  UndoObjectId GetUndoId(HandleParam object, bool createEntry = true);

  template <typename type>
  type* FindObject(UndoObjectId undoId)
  {
    return (type*)FindObject(undoId).Get<type*>();
  }

private:
  u64 GetObjectId(HandleParam object);

  // Undo id to live object.
  HashMap<UndoObjectId, Handle> UndoIdToObject;

  // Live object id to undo id.
  HashMap<u64, UndoObjectId> ObjectToUndoId;

  // Current incrementing undo id.
  UndoObjectId UndoIdNum;
};

namespace Z
{
extern UndoMap* gUndoMap;
}

} // namespace Raverie
