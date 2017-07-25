///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ThreadSafeReferenceCountedHandleData
{
public:
  void* mRawObject;
  u64 mId;
};

// Reference counted handles that will go null if destroyed from C++
template <typename T>
class ThreadSafeReferenceCountedHandleManager : public HandleManager
{
public:
  ThreadSafeReferenceCountedHandleManager(ExecutableState* state) : HandleManager(state) {}

  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override
  {
    ThreadSafeReferenceCountedHandleData& data = *(ThreadSafeReferenceCountedHandleData*)(handleToInitialize.Data);
    data.mId = (u64)-1;
    data.mRawObject = zAllocate(type->Size);
  }

  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override
  {
    if (object == nullptr)
      return;
    
    T* instance = (T*)object;

    ThreadSafeReferenceCountedHandleData& data = *(ThreadSafeReferenceCountedHandleData*)(handleToInitialize.Data);
    data.mId = instance->mHandleId.mId;
    data.mRawObject = nullptr;

    instance->AddReference();
  }

  byte* HandleToObject(const Handle& handle) override
  {
    ThreadSafeReferenceCountedHandleData& data = *(ThreadSafeReferenceCountedHandleData*)(handle.Data);

    if (data.mRawObject)
      return (byte*)data.mRawObject;

    T::mLock.Lock();
    T* val = T::mLiveObjects.FindValue(data.mId, nullptr);
    T::mLock.Unlock();
    // METAREFACTOR - This manager is currently expected to be used on the base class define below (ThreadSafeReferenceCounted)
    // when getting the derived object, I do not believe this will thunk correctly if required.
    return (byte*)val;
  }

  void AddReference(const Handle& handle) override
  {
    ThreadSafeReferenceCountedHandleData& data = *(ThreadSafeReferenceCountedHandleData*)(handle.Data);

    T* instance = (T*)data.mRawObject;
    if (instance == nullptr)
    {
      T::mLock.Lock();
      instance = T::mLiveObjects.FindValue(data.mId, nullptr);
      T::mLock.Unlock();
    }

    ErrorIf(instance == nullptr, "Adding reference on a null handle.");
    instance->AddReference();
  }

  ReleaseResult::Enum ReleaseReference(const Handle& handle) override
  {
    ThreadSafeReferenceCountedHandleData& data = *(ThreadSafeReferenceCountedHandleData*)(handle.Data);

    T* instance = (T*)data.mRawObject;
    if (instance == nullptr)
    {
      T::mLock.Lock();
      instance = T::mLiveObjects.FindValue(data.mId, nullptr);
      T::mLock.Unlock();
    }

    ErrorIf(instance == nullptr, "Releasing reference on a null handle.");
    instance->Release();
    return ReleaseResult::TakeNoAction;
  }
};

// Call within the class definition
#define DeclareThreadSafeReferenceCountedHandle(type)                                 \
  static ThreadLock mLock;                                                            \
  static u64 mCurrentId;                                                              \
  static HashMap<u64, type*> mLiveObjects;                                            \
  HandleIdData<u64> mHandleId;                                                        \
  ReferenceCountData mReferenceCount;                                                 \
  int GetReferenceCount() { return mReferenceCount.mCount; }                          \
  void AddReference() { ++mReferenceCount.mCount; }                                   \
  int Release()                                                                       \
  {                                                                                   \
    ErrorIf(mReferenceCount.mCount == 0, "Invalid Release. ReferenceCount is zero."); \
    int referenceCount = --mReferenceCount.mCount;                                    \
    if (referenceCount == 0) delete this;                                             \
    return referenceCount;                                                            \
  }

// Call in the cpp file next to the class implementation
#define DefineThreadSafeReferenceCountedHandle(type) \
  ThreadLock type::mLock;                            \
  u64 type::mCurrentId = 1;                          \
  HashMap<u64, type*> type::mLiveObjects;

// Call in the constructor of the object
#define ConstructThreadSafeReferenceCountedHandle()                                                                      \
  ErrorIf(ZilchVirtualTypeId(this)->HandleManager != ZilchManagerId(ThreadSafeReferenceCountedHandleManager<ZilchSelf>), \
    "Set type->HandleManager = ZilchManagerId(ThreadSafeReferenceCountedHandleManager<ZilchSelf>; in binding");          \
  mLock.Lock();                                                                                                          \
  mHandleId.mId = mCurrentId++;                                                                                          \
  mLiveObjects.Insert(mHandleId.mId, this);                                                                              \
  mLock.Unlock();                                                                                                        \
  mReferenceCount.mCount = 1;

// Call in the destructor of the object
#define DestructThreadSafeReferenceCountedHandle()                                        \
  mLock.Lock();                                                                           \
  bool isErased = mLiveObjects.Erase(mHandleId.mId);                                      \
  mLock.Unlock();                                                                         \
  ErrorIf(!isErased, "The handle was not in the live objects map, but should have been");

// Call in the meta definition of the object
#define ZeroBindThreadSafeReferenceCountedHandle()                                          \
  type->HandleManager = ZilchManagerId(ThreadSafeReferenceCountedHandleManager<ZilchSelf>);

// Call in engine/system initialization for type that will be using this manager
#define ZeroRegisterThreadSafeReferenceCountedHandleManager(type) ZilchRegisterSharedHandleManager(ThreadSafeReferenceCountedHandleManager<type>);

// Inherit from this class to get all standard behavior of this handle manager
class ThreadSafeReferenceCounted
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareThreadSafeReferenceCountedHandle(ThreadSafeReferenceCounted);

  ThreadSafeReferenceCounted();
  ThreadSafeReferenceCounted(const ThreadSafeReferenceCounted&);
  virtual ~ThreadSafeReferenceCounted();
};

} // namespace Zero
