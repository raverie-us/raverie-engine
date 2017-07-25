///////////////////////////////////////////////////////////////////////////////
///
/// \file CyclicArray.hpp
/// CyclicArray container.
///
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Containers/Array.hpp"

namespace Zero
{

//------------------------------------------------------------------------ Array
/// Stores objects similar to a stack (always push to the front), but unlike a
/// stack, it has a set size.  Once the size is reached, the last element
/// is automatically popped off.
/// Also referred to as "circular buffer" or "ring buffer."
template<typename type>
class CyclicArray
{
public:
  //---------------------------------------------------------- Standard Typedefs
  typedef size_t size_type;

  typedef type value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  typedef CyclicArray<type> this_type;
  typedef Array<type> ArrayType;

  //--------------------------------------------------------------- Constructors
  /// Default constructor.
  CyclicArray() : mSize(0), mFront(0)
  {

  }

  explicit CyclicArray(size_type maxSize) : mArray(maxSize), mSize(0)
  {
    mFront = 0;
  }

  /// Copy constructor.
  CyclicArray(const this_type& other)
    : mArray(other.mArray),
      mFront(other.mFront),
      mSize(other.mSize)
  {

  }

  //--------------------------------------------------- Element Access Functions
  /// Indexing operator.
  reference operator[](size_type index)
  {
    ErrorIf(index >= GetMaxSize(), "Accessed CyclicArray out of bounds.");

    // Return the data at the local index
    size_type localIndex = GetLocalIndex(index);
    return mArray[localIndex];
  }

  /// Const indexing operator.
  const_reference operator[](size_type index) const
  {
    ErrorIf(index >= GetMaxSize(), "Accessed CyclicArray out of bounds.");

    // Return the data at the local index
    size_type localIndex = GetLocalIndex(index);
    return mArray[localIndex];
  }

  /// Returns a reference to the item at the front of the array.
  reference Front()
  {
    ErrorIf(Size() == 0, "Empty array, no front element.");
    return mArray[mFront];
  }

  /// Returns a reference to the item at the front of the array.
  const_reference Front() const
  {
    ErrorIf(Size() == 0, "Empty array, no front element.");
    return mArray[mFront];
  }

  /// Returns a reference to the item at the back of the array.
  reference Back()
  {
    ErrorIf(Size() == 0, "Empty array, no back element.");
    return mArray[GetBackIndex()];
  }

  /// Returns a reference to the item at the back of the array.
  const_reference Back() const
  {
    ErrorIf(Size() == 0, "Empty array, no back element.");
    return mArray[GetBackIndex()];
  }

  //----------------------------------------------------- Modification Functions
  /// Pushes the given item onto the front of the cyclic array.
  void Push(const type& item)
  {
    // Push the front pointer forward
    mFront = PreviousIndex(mFront);

    // Set the item
    mArray[mFront] = item;

    if(mSize < GetMaxSize())
      ++mSize;
  }

  /// Sets the maximum amount of items that can be pushed into the array.
  void SetMaxSize(size_type size)
  {
    // Make a temporary copy of ourselves
    CyclicArray<value_type> tempCopy(*this);

    // Set up ourselves to start pushing again
    mArray.SetCapacity(size);
    mArray.Resize(size);
    mSize = 0;
    mFront = 0;
    
    // No need to do anything if it was already empty
    if(tempCopy.Size() == 0)
      return;

    // If we resized to be smaller, we want to chop of the difference
    int start = (int)(tempCopy.Size()) - 1;
    if(size < tempCopy.Size())
      start = size - 1;

    // We want to push things on backwards
    for(int i = start; i >= 0; --i)
      Push(tempCopy[i]);
  }

  /// Clears all elements in the array.
  void Clear()
  {
    mSize = 0;
    mFront = 0;
  }

  /// Removes the first element in the array.
  void PopFront()
  {
    ErrorIf(Size() == 0, "Empty array, no front element.");
    mFront = NextIndex(mFront);
    --mSize;
  }

  /// Removes the last element in the array.
  void PopBack()
  {
    ErrorIf(Size() == 0, "Empty array, no back element.");
    --mSize;
  }
  
  //------------------------------------------------------ Information Functions
  /// The amount of items pushed into the array.
  size_type Size() const
  {
    return mSize;
  }

  /// The amount of items that can be pushed into the array.
  size_type GetMaxSize() const
  {
    return mArray.Size();
  }

  /// Returns whether the array Contains any elements.
  bool Empty() const
  {
    return Size() == 0;
  }

  /// Returns whether the array Contains the given element
  bool Contains(const type& item) const
  {
    // If the item is contained, return true
    range r = All();
    for(;!r.Empty();r.PopFront())
    {
      if(r.Front() == item)
        return true;
    }
    return false;
  }

  //------------------------------------------------------------------ Iterators
  /// Used for simple iteration over the cyclic array.
  struct range
  {
    /// Constructors.
    range() : mArray(nullptr), mBegin(0), mEnd(0) {}
    range(CyclicArray* array, uint begin, uint end) : mArray(array), 
                                                     mBegin(begin), mEnd(end) {}

    /// Iteration functionality.
    reference Front() { return (*mArray)[mBegin]; }
    void PopFront() { ++mBegin; }
    bool Empty() const { return mBegin == mEnd; }
    size_type Length() const {return mEnd - mBegin; }
    range& All() { return *this; }
    const range& All() const { return *this; }

    // We need access to the array (cannot use pointers as the array wraps).
    CyclicArray* mArray;

    // These are NOT local indices into the array (therefore, 
    // do not need to be wrapped).
    uint mBegin, mEnd;
  };

  /// Returns a range for all the elements in the container.
  range All() { return range(this, 0, Size()); }

  /// Returns a range for all the elements in the container.
  range All() const { return const_cast<CyclicArray*>(this)->All(); }

  /// Returns a range for the specified range of elements in the container.
  range SubRange(size_type index, size_type length)
  {
    ErrorIf(index >= Size(), "Accessed array out of bounds.");
    ErrorIf(index + length > Size(), "Accessed array out of bounds.");
    return range(this, index, index + length);
  }

private:
  size_type GetLocalIndex(size_type index) const
  {
    return (mFront + index) % GetMaxSize();
  }

  /// Returns the index at the back of the buffer.
  size_type GetBackIndex() const
  {
    return (mFront + mSize - 1) % GetMaxSize();
  }

  /// Increments the given index and wraps it if needed.
  size_type NextIndex(size_type index) const
  {
    return (index + 1) % GetMaxSize();
  }

  /// Decrements the given index and wraps it if needed.
  size_type PreviousIndex(size_type index) const
  {
    if(index == 0)
      return GetMaxSize() - 1;
    return index - 1;
  }

  /// Always points to the first element in the array.
  size_type mFront;

  /// The amount of items added into the array.
  size_type mSize;

  /// The underlying data array.
  ArrayType mArray;
};

}//namespace Zero
