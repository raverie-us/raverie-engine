///////////////////////////////////////////////////////////////////////////////
///
/// \file BoolVector2.hpp
/// Declaration of the BoolVector2 structure.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"

namespace Math
{

struct BoolVector2;
typedef BoolVector2 BoolVec2;
typedef const BoolVector2& BoolVec2Param;
typedef BoolVector2& BoolVec2Ref;
typedef BoolVector2* BoolVec2Ptr;

///2 dimensional integral vector.
struct ZeroShared BoolVector2
{
  BoolVector2(void) {}
  explicit BoolVector2(bool x, bool y);

  bool& operator[](uint index);
  bool operator[](uint index) const;

  //Unary Operators
  BoolVector2 operator!(void) const;

  //Binary Vector Comparisons
  bool operator==(BoolVec2Param rhs) const;
  bool operator!=(BoolVec2Param rhs) const;

  ///Set all of the values of this vector at once.
  void Set(bool x, bool y);

  union
  {
    struct  
    {
      bool x, y;
    };
    bool array[2];
  };

  static const BoolVector2 cZero;
  static const BoolVector2 cXAxis;
  static const BoolVector2 cYAxis;
};

}// namespace Math
