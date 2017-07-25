///////////////////////////////////////////////////////////////////////////////
///
/// \file IndexPool.hpp
/// Interface for the index pool
/// 
/// Authors: Killian Koenig
/// Copyright 2013, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#pragma once

template <typename T, typename index_t= s32>
class IndexPool
{
public:
  IndexPool();
  IndexPool(index_t capacity);

  ~IndexPool();

  index_t Allocate();
  void Free(index_t index);

  T* GetElement(index_t index);
  const T* GetElement(index_t index) const;

  index_t GetIndex(const T* node) const;

  index_t GetCapacity() const;
  index_t GetSize() const;

private:
  union Node
  {
    T Element;
    index_t Next;
  };
  
  void Grow();

  Node* mData;
  index_t mCapacity;
  index_t mSize;
  index_t mFreeList;
};

template <typename T, typename index_t>
IndexPool<T, index_t>::IndexPool()
  : mFreeList(-1)
  , mCapacity(8)
  , mSize(0)
{
  mData = new Node[mCapacity];
  for(index_t i = mCapacity - 1; i >= 0; --i)
  {
    Node* node = mData + i;
    node->Next = mFreeList;
    mFreeList = i;   
  }
}

template <typename T, typename index_t>
IndexPool<T, index_t>::IndexPool(index_t capacity)
  : mFreeList(-1)
  , mCapacity(capacity)
  , mSize(0)
{
  mData = new Node[mCapacity];
  for(index_t i = mCapacity - 1; i >= 0; --i)
  {
    Node* node = mData + i;
    node->Next = mFreeList;
    mFreeList = i;   
  }
}

template <typename T, typename index_t>
IndexPool<T, index_t>::~IndexPool()
{
  delete[] mData;
}

template <typename T, typename index_t>
index_t IndexPool<T, index_t>::Allocate()
{
  if(mFreeList == -1)
  {
    Grow();
  }

  index_t node = mFreeList;
  mFreeList = mData[node].Next;
  ++mSize;
  return node;
}

template <typename T, typename index_t>
void IndexPool<T, index_t>::Free(index_t index)
{
  Node* node = mData + index;
  node->Next = mFreeList;
  mFreeList = index;
  --mSize;
}

template <typename T, typename index_t>
inline T* IndexPool<T, index_t>::GetElement(index_t index)
{
  return &mData[index].Element;
}

template <typename T, typename index_t>
inline const T* IndexPool<T, index_t>::GetElement(index_t index) const
{
  return &mData[index].Element;
}

template <typename T, typename index_t>
void IndexPool<T, index_t>::Grow()
{
  Node* old = mData;

  index_t newCapacity = mCapacity << 1;
  mData = new Node[newCapacity];
  for(index_t i = 0; i < mCapacity; ++i)
  {
    *(mData + i) = *(old + i);
  }

  for(index_t i = newCapacity - 1; i >= mCapacity; --i)
  {
    Node* node = mData + i;
    node->Next = mFreeList;
    mFreeList = i;
  }
  mCapacity = newCapacity;

  delete[] old;
}

template <typename T, typename index_t>
index_t IndexPool<T, index_t>::GetIndex(const T* node) const
{
  return (const Node*)node - mData;
}

template <typename T, typename index_t>
index_t IndexPool<T, index_t>::GetCapacity() const
{
  return mCapacity;
}

template <typename T, typename index_t>
index_t IndexPool<T, index_t>::GetSize() const
{
  return mSize;
}
