///////////////////////////////////////////////////////////////////////////////
///
///  \file Intersection.hpp
///  Lists all of the functions that test for object intersection.
///
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Intersection
{

enum IntersectionType
{
  //----------------------------------------------------------- Negative Results
  None = -3,
  Outside,  //Used in ClosestPointOn____ToPoint (if original point was outside)

  //----------------------------------------------------------- Positive Results
  Point = 1,
  Edge,
  Triangle,
  Face,
  Segment,
  Ray,
  Line,
  Polygon,
  Inside,   //Used in ClosestPointOn____ToPoint (if original point was inside)
  Other,

  //----------------------------------------------------------- Specific Results
  //Points
  //PointPoint = (uint(Point) << 16) | uint(Point),        //Point
  PointEdge  = (uint(Point) << 16) | uint(Edge),         //Edge
  PointTri   = (uint(Point) << 16) | uint(Triangle),     //Triangle
  PointFace  = (uint(Point) << 16) | uint(Face),         //Face

  //Edges
  EdgePoint  = (uint(Edge) << 16) | uint(Point),         //Point
  EdgeEdge   = (uint(Edge) << 16) | uint(Edge),          //Edge
  EdgeTri    = (uint(Edge) << 16) | uint(Triangle),      //Triangle
  EdgeFace   = (uint(Edge) << 16) | uint(Face),          //Face

  //Triangles
  TriPoint   = (uint(Triangle) << 16) | uint(Point),     //Point
  TriEdge    = (uint(Triangle) << 16) | uint(Edge),      //Edge
  TriTri     = (uint(Triangle) << 16) | uint(Triangle),  //Triangle
  TriFace    = (uint(Triangle) << 16) | uint(Face),      //Face

  //Faces
  FacePoint  = (uint(Face) << 16) | uint(Point),         //Point
  FaceEdge   = (uint(Face) << 16) | uint(Edge),          //Edge
  FaceTri    = (uint(Face) << 16) | uint(Triangle),      //Triangle
  FaceFace   = (uint(Face) << 16) | uint(Face),          //Face

  //------------------------------------------------------------ Special Results
  Unimplemented = 0
};

typedef IntersectionType Type;

const uint cMaxManifoldPoints = 4;

//---------------------------------------------------------------- Line Manifold
struct LineManifold
{
  ///Points of intersection in world coordinates. Point 0 corresponds to the
  ///point closest
  Vec3 Points[2];

  ///Normal at the first non-interior point of intersection.
  Vec3 Normal;

  ///Interpolant value of the first known intersection. Used for ray casts.
  real T;
};

//----------------------------------------------------------- Intersection Point
struct IntersectionPoint
{
  ///Points of intersection in world coordinates. Point 0 corresponds to the
  ///point on the first shape furthest along the intersection axis and point 1
  ///corresponds to the point on the second shape.
  Vec3 Points[2];

  union
  {
    ///Amount of overlap occurring in the direction of the normal.
    real Depth;

    ///Interpolant value of the first known intersection. Used for ray casts.
    real T;
  };
};

//--------------------------------------------------------------------- Manifold
struct Manifold
{
  ///Pairs of points of intersection describing the entire touching regions of
  ///two objects.
  IntersectionPoint Points[cMaxManifoldPoints];

  ///Direction of intersection in world coordinates. By convention, this always
  ///points in the direction away from the first shape to the second (in the
  ///function name).
  Vec3 Normal;

  ///Number of points in the manifold. This is used to tell the intersection
  ///tests the maximum number of points that are available/should be returned.
  ///This value will be changed by the intersection tests to reflect the actual
  ///number of points that were generated.
  uint PointCount;

  static uint cMaxPoints;

  Manifold(uint pointCount = cMaxPoints);
  IntersectionPoint& PointAt(uint index);
};

//--------------------------------------------------------------------- Interval
struct Interval
{
  Interval(void);
  Interval(real min, real max);

  ///Find the complement of this interval (A) in the given interval (B), or 
  ///otherwise the values in B but not in A. Similar to subtracting the elements
  ///of A from B.
  Interval Complement(const Interval& b) const;

  ///Intersect the two intervals, storing the largest minimum value and smallest
  ///maximum value.
  Interval Intersection(const Interval& interval) const;

  ///Combine the two intervals, storing the smallest minimum value and largest
  ///maximum value.
  Interval Union(const Interval& interval) const;

  ///Check to see if the interval is valid (validity defined as Min <= Max).
  bool IsValid(void) const;

  ///Returns the first valid (non-infinity) value in the interval, since the 
  ///range could include an infinity value in conjunction with a non-infinity 
  ///value.
  real FirstT(void) const;

  real Min;
  real Max;

  ///Normals at the surface corresponding to the min and max intersection 
  ///values along the line/ray/segment.
  Vec3 Normal[2];  

  static const Interval cInfinite;
  static const Interval cInvalid;
};

//---------------------------------------------------------------- Support Shape
class SupportShape;
typedef void (*SupportFunction)(const SupportShape* shape, void* data, Vec3Param direction,
                                Vec3Ptr support);
class SupportShape
{
public:
  SupportShape();
  SupportShape(Vec3Param center, SupportFunction support, void* data);

  void GetCenter(Vec3Ptr center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;
  void GetTranslation(Vec3Ptr translation) const;

  Vec3 GetDeltaPosition() const;
  Quat GetDeltaRotation() const;

  void SetDeltaPosition(Vec3 pos);
  void SetDeltaRotation(Quat rot);

  //hack function that should be removed later!!
  void SetCenter(Vec3Param center);
//private:
  Vec3            mCenter;
  SupportFunction mSupportFunction;
  void*           mData;

  Vec3 mDeltaPosition;
  Quat mDeltaRotation;
};

template<typename ShapeType>
void Support(const SupportShape* shape, void* data, Vec3Param direction, Vec3Ptr support)
{
  reinterpret_cast<const ShapeType*>(data)->Support(direction, support);
}

template<typename ShapeType>
void SupportDelta(const SupportShape* shape, void* data, Vec3Param direction, Vec3Ptr support)
{
  Vec3 pos;
  shape->GetCenter(&pos);

  Quat deltaRot = shape->GetDeltaRotation();
  Vec3 dir = Math::Multiply(deltaRot.Inverted(), direction);
  reinterpret_cast<const ShapeType*>(data)->Support(dir, support);
  *support = Math::Multiply(deltaRot, *support - pos) + pos + shape->GetDeltaPosition();
}

template<typename ShapeType>
Intersection::SupportShape MakeSupport(const ShapeType* object, bool supportDelta = false)
{
  Vec3 center;
  object->GetCenter(center);
  if (supportDelta)
    return Intersection::SupportShape(center, &SupportDelta<ShapeType>, (void*)object);
  else
    return Intersection::SupportShape(center, &Support<ShapeType>, (void*)object);
}


/*

  x = Done
  ~ = Never

            | Poi | Ray | Seg | AAB | Cap | Cyl | Ell | Fru*| OBB | Pla | Sph | Tri |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
      Point |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
        Ray |  x  |  ~  |  ~  |  x  |  x  |  x  |  x  |     |  x  |  x  |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
    Segment |  x  |  ~  |  ~  |  x  |  x  |  x  |     |     |  x  |  x  |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
       AABB |  x  |  x  |  x  |  x  |     |     |     |  x  |  x  |  x  |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
    Capsule |  x  |  x  |  x  |     |  x  |     |     |     |     |     |  x  |     |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   Cylinder |  x  |  x  |  x  |     |     |     |     |     |     |     |     |     |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
  Ellipsoid |  x  |  x  |     |     |     |     |     |     |     |     |     |     |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
    Frustum*|  x  |     |     |  x  |     |     |     |     |  x  |     |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
        OBB |  x  |  x  |  x  |  x  |     |     |     |  x  |  x  |  x  |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
      Plane |  x  |  x  |  x  |  x  |     |     |     |     |  x  |  x  |  x  |     |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
     Sphere |  x  |  x  |  x  |  x  |  x  |     |     |  x  |  x  |  x  |  x  |  x  |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
   Triangle |  x  |  x  |  x  |  x  |     |     |     |  x  |  x  |     |  x  |     |
------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+


            | Pla |
------------+-----+
      Plane | Pla |
------------+-----+

*Note: Frustums are unique in that they are mainly used for determining if an
       object is contained within them or not. The return types for all frustum
       tests say if the object is "Inside", "Outside", or intersecting ("Other")
       the frustum. If an actual intersection occurs ("Other") and the Manifold
       parameter is given, then the index of the intersecting plane (or the
       last plane found if multiple) is returned in the Manifold's Depth/Time
       data member.

            =================
              Closest Point
            =================
            |  In   |  On   |
------------+-------+-------+
      Point |       |       |
------------+-------+-------+
        Ray |       |   x   |
------------+-------+-------+
    Segment |       |   x   |
------------+-------+-------+
       AABB |       |   x   |
------------+-------+-------+
    Capsule |       |   x   |
------------+-------+-------+
   Cylinder |       |       |
------------+-------+-------+
  Ellipsoid |       |       |
------------+-------+-------+
    Frustum*|       |       |
------------+-------+-------+
        OBB |   x   |   x   |
------------+-------+-------+
      Plane |       |   x   |
------------+-------+-------+
     Sphere |       |   x   |
------------+-------+-------+
   Triangle |       |   x   |
------------+-------+-------+

*/


//---------------------------------------------------- Point Containment Queries

///Test to see if the given point lies on or inside the given point.
Type PointPoint(Vec3Param pointA, Vec3Param pointB);

///Test to see if the given point lies on or inside the given ray.
Type PointRay(Vec3Param point, Vec3Param rayStart, Vec3Param rayDirection);

///Test to see if the given point lies on or inside the given segment.
Type PointSegment(Vec3Param point, Vec3Param segmentStart, 
                  Vec3Param segmentEnd);

///Test to see if the given point lies on or inside the given axis-aligned
///bounding box.
Type PointAabb(Vec3Param point, Vec3Param aabbMin, Vec3Param aabbMax);

///Test to see if the given point lies on or inside the given capsule.
Type PointCapsule(Vec3Param point, Vec3Param capsulePointA, 
                  Vec3Param capsulePointB, real capsuleRadius);

///Test to see if the given point lies on or inside the given convex shape.
Type PointConvexShape(Vec3Param point, const SupportShape& support);

///Test to see if the given point lies on or inside the given cylinder.
Type PointCylinder(Vec3Param point, Vec3Param cylinderPointA, 
                   Vec3Param cylinderPointB, real cylinderRadius);

///Test to see if the given point lies on or inside the given ellipsoid.
Type PointEllipsoid(Vec3Param point, Vec3Param ellipsoidCenter, 
                    Vec3Param ellipsoidRadii, Mat3Param ellipsoidBasis);

///Test to see if the given point lies inside the given frustum. The 6 planes of
///the frustum are assumed to be pointing inwards.
Type PointFrustum(Vec3Param point, const Vec4 frustumPlanes[6]);

///Test to see if the given point lies on or inside the given oriented-bounding
///box.
Type PointObb(Vec3Param point, Vec3Param obbCenter, Vec3Param obbHalfExtents, 
              Mat3Param obbBasis);

///Test to see if the given point lies on or inside the given plane.
Type PointPlane(Vec3Param point, Vec3Param planeNormal, real planeDistance);

///Test to see if the given point lies on or inside the given sphere.
Type PointSphere(Vec3Param point, Vec3Param sphereCenter, real sphereRadius);

///Test to see if the given point lies on or inside the given tetrahedron.
Type PointTetrahedron(Vec3Param point, Vec3Param tetrahedronPointA,
                      Vec3Param tetrahedronPointB, Vec3Param tetrahedronPointC,
                      Vec3Param tetrahedronPointD);

///Test to see if the given point lies on or inside the given counterclockwise 
///triangle. Treats the point as if it was lying on the plane of the triangle,
///so this can be more accurately described as "point vs triangular prism".
///The epsilon is used to effectively fatten/shrink the triangle,
///primarily for numerical robustness with raycasting.
Type PointTriangle(Vec3Param point, Vec3Param trianglePointA,
                   Vec3Param trianglePointB, Vec3Param trianglePointC, real epsilon = real(0));

//-------------------------------------------------------- Closest Point Queries

///Find the closest point on a ray to the given point. Will return "Inside" if
///the closest point found is in the interval of t = [0, inf], otherwise returns
///"Outside".
Type ClosestPointOnRayToPoint(Vec3Param rayStart, Vec3Param rayDirection,
                              Vec3Ptr point);

///Find the closest point on a line segment to the given point.
Type ClosestPointOnSegmentToPoint(Vec3Param segmentPointA,
                                  Vec3Param segmentPointB,
                                  Vec3Ptr point);

///Find the closest point on an axis aligned bounding box to the given point.
///The point returned will always be on the surface of the axis aligned bounding
///box.
Type ClosestPointOnAabbToPoint(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                               Vec3Ptr point);

///Find the closest point on a capsule to the given point. The point returned
///will always be on the surface of the capsule.
Type ClosestPointOnCapsuleToPoint(Vec3Param capsulePointA,
                                  Vec3Param capsulePointB, real capsuleRadius,
                                  Vec3Ptr point);

///Find the closest point on an oriented bounding box to the given point. The
///point returned will always be on the surface of the oriented bounding box.
Type ClosestPointOnObbToPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                              Mat3Param obbBasis, Vec3Ptr point);

///Find the closest point in or on an oriented bounding box to the given point.
///The point returned can be inside or on the surface of the oriented bounding
///box.
Type ClosestPointInObbToPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                              Mat3Param obbBasis, Vec3Ptr point);

///Find the closest point in or on an oriented bounding box to the given point.
///The point returned can be inside or on the surface of the oriented bounding
///box.
Type ClosestPointInObbToPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                              const Vec3 obbAxes[3], Vec3Ptr point);

///Find the closest point on a plane to the given point. Assumes that the plane
///normal is normalized.
Type ClosestPointOnPlaneToPoint(Vec3Param planeNormal, real planeDistance,
                                Vec3Ptr point);

///Find the closest point on a sphere to the given point. The point returned
///will always be on the surface of the sphere.
Type ClosestPointOnSphereToPoint(Vec3Param sphereCenter, real sphereRadius,
                                 Vec3Ptr point);

///Find the closest point on a triangle to the given point.
Type ClosestPointOnTriangleToPoint(Vec3Param trianglePointA,
                                   Vec3Param trianglePointB,
                                   Vec3Param trianglePointC,
                                   Vec3Ptr point);

///Find the closest points between two lines. Assumes that the line directions
///provided are unit length.
Type ClosestPointsOfTwoLines(Vec3Param lineStartA, Vec3Param lineDirectionA,
                             Vec3Param lineStartB, Vec3Param lineDirectionB,
                             Vec3Ptr closestPointA, Vec3Ptr closestPointB,
                             Vec2Ptr interpolationValues = nullptr);

///Find the closest points between two line segments.
Type ClosestPointsOfTwoSegments(Vec3Param segmentOnePointA,
                                Vec3Param segmentOnePointB,
                                Vec3Param segmentTwoPointA,
                                Vec3Param segmentTwoPointB,
                                Vec3Ptr closestPointOne,
                                Vec3Ptr closestPointTwo);


//-------------------------------------------------------- Line Tests (Interval)

///Intersect a line with an axis-aligned bounding box.
Type LineAabb(Vec3Param linePoint, Vec3Param lineDirection, 
              Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
              Interval* interval);

///Intersect a line with a capsule defined by its center, local axes, radius,
///and half of the distance between the centers of the spherical endcaps.
Type LineCapsule(Vec3Param linePoint, Vec3Param lineDirection,
                 Vec3Param capsuleCenter, Mat3Param capsuleBasis, 
                 real capsuleRadius, real capsuleSegmentHalfLength, 
                 Interval* interval);

///Intersect a line with a capsule defined by the centers of the spherical 
///endcaps and the radius.
Type LineCapsule(Vec3Param linePoint, Vec3Param lineDirection, 
                 Vec3Param capsulePointA, Vec3Param capsulePointB, 
                 real capsuleRadius, Interval* interval);

///Intersect a line with a cylinder defined by its center, local axes, radius,
///and half height.
Type LineCylinder(Vec3Param linePoint, Vec3Param lineDirection, 
                  Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                  real cylinderRadius, real cylinderHalfHeight, 
                  Interval* interval);

///Intersect a line with a cylinder defined by the points at the planar endcaps
///and the radius.
Type LineCylinder(Vec3Param linePoint, Vec3Param lineDirection, 
                  Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                  real cylinderRadius, Interval* interval);

///Intersect a line with an elliptical cylinder defined by its center, local 
///axes, major radius (x-axis), minor radius (z-axis), and half height (y-axis).
Type LineCylinder(Vec3Param linePoint, Vec3Param lineDirection, 
                  Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                  real cylinderMajorRadius, real cylinderMinorRadius, 
                  real cylinderHalfHeight, Interval* interval);

///Intersect a line with an ellipsoid.
Type LineEllipsoid(Vec3Param linePoint, Vec3Param lineDirection,
                   Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii,
                   Mat3Param ellipsoidBasis, Interval* interval);

///Intersect a line with a plane.
Type LinePlane(Vec3Param linePoint, Vec3Param lineDirection, 
               Vec3Param planeNormal, real planeDistance, Interval* interval);

///Intersect a line with an oriented bounding box.
Type LineObb(Vec3Param linePoint, Vec3Param lineDirection, Vec3Param obbCenter,
             Vec3Param obbHalfExtents, Mat3Param obbBasis, Interval* interval);

///Intersect a line with a sphere.
Type LineSphere(Vec3Param linePoint, Vec3Param lineDirection, 
                Vec3Param sphereCenter, real sphereRadius, Interval* interval);

///Intersect a line with a torus.
Type LineTorus(Vec3Param linePoint, Vec3Param lineDirection, 
               Vec3Param torusCenter, Mat3Param torusBasis, 
               real torusRingRadius, real torusTubeRadius, Interval* interval);

///Intersect a line with a triangle.
Type LineTriangle(Vec3Param linePoint, Vec3Param lineDirection, 
                  Vec3Param trianglePointA, Vec3Param trianglePointB, 
                  Vec3Param trianglePointC, Interval* interval);


//--------------------------------------------------------- Ray Tests (Interval)

///Intersect a ray with an axis aligned bounding box.
Type RayAabb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param aabbMinPoint,
             Vec3Param aabbMaxPoint, Interval* interval);

///Intersect a ray with a capsule defined by its center, local axes, radius, and
///half of the distance between the centers of the spherical endcaps.
Type RayCapsule(Vec3Param rayStart, Vec3Param rayDirection,
                Vec3Param capsuleCenter, Mat3Param capsuleBasis, 
                real capsuleRadius, real capsuleSegmentHalfLength, 
                Interval* interval);

///Intersect a ray with a capsule defined by the centers of the spherical 
///endcaps and the radius.
Type RayCapsule(Vec3Param rayStart, Vec3Param lineDirection, 
                Vec3Param capsulePointA, Vec3Param capsulePointB, 
                real capsuleRadius, Interval* interval);

///Intersect a ray with a cylinder defined by its center, local axes, radius,
///and half height.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                 real cylinderRadius, real cylinderHalfHeight, 
                 Interval* interval);

///Intersect a ray with a cylinder defined by the points at the planar endcaps
///and the radius.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                 real cylinderRadius, Interval* interval);

///Intersect a ray with an elliptical cylinder defined by its center, local
///axes, major radius (x-axis), minor radius (z-axis), and half height (y-axis).
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                 real cylinderMajorRadius, real cylinderMinorRadius, 
                 real cylinderHalfHeight, Interval* interval);

///Intersect a ray with an ellipsoid, the inverse scaled basis is the
///combination of the ellipsoid's basis with its radii and then inverted.
Type RayEllipsoid(Vec3Param rayStart, Vec3Param rayDirection, 
                  Vec3Param ellipsoidCenter, Mat3Param ellipsoidInvScaledBasis, 
                  Interval* interval);

///Intersect a ray with an ellipsoid.
Type RayEllipsoid(Vec3Param rayStart, Vec3Param rayDirection, 
                  Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii,
                  Mat3Param ellipsoidBasis, Interval* interval);

///Intersect a ray with a plane.
Type RayPlane(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param planeNormal,
              real planeDistance, Interval* interval);

///Intersect a ray with an oriented bounding box.
Type RayObb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param obbCenter, 
            Vec3Param obbHalfExtents, Mat3Param obbBasis, Interval* interval);

///Intersect a ray with a sphere.
Type RaySphere(Vec3Param rayStart, Vec3Param rayDirection, 
               Vec3Param sphereCenter, real sphereRadius, Interval* interval);

//Intersect a ray with a tetrahedron.
Type RayTetrahedron(Vec3Param rayStart, Vec3Param rayDirection, 
                    Vec3Param tetrahedronPointA, Vec3Param tetrahedronPointB, 
                    Vec3Param tetrahedronPointC, Vec3Param tetrahedronPointD, 
                    Interval* interval);

///Intersect a ray with a torus.
Type RayTorus(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param torusCenter,
              Mat3Param torusBasis, real torusRingRadius, real torusTubeRadius,
              Interval* interval);

///Intersect a ray with a triangle. The epsilon is used to fatten/shrink the triangle.
Type RayTriangle(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param trianglePointA, Vec3Param trianglePointB, 
                 Vec3Param trianglePointC, Interval* interval, real epsilon = real(0));


//----------------------------------------------------- Segment Tests (Interval)

///Intersect a segment with an axis aligned bounding box.
Type SegmentAabb(Vec3Param segmentStart, Vec3Param segmentEnd, 
                 Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
                 Interval* interval);

///Intersect a segment with a capsule defined by its center, local axes, radius,
///and half of the distance between the centers of the spherical endcaps.
Type SegmentCapsule(Vec3Param segmentStart, Vec3Param segmentEnd,
                    Vec3Param capsuleCenter, Mat3Param capsuleBasis, 
                    real capsuleRadius, real capsuleSegmentHalfLength, 
                    Interval* interval);

///Intersect a segment with a capsule defined by the centers of the spherical 
///endcaps and the radius.
Type SegmentCapsule(Vec3Param segmentStart, Vec3Param segmentEnd,
                    Vec3Param capsulePointA, Vec3Param capsulePointB, 
                    real capsuleRadius, Interval* interval);

///Intersect a segment with a cylinder defined by its center, local axes, 
///radius, and half height.
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                     real cylinderRadius, real cylinderHalfHeight, 
                     Interval* interval);

///Intersect a segment with a cylinder defined by the points at the planar 
///endcap and the radius.
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                     real cylinderRadius, Interval* interval);

///Intersect a segment with an elliptical cylinder defined by its center, local
///axes, major radius (x-axis), minor radius (z-axis), and half height (y-axis).
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                     real cylinderMajorRadius, real cylinderMinorRadius, 
                     real cylinderHalfHeight, Interval* interval);

///Intersect a segment with an ellipsoid.
Type SegmentEllipsoid(Vec3Param segmentStart, Vec3Param segmentEnd,
                      Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii,
                      Mat3Param ellipsoidBasis, Interval* interval);

///Intersect a segment with a plane.
Type SegmentPlane(Vec3Param segmentStart, Vec3Param segmentEnd, 
                  Vec3Param planeNormal, real planeDistance, 
                  Interval* interval);

///Intersect a segment with an oriented bounding box.
Type SegmentObb(Vec3Param segmentStart, Vec3Param segmentEnd, 
                Vec3Param obbCenter, Vec3Param obbHalfExtents, 
                Mat3Param obbBasis, Interval* interval);

///Intersect a segment with a sphere.
Type SegmentSphere(Vec3Param segmentStart, Vec3Param segmentEnd,
                   Vec3Param sphereCenter, real sphereRadius, 
                   Interval* interval);

///Intersect a segment with a torus.
Type SegmentTorus(Vec3Param segmentStart, Vec3Param segmentEnd,
                  Vec3Param torusCenter, Mat3Param torusBasis, 
                  real torusRingRadius, real torusTubeRadius, 
                  Interval* interval);

///Intersect a segment with a triangle.
Type SegmentTriangle(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param trianglePointA, Vec3Param trianglePointB, 
                     Vec3Param trianglePointC, Interval* interval);


//----------------------------------------------------------- Ray Tests (Points)

///Intersect a ray with an axis aligned bounding box.
Type RayAabb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param aabbMinPoint,
             Vec3Param aabbMaxPoint,
             IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with a capsule. If the result is "Segment", the second point
///isn't guaranteed to be on the surface of the capsule (for now).
Type RayCapsule(Vec3Param rayStart, Vec3Param rayDirection,
                Vec3Param capsulePointA, Vec3Param capsulePointB,
                real capsuleRadius,
                IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with a cylinder defined by its center, local axes, radius,
///and half height.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                 real cylinderRadius, real cylinderHalfHeight, 
                 IntersectionPoint* intersectionPoint = nullptr);

//Intersect a ray with a cylinder defined by the points at the planar endcaps
//and the radius.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection,
                 Vec3Param cylinderPointA, Vec3Param cylinderPointB,
                 real cylinderRadius,
                 IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with an ellipsoid.
Type RayEllipsoid(Vec3Param rayStart, Vec3Param rayDirection, 
                  Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii, 
                  Mat3Param ellipsoidBasis, 
                  IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with an arbitrary collection of triangles. Base offset is the
///number of bytes from the beginning to the first value, vertex stride is the 
///byte distance between vertices, and the size of the index refers to the
///number of bytes used to represent the indices (generally 2 or 4 bytes).
Type RayMeshBuffer(Vec3Param rayStart, Vec3Param rayDirection, byte* vertexData, 
                   uint vertexStride, uint baseOffset, byte* indices, 
                   uint sizeOfIndex, uint triCount, bool backfaceCulling = true, 
                   bool anyIntersection = true, uint* hitTriIndex = nullptr, 
                   IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with a plane.
Type RayPlane(Vec3Param rayStart, Vec3Param rayDirection,
              Vec3Param planeNormal, real planeDistance,
              IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with an oriented bounding box.
Type RayObb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param obbCenter,
            Vec3Param obbHalfExtents, Mat3Param obbBasis,
            IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with a sphere.
Type RaySphere(Vec3Param rayStart, Vec3Param rayDirection,
               Vec3Param sphereCenter, real sphereRadius,
               IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with a tetrahedron.
Type RayTetrahedron(Vec3Param rayStart, Vec3Param rayDirection,
                    Vec3Param tetrahedronPointA, Vec3Param tetrahedronPointB,
                    Vec3Param tetrahedronPointC,Vec3Param tetrahedronPointD,
                    IntersectionPoint* intersectionPoint = nullptr);

///Intersect a ray with a triangle. The epsilon is used to fatten/shrink the triangle.
Type RayTriangle(Vec3Param rayStart, Vec3Param rayDirection,
                 Vec3Param trianglePointA, Vec3Param trianglePointB,
                 Vec3Param trianglePointC,
                 IntersectionPoint* intersectionPoint = nullptr, real epsilon = real(0));

///Intersect a ray with a torus.
Type RayTorus(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param torusCenter,
              Mat3Param torusBasis, real torusRingRadius, real torusTubeRadius,
              IntersectionPoint* intersectionpoint = nullptr);


//------------------------------------------------------- Segment Tests (Points)

///Intersect a segment with an axis aligned bounding box.
Type SegmentAabb(Vec3Param segmentStart, Vec3Param segmentEnd,
                 Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                 IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with a capsule.
Type SegmentCapsule(Vec3Param segmentStart, Vec3Param segmentEnd,
                    Vec3Param capsulePointA, Vec3Param capsulePointB,
                    real capsuleRadius,
                    IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with a cylinder.
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderPointA, Vec3Param cylinderPointB,
                     real cylinderRadius,
                     IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with an ellipsoid.
Type SegmentEllipsoid(Vec3Param segmentStart, Vec3Param segmentEnd, 
                      Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii, 
                      Mat3Param ellipsoidBasis, 
                      IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with a plane.
Type SegmentPlane(Vec3Param segmentStart, Vec3Param segmentEnd,
                  Vec3Param planeNormal, real planeDistance,
                  IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with an oriented bounding box.
Type SegmentObb(Vec3Param segmentStart, Vec3Param segmentEnd,
                Vec3Param obbCenter, Vec3Param obbHalfExtents,
                Mat3Param obbBasis,
                IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with a sphere.
Type SegmentSphere(Vec3Param segmentStart, Vec3Param segmentEnd,
                   Vec3Param sphereCenter, real sphereRadius,
                   IntersectionPoint* intersectionPoint = nullptr);

///Intersect a segment with a triangle.
Type SegmentTriangle(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param trianglePointA, Vec3Param trianglePointB,
                     Vec3Param trianglePointC,
                     IntersectionPoint* intersectionPoint = nullptr);


//----------------------------------------------------------------- Object Tests

///Intersect an axis aligned bounding box with an axis aligned bounding box.
Type AabbAabb(Vec3Param aabbOneMinPoint, Vec3Param aabbOneMaxPoint,
              Vec3Param aabbTwoMinPoint, Vec3Param aabbTwoMaxPoint,
              Manifold* manifold = nullptr);

///Intersect an axis aligned bounding box with a capsule.
Type AabbCapsule(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                 Vec3Param capsulePointA, Vec3Param capsulePointB,
                 real capsuleRadius, Manifold* manifold = nullptr);

///Intersect an axis aligned bounding box with a frustum. The 6 planes of the
///frustum are assumed to be pointing inwards.
///This test is an approximation because it only checks the aabb points against the frustum's planes.
///This doesn't cover all axes necessary for a SAT test so it can return false positives.
Type AabbFrustumApproximation(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                              const Vec4 frustumPlanes[6], Manifold* manifold = nullptr);

///Intersect an axis aligned bounding box with an oriented bounding box.
Type AabbObb(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
             Vec3Param obbCenter, Vec3Param obbHalfExtents,
             Mat3Param obbBasis, Manifold* manifold = nullptr);

///Intersect an axis aligned bounding box with a plane.
Type AabbPlane(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
               Vec3Param planeNormal, real planeDistance, 
               Manifold* manifold = nullptr);

///Intersect an axis aligned bounding box with a sphere.
Type AabbSphere(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                Vec3Param sphereCenter, real sphereRadius, 
                Manifold* manifold = nullptr);

///Intersect an axis aligned bounding box with a triangle.
Type AabbTriangle(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                  Vec3Param trianglePointA, Vec3Param trianglePointB,
                  Vec3Param trianglePointC, Manifold* manifold = nullptr);

///Intersect a capsule with a capsule.
Type CapsuleCapsule(Vec3Param capsuleOnePointA, Vec3Param capsuleOnePointB,
                    real capsuleOneRadius, Vec3Param capsuleTwoPointA,
                    Vec3Param capsuleTwoPointB, real capsuleTwoRadius,
                    Manifold* manifold = nullptr);

///Intersect a capsule with a frustum. The 6 planes of the frustum are assumed
///to be pointing inwards.
Type CapsuleFrustum(Vec3Param capsulePointA, Vec3Param capsulePointB,
                    real capsuleRadius, const Vec4 frustumPlanes[6],
                    Manifold* manifold = nullptr);

///Intersect a capsule with an oriented bounding box.
Type CapsuleObb(Vec3Param capsulePointA, Vec3Param capsulePointB,
                real capsuleRadius, Vec3Param obbCenter,
                Vec3Param obbHalfExtents, Vec3Param obbBasis,
                Manifold* manifold = nullptr);

///Intersect a capsule with a plane.
Type CapsulePlane(Vec3Param capsulePointA, Vec3Param capsulePointB,
                  real capsuleRadius, Vec3Param planeNormal, real planeDistance,
                  Manifold* manifold = nullptr);

///Intersect a capsule with a sphere.
Type CapsuleSphere(Vec3Param capsulePointA, Vec3Param capsulePointB,
                   real capsuleRadius, Vec3Param sphereCenter,
                   real sphereRadius, Manifold* manifold = nullptr);

///Intersect a capsule with a triangle.
Type CapsuleTriangle(Vec3Param capsulePointA, Vec3Param capsulePointB,
                     real capsuleRadius, Vec3Param trianglePointA,
                     Vec3Param trianglePointB, Vec3Param trianglePointC,
                     Manifold* manifold = nullptr);

///Intersect a frustum with a sphere. The 6 planes of the frustum are assumed to
///be pointing inwards.
Type FrustumSphereApproximation(const Vec4 frustumPlanes[6], Vec3Param sphereCenter,
                                real sphereRadius, Manifold* manifold = nullptr);

///Intersect a frustum with a triangle. The 6 planes of the frustum are assumed
///to be pointing inwards.
Type FrustumTriangle(const Vec4 frustumPlanes[6], Vec3Param trianglePointA,
                     Vec3Param trianglePointB, Vec3Param trianglePointC,
                     Manifold* manifold = nullptr);

///Intersect a frustum with an oriented bounding box. The 6 planes of the
///frustum are assumed to be pointing inwards.
Type FrustumObbApproximation(const Vec4 frustumPlanes[6], Vec3Param obbCenter,
                             Vec3Param obbHalfExtents, Mat3Param obbBasis,
                             Manifold* manifold = nullptr);

///Intersect a frustum with a plane. The 6 planes of the frustum are assumed to
///be pointing inwards.
Type FrustumPlane(const Vec4 frustumPlanes[6], Vec3Param planeNormal,
                  real planeDistance, Manifold* manifold = nullptr);

///Intersect an oriented bounding box with an oriented bounding box.
Type ObbObb(Vec3Param obbOneCenter, Vec3Param obbOneHalfExtents,
            Mat3Param obbOneBasis, Vec3Param obbTwoCenter,
            Vec3Param obbTwoHalfExtents, Mat3Param obbTwoBasis,
            Manifold* manifold = nullptr);

///Intersect an oriented bounding box with a plane.
Type ObbPlane(Vec3Param obbCenter, Vec3Param obbHalfExtents, Mat3Param obbBasis,
              Vec3Param planeNormal, real planeDistance, 
              Manifold* manifold = nullptr);

///Intersect an oriented bounding box with a sphere.
Type ObbSphere(Vec3Param obbCenter, Vec3Param obbHalfExtents,
               Mat3Param obbBasis, Vec3Param sphereCenter, real sphereRadius,
               Manifold* manifold = nullptr);

///Intersect an oriented bounding box with a triangle.
Type ObbTriangle(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                 Mat3Param obbBasis, Vec3Param trianglePointA,
                 Vec3Param trianglePointB, Vec3Param trianglePointC,
                 Manifold* manifold = nullptr);

///Intersect an oriented bounding box with a triangle. Different parameters.
Type ObbTriangle(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                 Mat3Param obbBasis, const Vec3 trianglePoints[3],
                 Manifold* manifold = nullptr);

///Intersect a plane with a plane. If intersection data is desired, the points
///returned are two points on the line of intersection.
Type PlanePlane(Vec3Param planeOneNormal, real planeOneDistance,
                Vec3Param planeTwoNormal, real planeTwoDistance,
                Manifold* manifold = nullptr);

///Intersect a plane with a plane with plane. If intersection
///data is desired, the point of collision is returned (if there is one).
Type PlanePlanePlane(Vec4Param planeA, Vec4Param planeB, Vec4Param planeC,
                     Manifold* manifold = nullptr);

///Intersect a plane with a sphere.
Type PlaneSphere(Vec3Param planeNormal, real planeDistance,
                 Vec3Param sphereCenter, real sphereRadius,
                 Manifold* manifold = nullptr);

///Intersect a plane with a triangle.
Type PlaneTriangle(Vec3Param planeNormal, real planeDistance,
                   Vec3Param trianglePointA, Vec3Param trianglePointB,
                   Vec3Param trianglePointC, Manifold* manifold = nullptr);

///Intersect a sphere with a sphere.
Type SphereSphere(Vec3Param sphereOneCenter, real sphereOneRadius,
                  Vec3Param sphereTwoCenter, real sphereTwoRadius,
                  Manifold* manifold = nullptr);

///Intersect a sphere with a triangle.
Type SphereTriangle(Vec3Param sphereCenter, real sphereRadius,
                    Vec3Param trianglePointA, Vec3Param trianglePointB,
                    Vec3Param trianglePointC, Manifold* manifold = nullptr);

///Intersect a triangle with a triangle. This may be slow, and the manifold 
///doesn't do anything.
Type TriangleTriangle(Vec3Param triangleOnePointA, Vec3Param triangleOnePointB,
                      Vec3Param triangleOnePointC, Vec3Param triangleTwoPointA,
                      Vec3Param triangleTwoPointB, Vec3Param triangleTwoPointC,
                      Manifold* manifold = nullptr);


//-------------------------------------------------------- Intersection Point 2D
struct IntersectionPoint2D
{
  ///Points of intersection in world coordinates. Point 0 corresponds to the 
  ///point on the first shape furthest along the intersection axis and point 1
  ///corresponds to the point on the second shape.
  Vec2 Points[2];

  union
  {
    ///Amount of overlap occurring in the direction of the normal.
    real Depth;

    ///Interpolant value of the first known intersection. Used for ray casts.
    real T;
  };
};

//------------------------------------------------------------------ Manifold 2D
struct Manifold2D
{
  ///Pairs of points of intersection describing the entire touching regions of
  ///two objects.
  IntersectionPoint2D Points[2];

  ///Direction of intersection in world coordinates. By convention, this always
  ///points in the direction away from the first shape to the second (in the
  ///function name).
  Vec2 Normal;

  ///Number of points in the manifold. This is used to tell the intersection 
  ///tests the maximum number of points that are available/should be returned.
  ///This value will be changed by the intersection tests to reflect the actual
  ///number of points that were generated.
  uint PointCount;

  Manifold2D(uint pointCount = 2);
  IntersectionPoint2D& PointAt(uint index);
};


//--------------------------------------------------------------------- 2D Tests
///Find the closest point on a line segment to the given point.
Type ClosestPointOnSegmentToPoint(Vec2Param segmentPointA,
                                  Vec2Param segmentPointB,
                                  Vec2Ptr point);

///Intersect a rotated box with a rotated box.
Type BoxBox(Vec2Param boxCenterA, Vec2Param boxHalfExtentsA, 
            const Vec2* boxAxesA, Vec2Param boxCenterB, 
            Vec2Param boxHalfExtentsB, const Vec2* boxAxesB,
            Manifold2D* manifold = nullptr);

///Intersect a rotated box with a circle.
Type BoxCircle(Vec2Param boxCenter, Vec2Param boxHalfExtents, 
               const Vec2* boxAxes, Vec2Param circleCenter, real circleRadius, 
               Manifold2D* manifold = nullptr);

///Intersect a circle with a circle.
Type CircleCircle(Vec2Param circleCenterA, real circleRadiusA,
                  Vec2Param circleCenterB, real circleRadiusB,
                  Manifold2D* manifold = nullptr);

///Intersect a circle with an n-sided convex polygon.
Type CircleConvexPolygon(Vec2Param circleCenter, real circleRadius, 
                         const Vec2* convexPolygonPoints, 
                         uint convexPolygonPointCount,
                         Manifold2D* manifold = nullptr);

///Intersect an n-sided convex polygon with an n-sided convex polygon.
Type ConvexPolygonConvexPolygon(const Vec2* convexPolygonPointsA, 
                                uint convexPolygonPointCountA,
                                const Vec2* convexPolygonPointsB, 
                                uint convexPolygonPointCountB,
                                Manifold2D* manifold = nullptr);

///Test if a point lies on a line segment.
Type PointSegment(Vec2Param point, Vec2Param segmentStart, 
                  Vec2Param segmentEnd);

///Intersect a ray with an axis aligned bounding box (2D).
Type RayAabb(Vec2Param rayStart, Vec2Param rayDirection,
             Vec2Param aabbMinPoint, Vec2Param aabbMaxPoint,
             Interval* interval = nullptr);

///Intersect two 2D segments.
Type SegmentSegment(Vec2Param segmentStartA, Vec2Param segmentEndA,
                    Vec2Param segmentStartB, Vec2Param segmentEndB, 
                    IntersectionPoint2D* intersectionPoint = nullptr);

}// namespace Intersection
