///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "SortedArray.hpp"

namespace Zero
{

/// Array Set is an Associative Sequenced Container
/// Contains a sorted set of unique objects in a contiguous dynamic array
/// Search, insertion, and removal operations have O(log n) complexity
/// Functionality is similar to std::set but with better performance
template<typename ValueType,
         typename Sorter    = SortPolicy<ValueType>,
         typename Allocator = DefaultAllocator>
class ArraySet : public SortedArray<ValueType, Sorter, Allocator>
{
public:
  /// Typedefs
  typedef ArraySet<ValueType, Sorter, Allocator>    this_type;
  typedef SortedArray<ValueType, Sorter, Allocator> sorted_array_type;
  typedef sorted_array_type                         base_type;

  typedef typename base_type::reference reference;
  typedef typename base_type::const_reference const_reference;
  typedef typename base_type::value_type value_type;
  typedef typename base_type::pointer_bool_pair pointer_bool_pair;

protected:
  /// Base methods hidden to provide container behavior
  using base_type::FindAll;
  using base_type::Insert;
  using base_type::Assign;
  using base_type::EraseAll;

public:
  //
  // Member Functions
  //

  /// Default Constructor
  ArraySet()
    : base_type()
  {
  }

  /// Copy Constructor
  ArraySet(const this_type& rhs)
    : base_type(rhs)
  {
  }

  /// Move Constructor
  ArraySet(MoveReference<this_type> rhs)
    : base_type(ZeroMove(static_cast<base_type&>(*rhs)))
  {
  }

  /// Copy Assignment Operator
  ArraySet& operator =(const this_type& rhs)
  {
    base_type::operator=(rhs);
    return *this;
  }

  /// Move Assignment Operator
  ArraySet& operator =(MoveReference<this_type> rhs)
  {
    base_type::operator=(ZeroMove(static_cast<base_type&>(*rhs)));
    return *this;
  }

  //
  // Insertion Operations
  //

  /// Inserts a new element if an equivalent element does not already exist
  /// Returns the value of the equivalent element in the array
  reference FindOrInsert(const_reference value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, value)) // Found?
      return *position;
    else
    {
      // Insert unique element
      size_type index = position - mData;
      base_type::Insert(position, value);
      return mData[index];
    }
  }
  reference FindOrInsert(MoveReference<value_type> value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), *value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, *value)) // Found?
      return *position;
    else
    {
      // Insert unique element
      size_type index = position - mData;
      base_type::Insert(position, *value);
      return mData[index];
    }
  }

  /// Inserts a unique element at it's sorted position in the array
  /// Returns a pair consisting of a pointer to the inserted element (or the equivalent element which prevented it's insertion),
  /// and a bool which will be true if the insertion took place, else false if no changes were made
  pointer_bool_pair Insert(const_reference value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, value)) // Found?
      return pointer_bool_pair(position, false);
    else
    {
      // Insert unique element
      size_type index = position - mData;
      base_type::Insert(position, value);
      return pointer_bool_pair(mData + index, true);
    }
  }
  pointer_bool_pair Insert(MoveReference<value_type> value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), *value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, *value)) // Found?
      return pointer_bool_pair(position, false);
    else
    {
      // Insert unique element
      size_type index = position - mData;
      base_type::Insert(position, ZeroMove(value));
      return pointer_bool_pair(mData + index, true);
    }
  }

  /// Inserts a unique element at it's sorted position in the array or assigns over a previously inserted equivalent element
  /// Returns a pair consisting of a pointer to the inserted element (or the equivalent element which was assigned to),
  /// and a bool which will be true if the insertion took place, else false if the assignment took place
  pointer_bool_pair InsertOrAssign(const_reference value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, value)) // Found?
    {
      // Assign over equivalent element
      *position = value;
      return pointer_bool_pair(position, false);
    }
    else
    {
      // Insert unique element
      size_type index = position - mData;
      base_type::Insert(position, value);
      return pointer_bool_pair(mData + index, true);
    }
  }
  pointer_bool_pair InsertOrAssign(MoveReference<value_type> value)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), *value, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, *value)) // Found?
    {
      // Assign over equivalent element
      *position = ZeroMove(value);
      return pointer_bool_pair(position, false);
    }
    else
    {
      // Insert unique element
      size_type index = position - mData;
      base_type::Insert(position, ZeroMove(value));
      return pointer_bool_pair(mData + index, true);
    }
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
};

} // namespace Zero
