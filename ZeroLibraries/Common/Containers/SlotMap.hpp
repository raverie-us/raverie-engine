///////////////////////////////////////////////////////////////////////////////
///
/// \file SlotMap.hpp
/// Slot map container.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ContainerCommon.hpp"
#include "Allocator.hpp"

namespace Zero
{

///Intrusive slot map container. Uses an array of slots combined with
//a unique id. Value Type MUST be a pointer.
template<typename keyType, typename dataType, typename slotPolicy, 
         typename Allocator = DefaultAllocator>
class Slotmap : public AllocationContainer<Allocator>
{
public:
  typedef dataType data_type;
  typedef dataType* pointer;
  typedef const dataType* const_pointer;
  typedef dataType& reference;
  typedef const dataType& const_reference;
  typedef ptrdiff_t difference_type;
  typedef Slotmap<keyType, dataType, slotPolicy> this_type;

  typedef AllocationContainer<Allocator> base_type;
  using base_type::mAllocator;

  static const size_t InvalidSlotIndex = 0xFFFFFFFF;

  pointer mData;
  data_type mNextFree;
  size_t mLargestSize;
  size_t mMaxSize;
  size_t mSize;
  pointer mBegin;
  pointer mEnd;


  Slotmap()
  {
    mSize = 0;
    mMaxSize = 0;
    mLargestSize = 0;
    mNextFree = nullptr;
    mBegin = nullptr;
    mEnd = nullptr;
    mData = nullptr;
  }

  ~Slotmap()
  {
    Deallocate();
  }

  //The range/iterator for this container may not be intuitive.
  //We move over an array of slots just like iterating over
  //array of object pointers. The difference is that 
  //invalid slots must be skipped over.
  struct range
  {
  public:
     typedef dataType value_type;
     typedef data_type FrontResult;

    range(pointer b, pointer e, this_type* owner)
      : begin(b), end(e), owner(owner)
    {}

    bool Empty() { return begin == end; }
    //NOT a reference
    data_type Front()
    {
      ErrorIf(Empty(),"Accessed empty range.");
      return *begin;
    }
    void PopFront()
    {
      ErrorIf(Empty(),"Popped empty range.");
      ++begin;
      //Skip over slots
      begin = owner->SkipInvalid(begin, end);
    }

    range& All() { return *this; }
    const range& All() const { return *this; }

  private:
    pointer begin;
    pointer end;
    this_type* owner;
  };

  range All()
  {
    pointer begin = PointerBegin();
    pointer end = PointerEnd();
    begin = SkipInvalid(begin, end);
    return range(begin, end, this);
  }

  bool IsValid(data_type objPointer)
  {
    if(objPointer == nullptr)
      return false;

    size_t* valueTableBase = (size_t*)mBegin;
    size_t* endOfValueTable = (size_t*)mEnd;
    if((size_t*)objPointer < endOfValueTable && (size_t*)objPointer >= valueTableBase)
      return false;
    else
      return true;
  }

  data_type FindValue(keyType& id)
  {
    //Get the slot index from the id
    size_t slotIndex = id.GetSlot();
    if(slotIndex > mMaxSize)
      return nullptr;

    //Get the value from the table
    data_type objPointer = mData[slotIndex]; 

    //This value may be a part of the internal free slot list
    //not a valid object pointer
    if(IsValid(objPointer))
    {
      //The pointer is valid by it may point to a object
      //that has replaced the deleted object in the slot.
      //Check to see if the id is equal.
      keyType& foundId = slotPolicy::AccessId(objPointer);
      //Use operator equals for the id.
      if(foundId == id)
      {
        //id and object match this is correct object
        return objPointer;
      }
    }

    return nullptr;
  }

  range Find(keyType& id)
  {
    if(mData == nullptr)
     return range(PointerEnd(), PointerEnd(), this);
    data_type value = FindValue(id);
    if(value == nullptr)
      return range(PointerEnd(), PointerEnd(), this);
    else
    {
      pointer b = PointerBegin() + id.GetSlot();
      return range(b, b + 1, this);
    }
  }

  ///Is the container empty?
  bool Empty() { return mSize == 0; }

  //How many elements are stored in the container.
  size_t Size() { return mSize; }

  void Clear()
  {
    mSize = 0;
    InitializeSlots(0, mMaxSize);
  }

  void Erase(keyType& eraseId)
  {
    //This slot index is now free
    size_t slotIndex = eraseId.GetSlot();
    ErrorIf(slotIndex > mMaxSize, "Invalid slot index.");

    //Get the value from the table
    data_type pointer = mData[slotIndex]; 
    keyType& foundId = slotPolicy::AccessId(pointer);
    if(foundId == eraseId)
    {
      AddToFreeList(eraseId.GetSlot());
      //Reduce the size
      --mSize;
#ifdef ZeroDebug
      eraseId.GetSlot() = InvalidSlotIndex;
#endif
    }

  }

  void Erase(data_type objectToErase)
  {
    keyType& eraseId = slotPolicy::AccessId(objectToErase);
    return Erase(eraseId);
  }

  void Deallocate()
  {
    if(mMaxSize != 0)
    {
      mAllocator.Deallocate(mData, mMaxSize * sizeof(data_type));
      mSize = 0;
      mMaxSize = 0;
      mData = nullptr;
      mNextFree = nullptr;
    }
  }

  //Must already have a unique id
  void Insert(data_type object)
  {
    keyType& insertKey = slotPolicy::AccessId(object);

    if(mNextFree == nullptr)
    {
      //Need more slots
      size_t currentSize = mMaxSize;
      size_t newMaxSize = mMaxSize * 2;
      if(newMaxSize == 0)
        newMaxSize = 4;

      pointer newTable = (pointer)mAllocator.Allocate(newMaxSize*sizeof(dataType));

      //Copy over old elements
      if(mMaxSize != 0)
      {
        mAllocator.MemCopy(newTable, mData, mMaxSize*sizeof(dataType));
        mAllocator.Deallocate(mData, mMaxSize*sizeof(data_type));
      }

      mData = newTable;
      mMaxSize = newMaxSize;

      InitializeSlots(currentSize, newMaxSize);

      mBegin = mData;
      mEnd = mData + mMaxSize;
    }

    size_t* base = (size_t*)mBegin;
    size_t index = (size_t*)mNextFree - base;
    mNextFree = *(data_type*)mNextFree;

    insertKey.Slot = index;
    mData[index] = object;

    //increase the size
    ++mSize;

    //increase the watermark size
    if(mSize > mLargestSize)
      mLargestSize = mSize;
  }

private:
  void InitializeSlots(size_t startIndex, size_t endIndex)
  {
    for(size_t i = startIndex; i < endIndex - 1; ++i)
    {
      data_type next = (data_type)&mData[i + 1];
      mData[i] = next;
    }

    mData[endIndex - 1] = nullptr;
    mNextFree = (data_type)&mData[startIndex];
  }

  void AddToFreeList(size_t slotIndex)
  {
    //Get a pointer to the slot
    size_t* base = (size_t*)mBegin;
    size_t* next = base + slotIndex;

    //Set it to the current free and the next
    //free to this one.
    mData[slotIndex] = (data_type)mNextFree;
    mNextFree = (data_type)next;
  }

  pointer SkipInvalid(pointer cur, pointer end)
  {
    //While not at the end of the list
    //and the slot is invalid move the pointer forward.
    while(cur!=end && !IsValid(*cur))
      ++cur;
    return cur;
  }

  pointer PointerBegin(){return mBegin;}
  pointer PointerEnd(){return mEnd;}

  //non copyable
  Slotmap(const Slotmap& other);
  void operator=(const Slotmap& other);


};

}//namespace Zero
