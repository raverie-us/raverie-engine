///////////////////////////////////////////////////////////////////////////////
///
/// \file HashedContainer.hpp
/// HahsedContainer Container used to implement of HashMap and HashSet.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Allocator.hpp"
#include "Hashing.hpp"

namespace Zero
{

void* const cHashOpenNode = nullptr;
void* const cHashEndNode = (void*)1;

template<typename ValueType, typename Hasher, typename Allocator>
class ZeroSharedTemplate HashedContainer : public AllocationContainer<Allocator>
{
public:
  //standard container typedefs
  typedef ValueType value_type;
  typedef size_t size_type;
  typedef ValueType& reference;
  typedef const ValueType& const_reference;
  typedef AllocationContainer<Allocator> base_type;
  typedef HashedContainer<ValueType, Hasher, Allocator> this_type;
  using base_type::mAllocator;

protected:
  //Internal node value
  struct Node
  {
    //The value stored in the node (only valid if 'next' is not set to 'cHashOpenNode');
    ValueType Value;

    //Can be set to another node in the chain, or to 'cHashOpenNode'
    //which means the node itself is open, or to 'cHashEndNode'
    //which means its at the end of the chain
    Node* next;
  };

public:
  //
  struct InsertResult
  {
    bool mIsNewInsert;
    ValueType* mValue;

    InsertResult(bool newInsert, Node* node) : mIsNewInsert(newInsert), mValue(&node->Value) {}

    operator bool() const { return mIsNewInsert; }

  };

  //Default constructor
  HashedContainer()
  {
    mTableSize = 0;
    mSize = 0;
    mTable = nullptr;
    mMaxLoadFactor = 0.8f;
  }

  ~HashedContainer()
  {
    Deallocate();
  }

  //Range for hash map.
  struct range
  {
    typedef typename this_type::value_type value_type;
    typedef reference FrontResult;

    range()
      : begin(nullptr), end(nullptr), mSize(0)
    {}

    range(Node* rbegin, Node* rend, size_t size)
      : begin(rbegin), end(rend)
    {
      mSize = size;
    }

    bool Empty()
    {
      return begin == end;
    }

    reference Front()
    {
      return begin->Value;
    }

    void PopFront()
    {
      ErrorIf(Empty(), "Popped an empty range.");
      ++begin;
      --mSize;

      begin = SkipDead(begin, end);
      //Skip empty slots
      while (begin != end && begin->next == cHashOpenNode)
        ++begin;
    }

    size_t Length() { return mSize; }

    size_type Size() { return Length(); }

  private:
    Node* begin;
    Node* end;
    size_t mSize;
  };

  //Get the hash value for a given value then
  //mod it by the table size to get a valid index.
  size_type HashedIndex(const_reference value)
  {
    return mHasher(value) % mTableSize;
  }

  ///////Container Global Modify//////////////////

  //Rehash the contents of the table.
  void Rehash(size_type newTableSize)
  {
    if (newTableSize < mSize)
      return;

    //Expand table to new size
    Node* oldTable = mTable;
    size_type oldTableSize = mTableSize;

    //Allocate the new table
    Node* newTable = (Node*)mAllocator.Allocate(newTableSize * sizeof(Node));

    //Set all buckets to the open node.
    for (size_type i = 0; i < newTableSize; ++i)
      newTable[i].next = (Node*)cHashOpenNode;

    //Set the internal member values
    mTable = newTable;
    mTableSize = newTableSize;
    mSize = 0;

    //Now reinsert all valid buckets values
    for (size_type i = 0; i < oldTableSize; ++i)
    {
      Node& node = oldTable[i];
      if (node.next != cHashOpenNode)
      {
        //If this errors here, it means most likely their 'equals' operator is wrong
        InsertInternal(node.Value, OnCollisionError);
      }
    }

    //Free the old table if it existed
    if (oldTableSize != 0)
    {
      DestructTableValues(oldTable, oldTableSize);
      mAllocator.Deallocate(oldTable, oldTableSize * sizeof(Node));
    }
  }

  //Destroy all elements.
  void Clear()
  {
    DestructTableValues(mTable, mTableSize);
    mSize = 0;
  }

  //Destroy all elements and frees all memory.
  void Deallocate()
  {
    if (mTable != nullptr)
    {
      //free all the values in the and then delete the table.
      DestructTableValues(mTable, mTableSize);
      mAllocator.Deallocate(mTable, mTableSize * sizeof(Node));
    }

    mTableSize = 0;
    mSize = 0;
    mTable = nullptr;
  }

  range All() const
  {
    Node* start = SkipDead(mTable, mTable + mTableSize);
    return range(start, mTable + mTableSize, mSize);
  }

  void Swap(this_type& other)
  {
    Zero::Swap(mTable, other.mTable);
    Zero::Swap(mTableSize, other.mTableSize);
    Zero::Swap(mSize, other.mSize);
    Zero::Swap(mMaxLoadFactor, other.mMaxLoadFactor);
    Zero::Swap(mHasher, other.mHasher);
  }

  ////////////Insertion///////////////////////

  //Override
  static Node* OnCollisionOverride(Node* dest, const_reference value)
  {
    dest->Value = value;
    return dest;
  }

  //Error
  static Node* OnCollisionError(Node* dest, const_reference value)
  {
    (void)value;
    (void)dest;
    Error("Double Insert, value was not inserted!");
    return nullptr;
  }

  //Just return the bucket
  static Node* OnCollisionReturn(Node* dest, const_reference value)
  {
    (void)value;
    return dest;
  }

  //Insert a value.
  template <typename CollisionFunc>
  InsertResult InsertInternal(const_reference value, CollisionFunc onCollison)
  {
    //Expand the table if insertion would break load factor
    //even if it might be a double Insert
    CheckForExpand(mSize + 1);


    //Find the node for this value this is its
    //primary bucket
    size_type curHash = HashedIndex(value);
    Node* node = mTable + curHash;

    //If the node is empty (see 'next')
    if (node->next != cHashOpenNode)
    {
      //If there is a collision check to see if it the
      //object is in its primary bucket.

      //Possible Collision or same key.
      if (mHasher.Equal(node->Value, value))
      {
        onCollison(node, value);
        return InsertResult(false, node);
      }
      else
      {
        //Hash Collision
        //If this hashed value is not in its primary bucket
        //steal this bucket (robin hood hashing)
        size_type actualHash = HashedIndex(node->Value);
        if (actualHash != curHash)
        {
          //Kick the node out of the bucket

          Node* movingNodePrimary = mTable + actualHash;

          ErrorIf(movingNodePrimary->next == cHashOpenNode,
                  "Bad hash function. Hash value has changed or other issue.");

          Node* movingNodePrev = movingNodePrimary;

          //Search through the moving nodes links
          //and find the previous node. This node
          //needs it next updated.
          while (movingNodePrev->next != node)
            movingNodePrev = movingNodePrev->next;

          //Remove the object from the chain.
          movingNodePrev->next = movingNodePrev->next->next;

          //Inert the old object into its bucket chain
          AppendToBucketChain(movingNodePrev, node->Value);

          //Replace in current node
          DestructNode(node);
          FillOpenNode(node, value);

          //Increase size
          ++mSize;
          return InsertResult(true, node);

        }
        else
        {
          //This bucket is the primary key for this value
          Node* primaryNode = node;

          //Different keys can be in the same bucket chain.
          //So the entire bucket chain must be checked
          //for double Insert.


          for (Node* searchNode = primaryNode; searchNode != cHashEndNode; searchNode = searchNode->next)
          {
            if (mHasher.Equal(searchNode->Value, value))
            {
              onCollison(searchNode, value);
              return InsertResult(false, searchNode);
            }

          }

          //Not in the list Insert into this chain
          ++mSize;
          return InsertResult(true, AppendToBucketChain(primaryNode, value));

        }
      }

    }
    else
    {
      //bucket is free use it
      //Construct the key
      //Copy data into it
      ++mSize;
      FillOpenNode(node, value);
      return InsertResult(true, node);
    }

  }


  ////////Find//////////////////////////////

  //Find an element value that hashes and compares to a
  //value in the hash map.
  template<typename searchType, typename searchHasherType>
  Node* InternalFindAs(const searchType& searchValue,
                         searchHasherType searchHasher) const
  {
    if (mTableSize == 0)
      return (Node*)cHashOpenNode;

    //Hash the value given with the provided hasher.
    size_type searchHash = searchHasher(searchValue) % mTableSize;
    Node* node = mTable + searchHash;

    //If the node's next is set to 'cHashOpenNode', it means that
    // the node itself is open/empty
    if (node->next != cHashOpenNode)
    {
      do
      {
        //Check to see if the value of this node is equal
        //to the search value.
        if (searchHasher.Equal(searchValue, node->Value))
          return node;

        //Move through all the objects in the linked list.
        node = node->next;
      } while (node != cHashEndNode);

    }
    return (Node*)cHashOpenNode;
  }

  size_t Count(const_reference value)
  {
    Node* foundNode = InternalFindAs(value, mHasher);
    if (foundNode != cHashOpenNode)
      return 1;
    else
      return 0;
  }


  ///////Erasing//////////////////////////


  //Erase a value if found.
  bool Erase(const_reference value)
  {
    Node* foundNode = InternalFindAs(value, mHasher);
    if(foundNode != cHashOpenNode)
    {
      EraseNode(foundNode);
      return true;
    }
    return false;
  }

  void EraseNode(Node* node)
  {
    ErrorIf(node == nullptr || node->next == cHashOpenNode,
            "Attempted to erase an invalid node.");
    size_type eraseHash = HashedIndex(node->Value);
    Node* bucketPrev = mTable + eraseHash;

    if (bucketPrev == node)
    {
      if (bucketPrev->next != (Node*)cHashEndNode)
      {
        //node is the first node in a bucket chain
        //remove the front by moving the next node
        //in the chain into this bucket
        MoveNode(bucketPrev, bucketPrev->next);
      }
      else
      {
        //Node chain just destroy it
        DestructNode(bucketPrev);
      }
      --mSize;
      return;
    }

    //Search for node's parent
    while (bucketPrev->next != node)
      bucketPrev = bucketPrev->next;

    //remove the node from the list
    bucketPrev->next = node->next;
    DestructNode(node);
    --mSize;
  }

  //////////Information Functions///////////
  size_type BucketCount() const { return mTableSize; }
  size_type Size() const { return mSize; }
  bool Empty()const { return mSize == 0; }

  //////////Load Factor///////////////////////
  float MaxLoadFactor()const { return mMaxLoadFactor; }
  float LoadFactor() const { return float(mSize) / float(mTableSize); }
  void SetMaxLoadFactor(float newMax)
  {
    mMaxLoadFactor = newMax;
    CheckForExpand(mSize);
  }


  ///Equals///////////

  bool operator==(const this_type& other)
  {
    if (other.Size() != this->Size())
      return false;

    range r = this->All();
    while (!r.Empty())
    {
      Node* node = other.InternalFindAs(r.Front(), mHasher);
      if (node == (Node*)cHashOpenNode)
        return false;

      if (r.Front() != node->Value)
        return false;

      r.PopFront();
    }

    return true;
  }

protected:

  static Node* SkipDead(Node* start, Node* end)
  {
    while (start != end && start->next == cHashOpenNode)
      ++start;
    return start;
  }

  Node* mTable;
  size_type mTableSize;
  size_type mSize;
  float mMaxLoadFactor;
  Hasher mHasher;
  typedef Node node_type;

  void CheckForExpand(size_type newsize)
  {
    if (mTableSize == 0 ||
      (float(newsize) / float(mTableSize)) > MaxLoadFactor())
    {
      size_type newTableSize = GetNextSize(mTableSize);
      if (newTableSize == 0)
        newTableSize = 16;
      Rehash(newTableSize);
    }
  }

  //assumes d is power of 2
  size_type GetNextSize(size_type d)
  {
    return d * 2;
  }

  Node* GetLastInBucketChain(Node* node)
  {
    while (node->next != cHashEndNode)
      node = node->next;
    return node;
  }

  inline Node* AppendToBucketChain(Node* root, const_reference value)
  {
    Node* lastInBucket = GetLastInBucketChain(root);
    Node* openSlot = GetNextOpenBucket(lastInBucket);

    FillOpenNode(openSlot, value);
    lastInBucket->next = openSlot;
    return openSlot;
  }

  static void DestructTableValues(Node* data, size_type size)
  {
    for (size_type i = 0; i < size; ++i)
    {
      //call the destructor on all the value types
      if (data[i].next != cHashOpenNode)
        data[i].Value.~ValueType();

      data[i].next = (Node*)cHashOpenNode;
    }
  }

  void MoveNode(Node* dest, Node* source)
  {
    DestructNode(dest);

    //Move
    new(&dest->Value) value_type(source->Value);
    dest->next = source->next;

    DestructNode(source);
  }

  void FillOpenNode(Node* node, const_reference value)
  {
    new(&node->Value) value_type(value);
    node->next = (Node*)cHashEndNode;
  }

  void DestructNode(Node* node)
  {
    //Call the destructor on the value
    node->Value.~ValueType();
    //Mark this bucket as open.
    node->next = (Node*)cHashOpenNode;
  }

  Node* GetNextOpenBucket(Node* startingNode)
  {
    Node* cur = startingNode;
    Node* end = mTable + mTableSize;

    //Find a 'nearby' bucket by searching around the current
    //bucket

    while (cur < end)
    {
      if (cur->next == cHashOpenNode)
        return cur;
      ++cur;
    }

    cur = startingNode;
    while (cur >= mTable)
    {
      if (cur->next == cHashOpenNode)
        return cur;
      --cur;
    }

    Error("No free slots. Hash map is not working correctly.");

    return (Node*)cHashOpenNode;
  }

};
}// namespace Zero
