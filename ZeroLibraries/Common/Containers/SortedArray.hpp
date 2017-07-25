///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "ContainerCommon.hpp"
#include "Algorithm.hpp"
#include "Array.hpp"

namespace Zero
{

/// Policy for how values are sorted
template<typename ValueType>
struct SortPolicy : public ComparePolicy<ValueType>
{
  /// Typedefs
  typedef ValueType value_type;

  /// Default behavior is to expect the value type to have a less-than operator callable with the compare type
  /// If a compilation error occurs here, define a compatible less-than operator or use another sort policy
  template<typename CompareType>
  bool operator()(const value_type& lhs, const CompareType& rhs) const
  {
    return lhs < rhs;
  }
};

/// Intended for use by containers that store associative pointer objects
template<typename ValueType>
struct PointerSortPolicy
{
  typedef ValueType value_type;

  /// Dereferences before comparing with another value
  template<typename CompareType>
  bool operator()(const value_type& lhs, const CompareType& rhs) const
  {
    return *lhs < rhs;
  }
  bool operator()(const value_type& lhs, const value_type& rhs) const
  {
    return *lhs < *rhs;
  }

  template<typename CompareType>
  bool Equal(const value_type& lhs, const CompareType& rhs) const
  {
    return *lhs == rhs;
  }
  bool Equal(const value_type& lhs, const value_type& rhs) const
  {
    return *lhs == *rhs;
  }
};

/// Sorted Array is an Associative Sequenced Container
/// Contains a sorted set of objects in a contiguous dynamic array
/// Insertion order of equivalent objects is preserved, with newer objects inserted at the lower bound
/// Search, insertion, and removal operations have O(log n) complexity
/// Functionality is similar to std::multiset but with better performance
template<typename ValueType,
         typename Sorter    = SortPolicy<ValueType>,
         typename Allocator = DefaultAllocator>
class SortedArray : public Array<ValueType, Allocator>
{
public:
  /// Typedefs
  typedef SortedArray<ValueType, Sorter, Allocator> this_type;
  typedef Array<ValueType, Allocator>               array_type;
  typedef array_type                                base_type;
  typedef typename base_type::pointer pointer;
  typedef typename base_type::iterator iterator;
  typedef typename base_type::size_type size_type;
  typedef typename base_type::const_reference const_reference;
  typedef typename base_type::const_iterator const_iterator;
  typedef typename base_type::value_type value_type;
  typedef typename base_type::range range;

  typedef Sorter                                    sorter_type;
  typedef Pair<pointer, bool>                       pointer_bool_pair;

protected:
  /// Base methods hidden to provide container behavior
  using base_type::PushBack;
  using base_type::Insert;
  using base_type::InsertAt;
  using base_type::Assign;
  using base_type::Append;
  //using base_type::erase_value;
  using base_type::FindIndex;
  //using base_type::find_iterator;
  // using base_type::find_pointer;
  //using base_type::Contains;

public:
  //
  // Member Functions
  //

  /// Default Constructor
  SortedArray()
    : base_type(),
      mSorter()
  {
  }

  /// Copy Constructor
  SortedArray(const this_type& rhs)
    : base_type(rhs),
      mSorter(rhs.mSorter)
  {
  }

  /// Move Constructor
  SortedArray(MoveReference<this_type> rhs)
    : base_type(ZeroMove(static_cast<base_type&>(*rhs))),
      mSorter(rhs->mSorter)
  {
  }

  /// Copy Assignment Operator
  SortedArray& operator =(const this_type& rhs)
  {
    base_type::operator=(rhs);
    mSorter = rhs.mSorter;
    return *this;
  }

  /// Move Assignment Operator
  SortedArray& operator =(MoveReference<this_type> rhs)
  {
    base_type::operator=(ZeroMove(static_cast<base_type&>(*rhs)));
    mSorter = rhs->mSorter;
    return *this;
  }

  //
  // Search Operations
  //

  /// Returns an iterator to the first equivalent element in the array, else End()
  template<typename CompareType>
  iterator FindIterator(const CompareType& value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, value)) // Found?
      return position;
    else
      return base_type::End();
  }
  template<typename CompareType>
  const_iterator FindIterator(const CompareType& value) const
  {
    return const_cast<this_type*>(this)->FindIterator(value);
  }

  /// Returns the index of the first equivalent element in the array, else InvalidIndex
  template<typename CompareType>
  size_type FindIndex(const CompareType& value) const
  {
    // Get lower bound
    iterator  position = LowerBound(base_type::All(), value, mSorter).Begin();
    size_type index    = position - mData;
    if(position != base_type::End()
    && mSorter.Equal(*position, value)) // Found?
      return index;
    else
      return InvalidIndex;
  }

  /// Returns a pointer to the first equivalent element in the array, else nullptr
  template<typename CompareType>
  pointer FindPointer(const CompareType& value) const
  {
    // Find first instance
    size_type index = FindIndex(value);
    if(index == InvalidIndex) // Unable?
      return nullptr;
    else
      return mData + index;
  }

  /// Returns the value of the first equivalent element in the array, else valueIfNotFound
  template<typename CompareType>
  const_reference FindValue(const CompareType& value, const_reference valueIfNotFound) const
  {
    // Find first instance
    size_type index = FindIndex(value);
    if(index == InvalidIndex) // Unable?
      return valueIfNotFound;
    else
      return mData[index];
  }

  /// Returns the range of all equivalent elements in the array
  template<typename CompareType>
  range FindAll(const CompareType& value) const
  {
    return LowerAndUpperBound(base_type::All(), value, mSorter);
  }

  /// Returns the number of all equivalent elements in the array
  template<typename CompareType>
  size_type Count(const CompareType& value) const
  {
    return FindAll().Length();
  }

  /// Returns true if the array Contains an equivalent element, else false
  template<typename CompareType>
  bool Contains(const CompareType& value) const
  {
    return FindIndex(value) != InvalidIndex;
  }
  bool Contains(const value_type& value) const
  {
    return Contains<value_type>(value);
  }

  //
  // Insertion Operations
  //

  /// Inserts an element at it's sorted position in the array
  /// Returns a pointer to the inserted element
  pointer Insert(const_reference value)
  {
    // Get lower bound
    iterator  position = LowerBound(base_type::All(), value, mSorter).Begin();
    size_type index    = position - mData;
    base_type::Insert(position, value);
    return mData + index;
  }
  pointer Insert(MoveReference<value_type> value)
  {
    // Get lower bound
    iterator  position = LowerBound(base_type::All(), *value, mSorter).Begin();
    size_type index    = position - mData;
    base_type::Insert(position, ZeroMove(value));
    return mData + index;
  }

  /// Inserts a range of elements at their sorted positions in the array
  template<typename iteratorType>
  void Insert(iteratorType begin, iteratorType end)
  {
    Insert(BuildRange(begin, end));
  }
  template<typename inputRangeType>
  void Insert(inputRangeType inputRange)
  {
    // Insert all elements in range
    for( ; inputRange.Empty(); inputRange.PopFront())
      Insert(inputRange.Front());
  }
  template<typename inputRangeType>
  void Insert(MoveReference<inputRangeType> inputRange)
  {
    // Insert all elements in range
    for( ; inputRange.Empty(); inputRange.PopFront())
      Insert(ZeroMove(inputRange.Front()));
  }

  /// Clears the array and inserts a range of elements at their sorted positions in the array
  template<typename iteratorType>
  void Assign(iteratorType begin, iteratorType end)
  {
    Clear();
    Insert(BuildRange(begin, end));
  }
  template<typename inputRangeType>
  void Assign(inputRangeType range)
  {
    Clear();
    Insert(range);
  }
  template<typename inputRangeType>
  void Assign(MoveReference<inputRangeType> range)
  {
    Clear();
    Insert(ZeroMove(range));
  }

  //
  // Removal Operations
  //

  /// Removes the first equivalent element from the array
  /// Returns a pair consisting of a pointer to the element following the erased element (or End() if the element could not be found),
  /// and a bool which will be true if the erase took place, else false
  template<typename CompareType>
  pointer_bool_pair EraseValue(const CompareType& value)
  {
    // Erase first instance
    size_type index = FindIndex(value);
    if(index == InvalidIndex) // Unable?
      return pointer_bool_pair(base_type::End(), false);
    else
      return pointer_bool_pair(base_type::Erase(mData + index), true);
  }
  pointer_bool_pair EraseValue(const value_type& value)
  {
    return EraseValue<value_type>(value);
  }

  /// Removes all equivalent elements from the array
  template<typename CompareType>
  void EraseAll(const CompareType& value)
  {
    // Erase all instances
    base_type::Erase(FindAll(value));
  }

protected:
  /// Sort policy functor
  Sorter mSorter;
};

} // namespace Zero
