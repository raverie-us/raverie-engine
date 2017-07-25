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

/// A hashmap that preserving insertion order, typically for displaying data in a consistent order such as in a property grid.
/// Currently implemented as a hashmap of linked list nodes. Can be optimized later, meant as a quick and easy solution for now.
template <typename KeyType, 
          typename ValueType,
          typename SubSorter = less<KeyType>>
class OrderedHashMap
{
public:
  typedef Pair<KeyType, ValueType> PairType;

  struct Node
  {
    Node() {}
    Node(const KeyType& key, const ValueType& value) :
      mPair(key, value)
    {
    }

    PairType mPair;
    Link<Node> link;
  };

  template <typename SubSorter>
  struct NodeSortPolicy
  {
    bool operator()(const Node& rhs, const Node& lhs)
    {
      return mSubSorter(rhs.mPair.first, lhs.mPair.first);
    }

    SubSorter mSubSorter;
  };

  typedef NodeSortPolicy<SubSorter> Sorter;
  typedef HashMap<KeyType, Node*> MapType;
  typedef InList<Node> ListType;
  typedef typename ListType::range ListTypeRange;

  OrderedHashMap() : mSorter()
  {
  }

  ~OrderedHashMap()
  {
    Clear();
  }

  void Clear()
  {
    while(!mList.Empty())
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

  void InsertInternal(const KeyType& key, const ValueType& value)
  {
    Node* node = new Node(key, value);
    mMap.Insert(key, node);
    mList.PushBack(node);
  }

  void InsertOrError(const KeyType& key, const ValueType& value)
  {
    ReturnIf(mMap.ContainsKey(key), , "Key already exists");

    InsertInternal(key, value);
  }

  void InsertOrIgnore(const KeyType& key, const ValueType& value)
  {
    if(mMap.ContainsKey(key))
      return;

    InsertInternal(key, value);
  }

  void InsertOrOverride(const KeyType& key, const ValueType& value)
  {
    Node* node = mMap.FindValue(key, nullptr);
    // If the key already exist then just override the value, don't update the order in the list
    if(node != nullptr)
    {
      node->mPair.second = value;
      return;
    }

    InsertInternal(key, value);
  }

  void SortedInsertInternal(const KeyType& key, const ValueType& value)
  {
    Node* node = new Node(key, value);
    mMap.Insert(key, node);
    mList.SortedInsert(node, mSorter);
  }

  void SortedInsertOrError(const KeyType& key, const ValueType& value)
  {
    ReturnIf(mMap.ContainsKey(key), , "Key already exists");

    SortedInsertInternal(key, value);
  }

  void SortedInsertOrIgnore(const KeyType& key, const ValueType& value)
  {
    if (mMap.ContainsKey(key))
      return;

    SortedInsertInternal(key, value);
  }

  void SortedInsertOrOverride(const KeyType& key, const ValueType& value)
  {
    Node* node = mMap.FindValue(key, nullptr);
    // If the key already exist then just override the value, don't update the order in the list
    if (node != nullptr)
    {
      node->mPair.second = value;
      return;
    }

    SortedInsertInternal(key, value);
  }

  bool Erase(const KeyType& key)
  {
    Node* node = mMap.FindValue(key, nullptr);
    if(node != nullptr)
    {
      mMap.Erase(key);
      mList.Erase(node);
      delete node;
      return true;
    }
    return false;
  }

  bool ContainsKey(const KeyType& key)
  {
    return mMap.ContainsKey(key);
  }

  ValueType FindValue(const KeyType& key, const ValueType& defaultValue)
  {
    Node* node = mMap.FindValue(key, nullptr);
    if(node == nullptr)
      return defaultValue;
    return node->mPair.second;
  }

  ValueType* FindPointer(const KeyType& key)
  {
    Node* node = mMap.FindValue(key, nullptr);
    if(node == nullptr)
      return nullptr;
    return &(node->mPair.second);
  }

  struct Range
  {
    typedef PairType& FrontResult;
    bool Empty() { return mRange.Empty(); }
    PairType& Front() { return mRange.Front().mPair; }
    void PopFront() { mRange.PopFront(); }
    Range& All() { return *this; }
    ListTypeRange mRange;
  };

  struct KeyRange
  {
    typedef KeyType& FrontResult;
    bool Empty() { return mRange.Empty(); }
    KeyType& Front() { return mRange.Front().mPair.first; }
    void PopFront() { mRange.PopFront(); }
    KeyRange& All() { return *this; }
    ListTypeRange mRange;
  };

  struct ValueRange
  {
    typedef ValueType& FrontResult;
    bool Empty() { return mRange.Empty(); }
    ValueType& Front() { return mRange.Front().mPair.second; }
    void PopFront() { mRange.PopFront(); }
    ValueRange& All() { return *this; }
    ListTypeRange mRange;
  };

  Range All()
  {
    Range range;
    range.mRange = mList.All();
    return range;
  }

  KeyRange Keys()
  {
    KeyRange range;
    range.mRange = mList.All();
    return range;
  }

  ValueRange Values()
  {
    ValueRange range;
    range.mRange = mList.All();
    return range;
  }

  ValueType& operator[](const KeyType& key)
  {
    Node*& node = mMap[key];
    if (node == nullptr)
      InsertInternal(key, ValueType());
    return node->mPair.second;
  }

  MapType mMap;
  ListType mList;

  protected:
    /// Sort policy functor
    Sorter mSorter;
};

}//namespace Zero

