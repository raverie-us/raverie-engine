///////////////////////////////////////////////////////////////////////////////
///
/// \file Frustum.hpp
/// Declaration of the Frustum class.
///
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct Aabb;
struct Sphere;

//Planes are defined as:
/*     4---------------5  A - Front Plane
      /|              /|  B - Back Plane
     / |      B     /  |  C - Right Plane
    /  |          /    |  D - Left Plane
   /   |        /      |  E - Bottom Plane
  / D  7------/--------6  F - Top Plane
 /    /     /    C   /
0---------1       /
|  /      |    /
|/   A    | /
3---------2          */

struct Frustum
{
  Frustum();
  Frustum(Plane* planes);
  ~Frustum();

  ///Sets all the planes.
  void Set(Plane* planes);

  // Accessor to set planes for script
  Plane& Get(uint index);
  void Set(uint index, const Plane& plane);

  ///Generates a Frustum from the give points.  Point order is described above.
  void Generate(Vec3 points[8]);
  void Generate(Vec3Param frontCenter, Vec3Param direction, Vec3Param up, Vec3Param dimensions);
  /// Implicitly builds frustum down -Z
  void Generate(Vec3Param position, Mat3Param basis, float near, float far, float aspect, float fov);

  ///Calculates the 8 points of the Aabb.
  void GetPoints(Vec3 points[8]) const;
  ///Returns an Aabb encasing the frustum.
  Aabb GetAabb() const;

  ///Tests if the given point is inside the frustum.
  bool Overlaps(Vec3Param point);
  ///Tests if the given Aabb is inside the frustum.
  bool Overlaps(const Aabb& aabb);
  ///Tests if the given Sphere is inside the frustum.
  bool Overlaps(const Sphere& sphere);

  const Vec4* GetIntersectionData() const;

  /// Sets the passed in vector to the center of the tetrahedra.
  void GetCenter(Vec3Ref center) const;
  /// Get the point on the shape furthest in the given direction,
  /// stored in the parameter.
  void Support(Vec3Param direction, Vec3Ptr support) const;

  /// Get Box Points at Distance from Near Plane
  void PointsAtDepth(Vec3 boxPoints[4], float depth) const;

  Frustum Transform(Mat4Param transformation) const;
  Frustum UniformTransform(Mat4Param transformation) const;
  Frustum TransformInverse(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Frustum TransformedShapeType;

  enum {PlaneDim = 6};
  Plane Planes[PlaneDim];
};

}//namespace Zero
