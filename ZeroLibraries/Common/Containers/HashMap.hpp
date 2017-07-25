///////////////////////////////////////////////////////////////////////////////
///
/// \file HashMap.hpp
/// Definition of the HashMap associative container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "TypeTraits.hpp"
#include "ContainerCommon.hpp"
#include "Hashing.hpp"
#include "HashedContainer.hpp"
#include "Allocator.hpp"

namespace Zero
{

template<typename Hasher, typename KeyType, typename DataType>
struct ZeroSharedTemplate PairHashAdapter
{
  Hasher mHasher;
  typedef Pair<KeyType, DataType> pair_type;

  size_t operator()(const KeyType& value)
  {
    return mHasher(value);
  }

  size_t operator()(const pair_type& value)
  {
    return mHasher(value.first);
  }

  bool Equal(const pair_type& left, const pair_type& right)
  {
    return mHasher.Equal(left.first, right.first);
  }

  template<typename otherPairType>
  bool Equal(const KeyType& left, const otherPairType& rightPair)
  {
    return mHasher.Equal(left, rightPair.first);
  }
};

///Hash Map is an Associative Hashed Container. 
//Stores values by hashing keys providing constant insertion, removal, and 
//searching. Iteration is not in done is sort order.
template< typename KeyType, typename DataType, 
          typename Hasher = HashPolicy<KeyType>, 
          typename Allocator = DefaultAllocator >
class ZeroSharedTemplate HashMap :  public HashedContainer< Pair<KeyType, DataType>, 
                                                            PairHashAdapter< Hasher, KeyType, DataType >, 
                                                            Allocator >
{
public:
  typedef KeyType key_type;
  typedef DataType data_type;
  typedef HashMap<KeyType, DataType> this_type;
  typedef Pair<KeyType, DataType> value_type;
  typedef Pair<KeyType, DataType> pair;
  typedef size_t size_type;
  typedef data_type& reference;
  typedef HashedContainer< value_type, 
                           PairHashAdapter<Hasher, KeyType, DataType>, 
                           Allocator > base_type;
  typedef typename base_type::Node* iterator;
  typedef typename base_type::Node Node;
  typedef typename base_type::range range;

  typedef typename base_type::InsertResult InsertResult;

  HashMap()
  {
  }

  ~HashMap()
  {
  }

  struct valuerange
  {
    typedef data_type value_type;
    typedef reference FrontResult;

    range r;
    valuerange() {}
    valuerange(const typename base_type::range& _r)
      : r(_r){}
    bool Empty() { return r.Empty(); }
    void PopFront() { return r.PopFront(); }
    size_type Size() { return r.Size(); }
    size_type Length() { return r.Size(); }
    reference Front() { return r.Front().second; }
    valuerange& All() { return *this; }
    const valuerange& All() const { return *this; }
  };

  struct keyrange
  {
    typedef key_type value_type;
    typedef value_type& FrontResult;
    range r;
    keyrange() {}
    keyrange(const typename base_type::range& _r)
      : r(_r) {}
    bool Empty() { return r.Empty(); }
    void PopFront() { return r.PopFront(); }
    size_type Size() { return r.Size(); }
    size_type Length() { return r.Size(); }
    value_type& Front() { return r.Front().first; }
    keyrange& All() { return *this; }
    const keyrange& All() const { return *this; }
  };

  // range of all the values in the map.
  valuerange Values() const { return valuerange(base_type::All()); }

  // range of all the keys in the map.
  keyrange Keys() const { return keyrange(base_type::All()); }

  data_type& operator[](const key_type& key)
  {
    Node* node = base_type::InternalFindAs(key, base_type::mHasher);
    if(node != cHashOpenNode)
    {
      return node->Value.second;
    }
    else
    {
      value_type newType(key, data_type());
      return base_type::InsertInternal(newType, base_type::OnCollisionOverride).mValue->second;
    }
  }

  InsertResult Insert(const value_type& datapair)
  {
    return base_type::InsertInternal(datapair, base_type::OnCollisionOverride);
  }

  InsertResult Insert(const key_type& key, const data_type& value)
  {
    return base_type::InsertInternal(value_type(key, value), base_type::OnCollisionOverride);
  }

  void Insert(range pair_range)
  {
    for (; !pair_range.Empty(); pair_range.PopFront())
    {
      base_type::InsertInternal(pair_range.Front(), base_type::OnCollisionOverride);
    }
  }

  bool InsertOrError(const value_type& datapair)
  {
    return base_type::InsertInternal(datapair, base_type::OnCollisionError) != false;
  }

  bool InsertOrError(const key_type& key, const data_type& value)
  {
    return base_type::InsertInternal(value_type(key, value), base_type::OnCollisionError) != false;
  }

  template <typename VType>
  bool InsertOrError(const VType& value, cstr error)
  {
    (void)error;
    bool result = InsertOrError(value);
    ErrorIf(result == false, "%s", error);
    return result;
  }

  template <typename KType, typename VType>
  bool InsertOrError(const KType& key, const VType& value, cstr error)
  {
    return InsertOrError(value_type(key, value), error);
  }

  InsertResult InsertNoOverwrite(const value_type& datapair)
  {
    return base_type::InsertInternal(datapair, base_type::OnCollisionReturn);
  }

  InsertResult InsertNoOverwrite(const key_type& key, const data_type& value)
  {
    return base_type::InsertInternal(value_type(key, value), base_type::OnCollisionReturn);
  }

  template<typename searchType, typename searchHasher>
  range FindAs(const searchType& searchKey,
                searchHasher keyHasher = HashPolicy<searchType>())
  {
    Node* node = base_type::InternalFindAs(searchKey, 
                                             PairHashAdapter< searchHasher,
                                                              searchType,
                                                              DataType >());
    if(node != cHashOpenNode)
      return range(node, node + 1, 1);
    else
      return range((Node*)cHashOpenNode, (Node*)cHashOpenNode, 0);
  }

  range Find(const key_type& searchKey) const
  {
    Node* node = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(node != cHashOpenNode)
      return range(node, node + 1, 1);
    else
      return range((Node*)cHashOpenNode, (Node*)cHashOpenNode, 0);
  }

  bool TryGetValue(const key_type& searchKey, data_type& valueOut)
  {
    Node* node = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(node != cHashOpenNode)
    {
      valueOut = node->Value.second;
      return true;
    }
    else
      return false;
  }

  bool Erase(const key_type& searchKey)
  {
    Node* node = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(node != (Node*)cHashOpenNode)
    {
      base_type::EraseNode(node);
      return true;
    }
    return false;
  }

  size_t Count(const key_type& searchKey)
  {
    Node* foundNode = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(foundNode != cHashOpenNode)
      return 1;
    else
      return 0;
  }

  data_type FindValue(const key_type& searchKey, const data_type& ifNotFound) const
  {
    Node* foundNode = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(foundNode != cHashOpenNode)
      return foundNode->Value.second;
    else
      return ifNotFound;
  }

  //Returns a pointer to the value if found, or null if not found
  data_type* FindPointer(const key_type& searchKey, data_type* ifNotFound = nullptr) const
  {
    Node* foundNode = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(foundNode != cHashOpenNode)
      return &(foundNode->Value.second);
    else
      return ifNotFound;
  }

  bool ContainsKey(const key_type& searchKey) const
  {
    Node* foundNode = base_type::InternalFindAs(searchKey, base_type::mHasher);
    return (foundNode != cHashOpenNode);
  }

  HashMap(const HashMap& other)
  {
    *this = other;
  }

  void operator = (const HashMap& other)
  {
    this->Clear();
    range r = other.All();
    while(!r.Empty())
    {
      Insert(r.Front());
      r.PopFront();
    }
  }

private:
};

}//namespace Zero
