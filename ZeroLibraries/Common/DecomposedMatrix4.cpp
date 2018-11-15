///////////////////////////////////////////////////////////////////////////////
///
/// \file DecomposedMatrix4.cpp
/// Implementation of the DecomposedMatrix4 structure.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

DecomposedMatrix4::DecomposedMatrix4()
{
  Translation.ZeroOut();
  Scale.Splat(real(1.0));
  Rotation.SetIdentity();
}

DecomposedMatrix4::DecomposedMatrix4(Mat4Param transform)
{
  Set(transform);
}

void DecomposedMatrix4::Set(Mat4Param transform)
{
  transform.Decompose(&Scale,&Rotation,&Translation);
}

Vector3 DecomposedMatrix4::TransformNormal(Vec3Param normal)
{
  Vector3 result = normal * Scale;
  return Math::Transform(Rotation,result);
}

Vector3 DecomposedMatrix4::InverseTransformNormal(Vec3Param normal)
{
  Vector3 result = Math::TransposedTransform(Rotation,normal);
  return result / Scale;
}

Vector3 DecomposedMatrix4::TransformSurfaceNormal(Vec3Param direction)
{
  Vector3 result = Math::Transform(Rotation, direction);
  return result / Scale;
}

Vector3 DecomposedMatrix4::InverseTransformSurfaceNormal(Vec3Param direction)
{
  Vector3 result = Math::TransposedTransform(Rotation, direction);
  return result * Scale;
}

Vector3 DecomposedMatrix4::TransformPoint(Vec3Param point)
{
  Vector3 result = point * Scale;
  result = Math::Transform(Rotation,result);
  return result + Translation;
}

Vector3 DecomposedMatrix4::InverseTransformPoint(Vec3Param point)
{
  Vector3 result = point - Translation;
  result = Math::TransposedTransform(Rotation,result);
  return result / Scale;
}

}//namespace Math
