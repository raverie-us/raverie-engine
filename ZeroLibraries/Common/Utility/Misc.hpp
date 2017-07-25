///////////////////////////////////////////////////////////////////////////////
///
/// \file Misc.hpp
/// Miscellaneous functions.
///
/// Authors: 
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Utility/Typedefs.hpp"
#include "Common/Time.hpp"

namespace Zero
{

//Smaller of two is put in the top 32 bits.
u64 GetLexicographicId(u32 id1, u32 id2);
//Unpacks the pair id into the two ids. Id1 is the lower 32 bits.
void UnPackLexicographicId(u32& id1, u32& id2, u64 pairId);

template <typename T>
size_t RangeCount(T range)
{
  size_t count = 0;
  while (!range.Empty())
  {
    ++count;
    range.PopFront();
  }

  return count;
}

template <typename T>
class RecallOnDestruction
{
public:
  T* mVariable;
  T mRecallToValue;

  RecallOnDestruction(T* variable)
  {
    mRecallToValue = *variable;
    mVariable = variable;
  }

  RecallOnDestruction(T* variable, T recallToValue)
  {
    mRecallToValue = recallToValue;
    mVariable = variable;
  }

  ~RecallOnDestruction()
  {
    *mVariable = mRecallToValue;
  }
};

template <typename T>
class SetAndRecallOnDestruction : public RecallOnDestruction<T>
{
public:
  SetAndRecallOnDestruction(T* variable, T setToValue) :
    RecallOnDestruction<T>(variable)
  {
    *variable = setToValue;
  }

  SetAndRecallOnDestruction(T* variable, T setToValue, T recallToValue) :
    RecallOnDestruction<T>(variable, recallToValue)
  {
    *variable = setToValue;
  }
};

template<typename RefType>
void SafeRelease(RefType& interfacePtr)
{
  if(interfacePtr)
    interfacePtr->Release();
  interfacePtr = nullptr;
}

template<typename RefType>
void SafeDelete(RefType& objectPtr)
{
  delete objectPtr;
  objectPtr = nullptr;
}

template<typename RefType>
void SafeDeleteArray(RefType& objectPtr)
{
  delete[] objectPtr;
  objectPtr = nullptr;
}

template<typename type>
void SafeDestroy(type*& instance)
{
  if(instance)
    instance->Destroy();
  instance = nullptr;
}

template<typename type>
void ZeroClassMemory(type& classRef)
{
  memset(&classRef, 0, sizeof(type));
}

extern const TimeType cTimeMax;

/// Test if the current machine is big or little endian
bool IsBigEndian();

/// Returns number of least significant zeros
/// If x is strictly a power of 2, will result in n where 2^n=x, values [0, 31]
/// More information: http://en.wikipedia.org/wiki/Find_first_set
u32 CountTrailingZeros(u32 x);
/// Returns number of most significant zeros
u32 CountLeadingZeros(u32 x);

/// Will result in zero if most significant bit is set
u32 NextPowerOfTwo(u32 x);

}//namespace Zero
