///////////////////////////////////////////////////////////////////////////////
///
/// \file IntVector3.hpp
/// Declaration of the IntVector3 structure.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"
#include "Math/BoolVector3.hpp"

namespace Math
{

struct IntVector3;
typedef IntVector3 IntVec3;
typedef const IntVector3& IntVec3Param;
typedef IntVector3& IntVec3Ref;
typedef IntVector3* IntVec3Ptr;

///3 dimensional integral vector.
struct ZeroShared IntVector3
{
  IntVector3(void) {}
  explicit IntVector3(int x, int y, int z);

  int& operator[](uint index);
  int operator[](uint index) const;

  //Unary Operators
  IntVector3 operator-(void) const;

  //Binary Assignment Operators (integers)
  void operator*=(int rhs);
  void operator/=(int rhs);

  //Binary Operators (integers)
  IntVector3 operator*(int rhs) const;
  IntVector3 operator/(int rhs) const;
  IntVector3 operator%(int rhs) const;

  //Unary operators
  IntVector3& operator++();
  IntVector3& operator--();

  //Binary Assignment Operators (vectors)
  void operator+=(IntVec3Param rhs);
  void operator-=(IntVec3Param rhs);
  void operator*=(IntVec3Param rhs);
  void operator/=(IntVec3Param rhs);

  //Binary Operators (vectors)
  IntVector3 operator+(IntVec3Param rhs) const;
  IntVector3 operator-(IntVec3Param rhs) const;

  //Binary Vector Comparisons
  bool operator==(IntVec3Param rhs) const;
  bool operator!=(IntVec3Param rhs) const;

  //Vector component-wise multiply and divide
  IntVector3 operator*(IntVec3Param rhs) const;
  IntVector3 operator/(IntVec3Param rhs) const;
  IntVector3 operator%(IntVec3Param rhs) const;
  
  //Bitwise operators
  IntVector3  operator~() const;
  IntVector3  operator<< (IntVec3Param rhs) const;
  IntVector3  operator>> (IntVec3Param rhs) const;
  IntVector3  operator|  (IntVec3Param rhs) const;
  IntVector3  operator^  (IntVec3Param rhs) const;
  IntVector3  operator&  (IntVec3Param rhs) const;
  IntVector3& operator<<=(IntVec3Param rhs);
  IntVector3& operator>>=(IntVec3Param rhs);
  IntVector3& operator|= (IntVec3Param rhs);
  IntVector3& operator^= (IntVec3Param rhs);
  IntVector3& operator&= (IntVec3Param rhs);

  //Comparison operators
  BoolVec3 operator< (IntVec3Param rhs) const;
  BoolVec3 operator<=(IntVec3Param rhs) const;
  BoolVec3 operator> (IntVec3Param rhs) const;
  BoolVec3 operator>=(IntVec3Param rhs) const;

  ///Set all of the values of this vector at once.
  void Set(int x, int y, int z);

  ///Set all of this vector's elements to 0.
  void ZeroOut(void);

  union
  {
    struct  
    {
      int x, y, z;
    };
    int array[3];
  };

  static const IntVector3 cZero;
  static const IntVector3 cXAxis;
  static const IntVector3 cYAxis;
  static const IntVector3 cZAxis;
};

///Binary Operators (integers)
ZeroShared IntVector3 operator*(int lhs, IntVec3Param rhs);

///Returns a vector with absolute valued elements of the given vector.
ZeroShared IntVector3 Abs(IntVec3Param vec);

///Returns the component-wise minimum vector of the two vectors.
ZeroShared IntVector3 Min(IntVec3Param lhs, IntVec3Param rhs);

///Returns the component-wise maximum vector of the two vectors.
ZeroShared IntVector3 Max(IntVec3Param lhs, IntVec3Param rhs);

}// namespace Math
