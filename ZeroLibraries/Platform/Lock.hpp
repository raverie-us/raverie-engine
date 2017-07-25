///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Containers/TypeTraits.hpp"
#include "ThreadSync.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  Lockable                                       //
//---------------------------------------------------------------------------------//

/// Represents an object which may be Locked
/// Provided to make Locked objects easier to manage
/// The object itself stores it's thread lock
struct ZeroShared Lockable
{
  /// Default Constructor
  Lockable()
    : mLock(new ThreadLock)
  {
  }

  /// Move Constructor
  Lockable(MoveReference<Lockable> rhs)
    : mLock(rhs->mLock)
  {
    rhs->mLock = nullptr;
  }

  /// Move Assignment Operator
  Lockable& operator =(MoveReference<Lockable> rhs)
  {
    if(mLock)
      delete mLock;

    mLock = rhs->mLock;

    rhs->mLock = nullptr;

    return *this;
  }

  /// Destructor
  ~Lockable()
  {
    if(mLock)
      delete mLock;
  }

  void Lock()
  {
    if(mLock)
      mLock->Lock();
  }
  void Unlock()
  {
    if(mLock)
      mLock->Unlock();
  }

  /// Thread lock
  ThreadLock* mLock;
};

//---------------------------------------------------------------------------------//
//                                   Locked                                        //
//---------------------------------------------------------------------------------//

/// Multithreaded locked object
/// Ownership provides exclusive thread-safe access
template <typename T>
struct ZeroSharedTemplate Locked
{
  /// Creates an empty locked object
  Locked()
    : mObject(nullptr),
      mLock(nullptr)
  {
  }

  /// Acquires a locked object
  Locked(T& object, ThreadLock& lock)
    : mObject(&object),
      mLock(&lock)
  {
    mLock->Lock();
  }
  Locked(Lockable& object)
    : mObject(static_cast<T*>(&object)),
      mLock(object.mLock)
  {
    mLock->Lock();
  }

  /// Releases a locked object
  ~Locked()
  {
    if(mLock)
      mLock->Unlock();
  }

  /// Note: Behaves like a move constructor! Provided for convenience only!
  /// Copy Constructor
  Locked(const Locked& rhs)
    : mObject(rhs.mObject),
      mLock(rhs.mLock)
  {
    const_cast<Locked&>(rhs).mObject = nullptr;
    const_cast<Locked&>(rhs).mLock   = nullptr;
  }

  /// Move Constructor
  Locked(MoveReference<Locked> rhs)
    : mObject(rhs->mObject),
      mLock(rhs->mLock)
  {
    rhs->mObject = nullptr;
    rhs->mLock   = nullptr;
  }

  /// Note: Behaves like a move assignment operator! Provided for convenience only!
  /// Copy Assignment Operator
  Locked& operator=(const Locked& rhs)
  {
    if(mLock)
      mLock->Unlock();

    mObject = rhs.mObject;
    mLock   = rhs.mLock;

    const_cast<Locked&>(rhs).mObject = nullptr;
    const_cast<Locked&>(rhs).mLock   = nullptr;

    return *this;
  }

  /// Move Assignment Operator
  Locked& operator=(MoveReference<Locked> rhs)
  {
    if(mLock)
      mLock->Unlock();

    mObject = rhs->mObject;
    mLock   = rhs->mLock;

    rhs->mObject = nullptr;
    rhs->mLock   = nullptr;

    return *this;
  }

  /// Assignment Operator
  Locked& operator=(Lockable& object)
  {
    if(mLock)
      mLock->Unlock();

    mObject = static_cast<T*>(&object);
    mLock   = object.mLock;

    mLock->Lock();

    return *this;
  }

  /// Arrow Operator
  /// Provides direct member access
  T* operator ->() const
  {
    return mObject;
  }

  /// Indirection Operator
  /// Provides reference access
  T& operator *() const
  {
    return *mObject;
  }

  /// Returns true if there is no locked object, else false (there is a locked object)
  bool IsEmpty() const
  {
    return mObject ? false : true;
  }

  /// Releases the locked object to the user
  T* Release()
  {
    if(mLock)
    {
      mLock->Unlock();
      mLock = nullptr;
    }

    T* object = mObject;
    mObject = nullptr;

    return object;
  }

  /// Returns the locked object as the specified type
  template <typename C>
  C* As()
  {
    return static_cast<C*>(mObject);
  }

private:
  /// Locked object
  T*          mObject;
  /// Thread lock
  ThreadLock* mLock;
};

//---------------------------------------------------------------------------------//
//                                    Lock                                         //
//---------------------------------------------------------------------------------//

/// Scoped thread lock
/// Ownership provides exclusive thread-safe access
class ZeroShared Lock
{
public:
  /// Constructor
  Lock(ThreadLock& lock)
    : mLock(&lock)
  {
    mLock->Lock();
  }

  /// Destructor
  ~Lock()
  {
    if(mLock)
      mLock->Unlock();
  }

private:
  /// Thread lock
  ThreadLock* mLock;
};

//---------------------------------------------------------------------------------//
//                                 LockedRange                                     //
//---------------------------------------------------------------------------------//

/// Locked container range of lockable pointer items
template<typename ContainerType>
class ZeroSharedTemplate LockedRange
{
public:
  /// Typedefs
  typedef ContainerType container_type;
  typedef typename container_type::size_type        size_type;
  typedef typename container_type::value_type       value_type;
  typedef typename remove_pointer<value_type>::type dereferenced_type;
  typedef typename container_type::range            range_type;
  typedef Locked<dereferenced_type> locked_type;

  // Default Constructor
  LockedRange()
    : mContainer(),
      mRange(mContainer.All()) // This is correct
  {
  }
  /// Constructor
  /// Note: We copy the container to provide thread-safety for the container itself
  /// This is also why LockedRange is designed for containers of pointer items
  LockedRange(const container_type& container)
    : mContainer(container),
      mRange(mContainer.All()) // This is correct
  {
  }

  /// Copy Constructor
  LockedRange(const LockedRange& rhs)
    : mContainer(rhs.mContainer),
      mRange(mContainer.All()) // This is correct
  {
  }

  /// Move Constructor
  LockedRange(MoveReference<LockedRange> rhs)
    : mContainer(ZeroMove(rhs->mContainer)),
      mRange(mContainer.All()) // This is correct
  {
  }

  /// Copy Assignment Operator
  LockedRange& operator =(const LockedRange& rhs)
  {
    mContainer = rhs.mContainer;
    mRange     = mContainer.All(); // This is correct

    return *this;
  }

  /// Move Assignment Operator
  LockedRange& operator =(MoveReference<LockedRange> rhs)
  {
    mContainer = ZeroMove(rhs->mContainer);
    mRange     = mContainer.All(); // This is correct

    return *this;
  }

  /// Data Access
  void            PopFront()                  { return mRange.PopFront();           }
  void            PopBack()                   { return mRange.PopBack();            }
  locked_type     Front()                     { return *(mRange.Front());           }
  locked_type     Back()                      { return *(mRange.Back());            }
  bool            Empty() const               { return mRange.Empty();              }
  size_type       Length() const              { return mRange.Length();             }
  size_type       Size() const                { return mRange.Size();               }
  locked_type     operator[](size_type index) { return *(mRange.operator[](index)); }

private:
  /// Fixed container copy of lockable pointer items
  container_type mContainer;
  /// Fixed pointer item range
  range_type     mRange;
};

} // namespace Zero
