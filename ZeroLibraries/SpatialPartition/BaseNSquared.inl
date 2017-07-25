///////////////////////////////////////////////////////////////////////////////
///
/// \file BaseNSquared.inl
/// Implementation of the BaseNSquared class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

namespace Zero
{

template <typename ClientDataType>
BaseNSquared<ClientDataType>::BaseNSquared()
{
  /*HeapAllocator allocator(mHeap);

  mData.SetAllocator(allocator);
  mDataPairs.SetAllocator(allocator);*/
}

template <typename ClientDataType>
BaseNSquared<ClientDataType>::~BaseNSquared()
{

}

template <typename ClientDataType>
void BaseNSquared<ClientDataType>::Serialize(Serializer& stream)
{
  //nothing to serialize here...
}

template <typename ClientDataType>
void BaseNSquared<ClientDataType>::CreateProxy(BroadPhaseProxy& proxy, DataType& data)
{
  uint index = GetNewProxyIndex();
  mData[index].mData = data;
  mData[index].mValid = true;
  proxy = BroadPhaseProxy((u32)index);
}

template <typename ClientDataType>
void BaseNSquared<ClientDataType>::RemoveProxy(BroadPhaseProxy& proxy)
{
  uint index = proxy.ToU32();
  mData[index].mValid = false;
  mFreeIndices.PushBack(index);
}

template <typename ClientDataType>
void BaseNSquared<ClientDataType>::UpdateProxy(BroadPhaseProxy& proxy, DataType& data)
{
  uint index = proxy.ToU32();
  ErrorIf(mData[index].mValid == false,"Updating an invalid proxy.");
  mData[index].mData = data;
}

template <typename ClientDataType>
uint BaseNSquared<ClientDataType>::GetNewProxyIndex()
{
  //if there were no free indices already, add a new item to the back
  //and return that index
  if(mFreeIndices.Empty())
  {
    mData.PushBack();
    return mData.Size() - 1;
  }

  //otherwise, just return an index in free indices
  uint index = mFreeIndices.Back();
  mFreeIndices.PopBack();
  return index;
}

}//namespace Zero
