///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleCgPolicies.hpp
/// Declaration of the SimpleCgPolicy.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Reals.hpp"

namespace Math
{

/// An example policy for conjugate gradient.
/// Shows the required functionality for a new policy.
struct SimpleCgPolicy
{
  Vec3 Add(Vec3Param lhs, Vec3Param rhs)
  {
    return lhs + rhs;
  }

  Vec3 Subtract(Vec3Param lhs, Vec3Param rhs)
  {
    return lhs - rhs;
  }

  real Dot(Vec3Param lhs, Vec3Param rhs)
  {
    return Math::Dot(lhs,rhs);
  }

  Vec3 Scale(Vec3Param lhs, real rhs)
  {
    return lhs * rhs;
  }

  Vec3 Transform(Mat3Param mat, Vec3Param vec)
  {
    return Math::Transform(mat,vec);
  }

  //v1 * scalar + v2
  //out is assumed to only ever be aliased as v2
  void MultiplyAdd(Vec3Param v1, real scalar, Vec3Param v2, Vec3Ptr out)
  {
    *out = v2 + v1 * scalar;
  }

  //-(v1 * scalar - v2) = v2 - v1 * scalar
  //out is assumed to only ever be aliased as v2
  void MultiplySubtract(Vec3Param v1, real scalar, Vec3Param v2, Vec3Ptr out)
  {
    *out = v2 - v1 * scalar;
  }

  //-(mat * v1 - v2) = v2 - mat * v1
  //out is assumed to only ever be aliased as v2
  void NegativeTransformSubtract(Mat3Param mat, Vec3Param v1, Vec3Param v2, Vec3Ptr out)
  {
    *out = v2 - Math::Transform(mat,v1);
  }
};

}//namespace Math
