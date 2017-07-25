///////////////////////////////////////////////////////////////////////////////
///
/// \file Shapes.cpp
/// Implementation of the Ray, Segment, Triangle, Obb, Ellipsoid, Cylinder
/// and Capsule classes. Also Contains conversion functions to and from shapes.
///
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//rotation for an object represented by two extreme points and a radius
void RotateCylinderOrCapsule(Vec3Ref pointA, Vec3Ref pointB, real& radius,
                             Mat3Param rotation)
{
  Vec3 center = (pointA + pointB) * real(.5);
  Vec3 axis = pointA - center;
  Math::Transform(rotation, axis);

  pointA = center + axis;
  pointB = center - axis;
}

//translation for an object represented by two extreme points and a radius
void TranslateCylinderOrCapsule(Vec3Ref pointA, Vec3Ref pointB, real& radius,
                                Vec3Param translation)
{
  pointA += translation;
  pointB += translation;
}

//-------------------------------------------------------------------------- Ray
Ray::Ray()
{
  Start = Vec3::cZero;
  Direction = Vec3::cZAxis;
}

Ray::Ray(Vec3Param start, Vec3Param direction)
{
  Start = start;
  Direction = direction;
}

Vec3 Ray::GetPoint(real t) const
{
  return Start + Direction * t;
}

real Ray::GetTValue(Vec3Param point) const
{
  return Math::Dot(point - Start, Direction);
}

Vec3 Ray::ClosestPoint(Vec3Param point)
{
  Vec3 result = point;
  Intersection::ClosestPointOnRayToPoint(Start, Direction, &result);
  return result;
}

Ray Ray::TransformInverse(Mat4Param transformation) const
{
  Mat4 invTransform = transformation.Inverted();

  Ray ret;
  ret.Start = Math::TransformPoint(invTransform, Start);
  ret.Direction = Math::TransformNormal(invTransform, Direction);
  return ret;
}

Ray Ray::Transform(Mat4Param transformation) const
{
  Ray ret;
  ret.Start = Math::TransformPoint(transformation, Start);
  ret.Direction = Math::TransformNormal(transformation, Direction);
  return ret;
}

Ray Ray::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//---------------------------------------------------------------------- Segment
Segment::Segment(Vec3Param start, Vec3Param end)
{
  Start = start;
  End = end;
}

Vec3 Segment::GetPoint(real t) const
{
  Vec3 direction = End - Start;
  return Start + direction * t;
}

real Segment::GetTValue(Vec3Param point) const
{
  Vec3 direction = End - Start;
  return Math::Dot(point - Start, direction);
}

Segment Segment::Transform(Mat4Param transformation) const
{
  Segment ret;
  ret.Start = Math::TransformPoint(transformation, Start);
  ret.End = Math::TransformPoint(transformation, End);
  return ret;
}

Segment Segment::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//--------------------------------------------------------------------- Triangle
Triangle::Triangle(Vec3Param point1, Vec3Param point2, Vec3Param point3)
{
  p0 = point1;
  p1 = point2;
  p2 = point3;
}

Vec3& Triangle::operator[](uint index)
{
  return *(&p0 + index);
}

const Vec3& Triangle::operator[](uint index)const
{
  return *(&p0 + index);
}

Vec3 Triangle::GetCenter() const
{
  return (p0 + p1 + p2) / real(3.0);
}

Vec3 Triangle::GetRawNormal() const
{
  Vec3 v1v0 = p1 - p0;
  Vec3 v2v0 = p2 - p0;
  return Math::Cross(v1v0,v2v0);
}

Vec3 Triangle::GetNormal() const
{
  return GetRawNormal().Normalized();
}

void Triangle::GetCenter(Vec3Ref center) const
{
  center = GetCenter();
}

void Triangle::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportTriangle(direction, (*this)[0], (*this)[1], (*this)[2],
                            support);
}

real Triangle::GetArea() const
{
  Vec3 e1 = p1 - p0;
  Vec3 e2 = p2 - p0;
  Vec3 e3 = Math::Cross(e1, e2);
  float area = e3.Length() * 0.5f;
  return area;
}

void Triangle::GetBarycentric(Vec3Param point, real& u, real& v, real& w)
{
  Vec3 barycentricCoordinates;
  Geometry::BarycentricTriangle(point, (*this)[0], (*this)[1], (*this)[2],
                                &barycentricCoordinates);
  u = barycentricCoordinates.x;
  v = barycentricCoordinates.y;
  w = barycentricCoordinates.z;
}

Vec3 Triangle::GetPointFromBarycentric(real u, real v, real w)
{
  return u * (*this)[0] + v * (*this)[1] + w * (*this)[2];
}

Triangle Triangle::Transform(Mat4Param transformation) const
{
  Triangle ret;
  ret.p0 = Math::TransformPoint(transformation,p0);
  ret.p1 = Math::TransformPoint(transformation,p1);
  ret.p2 = Math::TransformPoint(transformation,p2);
  return ret;
}

Triangle Triangle::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//-------------------------------------------------------------------SweptTriangle
SweptTriangle::SweptTriangle(Triangle& triangle, Vec3Param scaledDirection)
  : BaseTri(triangle)
  , ScaledDir(scaledDirection)
{
}

SweptTriangle::SweptTriangle(Vec3Param point1, Vec3Param point2, Vec3Param point3, Vec3Param scaledDirection)
  : BaseTri(point1, point2, point3)
  , ScaledDir(scaledDirection)
{
}

Vec3& SweptTriangle::operator[](uint index)
{
  return BaseTri[index];
}

const Vec3& SweptTriangle::operator[](uint index) const
{
  return BaseTri[index];
}

Vec3 SweptTriangle::GetCenter() const
{
  return BaseTri.GetCenter() + ScaledDir * 0.5f;
}

void SweptTriangle::GetCenter(Vec3Ref center) const
{
  center = GetCenter();
}

real SweptTriangle::GetVolume() const
{
  return BaseTri.GetArea() * ScaledDir.Length();
}

void SweptTriangle::Support(Vec3Param direction, Vec3Ptr support) const
{
  BaseTri.Support(direction, support);
  if (Math::Dot(direction, ScaledDir) > 0.0f)
    *support += ScaledDir;
}

SweptTriangle SweptTriangle::Transform(Mat4Param transformation) const
{
  SweptTriangle ret;
  ret.BaseTri = BaseTri.Transform(transformation);
  ret.ScaledDir = Math::TransformNormal(transformation, ScaledDir);
  return ret;
}

SweptTriangle SweptTriangle::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//-------------------------------------------------------------------Tetrahedra

Tetrahedron::Tetrahedron(Vec3Param point1, Vec3Param point2, Vec3Param point3, Vec3Param point4)
{
  p0 = point1;
  p1 = point2;
  p2 = point3;
  p3 = point4;
}

Vec3& Tetrahedron::operator[](uint index)
{
  return *(&p0 + index);
}

const Vec3& Tetrahedron::operator[](uint index)const
{
  return *(&p0 + index);
}

void Tetrahedron::GetCenter(Vec3Ref center) const
{
  Vec4 coordinates;
  coordinates.Splat(real(.25));
  center = GetPointFromBarycentric(coordinates);
}

void Tetrahedron::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportTetrahedron(direction,p0,p1,p2,p3,support);
}

real Tetrahedron::GetSignedVolume() const
{
  Mat3 e;
  e.SetBasis(0,p1 - p0);
  e.SetBasis(1,p2 - p0);
  e.SetBasis(2,p3 - p0);
  real oneSixth = real(1.0) / real(6.0);
  return oneSixth * e.Determinant();
}

real Tetrahedron::GetVolume() const
{
  return Math::Abs(GetSignedVolume());
}

void Tetrahedron::GetBarycentric(Vec3Param point, Vec4Ref coordinates) const
{
  Vec3 barycentricCoordinates;
  Geometry::BarycentricTetrahedron(point, p0, p1, p2, p3,
                                  &coordinates);
}

Vec3 Tetrahedron::GetPointFromBarycentric(Vec4Param coordinates) const
{
  Vec4 row1(p0.x,p1.x,p2.x,p3.x);
  Vec4 row2(p0.y,p1.y,p2.y,p3.y);
  Vec4 row3(p0.z,p1.z,p2.z,p3.z);

  Vec3 result;
  result.x = Math::Dot(row1,coordinates);
  result.y = Math::Dot(row2,coordinates);
  result.z = Math::Dot(row3,coordinates);

  return result;
}

Tetrahedron Tetrahedron::Transform(Mat4Param transformation) const
{
  Tetrahedron result;
  result.p0 = Math::TransformPoint(transformation,p0);
  result.p1 = Math::TransformPoint(transformation,p1);
  result.p2 = Math::TransformPoint(transformation,p2);
  result.p3 = Math::TransformPoint(transformation,p3);
  return result;
}

Tetrahedron Tetrahedron::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//-------------------------------------------------------------------------- Obb

void Obb::GetCenter(Vec3Ref center) const
{
  center = Center;
}

void Obb::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportObb(direction,Center,HalfExtents,Basis,support);
}

Obb Obb::Transform(Mat4Param transformation) const
{
  Mat4 t = Math::BuildTransform(Center,Basis,HalfExtents);
  Mat4 worldT = transformation * t;

  Obb ret;
  worldT.Decompose(&ret.HalfExtents,&ret.Basis,&ret.Center);
  return ret;
}

Obb Obb::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

Vec3 Obb::GetCenter() const
{
  Vec3 center;
  GetCenter(center);
  return center;
}

//-------------------------------------------------------------------- Ellipsoid

void Ellipsoid::GetCenter(Vec3Ref center) const
{
  center = Center;
}

void Ellipsoid::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportEllipsoid(direction,Center,Radii,Basis,support);
}

Ellipsoid Ellipsoid::Transform(Mat4Param transformation) const
{
  Mat4 t = Math::BuildTransform(Center,Basis,Radii);
  Mat4 worldT = transformation * t;

  Ellipsoid ret;
  worldT.Decompose(&ret.Radii,&ret.Basis,&ret.Center);
  return ret;
}

Ellipsoid Ellipsoid::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//--------------------------------------------------------------------- Cylinder
void Cylinder::Rotate(Mat3Param rotation)
{
  RotateCylinderOrCapsule(PointA, PointB, Radius, rotation);
}

void Cylinder::Translate(Vec3Param translation)
{
  TranslateCylinderOrCapsule(PointA, PointB, Radius, translation);
}

void Cylinder::GetCenter(Vec3Ref center) const
{
  center = (PointA + PointB) * .5f;
}

void Cylinder::Support(Vec3Param direction, Vec3Ptr support) const
{
  //fill out once I get the function from ben...
  Geometry::SupportCylinder(direction, PointA, PointB, Radius, support);
}

Cylinder Cylinder::Transform(Mat4Param transformation) const
{
  Mat4 localScale;
  localScale.Scale(Radius,Radius,Radius);
  Mat4 worldTransform = transformation * localScale;

  Vec3 scale, translation;
  Mat3 rot;
  worldTransform.Decompose(&scale,&rot,&translation);

  Cylinder ret;
  ret.PointA = Math::TransformPoint(transformation,PointA);
  ret.PointB = Math::TransformPoint(transformation,PointB);
  ret.Radius = scale.x;
  return ret;
}

Cylinder Cylinder::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

//---------------------------------------------------------------------- Capsule
Vec3 Capsule::GetCenter() const
{
  return (PointA + PointB) * real(.5);
}

Vec3 Capsule::GetPrimaryAxis() const
{
  return (PointA - PointB) * real(.5);
}

void Capsule::Rotate(Mat3Param rotation)
{
  RotateCylinderOrCapsule(PointA, PointB, Radius, rotation);
}

void Capsule::Translate(Vec3Param translation)
{
  TranslateCylinderOrCapsule(PointA, PointB, Radius, translation);
}

void Capsule::GetCenter(Vec3Ref center) const
{
  center = (PointA + PointB) * .5f;
}

void Capsule::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportCapsule(direction,PointA,PointB,Radius,support);
}

Capsule Capsule::Transform(Mat4Param transformation) const
{
  Mat4 localScale;
  localScale.Scale(Radius,Radius,Radius);
  Mat4 worldTransform = transformation * localScale;

  Vec3 scale, translation;
  Mat3 rot;
  worldTransform.Decompose(&scale,&rot,&translation);

  Capsule ret;
  ret.PointA = Math::TransformPoint(transformation,PointA);
  ret.PointB = Math::TransformPoint(transformation,PointB);
  ret.Radius = scale.x;
  return ret;
}

Capsule Capsule::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

}//namespace Zero
