///////////////////////////////////////////////////////////////////////////////
///
/// \file IntVector4.hpp
/// Declaration of the IntVector4 structure.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"
#include "Math/BoolVector4.hpp"

namespace Math
{

struct IntVector4;
typedef IntVector4 IntVec4;
typedef const IntVector4& IntVec4Param;
typedef IntVector4& IntVec4Ref;
typedef IntVector4* IntVec4Ptr;

///3 dimensional integral vector.
struct ZeroShared IntVector4
{
  IntVector4(void) {}
  explicit IntVector4(int x, int y, int z, int w);

  int& operator[](uint index);
  int operator[](uint index) const;

  //Unary Operators
  IntVector4 operator-(void) const;

  //Binary Assignment Operators (integers)
  void operator*=(int rhs);
  void operator/=(int rhs);

  //Binary Operators (integers)
  IntVector4 operator*(int rhs) const;
  IntVector4 operator/(int rhs) const;
  IntVector4 operator%(int rhs) const;

  //Unary operators
  IntVector4& operator++();
  IntVector4& operator--();

  //Binary Assignment Operators (vectors)
  void operator+=(IntVec4Param rhs);
  void operator-=(IntVec4Param rhs);
  void operator*=(IntVec4Param rhs);
  void operator/=(IntVec4Param rhs);

  //Binary Operators (vectors)
  IntVector4 operator+(IntVec4Param rhs) const;
  IntVector4 operator-(IntVec4Param rhs) const;

  //Binary Vector Comparisons
  bool operator==(IntVec4Param rhs) const;
  bool operator!=(IntVec4Param rhs) const;

  //Vector component-wise multiply and divide
  IntVector4 operator*(IntVec4Param rhs) const;
  IntVector4 operator/(IntVec4Param rhs) const;
  IntVector4 operator%(IntVec4Param rhs) const;
  
  //Bitwise operators
  IntVector4  operator~() const;
  IntVector4  operator<< (IntVec4Param rhs) const;
  IntVector4  operator>> (IntVec4Param rhs) const;
  IntVector4  operator|  (IntVec4Param rhs) const;
  IntVector4  operator^  (IntVec4Param rhs) const;
  IntVector4  operator&  (IntVec4Param rhs) const;
  IntVector4& operator<<=(IntVec4Param rhs);
  IntVector4& operator>>=(IntVec4Param rhs);
  IntVector4& operator|= (IntVec4Param rhs);
  IntVector4& operator^= (IntVec4Param rhs);
  IntVector4& operator&= (IntVec4Param rhs);

  //Comparison operators
  BoolVec4 operator< (IntVec4Param rhs) const;
  BoolVec4 operator<=(IntVec4Param rhs) const;
  BoolVec4 operator> (IntVec4Param rhs) const;
  BoolVec4 operator>=(IntVec4Param rhs) const;

  ///Set all of the values of this vector at once.
  void Set(int x, int y, int z, int w);

  ///Set all of this vector's elements to 0.
  void ZeroOut(void);

  union
  {
    struct  
    {
      int x, y, z, w;
    };
    int array[4];
  };

  static const IntVector4 cZero;
  static const IntVector4 cXAxis;
  static const IntVector4 cYAxis;
  static const IntVector4 cZAxis;
  static const IntVector4 cWAxis;
};

///Binary Operators (integers)
ZeroShared IntVector4 operator*(int lhs, IntVec4Param rhs);

///Returns a vector with absolute valued elements of the given vector.
ZeroShared IntVector4 Abs(IntVec4Param vec);

///Returns the component-wise minimum vector of the two vectors.
ZeroShared IntVector4 Min(IntVec4Param lhs, IntVec4Param rhs);

///Returns the component-wise maximum vector of the two vectors.
ZeroShared IntVector4 Max(IntVec4Param lhs, IntVec4Param rhs);

}// namespace Math
