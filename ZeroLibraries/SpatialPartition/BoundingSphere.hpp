/////////////////////////////////////////////////////////////////////////////////
///
/// \file BoundingSphere.hpp
/// Declaration of the BoundingSphere, BoundingSphereRange
///   and BoundingSpherePairRange classes.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A range for iterating through the cast results of the BoundingSphere BroadPhase.
/// This range is templated to check the Spheres against the provided QueryType
/// using the provided policy.
/// This range becomes invalidated when the BroadPhase is changed.
template <typename ClientDataType, typename QueryType, typename PolicyType = BroadPhasePolicy<QueryType, Sphere> >
struct BoundingSphereRange : public BroadPhaseArrayRange<ClientDataType, QueryType, PolicyType, SphereQueryCheck<ClientDataType, QueryType, PolicyType> >
{
  typedef SphereQueryCheck<ClientDataType, QueryType, PolicyType> QueryCheck;
  typedef BroadPhaseArrayRange<ClientDataType, QueryType, PolicyType, QueryCheck> BaseType;

  /// Constructs a range using the default Policy.
  BoundingSphereRange(typename BaseType::ObjectArray* data, const QueryType& queryObj)
    : BaseType(data,queryObj,PolicyType())
  {
  }

  /// Constructs a range using the policy type passed in.
  BoundingSphereRange(typename BaseType::ObjectArray* data, const QueryType& queryObj, PolicyType policy)
    : BaseType(data,queryObj,policy)
  {
  }
};

/// A range for iterating through the self pairs of the BoundingSphere BroadPhase.
/// This range becomes invalidated when the BroadPhase is changed.
template <typename ClientDataType>
struct BoundingSpherePairRange : public BroadPhaseArrayPairRange<ClientDataType, SphereQueryPairCheck<ClientDataType> >
{
  typedef BroadPhaseArrayPairRange<ClientDataType, SphereQueryPairCheck<ClientDataType> > BaseType;

  BoundingSpherePairRange(typename BaseType::ObjectArray* data)
    : BaseType(data)
  {
  }
};

/// A basic BroadPhase that does NSquared Bounding Sphere checks. Used mostly as
/// a comparison or for a real quick and dirty BroadPhase.
template <typename ClientDataType>
class BoundingSphere : public BaseNSquared<ClientDataType>
{
public:
  typedef BoundingSpherePairRange<ClientDataType> PairRange;
  typedef BaseNSquared<ClientDataType> BaseType;

  using BaseType::mData;

  BoundingSphere() {};
  ~BoundingSphere() {};

  /// Returns a range to iterate through any cast on this BroadPhase. The policy
  /// determines if the queryObj and a Sphere overlap through a function called
  /// Overlap.
  template <typename QueryType, typename Policy>
  BoundingSphereRange<ClientDataType,QueryType,Policy> 
    QueryWithPolicy(const QueryType& queryObj, Policy policy)
  {
    return BoundingSphereRange<ClientDataType,QueryType,Policy>(&mData, queryObj,policy);
  }

  /// Returns a range to iterate through any cast on this BroadPhase. The policy
  /// is defaulted to BroadPhasePolicy<QueryType, Sphere>. This assumes that there
  /// is a Overlap function in the extended intersection for this test.
  template <typename QueryType>
  BoundingSphereRange<ClientDataType,QueryType>
    Query(const QueryType& queryObj)
  {
      BroadPhasePolicy<QueryType,Sphere> policy;
      return QueryWithPolicy(queryObj,policy);
  }

  /// Returns a range to self pair intersections.
  PairRange QueryPairRange()
  {
    return PairRange(&mData);
  }
};

}//namespace Zero

