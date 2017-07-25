////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// ReferenceCounted
// SafeId
// ThreadSafe

// This is used to avoid people copy constructing their classes and copying over the Id.
template <typename IdType>
struct HandleIdData
{
  HandleIdData() {}
  HandleIdData(const HandleIdData& rhs) {}
  HandleIdData& operator=(const HandleIdData& rhs) { return *this; }

  IdType mId;
};

// Prevents count from being copied
class ReferenceCountData
{
public:
  ReferenceCountData() {}
  ReferenceCountData(const ReferenceCountData&) {}
  ReferenceCountData& operator=(const ReferenceCountData&) { return *this; }

  Atomic<int> mCount;
};

//------------------------------------------------------------------------------------------ Declare
// Call within the class definition
#define DeclareReferenceCountedHandle()                                                            \
  DeclareReferenceCountedHandleInternals()                                                         \
  typedef u32 HandleIdType;                                                                        \

// The reason for adding this internals version was so that we could avoid two HandleIdType
// typedefs. This should never be called outside this file.
#define DeclareReferenceCountedHandleInternals()                                                   \
  ReferenceCountData mZeroHandleReferenceCount;                                                    \
  void AddReference();                                                                             \
  int Release();

#define DeclareSafeIdHandle(idType)                                                                \
  typedef idType HandleIdType;                                                                     \
  HandleIdData<HandleIdType> mZeroHandleId;                                                        \
  static HandleIdType mZeroHandleCurrentId;                                                        \
  static HashMap<HandleIdType, ZilchSelf*> mZeroHandleLiveObjects;

#define DeclareThreadSafeIdHandle(idType)                                                          \
  DeclareSafeIdHandle(idType)                                                                      \
  static ThreadLock mZeroHandleLock;

#define DeclareReferenceCountedSafeIdHandle(idType)                                                \
  DeclareReferenceCountedHandleInternals()                                                         \
  DeclareSafeIdHandle(idType)

#define DeclareReferenceCountedThreadSafeIdHandle(idType)                                          \
  DeclareReferenceCountedHandleInternals()                                                         \
  DeclareThreadSafeIdHandle(idType)

//------------------------------------------------------------------------------------------- Define
// Call in the cpp file next to the class implementation
#define DefineReferenceCountedHandle(type)                                                         \
  void type::AddReference()                                                                        \
  {                                                                                                \
    ++mZeroHandleReferenceCount.mCount;                                                            \
  }                                                                                                \
  int type::Release()                                                                              \
  {                                                                                                \
    ErrorIf(mZeroHandleReferenceCount.mCount == 0, "Invalid Release. ReferenceCount is zero.");    \
    int referenceCount = --mZeroHandleReferenceCount.mCount;                                       \
    if (referenceCount == 0) delete this;                                                          \
    return referenceCount;                                                                         \
  }

#define DefineSafeIdHandle(type)                                                                   \
  type::HandleIdType type::mZeroHandleCurrentId = 1;                                               \
  HashMap<type::HandleIdType, type*> type::mZeroHandleLiveObjects;

#define DefineThreadSafeIdHandle(type)                                                             \
  DefineSafeIdHandle(type)                                                                         \
  ThreadLock type::mZeroHandleLock;                                                                \

#define DefineReferenceCountedSafeIdHandle(type)                                                   \
  DefineReferenceCountedHandle(type)                                                               \
  DefineSafeIdHandle(type)

#define DefineReferenceCountedThreadSafeIdHandle(type)                                             \
  DefineReferenceCountedHandle(type)                                                               \
  DefineThreadSafeIdHandle(type)

//-------------------------------------------------------------------------------------- Constructor
// Call in the constructor and copy constructor of the object
#define ConstructReferenceCountedHandle()                                                          \
  mZeroHandleReferenceCount.mCount = 0;

#define ConstructSafeIdHandle()                                                                    \
  mZeroHandleId.mId = mZeroHandleCurrentId++;                                                      \
  mZeroHandleLiveObjects.Insert(mZeroHandleId.mId, this);

#define ConstructThreadSafeIdHandle()                                                              \
  mZeroHandleLock.Lock();                                                                          \
  ConstructSafeIdHandle();                                                                         \
  mZeroHandleLock.Unlock();

#define ConstructReferenceCountedSafeIdHandle()                                                    \
  ConstructReferenceCountedHandle();                                                               \
  ConstructSafeIdHandle();

#define ConstructReferenceCountedThreadSafeIdHandle()                                              \
  ConstructReferenceCountedHandle();                                                               \
  ConstructThreadSafeIdHandle();

//--------------------------------------------------------------------------------------- Destructor
// Call in the destructor of the object
#define DestructReferenceCountedHandle()                                                           \
  ErrorIf(mZeroHandleReferenceCount.mCount != 0, "Bad reference Count. Object is being deleted with references!");

#define DestructSafeIdHandle()                                                                     \
  bool isErased = mZeroHandleLiveObjects.Erase(mZeroHandleId.mId);                                 \
  ErrorIf(!isErased, "The handle was not in the live objects map, but should have been");

#define DestructThreadSafeIdHandle()                                                               \
  mZeroHandleLock.Lock();                                                                          \
  DestructSafeIdHandle();                                                                          \
  mZeroHandleLock.Unlock();

// For the last two, we don't want to call 'DestructReferenceCountedHandle' because it's safe to 
// delete the object even if it still has references
#define DestructReferenceCountedSafeIdHandle()                                                     \
  DestructSafeIdHandle();

#define DestructReferenceCountedThreadSafeIdHandle()                                               \
  DestructThreadSafeIdHandle();

//------------------------------------------------------------------------------------- Bind Manager
// Call in the meta initialization of the class type
#define ZeroBindHandle()                                                                           \
  type->HandleManager = ZilchManagerId(ZeroHandleManager<ZilchSelf>);

//--------------------------------------------------------------------------------- Register Manager
// Call in the meta initialization of the library
#define ZeroRegisterHandleManager(type)                                                            \
  ZilchRegisterSharedHandleManager(ZeroHandleManager<type>);

//----------------------------------------------------------------------------------- Handle Manager
//**************************************************************************************************
template <typename T>
class ZeroHandleManager : public HandleManager
{
public:
  ZeroHandleManager(ExecutableState* state) : HandleManager(state) {}

  typedef typename T::HandleIdType IdType;

  ZeroDeclareHasMemberTrait(IsReferenceCounted, mZeroHandleReferenceCount);
  ZeroDeclareHasMemberTrait(IsSafeId, mZeroHandleId);
  ZeroDeclareHasMemberTrait(IsThreadSafe, mZeroHandleLock);

  //------------------------------------------------------------------------------------ Handle Data
  struct HandleData
  {
    void* mRawObject;
    IdType mId;
  };

  //************************************************************************************************
  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override
  {
    if (IsReferenceCounted<T>::value)
      handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;

    if (IsSafeId<T>::value)
    {
      // METAREFACTOR - If we only ever go through ZilchAllocate for objects that use this handle
      // manager, we can assign the id here and not have to deal with the raw object issue.
      // Is this something we should do? Or is that annoying to ZilchAllocate everything?
      HandleData& data = *(HandleData*)(handleToInitialize.Data);
      data.mId = (IdType)-1;
      data.mRawObject = zAllocate(type->Size);
    }
  }

  //************************************************************************************************
  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override
  {
    ObjectToHandleInternal(object, type, handleToInitialize);
  }

  //************************************************************************************************
  byte* HandleToObject(const Handle& handle) override
  {
    return HandleToObjectInternal(handle);
  }

  //************************************************************************************************
  void AddReference(const Handle& handle) override
  {
    AddReferenceInternal(handle);
  }

  //************************************************************************************************
  ReleaseResult::Enum ReleaseReference(const Handle& handle) override
  {
    return ReleaseReferenceInternal(handle);
  }

  //-------------------------------------------------------------------------------------- Internals
  // Internal versions of each function were added because they need to be templated for SFINAE

  //--------------------------------------------------------------------------------- ObjectToHandle
  //************************************************************************************************
  // Only reference counted
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object, BoundType* type, Handle& handleToInitialize,
                              P_ENABLE_IF(IsReferenceCounted<U>::value && !IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    if (instance != nullptr)
      instance->AddReference();

    handleToInitialize.HandlePointer = (byte*)object;
  }

  //************************************************************************************************
  // Only safe id
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object, BoundType* type, Handle& handleToInitialize,
                              P_ENABLE_IF(!IsReferenceCounted<U>::value && IsSafeId<U>::value))
  {
    T* instance = (T*)object;

    HandleData& data = *(HandleData*)(handleToInitialize.Data);
    if(instance != nullptr)
      data.mId = instance->mZeroHandleId.mId;
  }

  //************************************************************************************************
  // Reference counted and safe id
  template <typename U = T>
  void ObjectToHandleInternal(const byte* object, BoundType* type, Handle& handleToInitialize,
                              P_ENABLE_IF(IsReferenceCounted<U>::value && IsSafeId<U>::value))
  {
    T* instance = (T*)object;
    
    HandleData& data = *(HandleData*)(handleToInitialize.Data);
    data.mId = 0;

    if (instance != nullptr)
    {
      instance->AddReference();
      data.mId = instance->mZeroHandleId.mId;
    }
  }

  //--------------------------------------------------------------------------------- HandleToObject
  //************************************************************************************************
  // Only reference counted
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle,
                               P_ENABLE_IF(IsReferenceCounted<U>::value && !IsSafeId<U>::value))
  {
    return (byte*)handle.HandlePointer;
  }

  //************************************************************************************************
  // Only safe id
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle,
                               P_ENABLE_IF(IsSafeId<U>::value && !IsThreadSafe<U>::value))
  {
    HandleData& data = *(HandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T* object = T::mZeroHandleLiveObjects.FindValue(data.mId, nullptr);
    return (byte*)object;
  }

  //************************************************************************************************
  // Thread safe id
  template <typename U = T>
  byte* HandleToObjectInternal(const Handle& handle,
                               P_ENABLE_IF(IsSafeId<U>::value && IsThreadSafe<U>::value))
  {
    HandleData& data = *(HandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T::mZeroHandleLock.Lock();
    T* object = T::mZeroHandleLiveObjects.FindValue(data.mId, nullptr);
    T::mZeroHandleLock.Unlock();
    return (byte*)object;
  }

  //----------------------------------------------------------------------------------- AddReference
  //************************************************************************************************
  template <typename U = T>
  void AddReferenceInternal(const Handle& handle, P_ENABLE_IF(IsReferenceCounted<U>::value))
  {
    T* instance = (T*)HandleToObject(handle);
    instance->AddReference();
  }

  //************************************************************************************************
  template <typename U = T>
  void AddReferenceInternal(const Handle& handle, P_DISABLE_IF(IsReferenceCounted<U>::value))
  {
  }

  //------------------------------------------------------------------------------- ReleaseReference
  //************************************************************************************************
  template <typename U = T>
  ReleaseResult::Enum ReleaseReferenceInternal(const Handle& handle,
                                               P_ENABLE_IF(IsReferenceCounted<U>::value))
  {
    T* instance = (T*)HandleToObject(handle);
    instance->Release();
  
    // METAREFACTOR - To get rid of Reference class below, Release has to change to not manually
    // deleting itself. This will not call the zilch destructor on inherited types, causing leaks.
    return ReleaseResult::TakeNoAction;
  }
  
  //************************************************************************************************
  template <typename U = T>
  ReleaseResult::Enum ReleaseReferenceInternal(const Handle& handle,
                                               P_DISABLE_IF(IsReferenceCounted<U>::value))
  {
    return ReleaseResult::TakeNoAction;
  }
};

//------------------------------------------------------------------------------- Zero Handle Object
//**************************************************************************************************
// Used as the default base for inheritable handle types
class EmptyClass { };

//-------------------------------------------------------------------------------- Reference Counted
template <typename Base = EmptyClass>
class ReferenceCounted : public Base
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareReferenceCountedHandle();

  //************************************************************************************************
  ReferenceCounted()
  {
    ConstructReferenceCountedHandle();
  }

  //************************************************************************************************
  ReferenceCounted(const ReferenceCounted&)
  {
    ConstructReferenceCountedHandle();
  }

  //************************************************************************************************
  virtual ~ReferenceCounted()
  {
    DestructReferenceCountedHandle();
  }
};

//------------------------------------------------------------------------------------------ Safe Id
template <typename idType, typename Base = EmptyClass>
class SafeId : public Base
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareSafeIdHandle(idType);

  //************************************************************************************************
  SafeId()
  {
    ConstructSafeIdHandle();
  }

  //************************************************************************************************
  SafeId(const SafeId&)
  {
    ConstructSafeIdHandle();
  }

  //************************************************************************************************
  virtual ~SafeId()
  {
    DestructSafeIdHandle();
  }

  //************************************************************************************************
  SafeId& operator=(const SafeId& rhs)
  {
    return *this;
  }
};

//----------------------------------------------------------------------------------- Thread Safe Id
template <typename idType, typename Base = EmptyClass>
class ThreadSafeId : public Base
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareThreadSafeIdHandle(idType);

  //************************************************************************************************
  ThreadSafeId()
  {
    ConstructThreadSafeIdHandle();
  }

  //************************************************************************************************
  ThreadSafeId(const ThreadSafeId&)
  {
    ConstructThreadSafeIdHandle(); 
  }

  //************************************************************************************************
  virtual ~ThreadSafeId()
  {
    DestructThreadSafeIdHandle();
  }

  //************************************************************************************************
  ThreadSafeId& operator=(const ThreadSafeId& rhs)
  {
    return *this;
  }
};

//------------------------------------------------------------------------ Reference Counted Safe Id
template <typename idType, typename Base = EmptyClass>
class ReferenceCountedSafeId : public Base
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareReferenceCountedSafeIdHandle(idType);

  //************************************************************************************************
  ReferenceCountedSafeId()
  {
    ConstructReferenceCountedSafeIdHandle();
  }

  //************************************************************************************************
  ReferenceCountedSafeId(const ReferenceCountedSafeId&)
  {
    ConstructReferenceCountedSafeIdHandle();
  }

  //************************************************************************************************
  virtual ~ReferenceCountedSafeId()
  {
    DestructReferenceCountedSafeIdHandle();
  }

  //************************************************************************************************
  ReferenceCountedSafeId& operator=(const ReferenceCountedSafeId& rhs)
  {
    return *this;
  }
};

//----------------------------------------------------------------- Reference Counted Thread Safe Id
template <typename idType, typename Base = EmptyClass>
class ReferenceCountedThreadSafeId : public Base
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareReferenceCountedThreadSafeIdHandle(idType);

  //************************************************************************************************
  ReferenceCountedThreadSafeId()
  {
    ConstructReferenceCountedThreadSafeIdHandle();
  }

  //************************************************************************************************
  ReferenceCountedThreadSafeId(const ReferenceCountedThreadSafeId&)
  {
    ConstructReferenceCountedThreadSafeIdHandle();
  }

  //************************************************************************************************
  virtual ~ReferenceCountedThreadSafeId()
  {
    DestructReferenceCountedThreadSafeIdHandle();
  }

  //************************************************************************************************
  ReferenceCountedThreadSafeId& operator=(const ReferenceCountedThreadSafeId& rhs)
  {
    return *this;
  }
};

} // namespace Zero
