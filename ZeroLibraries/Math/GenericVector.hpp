///////////////////////////////////////////////////////////////////////////////
///
/// \file GenericVector.hpp
/// Declaration of the GenericVector structure.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Reals.hpp"

namespace Math
{

template <typename ScalarType, uint Dimension>
struct ZeroSharedTemplate GenericVector
{
  //should technically add a trait type for the scalar type to do things
  //like get zero and whatnot, but that's not necessary yet
  typedef ScalarType scalar;
  typedef GenericVector<ScalarType, Dimension> SelfType;
  typedef SelfType& SelfRef;
  typedef const SelfType& SelfParam;

  enum CompileTime{Dim = Dimension};

  GenericVector();
  explicit GenericVector(scalar* data);

  scalar& operator[](uint index);
  scalar operator[](uint index) const;

  //Unary Operators
  SelfType operator-(void) const;

  //Binary Assignment Operators (reals)
  void operator*=(scalar rhs);
  void operator/=(scalar rhs);

  //Binary Operators (Reals)
  SelfType operator*(scalar rhs) const;
  SelfType operator/(scalar rhs) const;

  //Binary Assignment Operators (vectors)
  void operator+=(SelfParam rhs);
  void operator-=(SelfParam rhs);

  //Binary Operators (vectors)
  SelfType operator+(SelfParam rhs) const;
  SelfType operator-(SelfParam rhs) const;

  //Vector component wise multiply and divide
  SelfType operator*(SelfParam rhs) const;
  SelfType operator/(SelfParam rhs) const;

  ///Component-wise assignment multiplication
  void operator*=(SelfParam rhs);
  void operator/=(SelfParam rhs);

  void ZeroOut();

  ///Set all of the values of the vector to the passed in value.
  void Splat(scalar value);

  ///Compute the dot product of this vector with the given vector.
  scalar Dot(SelfParam rhs) const;

  ///Get the squared length of this vector.
  scalar LengthSq(void) const;

  scalar array[Dimension];
};

}//namespace Math

#include "Math/GenericVector.inl"
