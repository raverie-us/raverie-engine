///////////////////////////////////////////////////////////////////////////////
///
/// \file HashSet.hpp
/// Definition of the HashSet associative container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ContainerCommon.hpp"
#include "HashedContainer.hpp"

namespace Zero
{

template<typename Hasher, typename ValueType>
struct SetHashAdapter
{
  Hasher mHasher;
  size_t operator()(const ValueType& value)
  {
    return (size_t)mHasher(value);
  }

  bool Equal(const ValueType& left, const ValueType& right)
  {
    return mHasher.Equal(left, right);
  }

  template<typename othertype>
  bool Equal(const ValueType& left, const othertype& right)
  {
    return mHasher.Equal(left, right);
  }
};

///Hash Set is an Associative Hashed Container. 
template< typename ValueType, 
          typename Hasher = HashPolicy<ValueType>, 
          typename Allocator = DefaultAllocator >
class ZeroSharedTemplate HashSet : public HashedContainer< ValueType, 
                                                           SetHashAdapter< Hasher, ValueType >, 
                                                           Allocator >
{
public:
  typedef ValueType value_type;
  typedef size_t size_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef HashSet<ValueType, Hasher, Allocator> this_type;
  typedef HashedContainer< ValueType, 
                           SetHashAdapter< Hasher, ValueType >,
                           Allocator > base_type;
  typedef typename base_type::Node* iterator;
  typedef typename base_type::Node Node;
  typedef typename base_type::range range;

  HashSet()
  {
  }
  HashSet(ContainerInitializerDummy*, const_reference p0)
  {
    Insert(p0);
  }
  HashSet(ContainerInitializerDummy*, const_reference p0, const_reference p1)
  {
    Insert(p0);
    Insert(p1);
  }
  HashSet(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2)
  {
    Insert(p0);
    Insert(p1);
    Insert(p2);
  }
  HashSet(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2, const_reference p3)
  {
    Insert(p0);
    Insert(p1);
    Insert(p2);
    Insert(p3);
  }
  HashSet(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2, const_reference p3, const_reference p4)
  {
    Insert(p0);
    Insert(p1);
    Insert(p2);
    Insert(p3);
    Insert(p4);
  }
  HashSet(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2, const_reference p3, const_reference p4, const_reference p5)
  {
    Insert(p0);
    Insert(p1);
    Insert(p2);
    Insert(p3);
    Insert(p4);
    Insert(p5);
  }

  ///Warning: Depending on the contents of the hash sets, this may be expensive.
  HashSet(const HashSet& other)
  {
    *this = other;
  }

  ///Warning: Depending on the contents of the hash sets, this may be expensive.
  void operator = (const HashSet& other)
  {
    // Don't self Assign
    if(&other == this)
      return;

    this->Clear();
    range r = other.All();
    while(!r.Empty())
    {
      base_type::InsertInternal(r.Front(), base_type::OnCollisionOverride);
      r.PopFront();
    }
  }

  range Find(const value_type& value)
  {
    //searching for the actual type of the container.
    Node* node = base_type::InternalFindAs(value, base_type::mHasher);
    if(node != cHashOpenNode)
      return range(node, node + 1, 1);
    else
      return range((Node*)cHashOpenNode, (Node*)cHashOpenNode, 0);
  }

  template<typename searchType, typename searchHasher>
  range FindAs(const searchType& searchKey, 
                searchHasher keyHasher = HashPolicy<searchType>()) const
  {
    Node* node = base_type::InternalFindAs(searchKey, keyHasher);
    if(node != cHashOpenNode)
      return range(node, node + 1, 1);
    else
      return range((Node*)cHashOpenNode, (Node*)cHashOpenNode, 0);
  }

  value_type FindValue(const value_type& searchKey, const value_type& ifNotFound) const
  {
    Node* node = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(node != cHashOpenNode)
      return node->Value;
    else
      return ifNotFound;
  }

  //Returns a pointer to the value if found, or null if not found
  value_type* FindPointer(const value_type& searchKey) const
  {
    Node* foundNode = base_type::InternalFindAs(searchKey, base_type::mHasher);
    if(foundNode != cHashOpenNode)
      return &(foundNode->Value);
    else
      return nullptr;
  }

  template<typename inputRangeType>
  void Append(inputRangeType inputRange)
  {
    for(; !inputRange.Empty(); inputRange.PopFront())
      Insert(inputRange.Front());
  }

  void Insert(const value_type& value)
  {
    base_type::InsertInternal(value, base_type::OnCollisionOverride);
  }

  bool InsertOrError(const value_type& value)
  {
    return base_type::InsertInternal(value, base_type::OnCollisionError) != false;
  }

  bool InsertOrError(const value_type& value, cstr error)
  {
    bool result = InsertOrError(value);
    ErrorIf(result == false, "%s", error);
    return result;
  }

  bool InsertNoOverwrite(const value_type& value)
  {
    return base_type::InsertInternal(value, base_type::OnCollisionReturn) != false;
  }

  bool Contains(const value_type& value) const
  {
    return base_type::InternalFindAs(value, base_type::mHasher)!=cHashOpenNode;
  }

  ~HashSet()
  {
  }
};

}//namespace Zero
