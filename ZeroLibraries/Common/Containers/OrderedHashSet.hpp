///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "HashMap.hpp"
#include "InList.hpp"

namespace Zero
{

/// A hashset that preserving insertion order, typically for displaying data in a consistent order such as in a property grid.
/// Currently implemented as a hashset of linked list nodes. Can be optimized later, meant as a quick and easy solution for now.
template <typename T>
class OrderedHashSet
{
public:
  OrderedHashSet()
  {
  }
  OrderedHashSet(ContainerInitializerDummy*, const T& p0)
  {
    InsertOrOverride(p0);
  }
  OrderedHashSet(ContainerInitializerDummy*, const T& p0, const T& p1)
  {
    InsertOrOverride(p0);
    InsertOrOverride(p1);
  }
  OrderedHashSet(ContainerInitializerDummy*, const T& p0, const T& p1, const T& p2)
  {
    InsertOrOverride(p0);
    InsertOrOverride(p1);
    InsertOrOverride(p2);
  }
  OrderedHashSet(ContainerInitializerDummy*, const T& p0, const T& p1, const T& p2, const T& p3)
  {
    InsertOrOverride(p0);
    InsertOrOverride(p1);
    InsertOrOverride(p2);
    InsertOrOverride(p3);
  }
  OrderedHashSet(ContainerInitializerDummy*, const T& p0, const T& p1, const T& p2, const T& p3, const T& p4)
  {
    InsertOrOverride(p0);
    InsertOrOverride(p1);
    InsertOrOverride(p2);
    InsertOrOverride(p3);
    InsertOrOverride(p4);
  }
  OrderedHashSet(ContainerInitializerDummy*, const T& p0, const T& p1, const T& p2, const T& p3, const T& p4, const T& p5)
  {
    InsertOrOverride(p0);
    InsertOrOverride(p1);
    InsertOrOverride(p2);
    InsertOrOverride(p3);
    InsertOrOverride(p4);
    InsertOrOverride(p5);
  }

  struct Node
  {
    Node() {}
    Node(const T& value) :
      mValue(value)
    {
    }
    T mValue;
    Link<Node> link;
  };

  typedef HashMap<T, Node*> MapType;
  typedef InList<Node> ListType;
  typedef typename ListType::range ListTypeRange;

  ~OrderedHashSet()
  {
    Clear();
  }

  void Clear()
  {
    while (!mList.Empty())
    {
      Node* node = &mList.Front();
      mList.Erase(node);
      delete node;
    }
    mList.Clear();
    mMap.Clear();
  }

  size_t Size() const
  {
    return mMap.Size();
  }

  bool Empty() const
  {
    return mMap.Empty();
  }

  void InsertInternal(const T& value)
  {
    Node* node = new Node(value);
    mMap.Insert(value, node);
    mList.PushBack(node);
  }

  void InsertOrError(const T& value)
  {
    ReturnIf(mMap.ContainsKey(value), , "Value already exists");

    InsertInternal(value);
  }

  void InsertOrIgnore(const T& value)
  {
    if (mMap.ContainsKey(value))
      return;

    InsertInternal(value);
  }

  void InsertOrOverride(const T& value)
  {
    Node* node = mMap.FindValue(value, nullptr);
    // If the key already exist then just override the value, don't update the order in the list
    if (node != nullptr)
    {
      node->mValue = value;
      return;
    }

    InsertInternal(value);
  }

  bool Erase(const T& value)
  {
    Node* node = mMap.FindValue(value, nullptr);
    if (node != nullptr)
    {
      mMap.Erase(value);
      mList.Erase(node);
      delete node;
      return true;
    }
    return false;
  }

  bool ContainsKey(const T& value)
  {
    return mMap.ContainsKey(value);
  }

  T FindValue(const T& key, const T& defaultValue)
  {
    Node* node = mMap.FindValue(key, nullptr);
    if (node == nullptr)
      return defaultValue;
    return node->mValue;
  }

  T* FindPointer(const T& key)
  {
    Node* node = mMap.FindValue(key, nullptr);
    if (node == nullptr)
      return nullptr;
    return &node->mValue;
  }

  struct Range
  {
    typedef T& FrontResult;
    bool Empty() { return mRange.Empty(); }
    T& Front() { return mRange.Front().mValue; }
    void PopFront() { mRange.PopFront(); }
    Range& All() { return *this; }
    ListTypeRange mRange;
  };

  Range All()
  {
    Range range;
    range.mRange = mList.All();
    return range;
  }

  MapType mMap;
  ListType mList;
};

}//namespace Zero

