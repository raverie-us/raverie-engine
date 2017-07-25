///////////////////////////////////////////////////////////////////////////////
///
/// \file BoolVector3.hpp
/// Declaration of the BoolVector3 structure.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"

namespace Math
{

struct BoolVector3;
typedef BoolVector3 BoolVec3;
typedef const BoolVector3& BoolVec3Param;
typedef BoolVector3& BoolVec3Ref;
typedef BoolVector3* BoolVec3Ptr;

///2 dimensional integral vector.
struct ZeroShared BoolVector3
{
  BoolVector3(void) {}
  explicit BoolVector3(bool x, bool y, bool z);

  bool& operator[](uint index);
  bool operator[](uint index) const;

  //Unary Operators
  BoolVector3 operator!(void) const;

  //Binary Vector Comparisons
  bool operator==(BoolVec3Param rhs) const;
  bool operator!=(BoolVec3Param rhs) const;

  ///Set all of the values of this vector at once.
  void Set(bool x, bool y, bool z);

  union
  {
    struct  
    {
      bool x, y, z;
    };
    bool array[3];
  };

  static const BoolVector3 cZero;
  static const BoolVector3 cXAxis;
  static const BoolVector3 cYAxis;
  static const BoolVector3 cZAxis;
};

}// namespace Math
