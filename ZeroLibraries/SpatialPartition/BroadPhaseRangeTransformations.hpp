///////////////////////////////////////////////////////////////////////////////
///
/// \file BroadPhaseRangeTransformations.hpp
/// Implements the RangeSorter struct.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Zero
{

/// A range that takes a range from broadphase and a functor to retrieve the
/// t value and sorts the items into the correct t first order. Templated
/// based upon the max number of items that will be recorded.
template <typename ClientDataType, uint ItemCount>
struct RangeSorter
{
  typedef ClientDataType ClientDataTypeDef;

  struct Item
  {
    ClientDataType Data;
    real T;
  };

  RangeSorter()
  {
    mCount = 0;
    mIteratorIndex = 0;
  }

  /// The functor takes the client data type and a reference to the t value to
  /// fill out and returns a boolean as to whether or not the item was hit.
  template <typename RangeType, typename Functor>
  void Sort(RangeType& range, Functor functor)
  {
    while(!range.Empty())
    {
      ClientDataType& data = range.Front();
      range.PopFront();

      Item item;
      item.Data = data;
      if(functor(item.Data,item.T))
        PushBack(item);
    }
  }

  bool Empty()
  {
    return mIteratorIndex == mCount;
  }

  ClientDataType& Front()
  {
    ErrorIf(mIteratorIndex >= mCount,"Cannot access front on an empty range.");
    return mItems[mIteratorIndex].Data;
  }

  void PopFront()
  {
    ErrorIf(mIteratorIndex >= mCount,"Cannot pop an empty range.");
    ++mIteratorIndex;
  }

private:
  void PushBack(Item& item)
  {
    //deal with the base case of having no items.
    if(mCount == 0)
    {
      mItems[0] = item;
      ++mCount;
      return;
    }

    //if we are at max capacity
    if(mCount == ItemCount)
    {
      //if the new item happens after all of our items then we don't care
      if(item.T > mItems[mCount - 1].T)
        return;
    }
    //if we still have capacity, then we have to increment the number of objects we have
    else
      ++mCount;

    //in an attempt to be more efficient, instead of inserting the new item and shuffling it down,
    //we shuffle up the items that happen after the new item then put the new item in.
    //This will reduce the copies by about half since we only copy the new item once.

    //we know that the last item happens after us, so we want to check all of the items before that
    uint index = mCount - 2;
    while(index > 0)
    {
      //if the current item happened before the new item, then we have found our insertion point which is one after this item
      if(mItems[index].T <= item.T)
        break;

      //otherwise, this item happened after the new item so push the old one back a slot
      mItems[index + 1] = mItems[index];
      --index;
    }

    // have to deal with the zero case specially because of unsigned integers
    if(index == 0)
    {
      //if the first item is sooner than the new item, Insert the new item after the first one
      if(mItems[index].T <= item.T)
        mItems[index + 1] = item;
      //otherwise we have to shuffle the first item back one and put the new one at the first slot
      else
      {
        mItems[index + 1] = mItems[index];
        mItems[index] = item;
      }
    }
    //if we weren't at the zero case we can just Insert the item
    else
      mItems[index + 1] = item;
  }

  uint mIteratorIndex;
  uint mCount;
  Item mItems[ItemCount];
};

/// A range that takes a range from broadphase and a functor to retrieve the
/// t value and sorts the items into the correct t first order. This will
/// retrieve all items from the broadphase.
template <typename ClientDataType>
struct FullRangeSorter
{
  typedef ClientDataType ClientDataTypeDef;

  struct Item
  {
    ClientDataType Data;
    real T;
  };

  FullRangeSorter()
  {
    mIteratorIndex = 0;
  }

  /// The functor takes the client data type and a reference to the t value to
  /// fill out and returns a boolean as to whether or not the item was hit.
  template <typename RangeType, typename Functor>
  void Sort(RangeType& range, Functor functor)
  {
    while(!range.Empty())
    {
      ClientDataType& data = range.Front();
      range.PopFront();

      Item item;
      item.Data = data;
      if(functor(item.Data,item.T))
        mItems.PushBack(item);
    }

    Sort(mItems.All());
  }

  bool Empty()
  {
    return mIteratorIndex == mCount;
  }

  ClientDataType& Front()
  {
    ErrorIf(mIteratorIndex >= mItems.Size(),"Cannot access front on an empty range.");
    return mItems[mIteratorIndex].Data;
  }

  void PopFront()
  {
    ErrorIf(mIteratorIndex >= mItems.Size(),"Cannot pop an empty range.");
    ++mIteratorIndex;
  }

  uint mIteratorIndex;
  Array<Item> mItems;
};

}//namespace Zero
