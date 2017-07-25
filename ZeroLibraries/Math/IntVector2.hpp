///////////////////////////////////////////////////////////////////////////////
///
/// \file IntVector2.hpp
/// Declaration of the IntVector2 structure.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"
#include "Math/BoolVector2.hpp"

namespace Math
{

struct IntVector2;
typedef IntVector2 IntVec2;
typedef const IntVector2& IntVec2Param;
typedef IntVector2& IntVec2Ref;
typedef IntVector2* IntVec2Ptr;

///2 dimensional integral vector.
struct ZeroShared IntVector2
{
  IntVector2(void) {}
  explicit IntVector2(int x, int y);

  int& operator[](uint index);
  int operator[](uint index) const;

  //Unary Operators
  IntVector2 operator-(void) const;

  //Binary Assignment Operators (integers)
  void operator*=(int rhs);
  void operator/=(int rhs);

  //Binary Operators (integers)
  IntVector2 operator*(int rhs) const;
  IntVector2 operator/(int rhs) const;
  IntVector2 operator%(int rhs) const;

  //Unary operators
  IntVector2& operator++();
  IntVector2& operator--();

  //Binary Assignment Operators (vectors)
  void operator+=(IntVec2Param rhs);
  void operator-=(IntVec2Param rhs);
  void operator*=(IntVec2Param rhs);
  void operator/=(IntVec2Param rhs);

  //Binary Operators (vectors)
  IntVector2 operator+(IntVec2Param rhs) const;
  IntVector2 operator-(IntVec2Param rhs) const;

  //Binary Vector Comparisons
  bool operator==(IntVec2Param rhs) const;
  bool operator!=(IntVec2Param rhs) const;

  //Vector component-wise multiply and divide
  IntVector2 operator*(IntVec2Param rhs) const;
  IntVector2 operator/(IntVec2Param rhs) const;
  IntVector2 operator%(IntVec2Param rhs) const;
  
  //Bitwise operators
  IntVector2  operator~() const;
  IntVector2  operator<< (IntVec2Param rhs) const;
  IntVector2  operator>> (IntVec2Param rhs) const;
  IntVector2  operator|  (IntVec2Param rhs) const;
  IntVector2  operator^  (IntVec2Param rhs) const;
  IntVector2  operator&  (IntVec2Param rhs) const;
  IntVector2& operator<<=(IntVec2Param rhs);
  IntVector2& operator>>=(IntVec2Param rhs);
  IntVector2& operator|= (IntVec2Param rhs);
  IntVector2& operator^= (IntVec2Param rhs);
  IntVector2& operator&= (IntVec2Param rhs);

  //Comparison operators
  BoolVec2 operator< (IntVec2Param rhs) const;
  BoolVec2 operator<=(IntVec2Param rhs) const;
  BoolVec2 operator> (IntVec2Param rhs) const;
  BoolVec2 operator>=(IntVec2Param rhs) const;

  ///Set all of the values of this vector at once.
  void Set(int x, int y);

  ///Set all of this vector's elements to 0.
  void ZeroOut(void);

  union
  {
    struct  
    {
      int x, y;
    };
    int array[2];
  };

  static const IntVector2 cZero;
  static const IntVector2 cXAxis;
  static const IntVector2 cYAxis;
};

///Binary Operators (integers)
ZeroShared IntVector2 operator*(int lhs, IntVec2Param rhs);

///Returns a vector with absolute valued elements of the given vector.
ZeroShared IntVector2 Abs(IntVec2Param vec);

///Returns the component-wise minimum vector of the two vectors.
ZeroShared IntVector2 Min(IntVec2Param lhs, IntVec2Param rhs);

///Returns the component-wise maximum vector of the two vectors.
ZeroShared IntVector2 Max(IntVec2Param lhs, IntVec2Param rhs);

}// namespace Math
