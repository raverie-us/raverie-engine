///////////////////////////////////////////////////////////////////////////////
///
/// \file BlockArray.hpp
/// Declaration of the BlockArray container.
///
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Containers/ContainerCommon.hpp"
#include "Containers/Allocator.hpp"
#include "Containers/Algorithm.hpp"

namespace Zero
{

///Generic front deque array class. Store objects in buckets of a 1 << shiftSize.
///Fast when the total number of objects being inserted is not known.
///Currently hardcoded as only storing pod types.
template<typename type, uint shiftSize = 6, typename Allocator = DefaultAllocator>
class PodBlockArray : public AllocationContainer<Allocator>
{
public:
  //-------------------------------------------------------- Standard Typedefs
  typedef type value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef PodBlockArray<type, shiftSize, Allocator> this_type;
  typedef AllocationContainer<Allocator> base_type;
  using base_type::mAllocator;

  enum
  {
    /*used to determine the size of a bucket as well as to mask off
    the lower bits and determine the number of buckets*/
    ShiftSize = shiftSize,
    BucketSize = 1 << ShiftSize,//the size of a bucket (how many objects)
    BucketMask = BucketSize - 1,//mask to determine the index within a bucket
  };

  struct iterator
  {
    iterator()
    {
      mBlockArray = nullptr;
      mCurrentIndex = 0;
    }

    iterator(this_type* blockArray, size_type startIndex)
    {
      mBlockArray = blockArray;
      mCurrentIndex = startIndex;
    }

    void operator++()
    {
      ++mCurrentIndex;
    }

    void operator--()
    {
      --mCurrentIndex;
    }

    reference operator*()
    {
      return (*mBlockArray)[mCurrentIndex];
    }

    size_type operator-(const iterator& other)
    {
      return mCurrentIndex - other.mCurrentIndex;
    }

    iterator operator+(size_type offset)
    {
      return iterator(mBlockArray, mCurrentIndex + offset);
    }

    bool operator==(const iterator& other)
    {
      if(mBlockArray == other.mBlockArray && 
         mCurrentIndex == other.mCurrentIndex)
        return true;
      return false;
    }

    bool operator<(const iterator& other)
    {
      return mCurrentIndex < other.mCurrentIndex;
    }

    bool operator!=(const iterator& other)
    {
      return !(*this == other);
    }

    this_type* mBlockArray;
    size_type mCurrentIndex;
  };

  struct range
  {
    typedef typename this_type::value_type      value_type;
    typedef value_type                          contiguousRangeType;

    range()
    {
      mCurrentIndex = 0;
      mEndIndex = 0;
      mBlockArray = nullptr;
    }

    range(this_type* blockArray)
    {
      mBlockArray = blockArray;
      mCurrentIndex = 0;
      mEndIndex = blockArray->mSize;
    }

    range(this_type* blockArray, size_type startIndex, size_type endIndex)
    {
      mBlockArray = blockArray;
      mCurrentIndex = startIndex;
      mEndIndex = endIndex;
    }

    reference Front()
    {
      return (*mBlockArray)[mCurrentIndex];
    }

    void PopFront()
    {
      ++mCurrentIndex;
    }

    bool Empty()
    {
      return mCurrentIndex >= mEndIndex;
    }

    iterator Begin()
    {
      return iterator(mBlockArray,mCurrentIndex);
    }

    iterator End()
    {
      return iterator(mBlockArray,mEndIndex);
    }

    range& All() { return *this; }
    const range& All() const { return *this; }

    this_type* mBlockArray;
    size_type mCurrentIndex;
    size_type mEndIndex;
  };

  PodBlockArray()
  {
    mSize = 0;
    mCapacity = 0;
    mData = nullptr;
  }

  PodBlockArray(const this_type& other)
  {
    mSize = 0;
    mCapacity = 0;
    mData = nullptr;
    Copy(other);
  }

  PodBlockArray(size_type size)
  {
    mData = nullptr;
    mSize = 0;
    mCapacity = 0;
    Resize(size);
  }

  PodBlockArray(size_type size, const_reference defaultValue)
  {
    mData = nullptr;
    mSize = 0;
    mCapacity = 0;
    Resize(size, defaultValue);
  }

  ~PodBlockArray()
  {
    Deallocate();
  }

  void operator=(const this_type& other)
  {
    Copy(other);
  }

  bool Empty() const
  {
    return mSize == 0;
  }

  size_t Size() const
  {
    return mSize;
  }

  reference operator[](size_t index)
  {
    ErrorIf(index > mSize, "Accessed block array out of bounds.");
    return mData[index >> ShiftSize][index & BucketMask];
  }

  const_reference operator[](size_t index) const
  {
    ErrorIf(index > mSize, "Accessed block array out of bounds.");
    return mData[index >> ShiftSize][index & BucketMask];
  }

  iterator Begin()
  {
    return iterator(this,0);
  }

  iterator End()
  {
    return iterator(this, mSize);
  }

  reference Front()
  {
    ErrorIf(mSize == 0, "Empty block array, no front element.");
    return mData[0][0];
  }

  const_reference Front() const
  {
    ErrorIf(mSize == 0, "Empty block array, no front element.");
    return mData[0][0];
  }

  reference Back()
  {
    ErrorIf(mSize == 0, "Empty block array, no back element.");
    return (*this)[mSize - 1];
  }

  const_reference Back() const
  {
    ErrorIf(mSize == 0, "Empty block array, no back element.");
    return (*this)[mSize - 1];
  }

  void PushBack(const_reference item)
  {
    //if adding one object causes us to go over our capacity,
    //then increase our capacity by one bucket
    if(mSize >= mCapacity)
      ChangeCapacity(mCapacity + 1);

    //Insert the new item at the last index
    (*this)[mSize] = item;
    ++mSize;
  }
  
  reference PushBack()
  {
    //see the above PushBack
    if(mSize >= mCapacity)
      ChangeCapacity(mCapacity + 1);

    //we aren't filling in the last item, just giving
    //back a reference for the user to fill out
    ++mSize;
    return (*this)[mSize - 1];
  }

  void PopBack()
  {
    ErrorIf(mSize == 0,"Empty block array, can not pop back element.");
    //don't change capacity at all, just decrement our size
    //(since we are pod we don't have to call a destructor on the last item)
    --mSize;
  }

  void Clear()
  {
    //pod assumption, don't call destructors on the objects
    mSize = 0;
  }

  void Deallocate()
  {
    size_type bucketCount = BucketCountFromSize(mCapacity);
    //deallocate all of our buckets
    for(uint i = 0; i < bucketCount; ++i)
      mAllocator.Deallocate(mData[i], sizeof(value_type) * BucketSize);
    //now deallocate the array of our buckets
    mAllocator.Deallocate(mData, sizeof(pointer) * bucketCount);
  }

  void ChangeCapacity(size_type newCapacity)
  {
    //we never shrink at the moment, so if the capacity is the same or less do nothing
    if(newCapacity <= mCapacity)
      return;

    //round capacity up to the next bucket size
    if(newCapacity & BucketMask)
      newCapacity = (newCapacity & ~BucketMask) + (1 << ShiftSize);

    size_type newBucketSize = BucketCountFromSize(newCapacity);
    size_type oldBucketSize = BucketCountFromSize(mCapacity);
    //allocate our new array of buckets
    pointer* newData = (pointer*)mAllocator.Allocate(newBucketSize * sizeof(pointer));
    
    //if we had any buckets previously, copy over our pointers to them
    //then deallocate the old array of buckets
    if(mCapacity != 0)
    {
      UninitializedMove(newData,mData,oldBucketSize,true_type());
      mAllocator.Deallocate(mData, sizeof(pointer) * oldBucketSize);
    }

    //now allocate any new buckets we need
    for(size_t i = oldBucketSize; i < newBucketSize; ++i)
      newData[i] = (pointer)mAllocator.Allocate(sizeof(value_type) * BucketSize);

    //finally set our new state
    mData = newData;
    mCapacity = newCapacity;
  }

  range All()
  {
    return range(this);
  }

  //Returns and range for all the elements in the container.
  range All() const
  {
    return const_cast<PodBlockArray*>(this)->All();
  }

  range SubRange(size_type index, size_type length)
  {
    ErrorIf(index >= mSize, "Accessed block array out of bounds.");
    ErrorIf(index + length > mSize, "Accessed block array out of bounds.");
    return range(this, index, index + length);
  }

  bool operator==(this_type& other)
  {
    return Equal(this->All(), other.All());
  }

  void Resize(size_t newSize)
  {
    //resizing to the same size, do nothing
    if(mSize == newSize)
      return;

    //we only need to handle getting bigger, because getting smaller is
    //just overriding our size variable (pod assumption)

    //if our new size causes us to go beyond our capacity,
    //compute and change the capacity
    if(newSize > mCapacity)
      ChangeCapacity(newSize);

    mSize = newSize;
  }

  void Resize(size_t newSize, const_reference defaultvalue)
  {
    //resizing to the same size, do nothing
    if(mSize == newSize)
      return;

    //compute our old/new capacity and the index inside of those buckets
    size_type oldBucketCount = BucketCountFromSize(mSize);
    size_type newBucketCount = BucketCountFromSize(newSize);
    
    //update the capacity
    ChangeCapacity(newSize);
      
    //now we have to set the default value in all of the new buckets (only if we grew)
    if(newSize > mSize)
    {
      // The sizes within each bucket. Note that size with a block size of 64, size 64 needs
      // to stay 64 and not turn into 0. This is done with the extra +-1 terms. This will cause a
      // size of 0 to get incorrect values, but we should only ever have a size 0 here when going
      // from 0 to something which is handled as a special case.
      size_type sizeInOldBucket = ((mSize - 1) & BucketMask) + 1;
      size_type sizeInNewBucket = ((newSize - 1) & BucketMask) + 1;
      // The index of each bucket
      size_type oldBucketIndex = oldBucketCount - 1;
      size_type newBucketIndex = newBucketCount - 1;
      
      // Deal with having come from no buckets (size of 0)
      if(oldBucketCount == 0)
      {
        // Completely fill any buckets we've skipped
        for(size_type i = 0; i < newBucketIndex; ++i)
          UninitializedFill(mData[i], BucketSize, defaultvalue);
        // Fill the remaining portion of the last bucket
        UninitializedFill(mData[newBucketIndex], sizeInNewBucket, defaultvalue);
      }
      // The bucket sizes are the same, so we just grew within the bucket.
      else if(oldBucketCount == newBucketCount)
      {
        UninitializedFill(mData[newBucketIndex] + sizeInOldBucket, sizeInNewBucket - sizeInOldBucket, defaultvalue);
      }
      else
      {
        // First, set the default value in any remaining portion of the old bucket
        UninitializedFill(mData[oldBucketIndex] + sizeInOldBucket, BucketSize - sizeInOldBucket, defaultvalue);
        // Now fill any complete buckets up
        for(uint i = oldBucketIndex + 1; i < newBucketIndex; ++i)
          UninitializedFill(mData[i], BucketSize, defaultvalue);
        // Finally, fill out the used portion of the last bucket
        UninitializedFill(mData[newBucketIndex], sizeInNewBucket, defaultvalue);
      }
    }

    mSize = newSize;
  }

  void Copy(const this_type& other)
  {
    //fix our size to be the same as the other object
    Resize(other.mSize);

    //if we have no size, don't do anything
    if(other.mSize == 0)
      return;

    size_type bucketSize = BucketCountFromSize(mSize);
    size_type lastBucket = bucketSize - 1;
    size_type lastBucketFillCount = mSize & BucketMask;

    //fill out all complete buckets
    for(size_type i = 0; i < lastBucket; ++i)
      UninitializedCopy(mData[i],other.mData[i],BucketSize,true_type());
    //fill out the remaining portion of the last bucket
    UninitializedCopy(mData[lastBucket],other.mData[lastBucket],lastBucketFillCount,true_type());
  }

  size_type BucketCountFromSize(size_type size)
  {
    size_type buckets = size >> ShiftSize;
    if(size & BucketMask)
      ++buckets;
    return buckets;
  }

  ///The current number of items we have
  size_type mSize;
  ///The max number of items we can store before we
  ///need to allocate more (bucketSize * numOfBuckets)
  size_type mCapacity;
  ///The start of our 2d array
  pointer* mData;
};

}//namespace Zero
