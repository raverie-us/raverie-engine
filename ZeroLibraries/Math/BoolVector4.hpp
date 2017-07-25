///////////////////////////////////////////////////////////////////////////////
///
/// \file BoolVector4.hpp
/// Declaration of the BoolVector4 structure.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"

namespace Math
{

struct BoolVector4;
typedef BoolVector4 BoolVec4;
typedef const BoolVector4& BoolVec4Param;
typedef BoolVector4& BoolVec4Ref;
typedef BoolVector4* BoolVec4Ptr;

///2 dimensional integral vector.
struct ZeroShared BoolVector4
{
  BoolVector4(void) {}
  explicit BoolVector4(bool x, bool y, bool z, bool w);

  bool& operator[](uint index);
  bool operator[](uint index) const;

  //Unary Operators
  BoolVector4 operator!(void) const;

  //Binary Vector Comparisons
  bool operator==(BoolVec4Param rhs) const;
  bool operator!=(BoolVec4Param rhs) const;

  ///Set all of the values of this vector at once.
  void Set(bool x, bool y, bool z, bool w);

  union
  {
    struct  
    {
      bool x, y, z, w;
    };
    bool array[4];
  };

  static const BoolVector4 cZero;
  static const BoolVector4 cXAxis;
  static const BoolVector4 cYAxis;
  static const BoolVector4 cZAxis;
  static const BoolVector4 cWAxis;
};

}// namespace Math
