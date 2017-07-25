///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "ArraySet.hpp"

namespace Zero
{

/// Policy for how key-data pairs are sorted
template<typename KeyType, typename DataType>
struct PairSortPolicy
{
  /// Typedefs
  typedef typename Pair<KeyType, DataType> value_type;
  typedef KeyType  key_type;
  typedef DataType data_type;

  /// Sorts according to key
  template<typename KeyCompareType>
  bool operator()(const value_type& leftPair, const KeyCompareType& searchKey) const
  {
    return leftPair.first < searchKey;
  }
  bool operator()(const value_type& leftPair, const value_type& searchPair) const
  {
    return leftPair.first < searchPair.first;
  }

  /// Equates according to key
  template<typename KeyCompareType>
  bool Equal(const value_type& leftPair, const KeyCompareType& searchKey) const
  {
    return leftPair.first == searchKey;
  }
  bool Equal(const value_type& leftPair, const value_type& searchPair) const
  {
    return leftPair.first == searchPair.first;
  }
};

/// Array Map is an Associative Sequenced Container
/// Contains a sorted set of unique key-data pairs in a contiguous dynamic array
/// Search, insertion, and removal operations have O(log n) complexity
/// Functionality is similar to std::map but with better performance
template<typename KeyType,
         typename DataType,
         typename Sorter    = PairSortPolicy<KeyType, DataType>,
         typename Allocator = DefaultAllocator>
class ArrayMap : public ArraySet<Pair<KeyType, DataType>, Sorter, Allocator>
{
public:
  /// Typedefs
  typedef ArrayMap<KeyType, DataType, Sorter, Allocator>       this_type;
  typedef ArraySet<Pair<KeyType, DataType>, Sorter, Allocator> array_set_type;
  typedef array_set_type                                       base_type;
  typedef KeyType                                              key_type;
  typedef key_type*                                            key_pointer;
  typedef const key_type*                                      const_key_pointer;
  typedef key_type&                                            key_reference;
  typedef const key_type&                                      const_key_reference;
  typedef DataType                                             data_type;
  typedef data_type*                                           data_pointer;
  typedef const data_type*                                     const_data_pointer;
  typedef data_type&                                           data_reference;
  typedef const data_type&                                     const_data_reference;
  typedef typename base_type::range range;
  typedef typename base_type::size_type size_type;
  typedef typename base_type::pointer_bool_pair pointer_bool_pair;
  typedef typename base_type::iterator iterator;

  /// Range adapter that presents only the key members in a key-data pair range, intended for convenience
  struct key_range : public range
  {
    typedef key_reference FrontResult;

    /// Member Functions
    key_range()                 : range()    {}
    key_range(const range& rhs) : range(rhs) {}

    /// Data Access
    key_reference       Front()                           { return range::Front().first;         }
    key_reference       Back()                            { return range::Back().first;          }
    key_reference       operator[](size_type index)       { return range::operator[index].first; }
    const_key_reference operator[](size_type index) const { return range::operator[index].first; }
  };

  /// Range adapter that presents only the data members in a key-data pair range, intended for convenience
  struct data_range : public range
  {
    typedef data_reference FrontResult;

    /// Member Functions
    data_range()                 : range()    {}
    data_range(const range& rhs) : range(rhs) {}

    /// Data Access
    data_reference       Front()                           { return range::Front().second;         }
    data_reference       Back()                            { return range::Back().second;          }
    data_reference       operator[](size_type index)       { return range::operator[index].second; }
    const_data_reference operator[](size_type index) const { return range::operator[index].second; }
  };

  //
  // Member Functions
  //

  /// Default Constructor
  ArrayMap()
    : base_type()
  {
  }

  /// Copy Constructor
  ArrayMap(const this_type& rhs)
    : base_type(rhs)
  {
  }

  /// Move Constructor
  ArrayMap(MoveReference<this_type> rhs)
    : base_type(ZeroMove(static_cast<base_type&>(*rhs)))
  {
  }

  /// Copy Assignment Operator
  ArrayMap& operator =(const this_type& rhs)
  {
    base_type::operator=(rhs);
    return *this;
  }

  /// Move Assignment Operator
  ArrayMap& operator =(MoveReference<this_type> rhs)
  {
    base_type::operator=(ZeroMove(static_cast<base_type&>(*rhs)));
    return *this;
  }

  //
  // Data Access
  //

  /// Returns a range of all key members in the array
  key_range Keys() const
  {
    return key_range(base_type::All());
  }

  /// Returns a range of all data members in the array
  data_range Values() const
  {
    return data_range(base_type::All());
  }

  //
  // Search Operations
  //

  /// Returns a pointer to the equivalent element in the array, else nullptr
  template<typename KeyCompareType>
  pointer FindPairPointer(const KeyCompareType& searchKey) const
  {
    return base_type::FindPointer(searchKey);
  }

  /// Returns the equivalent element in the array, else ifNotFound
  template<typename KeyCompareType>
  const_reference FindPairValue(const KeyCompareType& searchKey, const_reference ifNotFound) const
  {
    return base_type::FindValue(searchKey, ifNotFound);
  }

  /// Returns a pointer to the key of the equivalent element in the array, else nullptr
  template<typename KeyCompareType>
  key_pointer FindKeyPointer(const KeyCompareType& searchKey) const
  {
    // Find instance
    size_type index = FindIndex(searchKey);
    if(index == InvalidIndex) // Unable?
      return nullptr;
    else
      return &(mData[index].first);
  }

  /// Returns the key of the equivalent element in the array, else ifNotFound
  template<typename KeyCompareType>
  const_key_reference FindKeyValue(const KeyCompareType& searchKey, const_key_reference ifNotFound) const
  {
    // Find instance
    size_type index = FindIndex(searchKey);
    if(index == InvalidIndex) // Unable?
      return ifNotFound;
    else
      return mData[index].first;
  }

  /// Returns a pointer to the data of the equivalent element in the array, else nullptr
  template<typename KeyCompareType>
  data_pointer FindPointer(const KeyCompareType& searchKey) const
  {
    // Find instance
    size_type index = FindIndex(searchKey);
    if(index == InvalidIndex) // Unable?
      return nullptr;
    else
      return &(mData[index].second);
  }

  /// Returns the data of the equivalent element in the array, else ifNotFound
  template<typename KeyCompareType>
  const_data_reference FindValue(const KeyCompareType& searchKey, const_data_reference ifNotFound) const
  {
    // Find instance
    size_type index = FindIndex(searchKey);
    if(index == InvalidIndex) // Unable?
      return ifNotFound;
    else
      return mData[index].second;
  }

  //
  // Insertion Operations
  //

  /// Inserts a new element if an equivalent element does not already exist
  /// Returns a pair consisting of a pointer to the inserted element (or the equivalent element which prevented it's insertion),
  /// and a bool which will be true if the insertion took place, else false if no changes were made
  pointer_bool_pair FindOrInsert(const_key_reference key, const_data_reference data)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), key, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, key)) // Found?
      return pointer_bool_pair(position, false);
    else
    {
      // Insert unique element
      size_type index = position - mData;
      value_type newValue = value_type(key, data);
      base_type::Insert(position, ZeroMove(newValue));
      return pointer_bool_pair(mData + index, true);
    }
  }
  pointer_bool_pair FindOrInsert(const_key_reference key, MoveReference<data_type> data)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), key, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, key)) // Found?
      return pointer_bool_pair(position, false);
    else
    {
      // Insert unique element
      size_type index = position - mData;
      value_type newValue = value_type(key, ZeroMove(data));
      base_type::Insert(position, ZeroMove(newValue));
      return pointer_bool_pair(mData + index, true);
    }
  }
  /// Inserts a new element if an equivalent element does not already exist
  /// Returns the data of the equivalent element in the array
  data_reference FindOrInsert(const_key_reference key)
  {
    // Get lower bound
    iterator position = LowerBound(base_type::All(), key, mSorter).Begin();
    if(position != base_type::End()
    && mSorter.Equal(*position, key)) // Found?
      return position->second;
    else
    {
      // Insert unique element
      size_type index = position - mData;
      value_type newValue = value_type(key);
      base_type::Insert(position, ZeroMove(newValue));
      return mData[index].second;
    }
  }

  /// Inserts a unique element at it's sorted position in the array
  /// Returns a pair consisting of a pointer to the inserted element (or the equivalent element which prevented it's insertion),
  /// and a bool which will be true if the insertion took place, else false if no changes were made
  pointer_bool_pair Insert(const_key_reference key, const_data_reference data)
  {
    value_type value = value_type(key, data);
    return base_type::Insert(ZeroMove(value));
  }
  pointer_bool_pair Insert(const_key_reference key, MoveReference<data_type> data)
  {
    value_type value = value_type(key, ZeroMove(data));
    return base_type::Insert(ZeroMove(value));
  }

  /// Inserts a unique element at it's sorted position in the array or assigns over a previously inserted equivalent element
  /// Returns a pair consisting of a pointer to the inserted element (or the equivalent element which was assigned to),
  /// and a bool which will be true if the insertion took place, else false if the assignment took place
  pointer_bool_pair InsertOrAssign(const_key_reference key, const_data_reference data)
  {
    value_type value = value_type(key, data);
    return base_type::InsertOrAssign(ZeroMove(value));
  }
  pointer_bool_pair InsertOrAssign(const_key_reference key, MoveReference<data_type> data)
  {
    value_type value = value_type(key, ZeroMove(data));
    return base_type::InsertOrAssign(ZeroMove(value));
  }

  //
  // Removal Operations
  //

  /// Removes the first element with equivalent data from the array
  /// Note: This is a linear operation
  /// Returns true if an erase took place, else false if the element was not found
  template<typename DataCompareType>
  bool EraseData(const DataCompareType& searchData)
  {
    // Erase first instance
    for(size_type i = 0; i < Size(); ++i)
    {
      // Found?
      if(mData[i].second == searchData)
      {
        // Erase and return
        base_type::EraseAt(i);
        return true;
      }
    }

    // Not found
    return false;
  }

  /// Removes all elements with equivalent data from the array
  /// Note: This is a linear operation
  template<typename DataCompareType>
  void EraseAllData(const DataCompareType& searchData)
  {
    // Erase all instances
    for(size_type i = 0; i < Size(); )
    {
      // Found?
      if(mData[i].second == searchData)
        base_type::EraseAt(i); // Erase and advance
      else
        ++i; // Advance
    }
  }
};

} // namespace Zero
