///////////////////////////////////////////////////////////////////////////////
///
/// \file Math.cpp
/// Central location for all the math used by the Zero engine.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{
typedef Vector2    Vec2;
typedef Vector3    Vec3;
typedef Vector4    Vec4;
typedef Matrix3    Mat3;
typedef Matrix4    Mat4;
typedef Quaternion Quat;

namespace
{
const uint I = 0; 
const uint J = 1; 
const uint H = 2;

void ClampAngle(real* angle)
{
  while(*angle < -Math::cPi)
  {
    *angle += Math::cTwoPi;
  }
  while(*angle > Math::cPi)
  {
    *angle -= Math::cTwoPi;
  }
}

} // namespace

///Creates a skew symmetric matrix from the given 3D vector. Multiplying a 
///vector by this matrix is equivalent to the cross product using the input 
///vector.
Matrix3 SkewSymmetric(Vec3Param vec3)
{
  Matrix3 mtx;
  mtx.m22 = mtx.m11 = mtx.m00 = real(0.0);
  mtx.m01 = -vec3[2];
  mtx.m02 =  vec3[1];
  mtx.m10 =  vec3[2];
  mtx.m12 = -vec3[0];
  mtx.m20 = -vec3[1];
  mtx.m21 =  vec3[0];
  return mtx;
}

///Converts a quaternion to an axis-angle pair (in radians). Axis is stored in 
///the Vector4's xyz and the angle is stored in the w.
Vector4 ToAxisAngle(QuatParam quaternion)
{
  Vector4 axisAngle;
  ToAxisAngle(quaternion, &axisAngle);
  return axisAngle;
}

void ToAxisAngle(QuatParam quaternion, Vec4Ptr axisAngle)
{
  ErrorIf(axisAngle == nullptr, "Math - Null pointer passed for axis-angle pair.");
  Quat tempQuat(Normalized(quaternion));

  axisAngle->w = real(2.0) * Math::ArcCos(tempQuat.w);
  real invSinAngle = Math::Sqrt(real(1.0) - tempQuat.w * tempQuat.w);

  if(Math::Abs(invSinAngle) < real(0.0005))
  {
    invSinAngle = real(1.0);
  }
  else
  {
    invSinAngle = real(1.0) / invSinAngle;
  }
  axisAngle->x = tempQuat.x * invSinAngle;
  axisAngle->y = tempQuat.y * invSinAngle;
  axisAngle->z = tempQuat.z * invSinAngle;
}

///Converts a quaternion to an axis-angle pair (in radians).
void ToAxisAngle(QuatParam quaternion, Vec3Ptr axis, real* radians)
{
  ErrorIf(axis == nullptr, "Math - Null pointer passed for axis.");
  ErrorIf(radians == nullptr, "Math - Null pointer passed for radians.");
  Quat tempQuat(Normalized(quaternion));

  *radians = real(2.0) * Math::ArcCos(tempQuat.w);
  real invSinAngle = Math::Sqrt(real(1.0) - tempQuat.w * tempQuat.w);

  if(Math::Abs(invSinAngle) < real(0.0005))
  {
    invSinAngle = real(1.0);
  }
  else
  {
    invSinAngle = real(1.0) / invSinAngle;
  }
  axis->x = tempQuat.x * invSinAngle;
  axis->y = tempQuat.y * invSinAngle;
  axis->z = tempQuat.z * invSinAngle;
}

///Convert a 3x3 matrix to a set of Euler angles (in radians). The desired order
///of the rotations is expected to be in the given Euler angle structure.
EulerAngles ToEulerAngles(Mat3Param matrix, EulerOrders::Enum order)
{
  EulerAngles eulerAngles(order);
  ToEulerAngles(matrix, &eulerAngles);
  return eulerAngles;
}

void ToEulerAngles(Mat3Param matrix, EulerAnglesPtr eulerAngles)
{
  ErrorIf(eulerAngles == nullptr, "Math - Null pointer passed for Euler angles.");
  uint i, j, k, h, parity, repeated, frame;
  EulerOrder::GetOrder(eulerAngles->Order, i, j, k, h, parity, repeated, frame);
  if(EulerOrders::Yes == repeated)
  {
    real sy = Math::Sqrt(matrix(i, j) * matrix(i, j) + 
                         matrix(i, k) * matrix(i, k));
    if(sy > real(16.0) * real(FLT_EPSILON))
    {
      (*eulerAngles)[cX] = Math::ArcTan2(matrix(i, j), matrix(i, k));
      (*eulerAngles)[cY] = Math::ArcTan2(sy, matrix(i, i));
      (*eulerAngles)[cZ] = Math::ArcTan2(matrix(j, i), -matrix(k, i));
    }
    else
    {
      (*eulerAngles)[cX] = Math::ArcTan2(-matrix(j, k), matrix(j, j));
      (*eulerAngles)[cY] = Math::ArcTan2(sy, matrix(i, i));
      (*eulerAngles)[cZ] = real(0.0);
    }
  }
  else
  {
    real cy = Math::Sqrt(matrix(i, i) * matrix(i, i) + 
                         matrix(j, i) * matrix(j, i));
    if(cy > real(16.0) * real(FLT_EPSILON))
    {
      (*eulerAngles)[cX] = Math::ArcTan2(matrix(k, j), matrix(k, k));
      (*eulerAngles)[cY] = Math::ArcTan2(-matrix(k, i), cy);
      (*eulerAngles)[cZ] = Math::ArcTan2(matrix(j, i), matrix(i, i));
    }
    else
    {
      (*eulerAngles)[cX] = Math::ArcTan2(-matrix(j, k), matrix(j, j));
      (*eulerAngles)[cY] = Math::ArcTan2(-matrix(k, i), cy);
      (*eulerAngles)[cZ] = real(0.0);
    }
  }
  if(EulerOrders::Odd == parity)
  {
    (*eulerAngles)[cX] *= real(-1.0);
    (*eulerAngles)[cY] *= real(-1.0);
    (*eulerAngles)[cZ] *= real(-1.0);
  }

  ClampAngle(&((*eulerAngles)[cX]));
  ClampAngle(&((*eulerAngles)[cY]));
  ClampAngle(&((*eulerAngles)[cZ]));

  if(EulerOrders::Rotated == frame)
  {
    Math::Swap((*eulerAngles)[cX], (*eulerAngles)[cZ]);
  }
}

///Convert a 4x4 matrix to a set of Euler angles in radians. The desired order
///of the rotations is expected to be in the given Euler angle structure.
EulerAngles ToEulerAngles(Mat4Param matrix, EulerOrders::Enum order)
{
  EulerAngles eulerAngles(order);
  ToEulerAngles(matrix, &eulerAngles);
  return eulerAngles;
}

void ToEulerAngles(Mat4Param matrix, EulerAnglesPtr eulerAngles)
{
  ErrorIf(eulerAngles == nullptr, "Math - Null pointer passed for Euler angles.");

  uint i, j, k, h, parity, repeated, frame;
  EulerOrder::GetOrder(eulerAngles->Order, i, j, k, h, parity, repeated, frame);
  if(EulerOrders::Yes == repeated)
  {
    real sy = Math::Sqrt(matrix(i, j) * matrix(i, j) + 
                         matrix(i, k) * matrix(i, k));
    if(sy > real(16.0) * real(FLT_EPSILON))
    {
      (*eulerAngles)[cX] = Math::ArcTan2(matrix(i, j), matrix(i, k));
      (*eulerAngles)[cY] = Math::ArcTan2(sy, matrix(i, i));
      (*eulerAngles)[cZ] = Math::ArcTan2(matrix(j, i), -matrix(k, i));
    }
    else
    {
      (*eulerAngles)[cX] = Math::ArcTan2(-matrix(j, k), matrix(j, j));
      (*eulerAngles)[cY] = Math::ArcTan2(sy, matrix(i, i));
      (*eulerAngles)[cZ] = real(0.0);
    }
  }
  else
  {
    real cy = Math::Sqrt(matrix(i, i) * matrix(i, i) + 
                         matrix(j, i) * matrix(j, i));
    if(cy > real(16.0) * real(FLT_EPSILON))
    {
      (*eulerAngles)[cX] = Math::ArcTan2(matrix(k, j), matrix(k, k));
      (*eulerAngles)[cY] = Math::ArcTan2(-matrix(k, i), cy);
      (*eulerAngles)[cZ] = Math::ArcTan2(matrix(j, i), matrix(i, i));
    }
    else
    {
      (*eulerAngles)[cX] = Math::ArcTan2(-matrix(j, k), matrix(j, j));
      (*eulerAngles)[cY] = Math::ArcTan2(-matrix(k, i), cy);
      (*eulerAngles)[cZ] = real(0.0);
    }
  }
  if(EulerOrders::Odd == parity)
  {
    (*eulerAngles)[cX] *= real(-1.0);
    (*eulerAngles)[cY] *= real(-1.0);
    (*eulerAngles)[cZ] *= real(-1.0);
  }

  ClampAngle(&((*eulerAngles)[cX]));
  ClampAngle(&((*eulerAngles)[cY]));
  ClampAngle(&((*eulerAngles)[cZ]));

  if(EulerOrders::Rotated == frame)
  {
    Math::Swap((*eulerAngles)[cX], (*eulerAngles)[cZ]);
  }
}

///Convert a quaternion to a set of Euler angles (in radians). The desired order
///of the rotations is expected to be in the given Euler angle structure.
EulerAngles ToEulerAngles(QuatParam quaternion, EulerOrders::Enum order)
{
  EulerAngles eulerAngles(order);
  ToEulerAngles(quaternion, &eulerAngles);
  return eulerAngles;
}

void ToEulerAngles(QuatParam quaternion, EulerAnglesPtr eulerAngles)
{
  ErrorIf(eulerAngles == nullptr, "Math - Null pointer passed for Euler angles.");

  Matrix3 matrix;
  ToMatrix3(quaternion, &matrix);
  ToEulerAngles(matrix, eulerAngles);
}

/// Converts from Vector3 to Vector2, removing the z component of the Vector3.
Vector2 ToVector2(Vec3Param v3)
{
  return Vector2(v3.x, v3.y);
}

/// Converts from Vector2 to Vector3, adding the given z component.
Vector3 ToVector3(Vec2Param v2, real z)
{
  return Vector3(v2.x, v2.y, z);
}

Vector3 ToVector3(Vec4Param v)
{
  return Vector3(v.x, v.y, v.z);
}

///Converts an axis-angle pair to a 3x3 (in radians). Axis is stored in the
///Vector4's xyz and the angle is stored in the w. Axis is assumed to be 
///normalized.
Matrix3 ToMatrix3(Vec4Param axisAngle)
{
  Matrix3 matrix;
  ToMatrix3(axisAngle, &matrix);
  return matrix;
}

void ToMatrix3(Vec4Param axisAngle, Mat3Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Math - Null pointer passed for matrix.");

  real c0 = Math::Cos(axisAngle.w);
  real n1C0 = real(1.0) - c0;
  real s0 = Math::Sin(axisAngle.w);

  //| x^2(1-c0)+c0  xy(1-c0)-zs0  xz(1-c0)+ys0 |
  //| xy(1-c0)+zs0  y^2(1-c0)+c0  yz(1-c0)-xs0 |
  //| xz(1-c0)-ys0  yz(1-c0)+xs0  z^2(1-c0)+c0 |
  matrix->m00 = axisAngle.x * axisAngle.x * n1C0 + c0;
  matrix->m01 = axisAngle.x * axisAngle.y * n1C0 - axisAngle.z * s0;
  matrix->m02 = axisAngle.x * axisAngle.z * n1C0 + axisAngle.y * s0;

  matrix->m10 = axisAngle.x * axisAngle.y * n1C0 + axisAngle.z * s0;
  matrix->m11 = axisAngle.y * axisAngle.y * n1C0 + c0;
  matrix->m12 = axisAngle.y * axisAngle.z * n1C0 - axisAngle.x * s0;

  matrix->m20 = axisAngle.x * axisAngle.z * n1C0 - axisAngle.y * s0;
  matrix->m21 = axisAngle.y * axisAngle.z * n1C0 + axisAngle.x * s0;
  matrix->m22 = axisAngle.z * axisAngle.z * n1C0 + c0;
}

///Converts an axis-angle pair to a 3x3 matrix (in radians). Axis is assumed to
///be normalized.
Matrix3 ToMatrix3(Vec3Param axis, real radians)
{
  Matrix3 matrix;
  ToMatrix3(axis, radians, &matrix);
  return matrix;
}

void ToMatrix3(Vec3Param axis, real radians, Mat3Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Math - Null pointer passed for matrix.");

  real c0 = Math::Cos(radians);
  real n1C0 = real(1.0) - c0;
  real s0 = Math::Sin(radians);

  //| x^2(1-c0)+c0  xy(1-c0)-zs0  xz(1-c0)+ys0 |
  //| xy(1-c0)+zs0  y^2(1-c0)+c0  yz(1-c0)-xs0 |
  //| xz(1-c0)-ys0  yz(1-c0)+xs0  z^2(1-c0)+c0 |
  matrix->m00 = axis.x * axis.x * n1C0 + c0;
  matrix->m01 = axis.x * axis.y * n1C0 - axis.z * s0;
  matrix->m02 = axis.x * axis.z * n1C0 + axis.y * s0;

  matrix->m10 = axis.x * axis.y * n1C0 + axis.z * s0;
  matrix->m11 = axis.y * axis.y * n1C0 + c0;
  matrix->m12 = axis.y * axis.z * n1C0 - axis.x * s0;

  matrix->m20 = axis.x * axis.z * n1C0 - axis.y * s0;
  matrix->m21 = axis.y * axis.z * n1C0 + axis.x * s0;
  matrix->m22 = axis.z * axis.z * n1C0 + c0;
}

///Convert a set of Euler angles to a 3x3 matrix (in radians).
Matrix3 ToMatrix3(EulerAnglesParam eulerAngles)
{
  Matrix3 matrix;
  ToMatrix3(eulerAngles, &matrix);
  return matrix;
}

void ToMatrix3(EulerAnglesParam eulerAngles, Mat3Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Math - Null pointer passed for matrix.");

  real angles[3] = { eulerAngles[0], eulerAngles[1], eulerAngles[2] };
  uint i, j, k, h, parity, repeated, frame;
  EulerOrder::GetOrder(eulerAngles.Order, i, j, k, h, parity, repeated, frame);
  if(EulerOrders::Rotated == frame)
  {
    Math::Swap(angles[cX], angles[cZ]);
  }
  if(EulerOrders::Odd == parity)
  {
    angles[cX] *= real(-1.0);
    angles[cY] *= real(-1.0);
    angles[cZ] *= real(-1.0);
  }
  
  real t[3], c[3], s[3];
  t[I] = angles[cX];      t[J] = angles[cY];      t[H] = angles[cZ];
  c[I] = Math::Cos(t[I]); c[J] = Math::Cos(t[J]); c[H] = Math::Cos(t[H]);
  s[I] = Math::Sin(t[I]); s[J] = Math::Sin(t[J]); s[H] = Math::Sin(t[H]);

  const real cc = c[I] * c[H]; 
  const real cs = c[I] * s[H]; 
  const real sc = s[I] * c[H]; 
  const real ss = s[I] * s[H];
  if(EulerOrders::Yes == repeated)
  {
    (*matrix)(i, i) =  c[J];        
    (*matrix)(i, j) =  c[J] * s[I];
    (*matrix)(i, k) =  c[J] * c[I];

    (*matrix)(j, i) =  c[J] * s[H];
    (*matrix)(j, j) = -c[J] * ss + cc;
    (*matrix)(j, k) = -c[J] * cs - sc;

    (*matrix)(k, i) = -c[J] * c[H];
    (*matrix)(k, j) =  c[J] * sc + cs;
    (*matrix)(k, k) =  c[J] * cc - ss;
  } 
  else 
  {
    (*matrix)(i, i) =  c[J] * c[H];
    (*matrix)(j, i) =  c[J] * s[H];
    (*matrix)(k, i) = -s[J];

    (*matrix)(i, j) =  s[J] * sc - cs;
    (*matrix)(j, j) =  s[J] * ss + cc;
    (*matrix)(k, j) =  c[J] * s[I];

    (*matrix)(i, k) =  s[J] * cc + ss;
    (*matrix)(j, k) =  s[J] * cs - sc;
    (*matrix)(k, k) =  c[J] * c[I];
  }
}

ZeroShared Matrix3 ToMatrix3(real xRadians, real yRadians, real zRadians)
{
  EulerAngles euler(xRadians, yRadians, zRadians, Math::EulerOrders::XYZs);
  return Math::ToMatrix3(euler);
}

///Convert a 4x4 matrix to a 3x3 matrix. Simply copies the 4x4 matrix's upper 
///3x3 matrix (rotation & scale) to the 3x3 matrix.
Matrix3 ToMatrix3(Mat4Param matrix)
{
  Matrix3 mat3;
  ToMatrix3(matrix, &mat3);
  return mat3;
}

void ToMatrix3(Mat4Param mat4, Mat3Ptr mat3)
{
  ErrorIf(mat3 == nullptr, "Math - Null pointer passed for matrix.");

  //First "cross" components
  mat3->m00 = mat4.m00; 
  mat3->m01 = mat4.m01;
  mat3->m02 = mat4.m02;

  //Second "cross" components
  mat3->m10 = mat4.m10;
  mat3->m11 = mat4.m11; 
  mat3->m12 = mat4.m12;

  //Third "cross" components
  mat3->m20 = mat4.m20;
  mat3->m21 = mat4.m21;
  mat3->m22 = mat4.m22;
}

///Converts a quaternion to a 3x3 rotation matrix (in radians).
Matrix3 ToMatrix3(QuatParam quaternion)
{
  Matrix3 matrix;
  ToMatrix3(quaternion, &matrix);
  return matrix;
}

void ToMatrix3(QuatParam quaternion, Mat3Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Math - Null pointer passed for matrix.");

  //     |       2     2                                |
  //     | 1 - 2Y  - 2Z    2XY - 2ZW      2XZ + 2YW     |
  //     |                                              |
  //     |                       2     2                |
  // M = | 2XY + 2ZW       1 - 2X  - 2Z   2YZ - 2XW     |
  //     |                                              |
  //     |                                      2     2 |
  //     | 2XZ - 2YW       2YZ + 2XW      1 - 2X  - 2Y  |
  //     |                                              |

  real xx = quaternion.x * quaternion.x;
  real xy = quaternion.x * quaternion.y;
  real xz = quaternion.x * quaternion.z;
  real yy = quaternion.y * quaternion.y;
  real yz = quaternion.y * quaternion.z;
  real zz = quaternion.z * quaternion.z;
  real zw = quaternion.z * quaternion.w;
  real yw = quaternion.y * quaternion.w;
  real xw = quaternion.x * quaternion.w;

  matrix->m00 = real(1.0) - real(2.0) * (yy + zz);
  matrix->m01 = real(2.0) * (xy - zw);
  matrix->m02 = real(2.0) * (xz + yw);

  matrix->m10 = real(2.0) * (xy + zw);
  matrix->m11 = real(1.0) - real(2.0) * (xx + zz);
  matrix->m12 = real(2.0) * (yz - xw);

  matrix->m20 = real(2.0) * (xz - yw);
  matrix->m21 = real(2.0) * (yz + xw);
  matrix->m22 = real(1.0) - real(2.0) * (xx + yy);
}

Matrix3 ToMatrix3(Vec3Param facing)
{
  Vec3 up, right;
  Math::GenerateOrthonormalBasis(facing, &right, &up);
  return ToMatrix3(facing, up, right);
}

Matrix3 ToMatrix3(Vec3Param facing, Vec3Param up)
{
  //Get the right vector
  Vec3 right = Math::Cross(facing, up);
  return ToMatrix3(facing, up, right);
}

Matrix3 ToMatrix3(Vec3Param facing, Vec3Param up, Vec3Param right)
{
  Matrix3 mat;
  mat.SetBasis(0, right);
  mat.SetBasis(1, up);
  mat.SetBasis(2, -facing);
  return mat;
}

///Convert a set of Euler angles to a 4x4 matrix (in radians).
Matrix4 ToMatrix4(EulerAnglesParam eulerAngles)
{
  Matrix4 matrix;
  ToMatrix4(eulerAngles, &matrix);
  return matrix;
}

void ToMatrix4(EulerAnglesParam eulerAngles, Mat4Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Math - Null pointer passed for matrix.");

  real angles[3] = { eulerAngles[0], eulerAngles[1], eulerAngles[2] };
  uint i, j, k, h, parity, repeated, frame;
  EulerOrder::GetOrder(eulerAngles.Order, i, j, k, h, parity, repeated, frame);
  if(EulerOrders::Rotated == frame)
  {
    Math::Swap(angles[cX], angles[cZ]);
  }
  if(EulerOrders::Odd == parity)
  {
    angles[cX] *= real(-1.0);
    angles[cY] *= real(-1.0);
    angles[cZ] *= real(-1.0);
  }
  real t[3], c[3], s[3];
  t[I] = angles[cX];       t[J] = angles[cY];       t[H] = angles[cZ];
  c[I] = Math::Cos(t[I]); c[J] = Math::Cos(t[J]); c[H] = Math::Cos(t[H]);
  s[I] = Math::Sin(t[I]); s[J] = Math::Sin(t[J]); s[H] = Math::Sin(t[H]);
  real cc = c[I] * c[H]; 
  real cs = c[I] * s[H]; 
  real sc = s[I] * c[H]; 
  real ss = s[I] * s[H];
  if(EulerOrders::Yes == repeated)
  {
    (*matrix)(i, i) =  c[J];        
    (*matrix)(i, j) =  c[J] * s[I];
    (*matrix)(i, k) =  c[J] * c[I];

    (*matrix)(j, i) =  c[J] * s[H];
    (*matrix)(j, j) = -c[J] * ss + cc;
    (*matrix)(j, k) = -c[J] * cs - sc;

    (*matrix)(k, i) = -c[J] * c[H];
    (*matrix)(k, j) =  c[J] * sc + cs;
    (*matrix)(k, k) =  c[J] * cc - ss;
  } 
  else 
  {
    (*matrix)(i, i) =  c[J] * c[H];
    (*matrix)(j, i) =  c[J] * s[H];
    (*matrix)(k, i) = -s[J];

    (*matrix)(i, j) =  s[J] * sc - cs;
    (*matrix)(j, j) =  s[J] * ss + cc;
    (*matrix)(k, j) =  c[J] * s[I];

    (*matrix)(i, k) =  s[J] * cc + ss;
    (*matrix)(j, k) =  s[J] * cs - sc;
    (*matrix)(k, k) =  c[J] * c[I];
  }
  matrix->m03 = real(0.0);  matrix->m13 = real(0.0);  matrix->m23 = real(0.0);
  matrix->m30 = real(0.0);  matrix->m31 = real(0.0);  matrix->m32 = real(0.0);
  matrix->m33 = real(1.0);
}

///Convert a 3x3 matrix to a 4x4 matrix. Simply copies the 3x3 matrix's values
///into the rotational part of the 4x4 matrix.
Matrix4 ToMatrix4(Mat3Param matrix)
{
  Matrix4 matrix4;
  ToMatrix4(matrix, &matrix4);
  return matrix4;
}

void ToMatrix4(Mat3Param mat3, Mat4Ptr mat4)
{
  ErrorIf(mat4 == nullptr, "Math - Null pointer passed for matrix.");

  //First "cross" components
  mat4->m00 = mat3.m00;
  mat4->m01 = mat3.m01;   
  mat4->m02 = mat3.m02;
  mat4->m03 = real(0.0);
  
  //Second "cross" components
  mat4->m10 = mat3.m10;
  mat4->m11 = mat3.m11;   
  mat4->m12 = mat3.m12;
  mat4->m13 = real(0.0);
  
  //Third "cross" components
  mat4->m20 = mat3.m20;
  mat4->m21 = mat3.m21;   
  mat4->m22 = mat3.m22;
  mat4->m23 = real(0.0);
  
  //Fourth "cross" components
  mat4->m30 = real(0.0);
  mat4->m31 = real(0.0);  
  mat4->m32 = real(0.0);
  mat4->m33 = real(1.0);
}

///Converts a quaternion to a 4x4 rotation matrix (in radians).
Matrix4 ToMatrix4(QuatParam quaternion)
{
  Matrix4 matrix;
  ToMatrix4(quaternion, &matrix);
  return matrix;
}

void ToMatrix4(QuatParam quaternion, Mat4Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Math - Null pointer passed for matrix.");

  //     |       2     2                                |
  //     | 1 - 2Y  - 2Z    2XY - 2ZW      2XZ + 2YW     |
  //     |                                              |
  //     |                       2     2                |
  // M = | 2XY + 2ZW       1 - 2X  - 2Z   2YZ - 2XW     |
  //     |                                              |
  //     |                                      2     2 |
  //     | 2XZ - 2YW       2YZ + 2XW      1 - 2X  - 2Y  |
  //     |                                              |

  real xx = quaternion.x * quaternion.x;
  real xy = quaternion.x * quaternion.y;
  real xz = quaternion.x * quaternion.z;
  real yy = quaternion.y * quaternion.y;
  real yz = quaternion.y * quaternion.z;
  real zz = quaternion.z * quaternion.z;
  real zw = quaternion.z * quaternion.w;
  real yw = quaternion.y * quaternion.w;
  real xw = quaternion.x * quaternion.w;

  matrix->m00 = real(1.0) - real(2.0) * (yy + zz);
  matrix->m01 = real(2.0) * (xy - zw);
  matrix->m02 = real(2.0) * (xz + yw);
  matrix->m03 = real(0.0);

  matrix->m10 = real(2.0) * (xy + zw);
  matrix->m11 = real(1.0) - real(2.0) * (xx + zz);
  matrix->m12 = real(2.0) * (yz - xw);
  matrix->m13 = real(0.0);

  matrix->m20 = real(2.0) * (xz - yw);
  matrix->m21 = real(2.0) * (yz + xw);
  matrix->m22 = real(1.0) - real(2.0) * (xx + yy);
  matrix->m23 = real(0.0);

  matrix->m30 = real(0.0);
  matrix->m31 = real(0.0);
  matrix->m32 = real(0.0);
  matrix->m33 = real(1.0);
}

///Converts an axis-angle pair to a quaternion (in radians). Axis is stored in
///the Vector4's xyz and the angle is stored in the w. Axis is assumed to be 
///normalized.
Quat ToQuaternion(Vec4Param axisAngle)
{
  Quat quaternion;
  ToQuaternion(axisAngle, &quaternion);
  return quaternion;
}

void ToQuaternion(Vec4Param axisAngle, QuatPtr quaternion)
{
  ErrorIf(quaternion == nullptr, "Math - Null pointer passed for quaternion.");

  real alpha = axisAngle.w * real(0.5);
  real sinAlpha = Math::Sin(alpha);

  quaternion->x = axisAngle.x * sinAlpha;
  quaternion->y = axisAngle.y * sinAlpha;
  quaternion->z = axisAngle.z * sinAlpha;
  quaternion->w = Math::Cos(alpha);
}

///Converts an axis-angle pair to a quaternion (in radians). Axis is assumed to
///be normalized.
Quaternion ToQuaternion(Vec3Param axis, real radians)
{
  Quat quaternion;
  ToQuaternion(Math::AttemptNormalized(axis), radians, &quaternion);
  return quaternion;
}

void ToQuaternion(Vec3Param axis, real radians, QuatPtr quaternion)
{
  ErrorIf(quaternion == nullptr, "Math - Null pointer passed for quaternion.");

  real alpha = radians * real(0.5);
  real sinAlpha = Math::Sin(alpha);

  quaternion->x = axis.x * sinAlpha;
  quaternion->y = axis.y * sinAlpha;
  quaternion->z = axis.z * sinAlpha;
  quaternion->w = Math::Cos(alpha);
}

///Convert a set of Euler angles to a quaternion (in radians).
Quat ToQuaternion(EulerAnglesParam eulerAngles)
{
  Quat quaternion;
  ToQuaternion(eulerAngles, &quaternion);
  return quaternion;
}

void ToQuaternion(EulerAnglesParam eulerAngles, QuatPtr quaternion)
{
  ErrorIf(quaternion == nullptr, "Math - Null pointer passed for quaternion.");

  real angles[3] = { eulerAngles[cX], eulerAngles[cY], eulerAngles[cZ] };
  uint i, j, k, h, parity, repeated, frame;
  EulerOrder::GetOrder(eulerAngles.Order, i, j, k, h, parity, repeated, frame);
  if(EulerOrders::Rotated == frame)
  {
    Math::Swap(angles[cX], angles[cZ]);
  }

  if(EulerOrders::Odd == parity)
  {
    angles[cY] *= real(-1.0);
  }

  real t[3], c[3], s[3];
  t[I] = angles[cX] * real(0.5); c[I] = Math::Cos(t[I]); s[I] = Math::Sin(t[I]);
  t[J] = angles[cY] * real(0.5); c[J] = Math::Cos(t[J]); s[J] = Math::Sin(t[J]);
  t[H] = angles[cZ] * real(0.5); c[H] = Math::Cos(t[H]); s[H] = Math::Sin(t[H]);
  
  const real cc = c[I] * c[H];
  const real cs = c[I] * s[H];
  const real sc = s[I] * c[H];
  const real ss = s[I] * s[H];
  if(EulerOrders::Yes == repeated)
  {
    angles[i] = c[J] * (cs + sc);
    angles[j] = s[J] * (cc + ss);
    angles[k] = s[J] * (cs - sc);
    quaternion->w = c[J] * (cc - ss);
  }
  else
  {
    angles[i] = c[J] * sc - s[J] * cs;
    angles[j] = c[J] * ss + s[J] * cc;
    angles[k] = c[J] * cs - s[J] * sc;
    quaternion->w = c[J] * cc + s[J] * ss;
  }
  if(EulerOrders::Odd == parity)
  {
    angles[j] *= real(-1.0);
  }
  quaternion->x = angles[cX];
  quaternion->y = angles[cY];
  quaternion->z = angles[cZ];
}

///Converts a 3x3 matrix to a quaternion (in radians).
Quat ToQuaternion(Mat3Param matrix)
{
  Quat quaternion;
  ToQuaternion(matrix, &quaternion);
  return quaternion;
}

void ToQuaternion(Mat3Param matrix, QuatPtr quaternion)
{
  ErrorIf(quaternion == nullptr, "Math - Null pointer passed for quaternion.");

  if(matrix.m00 + matrix.m11 + matrix.m22 > real(0.0))
  {
    real t = matrix.m00 + matrix.m11 + matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);

    (*quaternion)[3] = s * t;
    (*quaternion)[2] = (matrix.m10 - matrix.m01) * s;
    (*quaternion)[1] = (matrix.m02 - matrix.m20) * s;
    (*quaternion)[0] = (matrix.m21 - matrix.m12) * s;
  }
  else if(matrix.m00 > matrix.m11 && matrix.m00 > matrix.m22)
  {
    real t = matrix.m00 - matrix.m11 - matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);

    (*quaternion)[0] = s * t;
    (*quaternion)[1] = (matrix.m10 + matrix.m01) * s;
    (*quaternion)[2] = (matrix.m02 + matrix.m20) * s;
    (*quaternion)[3] = (matrix.m21 - matrix.m12) * s;
  }
  else if(matrix.m11 > matrix.m22)
  {
    real t = -matrix.m00 + matrix.m11 - matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);
    
    (*quaternion)[1] = s * t;
    (*quaternion)[0] = (matrix.m10 + matrix.m01) * s;
    (*quaternion)[3] = (matrix.m02 - matrix.m20) * s;
    (*quaternion)[2] = (matrix.m21 + matrix.m12) * s;
  }
  else
  {
    real t = -matrix.m00 - matrix.m11 + matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);

    (*quaternion)[2] = s * t;
    (*quaternion)[3] = (matrix.m10 - matrix.m01) * s;
    (*quaternion)[0] = (matrix.m02 + matrix.m20) * s;
    (*quaternion)[1] = (matrix.m21 + matrix.m12) * s;
  }
}

///Converts a 4x4 matrix to a quaternion (in radians).
Quat ToQuaternion(Mat4Param matrix)
{
  Quat quaternion;
  ToQuaternion(matrix, &quaternion);
  return quaternion;
}

void ToQuaternion(Mat4Param matrix, QuatPtr quaternion)
{
  ErrorIf(quaternion == nullptr, "Math - Null pointer passed for quaternion.");

  if(matrix.m00 + matrix.m11 + matrix.m22 > real(0.0))
  {
    real t = matrix.m00 + matrix.m11 + matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);

    (*quaternion)[3] = s * t;
    (*quaternion)[2] = (matrix.m10 - matrix.m01) * s;
    (*quaternion)[1] = (matrix.m02 - matrix.m20) * s;
    (*quaternion)[0] = (matrix.m21 - matrix.m12) * s;
  }
  else if(matrix.m00 > matrix.m11 && matrix.m00 > matrix.m22)
  {
    real t = matrix.m00 - matrix.m11 - matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);

    (*quaternion)[0] = s * t;
    (*quaternion)[1] = (matrix.m10 + matrix.m01) * s;
    (*quaternion)[2] = (matrix.m02 + matrix.m20) * s;
    (*quaternion)[3] = (matrix.m21 - matrix.m12) * s;
  }
  else if(matrix.m11 > matrix.m22)
  {
    real t = -matrix.m00 + matrix.m11 - matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);
    
    (*quaternion)[1] = s * t;
    (*quaternion)[0] = (matrix.m10 + matrix.m01) * s;
    (*quaternion)[3] = (matrix.m02 - matrix.m20) * s;
    (*quaternion)[2] = (matrix.m21 + matrix.m12) * s;
  }
  else
  {
    real t = -matrix.m00 - matrix.m11 + matrix.m22 + real(1.0);
    real s = Math::Rsqrt(t) * real(0.5);

    (*quaternion)[2] = s * t;
    (*quaternion)[3] = (matrix.m10 - matrix.m01) * s;
    (*quaternion)[0] = (matrix.m02 + matrix.m20) * s;
    (*quaternion)[1] = (matrix.m21 + matrix.m12) * s;
  }
}

Quaternion ToQuaternion(Vec3Param facing, Vec3Param up)
{
  return ToQuaternion(ToMatrix3(facing, up));
}

Quaternion ToQuaternion(Vec3Param facing, Vec3Param up, Vec3Param right)
{
  return ToQuaternion(ToMatrix3(facing, up, right));
}

Quaternion ToQuaternion(Vec3Param eulerVector)
{
  return ToQuaternion(EulerAngles(eulerVector.x, eulerVector.y, eulerVector.z, Math::EulerOrders::XYZs));
}

Quaternion ToQuaternion(real x, real y, real z)
{
  Mat3 rotMat = ToMatrix3(EulerAngles(x, y, z, Math::EulerOrders::XYZs));
  return ToQuaternion(rotMat);
}

Quaternion RotationQuaternionBetween(Vec3Param start, Vec3Param end)
{
  Vec3 a = start;
  a.AttemptNormalize();
  Vec3 b = end;
  b.AttemptNormalize();
  Vec3 axis = Math::Cross(a, b);
  float length = axis.AttemptNormalize();
  float dot = Dot(a, b);
  float angle = Math::ArcCos(dot);
  if(length == 0)
    return Quat::cIdentity;

  return ToQuaternion(axis, angle);
}

///Generates a set of orthonormal vectors from the given vectors, modifying u 
///and v.
void GenerateOrthonormalBasis(Vec3Param w, Vec3Ptr u, Vec3Ptr v)
{
  ErrorIf(u == nullptr, "Math - Null pointer passed for vector U.");
  ErrorIf(v == nullptr, "Math - Null pointer passed for vector V.");

  if((Math::Abs(w.x) >= Math::Abs(w.y)) && (Math::Abs(w.x) >= Math::Abs(w.z)))
  {
    u->x = -w.y;
    u->y = w.x;
    u->z = real(0.0);
  }
  else
  {
    u->x = real(0.0);
    u->y = w.z;
    u->z = -w.y;
  }
  Normalize(*u);
  *v = Cross(w, *u);
  Normalize(*v);
}

///Generates a set of orthonormal vectors from the given vectors while using 
///debug checks, modifies u and v
void DebugGenerateOrthonormalBasis(Vec3Param w, Vec3Ptr u, Vec3Ptr v)
{
  ErrorIf(u == nullptr, "Math - Null pointer passed for vector U.");
  ErrorIf(v == nullptr, "Math - Null pointer passed for vector V.");

  if((Math::Abs(w.x) >= Math::Abs(w.y)) && (Math::Abs(w.x) >= Math::Abs(w.z)))
  {
    u->x = -w.y;
    u->y = w.x;
    u->z = real(0.0);
  }
  else
  {
    u->x = real(0.0);
    u->y = w.z;
    u->z = -w.y;
  }
  AttemptNormalize(*u);
  *v = Cross(w, *u);
  AttemptNormalize(*v);
}

///Converts a 32-bit float into a compressed 16-bit floating point value;
///referenced from Insomniac Games math library.
half ToHalf(float value)
{
  //------------------------------------------------------------------ Constants
  //Base value for the exponent part of the 32-bit floating point number
  const s32 cFloatExponentBase = 127;

  //Base value for the exponent part of the 16-bit floating point number
  const s32 cHalfExponentBase = 15;

  //Number of bits needed to move the float's sign to the half's sign spot
  const s32 cSignShift = 16;

  //Mask to only have the shifted sign portion of the float
  const s32 cShiftedSignMask = 0x00008000;

  //Number of bits needed to move the float's exponent bits such that they 
  //occupy
  const s32 cExponentShift = 23;

  //Mask to only have the shifted exponent portion of the float
  const s32 cShiftedExponentMask = 0x000000FF;

  //The value to subtract from the exponent portion to map it to the 4-bit 
  //format
  const s32 cExponentBaseChange = cFloatExponentBase - cHalfExponentBase;

  //Mask to only have the mantissa portion of the float
  const s32 cMantissaMask = 0x007FFFFF;

  //Bit that would be set if the mantissa overflowed into the exponent portion
  const s32 cMantissaOverflowBit = 0x00800000;

  //Bit used to check if the mantissa needs to be rounded up or not, checks the
  //least significant bit of the final shifted mantissa
  const s32 cMantissaRoundingBit = 0x00001000;

  //Value used when rounding floating point values up
  const s32 cMantissaRoundingValue = 0x00002000;

  //Number of bits needed to move the mantissa bits in order for them to fit in
  //the half-float format
  const s32 cMantissaShift = 13;

  //Value of the fully shifted exponent bit-combination (all bits are set)
  const s32 cFullShiftedExponent = 0x0000008F;

  //Used to check if a float is set to infinity
  const s32 cInfinityCheck = 0x0000000;

  //Value, for half-floats, of all the exponent bits set
  const s32 cFullExponent = 0x00007C00;

  //Redundant value used for clarification
  const s32 cZeroMantissa = 0x00000000;
  //----------------------------------------------------------------------------

  //Bit interpretation of the floating point value
  s32 v = *reinterpret_cast<s32*>(&value);
  
  //Sign
  s32 s = (v >> cSignShift) & cShiftedSignMask;

  //Exponent
  s32 e = ((v >> cExponentShift) & cShiftedExponentMask) - cExponentBaseChange;

  //Mantissa
  s32 m = v & cMantissaMask;

  //Handle values in the range [0,1] (negative exponent)
  if(e <= 0)
  {
    //If the exponent part is too small then treat value as 0
    if(e < -10)
    {
      return 0;
    }

    //Since the number is so small, attempt to round it
    m = (m | cMantissaOverflowBit) >> (1 - e);

    //Check to see if rounding is needed
    if(m & cMantissaRoundingBit)
    {
      m += cMantissaRoundingValue;
    }

    return static_cast<half>(s | (m >> cMantissaShift));
  }
  //Handle infinity and NaN
  else if(e == cFullShiftedExponent)
  {
    //Result is either positive or negative infinity
    if(m == cInfinityCheck)
    {
      return static_cast<half>(s | cFullExponent | cZeroMantissa);
    }
    //Result is NaN
    else
    {
      return static_cast<half>(s | cFullExponent | (m >> cMantissaShift));
    }
  }
  else
  {
    //Check if rounding is needed
    if(m & cMantissaRoundingBit)
    {
      m += cMantissaRoundingValue;

      //Check if the rounding has overflowed into the exponent part
      if(m & cMantissaOverflowBit)
      {
        m = 0;
        e += 1;
      }
    }

    //Check to see if all of the exponent bits are set
    if(e > 30)
    {
      //Returns a signed infinity
      return static_cast<half>(s | (e << 10) | (m >> cMantissaShift));
    }

    //Normal half
    return static_cast<half>(s | (e << 10) | (m >> cMantissaShift));
  }
}

///Converts a 16-bit compressed floating point value back into a 32-bit float;
///referenced from Insomniac Games math library.
float ToFloat(half value)
{
  //------------------------------------------------------------------ Constants
  //Base value for the exponent part of the 32-bit floating point number
  const s32 cFloatExponentBase = 127;

  //Base value for the exponent part of the 16-bit floating point number
  const s32 cHalfExponentBase = 15;

  //The value to subtract from the exponent portion to map it to the 4-bit 
  //format
  const s32 cExponentBaseChange = cFloatExponentBase - cHalfExponentBase;

  //Bit mask to ensure that the shifted sign is the only value in the bit field
  s32 cShiftedSignMask = 0x00000001;

  //Bit mask to ensure that the shifted exponent is the only value in the bit
  //field
  s32 cShiftedExponentMask = 0x0000001F;

  //Bit mask to ensure that the mantissa is the only value in the bit field
  s32 cMantissaMask = 0x000003FF;

  //Bit mask to check against the most significant bit in the mantissa
  s32 cMantissaMsbMask = 0x00000400;

  //Bit mask to ensure all the exponent bits are set
  s32 cFullExponent = 0x7F800000;
  //----------------------------------------------------------------------------
  
  //Sign
  s32 s = (value >> 15) & cShiftedSignMask;

  //Exponent
  s32 e = (value >> 10) & cShiftedExponentMask;

  //Mantissa
  s32 m = value & cMantissaMask;

  //No exponent, denormalized OR zero
  if(e == 0)
  {
    //Positive or negative zero
    if(m == 0)
    {
      uint result = s << 31;
      return *reinterpret_cast<float*>(&result);
    }
    //Denormalized number
    else
    {
      //Continuously move the mantissa until the most significant bit of the
      //mantissa has been set
      while(!(m & cMantissaMsbMask))
      {
        m <<= 1;
        e -= 1;
      }
      e += 1;

      //Make sure the most significant bit of the mantissa is cleared
      m &= ~cMantissaMsbMask;
    }
  }
  //Full exponent
  else if(e == 31)
  {
    //Positive or negative infinity
    if(m == 0)
    {
      uint result = (s << 31) | cFullExponent;
      return *reinterpret_cast<float*>(&result);
    }
    //NaN
    else
    {
      uint result = (s << 31) | cFullExponent | (m << 13);
      return *reinterpret_cast<float*>(&result);
    }
  }
  
  //Normalized number
  e += cExponentBaseChange;
  m <<= 13;

  //Create the float
  uint result = (s << 31) | (e << 23) | m;
  return *reinterpret_cast<float*>(&result);
}

//----------------------------------------------------------- Rotation Functions
real Angle2D(Vec3Param a)
{
  return ArcTan2(a.y, a.x);
}

Vector2 RotateTowards(Vec2Param a, Vec2Param b, real maxAngle)
{
  Vec2 an = a.Normalized();
  Vec2 bn = b.Normalized();
  return GenericTowards(an, bn, maxAngle);
}

Vector3 RotateTowards(Vec3Param a, Vec3Param b, real maxAngle)
{
  Vector3 an = a.Normalized();
  Vector3 bn = b.Normalized();
  return GenericTowards(an, bn, maxAngle);
}

Quat RotateTowards(QuatParam a, QuatParam b, real maxAngle)
{
  return GenericTowards(a, b, maxAngle);
}

Vector2 SafeRotateTowards(Vec2Param a, Vec2Param b, real maxAngle)
{
  Vec2 an = a;
  Vec2 bn = b;
  an.AttemptNormalize();
  bn.AttemptNormalize();
  return SafeGenericTowards(an, bn, maxAngle);
}

Vector3 SafeRotateTowards(Vec3Param a, Vec3Param b, real maxAngle)
{
  Vec3 an = a;
  Vec3 bn = b;
  an.AttemptNormalize();
  bn.AttemptNormalize();
  return SafeGenericTowards(an, bn, maxAngle);
}

// Get the rotation angle between two vectors (in radians)
real SignedAngle(Vec3Param a, Vec3Param b, Vec3Param up)
{
  // Get the right vector
  Vec3 right = Math::Cross(a, up);
  right.AttemptNormalize();

  // Get the forward and right dot products
  real forwardDot = Math::Clamp(Math::Dot(a, b), real(-1.0), real(1.0));
  real rightDot = Math::Clamp(Math::Dot(right, b), real(-1.0), real(1.0));

  // Get the actual angle from the forward dot product
  real finalAngle = Math::ArcCos(forwardDot);

  // If we're actually on the left side...
  if(rightDot > real(0.0))
  {
    // Compute the real final angle given the quadrant it's in (kinda like atan2)
    finalAngle = -finalAngle;
  }

  // Return the finally computed angle
  return finalAngle;
}

Vector3 RotateVector(Vec3Param a, Vec3Param axis, real radians)
{
  Mat3 rot = Math::ToMatrix3(axis, radians);
  return Math::Transform(rot, a);
}

Quat EulerDegreesToQuat(Vec3Param eulerDegrees)
{
  Math::EulerAngles angle(Math::DegToRad(eulerDegrees[0]), 
                          Math::DegToRad(eulerDegrees[1]), 
                          Math::DegToRad(eulerDegrees[2]), 
                          Math::EulerOrders::XYZs);

  return Math::ToQuaternion(angle);
}

Vector3 QuatToEulerDegrees(QuatParam rotation)
{
  Math::EulerAngles angles(rotation, Math::EulerOrders::XYZs);
  Vector3 newData = Vector3::cZero;
  newData[0] = Math::Round(Math::RadToDeg(angles[0]), -1);
  newData[1] = Math::Round(Math::RadToDeg(angles[1]), -1);
  newData[2] = Math::Round(Math::RadToDeg(angles[2]), -1);
  return newData;
}

}// namespace Math
