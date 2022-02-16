// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Permutation : public Array<size_t>
{
public:
  Permutation();

  bool mInOrder;
  bool mNoDuplicates;
  size_t mMinIndex;
  size_t mMaxIndex;
};

class Permuter
{
public:
  typedef const Permutation& FrontResult;

  Permuter();
  Permuter(size_t arraySize, size_t selectCount);

  bool Empty() const;
  FrontResult Front() const;
  void PopFront();
  Permuter& All()
  {
    return *this;
  }
  const Permuter& All() const
  {
    return *this;
  }

  // Internals
  void ComputeStats();

  size_t mArraySize;
  size_t mSelectCount;
  Permutation mPermutation;
  bool mEmpty;
};

class MultiPermuter
{
public:
  typedef const Permutation& FrontResult;

  MultiPermuter();
  MultiPermuter(size_t arraySize, size_t minSelect, size_t maxSelect);

  bool Empty() const;
  FrontResult Front() const;
  void PopFront();
  MultiPermuter& All()
  {
    return *this;
  }
  const MultiPermuter& All() const
  {
    return *this;
  }

  // Internals
  Permuter mPermuter;
  size_t mMaxSelect;
  bool mEmpty;
};

} // namespace Zero
