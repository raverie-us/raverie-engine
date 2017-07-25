///////////////////////////////////////////////////////////////////////////////
///
/// Authors: 
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class VisibilityFlag
{
public:
  static const uint sMaxVisibilityId = 31;

  VisibilityFlag() : mFlags(0) {}

  inline void SetFlag(uint visibilityId)
  {
    mFlags |= (1u << visibilityId);
  }

  inline void ClearFlag(uint visibilityId)
  {
    mFlags &= ~(1u << visibilityId);
  }

  inline void ClearAll()
  {
    mFlags = 0;
  }

  inline void SetAll()
  {
    mFlags = (uint)-1;
  }

  inline bool CheckFlag(uint visibilityId)
  {
    return (mFlags & (1u << visibilityId)) != 0;
  }

  inline bool Any()
  {
    return mFlags != 0;
  }

  uint mFlags;

  class ChangeRange
  {
  public:
    ChangeRange(uint flagsA, uint flagsB)
    {
      mDeltaFlags = flagsA ^ flagsB;
      PopFront();
    }

    bool Empty()
    {
      return mCurrentId > sMaxVisibilityId;
    }

    uint Front()
    {
      return mCurrentId;
    }

    void PopFront()
    {
      if (!mDeltaFlags)
      {
        mCurrentId = (uint)-1;
        return;
      }

      uint flagsLeft = mDeltaFlags & (mDeltaFlags - 1);
      uint currentFlag = mDeltaFlags ^ flagsLeft;

      // Get id for isolated flag
      mCurrentId = CountTrailingZeros(currentFlag);

      mDeltaFlags = flagsLeft;
    }

    uint mDeltaFlags;
    uint mCurrentId;
  };

  inline ChangeRange GetChangeRange(VisibilityFlag& visibilityFlag)
  {
    return ChangeRange(mFlags, visibilityFlag.mFlags);
  }
};

} // namespace Zero
