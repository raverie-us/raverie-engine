///////////////////////////////////////////////////////////////////////////////
///
/// \file SapContainers.hpp
/// Declares the SapPairManager, SapBox, SapEndPoint, SapRange and
/// SapPairRange classes.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///The SapPairManager is used to keep track pairs in Sap while performing
///incremental updates. This pair manager is designed for use with multi-Sap
///which means it keeps a reference count for each Sap that has an overlap.
///This pair manager requires that the ClientDataType has a hasher.
template <typename ClientDataType>
class SapPairManager
{
public:
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef BaseClientPair<ClientDataType> ClientPairType;
  typedef Pair<uint, ClientPairType> ReferencedPair;
  typedef HashMap<PairId, ReferencedPair> PairMap;

  typedef typename PairMap::range range;

  SapPairManager() {};
  ~SapPairManager() {};

  void AddPair(DataType& data1, DataType& data2)
  {
    //get the id's of the two client data's
    HashPolicy<ClientDataType> hasher;
    uint id1 = hasher(data1.mClientData);
    uint id2 = hasher(data2.mClientData);
    //make sure the aren't the same
    if(id1 == id2)
      return;

    //get the lexicographic index of this pair
    PairId index = GetLexicographicId(id1,id2);

    //since this manager is designed for multi-sap, we need to
    //keep track of how many Sap's have inserted this key. Therefore,
    //if the key does not exist, create it, otherwise
    //increment the reference count on that key.

    typename PairMap::range r = mPairs.Find(index);

    //If it's not there, Insert it.
    if(r.Empty())
    {
      ClientPairType pair(data1,data2);
      mPairs.Insert(index, ReferencedPair(1,pair));
    }
    //Else, increment the reference count.
    else
      ++r.Front().second.first;
  }

  void RemovePair(DataType& data1, DataType& data2)
  {
    //get the id's of the two client data's
    HashPolicy<ClientDataType> hasher;
    uint id1 = hasher(data1.mClientData);
    uint id2 = hasher(data2.mClientData);
    //get the lexicographic index of this pair
    PairId index = GetLexicographicId(id1,id2);

    typename PairMap::range r = mPairs.Find(index);

    //Since this is designed for multi-Sap, this pair is being removed
    //from only one Sap right now. Therefore, if the pair would still
    //have a reference left, just decrement the counter. Otherwise, remove it.

    ErrorIf(r.Empty(), "Invalid remove.");

    //Erase the pair if theres only one reference left
    if(r.Front().second.first == 1)
      mPairs.Erase(index);
    //Else, decrement the count
    else
      --r.Front().second.first;
  }

  void Clear()
  {
    //Remove all the pairs that were stored.
    //This class owns no new'ed data, so nothing needs to be deleted.
    mPairs.Clear();
  }

  typename PairMap::range All()
  {
    return mPairs.All();
  }

private:
  PairMap mPairs;
};

///An endpoint is used to store a min/max value of an Aabb on an axis.
///It also stores its own index into the indices in the main Sap structure.
///Each axis will contain two endpoints corresponding to the min/max
///of the Aabb on that axis.
struct SapEndPoint
{
  static const uint sMinBit = static_cast<uint>((uint)1 << (sizeof(uint) * 8 - 1));

  SapEndPoint()
  {
    //Set the index to the largest unsigned int value.
    mIndex = static_cast<uint>(-1);
  }

  SapEndPoint(uint index, bool isMin, real val)
  {
    mIndex = index;
    mVal = val;

    //set the min bit depending on if this is a min or max endpoint
    if(isMin)
      mIndex |= sMinBit;
    else
      mIndex &= ~sMinBit;
  }

  ///Returns the index with the min bit stripped out.
  unsigned int GetIndex()const
  {
    //return the index without the min bit
    return (~sMinBit) & mIndex;
  }

  ///Returns the min bit from the index.
  bool isMin()const
  {
    //return the min bit of index
    return (sMinBit & mIndex) != 0;
  }

  bool operator>(const SapEndPoint& rhs) const
  {
    return mVal > rhs.mVal;
  }

  ///The highest bit is a flag signifying if this is a min 
  ///or max while the rest is used to store the index.
  uint mIndex;
  real mVal;
};

///The SapBox stores the ClientData provided by the user as well
///as the current Aabb of that object.
template <typename ClientDataType>
struct SapBox
{
  typedef BaseBroadPhaseData<ClientDataType> DataType;

  SapBox() {mObj = nullptr;}
  SapBox(DataType& data)
  {
    mData = data;
    mObj = &mData;

    //Updates each index of the box.
    UpdateBox(0,data);
    UpdateBox(1,data);
    UpdateBox(2,data);
  }
  SapBox(const SapBox& box)
  {
    Copy(box);
  }

  SapBox& operator=(const SapBox& box)
  {
    Copy(box);
    return *this;
  }

  void Copy(const SapBox& box)
  {
    mData = box.mData;
    mObj = &mData;
    mMins = box.mMins;
    mMaxs = box.mMaxs;
  }

  bool Valid() const
  {
    return mObj != nullptr;
  }

  void UpdateBox(uint axis, DataType& data)
  {
    //This seems like overkill now, but when swept collision 
    //is taken into account it becomes necessary to easily update 
    //only one index while taking into account the extension from velocity.

    //don't override the proxy here!!!

    //get the min and max extents of the aabb
    mMins[axis] = data.mAabb.mMin[axis];
    mMaxs[axis] = data.mAabb.mMax[axis];
  }

  void* mObj;
  DataType mData;
  Vec3 mMins;
  Vec3 mMaxs;
};

///A range for performing a re-entrant cast into Sap. Used to
///perform queries without having to provide a callback function.
///Note: this range will become completely invalidated if any operations are
///performed on Sap.
template <typename ClientDataType, typename QueryType, typename PolicyType = BroadPhasePolicy<QueryType,Aabb> >
struct SapRange
{
  typedef SapBox<ClientDataType> BoxType;
  typedef Array<SapEndPoint> EndPointArray;
  typedef Array<BoxType> BoxArray;
  typedef Array<uint> IndexArray;

  SapRange(BoxArray* boxes, const QueryType& queryObj)
  {
    SetUp(boxes,queryObj,PolicyType());
  }

  SapRange(BoxArray* boxes, const QueryType& queryObj, PolicyType policy)
  {
    SetUp(boxes,queryObj,policy);
  }

  void SetUp(BoxArray* boxes, const QueryType& queryObj, PolicyType policy)
  {
    mPolicy = policy;
    mBoxes = boxes;
    mQueryObj = queryObj;
    mQueryAabb = ToAabb(mQueryObj);
    mIndex = 1;//skip the sentinel
    SkipDead();
  }

  void PopFront()
  {
    ErrorIf(Empty(),"Popped an empty range.");

    ++mIndex;
    SkipDead();
  }

  ClientDataType Front()
  {
    return (*mBoxes)[mIndex].mData.mClientData;
  }

  bool Empty() const
  {
    //skip the sentinel
    return mIndex >= mBoxes->Size();
  }

  void SkipDead()
  {
    //we have to loop over all of the boxes because we can't otherwise
    //deal with a box that completely Contains the query aabb
    uint end = mBoxes->Size();
    for(; mIndex < end; ++mIndex)
    {
      BoxType& box = (*mBoxes)[mIndex];
      if(!box.Valid())
        continue;

      //test the queryAabb against this box's aabb
      Aabb boxAabb;
      boxAabb.SetMinAndMax(box.mMins,box.mMaxs);
      if(!boxAabb.Overlap(mQueryAabb))
        continue;
      //if that works, then see if the actual query object overlaps
      //this box's aabb.
      if(!mPolicy.Overlap(mQueryObj,boxAabb))
        continue;
      break;
    }
  }

  ///The current box index to search
  uint mIndex;

  static const uint mSearchAxis = 0;
  
  PolicyType mPolicy;
  BoxArray* mBoxes;
  QueryType mQueryObj;
  Aabb mQueryAabb;
};

///A range for iterating through the self pairs of Sap.
///Note: this range will become completely invalidated if any operations are
///performed on Sap.
template <typename ClientDataType>
struct SapPairRange
{
  typedef SapPairManager<ClientDataType> PairManagerType;

  SapPairRange(PairManagerType* manager)
    : mRange(manager->All())
  {

  }

  bool Empty()
  {
    return mRange.Empty();
  }

  void PopFront()
  {
    mRange.PopFront();
  }

  BaseClientPair<ClientDataType> Front()
  {
    return mRange.Front().second.second;
  }

  uint Length()
  {
    return mRange.Size();
  }

  typename PairManagerType::range mRange;
};

}//namespace Zero

