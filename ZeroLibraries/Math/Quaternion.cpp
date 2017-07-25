///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{
typedef Quaternion Quat;

const Quaternion Quaternion::cIdentity = Quaternion(real(0.0), real(0.0), 
                                                    real(0.0), real(1.0));

Quaternion::Quaternion(real xx, real yy, real zz, real ww)
{
  x = xx;
  y = yy;
  z = zz;
  w = ww;
}

real& Quaternion::operator[](uint index)
{
  ErrorIf(index > 3, "Quaternion - Subscript out of range.");
  return V4()[index];
}

real Quaternion::operator[](uint index) const
{
  ErrorIf(index > 3, "Quaternion - Subscript out of range.");
  return V4()[index];
}

Quaternion Quaternion::operator-() const
{
  return Quaternion(-x, -y, -z, -w);
}

void Quaternion::operator*=(real rhs)
{
  x *= rhs;
  y *= rhs;
  z *= rhs;
  w *= rhs;
}

void Quaternion::operator/=(real rhs)
{
  ErrorIf(Math::IsZero(rhs), "Quaternion - Division by zero.");
  real reciprocal = real(1.0) / rhs;
  x *= reciprocal;
  y *= reciprocal;
  z *= reciprocal;
  w *= reciprocal;
}

Quaternion Quaternion::operator*(real rhs) const
{
  return Quat(x * rhs, y * rhs, z * rhs, w * rhs);
}

Quaternion Quaternion::operator/(real rhs) const
{
  ErrorIf(Math::IsZero(rhs), "Quaternion - Division by zero.");
  real reciprocal = real(1.0) / rhs;
  return Quat(x * reciprocal, y * reciprocal, z * reciprocal, w * reciprocal);
}

void Quaternion::operator+=(QuatParam rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  w += rhs.w;
}

void Quaternion::operator-=(QuatParam rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  w -= rhs.w;
}

Quaternion Quaternion::operator+(QuatParam rhs) const
{
  return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

Quaternion Quaternion::operator-(QuatParam rhs) const
{
  return Quat(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

void Quaternion::operator*=(QuatParam rhs)
{
  *this = Quaternion::Multiply(*this, rhs);
}

Quaternion Quaternion::operator*(QuatParam rhs) const
{
  return Quaternion::Multiply(*this, rhs);
}

bool Quaternion::operator==(QuatParam rhs) const
{
  return Equal(x, rhs.x) &&
         Equal(y, rhs.y) &&
         Equal(z, rhs.z) &&
         Equal(w, rhs.w);
}

bool Quaternion::operator!=(QuatParam rhs) const
{
  return !(*this == rhs);
}

void Quaternion::SetIdentity()
{
  *this = cIdentity;
}

void Quaternion::Set(real xx, real yy, real zz, real ww)
{
  x = xx;
  y = yy;
  z = zz;
  w = ww;
}

bool Quaternion::Valid() const
{
  return Math::IsValid(x) && Math::IsValid(y) && 
         Math::IsValid(z) && Math::IsValid(w);
}

Vector3& Quaternion::V3()
{
  return *(Vector3*)this;
}

Vector4& Quaternion::V4()
{
  return *(Vector4*)this;
}

const Vector3& Quaternion::V3() const
{
  return *(Vector3*)this;
}

const Vector4& Quaternion::V4() const
{
  return *(Vector4*)this;
}

real Quaternion::Dot(QuatParam lhs, QuatParam rhs)
{
  Vector4 lhsV4 = lhs.V4();
  Vector4 rhsV4 = rhs.V4();
  return Vector4::Dot(lhsV4, rhsV4);
}

real Quaternion::Length(QuatParam value)
{
  real sqLength = Quaternion::LengthSq(value);
  return Math::Sqrt(sqLength);
}

real Quaternion::LengthSq(QuatParam value)
{
  return Quaternion::Dot(value, value);
}

real Quaternion::Normalize(QuatRef value)
{
  real lengthSq = Quaternion::LengthSq(value);
  if(Math::Equal(lengthSq, real(0.0)))
  {
    value.x = real(0.0);
    value.y = value.x;
    value.z = value.x;
    value.w = real(1.0);
    return real(0.0);
  }
  
  real length = Math::Rsqrt(lengthSq);
  value *= length;
  return length;
}

Quaternion Quaternion::Normalized(QuatParam value)
{
  Quaternion result = value;
  Quaternion::Normalize(result);
  return result;
}

void Quaternion::Conjugate(QuatRef value)
{
  value.x *= -real(1.0);
  value.y *= -real(1.0);
  value.z *= -real(1.0);
}

Quaternion Quaternion::Conjugated(QuatParam value)
{
  Quaternion result = value;
  Quaternion::Conjugate(result);
  return result;
}

void Quaternion::Invert(QuatRef value)
{
  Quaternion::Conjugate(value);
  real lengthSq = Quaternion::LengthSq(value);

  ErrorIf(Math::IsZero(lengthSq), "Quaternion - Division by zero.");
  value /= lengthSq;
}

Quaternion Quaternion::Inverted(QuatParam value)
{
  Quaternion result = value;
  Quaternion::Invert(result);
  return result;
}

Quaternion Quaternion::Multiply(QuatParam lhs, QuatParam rhs)
{
  Quat result;
  result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
  result.y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z;
  result.z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x;
  result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
  return result;
}

Vector3 Quaternion::Multiply(QuatParam lhs, Vec3Param rhs)
{
  Quaternion pureQuat = Quat(rhs.x, rhs.y, rhs.z, real(0.0));
  Quaternion conjugate = Quaternion::Conjugated(lhs);
  Quaternion result(lhs);
  result *= pureQuat;
  result *= conjugate;
  return Vector3(result.x, result.y, result.z);
}

Quaternion Quaternion::Lerp(QuatParam start, QuatParam end, real tValue)
{
  Quaternion result = Math::Lerp<Quaternion, real>(start, end, tValue);
  Quaternion::Normalize(result);
  return result;
}

Quaternion Quaternion::Slerp(QuatParam start, QuatParam end, real tValue)
{
  WarnIf(!Math::InRange(tValue, real(0.0), real(1.0)), 
         "Quaternion - Interpolation value is not in the range of [0, 1]");

  //
  // Quaternion Interpolation With Extra Spins, pp. 96f, 461f
  // Jack Morrison, Graphics Gems III, AP Professional
  //

  const real cSlerpEpsilon = real(0.00001);

  bool flip;

  real cosTheta = Dot(start, end); 

  //Check to ensure that the shortest path is taken (cosine of the angle between 
  //the two quaternions is positive).
  flip = cosTheta < real(0.0);
  if(flip)
  {
    cosTheta = -cosTheta;
  }

  real startVal, endVal;
  if((real(1.0) - cosTheta) > cSlerpEpsilon)
  {
    real theta = Math::ArcCos(cosTheta);
    real sinTheta = Math::Sin(theta);
    startVal = real(Math::Sin((real(1.0) - tValue) * theta) / sinTheta);
    endVal = real(Math::Sin(tValue * theta) / sinTheta);
  }
  else
  {
    startVal = real(1.0) - tValue;
    endVal = real(tValue);
  }

  if(flip)
  {
    endVal = -endVal;
  }

  return Quaternion(startVal * start.x + endVal * end.x,
                    startVal * start.y + endVal * end.y,
                    startVal * start.z + endVal * end.z,
                    startVal * start.w + endVal * end.w);
}

Quaternion Quaternion::Exponent(QuatParam value)
{
  real angle = value.V3().Length();
  Quat result(value.x, value.y, value.z, real(0.0));

  if(Math::Abs(angle) > Math::Epsilon())
  {
    result.w = Math::Cos(angle);
    angle = Math::Sin(angle) / angle;
    result.x *= angle;
    result.y *= angle;
    result.z *= angle;
  }
  return result;
}

Quaternion Quaternion::Logarithm(QuatParam value)
{
  Quat result(value.x, value.y, value.z, real(0.0));
  real theta = Math::ArcCos(value.w);
  real sinTheta = Math::Sin(theta);

  if(Math::Abs(sinTheta) > Math::Epsilon())
  {
    theta = theta / sinTheta;
    result.x *= theta;
    result.y *= theta;
    result.z *= theta;
  }
  return result;
}

real Quaternion::AngleBetween(QuatParam a, QuatParam b)
{
  real dot = Dot(a, b);
  dot = Math::Clamp(dot, real(-1.0), real(1.0));
  //quaternions are a 2-1 mapping, so we could get a rotation that is 400 degrees
  //instead of 40 degrees, to fix this we can simply abs the dot product. This works
  //out because we convert our initial [0,360] range to [0,180] then scale up by 2 (2-1 mapping).
  real correctedDot = Math::Abs(dot);
  real angle = real(2.0) * Math::ArcCos(correctedDot);
  return angle;
}

Quaternion Quaternion::Integrate(QuatParam rotation, Vec3Param angularVelocity, real dt)
{
  Quat result = rotation;
  
  Quat velocityQuat(angularVelocity.x * dt, angularVelocity.y * dt, angularVelocity.z * dt, real(0.0));
  velocityQuat *= rotation;
  
  result.x += real(0.5) * velocityQuat.x;
  result.y += real(0.5) * velocityQuat.y;
  result.z += real(0.5) * velocityQuat.z;
  result.w += real(0.5) * velocityQuat.w;
  return result;
}

real Quaternion::Dot(QuatParam rhs) const
{
  return Quaternion::Dot(*this, rhs);
}

real Quaternion::Length() const
{
  return Quaternion::Length(*this);
}

real Quaternion::LengthSq() const
{
  return Quaternion::LengthSq(*this);
}

real Quaternion::Normalize()
{
  return Quaternion::Normalize(*this);
}

Quaternion Quaternion::Normalized() const
{
  return Quaternion::Normalized(*this);
}

void Quaternion::Conjugate()
{
  Quaternion::Conjugate(*this);
}

Quaternion Quaternion::Conjugated() const
{
  return Quaternion::Conjugated(*this);
}

void Quaternion::Invert()
{
  Quaternion::Invert(*this);
}

Quaternion Quaternion::Inverted() const
{
  return Quaternion::Inverted(*this);
}

//-------------------------------------------------------------------Global Functions
Quaternion operator*(real lhs, QuatParam rhs)
{
  return rhs * lhs;
}

real Dot(QuatParam lhs, QuatParam rhs)
{
  return Quaternion::Dot(lhs, rhs);
}

real Length(QuatParam quaternion)
{
  return Quaternion::Length(quaternion);
}

real LengthSq(QuatParam quaternion)
{
  return Quaternion::LengthSq(quaternion);
}

void Normalize(QuatRef quaternion)
{
  Quaternion::Normalize(quaternion);
}

Quaternion Normalized(QuatParam quaternion)
{
  return Quaternion::Normalized(quaternion);
}

Quaternion Multiply(QuatParam lhs, QuatParam rhs)
{
  return Quaternion::Multiply(lhs, rhs);
}

Vector3 Multiply(QuatParam lhs, Vec3Param rhs)
{
  return Quaternion::Multiply(lhs, rhs);
}

Quaternion Lerp(QuatParam start, QuatParam end, real tValue)
{
  return Quaternion::Lerp(start, end, tValue);
}

Quaternion Slerp(QuatParam start, QuatParam end, real tValue)
{
  return Quaternion::Slerp(start, end, tValue);
}

real AngleBetween(QuatParam a, QuatParam b)
{
  return Quaternion::AngleBetween(a, b);
}

//-------------------------------------------------------------------Legacy
Quaternion CreateDiagonalizer(Mat3Param matrix)
{
  const uint cMaxSteps = 50;

  Quaternion quat(real(0.0), real(0.0), real(0.0), real(1.0));
  Matrix3 quatMatrix;
  Matrix3 diagMatrix;
  for(uint i = 0; i < cMaxSteps; ++i)
  {
    ToMatrix3(quat, &quatMatrix);
    diagMatrix = Multiply(Multiply(quatMatrix, matrix), quatMatrix.Transposed());

    //Elements not on the diagonal
    Vector3 offDiag(diagMatrix(1, 2), diagMatrix(0, 2), diagMatrix(0, 1));

    //Magnitude of the off-diagonal elements
    Vector3 magDiag = Abs(offDiag);

    //Index of the largest element 
    uint k = ((magDiag.x > magDiag.y) && (magDiag.x > magDiag.z)) ? 0 :
             ((magDiag.y > magDiag.z) ? 1 : 2);
    uint k1 = (k + 1) % 3;
    uint k2 = (k + 2) % 3;

    //Diagonal already
    if(offDiag[k] == real(0.0))
    {
      break;
    }

    real theta = (diagMatrix(k2, k2) - diagMatrix(k1, k1)) / 
                 (real(2.0) * offDiag[k]);
    real sign = Math::GetSign(theta);
    
    //Make theta positive
    theta *= sign;

    //Large term in T
    real thetaTerm = theta < real(1e6) ? Math::Sqrt(Math::Sq(theta) + real(1.0))
                                       : theta;

    //Sign(T) / (|T| + sqrt(T^2 + 1))
    real t = sign / (theta + thetaTerm);

    //c = 1 / (t^2 + 1)      t = s / c
    real c = real(1.0) / Math::Sqrt(Math::Sq(t) + real(1.0));

    //No room for improvement - reached machine precision.
    if(c == real(1.0))
    {
      break;
    }

    //Jacobi rotation for this iteration
    Quaternion jacobi(real(0.0), real(0.0), real(0.0), real(0.0));

    //Using 1/2 angle identity sin(a/2) = sqrt((1-cos(a))/2)
    jacobi[k] = sign * Math::Sqrt((real(1.0) - c) / real(2.0));

    //Since our quat-to-matrix convention was for v*M instead of M*v
    jacobi.w = Math::Sqrt(real(1.0) - Math::Sq(jacobi[k]));

    //Reached limits of floating point precision
    if(jacobi.w == real(1.0))
    {
      break;
    }

    quat *= jacobi;
    Quaternion::Normalize(quat);
  }

  return quat;
}

}// namespace Math
