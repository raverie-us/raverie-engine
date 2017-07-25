///////////////////////////////////////////////////////////////////////////////
///
/// \file WeightedProbabilityTable.hpp
/// Implementation of the WeightedProbabilityTable class.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Random.hpp"

namespace Math
{

/// A weighted probability table that is 0(1) lookup, O(n) build time and
/// O(n) memory usage. Very useful for randomly sampling a weighted die.
/// Implementation reference: http://www.keithschwarz.com/darts-dice-coins/
template <typename valueType>
struct ZeroSharedTemplate WeightedProbabilityTable
{
  typedef real WeightType;
  typedef valueType ValueType; 
  typedef WeightedProbabilityTable<valueType> SelfType;

  /// Internal item entry.
  struct Item
  {
    Item() { mAlias = 0; };

    WeightType mProbability;
    ValueType mValue;

  //private: (not private because of unit tests)
    WeightType mWeightedProbability;
    uint mAlias;
  };
  typedef Zero::Array<Item> Items;

  WeightedProbabilityTable()
  {
    mTotalProbability = WeightType(0.0);
  }

  uint Size() const
  {
    return mItems.Size();
  }

  /// Clear removes all of the items in the table. Warning, the table does not
  /// handle sample being called when there are no items. This should only be
  /// used to quickly clear the table before adding more items.
  void Clear()
  {
    mItems.Clear();
    mTotalProbability = WeightType(0.0);
  }

  /// Adds a new value and probability weight to the table.
  /// Doesn't build the new table with this value on its own.
  void AddItem(const ValueType& value, const WeightType& probability)
  {
    //create a new item, Assign it it's true probability
    //(not normalized) and whatever the value is
    Item& item = mItems.PushBack();
    item.mProbability = probability;
    item.mValue = value;

    //we need the total probability to normalize later
    mTotalProbability += probability;
  }

  /// Removes the item at the given index.
  /// Doesn't build the new table on it's own.
  void RemoveItemAt(uint index)
  {
    ErrorIf(index >= Size(),"Invalid index.");

    mTotalProbability -= mItems[index].mProbability;
    mItems.EraseAt(index);
  }

  void RecomputeTotalProbability()
  {
    mTotalProbability = WeightType(0.0);
    for(uint i = 0; i < Size(); ++i)
      mTotalProbability += mItems[i].mProbability;
  }

  /// Given all of the current inputs, rebuilds the table from scratch. There
  /// is no way to incrementally build this table, but it's only 3n to compute.
  void BuildTable()
  {
    //make sure that the total probability is correct (used to re-normalize)
    RecomputeTotalProbability();

    uint count = mItems.Size();

    //we need to figure out which items when normalized are > 1 and which are < 1
    typedef Zero::Array<uint> IndexArray;
    IndexArray smallItems,largeItems;
    smallItems.Reserve(count);
    largeItems.Reserve(count);

    //loop through all the items and figure out which bucket they go in. First,
    //normalize all the values by dividing them by the total probability so the
    //total probability is 1. Then we change the total probability to be n by
    //multiplying by the size of the array. This will produce the table size
    //we need for this algorithm.
    for(uint i = 0; i < count; ++i)
    {
      mItems[i].mWeightedProbability = count * mItems[i].mProbability / mTotalProbability;
      if(mItems[i].mWeightedProbability < 1)
        smallItems.PushBack(i);
      else
        largeItems.PushBack(i);
    }

    //now as long as we have an item in both large and small, we'll continue
    //(check both because of numerical instabilities)
    while(!smallItems.Empty() && !largeItems.Empty())
    {
      //grab and remove 1 item from both the large and small stacks
      uint smallIndex = smallItems.Back();
      uint largeIndex = largeItems.Back();
      smallItems.PopBack();
      largeItems.PopBack();

      //since the small item has room left over, we'll "fill" the remainder
      //of that room with this item from the large bucket.
      mItems[smallIndex].mAlias = largeIndex;
      //now we removed some of the current probability from the large bucket,
      //so recompute what portion remains (could do large -= small,
      //but (large + small) - 1 is apparently more numerically stable.
      mItems[largeIndex].mWeightedProbability = (mItems[largeIndex].mWeightedProbability + mItems[smallIndex].mWeightedProbability) - 1;
      //now we have to check to see if the large item is still large or not, put it in the correct bucket
      if(mItems[largeIndex].mWeightedProbability < 1)
        smallItems.PushBack(largeIndex);
      else
        largeItems.PushBack(largeIndex);
    }

    //normally we will run out of small items and still have some large ones.
    //By a mathematical proof in the link above, all of these large items
    //should have a probability of 1. So just set their probability
    //to exactly 1 to deal with numerical issues.
    while(!largeItems.Empty())
    {
      uint index = largeItems.Back();
      mItems[index].mWeightedProbability = WeightType(1.0);
      largeItems.PopBack();
    }

    //same as above, but this case should only happen because a large
    //item accidentally being put into small (aka end up with .99 instead
    //of 1.01 because of rounding errors)
    while(!smallItems.Empty())
    {
      uint index = smallItems.Back();
      mItems[index].mWeightedProbability = WeightType(1.0);
      smallItems.PopBack();
    }
  }

  /// Given a randomly rolled fair die and a randomly flipped weighted coin,
  /// returns the index of the item that was rolled on a weighted die.
  uint SampleIndex(uint dieIndex, const WeightType& coinFlip)
  {
    //what we've changed a loaded die roll to is a fair die roll combined
    //with a loaded coin flip (which is easy to do in a computer)

    //so given our fair die roll see if the weighted coin flip is within the
    //probability for this bucket, if it isn't then that means we are in
    //the remaining portion that belongs to our aliased item.
    if(coinFlip < mItems[dieIndex].mWeightedProbability)
      return dieIndex;
    
    return mItems[dieIndex].mAlias;
  }

  /// Given two randomly rolled floats that are in the range [0,1), returns the
  /// index of the items that was rolled on a weighted die.
  uint SampleIndex(float random1, float random2)
  {
    ErrorIf(random1 >= 1.0f || random2 >= 1.0f,
      "random value passed in was not in the range of [0,1).");
    uint dieIndex = static_cast<uint>(random1 * Size());
    return SampleIndex(dieIndex,random2);
  }

  /// Given a random class, returns the index that was randomly sampled.
  /// Mainly useful in the unit tests, but can be used if you ever need
  /// the index as well for some reason.
  uint SampleIndex(Random& random)
  {
    uint dieIndex = (uint)random.IntRangeInEx(0,mItems.Size());
    WeightType coinFlip = random.FloatRange(0,1);

    return SampleIndex(dieIndex,coinFlip);
  }

  /// Given a random class, return the value that was randomly sampled.
  ValueType& Sample(Random& random)
  {
    uint index = SampleIndex(random);

    return mItems[index].mValue;
  }
  
  /// Given two random floats from [0,1), returns
  /// the value that was randomly sampled
  ValueType& Sample(float random1, float random2)
  {
    uint index = SampleIndex(random1,random2);

    return mItems[index].mValue;
  }

  Items mItems;
  /// Total probability is used to re-normalize the
  /// values to a total probability of 1.
  WeightType mTotalProbability;
};

}//namespace Math
