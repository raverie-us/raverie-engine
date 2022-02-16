// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

Permutation::Permutation() : mInOrder(false), mNoDuplicates(false), mMinIndex(0), mMaxIndex(0)
{
}

Permuter::Permuter() : mArraySize(0), mEmpty(true)
{
}

Permuter::Permuter(size_t arraySize, size_t howManyToSelect) : mArraySize(arraySize), mEmpty(false)
{
  if (howManyToSelect == 0 || arraySize == 0)
  {
    mEmpty = true;
    return;
  }

  // All the indices start at 0
  mPermutation.Resize(howManyToSelect);
  memset(mPermutation.Data(), 0, mPermutation.Size() * sizeof(size_t));

  ComputeStats();
}

bool Permuter::Empty() const
{
  return mEmpty;
}

Permuter::FrontResult Permuter::Front() const
{
  return mPermutation;
}

void Permuter::PopFront()
{
  // Increment the first index
  ++mPermutation[0];

  size_t lastIndex = mPermutation.Size() - 1;

  // Now we need to check if we need to overflow into the next slot
  for (size_t i = 0; i < mPermutation.Size(); ++i)
  {
    // If there's nothing to overflow here, then we're done!
    if (mPermutation[i] != mArraySize)
      break;

    // Otherwise we're overflowing here, but if this is the last value then
    // we're done
    if (i == lastIndex)
    {
      mEmpty = true;
      return;
    }

    // Reset the current index and overflow into the next index
    // Note that we do not need to worry about checking i + 1
    // because we checked if we were the last index above
    mPermutation[i] = 0;
    ++mPermutation[i + 1];
  }

  ComputeStats();
}

void Permuter::ComputeStats()
{
  // Do some more processing to discover if this is in order or has duplicates
  mPermutation.mInOrder = true;
  mPermutation.mNoDuplicates = true;
  mPermutation.mMinIndex = (size_t)-1;
  mPermutation.mMaxIndex = 0;

  // This array tells us if we've already used a component from the selection
  // array
  size_t usedComponentSizeBytes = mArraySize * sizeof(bool);
  bool* usedComponent = (bool*)alloca(usedComponentSizeBytes);
  memset(usedComponent, 0, usedComponentSizeBytes);

  for (size_t i = 0; i < mPermutation.Size(); ++i)
  {
    size_t index = mPermutation[i];

    if (index < mPermutation.mMinIndex)
      mPermutation.mMinIndex = index;
    if (index > mPermutation.mMaxIndex)
      mPermutation.mMaxIndex = index;

    // Check if the components are in order
    if (i > 0 && mPermutation[i - 1] != index - 1)
      mPermutation.mInOrder = false;

    bool& used = usedComponent[index];

    if (used)
      mPermutation.mNoDuplicates = false;

    used = true;
  }
}

MultiPermuter::MultiPermuter() : mMaxSelect(0), mEmpty(true)
{
}

MultiPermuter::MultiPermuter(size_t arraySize, size_t minSelect, size_t maxSelect) :
    mMaxSelect(maxSelect),
    mPermuter(arraySize, minSelect),
    mEmpty(false)
{
  if (minSelect == 0)
    minSelect = 1;

  if (maxSelect < minSelect)
    maxSelect = minSelect;

  if (maxSelect == 0 || arraySize == 0)
    mEmpty = true;
}

bool MultiPermuter::Empty() const
{
  return mEmpty;
}

MultiPermuter::FrontResult MultiPermuter::Front() const
{
  return mPermuter.Front();
}

void MultiPermuter::PopFront()
{
  mPermuter.PopFront();

  if (mPermuter.Empty())
  {
    size_t nextSelect = mPermuter.mPermutation.Size() + 1;

    if (nextSelect > mMaxSelect)
    {
      mEmpty = true;
      return;
    }

    mPermuter = Permuter(mPermuter.mArraySize, nextSelect);
  }
}

} // namespace Zero
