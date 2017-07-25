///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Containers/ArraySet.hpp"
#include "Containers/BitStream.hpp"

namespace Zero
{

/// Maintains a complete ID sequence record
/// May be used with wrap-aware ID's for persistent sequence records
template <typename Id>
class IdSequence
{
public:
  /// Typedefs
  typedef Id id_type;

  /// Default Constructor
  IdSequence()
    : mNext(1),
      mMissing(),
      mVerified(0)
  {
  }

  /// Move Constructor
  IdSequence(MoveReference<IdSequence> rhs)
    : mNext(rhs->mNext),
      mMissing(ZeroMove(rhs->mMissing)),
      mVerified(rhs->mVerified)
  {
  }

  /// Move Assignment Operator
  IdSequence& operator =(MoveReference<IdSequence> rhs)
  {
    mNext     = rhs->mNext;
    mMissing  = ZeroMove(rhs->mMissing);
    mVerified = rhs->mVerified;

    return *this;
  }

  /// Returns true if the specified ID is a duplicate, else false
  bool IsDuplicate(Id id) const
  {
    // Before Verified?
    if(id <= mVerified)
      return true;
    // Between Verified and Next?
    else if(id < mNext)
    {
      // Not Missing?
      ArraySet<Id>::const_iterator iter = mMissing.FindIterator(id);
      if(iter == mMissing.End())
        return true;
    }

    return false;
  }

  /// Attempts to add the specified ID to the ID sequence
  /// Returns true if successful, else false (is duplicate)
  bool Add(Id id)
  {
    Assert(mVerified < mNext);

    //
    // Add to Sequence
    //

    // Before Verified?
    if(id <= mVerified)
      return false; // Duplicate
    // Between Verified and Next?
    else if(id < mNext)
    {
      // Not Missing?
      ArraySet<Id>::iterator iter = mMissing.FindIterator(id);
      if(iter == mMissing.End())
        return false; /// Duplicate
      // Otherwise,
      mMissing.Erase(iter); /// Erase Missing entry
    }
    // Is Next?
    else if(id == mNext)
      ++mNext;
    // After Next?
    else
    {
      Assert(id > mNext);
      while(mNext != id)
      {
        mMissing.Insert(mNext);
        ++mNext;
      }
      ++mNext;
    }

    // Update Verified
    mVerified = mMissing.Empty() ? mNext - 1 : mMissing.Front() - 1;
    Assert(mVerified < mNext);

    // Success
    return true;
  }

  /// Returns true if the specified ID is verified (preceding sequence is complete), else false
  bool IsVerified(Id id) const
  {
    return id <= mVerified;
  }

  /// ID sequence history record
  struct IdSequenceHistory
  {
    /// Default Constructor
    IdSequenceHistory()
      : mNext(0),
        mHistory()
    {
    }

    /// Constructor
    IdSequenceHistory(Id next, MoveReference<BitStream> history)
      : mNext(next),
        mHistory(ZeroMove(history))
    {
    }

    /// Move Constructor
    IdSequenceHistory(MoveReference<IdSequenceHistory> rhs)
      : mNext(rhs->mNext),
        mHistory(ZeroMove(rhs->mHistory))
    {
    }

    Id        mNext;    /// Next expected ID
    BitStream mHistory; /// History bitfield
  };

  /// Returns the current ID sequence history through the specified range
  IdSequenceHistory GetSequenceHistory(uint range) const
  {
    Assert(range > 0);

    // Generate history bitfield
    BitStream history;
    Id        current = mNext;
    while(range)
    {
      --current;

      // Verified, or not missing?
      if(IsVerified(current)
      || !mMissing.Contains(current))
        history.WriteBit(true);
      else
        history.WriteBit(false);

      --range;
    }

    return IdSequenceHistory(mNext, ZeroMove(history));
  }

private:
  Id           mNext;     /// Next expected cursor
  ArraySet<Id> mMissing;  /// Missing SequenceNumbers (between verified and next)
  Id           mVerified; /// Verified complete (gap-free) cursor
};

/// IdSequence Move-Without-Destruction Operator
template <typename Id>
struct MoveWithoutDestructionOperator< IdSequence<Id> >
{
  static inline void MoveWithoutDestruction(IdSequence<Id>* dest, IdSequence<Id>* source)
  {
    new(dest) IdSequence<Id>(ZeroMove(*source));
  }
};

} //  namespace Zero
