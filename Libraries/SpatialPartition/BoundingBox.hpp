/////////////////////////////////////////////////////////////////////////////////
///
/// \file BoundingBox.hpp
/// Declaration of the BoundingBox, BoundingBoxRange and
/// BoundingBoxPairRange classes.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A range for iterating through the cast results of the BoundingBox BroadPhase.
/// This range is templated to check the Aabbs against the provided QueryType
/// using the provided policy.
/// This range becomes invalidated when the BroadPhase is changed.
template <typename ClientDataType, typename QueryType, typename PolicyType = BroadPhasePolicy<QueryType, Aabb> >
struct BoundingBoxRange : public BroadPhaseArrayRange<ClientDataType, QueryType, PolicyType, BoxQueryCheck<ClientDataType, QueryType, PolicyType> >
{
  typedef BoxQueryCheck<ClientDataType, QueryType, PolicyType> QueryCheck;
  typedef BroadPhaseArrayRange<ClientDataType, QueryType, PolicyType, QueryCheck> BaseType;

  ///Constructs a range using the default Policy.
  BoundingBoxRange(typename BaseType::ObjectArray* data, const QueryType& queryObj)
    : BaseType(data,queryObj,PolicyType())
  {
  }

  ///Constructs a range using the policy type passed in.
  BoundingBoxRange(typename BaseType::ObjectArray* data, const QueryType& queryObj, PolicyType policy)
    : BaseType(data,queryObj,policy)
  {

  }
};

///A range for iterating through the self pairs of the BoundingBox BroadPhase.
///This range becomes invalidated when the BroadPhase is changed.
template <typename ClientDataType>
struct BoundingBoxPairRange : public BroadPhaseArrayPairRange<ClientDataType, BoxQueryPairCheck<ClientDataType> >
{
  typedef BroadPhaseArrayPairRange<ClientDataType, BoxQueryPairCheck<ClientDataType> > BaseType;

  BoundingBoxPairRange(typename BaseType::ObjectArray* data)
    : BaseType(data) 
  {
  }
};

/// A basic BroadPhase that does NSquared Bounding Box checks. Used mostly as
/// a comparison or for a real quick and dirty BroadPhase.
template <typename ClientDataType>
class BoundingBox : public BaseNSquared<ClientDataType>
{
public:
  typedef BaseNSquared<ClientDataType> BaseType;
  typedef BoundingBoxPairRange<ClientDataType> PairRange;

  using BaseType::mData;

  BoundingBox() {}
  ~BoundingBox() {}

  /// Returns a range to iterate through any cast on this BroadPhase.
  /// The policy determines if the queryObj and an Aabb overlap
  /// through a function called Overlap.
  template <typename QueryType, typename Policy>
  BoundingBoxRange<ClientDataType,QueryType,Policy> 
    QueryWithPolicy(const QueryType& queryObj, Policy policy)
  {
    return BoundingBoxRange<ClientDataType,QueryType,Policy>(&mData, queryObj,policy);
  }

  /// Returns a range to iterate through any cast on this BroadPhase. The
  /// policy is defaulted to BroadPhasePolicy<QueryType, Aabb>. This assumes
  /// that there is a Overlap function in the extended intersection for this
  /// test.
  template <typename QueryType>
  BoundingBoxRange<ClientDataType,QueryType>
    Query(const QueryType& queryObj)
  {
    BroadPhasePolicy<QueryType,Aabb> policy;
    return QueryWithPolicy(queryObj,policy);
  }

  /// Returns a range to self pair intersections.
  PairRange QueryPairRange()
  {
    return PairRange(&mData);
  }
};

}//namespace Zero

