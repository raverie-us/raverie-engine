///////////////////////////////////////////////////////////////////////////////
///
/// \file DecomposedMatrix4.hpp
/// Declaration of the DecomposedMatrix4 structure.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

/// Stores a decomposed matrix 4. This means that shear is lost.
/// This does however allow efficient inverse transforms and
/// extracting of each portion of a transformation.
struct ZeroShared DecomposedMatrix4
{
  DecomposedMatrix4();
  DecomposedMatrix4(Mat4Param transform);

  void Set(Mat4Param transform);

  Vector3 TransformNormal(Vec3Param normal);
  Vector3 InverseTransformNormal(Vec3Param normal);

  /// Transform the surface normal by doing the inverse transpose of the transform.
  Vector3 TransformSurfaceNormal(Vec3Param direction);
  Vector3 InverseTransformSurfaceNormal(Vec3Param direction);

  Vector3 TransformPoint(Vec3Param point);
  Vector3 InverseTransformPoint(Vec3Param point);

  Vector3 Scale;
  Matrix3 Rotation;
  Vector3 Translation;
};

}//namespace Math
