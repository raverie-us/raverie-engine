///////////////////////////////////////////////////////////////////////////////
///
/// \file BaseNSquared.hpp
/// Declaration of the BaseNSquared class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///The most basic BroadPhase that could ever be implemented. Serves no real
///purpose other than stubbing and very basic time comparisons.
template <typename ClientDataType>
class BaseNSquared
{
public:
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef BaseDataArrayObject<ClientDataType> ArrayObjectType;
  typedef Array<ArrayObjectType> ObjectArray;

  BaseNSquared();
  ~BaseNSquared();

  void Serialize(Serializer& stream);

  ///Inserts the given data and fills out the proxy for future operations on data.
  void CreateProxy(BroadPhaseProxy& proxy, DataType& data);
  ///Removes the object that the given proxy points to.
  void RemoveProxy(BroadPhaseProxy& proxy);
  ///Updates the data that the proxy points to with the new data.
  void UpdateProxy(BroadPhaseProxy& proxy, DataType& data);

protected:
  uint GetNewProxyIndex();

  ObjectArray mData;
  //change to a deque!!
  Array<uint> mFreeIndices;
};

}//namespace Zero

#include "SpatialPartition/BaseNSquared.inl"
