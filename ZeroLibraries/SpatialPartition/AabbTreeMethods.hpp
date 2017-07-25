///////////////////////////////////////////////////////////////////////////////
///
/// \file AabbTreeMethods.hpp
/// Declaration of the helper functions used by Aabb Trees in order to
/// cut down on code re-use.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum3(PartitionMethods, MinimizeVolumeSum, 
                               MinimuzeSurfaceAreaSum, MidPoint);



//-----------------------------------------------------Tree Construction Methods
template <typename NodeType>
class PartitionNodeMethod
{
public:
  typedef uint (*PartitionNodeAxisMethod)(Array<NodeType*>&);
};

///Builds an Aabb tree using the passed in partition axis method.
///NOTE*  ObjectType must have a public Aabb member named mAabb.
template <typename NodeType>
NodeType* BuildTreeTopDownNodes(Array<NodeType*>& leafNodes,
  typename PartitionNodeMethod<NodeType>::PartitionNodeAxisMethod partitionMethod);

///Calculates an Aabb encompassing all objects passed in.
template <typename NodeType>
Aabb CalculateAabbNodes(Array<NodeType*>& leafNodes);

///Partition axis method that optimizes object collision test speed.
template <typename NodeType>
uint MinimizeVolumeSumNodes(Array<NodeType*>& leafNodes);

///Partition axis method that optimizes ray cast tests.
template <typename NodeType>
uint MinimizeSurfaceAreaSumNodes(Array<NodeType*>& leafNodes);

///Simple partition axis method.
template <typename NodeType>
uint MidPointNodes(Array<NodeType*>& leafNodes);

//-------------------------------------Old functions (still used, can't remove)
template <typename ObjectType>
class PartitionMethod
{
public:
  typedef uint (*PartitionAxisMethod)(Array<ObjectType>&);
};

///Builds an Aabb tree using the passed in partition axis method.
///NOTE*  ObjectType must have a public Aabb member named mAabb.
template <typename NodeType, typename ObjectType>
NodeType* BuildTreeTopDown(Array<ObjectType>& proxies,
     typename PartitionMethod<ObjectType>::PartitionAxisMethod partitionMethod);

///Calculates an Aabb encompassing all objects passed in.
template <typename ObjectType>
Aabb CalculateAabb(Array<ObjectType>& proxies);

///Partition axis method that optimizes object collision test speed.
template <typename ObjectType>
uint MinimizeVolumeSum(Array<ObjectType>& proxies);

///Partition axis method that optimizes ray cast tests.
template <typename ObjectType>
uint MinimizeSurfaceAreaSum(Array<ObjectType>& proxies);

///Simple partition axis method.
template <typename ObjectType>
uint MidPoint(Array<ObjectType>& proxies);

//-----------------------------------------------------------------Ray Functions
///Callbacks for simple tests to see if we should traverse further 
///down the tree.  Returns the time of collision only for ray casts.
template <typename NodeType>
bool RayNodeTest(NodeType* tree, CastDataParam castData, real& t);

template <typename NodeType>
bool SegmentNodeTest(NodeType* tree, CastDataParam castData, real& t);

//---------------------------------------------------------Volume Test Functions
///Callbacks for simple tests to see if we should traverse further 
///down the tree.  Returns the time of collision only for ray casts.
template <typename NodeType>
bool AabbNodeTest(NodeType* tree, CastDataParam castData, real& t);

template <typename NodeType>
bool SphereNodeTest(NodeType* tree, CastDataParam castData, real& t);

template <typename NodeType>
bool FrustumNodeTest(NodeType* tree, CastDataParam castData, real& t);

#include "SpatialPartition/AabbTreeMethods.inl"

}//namespace Zero
