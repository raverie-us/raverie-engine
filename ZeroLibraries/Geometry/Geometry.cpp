
///  \file Geometry.cpp
///  Source to all functions that deal with geometry but not intersection.
///
///  Authors: Benjamin Strukus, Joshua Claeys
///  Copyright 2010-2012, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Solids.hpp"
#include "Math/Math.hpp"

namespace Geometry
{

///Determines whether all intersection and geometry functions will utilize extra 
///checks to prevent floating point errors.
const bool cGeometrySafeChecks = true;

namespace
{
const real cAabbFaceThreshold = real(0.00001);
const real cCylinderEndcapThreshold = real(0.0001);
}// namespace

//----------------------------------------------------------------- 2D Functions
//Calculate the centroid of the 2D polygon. Assumes the 2D points are ordered
//in such a way that they describe the polygon's perimeter.
void CalculatePolygonCentriod(const Vec2* polyPoints, uint polyPointCount,
                              Vec2Ptr centroid)
{
  ErrorIf(polyPoints == nullptr, "Geometry - Null pointer passed, this function "\
                              "needs a valid pointer.");
  ErrorIf(centroid == nullptr, "Geometry - Null pointer passed, this function "\
                            "needs a valid pointer.");

  //Check out http://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
  if(polyPointCount == 1)
  {
    *centroid = *polyPoints;
    return;
  }
  if(polyPointCount == 2)
  {
    *centroid = polyPoints[0];
    *centroid += polyPoints[1];
    *centroid *= real(0.5);
    return;
  }

  real area = real(0.0);
  centroid->ZeroOut();
  real xyXY;
  uint loopCount = polyPointCount - 1;

  for(uint i = 0; i < loopCount; ++i)
  {
    xyXY  = polyPoints[i].x * polyPoints[i + 1].y;
    xyXY -= polyPoints[i + 1].x * polyPoints[i].y;

    area += xyXY;

    centroid->x += xyXY * (polyPoints[i].x + polyPoints[i + 1].x);
    centroid->y += xyXY * (polyPoints[i].y + polyPoints[i + 1].y);
  }

  //Have to complete the loop around the polygon
  xyXY  = polyPoints[loopCount].x * polyPoints[0].y;
  xyXY -= polyPoints[0].x * polyPoints[loopCount].y;

  area += xyXY;

  centroid->x += xyXY * (polyPoints[loopCount].x + polyPoints[0].x);
  centroid->y += xyXY * (polyPoints[loopCount].y + polyPoints[0].y);

  if(cGeometrySafeChecks)
  {
    if(Math::IsZero(area))
    {
      *centroid *= Math::PositiveMax();
      return;
    }
  }

  *centroid *= real(1.0) / (real(3.0) * area);
}

//Generate an axis-aligned bounding box for the given set of 2D points.
void GenerateAabb(const Vec2* points, uint pointCount, Vec2Ptr min, Vec2Ptr max)
{
  ErrorIf(points == nullptr, "Geometry - Null pointer passed, this "\
                          "function needs a valid pointer.");
  ErrorIf(min == nullptr, "Geometry - Null pointer passed, this "\
                       "function needs a valid pointer.");
  ErrorIf(max == nullptr, "Geometry - Null pointer passed, this "\
                       "function needs a valid pointer.");

  *min = Vec2(Math::PositiveMax(), Math::PositiveMax());
  *max = -(*min);
  for(uint i = 0; i < pointCount; ++i)
  {
    *min = Math::Min(points[i], *min);
    *max = Math::Max(points[i], *max);
  }
}

//Given an ordered set of 2D points that describe the perimeter of a polygon,
//return whether the points are clockwise (negative) or
//counter-clockwise (positive).
real DetermineWindingOrder(const Vec2* polyPoints, uint polyPointCount)
{
  real result = real(0.0);
  for(uint i = polyPointCount - 1, j = 0; j < polyPointCount; i = j, ++j)
  {
    result += (polyPoints[i].x - polyPoints[j].x) *
              (polyPoints[i].y + polyPoints[j].y);
  }
  return result;
}

//Returns 2 times the signed triangle area. The result is positive is abc is
//counter-clockwise, negative if abc is clockwise, zero if abc is degenerate.
real Signed2DTriArea(Vec2Param a, Vec2Param b, Vec2Param c)
{
  return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

//----------------------------------------------------------------- 3D Functions
//Generate an axis-aligned bounding box for the given set of 3D points.
void GenerateAabb(const Vec3* points, uint pointCount, Vec3Ptr min, Vec3Ptr max)
{
  ErrorIf(points == nullptr, "Geometry - Null pointer passed, this "\
                          "function needs a valid pointer.");
  ErrorIf(min == nullptr, "Geometry - Null pointer passed, this "\
                       "function needs a valid pointer.");
  ErrorIf(max == nullptr, "Geometry - Null pointer passed, this "\
                       "function needs a valid pointer.");

  *min = Vec3(Math::PositiveMax(), Math::PositiveMax(), Math::PositiveMax());
  *max = -(*min);
  for(uint i = 0; i < pointCount; ++i)
  {
    *min = Math::Min(points[i], *min);
    *max = Math::Max(points[i], *max);
  }
}

//Get a unit length vector which is orthogonal to the plane that points A, B,
//and C lie on. This does the cross product on the vector from A to B and from
//B to C.
Vec3 GenerateNormal(Vec3Param pointA, Vec3Param pointB, Vec3Param pointC)
{
  Vec3 aToB = pointB - pointA;
  Vec3 bToC = pointC - pointB;
  bToC = Cross(aToB, bToC);
  if(cGeometrySafeChecks)
  {
    real length = AttemptNormalize(bToC);
    if(Math::IsZero(length))
    {
      bToC = Vec3::cZero;
    }
  }
  else
  {
    Normalize(bToC);
  }
  return bToC;
}

//Returns whether or not the given triangle is valid for physics.
bool IsDegenerate(Vec3Param pointA, Vec3Param pointB, Vec3Param pointC)
{
  const real cAreaEpsilon = real(0.0001f);
  const real cAreaEpsilonSquared = cAreaEpsilon * cAreaEpsilon;

  Vec3 cross = (pointA - pointB).Cross(pointC - pointB);
  return cross.LengthSq() < cAreaEpsilonSquared;
}

//Get the signed distance of a point to a plane.
real SignedDistanceToPlane(Vec3Param point, Vec3Param planeNormal,
                           real planeDistance)
{
  return Dot(point, planeNormal) - planeDistance;
}

//Calculate the barycentric coordinates of the point with respect to the
//triangle. Returns XYZ correspond to triangle's ABC points, respectively.
void BarycentricTriangle(Vec3Param point, Vec3Param trianglePointA,
                         Vec3Param trianglePointB, Vec3Param trianglePointC,
                         Vec3Ptr barycentricCoordinates)
{
  ErrorIf(barycentricCoordinates == nullptr, "Geometry - Null pointer passed, "\
                                             "this function needs a valid "    \
                                             "pointer.");

  //Barycentric coordinates can be computed as a ratio of triangle areas.
  //This can be computed from a ratio of the normal of the sub
  //triangles. See the orange book for more details.

  //Store the normal and unit normal of the triangle.
  Vec3 normal0 = Math::Cross(trianglePointB - trianglePointA, trianglePointC - trianglePointA);
  Vec3 unitNormal0 = normal0;
  normal0.AttemptNormalize();

  //grab each sub-triangle's normal
  Vec3 normal1 = Math::Cross(trianglePointB - point,trianglePointC - point);
  Vec3 normal2 = Math::Cross(trianglePointC - point,trianglePointA - point);
  Vec3 normal3 = Math::Cross(trianglePointA - point,trianglePointB - point);

  //store the denominator and do error checking
  real denom = Math::Dot(normal0,unitNormal0);
  if(denom == real(0.0))
  {
    //I have no clue what to set this to, but for now just set
    //every coordinate to 1/3 to avoid garbage numbers.
    barycentricCoordinates->x = barycentricCoordinates->y = barycentricCoordinates->z = real(.33333);
    Error("Denominator in barycentric coordinates is zero.");
    return;
  }

  barycentricCoordinates->x = Math::Dot(normal1,normal0) / denom;
  barycentricCoordinates->y = Math::Dot(normal2,normal0) / denom;
  barycentricCoordinates->z = Math::Dot(normal3,normal0) / denom;
}

//Calculate the barycentric coordinates of the point with respect to the
//tetrahedron. Returns XYZW correspond to tetrahedron's ABCD points,
//respectively.
void BarycentricTetrahedron(Vec3Param point, Vec3Param tetrahedronPointA,
                            Vec3Param tetrahedronPointB,
                            Vec3Param tetrahedronPointC,
                            Vec3Param tetrahedronPointD,
                            Vec4Ptr barycentricCoordinates)
{
  ErrorIf(barycentricCoordinates == nullptr, "Geometry - Null pointer passed, "\
                                             "this function needs a valid "    \
                                             "pointer.");

  Vec3 points[3] = { tetrahedronPointA - tetrahedronPointD,
                     tetrahedronPointB - tetrahedronPointD,
                     tetrahedronPointC - tetrahedronPointD };
  Mat3 matrix;
  matrix.SetBasis(0, points[0]);
  matrix.SetBasis(1, points[1]);
  matrix.SetBasis(2, points[2]);
  Invert(&matrix);

  Vec3 localPoint = point - tetrahedronPointD;
  Math::Transform(matrix, &localPoint);
  barycentricCoordinates->x = localPoint.x;
  barycentricCoordinates->y = localPoint.y;
  barycentricCoordinates->z = localPoint.z;
  barycentricCoordinates->w = real(1.0) - localPoint.x
                                        - localPoint.y
                                        - localPoint.z;
}

//Clip the set of coplanar polygon points against the plane.
uint ClipPolygonWithPlane(const Vec3* polygonPoints, uint polygonPointCount,
                          Vec3Param planeNormal, real planeDistance,
                          Vec3Ptr clippedPoints)
{
  ErrorIf(polygonPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(clippedPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");

  //Store which points are behind the plane
  bool behindPlane[cMaxSupportPoints];

  //If true then there is at least one point behind the plane
  bool pointsBehind = false;

  //If true then there is at least one point in front of the plane
  bool pointsInFront = false;

  //Go through all of the polygon's points and check to see which side of the
  //plane they are on
  for(uint i = 0; i < polygonPointCount; ++i)
  {
    real side = Dot(polygonPoints[i], planeNormal) - planeDistance;
    behindPlane[i] = (side < real(0.0)) ? true : false;
    pointsBehind  |= behindPlane[i];
    pointsInFront |= !behindPlane[i];
  }

  //There aren't any points behind the plane, polygon is not touching the face
  if(pointsBehind == false)
  {
    return 0;
  }

  //There aren't any points in front of the plane
  if(pointsInFront == false)
  {
    //All of the points are behind the plane, add them all
    for(uint i = 0; i < polygonPointCount; ++i)
    {
      clippedPoints[i] = polygonPoints[i];
    }
    return polygonPointCount;
  }

  //Start off with the last point
  uint curr = polygonPointCount - 1;

  //Get the total number of points
  uint pointCount = (polygonPointCount > 2) ? polygonPointCount : 1;

  uint clippedPointCount = 0;

  //Attempt to clip each edge of the polygon with the plane
  for(uint next = 0; next < pointCount; curr = next, ++next)
  {
    //The current point is behind the plane
    if(behindPlane[curr])
    {
      ErrorIf(clippedPointCount >= cMaxSupportPoints, "Geometry - Too many "\
                                                      "points passed to "   \
                                                      "clipping function");
      //Add it to the array of clipped points
      clippedPoints[clippedPointCount] = polygonPoints[curr];
      ++clippedPointCount;
    }

    //Only one of the points is behind the plane
    if(behindPlane[curr] ^ behindPlane[next])
    {
      ErrorIf(clippedPointCount >= cMaxSupportPoints, "Geometry - Too many "\
                                                      "points passed to "   \
                                                      "clipping function");

      //Compute the t value for the directed line ab intersecting the plane
      Vec3 segment = polygonPoints[next] - polygonPoints[curr];

      real t  = (planeDistance - Dot(planeNormal, polygonPoints[curr]));
           t /= Dot(planeNormal, segment);
      clippedPoints[clippedPointCount] = polygonPoints[curr];
      clippedPoints[clippedPointCount] = Math::MultiplyAdd(clippedPoints[clippedPointCount], segment, t);
      ++clippedPointCount;
    }
  }
  if((pointCount == 1) && (clippedPointCount == 1))
  {
    clippedPoints[1] = behindPlane[0] ? polygonPoints[0] : polygonPoints[1];
    clippedPointCount = 2;
  }
  return clippedPointCount;
}

//Clip the set of coplanar polygon points against the given clipping planes.
//The planes are stored in a Vector4 with the plane normal in the (x,y,z) and
//the plane's distance in the w.
uint ClipPolygonWithPlanes(const Vec4* clipperPlanes, uint clipperPlaneCount,
                           const Vec3* polygonPoints, uint polygonPointCount,
                           Vec3Ptr clippedPoints)
{
  ErrorIf(clipperPlanes == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(polygonPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(clippedPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(polygonPointCount < 2, "Geometry - Not enough points, can't clip.");


  uint clippedPointCount = 0;
  if(clipperPlaneCount == 0)
  {
    return 0;
  }

  //The points that define the polygon that is to be clipped
  Vec3 clippingResults[cMaxSupportPoints];

  //Number of points in the polygon
  uint clippingResultsCount = polygonPointCount;

  //Copy over the polygon's points into the temporary buffer
  memcpy(clippingResults, polygonPoints, sizeof(clippingResults[0]) *
                                              clippingResultsCount);

  //Clip the polygon edges against all the infinite planes
  for(uint i = 0; i < clipperPlaneCount; ++i)
  {
    const Vec4& currentPlane = clipperPlanes[i];
    Vec3 planeNormal = Vec3(currentPlane.x, currentPlane.y, currentPlane.z);
    real planeDistance = currentPlane.w;
    clippedPointCount = ClipPolygonWithPlane(clippingResults,
                                             clippingResultsCount, planeNormal,
                                             planeDistance, clippedPoints);
    //No clipping occurred
    if(clippedPointCount == 0)
    {
      //No clipping will occur, quit
      return 0;
    }

    clippingResultsCount = clippedPointCount;
    memcpy(clippingResults, clippedPoints, sizeof(clippingResults[0]) *
                                                clippingResultsCount);
  }
  return clippedPointCount;
}

//Clip the set of coplanar polygon points against the planes that define the
//clipping polygon.
uint ClipPolygonWithPolygon(const Vec3* clipperPoints, uint clipperPointCount,
                            const Vec3* polygonPoints, uint polygonPointCount,
                            Vec3Ptr clippedPoints)
{
  ErrorIf(clipperPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(polygonPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(clippedPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");

  uint clippedPointCount = 0;

  ErrorIf(polygonPointCount < 2, "Geometry - Not enough points, can't clip.");

  if(clipperPointCount <= 2)
  {
    return 0;
  }

  //Face normal of the clipping polygon
  Vec3 faceNormal = GenerateNormal(clipperPoints[0], clipperPoints[1],
                                   clipperPoints[2]);

  //The points that define the polygon that is to be clipped
  Vec3 clippingResults[cMaxSupportPoints];

  //Number of points in the polygon
  uint clippingResultsCount = polygonPointCount;

  //Copy over the polygon's points into the temporary buffer
  memcpy(clippingResults, polygonPoints, sizeof(clippingResults[0]) *
                                              clippingResultsCount);

  //Start off with the last point
  uint curr = clipperPointCount - 1;

  //Edge of the face
  Vec3 edgeDirection;

  //Clip the polygon edges against all the infinite planes that define the face
  for(uint next = 0; next < clipperPointCount; curr = next, ++next)
  {
    //Compute new face edge
    edgeDirection = clipperPoints[next] - clipperPoints[curr];

    //Plane that defines the face's edge boundary
    Vec3 planeNormal = Cross(edgeDirection, faceNormal);
    real planeDistance = Dot(planeNormal, clipperPoints[curr]);

    clippedPointCount = ClipPolygonWithPlane(clippingResults,
                                             clippingResultsCount, planeNormal,
                                             planeDistance, clippedPoints);
    //No clipping occurred
    if(clippedPointCount == 0)
    {
      //No clipping will occur, quit
      return 0;
    }

    clippingResultsCount = clippedPointCount;
    memcpy(clippingResults, clippedPoints, sizeof(clippingResults[0]) *
                                                clippingResultsCount);
  }
  return clippedPointCount;
}

//Calculate the barycenter of the given point set.
void CalculateBarycenter(const Vec3* points, uint count, Vec3Ptr barycenter)
{
  ErrorIf(points == nullptr, "Geometry - Null pointer passed, this function "\
                             "needs a valid pointer.");
  ErrorIf(barycenter == nullptr, "Geometry - Null pointer passed, this function "\
                                 "needs a valid pointer.");

  *barycenter = points[0];
  for(uint i = 1; i < count; ++i)
  {
    *barycenter += points[i];
  }
  *barycenter /= real(count);
}

//Given n-gon specified by points v[], compute a good representative plane p.
void ComputeBestFitPlane(const Vec3* polyPoints, uint polyPointCount,
                         Vec3Ptr planeNormal, real* planeDistance)
{
  ErrorIf(polyPoints == nullptr, "Geometry - Null pointer passed, this "\
                                 "function needs a valid pointer.");
  ErrorIf(planeNormal == nullptr, "Geometry - Null pointer passed, this "\
                                  "function needs a valid pointer.");
  ErrorIf(planeDistance == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");

  //Compute normal as being proportional to projected areas of polygon onto the
  //yz, xz, and xy planes. Also compute centroid as representative point on the
  //plane.
  Vec3 centroid = Vec3::cZero;
  Vec3 normal = Vec3::cZero;
  for(uint i = polyPointCount - 1, j = 0; j < polyPointCount; i = j, ++j)
  {
    normal.x += (polyPoints[i].y - polyPoints[j].y) *
                (polyPoints[i].z + polyPoints[j].z);
    normal.y += (polyPoints[i].z - polyPoints[j].z) *
                (polyPoints[i].x + polyPoints[j].x);
    normal.z += (polyPoints[i].x - polyPoints[j].x) *
                (polyPoints[i].y + polyPoints[j].y);
    centroid += polyPoints[j];
  }

  //Normalize normal and fill in the plane equation fields.
  *planeNormal = normal;

  //"centroid / n" is the true centroid point.
  *planeDistance = Dot(centroid, normal) / real(polyPointCount);
}

//Calculate the volume of a triangular mesh.
real CalculateTriMeshVolume(const Vec3* triMeshPoints,
                            const uint* triMeshTriangles,
                            uint triangleCount, Vec3Param scale)
{
  ErrorIf(triMeshPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(triMeshTriangles == nullptr, "Geometry - Null pointer passed, this "\
                                       "function needs a valid pointer.");

  //Volume of the space defined by the mesh
  real volume = real(0.0);

  //Used to easily calculate tetrahedron volume
  Mat3 triPoints;

  //For each triangle...
  for(uint i = 0; i < triangleCount; ++i)
  {
    //Grab the points of the triangle and throw them into the matrix
    uint pointIndex = triMeshTriangles[i * 3];
    triPoints.SetCross(0, triMeshPoints[pointIndex] * scale);
    pointIndex = triMeshTriangles[(i * 3) + 1];
    triPoints.SetCross(1, triMeshPoints[pointIndex] * scale);
    pointIndex = triMeshTriangles[(i * 3) + 2];
    triPoints.SetCross(2, triMeshPoints[pointIndex] * scale);

    //Divide by 6 once in the end rather than every time
    volume += triPoints.Determinant();
  }

  return volume / real(6.0);
}

real CalculateTriMeshVolume(const Array<Vec3>& triMeshPoints,
                            const Array<uint>& triMeshTriangles, Vec3Param scale)
{
  uint triangleCount = triMeshTriangles.Size() / 3;
  return CalculateTriMeshVolume(triMeshPoints.Data(), triMeshTriangles.Data(), triangleCount, scale);
}

//Calculate the center of mass of a triangular mesh, assuming uniform density.
Vec3 CalculateTriMeshCenterOfMass(const Vec3* triMeshPoints,
                                  const uint* triMeshTriangles,
                                  uint triangleCount, Vec3Param scale)
{
  Vec3 centerOfMass;
  real volume;
  CalculateTriMeshCenterOfMassAndVolume(triMeshPoints, triMeshTriangles, triangleCount, centerOfMass, volume);
  return centerOfMass * scale;
}

Vec3 CalculateTriMeshCenterOfMass(const Array<Vec3>& triMeshPoints,
                                  const Array<uint>& triMeshTriangles,
                                  Vec3Param scale)
{
  uint triangleCount = triMeshTriangles.Size() / 3;
  return CalculateTriMeshCenterOfMass(triMeshPoints.Data(), triMeshTriangles.Data(), triangleCount, scale);
}

void CalculateTriMeshCenterOfMassAndVolume(const Vec3* triMeshPoints, 
                                           const uint* triMeshTriangles, 
                                           uint triangleCount,
                                           Vec3Ref centerOfMass, real& volume)
{
  ErrorIf(triMeshPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(triMeshTriangles == nullptr, "Geometry - Null pointer passed, this "\
                                       "function needs a valid pointer.");

  //Resulting center of mass of the mesh
  centerOfMass = Vec3(real(0.0));

  //Used to easily calculate tetrahedron volume
  Mat3 triPoints;

  //Store the volume for division later, accumulates 6x the volume
  volume = real(0.0);

  //For each triangle...
  for(uint i = 0; i < triangleCount; ++i)
  {
    //Grab the points of the triangle and throw them into the matrix
    uint pointIndex = triMeshTriangles[i * 3];
    triPoints.SetCross(0, triMeshPoints[pointIndex]);
    pointIndex = triMeshTriangles[(i * 3) + 1];
    triPoints.SetCross(1, triMeshPoints[pointIndex]);
    pointIndex = triMeshTriangles[(i * 3) + 2];
    triPoints.SetCross(2, triMeshPoints[pointIndex]);

    //Volume of the current tetrahedron. There is no point in dividing by 6
    //since we are calculating ratios. The volume of a tetrahedron is divided by
    //the total volume of the mesh, which gives that tetrahedron's contribution
    //to the total mesh volume as a value between 0 and 1. If we divided by 6,
    //both for the tetrahedral volume as well as the mesh volume, then the 6's
    //would cancel out and we would be left with the same ratio. :P
    real tetraVol = triPoints.Determinant();

    //Tetrahedron contributes its own center of mass, but weighted by its volume
    centerOfMass += tetraVol * (triPoints.GetCross(0) +
                                triPoints.GetCross(1) +
                                triPoints.GetCross(2));
    volume += tetraVol;
  }

  centerOfMass /= (real(4.0) * volume);
}

void CalculateTriMeshCenterOfMassAndVolume(const Array<Vec3>& triMeshPoints,
                                           const Array<uint>& triMeshTriangles,
                                           Vec3Ref centerOfMass, real& volume)
{
  uint triangleCount = triMeshTriangles.Size() / 3;
  CalculateTriMeshCenterOfMassAndVolume(triMeshPoints.Data(), triMeshTriangles.Data(),
                                        triangleCount, centerOfMass, volume);
}

//Calculate the inertia tensor of a triangular mesh. Assumes a mass of 1, which
//allows for the mass to be easily factored in later.
void CalculateTriMeshInertiaTensor(const Vec3* triMeshPoints,
                                   const uint* triMeshTriangles,
                                   uint triangleCount, Vec3Param centerOfMass,
                                   Mat3Ptr inertiaTensor, Vec3Param scale)
{
  ErrorIf(triMeshPoints == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");
  ErrorIf(triMeshTriangles == nullptr, "Geometry - Null pointer passed, this "\
                                       "function needs a valid pointer.");
  ErrorIf(inertiaTensor == nullptr, "Geometry - Null pointer passed, this "\
                                    "function needs a valid pointer.");

  //Double precision types are used to improve accuracy
  //typedef double real;

  //Accumulate matrix main diagonal integrals [x*x, y*y, z*z]
  double diagElement[3] = { double(0.0), double(0.0), double(0.0) };

  //Accumulate matrix off-diagonal integrals [y*z, x*z, x*y]
  double offDiagElement[3] = { double(0.0), double(0.0), double(0.0) };

  //Used to easily calculate tetrahedron volume
  Vec3 triPoint[3];

  //Store the volume for division later, accumulates 6x the volume
  double meshVolume = double(0.0);

  //For each triangle...
  for(uint i = 0; i < triangleCount; ++i)
  {
    //Grab the points of the triangle and throw them into the array
    uint pointIndex = triMeshTriangles[i * 3];
    triPoint[0] = (triMeshPoints[pointIndex] - centerOfMass) * scale; // Storage Visual
    pointIndex = triMeshTriangles[(i * 3) + 1];                       // | Ax  Ay  Az |
    triPoint[1] = (triMeshPoints[pointIndex] - centerOfMass) * scale; // | Bx  By  Bz |
    pointIndex = triMeshTriangles[(i * 3) + 2];                       // | Cx  Cy  Cz |
    triPoint[2] = (triMeshPoints[pointIndex] - centerOfMass) * scale;

    //Volume of tiny parallelepiped = d * dR * dS * dT (the 3 partials of the
    //tetrahedral triple integral equation). Scalar triple product (a * (b x c))
    double tetraVolume = double(Dot(triPoint[0], Cross(triPoint[1], triPoint[2])));

    //Add volume of current tetrahedron (Note: it could be negative - that's ok,
    //we need that sometimes)
    meshVolume += tetraVolume;

    //For each axis...
    for(uint j = 0; j < 3; ++j)
    {
      uint u = (j + 1) % 3;
      uint v = (j + 2) % 3;

      //Diagonal inertia tensor element calculation
      diagElement[j] += double((triPoint[0][j] * triPoint[1][j] + // Aj * Bj
                              triPoint[1][j] * triPoint[2][j] + // Bj * Cj
                              triPoint[2][j] * triPoint[0][j] + // Cj * Aj
                              Math::Sq(triPoint[0][j]) +        // Aj * Aj
                              Math::Sq(triPoint[1][j]) +        // Bj * Bj
                              Math::Sq(triPoint[2][j])) *       // Cj * Cj
                              tetraVolume);

      //Off-diagonal inertia tensor element calculation
      offDiagElement[j] += double(tetraVolume *
                                 //   Au * (2 * Av + Bv + Cv)
                                (triPoint[0][u] * (triPoint[0][v]  +
                                                   triPoint[0][v]  +
                                                   triPoint[1][v]  +
                                                   triPoint[2][v]) +
                                 //   Bu * (Av + 2 * Bv + Cv)
                                 triPoint[1][u] * (triPoint[0][v]  +
                                                   triPoint[1][v]  +
                                                   triPoint[1][v]  +
                                                   triPoint[2][v]) +
                                 //   Cu * (Av + Bv + 2 * Cv)
                                 triPoint[2][u] * (triPoint[0][v]  +
                                                   triPoint[1][v]  +
                                                   triPoint[2][v]  +
                                                   triPoint[2][v])));
    }
  }

  //Divide by total volume (volume/6) since density = 1/volume
  double totalVolume = meshVolume * double(60.0 / 6.0);
  diagElement[0] /= totalVolume;
  diagElement[1] /= totalVolume;
  diagElement[2] /= totalVolume;

  totalVolume = meshVolume * double(120.0 / 6.0);
  offDiagElement[0] /= totalVolume;
  offDiagElement[1] /= totalVolume;
  offDiagElement[2] /= totalVolume;

  //Diagonal elements of the inertia tensor matrix
  (*inertiaTensor)(0, 0) = real(diagElement[1] + diagElement[2]);
  (*inertiaTensor)(1, 1) = real(diagElement[0] + diagElement[2]);
  (*inertiaTensor)(2, 2) = real(diagElement[0] + diagElement[1]);

  //Off-diagonal elements of the inertia tensor matrix
  (*inertiaTensor)(0, 1) = real(-offDiagElement[2]);
  (*inertiaTensor)(0, 2) = real(-offDiagElement[1]);
  (*inertiaTensor)(1, 0) = real(-offDiagElement[2]);
  (*inertiaTensor)(1, 2) = real(-offDiagElement[0]);
  (*inertiaTensor)(2, 0) = real(-offDiagElement[1]);
  (*inertiaTensor)(2, 1) = real(-offDiagElement[0]);
}

void CalculateTriMeshInertiaTensor(const Array<Vec3>& triMeshPoints,
                                   const Array<uint>& triMeshTriangles, Vec3Param centerOfMass,
                                   Mat3Ptr inertiaTensor, Vec3Param scale)
{
  uint triangleCount = triMeshTriangles.Size() / 3;
  CalculateTriMeshInertiaTensor(triMeshPoints.Data(), triMeshTriangles.Data(), triangleCount,
                                centerOfMass, inertiaTensor, scale);
}

void CombineInertiaTensor(Mat3Ref totalInertiaTensor, Vec3Param totalCenterOfMass,
                          Mat3Param localInertiaTensor, Vec3Param localCenterOfMass, real localMass)
{
  Mat3 identity;
  identity.SetIdentity();

  Vec3 r = localCenterOfMass - totalCenterOfMass;
  Mat3 outerProduct;
  outerProduct.SetCross(0, r[0] * r);
  outerProduct.SetCross(1, r[1] * r);
  outerProduct.SetCross(2, r[2] * r);

  totalInertiaTensor += localInertiaTensor + (Math::Dot(r, r) * identity - outerProduct) * localMass;
}

//Find the point furthest in the direction on an axis-aligned bounding box.
void SupportAabb(Vec3Param direction, Vec3Param aabbMinPoint,
                 Vec3Param aabbMaxPoint, Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                              "needs a valid pointer.");

  *support = real(0.5) * (aabbMaxPoint - aabbMinPoint);
  support->x *= Math::GetSign(direction.x);
  support->y *= Math::GetSign(direction.y);
  support->z *= Math::GetSign(direction.z);
  *support += real(0.5) * (aabbMaxPoint + aabbMinPoint);
}

//Find the point furthest in the direction on a capsule.
void SupportCapsule(Vec3Param direction, Vec3Param capsulePointA,
                    Vec3Param capsulePointB, real capsuleRadius,
                    Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                              "needs a valid pointer.");

  SupportSegment(direction, capsulePointA, capsulePointB, support);
  Vec3 endCapSupport;
  SupportSphere(direction, Vec3::cZero, capsuleRadius, &endCapSupport);
  *support += endCapSupport;
}

//Find the point furthest in the direction on a cylinder.
void SupportCylinder(Vec3Param direction, Vec3Param cylinderPointA,
                     Vec3Param cylinderPointB, real cylinderRadius,
                     Vec3Ptr support)
{
  Vec3 yAxis = cylinderPointB - cylinderPointA;
  real halfLength = real(0.5) * Normalize(yAxis);
  Vec3 center = cylinderPointA + (halfLength * yAxis);
  Vec3 xAxis, zAxis;
  Math::GenerateOrthonormalBasis(yAxis, &zAxis, &xAxis);
  Mat3 basis;
  basis.SetBasis(0, xAxis);
  basis.SetBasis(1, yAxis);
  basis.SetBasis(2, zAxis);
  SupportCylinder(direction, center, halfLength, cylinderRadius, basis,
                  support);
}

//Find the point furthest in the direction on a cylinder.
void SupportCylinder(Vec3Param direction, Vec3Param cylinderCenter,
                     real cylinderHalfHeight, real cylinderRadius,
                     Mat3Param cylinderBasis, Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  //First take the direction vector into the cylinder's body space
  *support = Math::TransposedTransform(cylinderBasis, direction);

  //Save the y-value for later
  real y = support->y;

  //Get the support point on the disc
  support->y = real(0.0);
  if(support->x != real(0.0) || support->z != real(0.0))
  {
    if(cGeometrySafeChecks)
    {
      support->AttemptNormalize();
    }
    else
    {
      Normalize(*support);
    }

    *support *= cylinderRadius;
  }

  //Get the support point on the line segment
  support->y = cylinderHalfHeight * Math::GetSign(y);

  //Transform the vector back into world space
  Math::Transform(cylinderBasis, support);

  *support += cylinderCenter;
}

//Find the point furthest in the direction on an ellipsoid.
void SupportEllipsoid(Vec3Param direction, Vec3Param ellipsoidCenter,
                      Vec3Param ellipsoidRadii, Mat3Param ellipsoidBasis,
                      Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  //First take the direction vector into the ellipsoid's body space
  *support = Math::TransposedTransform(ellipsoidBasis, direction);

  support->x *= ellipsoidRadii.x;
  support->y *= ellipsoidRadii.y;
  support->z *= ellipsoidRadii.z;

  real length = Length(*support);

  support->x *= ellipsoidRadii.x;
  support->y *= ellipsoidRadii.y;
  support->z *= ellipsoidRadii.z;

  *support /= length;

  Math::Transform(ellipsoidBasis, support);

  *support += ellipsoidCenter;
}

//Find the point furthest in the direction on an oriented bounding box. The
//direction vector is expected to be unit length.
void SupportObb(Vec3Param direction, Vec3Param obbCenter,
                Vec3Param obbHalfExtents, Mat3Param obbBasis, Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  //First take the direction vector into the box's body space
  *support = Math::TransposedTransform(obbBasis, direction);

  //Then calculate the point in the furthest direction based on the sign of
  //the components of the direction vector
  support->x = obbHalfExtents.x * Math::GetSign(support->x);
  support->y = obbHalfExtents.y * Math::GetSign(support->y);
  support->z = obbHalfExtents.z * Math::GetSign(support->z);

  //Transform the direction vector back into world space
  Math::Transform(obbBasis, support);

  //Add the position of the box to the direction vector
  *support += obbCenter;
}

///Find the point furthest in the direction for a given set of points. The
///direction vector is expected to be unit length;
void SupportPointSet(Vec3Param direction, const Vec3Ptr points, uint pointCount,
                     Vec3Param center, Vec3Param scale, Mat3Param basis,
                     Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  //First take the direction vector into the octahedron's body space.
  Vec3 localDirection = Math::TransposedTransform(basis, direction);

  //Then go through all of the points in the octahedron's body space and find
  //the point furthest in the direction of body space direction.
  uint furthestPoint = uint(-1);
  real maxProj = -Math::PositiveMax();
  for(uint i = 0; i < pointCount; ++i)
  {
    Vec3 point = points[i];
    point *= scale;
    real proj = Dot(direction, point);
    if(maxProj < proj)
    {
      maxProj = proj;
      furthestPoint = i;
    }
  }

  *support = points[furthestPoint];
  (*support) *= scale;

  //Transform the body space point into world space.
  Math::Transform(basis, support);

  //Add the position of the octahedron to the body space point.
  *support += center;
}

//Find the point furthest in the direction on a segment. The direction vector
//is expected to be unit length.
void SupportSegment(Vec3Param direction, Vec3Param segmentStart,
                    Vec3Param segmentEnd, Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  //Calculate the center of the segment.
  Vec3 segmentCenter = real(0.5) * (segmentStart + segmentEnd);

  //Move the segment's endpoints to be about the origin
  Vec3 segment[2] = { segmentStart - segmentCenter,
                      segmentEnd - segmentCenter    };

  //Segment's start is further in the direction
  if(Dot(direction, segment[0]) > Dot(direction, segment[1]))
  {
    *support = segmentStart;
  }
  else
  {
    *support = segmentEnd;
  }
}

//Find the point furthest in the direction on a sphere. The direction vector is
//expected to be unit length.
void SupportSphere(Vec3Param direction, Vec3Param sphereCenter,
                   real sphereRadius, Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  *support = Normalized(direction);
  *support *= sphereRadius;
  *support += sphereCenter;
}

//Find the point furthest in the direction on a tetrahedron. The direction
//vector is expected to be unit length.
void SupportTetrahedron(Vec3Param direction, Vec3Param tetraPointA,
                        Vec3Param tetraPointB, Vec3Param tetraPointC,
                        Vec3Param tetraPointD, Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  Vec3 center  = tetraPointA + tetraPointB + tetraPointC + tetraPointD;
       center /= real(4.0);
  Vec3 tetraPoints[4] = { tetraPointA - center, tetraPointB - center,
                          tetraPointC - center, tetraPointD - center };
  real maxDot = -Math::PositiveMax();
  for(uint i = 0; i < 4; ++i)
  {
    real proj = Dot(direction, tetraPoints[i]);
    if(proj > maxDot)
    {
      maxDot = proj;
      *support = tetraPoints[i];
    }
  }
  *support += center;
}

//Find the point furthest in the direction on a triangle. The direction vector
//is expected to be unit length.
void SupportTriangle(Vec3Param direction, Vec3Param trianglePointA,
                     Vec3Param trianglePointB, Vec3Param trianglePointC,
                     Vec3Ptr support)
{
  ErrorIf(support == nullptr, "Geometry - Null pointer passed, this function "\
                           "needs a valid pointer.");

  Vec3 center  = trianglePointA + trianglePointB + trianglePointC;
       center /= real(3.0);
  Vec3 triPoints[3] = { trianglePointA - center,
                        trianglePointB - center,
                        trianglePointC - center };
  real maxDot = -Math::PositiveMax();
  for(uint i = 0; i < 3; ++i)
  {
    real proj = Dot(direction, triPoints[i]);
    if(proj > maxDot)
    {
      maxDot = proj;
      *support = triPoints[i];
    }
  }
  *support += center;
}

//Get the normal on an axis-aligned bounding box at the specified point on the
//given axis-aligned bounding box.
Vec3 NormalFromPointOnAabb(Vec3Param point, Vec3Param aabbMinPoint,
                           Vec3Param aabbMaxPoint)
{
  //The normal is the face normal of the face of the AABB that the given point
  //is closest to.
  const Vec3 normals[6] = { -Vec3::cXAxis, -Vec3::cYAxis, -Vec3::cZAxis,
                             Vec3::cXAxis,  Vec3::cYAxis,  Vec3::cZAxis };
  real differences[6] = { Math::Abs(point.x - aabbMinPoint.x),
                          Math::Abs(point.y - aabbMinPoint.y),
                          Math::Abs(point.z - aabbMinPoint.z),
                          Math::Abs(point.x - aabbMaxPoint.x),
                          Math::Abs(point.y - aabbMaxPoint.y),
                          Math::Abs(point.z - aabbMaxPoint.z) };
  real minDiff = Math::PositiveMax();
  uint minIndex = uint(-1);
  for(uint i = 0; i < 6; ++i)
  {
    if(differences[i] < minDiff)
    {
      minDiff = differences[i];
      minIndex = i;
    }
  }
  return normals[minIndex];
}

//Get the normal on a capsule at the specified point on the given capsule.
Vec3 NormalFromPointOnCapsule(Vec3Param point, Vec3Param capsulePointA,
                              Vec3Param capsulePointB, real capsuleRadius)
{
  Vec3 aToB = capsulePointB - capsulePointA;
  real t = Dot(aToB, point - capsulePointA);

  if(t < real(0.0))
  {
    //Spherical endcap near point A
    return Normalized(point - capsulePointA);
  }
  else
  {
    real length = Dot(aToB, aToB);
    if(t > length)
    {
      //Spherical endcap near point B
      return Normalized(point - capsulePointB);
    }
    else
    {
      t /= length;
      aToB *= t;
      aToB += capsulePointA;
      return Normalized(point - aToB);
    }
  }
}

//Get the normal on a cylinder at the specified point on the given cylinder.
Vec3 NormalFromPointOnCylinder(Vec3Param point, Vec3Param cylinderCenter,
                               real cylinderRadius, real cylinderHalfHeight,
                               Mat3Param cylinderBasis)
{
  Vec3 cylinderPoint = point - cylinderCenter;
  Math::TransposedTransform(cylinderBasis, &cylinderPoint);

  if(cylinderPoint.y < -(cylinderHalfHeight - cCylinderEndcapThreshold))
  {
    return -cylinderBasis.BasisY();
  }
  else if(cylinderPoint.y > (cylinderHalfHeight - cCylinderEndcapThreshold))
  {
    return cylinderBasis.BasisY();
  }

  //Shaft point
  cylinderPoint.y = real(0.0);
  Math::Transform(cylinderBasis, &cylinderPoint);
  Normalize(cylinderPoint);
  return cylinderPoint;
}


//Get the normal on an ellipsoid at the specified point on the given ellipsoid.
Vec3 NormalFromPointOnEllipsoid(Vec3Param point, Vec3Param ellipsoidCenter,
                                Vec3Param ellipsoidRadii,
                                Mat3Param ellipsoidBasis)
{
  Vec3 localPoint = point - ellipsoidCenter;
  Vec3 invScale = Vec3(real(1.0) / ellipsoidRadii.x,
                       real(1.0) / ellipsoidRadii.y,
                       real(1.0) / ellipsoidRadii.z);
  Mat3 scaling = Mat3::cIdentity;
       scaling.Scale(invScale);
  Mat3 rotation = ellipsoidBasis.Transposed();
  Mat3 normalMatrix = Multiply(scaling, rotation);
  Transform(Multiply(normalMatrix.Transposed(), normalMatrix), &localPoint);

  return Normalized(localPoint);
}

//Get the normal on an oriented bounding box at the specified point on the
//given oriented bounding box.
Vec3 NormalFromPointOnObb(Vec3Param point, Vec3Param obbCenter,
                          Vec3Param obbHalfExtents, Mat3Param obbBasis)
{
  Vec3 pointInModel = point - obbCenter;
  Math::TransposedTransform(obbBasis, &pointInModel);
  Vec3 boxMin = -obbHalfExtents;
  Vec3 boxMax = obbHalfExtents;
  Vec3 normal = NormalFromPointOnAabb(pointInModel, boxMin, boxMax);
  Math::Transform(obbBasis, &normal);
  return normal;
}

//Get the normal on a sphere at the specified point on the given sphere.
Vec3 NormalFromPointOnSphere(Vec3Param point, Vec3Param sphereCenter,
                             real sphereRadius)
{
  Vec3 normal = point - sphereCenter;
  if(cGeometrySafeChecks)
  {
    real length = AttemptNormalize(normal);
    if(Math::IsZero(length))
    {
      return Vec3::cZero;
    }
  }
  return Normalized(normal);
}

//Get the normal on a torus at the specified point on the given triangle. The
//torus's z-axis is the axis going through its hole.
Vec3 NormalFromPointOnTorus(Vec3Param point, Vec3Param torusCenter,
                            real torusRingRadius, real torusTubeRadius,
                            Mat3Param torusBasis)
{
  Vec3 normal = TransposedTransform(torusBasis, point - torusCenter);

  real x = normal.x;
  real y = normal.y;
  real z = normal.z;
  real xyz = (x * x) + (y * y) + (z * z);
  real r = torusTubeRadius * torusTubeRadius;
  real R = torusRingRadius * torusRingRadius;
  normal.Set(real(4.0) * x * (xyz - R - r),
             real(4.0) * y * (xyz - R - r),
             real(4.0) * z * (xyz + R - r));
  Transform(torusBasis, &normal);
  return Normalized(normal);
}

//Get the normal on a triangle at the specified point on the given triangle.
Vec3 NormalFromPointOnTriangle(Vec3Param point, Vec3Param trianglePointA,
                               Vec3Param trianglePointB,
                               Vec3Param trianglePointC)
{
  //This may want to return the edge normal if the given point lies on the edge
  //of the triangle, but I chose not to do this.
  return GenerateNormal(trianglePointA, trianglePointB, trianglePointC);
}

//Get the texture coordinates on a cylinder at the specified point on the given
//cylinder.
Vec2 TextureCoordinatesFromPointOnCylinder(Vec3Param point,
                                           Vec3Param cylinderCenter,
                                           real cylinderHalfHeight,
                                           real cylinderRadius,
                                           Mat3Param cylinderBasis)
{
  //Bring the point into the box's local space.
  Vec3 localPoint = point - cylinderCenter;
  TransposedTransform(cylinderBasis, &localPoint);

  //Determine which part of the cylinder the point is on (endcaps or middle?).
  if(localPoint.y < -(cylinderHalfHeight - cCylinderEndcapThreshold) ||
     localPoint.y > (cylinderHalfHeight - cCylinderEndcapThreshold))
  {
    real diameter = cylinderRadius * real(2.0);
    Vec2 uv = Vec2((localPoint.x / diameter) + real(0.5),
                   (localPoint.z / diameter) + real(0.5));
    return uv;
  }

  real angle = Math::ArcTan2(localPoint.z, localPoint.x);
  angle /= Math::cTwoPi;
  real v = (localPoint.y / (real(2.0) * cylinderHalfHeight)) + real(0.5);
  return Vec2(angle, v);
}

//Get the texture coordinates on an ellipsoid at the specified point on the
//given ellipsoid.
Vec2 TextureCoordinatesFromPointOnEllipsoid(Vec3Param point,
                                            Vec3Param ellipsoidCenter,
                                            Vec3Param ellipsoidRadii,
                                            Mat3Param ellipsoidBasis)
{
  Vec3 localPoint = point - ellipsoidCenter;
  Vec3 invScale = Vec3(real(1.0) / ellipsoidRadii.x,
                       real(1.0) / ellipsoidRadii.y,
                       real(1.0) / ellipsoidRadii.z);
  Mat3 scaling = Mat3::cIdentity;
       scaling.Scale(invScale);
  Mat3 rotation = ellipsoidBasis.Transposed();
  Mat3 normalMatrix = Multiply(scaling, rotation);
  Transform(normalMatrix, &localPoint);
  Normalize(localPoint);

  Vec2 uvCoords;

  //Compute the texture coordinates at the local space point position.
  real normalizedX = real(0.0);
  real normalizedZ = real(-1.0);
  real sqNormalX = localPoint.x * localPoint.x;
  real sqNormalZ = localPoint.z * localPoint.z;
  if((sqNormalX + sqNormalZ) > real(0.0))
  {
    normalizedX = Math::Sqrt(sqNormalX / (sqNormalX + sqNormalZ));
    if(localPoint.x < real(0.0))
    {
      normalizedX = -normalizedX;
    }
    normalizedZ = Math::Sqrt(sqNormalZ / (sqNormalX + sqNormalZ));

    if(localPoint.z < real(0.0))
    {
      normalizedZ = -normalizedZ;
    }
  }

  if(normalizedZ == real(0.0))
  {
    uvCoords[0] = Math::cPi * real(0.5) * normalizedX;
  }
  else
  {
    uvCoords[0] = Math::ArcTan(normalizedX / normalizedZ);
    if(normalizedZ < real(0.0))
    {
      uvCoords[0] += Math::cPi;
    }
  }
  if(uvCoords[0] < real(0.0))
  {
    uvCoords[0] += real(2.0) * Math::cPi;
  }
  uvCoords[0] /= real(2.0) * Math::cPi;
  uvCoords[1] = (real(1.0) - localPoint.y) / real(2.0);

  return uvCoords;
}

//Get the texture coordinates on an oriented bounding box at the specified point
//on the given oriented bounding box.
Vec2 TextureCoordinatesFromPointOnObb(Vec3Param point,
                                      Vec3Param obbCenter,
                                      Vec3Param obbHalfExtents,
                                      Mat3Param obbBasis)
{
  //Bring the point into the box's local space.
  Vec3 localPoint = point - obbCenter;
  TransposedTransform(obbBasis, &localPoint);

  //Find the side of the box that is closest
  real differences[3] = { Math::Abs(Math::Abs(localPoint.x) - obbHalfExtents.x),
                          Math::Abs(Math::Abs(localPoint.y) - obbHalfExtents.y),
                          Math::Abs(Math::Abs(localPoint.z) - obbHalfExtents.z) };
  real maxLength = Math::PositiveMax();
  uint a, b, c;
  for(uint i = 0; i < 3; ++i)
  {
    if(differences[i] < maxLength)
    {
      maxLength = differences[i];
      a = i;
      b = (a + 1) % 3;
      c = (b + 1) % 3;
    }
  }

  if(b >= 3 || c >= 3)
  {
    Error("Invalid points, did we have bad floating point values?");
    return Vec2::cZero;
  }

  //Make sure the y-axis is always tied to the v-axis.
  if(b == 1)
  {
    Math::Swap(b, c);
  }
  Vec3 fullExtents = obbHalfExtents * real(2.0);
  Vec2 uv = Vec2((localPoint[b] / fullExtents[b]) + real(0.5),
                 (localPoint[c] / fullExtents[c]) + real(0.5));
  return uv;
}

//Get the texture coordinates on a sphere at the specified point on the sphere.
Vec2 TextureCoordinatesFromPointOnSphere(Vec3Param point,
                                         Vec3Param sphereCenter,
                                         real sphereRadius,
                                         Mat3Param sphereBasis)
{
  //Bring the point into the sphere's local space.
  Vec3 localPoint = point - sphereCenter;
  TransposedTransform(sphereBasis, &localPoint);
  AttemptNormalize(localPoint);

  Vec2 uvCoords;

  //Compute the texture coordinates at the local space point position.
  real normalizedX = real(0.0);
  real normalizedZ = real(-1.0);
  real sqNormalX = localPoint.x * localPoint.x;
  real sqNormalZ = localPoint.z * localPoint.z;
  if((sqNormalX + sqNormalZ) > real(0.0))
  {
    normalizedX = Math::Sqrt(sqNormalX / (sqNormalX + sqNormalZ));
    if(localPoint.x < real(0.0))
    {
      normalizedX = -normalizedX;
    }
    normalizedZ = Math::Sqrt(sqNormalZ / (sqNormalX + sqNormalZ));

    if(localPoint.z < real(0.0))
    {
      normalizedZ = -normalizedZ;
    }
  }

  if(normalizedZ == real(0.0))
  {
    uvCoords[0] = Math::cPi * real(0.5) * normalizedX;
  }
  else
  {
    uvCoords[0] = Math::ArcTan(normalizedX / normalizedZ);
    if(normalizedZ < real(0.0))
    {
      uvCoords[0] += Math::cPi;
    }
  }
  if(uvCoords[0] < real(0.0))
  {
    uvCoords[0] += real(2.0) * Math::cPi;
  }
  uvCoords[0] /= real(2.0) * Math::cPi;
  uvCoords[1] = (real(1.0) - localPoint.y) / real(2.0);

  return uvCoords;
}

}// namespace Geometry
