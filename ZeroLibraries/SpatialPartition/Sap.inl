///////////////////////////////////////////////////////////////////////////////
///
/// \file Sap.cpp
/// Implementation of the Sap class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace SapInternal
{

const int cSentinelPattern = 0xffdeadff;

}//namespace SapInternal

namespace Zero
{

template <typename ClientDataType>
Sap<ClientDataType>::Sap()
{
  Setup();
  mStandAlone = true;
  mPairManager = new PairManagerType();
  //mPairManager = Memory::HeapAllocate<SapPairManager>(mHeap);
}

template <typename ClientDataType>
Sap<ClientDataType>::Sap(PairManagerType* pairManager)
{
  Setup();
  mStandAlone = false;
  mPairManager = pairManager;
}

template <typename ClientDataType>
Sap<ClientDataType>::~Sap()
{
  if(mStandAlone)
    delete mPairManager;
  //  Memory::HeapDeallocate(mHeap,mPairManager);
}

template <typename ClientDataType>
void Sap<ClientDataType>::CreateProxy(BroadPhaseProxy& proxy, DataType& data)
{
  //Create the box from the physics component.
  BoxType box(data);

  uint index = GetNewBoxIndex();
  mBoxes[index] = box;

  //Insert each endpoint
  InsertEndpoint<0>(box,index);
  InsertEndpoint<1>(box,index);
  InsertEndpoint<2>(box,index);

  proxy = BroadPhaseProxy((u32)index);
}

template <typename ClientDataType>
void Sap<ClientDataType>::CreateProxies(DataObjectArray& objects)
{
  //need to keep track of what boxes we were inserting
  Array<uint> insertBoxes;
  insertBoxes.Reserve(objects.Size());

  //resize the endpoint arrays to account for the new proxies
  Array<EndPointType> newEndpoints[3];
  newEndpoints[0].Reserve(objects.Size() * 2);
  newEndpoints[1].Reserve(objects.Size() * 2);
  newEndpoints[2].Reserve(objects.Size() * 2);
  for(uint i = 0; i < objects.Size(); ++i)
  {
    BoxType box(objects[i].mData);
    uint index = GetNewBoxIndex();
    mBoxes[index] = box;

    //push the new endpoints onto lists for each axis
    newEndpoints[0].PushBack(EndPointType(GetBoxMin(0,index),true,box.mMins[0]));
    newEndpoints[0].PushBack(EndPointType(GetBoxMax(0,index),false,box.mMaxs[0]));
    newEndpoints[1].PushBack(EndPointType(GetBoxMin(1,index),true,box.mMins[1]));
    newEndpoints[1].PushBack(EndPointType(GetBoxMax(1,index),false,box.mMaxs[1]));
    newEndpoints[2].PushBack(EndPointType(GetBoxMin(2,index),true,box.mMins[2]));
    newEndpoints[2].PushBack(EndPointType(GetBoxMax(2,index),false,box.mMaxs[2]));

    //keep track of this index so later we can not add pairs between
    //two objects that already existed
    insertBoxes.PushBack(index);

    *(objects[i].mProxy) = BroadPhaseProxy((u32)index);
  }

  //mark an object as just being inserted by nulling it's obj ptr.
  //have to do this here because the copy constructor cannot make this
  //ptr work properly and the box array may be resized above.
  Array<uint>::range range = insertBoxes.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint boxIndex = range.Front();
    mBoxes[range.Front()].mObj = nullptr;
  }

  //sort all of the endpoints to be in the correct spot
  uint startIndex;
  BatchSort<0>(newEndpoints[0]);
  BatchSort<1>(newEndpoints[1]);
  startIndex = BatchSort<2>(newEndpoints[2]);

  //add all of the pairs that we just created
  BatchPairAdd<2>(startIndex);

  //fill back out the obj ptr that we used to
  //signify this box was just added
  range = insertBoxes.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint boxIndex = range.Front();
    mBoxes[boxIndex].mObj = &mBoxes[boxIndex].mData;
  }
}

template <typename ClientDataType>
void Sap<ClientDataType>::RemoveProxy(BroadPhaseProxy& proxy)
{
  uint boxNum = proxy.ToU32();
  ErrorIf(boxNum >= mBoxes.Size() || mBoxes[boxNum].mObj == nullptr,
    "Invalid proxy removed. Proxy did not reference a valid object.");

  //need to take care of the endpoints by setting them to infinity 
  //and updating them. Doing this will properly move the endpoint to 
  //the end and remove all of the pairs that contained this component.
  //do max then min so that a false pair with itself is not created


  //take care to shuffle the max to be larger than the min, but to
  //avoid passing the sentinel node. Manually shift pass the sentinel
  //node once the endpoint is at the end.
  RemoveEndPoints<0>(boxNum);
  RemoveEndPoints<1>(boxNum);
  RemoveEndPoints<2>(boxNum);

  //clear the box pointer
  mBoxes[boxNum].mObj = nullptr;
  //this index is now open
  mOpenIndices.PushBack(boxNum);
}

template <typename ClientDataType>
void Sap<ClientDataType>::RemoveProxies(ProxyHandleArray& proxies)
{
  //null out all of the box obj ptrs
  ProxyHandleArray::range range = proxies.All();
  for(; !range.Empty(); range.PopFront())
  {
    BroadPhaseProxy& proxy = *range.Front();
    uint boxIndex = proxy.ToU32();
    ErrorIf(boxIndex >= mBoxes.Size() || mBoxes[boxIndex].mObj == nullptr,
      "Invalid proxy removed. Proxy did not reference a valid object.");
    mBoxes[boxIndex].mObj = nullptr;
    mOpenIndices.PushBack(boxIndex);
  }

  //let's pretend the z axis is the one of most variance...
  BatchPairRemove<2>();

  //remove all deleted endpoints
  BatchEndPointRemove<0>();
  BatchEndPointRemove<1>();
  BatchEndPointRemove<2>();
}

template <typename ClientDataType>
void Sap<ClientDataType>::UpdateProxy(BroadPhaseProxy& proxy, DataType& data)
{
  uint index = proxy.ToU32();
  ErrorIf(index >= mBoxes.Size() || mBoxes[index].mObj == nullptr,
    "Invalid proxy updated. Proxy did not reference a valid object.");

  //there could be an update where our client data changed
  //so make sure to update it (ie. a remove->Insert)
  mBoxes[index].mData.mClientData = data.mClientData;

  UpdateEndpoint<0>(index,data,true);
  UpdateEndpoint<1>(index,data,true);
  UpdateEndpoint<2>(index,data,true);
}

template <typename ClientDataType>
void Sap<ClientDataType>::UpdateProxies(DataObjectArray& objects)
{
  typename DataObjectArray::range range = objects.All();
  while(!range.Empty())
  {
    DataObjectType& object = range.Front();
    range.PopFront();

    Sap::UpdateProxy(*object.mProxy,object.mData);
  }
}

template <typename ClientDataType>
template <typename QueryType, typename PolicyType>
SapRange<ClientDataType,QueryType> Sap<ClientDataType>::QueryWithPolicy(const QueryType& queryObj, PolicyType policy)
{
  return SapRange<ClientDataType,QueryType>(&mBoxes,queryObj,policy);
}

template <typename ClientDataType>
template <typename QueryType>
SapRange<ClientDataType,QueryType> Sap<ClientDataType>::Query(const QueryType& queryObj)
{
  return SapRange<ClientDataType,QueryType>(&mBoxes,queryObj);
}

template <typename ClientDataType>
SapPairRange<ClientDataType> Sap<ClientDataType>::QuerySelf()
{
  return SapPairRange<ClientDataType>(mPairManager);
}

template <typename ClientDataType>
void Sap<ClientDataType>::Clear()
{
  //Null out all of the box obj ptrs making sure to ignore the sentinel
  for(uint i = 1; i < mBoxes.Size(); ++i)
  {
    mBoxes[i].mObj = nullptr;
    mOpenIndices.PushBack(i);
  }

  //let's pretend the z axis is the one of most variance...
  BatchPairRemove<2>();

  //remove all deleted endpoints
  BatchEndPointRemove<0>();
  BatchEndPointRemove<1>();
  BatchEndPointRemove<2>();

  //clear all the pairs
  mPairManager->Clear();
}

template <typename ClientDataType>
void Sap<ClientDataType>::Draw(int level, uint debugDrawFlags)
{
  //Draw the axis
  Vec3 drawAxis(0, real(level), 0);
  gDebugDraw->Add(Debug::Line(drawAxis - Vec3(20,0,0), drawAxis + Vec3(20,0,0)).Color(Color::MediumAquamarine));
  gDebugDraw->Add(Debug::Line(drawAxis - Vec3(0,20,0), drawAxis + Vec3(0,20,0)).Color(Color::MediumAquamarine));
  gDebugDraw->Add(Debug::Line(drawAxis - Vec3(0,0,20), drawAxis + Vec3(0,0,20)).Color(Color::MediumAquamarine));

  typename Array<BoxType>::range r = mBoxes.All();
  while(!r.Empty())
  {
    if(r.Front().mObj == nullptr)
    {
      r.PopFront();
      continue;
    }

    Vec3 min = r.Front().mMins;
    Vec3 max = r.Front().mMaxs;

    //Draw the bounding box
    Vec3 center = (min + max) / real(2.0);
    Vec3 extents = (max - min) / real(2.0);
    Vec3 centerProjY(center.x, drawAxis.y, center.z);
    Vec3 extentsProjY(extents.x, real(0.0), extents.z);
    gDebugDraw->Add(Debug::Obb(center, extents).Color(Color::Aquamarine));
    gDebugDraw->Add(Debug::Obb(centerProjY, extentsProjY).Color(Color::Aquamarine));

    //x-axis
    Vec3 xMin(min.x,   real(0.0) + drawAxis.y, min.z);
    Vec3 xMin2(xMin.x, real(0.0), real(0.0));
    Vec3 xMax(max.x,   real(0.0) + drawAxis.y, min.z);
    Vec3 xMax2(xMax.x, real(0.0), real(0.0));
    gDebugDraw->Add(Debug::Line(xMin, xMin2 + drawAxis).Color(Color::Aquamarine));
    gDebugDraw->Add(Debug::Line(xMax, xMax2 + drawAxis).Color(Color::Aquamarine));

    //z-axis
    Vec3 zMin(max.x,      real(0.0) + drawAxis.y, min.z);
    Vec3 zMin2(real(0.0), real(0.0), zMin.z);
    Vec3 zMax(max.x,      real(0.0) + drawAxis.y, max.z);
    Vec3 zMax2(real(0.0), real(0.0), zMax.z);
    gDebugDraw->Add(Debug::Line(zMin, zMin2 + drawAxis).Color(Color::Aquamarine));
    gDebugDraw->Add(Debug::Line(zMax, zMax2 + drawAxis).Color(Color::Aquamarine));

    //y-axis
    Vec3 yMin(min.x,      min.y, min.z);
    Vec3 yMin2(real(0.0), min.y - drawAxis.y, real(0.0));
    Vec3 yMax(min.x,      max.y, min.z);
    Vec3 yMax2(real(0.0), max.y - drawAxis.y, real(0.0));
    gDebugDraw->Add(Debug::Line(yMin, yMin2 + drawAxis).Color(Color::Aquamarine));
    gDebugDraw->Add(Debug::Line(yMax, yMax2 + drawAxis).Color(Color::Aquamarine));

    r.PopFront();
  }
}

template <typename ClientDataType>
void Sap<ClientDataType>::Validate()
{
  for(uint i = 0; i < mBoxes.Size(); ++i)
  {
    BoxType& box = mBoxes[i];
    if(i == 0)
      ErrorIf(box.mObj != (void*)cSentinelPattern,"Sentinel object pointer has been modified");

    if(box.mObj == nullptr)
      continue;

    for(uint j = 0; j < 6; ++j)
    {
      uint axis = j / 2;
      uint index = i * 6 + j;
      uint axisIndex = mIndices[index];

      //validate the endpoints and indices match
      ErrorIf(axisIndex < 0 || axisIndex >= mAxes[axis].Size(), "Index in indices is invalid.");
      EndPointType& endPoint = mAxes[axis][axisIndex];
      ErrorIf(endPoint.GetIndex() != index,"Endpoint index does not point to its index.");
      ErrorIf(mBoxes[endPoint.GetIndex() / 6].mObj == nullptr,"Endpoint points to a deleted box");

      //validate that the sentinels are in the right spot and flagged min/max correctly
      if(i == 0)
      {
        //check the max
        if(j % 2)
        {
          ErrorIf(!endPoint.isMin(),"Sentinel max should be flagged as a min");
          ErrorIf(axisIndex != mAxes[axis].Size() - 1,"Sentinel max is not the last endpoint");
        }
        //check the min
        else
        {
          ErrorIf(endPoint.isMin(),"Sentinel min should be flagged as a max");
          ErrorIf(axisIndex != 0,"Sentinel min is not the first endpoint.");
        }
      }
      //validate normal endpoints
      else
      {
        //make sure the min/max flags are set correctly
        if(j % 2)
          ErrorIf(endPoint.isMin(),"Endpoint is a max but flagged as a min.");
        else
          ErrorIf(!endPoint.isMin(),"Endpoint is a min but flagged as a max.");

        //validate correct sorting
        EndPointType& leftEndpoint = mAxes[axis][axisIndex - 1];
        EndPointType& rightEndpoint = mAxes[axis][axisIndex + 1];
        ErrorIf(endPoint.mVal < leftEndpoint.mVal,
          "Endpoint of box %d is smaller than the box %d to the left.",
          endPoint.GetIndex() / 6, leftEndpoint.GetIndex() / 6);
        ErrorIf(endPoint.mVal > rightEndpoint.mVal,
          "Endpoint of box %d is greater than the box %d to the right.",
          endPoint.GetIndex() / 6, rightEndpoint.GetIndex() / 6);
      }
    }
  }
}

template <typename ClientDataType>
void Sap<ClientDataType>::Setup()
{
  /*HeapAllocator allocator(mHeap);
  mBoxes.SetAllocator(allocator);
  mIndices.SetAllocator(allocator);
  mOpenIndices.SetAllocator(allocator);
  mAxes[0].SetAllocator(allocator);  
  mAxes[1].SetAllocator(allocator);  
  mAxes[2].SetAllocator(allocator); */ 

  //Setting the initial size of the containers to account
  //for 300 boxes. This is just the value that I currently use
  //in my game.
  uint objectStartSize = 300;
  mBoxes.Reserve(objectStartSize);
  mIndices.Reserve(objectStartSize * 6);
  mAxes[0].Reserve(objectStartSize * 2);
  mAxes[1].Reserve(objectStartSize * 2);
  mAxes[2].Reserve(objectStartSize * 2);

  MakeSentinel();
}

template <typename ClientDataType>
void Sap<ClientDataType>::MakeSentinel()
{
  //push back the sentinel
  DataType sentinelData;
  sentinelData.mAabb.mMax = Vec3(Math::PositiveMax(),Math::PositiveMax(),Math::PositiveMax());
  sentinelData.mAabb.mMin = Vec3(-Math::PositiveMax(),-Math::PositiveMax(),-Math::PositiveMax());
  sentinelData.mBoundingSphere.mCenter = Vec3(0,0,0);
  sentinelData.mBoundingSphere.mRadius = Math::PositiveMax();


  BoxType sentinelBox(sentinelData);
  sentinelBox.mObj = (void*)SapInternal::cSentinelPattern;
  mBoxes.PushBack(sentinelBox);
  for(uint i = 0; i < 6; ++i)
    mIndices.PushBack(i % 2);

  for(uint i = 0; i < 3; ++i)
  {
    //to avoid boxes inserting pairs with the sentinel, mark the max
    //endpoint as a min and mark the min as a max
    EndPointType minEndpoint(i * 2,false,-Math::PositiveMax());
    EndPointType maxEndpoint(i * 2 + 1,true,Math::PositiveMax());

    mAxes[i].PushBack(minEndpoint);
    mAxes[i].PushBack(maxEndpoint);
  }
}

template <typename ClientDataType>
uint Sap<ClientDataType>::GetNewBoxIndex()
{
  uint index;
  //if there are no open indices
  if(mOpenIndices.Empty())
  {
    //add a new box and calculate its index
    mBoxes.PushBack();
    index = mBoxes.Size() - 1;

    //add 6 indices for the endpoints
    for(uint i = 0; i < 6; ++i)
      mIndices.PushBack(0);
  }
  //if there are open indices
  else
  {
    //get an available index
    index = mOpenIndices.Back();
    //remove that index since we are now using it
    mOpenIndices.PopBack();
  }

  return index;
}

template <typename ClientDataType>
uint Sap<ClientDataType>::GetBoxIndex(uint axis,uint index)
{
  ErrorIf(axis >= 3,"Accessing invalid axis");
  ErrorIf(index >= mAxes[axis].Size(),"Accessing invalid axis");
  return mAxes[axis][index].GetIndex() / 6;
}

template <typename ClientDataType>
real Sap<ClientDataType>::GetEndpointValue(uint axis,uint endpointIndex)
{
  ErrorIf(axis >= 3,"Accessing invalid axis");
  ErrorIf(endpointIndex >= mAxes[axis].Size(),"Accessing invalid axis");
  return mAxes[axis][endpointIndex].mVal;
}

template <typename ClientDataType>
uint Sap<ClientDataType>::GetIndicesIndex(uint axis,uint index)
{
  ErrorIf(axis >= 3,"Accessing invalid axis");
  ErrorIf(index >= mAxes[axis].Size(),"Accessing invalid axis");
  return mAxes[axis][index].GetIndex();
}

template <typename ClientDataType>
uint Sap<ClientDataType>::GetBoxMin(uint axis, uint boxIndex) const
{
  return boxIndex * 6 + 2 * axis;
}

template <typename ClientDataType>
uint Sap<ClientDataType>::GetBoxMax(uint axis, uint boxIndex) const
{
  return boxIndex * 6 + 2 * axis + 1;
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::RemoveEndPoints(uint boxNum)
{
  real halfFloatMax = Math::PositiveMax() / real(2.0);
  uint boxMin = GetBoxMin(Axis,boxNum);
  uint boxMax = GetBoxMax(Axis,boxNum);

  mAxes[Axis][mIndices[boxMax]].mVal = halfFloatMax * real(1.1);
  mAxes[Axis][mIndices[boxMin]].mVal = halfFloatMax;
  UpdateEndpointOnAxis<Axis>(mIndices[boxMax],true);
  UpdateEndpointOnAxis<Axis>(mIndices[boxMin],true);
  ShiftRight<Axis>(mIndices[boxMax],false);
  ShiftRight<Axis>(mIndices[boxMin],false);
  mAxes[Axis].PopBack();
  mAxes[Axis].PopBack();
}

template <typename ClientDataType>
void Sap<ClientDataType>::InsertPair(uint box1Index, uint box2Index)
{
  DataType& data1 = mBoxes[box1Index].mData;
  DataType& data2 = mBoxes[box2Index].mData;
  mPairManager->AddPair(data1,data2);
}

template <typename ClientDataType>
void Sap<ClientDataType>::RemovePair(uint box1Index, uint box2Index)
{
  DataType& data1 = mBoxes[box1Index].mData;
  DataType& data2 = mBoxes[box2Index].mData;
  mPairManager->RemovePair(data1,data2);
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::InsertWithSet(BoxSet& boxSet, uint boxIndex)
{
  BoxSet::range range = boxSet.All();
  for(;!range.Empty(); range.PopFront())
  {
    uint setIndex = range.Front();
    //if the boxes are not overlapping on the 
    //other axes, we don't care, continue
    if(!CheckRemainingAxesOverlap<Axis>(boxIndex,setIndex))
      continue;
    //otherwise Insert a pair from the pair manager
    InsertPair(boxIndex,setIndex);
  }
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::RemoveWithSet(BoxSet& boxSet, uint boxIndex)
{
  BoxSet::range range = boxSet.All();
  for(; !range.Empty(); range.PopFront())
  {
    uint setIndex = range.Front();
    //if the boxes are not overlapping on the 
    //other axes, we don't care, continue
    if(!CheckRemainingAxesOverlap<Axis>(boxIndex,setIndex))
      continue;
    //otherwise remove a pair from the pair manager
    RemovePair(boxIndex,setIndex);
  }
}

template <typename ClientDataType>
template <uint Axis>
uint Sap<ClientDataType>::BatchSort(EndPointArray& newEndpoints)
{
  //reverse sort the new endpoints
  Zero::Sort(newEndpoints.All(),greater<EndPointType>());

  EndPointArray& axis = mAxes[Axis];

  uint oldPos = axis.Size() - 1;
  axis.Resize(axis.Size() + newEndpoints.Size());
  uint insertIndex = axis.Size() - 1;

  //loop from the end to the front while we still have new endpoints to Insert
  Array<EndPointType>::range newRange = newEndpoints.All();
  while(!newRange.Empty())
  {
    //check to see which is larger between the old max endpoint
    //and the new max endpoint. Take whichever was larger and Insert
    //it at the Insert position then move to the next element.
    if(axis[oldPos] > newRange.Front())
    {
      axis[insertIndex] = axis[oldPos];
      --oldPos;
    }
    else
    {
      axis[insertIndex] = newRange.Front();
      newRange.PopFront();
    }

    //fix the index of the updated endpoint
    mIndices[axis[insertIndex].GetIndex()] = insertIndex;
    //move to the next Insert position
    --insertIndex;
  }

  //the last modified endpoint is 1 past where we left off
  return insertIndex + 1;
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::BatchPairAdd(uint startIndex)
{
  typedef HashSet<uint> BoxSet;
  BoxSet boxSet;
  BoxSet insertBoxSet;

  EndPointArray& axis = mAxes[Axis];
  //make sure to avoid the sentinel node
  uint lastEndPoint = axis.Size() - 2;
  for(uint i = lastEndPoint; i >= startIndex; --i)
  {
    EndPointType& endPoint = axis[i];
    uint boxIndex = endPoint.GetIndex() / 6;

    //if this box was just inserted
    if(mBoxes[boxIndex].mObj == nullptr)
    {
      //if a min, remove it from the correct set
      if(endPoint.isMin())
        insertBoxSet.Erase(boxIndex);
      else
      {
        //add pairs with objects that already existed
        InsertWithSet<Axis>(boxSet,boxIndex);
        //add pairs with objects that were just inserted
        InsertWithSet<Axis>(insertBoxSet,boxIndex);

        //add this box to the inserted set
        insertBoxSet.Insert(boxIndex);
      }
    }
    //this box was not just inserted
    else
    {
      //if a min, remove it from the correct set
      if(endPoint.isMin())
        boxSet.Erase(boxIndex);
      else
      {
        //only add pairs with objects just inserted
        InsertWithSet<Axis>(insertBoxSet,boxIndex);

        //add this box to the normal set
        boxSet.Insert(boxIndex);
      }
    }
  }
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::BatchPairRemove()
{
  EndPointArray& axis = mAxes[Axis];

  typedef HashSet<uint> BoxSet;
  BoxSet normalBoxes,removeBoxes;

  //make sure to ignore the sentinel
  uint size = axis.Size() - 1;
  for(uint i = 1; i < size; ++i)
  {
    EndPointType& endpoint = axis[i];
    uint boxIndex = endpoint.GetIndex() / 6;

    //if this object is being deleted
    if(mBoxes[boxIndex].mObj == nullptr)
    {
      if(endpoint.isMin())
      {
        //remove pairs with objects that already existed
        RemoveWithSet<Axis>(normalBoxes,boxIndex);
        //remove pairs with objects that are being deleted
        RemoveWithSet<Axis>(removeBoxes,boxIndex);

        removeBoxes.Insert(boxIndex);
      }
      else
        removeBoxes.Erase(boxIndex);
    }
    //if this object is not being deleted
    else
    {
      if(endpoint.isMin())
      {
        //only remove pairs with objects that are being deleted
        RemoveWithSet<Axis>(removeBoxes,boxIndex);

        normalBoxes.Insert(boxIndex);
      }
      else
        normalBoxes.Erase(boxIndex);
    }
  }
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::BatchEndPointRemove()
{
  EndPointArray& axis = mAxes[Axis];
  uint startIndex = 1;
  uint endIndex = axis.Size() - 1;
  uint insertPosition = startIndex;

  //loop from beginning to end, ignoring the sentinels
  for(uint i = startIndex; i < endIndex; ++i)
  {
    EndPointType& endpoint = axis[i];
    uint index = endpoint.GetIndex();
    uint boxIndex = index / 6;

    //if this object is not being deleted, we need to keep
    //it's endpoints. So copy it to the Insert position and then
    //advance the Insert position. (Don't forget to update the index value)
    if(mBoxes[boxIndex].mObj != nullptr)
    {
      axis[insertPosition] = axis[i];
      mIndices[index] = insertPosition;
      ++insertPosition;
    }
  }

  //move the sentinel to where we left off and fix it's index value
  axis[insertPosition] = axis[endIndex];
  mIndices[axis[insertPosition].GetIndex()] = insertPosition;
  //remove everything after the sentinel, we don't need them anymore
  axis.Resize(insertPosition + 1);
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::InsertEndpoint(BoxType& box, uint index)
{
  //get the min and max endpoint of this axis
  EndPointType minEndpoint(GetBoxMin(Axis,index),true,box.mMins[Axis]);
  EndPointType maxEndpoint(GetBoxMax(Axis,index),false,box.mMaxs[Axis]);
  //Insert the endpoint on this axis, but only update the
  //pair manager if this is the given index
  InsertEndpointOnAxis<Axis>(minEndpoint,Axis == 2);
  InsertEndpointOnAxis<Axis>(maxEndpoint,Axis == 2);
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::InsertEndpointOnAxis(const EndPointType& endpoint, bool finalAxis)
{   
  //get the index of the old last endpoint
  int i = mAxes[Axis].Size() - 1;
  //put the endpoint on the back of this axis
  mAxes[Axis].PushBack(endpoint);

  //shift pass the sentinel node
  ShiftLeft<Axis>(i + 1,false);
  --i;

  //shift this endpoint left until it is larger than the endpoint to its left
  while(mAxes[Axis][i].mVal > endpoint.mVal)
  {
    ShiftLeft<Axis>(i + 1,finalAxis);      
    --i;
  }

  //set its index the index of the endpoint now that it is in its final position
  mIndices[endpoint.GetIndex()] = i + 1;
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::UpdateEndpoint(uint index, DataType& data, bool finalAxis)
{
  //get the index in the indices list for the max endpoint
  uint maxIndex = GetBoxMax(Axis,index);
  //get the index in the indices list for the min endpoint
  uint minIndex = GetBoxMin(Axis,index);

  //we need to determine which direction the object is moving
  //and update the min and max accordingly. This is done to prevent
  //false pairs from fast moving objects and pairs with itself. This
  //also prevents an update from stopping a min on it's own max.

  real oldMin = mBoxes[index].mMins[Axis];
  real oldMax = mBoxes[index].mMaxs[Axis];
  real newMin = data.mAabb.mMin[Axis];
  real newMax = data.mAabb.mMax[Axis];
  uint endPoint1,endPoint2;

  //if both endpoints are moving to the left, update min then max
  if(newMin < oldMin && newMax < oldMax)
  {
    //sort min then max
    endPoint1 = minIndex;
    endPoint2 = maxIndex;
  }
  //otherwise they are both moving to the right or in opposite directions,
  //so just update them max then min
  else
  {
    //sort max then min
    endPoint1 = maxIndex;
    endPoint2 = minIndex;
  }

  //update box values for this index
  mBoxes[index].UpdateBox(Axis,data);

  //update the endpoint values
  mAxes[Axis][mIndices[maxIndex]].mVal = mBoxes[index].mMaxs[Axis];
  mAxes[Axis][mIndices[minIndex]].mVal = mBoxes[index].mMins[Axis];

  //sort the endpoints in the appropriate order 
  //(determined by the direction the box is moving)
  UpdateEndpointOnAxis<Axis>(mIndices[endPoint1],true);
  UpdateEndpointOnAxis<Axis>(mIndices[endPoint2],true);
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::UpdateEndpointOnAxis(uint index, bool finalAxis)
{
  //while this index is in the valid range and is greater than the next
  //value, shift it right until it finds the right spot
  while(GetEndpointValue(Axis,index) > GetEndpointValue(Axis,index + 1))
  {
    ErrorIf(index >= mAxes[Axis].Size() - 1,"Sentinels failed, endpoint made it to a sentinel");
    ShiftRight<Axis>(index,finalAxis);
    ++index;
  }

  //while this index is in the valid range and is less than the previous 
  //value, shift it left until it finds the right spot
  while(GetEndpointValue(Axis,index) < GetEndpointValue(Axis,index - 1))
  {
    ErrorIf(index <= 0,"Sentinels failed, endpoint made it to a sentinel");
    ShiftLeft<Axis>(index,finalAxis);
    --index;
  }

  //The reason that both loops are performed is for ease of reading and slight
  //speed performance. There is no reason to check if left or right shifting 
  //is in order since the first part of the while loop will be the same as 
  //the if statement.
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::ShiftLeft(uint index, bool finalAxis)
{
  //swap the endpoint with the one to the left of it
  Swap(mAxes[Axis][index],mAxes[Axis][index - 1]);
  //update the indices of both endpoints
  ++mIndices[GetIndicesIndex(Axis,index)];
  --mIndices[GetIndicesIndex(Axis,index - 1)];

  //if this is the final axis sorted, check "collision" for the pair manager
  if(!finalAxis)
    return;

  uint box1 = GetBoxIndex(Axis,index - 1);
  uint box2 = GetBoxIndex(Axis,index);
  EndPointType& newLeft = mAxes[Axis][index - 1];
  EndPointType& newRight = mAxes[Axis][index];

  //a min became less than a max, maybe add pair depending on if the
  //other axes have overlap
  if(newLeft.isMin() && !newRight.isMin())
  {
    //check all the other axes for overlap
    if(CheckRemainingAxesOverlap<Axis>(box1,box2))
      InsertPair(box1,box2);
  }
  //a max became less than a min, remove pair
  else if(!newLeft.isMin() && newRight.isMin())
  {
    //Sap would work without these two checks (because hash map simply
    //wouldn't do anything on a double remove), however we are keeping
    //track of a reference count for Multi-Sap, so it is currently necessary.
    if(CheckRemainingAxesOverlap<Axis>(box1,box2))
      RemovePair(box1,box2);
  }
}

template <typename ClientDataType>
template <uint Axis>
void Sap<ClientDataType>::ShiftRight(uint index, bool finalAxis)
{
  //swap the endpoint with the one to the right of it
  Swap(mAxes[Axis][index],mAxes[Axis][index + 1]);
  //update the indices of both endpoints
  --mIndices[GetIndicesIndex(Axis,index)];
  ++mIndices[GetIndicesIndex(Axis,index + 1)];

  //if this is the final axis sorted, check "collision" for the pair manager
  if(!finalAxis)
    return;

  uint box1 = GetBoxIndex(Axis,index + 1);
  uint box2 = GetBoxIndex(Axis,index);
  EndPointType& newLeft = mAxes[Axis][index];
  EndPointType& newRight = mAxes[Axis][index + 1];

  //if a min became greater than a max, remove pair
  if(newRight.isMin() && !newLeft.isMin())
  {
    //Same as the left shift with regards to checking before removing.
    if(CheckRemainingAxesOverlap<Axis>(box1,box2))
      RemovePair(box1,box2);
  }
  //if a max became greater than a min, maybe add pair depending on if the
  //other axes have overlap
  else if(!newRight.isMin() && newLeft.isMin())
  {
    //check all the other axes for overlap
    if(CheckRemainingAxesOverlap<Axis>(box1,box2))
      InsertPair(box1,box2);
  }
}

template <typename ClientDataType>
template <uint Axis>
bool Sap<ClientDataType>::CheckAxisOverlap(uint box1, uint box2)const
{
  //get the min of each box
  const uint box1Min = GetBoxMin(Axis,box1);
  const uint box2Min = GetBoxMin(Axis,box2);

  //if box1 min is greater than box2 max, no intersection
  if(mIndices[box1Min] > mIndices[box2Min + 1])
    return false;
  //if box2 min is greater than box1 max, no intersection 
  if(mIndices[box2Min] > mIndices[box1Min + 1])
    return false;

  //otherwise there is an overlap
  return true;
}

template <typename ClientDataType>
template <uint Axis>
bool Sap<ClientDataType>::CheckRemainingAxesOverlap(uint box1, uint box2) const
{
  //Check the other two axes. The compiler should optimize the axis
  //calculation since these are compile time constants.
  if(CheckAxisOverlap<(Axis + 1) % 3>(box1,box2))
  {
    if(CheckAxisOverlap<(Axis + 2) % 3>(box1,box2))
      return true;
  }
  return false;
}

}//namespace Zero
