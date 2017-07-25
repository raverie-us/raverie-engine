///////////////////////////////////////////////////////////////////////////////
///
/// \file NSquared.hpp
/// Declaration of the BaseNSquared, NSquaredRange
/// and NSquaredPairRange classes.
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///A range for iterating through the cast results of the NSquared BroadPhase.
///This range becomes invalidated when the BroadPhase is changed.
template <typename ClientDataType>
struct NSquaredRange : public BroadPhaseArrayRange<ClientDataType>
{
  typedef BroadPhaseArrayRange<ClientDataType> BaseType;

  NSquaredRange(typename BaseType::ObjectArray* data) : BaseType(data)
  {
  }
};

///A range for iterating through the self pairs of the NSquared BroadPhase.
///This range becomes invalidated when the BroadPhase is changed.
template <typename ClientDataType>
struct NSquaredPairRange : public BroadPhaseArrayPairRange<ClientDataType>
{
  typedef BroadPhaseArrayPairRange<ClientDataType> BaseType;

  NSquaredPairRange(typename BaseType::ObjectArray* data) : BaseType(data)
  {
  }
};

///The most basic BroadPhase that could ever be implemented. Serves no real
///purpose other than stubbing and very basic time comparisons.
template <typename ClientDataType>
class NSquared : public BaseNSquared<ClientDataType>
{
public:

  typedef BaseNSquared<ClientDataType> BaseType;

  typedef NSquaredPairRange<ClientDataType> PairRange;

  using BaseType::mData;

  NSquared() {}
  ~NSquared() {}

  ///Returns a range to iterate through any cast on this BroadPhase. No
  ///cast parameters are specified since this is the NSquared BroadPhase.
  NSquaredRange<ClientDataType> Query()
  {
    return NSquaredRange<ClientDataType>(&mData);
  }

  ///Returns a range to self pair intersections.
  PairRange QueryPairRange()
  {
     return PairRange(&mData);
  }
};

}//namespace Zero
