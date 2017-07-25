///////////////////////////////////////////////////////////////////////////////
///
/// \file GenericVector.inl
/// Implementation of the GenericVector structure.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Diagnostic/Diagnostic.hpp"

namespace Math
{

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension>::GenericVector()
{
  ZeroOut();
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension>::GenericVector(scalar* data)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] = data[i];
}

template <typename ScalarType, uint Dimension>
typename GenericVector<ScalarType,Dimension>::scalar& GenericVector<ScalarType,Dimension>::operator[](uint index)
{
  ErrorIf(index > Dim, "Math::GenericVector - Subscript out of range.");
  return array[index];
}

template <typename ScalarType, uint Dimension>
typename GenericVector<ScalarType,Dimension>::scalar GenericVector<ScalarType,Dimension>::operator[](uint index) const
{
  ErrorIf(index > Dim, "Math::GenericVector - Subscript out of range.");
  return array[index];
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator-(void) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = -array[i];
  return ret;
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::operator*=(scalar rhs)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] *= rhs;
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::operator/=(scalar rhs)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] /= rhs;
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator*(scalar rhs) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = array[i] * rhs;
  return ret;
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator/(scalar rhs) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = array[i] / rhs;
  return ret;
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::operator+=(SelfParam rhs)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] += rhs[i];
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::operator-=(SelfParam rhs)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] -= rhs[i];
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator+(SelfParam rhs) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = array[i] + rhs[i];
  return ret;
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator-(SelfParam rhs) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = array[i] - rhs[i];
  return ret;
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator*(SelfParam rhs) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = array[i] * rhs[i];
  return ret;
}

template <typename ScalarType, uint Dimension>
GenericVector<ScalarType,Dimension> GenericVector<ScalarType,Dimension>::operator/(SelfParam rhs) const
{
  SelfType ret;
  for(uint i = 0; i < Dim; ++i)
    ret[i] = array[i] / rhs[i];
  return ret;
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::operator*=(SelfParam rhs)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] *= rhs[i];
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::operator/=(SelfParam rhs)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] /= rhs[i];
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::ZeroOut()
{
  for(uint i = 0; i < Dim; ++i)
    array[i] = scalar(0);
}

template <typename ScalarType, uint Dimension>
void GenericVector<ScalarType,Dimension>::Splat(scalar value)
{
  for(uint i = 0; i < Dim; ++i)
    array[i] = value;
}

template <typename ScalarType, uint Dimension>
typename GenericVector<ScalarType,Dimension>::scalar GenericVector<ScalarType,Dimension>::Dot(SelfParam rhs) const
{
  scalar ret = scalar(0);
  for(uint i = 0; i < Dim; ++i)
    ret += array[i] * rhs[i];
  return ret;
}

template <typename ScalarType, uint Dimension>
typename GenericVector<ScalarType,Dimension>::scalar GenericVector<ScalarType,Dimension>::LengthSq(void) const
{
  return Dot(*this);
}

}//namespace Math
