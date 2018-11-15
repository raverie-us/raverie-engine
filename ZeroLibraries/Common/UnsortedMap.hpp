///////////////////////////////////////////////////////////////////////////////
///
/// \file Array.hpp
/// Declaration of the Array container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ContainerCommon.hpp"
#include "Allocator.hpp"
#include "Array.hpp"
#include "Hashing.hpp"

namespace Zero
{

/// Array map is an associative container (like a hash-map) however the key type is neither sorted nor hashed
/// and therefore the search time for an element is linear. The memory footprint, however, is much smaller than
/// that of a hash-map. Furthermore, because the key is neither sorted nor hashed, the key can be changed without
/// having to remove and re-Insert an element (and can be changed while iterating through this container).
template< typename KeyType, typename DataType, 
          typename Comparer = ComparePolicy<KeyType>, 
          typename Allocator = DefaultAllocator >
class UnsortedMap :  public Array<Pair<KeyType, DataType>, Allocator>
{
public:
  typedef Array<Pair<KeyType, DataType>, Allocator> base_type;
  typedef typename base_type::range range;
  typedef typename base_type::value_type value_type;

  // Index into the map by key
  DataType& operator[](const KeyType& key)
  {
    // Attempt to find an element using the given key
    range found = Find(key);

    // If we don't have the element...
    if (found.Empty())
    {
      // Add a new element
      value_type& newElement = base_type::PushBack();

      // Set the first to be the key
      newElement.first = key;

      // Return a reference to the pair value type
      return newElement.second;
    }
    else
    {
      // Otherwise, return a reference to the pair value type
      return found.Front().second;
    }
  }

  // Find an element by a key type that is different than the template key type
  template<typename SearchType, typename comparePolicy>
  range FindAs(const SearchType& key, comparePolicy comparer = ComparePolicy<SearchType>())
  {
    // Loop through all the elements
    for (size_t i = 0; i < base_type::Size(); ++i)
    {
      // Get the current element
      const value_type& current = *(this->Data() + i);

      // If the comparer matches...
      if (comparer.Equal(current.first, key))
      {
        // Return a range that points at the found element
        return range(this->Data() + i, this->Data() + i + 1);
      }
    }

    // Return an empty range
    return range();
  }

  // Find an element by key
  range Find(const KeyType& key)
  {
    // Create the comparer functor
    Comparer comparer;

    // Call find as and return its result
    return FindAs(key, comparer);
  }

  // Does the map contain a particular key?
  bool ContainsKey(const KeyType& key)
  {
    return !Find(key).Empty();
  }

  void Erase(const KeyType& key)
  {
    base_type::Erase(Find(key));
  }
};

}//namespace Zero


