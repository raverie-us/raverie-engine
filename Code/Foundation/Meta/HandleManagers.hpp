// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// ReferenceCounted
// SafeId
// ThreadSafe

// This is used to avoid people copy constructing their classes and copying over
// the Id.
template <typename IdType>
struct HandleIdData
{
  HandleIdData()
  {
  }
  HandleIdData(const HandleIdData& rhs)
  {
  }
  HandleIdData& operator=(const HandleIdData& rhs)
  {
    return *this;
  }

  IdType mId;
};

// Prevents count from being copied
class ReferenceCountData
{
public:
  ReferenceCountData()
  {
  }
  ReferenceCountData(const ReferenceCountData&)
  {
  }
  ReferenceCountData& operator=(const ReferenceCountData&)
  {
    return *this;
  }

  Atomic<int> mCount;
};

// Declare
// Call within the class definition
#define DeclareReferenceCountedHandle() DeclareReferenceCountedHandleInternals() typedef u32 HandleIdType;

// The reason for adding this internals version was so that we could avoid two
// HandleIdType typedefs. This should never be called outside this file.
#define DeclareReferenceCountedHandleInternals()                                                                       \
  ReferenceCountData mRaverieHandleReferenceCount;                                                                        \
  void AddReference()                                                                                                  \
  {                                                                                                                    \
    ++mRaverieHandleReferenceCount.mCount;                                                                                \
  }                                                                                                                    \
  int Release()                                                                                                        \
  {                                                                                                                    \
    ErrorIf(mRaverieHandleReferenceCount.mCount == 0, "Invalid Release. ReferenceCount is zero.");                        \
    int referenceCount = --mRaverieHandleReferenceCount.mCount;                                                           \
    if (referenceCount == 0)                                                                                           \
      delete this;                                                                                                     \
    return referenceCount;                                                                                             \
  }

#define DeclareSafeIdHandle(idType)                                                                                    \
  typedef idType HandleIdType;                                                                                         \
  HandleIdData<HandleIdType> mRaverieHandleId;                                                                            \
  static HandleIdType mRaverieHandleCurrentId;                                                                            \
  static HashMap<HandleIdType, RaverieSelf*> mRaverieHandleLiveObjects;

#define DeclareThreadSafeIdHandle(idType) DeclareSafeIdHandle(idType) static ThreadLock mRaverieHandleLock;

#define DeclareReferenceCountedSafeIdHandle(idType) DeclareReferenceCountedHandleInternals() DeclareSafeIdHandle(idType)

#define DeclareReferenceCountedThreadSafeIdHandle(idType)                                                              \
  DeclareReferenceCountedHandleInternals() DeclareThreadSafeIdHandle(idType)

// Define
#define DefineSafeIdHandle(type)                                                                                       \
  type::HandleIdType type::mRaverieHandleCurrentId = 1;                                                                   \
  HashMap<type::HandleIdType, type*> type::mRaverieHandleLiveObjects;

#define DefineThreadSafeIdHandle(type) DefineSafeIdHandle(type) ThreadLock type::mRaverieHandleLock;

#define DefineReferenceCountedSafeIdHandle(type) DefineReferenceCountedHandle(type) DefineSafeIdHandle(type)

#define DefineReferenceCountedThreadSafeIdHandle(type) DefineReferenceCountedHandle(type) DefineThreadSafeIdHandle(type)

// Constructor
// Call in the constructor and copy constructor of the object
#define ConstructReferenceCountedHandle() mRaverieHandleReferenceCount.mCount = 0;

#define ConstructSafeIdHandle()                                                                                        \
  mRaverieHandleId.mId = mRaverieHandleCurrentId++;                                                                          \
  mRaverieHandleLiveObjects.Insert(mRaverieHandleId.mId, this);

#define ConstructThreadSafeIdHandle()                                                                                  \
  mRaverieHandleLock.Lock();                                                                                              \
  ConstructSafeIdHandle();                                                                                             \
  mRaverieHandleLock.Unlock();

#define ConstructReferenceCountedSafeIdHandle()                                                                        \
  ConstructReferenceCountedHandle();                                                                                   \
  ConstructSafeIdHandle();

#define ConstructReferenceCountedThreadSafeIdHandle()                                                                  \
  ConstructReferenceCountedHandle();                                                                                   \
  ConstructThreadSafeIdHandle();

// Destructor
// Call in the destructor of the object
#define DestructReferenceCountedHandle()                                                                               \
  ErrorIf(mRaverieHandleReferenceCount.mCount != 0, "Bad reference Count. Object is being deleted with references!");

#define DestructSafeIdHandle()                                                                                         \
  bool isErased = mRaverieHandleLiveObjects.Erase(mRaverieHandleId.mId);                                                     \
  ErrorIf(!isErased, "The handle was not in the live objects map, but should have been");

#define DestructThreadSafeIdHandle()                                                                                   \
  mRaverieHandleLock.Lock();                                                                                              \
  DestructSafeIdHandle();                                                                                              \
  mRaverieHandleLock.Unlock();

// For the last two, we don't want to call 'DestructReferenceCountedHandle'
// because it's safe to delete the object even if it still has references
#define DestructReferenceCountedSafeIdHandle() DestructSafeIdHandle();

#define DestructReferenceCountedThreadSafeIdHandle() DestructThreadSafeIdHandle();

// Bind Manager
// Call in the meta initialization of the class type
#define RaverieBindHandle() type->HandleManager = RaverieManagerId(RaverieHandleManager<RaverieSelf>);

// Register Manager
// Call in the meta initialization of the library
#define RaverieRegisterHandleManager(type) RaverieRegisterSharedHandleManager(RaverieHandleManager<type>);

// Handle Manager
template <typename T>
class RaverieHandleManager : public HandleManager
{
public:
  RaverieHandleManager(ExecutableState* state) : HandleManager(state)
  {
  }

  typedef typename T::HandleIdType IdType;

  RaverieDeclareHasMemberTrait(IsReferenceCounted, mRaverieHandleReferenceCount);
  RaverieDeclareHasMemberTrait(IsSafeId, mRaverieHandleId);
  RaverieDeclareHasMemberTrait(IsThreadSafe, mRaverieHandleLock);

  // Handle Data
  struct HandleData
  {
    void* mRawObject;
    IdType mId;
  };

  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override
  {
    if (IsReferenceCounted<T>::value)
      handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;

    if (IsSafeId<T>::value)
    {
      // METAREFACTOR - If we only ever go through RaverieAllocate for objects
      // that use this handle manager, we can assign the id here and not have to
      // deal with the raw object issue. Is this something we should do? Or is
      // that annoying to RaverieAllocate everything?
      HandleData& data = *(HandleData*)(handleToInitialize.Data);
      data.mId = (IdType)-1;
      data.mRawObject = zAllocate(type->Size);
    }
  }

  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override
  {
    ObjectToHandleInternal(object, type, handleToInitialize);
  }

  byte* HandleToObject(const Handle& handle) override
  {
    return HandleToObjectInternal(handle);
  }

  void AddReference(const Handle& handle) override
  {
    AddReferenceInternal(handle);
  }

  ReleaseResult::Enum ReleaseReference(const Handle& handle) override
  {
    return ReleaseReferenceInternal(handle);
  }

  // Internals
  // Internal versions of each function were added because they need to be
  // templated for SFINAE

  // ObjectToHandle
  // Only reference counted
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object,
                              BoundType* type,
                              Handle& handleToInitialize,
                              P_ENABLE_IF(IsReferenceCounted<U>::value && !IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    if (instance != nullptr)
      instance->AddReference();

    handleToInitialize.HandlePointer = (byte*)object;
  }

  // Only safe id
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object,
                              BoundType* type,
                              Handle& handleToInitialize,
                              P_ENABLE_IF(!IsReferenceCounted<U>::value && IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    HandleData& data = *(HandleData*)(handleToInitialize.Data);
    if (instance != nullptr)
      data.mId = instance->mRaverieHandleId.mId;
  }

  // Reference counted and safe id
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object,
                              BoundType* type,
                              Handle& handleToInitialize,
                              P_ENABLE_IF(IsReferenceCounted<U>::value&& IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    HandleData& data = *(HandleData*)(handleToInitialize.Data);
    data.mId = 0;

    if (instance != nullptr)
    {
      instance->AddReference();
      data.mId = instance->mRaverieHandleId.mId;
    }
  }

  // HandleToObject
  // Only reference counted
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value && !IsSafeId<U>::value))
  {
    return (byte*)handle.HandlePointer;
  }

  // Only safe id
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle, P_ENABLE_IF(IsSafeId<U>::value && !IsThreadSafe<U>::value))
  {
    HandleData& data = *(HandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T* object = T::mRaverieHandleLiveObjects.FindValue(data.mId, nullptr);
    return (byte*)object;
  }

  // Thread safe id
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle, P_ENABLE_IF(IsSafeId<U>::value&& IsThreadSafe<U>::value))
  {
    HandleData& data = *(HandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T::mRaverieHandleLock.Lock();
    T* object = T::mRaverieHandleLiveObjects.FindValue(data.mId, nullptr);
    T::mRaverieHandleLock.Unlock();
    return (byte*)object;
  }

  // AddReference
  template <typename U = T>
  void AddReferenceInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value))
  {
    T* instance = (T*)HandleToObject(handle);
    instance->AddReference();
  }

  template <typename U = T>
  void AddReferenceInternal(const Handle& handle, P_DISABLE_IF(IsReferenceCounted<U>::value))
  {
  }

  // ReleaseReference
  template <typename U = T>
  ReleaseResult::Enum ReleaseReferenceInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value))
  {
    T* instance = (T*)HandleToObject(handle);
    instance->Release();

    // METAREFACTOR - To get rid of Reference class below, Release has to change
    // to not manually deleting itself. This will not call the raverie destructor
    // on inherited types, causing leaks.
    return ReleaseResult::TakeNoAction;
  }

  template <typename U = T>
  ReleaseResult::Enum ReleaseReferenceInternal(const Handle& handle, P_DISABLE_IF(IsReferenceCounted<U>::value))
  {
    return ReleaseResult::TakeNoAction;
  }
};

// Raverie Handle Object
// Used as the default base for inheritable handle types
class EmptyClass
{
};

// Reference Counted
template <typename Base = EmptyClass>
class ReferenceCounted : public Base
{
public:
  RaverieDeclareType(ReferenceCounted, TypeCopyMode::ReferenceType);
  DeclareReferenceCountedHandle();

  ReferenceCounted()
  {
    ConstructReferenceCountedHandle();
  }

  ReferenceCounted(const ReferenceCounted&)
  {
    ConstructReferenceCountedHandle();
  }

  virtual ~ReferenceCounted()
  {
    DestructReferenceCountedHandle();
  }
};

template <typename Base>
void ReferenceCounted<Base>::RaverieSetupType(::Raverie::LibraryBuilder& builder, ::Raverie::BoundType* type)
{
  RaverieBindHandle();
}

// Safe Id
template <typename idType, typename Base = EmptyClass>
class SafeId : public Base
{
public:
  RaverieDeclareType(SafeId, TypeCopyMode::ReferenceType);
  DeclareSafeIdHandle(idType);

  SafeId()
  {
    ConstructSafeIdHandle();
  }

  SafeId(const SafeId&)
  {
    ConstructSafeIdHandle();
  }

  virtual ~SafeId()
  {
    DestructSafeIdHandle();
  }

  SafeId& operator=(const SafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename SafeId<idType, Base>::HandleIdType SafeId<idType, Base>::mRaverieHandleCurrentId = 1;
template <typename idType, typename Base>
HashMap<typename SafeId<idType, Base>::HandleIdType, SafeId<idType, Base>*>
    SafeId<idType, Base>::mRaverieHandleLiveObjects;

template <typename idType, typename Base>
void SafeId<idType, Base>::RaverieSetupType(::Raverie::LibraryBuilder& builder, ::Raverie::BoundType* type)
{
  RaverieBindHandle();
}

// Thread Safe Id
template <typename idType, typename Base = EmptyClass>
class ThreadSafeId : public Base
{
public:
  RaverieDeclareType(ThreadSafeId, TypeCopyMode::ReferenceType);
  DeclareThreadSafeIdHandle(idType);

  ThreadSafeId()
  {
    ConstructThreadSafeIdHandle();
  }

  ThreadSafeId(const ThreadSafeId&)
  {
    ConstructThreadSafeIdHandle();
  }

  virtual ~ThreadSafeId()
  {
    DestructThreadSafeIdHandle();
  }

  ThreadSafeId& operator=(const ThreadSafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename ThreadSafeId<idType, Base>::HandleIdType ThreadSafeId<idType, Base>::mRaverieHandleCurrentId = 1;
template <typename idType, typename Base>
HashMap<typename ThreadSafeId<idType, Base>::HandleIdType, ThreadSafeId<idType, Base>*>
    ThreadSafeId<idType, Base>::mRaverieHandleLiveObjects;
template <typename idType, typename Base>
ThreadLock ThreadSafeId<idType, Base>::mRaverieHandleLock;

template <typename idType, typename Base>
void ThreadSafeId<idType, Base>::RaverieSetupType(::Raverie::LibraryBuilder& builder, ::Raverie::BoundType* type)
{
  RaverieBindHandle();
}
// Reference Counted Safe Id
template <typename idType, typename Base = EmptyClass>
class ReferenceCountedSafeId : public Base
{
public:
  RaverieDeclareType(ReferenceCountedSafeId, TypeCopyMode::ReferenceType);
  DeclareReferenceCountedSafeIdHandle(idType);

  ReferenceCountedSafeId()
  {
    ConstructReferenceCountedSafeIdHandle();
  }

  ReferenceCountedSafeId(const ReferenceCountedSafeId&)
  {
    ConstructReferenceCountedSafeIdHandle();
  }

  virtual ~ReferenceCountedSafeId()
  {
    DestructReferenceCountedSafeIdHandle();
  }

  ReferenceCountedSafeId& operator=(const ReferenceCountedSafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename ReferenceCountedSafeId<idType, Base>::HandleIdType ReferenceCountedSafeId<idType, Base>::mRaverieHandleCurrentId =
    1;
template <typename idType, typename Base>
HashMap<typename ReferenceCountedSafeId<idType, Base>::HandleIdType, ReferenceCountedSafeId<idType, Base>*>
    ReferenceCountedSafeId<idType, Base>::mRaverieHandleLiveObjects;

template <typename idType, typename Base>
void ReferenceCountedSafeId<idType, Base>::RaverieSetupType(::Raverie::LibraryBuilder& builder, ::Raverie::BoundType* type)
{
  RaverieBindHandle();
}
// Counted Thread Safe Id
template <typename idType, typename Base = EmptyClass>
class ReferenceCountedThreadSafeId : public Base
{
public:
  RaverieDeclareType(ReferenceCountedThreadSafeId, TypeCopyMode::ReferenceType);
  DeclareReferenceCountedThreadSafeIdHandle(idType);

  ReferenceCountedThreadSafeId()
  {
    ConstructReferenceCountedThreadSafeIdHandle();
  }

  ReferenceCountedThreadSafeId(const ReferenceCountedThreadSafeId&)
  {
    ConstructReferenceCountedThreadSafeIdHandle();
  }

  virtual ~ReferenceCountedThreadSafeId()
  {
    DestructReferenceCountedThreadSafeIdHandle();
  }

  ReferenceCountedThreadSafeId& operator=(const ReferenceCountedThreadSafeId& rhs)
  {
    return *this;
  }
};

template <typename idType, typename Base>
typename ReferenceCountedThreadSafeId<idType, Base>::HandleIdType
    ReferenceCountedThreadSafeId<idType, Base>::mRaverieHandleCurrentId = 1;
template <typename idType, typename Base>
HashMap<typename ReferenceCountedThreadSafeId<idType, Base>::HandleIdType, ReferenceCountedThreadSafeId<idType, Base>*>
    ReferenceCountedThreadSafeId<idType, Base>::mRaverieHandleLiveObjects;
template <typename idType, typename Base>
ThreadLock ReferenceCountedThreadSafeId<idType, Base>::mRaverieHandleLock;

template <typename idType, typename Base>
void ReferenceCountedThreadSafeId<idType, Base>::RaverieSetupType(::Raverie::LibraryBuilder& builder, ::Raverie::BoundType* type)
{
  RaverieBindHandle();
}
} // namespace Raverie
