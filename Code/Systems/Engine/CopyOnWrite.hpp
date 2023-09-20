// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

template <typename T>
class CopyOnWriteHandle;

template <typename T>
class CopyOnWriteData
{
public:
  friend class CopyOnWriteHandle<T>;

  CopyOnWriteData() : mReferenceCount(0)
  {
  }

  CopyOnWriteData(const CopyOnWriteData& rhs) : mObject(rhs.mObject), mReferenceCount(rhs.mReferenceCount)
  {
  }

  T mObject;

private:
  ThreadLock mLock;
  int mReferenceCount;
};

// This handle should only ever be used by the main thread.
template <typename T>
class CopyOnWriteHandle
{
public:
  CopyOnWriteHandle() : mData(nullptr)
  {
  }

  CopyOnWriteHandle(const CopyOnWriteHandle& rhs) : mData(rhs.mData)
  {
    Lock stackLock(mData->mLock);
    ++mData->mReferenceCount;
  }

  CopyOnWriteHandle(CopyOnWriteData<T>* data) : mData(data)
  {
    Lock stackLock(mData->mLock);
    ++data->mReferenceCount;
  }

  ~CopyOnWriteHandle()
  {
    if (!mData)
      return;

    mData->mLock.Lock();
    if (--mData->mReferenceCount == 0)
    {
      mData->mLock.Unlock();
      delete mData;
    }
    mData->mLock.Unlock();
  }

  CopyOnWriteHandle& operator=(const CopyOnWriteHandle& rhs)
  {
    this->~CopyOnWriteHandle();
    new (this) CopyOnWriteHandle(rhs);
    return *this;
  }

  CopyOnWriteHandle& operator=(CopyOnWriteData<T>* data)
  {
    this->~CopyOnWriteHandle();
    new (this) CopyOnWriteHandle(data);
    return *this;
  }

  T* operator->() const
  {
    ReturnIf(mData == nullptr, nullptr, "Attempting to arrow -> off an invalid CopyOnWriteHandle");
    return &mData->mObject;
  }

  T& operator*() const
  {
    ReturnIf(mData == nullptr, GetInvalidObject<T>(), "Attempting to dereference * off an invalid CopyOnWriteHandle");
    return mData->mObject;
  }

  operator T*() const
  {
    return &mData->mObject;
  }

  // Called right before we make a write or modification to the data.
  // This only copies if the reference count is 1 (meaning we own the object).
  void CopyIfNeeded()
  {
    ReturnIf(!mData, , "The handle did not point at any data so a copy is not possible");

    Lock stackLock(mData->mLock);
    // If we have the only reference count then we don't need to copy
    // Note that the reference count could increase immediately after this line
    // but it may not decrease because we are still holding a reference.
    if (mData->mReferenceCount == 1)
      return;

    // Remove the original reference
    --mData->mReferenceCount;

    // Now make our copy of the object
    mData = new CopyOnWriteData<T>(*mData);
    mData->mReferenceCount = 1;
  }

private:
  CopyOnWriteData<T>* mData;
};

} // namespace Raverie
