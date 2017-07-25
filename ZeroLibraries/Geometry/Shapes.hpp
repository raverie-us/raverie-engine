///////////////////////////////////////////////////////////////////////////////
///
/// \file Shapes.hpp
/// Declaration of the Ray, Segment, Triangle, Obb, Ellipsoid, Cylinder
/// and Capsule classes. Also Contains conversion functions to and from shapes.
///
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//add closest point on for each

//-------------------------------------------------------------------------- Ray
///An object to represent a ray. Used to simplify ray usage and to more easily
///deal with intersection functions.
struct Ray
{
  Ray();
  Ray(Vec3Param start, Vec3Param direction);

  Vec3 GetPoint(real t) const;
  real GetTValue(Vec3Param point) const;

  Vec3 ClosestPoint(Vec3Param point);

  Ray TransformInverse(Mat4Param transformation) const;
  Ray Transform(Mat4Param transformation) const;
  Ray UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Ray TransformedShapeType;

  Vec3 Start;
  Vec3 Direction;
};

typedef const Ray& RayParam;

//---------------------------------------------------------------------- Segment
///An object to represent a segment. Used to simplify segment usage and to more
///easily deal with intersection functions.
struct Segment
{
  Segment() {}
  Segment(Vec3Param start, Vec3Param end);

  Vec3 GetPoint(real t) const;
  real GetTValue(Vec3Param point) const;

  Segment Transform(Mat4Param transformation) const;
  Segment UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Segment TransformedShapeType;

  Vec3 Start;
  Vec3 End;
};

//--------------------------------------------------------------------- Triangle
///An object to represent a triangle. Used to simplify triangle usage and to
///more easily deal with intersection functions.
struct Triangle
{
  Triangle() {}
  Triangle(Vec3Param point1, Vec3Param point2, Vec3Param point3);

  /// Index operators.
  Vec3& operator[](uint index);
  const Vec3& operator[](uint index)const;

  /// Returns the averaged center of the triangle
  Vec3 GetCenter() const;
  /// Returns the direct result of cross(p1-p0,p2-p0) un-normalized.
  /// Can be used as a rough area estimate to detect degenerate triangles.
  Vec3 GetRawNormal() const;
  /// Returns the normal of this triangle
  Vec3 GetNormal() const;

  /// Sets the passed in vector to the center of the triangle.
  void GetCenter(Vec3Ref center) const;
  /// Get the point on the shape furthest in the given direction,
  /// stored in the parameter.
  void Support(Vec3Param direction, Vec3Ptr support) const;
  /// Get the vector to the shape's known future position.
  void GetTranslation(Vec3Ref translation) const {}

  /// Returns the area of the triangle.
  real GetArea() const;

  void GetBarycentric(Vec3Param point, real& u, real& v, real& w);
  Vec3 GetPointFromBarycentric(real u, real v, real w);

  Triangle Transform(Mat4Param transformation) const;
  Triangle UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Triangle TransformedShapeType;

  Vec3 p0, p1, p2;
};

typedef Array<Triangle> TriangleArray;

//-------------------------------------------------------------------SweptTriangle
/// A triangle that has been swept along an axis vector.
/// Used to make thick triangles, primarily for the height map.
struct SweptTriangle
{
  SweptTriangle() {}
  SweptTriangle(Triangle& triangle, Vec3Param scaledDirection);
  SweptTriangle(Vec3Param point1, Vec3Param point2, Vec3Param point3, Vec3Param scaledDirection);

  /// Index operators.
  Vec3& operator[](uint index);
  const Vec3& operator[](uint index) const;

  /// Returns the averaged center of the prism.
  Vec3 GetCenter() const;
  /// Sets the passed in vector to the center of the prism.
  void GetCenter(Vec3Ref center) const;
  /// Returns the volume of the prism.
  real GetVolume() const;

  /// Get the point on the shape furthest in the given direction,
  /// stored in the parameter.
  void Support(Vec3Param direction, Vec3Ptr support) const;

  SweptTriangle Transform(Mat4Param transformation) const;
  SweptTriangle UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef SweptTriangle TransformedShapeType;

  Triangle BaseTri;
  Vec3 ScaledDir;
};

//-------------------------------------------------------------------Tetrahedron
///An object to represent a tetrahedra. Used to simplify tetrahedra usage and to
///more easily deal with intersection functions.
struct Tetrahedron
{
  Tetrahedron() {}
  Tetrahedron(Vec3Param point1, Vec3Param point2, Vec3Param point3, Vec3Param point4);

  /// Index operators.
  Vec3& operator[](uint index);
  const Vec3& operator[](uint index) const;

  /// Sets the passed in vector to the center of the tetrahedra.
  void GetCenter(Vec3Ref center) const;
  /// Get the point on the shape furthest in the given direction,
  /// stored in the parameter.
  void Support(Vec3Param direction, Vec3Ptr support) const;

  /// Returns the signed volume of the tetrahedra.
  //(Positive when p0, p1, p2 form a counter-clockwise cycle seen from p3)
  real GetSignedVolume() const;
  /// Returns the volume of the tetrahedra.
  real GetVolume() const;

  void GetBarycentric(Vec3Param point, Vec4Ref coordinates) const;
  Vec3 GetPointFromBarycentric(Vec4Param coordinates) const;

  Tetrahedron Transform(Mat4Param transformation) const;
  Tetrahedron UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Tetrahedron TransformedShapeType;

  Vec3 p0, p1, p2, p3;
};

//-------------------------------------------------------------------------- Obb
///An object to represent a Oriented Bounding Box. Used to simplify OBB usage
///and to more easily deal with intersection functions.
struct Obb
{
  Obb() {}
  Obb(Vec3Param center, Vec3Param radii, Mat3Param basis)
    : Center(center), HalfExtents(radii), Basis(basis)
  {
  }

  void GetCenter(Vec3Ref center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;
  Vec3 GetCenter() const;

  Obb Transform(Mat4Param transformation) const;
  Obb UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Obb TransformedShapeType;

  Vec3 Center;
  Vec3 HalfExtents;
  Mat3 Basis;
};

//-------------------------------------------------------------------- Ellipsoid
///An object to represent a ellipsoid. Used to simplify ellipsoid usage and to
///more easily deal with intersection functions.
struct Ellipsoid
{
  Ellipsoid() {}
  Ellipsoid(Vec3Param center, Vec3Param radii, Mat3Param basis)
    : Center(center), Radii(radii), Basis(basis)
  {
  }

  void GetCenter(Vec3Ref center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;

  Ellipsoid Transform(Mat4Param transformation) const;
  Ellipsoid UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Ellipsoid TransformedShapeType;

  Vec3 Center;
  Vec3 Radii;
  Mat3 Basis;
};

//--------------------------------------------------------------------- Cylinder
///An object to represent a cylinder. Used to simplify cylinder
///usage and to more easily deal with intersection functions.
struct Cylinder
{
  Cylinder() {}
  Cylinder(Vec3Param pointA, Vec3Param pointB, real radius)
    : PointA(pointA), PointB(pointB), Radius(radius)
  {
  }

  Cylinder(Vec3Param center, Vec3Param direction, real length, real radius)
  {
    PointA = center + direction * length;
    PointB = center - direction * length;
    Radius = radius;
  }

  void Rotate(Mat3Param rotation);
  void Translate(Vec3Param translation);

  void GetCenter(Vec3Ref center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;

  Cylinder Transform(Mat4Param transformation) const;
  Cylinder UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Cylinder TransformedShapeType;

  Vec3 PointA;
  Vec3 PointB;
  real Radius;
};

//---------------------------------------------------------------------- Capsule
///An object to represent a capsule. Used to simplify capsule
///usage and to more easily deal with intersection functions.
struct Capsule
{
  Capsule() {}
  Capsule(Vec3Param pointA, Vec3Param pointB, real radius)
    : PointA(pointA), PointB(pointB), Radius(radius)
  {
  }

  Capsule(Vec3Param center, Vec3Param direction, real length, real radius)
  {
    PointA = center + direction * length;
    PointB = center - direction * length;
    Radius = radius;
  }

  Vec3 GetCenter() const;
  Vec3 GetPrimaryAxis() const;

  void Rotate(Mat3Param rotation);
  void Translate(Vec3Param translation);

  void GetCenter(Vec3Ref center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;

  Capsule Transform(Mat4Param transformation) const;
  Capsule UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Capsule TransformedShapeType;

  Vec3 PointA;
  Vec3 PointB;
  real Radius;
};

}//namespace Zero
