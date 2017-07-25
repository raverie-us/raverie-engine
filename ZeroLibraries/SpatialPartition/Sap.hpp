///////////////////////////////////////////////////////////////////////////////
///
/// \file Sap.hpp
/// Declaration of the Sap class.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///This is the Sap (Sweep and Prune) broad phase structure. Sap is a BroadPhase
///that is very efficient for lots of moving objects scattered around a world.
///Sap has two main failing points: If a large number of objects cluster on
///one axis, Sap starts performing poorly. The other issue is Sap is
///fairly inefficient when it comes to any form of casting.
template <typename ClientDataType>
class Sap
{
public:
  typedef ClientDataType ClientDataTypeDef;
  typedef SapBox<ClientDataType> BoxType;
  typedef SapPairManager<ClientDataType> PairManagerType;
  typedef SapEndPoint EndPointType;
  typedef BaseBroadPhaseData<ClientDataType> DataType;
  typedef BaseBroadPhaseObject<ClientDataType> DataObjectType;
  typedef Array<DataObjectType> DataObjectArray;
  typedef Array<EndPointType> EndPointArray;
  typedef HashSet<uint> BoxSet;

  Sap();
  Sap(PairManagerType* pairManager);
  ~Sap();

  void CreateProxy(BroadPhaseProxy& proxy, DataType& data);
  void CreateProxies(DataObjectArray& objects);
  void RemoveProxy(BroadPhaseProxy& proxy);
  void RemoveProxies(ProxyHandleArray& proxies);
  void UpdateProxy(BroadPhaseProxy& proxy, DataType& data);
  void UpdateProxies(DataObjectArray& objects);

  ///A range for performing a re-entrant cast into Sap. A policy must be
  ///provided that tells the range how to collide an Aabb with a QueryType
  ///through a function called Overlap. Most implementations should just
  ///call Query which uses the policy BroadPhasePolicy<QueryType,Aabb>.
  template <typename QueryType, typename PolicyType>
  SapRange<ClientDataType,QueryType> QueryWithPolicy(const QueryType& queryObj, PolicyType policy);

  ///The same functionality as the QueryWithPolicy function except
  ///the Policy is implied from the QueryType. The policy will be
  ///defaulted to BroadPhasePolicy<QueryType,Aabb>.
  template <typename QueryType>
  SapRange<ClientDataType,QueryType> Query(const QueryType& queryObj);

  ///Returns a range to self pair intersections.
  SapPairRange<ClientDataType> QuerySelf();

  void Clear();

  void Draw(int level, uint debugDrawFlags);

  //Makes sure everything is correct. Slow!
  void Validate();

private:

  //setup and creation

  void Setup();
  void MakeSentinel();
  uint GetNewBoxIndex();
  
  //helpers

  uint GetBoxIndex(uint axis,uint index);
  real GetEndpointValue(uint axis,uint endpointIndex);
  uint GetIndicesIndex(uint axis,uint index);
  //Gets the index of a min endpoint for a given box on a given axis.
  uint GetBoxMin(uint axis, uint boxIndex) const;
  //Gets the index of a max endpoint for a given box on a given axis.
  uint GetBoxMax(uint axis, uint boxIndex) const;


  //batch operations

  template <uint Axis> void RemoveEndPoints(uint boxNum);
  void InsertPair(uint box1Index, uint box2Index);
  void RemovePair(uint box1Index, uint box2Index);
  ///Adds a pair between boxIndex and everything in the set if the other axes overlap.
  template <uint Axis> void InsertWithSet(BoxSet& boxSet, uint boxIndex);
  ///Removes a pair between boxIndex and everything in the set if the other axes overlap.
  template <uint Axis> void RemoveWithSet(BoxSet& boxSet, uint boxIndex);
  ///Sorts the new endpoints into the old endpoints. Does not add pairs. 
  ///Returns the last position that was iterated to for sorting. Call for each axis.
  template <uint Axis> uint BatchSort(EndPointArray& newEndpoints);
  ///On one axis only, from a given start index, adds all of the pairs that were
  ///just generated from the batch sort phase.
  template <uint Axis> void BatchPairAdd(uint startIndex);
  ///On one axis only, removes the pairs that involve objects 
  ///being deleted (where Box obj ptr equals null)
  template <uint Axis> void BatchPairRemove();
  ///Removes the endpoints that are being deleted while preserving order
  ///of the old endpoints. Call for each axis.
  template <uint Axis> void BatchEndPointRemove();

  //internal endpoint updates

  //Given a box and its index, create its endpoints 
  //and Insert them on the provided axis.
  template <uint Axis> void InsertEndpoint(BoxType& box, uint index);
  //Inserts an endpoint onto a given axis but only checks for overlaps if this is
  //the final axis.
  template <uint Axis> void InsertEndpointOnAxis(const EndPointType& endpoint, bool finalAxis);
  //Updates the endpoint values and then shifts them to the correct spot.
  template <uint Axis> void UpdateEndpoint(uint index, DataType& data, bool finalAxis);
  //Shifts an endpoint to the left or right until it is in the correct position.
  template <uint Axis> void UpdateEndpointOnAxis(uint index, bool finalAxis);
  //Shifts an endpoint to the left and inserts/removes pairs if this is the final axis.
  template <uint Axis> void ShiftLeft(uint index, bool finalAxis);
  //Shifts an endpoint to the right and inserts/removes pairs if this is the final axis.
  template <uint Axis> void ShiftRight(uint index, bool finalAxis);
  //Checks to see if there is overlap on an axis for two given boxes.
  template <uint Axis> bool CheckAxisOverlap(uint box1, uint box2) const;
  //Checks the other two axes passed in to see if there is an overlap.
  template <uint Axis> bool CheckRemainingAxesOverlap(uint box1, uint box2) const;

  //Determines whether or not it is in a multi sap or not.
  bool mStandAlone;

  PairManagerType* mPairManager;

  Array<BoxType> mBoxes;
  //Has 6 indices for every box. Each index is where to find
  //the corresponding min/max for an endpoint on its axis.
  //The exact layout is minX,maxX,minY,maxY,minZ,maxZ
  Array<uint> mIndices;
  //Stores the available slots from delete boxes.
  Array<uint> mOpenIndices;
  //Where the endpoints are stored for each axis.
  EndPointArray mAxes[3];  
};

}//namespace Zero

#include "SpatialPartition/Sap.inl"
