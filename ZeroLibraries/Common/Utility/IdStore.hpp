///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Containers/ArraySet.hpp"
#include "UintN.hpp"

namespace Zero
{

/// Manages a recyclable ID system (where 0 represents an invalid ID)
/// Designed for use with UintN IDs
template <typename Id>
class IdStore
{
public:
  /// Typedefs
  typedef Id id_type;

  /// Constructor
  IdStore()
  {
    Reset();
  }

  /// Move Constructor
  IdStore(MoveReference<IdStore> rhs)
    : mNewId(rhs->mNewId),
      mFreeIds(ZeroMove(rhs->mFreeIds))
  {
  }

  /// Move Assignment Operator
  IdStore& operator =(MoveReference<IdStore> rhs)
  {
    mNewId   = rhs->mNewId;
    mFreeIds = ZeroMove(rhs->mFreeIds);

    return *this;
  }

  /// Resets the ID store
  void Reset()
  {
    mNewId = 1;
    mFreeIds.Clear();
  }

  /// Acquires the next free ID in the sequence if there are free IDs available
  /// Returns the acquired ID if successful, else 0
  Id AcquireId()
  {
    // Free ID available?
    if(!mFreeIds.Empty())
    {
      Id result = mFreeIds.Front();
      mFreeIds.PopFront();
      return result;
    }

    // New ID available?
    else if(mNewId != 0)
      return mNewId++;

    // No ID available
    else
      return 0;
  }

  /// Frees an ID to be acquired again later
  /// Returns true if successful, else false
  bool FreeId(Id id)
  {
    // Invalid ID?
    if(id == 0)
      return false;

    // Never issued?
    if(mNewId != 0 && id >= mNewId)
      return false;

    // Attempt to put back into free IDs
    return mFreeIds.Insert(id).second;
  }

  /// Returns the next ID to be acquired if available, else 0
  Id GetNextId() const
  {
    // Free ID available?
    if(!mFreeIds.Empty())
      return mFreeIds.Front();

    // New ID available?
    else if(mNewId != 0)
      return mNewId;

    // No ID available
    else
      return 0;
  }

  /// Returns true if there are free IDs available, else false
  bool HasFreeIds() const
  {
    return mNewId != 0 || !mFreeIds.Empty();
  }
  /// Returns the number of free IDs available
  size_t GetFreeIdCount() const
  {
    return size_t((mNewId != 0 ? (Id(Id::max) - mNewId) + 1 : 0).value()) + mFreeIds.Size();
  }

  /// Returns true if there are acquired IDs in use, else false
  bool HasAcquiredIds() const
  {
    return GetAcquiredIdCount() ? true : false;
  }
  /// Returns the number of acquired IDs in use
  size_t GetAcquiredIdCount() const
  {
    return GetTotalIdCount() - GetFreeIdCount();
  }

  /// Returns the total number of IDs possible with this ID type
  static size_t GetTotalIdCount()
  {
    return Id::max;
  }

private:
  /// Data
  Id           mNewId;
  ArraySet<Id> mFreeIds;
};

/// IdStore Move-Without-Destruction Operator
template <typename Id>
struct MoveWithoutDestructionOperator< IdStore<Id> >
{
  static inline void MoveWithoutDestruction(IdStore<Id>* dest, IdStore<Id>* source)
  {
    new(dest) IdStore<Id>(ZeroMove(*source));
  }
};

} // namespace Zero
