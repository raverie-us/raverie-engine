///////////////////////////////////////////////////////////////////////////////
///
/// \file Mpr.cpp
/// MPR algorithm for finding the closest features of two convex shapes.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Mpr.hpp"
#include "Geometry/Geometry.hpp"

#include "Geometry/DebugDraw.hpp"

namespace Intersection
{
namespace
{
const real cContainmentEpsilon = real(0.005);
const real cDiscoveryEpsilon = real(0.0001220703125);
const real cRefinementEpsilon = cDiscoveryEpsilon;
const uint cDiscoveryIterations = 25;
const uint cRefinementIterations = 25;

const real cTimeOfIntersectionEpsilon = real(-0.01);

const Vec3 cSafetyDirection = Vec3(real(0.57735026918962576450914878050196),
                                   real(0.57735026918962576450914878050196),
                                   real(0.57735026918962576450914878050196));

const uint cLeftShape = 0;
const uint cRightShape = 1;
}//namespace

//-------------------------------------------------------------------------- Mpr
/*
  Pseudocode for Mpr

  //Phase 1: Portal Discovery
  find_origin_ray();
  find_candidate_portal();
  while(origin ray does not intersect candidate)
  {
    choose_new_candidate();
  }
  
  //Phase 2: Portal Refinement
  while(true)
  {
    if(origin inside portal)
      return HIT;

    find_support_in_direction_of_portal();
    if(origin outside support plane)
      return MISS;

    if(support plane close to point)
      return MISS;

    choose_new_portal();
  }
*/

Type Mpr::Test(const SupportShape* shapeA, const SupportShape* shapeB, 
               Intersection::Manifold* manifold)
{
  Init(shapeA, shapeB, Discrete);

  CalculateCenter();

  //No good solution if the two objects share the same center, abort!
  if(mCenter[cLeftShape] == mCenter[cRightShape])
  {
    return Intersection::None;
  }

  //Find support in the direction of the origin ray
  //From the interior point, to the origin (origin - v0)
  mDirection = -mCsoCenter;

  if(InitialPortal() == false)
  {
    return Intersection::None;
  }

  if(Discover() == false)
  {
    return Intersection::None;
  }

  if(Refine(false) == false)
  {
    return Intersection::None;
  }

  if(manifold != nullptr)
  {
    //Refine the collision information
    mCsoCenter = -mDirection;

    //InitialPortal();

    if(Discover() == false)
    {
      return Intersection::None;
    }

    if(Refine(true))
    {
      FillManifold(*manifold);
      //DrawDebug(DebugDraw::DrawMpr | DebugDraw::DrawOnTop);
      return Intersection::Other;
    }
    return Intersection::None;
  }
  return Intersection::Other;
}

///Check to see if (and when) two moving shapes are intersecting
Type Mpr::SweptTest(const SupportShape* shapeA, const SupportShape* shapeB, 
                    Intersection::Manifold* manifold)
{
  Init(shapeA, shapeB, Swept);
  CalculateTranslation();
  CalculateCenter();
  
  //Center is offset by 1/2 of the distance of the volumes' total translation
  mCsoCenter = Math::MultiplyAdd(mCsoCenter, mTranslation, real(0.5));

  //Find support in the direction of the origin ray
  //From the interior point, to the origin (origin - v0)
  mDirection = -mCsoCenter;
  Normalize(mDirection);

  InitialPortal();

  if(Discover() == false)
  {
    return Intersection::None;
  }

  //First check if the objects have collided
  if(Refine(false))
  {
    if(manifold != nullptr)
    {
//       Vec3 translation = -mTranslation;
//       mDirection = translation;
//       Normalize(mDirection);
// 
//       GetBias(mCsoCenter);
//       mCsoCenter.AddScaledVector(translation, real(-0.02));
// 
//       InitialPortal();
//       if(Discover() == false)
//       {
//         return Mintersect::None;
//       }
//       Refine(true);
// 
//       Ray ray(cOrigin, translation, Ray::GetMaxLength());
//       RayCast::HalfspaceRay(mCsoSupport[0], mCsoSupport[2], 
//                   mCsoSupport[1], ray);
//       real translationLength = Mag(translation);
//       real differenceLength = ray.LengthToPoint();
//       real t = differenceLength / translationLength;
//       t += cTimeOfIntersectionEpsilon;
//       t = real(1.0) - t;
// 
//       //Offset for the support points as if the left object were at the top
//       mLeftOffset  = mTranslation;
//       mLeftOffset *= t - real(1.0);
// 
//       //Modified version of center calculation
//       mCenter[cLeftShape] += mLeftOffset;
//       mShapes[1]->GetCenter(mCenter[cRightShape]);
//       mCsoCenter  = mCenter[cRightShape];
//       mCsoCenter -= mCenter[cLeftShape];
//       mCsoCenter += GetBias();
// 
//       //Find support in the direction of the origin ray
//       //From the interior point, to the origin (origin - v0)
//       mDirection  = mCsoCenter;
//       mDirection *= -real(1.0);
//       Normalize(mDirection);
// 
//       InitialPortal();
//       if(Discover() == false)
//       {
//         return Mintersect::None;
//       }
//       Refine(false);
// 
//       mCsoCenter  = mDirection;
//       mCsoCenter *= -real(1.0);
// 
//       InitialPortal();
//       if(Discover() == false)
//       {
//         return Mintersect::None;
//       }
// 
//       Refine(true);
//       FillManifold(shapeA, shapeB, *manifold);
    }
    return Intersection::Other;
  }
  return Intersection::None;
}

///Calculate the CSO's center bases off of the centers of the two shapes
void Mpr::CalculateCenter(void)
{
  mShapes[cLeftShape]->GetCenter(&(mCenter[cLeftShape]));
  mShapes[cRightShape]->GetCenter(&(mCenter[cRightShape]));
  mCsoCenter = mCenter[cRightShape] - mCenter[cLeftShape];
}

///Calculate the relative motion of the two shapes.
void Mpr::CalculateTranslation(void)
{
  mShapes[cLeftShape]->GetTranslation(&mTranslation);

  Vec3 translation;
  mShapes[cRightShape]->GetTranslation(&translation);
  mTranslation -= translation;
}

///Get the point furthest in the stored direction on the CSO and store the 
///results in the specified index
bool Mpr::Support(uint index)
{
  Vec3 direction = mDirection;
//   if(AttemptNormalize(&direction) == real(0.0))
//   {
//     return false;
//   }

  mShapes[cRightShape]->Support(direction, &(mRightSupport[index]));

  Negate(&direction);
  mShapes[cLeftShape]->Support(direction, &(mLeftSupport[index]));
  
  mCsoSupport[index] = mRightSupport[index] - mLeftSupport[index];
  return true;
}

///Compute the initial portal to start the algorithm
bool Mpr::InitialPortal(void)
{
  Support(0);
  
  //We will never be able to encapsulate the origin if it's located further
  //away than the first support result in the initial direction
  if(Dot(mCsoSupport[0], mDirection) < real(0.0))
  {
    return false;
  }

  //Find support perpendicular to plane containing origin, interior point, 
  //and first support
  Vec3 temp = mCsoSupport[0];
  mDirection = Cross(temp, mCsoCenter);

  //Check to make sure that the direction is valid, and attempt to make it
  //valid if it is invalid.
  real length = Length(mDirection);
  if(length == real(0.0))
  {
    mDirection = mCsoSupport[0];
    Math::Swap(mDirection.x, mDirection.y);
    mDirection.z *= real(-1.0);
  }
  //Deal with any invalid direction vector. Just offset the cso center
  //in the x, y and z axes to make sure we get a non zero vector.
  uint tempIndex = 0;
  while(Length(mDirection) == real(0.0) && tempIndex < 3)
  {
    Vec3 randOffset = Vec3::cZero;
    randOffset[tempIndex] = real(.1);
    mDirection = Cross(temp, mCsoCenter + randOffset);
    ++tempIndex;
  }
  Support(1);

  //Find support perpendicular to plane containing interior point and first
  //two supports
  mDirection = mCsoSupport[1] - mCsoCenter;
  temp -= mCsoCenter;
  mDirection = Cross(temp, mDirection);
  //Make sure we get a valid direction vector. See the above comment.
  while(Length(mDirection) == real(0.0) && tempIndex < 3)
  {
    Vec3 randOffset = Vec3::cZero;
    randOffset[tempIndex] = real(.1);
    mDirection = mCsoSupport[1] - mCsoCenter + randOffset;
    temp -= mCsoCenter;
    mDirection = Cross(temp, mDirection);
    ++tempIndex;
  }
  //AttemptNormalize(&mDirection);
  Support(2);

  return true;
}

///Iterate over the CSO hull to find a portal which Contains the origin ray
bool Mpr::Discover(void)
{
  //Begin assuming origin is NOT in the portal
  bool originRayNotInsidePortal = true;
  
  //If origin is outside a face of the portal, the point not on that faces is
  //invalidated and a new point must be found.
  uint i = 0;
  while(originRayNotInsidePortal && i < cDiscoveryIterations)
  {
    if(!PortalFaceCheck(0, 2, 1, originRayNotInsidePortal))
    {
      return false;
    }

    if(!PortalFaceCheck(1, 0, 2, originRayNotInsidePortal))
    {
      return false;
    }

    if(!PortalFaceCheck(2, 1, 0, originRayNotInsidePortal))
    {
      return false;
    }
//     //Cross (v01, v03)
//     vecA = mCsoSupport[0] + toOrigin;
//     vecB = mCsoSupport[2] + toOrigin;
//     normal = Cross(vecA, vecB);
//     real dot = Dot(normal, toOrigin);
//     if(dot > -cDiscoveryEpsilon)
//     {
//       //Support point v2 must be changed
//       mDirection = normal;
//       if(!Support(1))
//       {
//         return false; //Invalid direction, bad situation, abort!
//       }
// 
//       //Points v1 and v3 must be swapped
//       Swap(0, 2);
// 
//       //Still not sure if origin is in portal...
//       originRayNotInsidePortal = true;
//     }
//     else
//     {
//       //Everything is ok, continue testing
//       originRayNotInsidePortal = false;
//     }
//     
//     //Cross (v03, v02)
//     vecA = mCsoSupport[2] + toOrigin; 
//     vecB = mCsoSupport[1] + toOrigin;
//     normal = Cross(vecA, vecB);
//     dot = Dot(normal, toOrigin);
//     if(dot > -cDiscoveryEpsilon)
//     {
//       //Support point v1 must be changed
//       mDirection = normal;
//       if(!Support(0))
//       {
//         return false; //Invalid direction, bad situation, abort!
//       }
// 
//       //Points v2 and v3 must be swapped
//       Swap(1, 2);
// 
//       //Still not sure if origin is in portal...
//       originRayNotInsidePortal = true;
//     }
//     else
//     {
//       //Everything is ok, continue testing
//       originRayNotInsidePortal = false;
//     }
// 
//     //Cross (v02, v01)
//     vecA = mCsoSupport[1] + toOrigin;
//     vecB = mCsoSupport[0] + toOrigin;
//     normal = Cross(vecA, vecB);
//     dot = Dot(normal, toOrigin);
//     if(dot > -cDiscoveryEpsilon)
//     {
//       //Support point v3 must be changed
//       mDirection = normal;
//       if(!Support(2))
//       {
//         return false; //Invalid direction, bad situation, abort!
//       }
// 
//       //Points v1 and v2 must be swapped
//       Swap(0, 1);
// 
//       //Still not sure if origin is in portal...
//       originRayNotInsidePortal = true;
//     }
//     else
//     {
//       //Everything is ok, continue testing
//       originRayNotInsidePortal = false;
//     }
    ++i;
  }

  if(i == cDiscoveryIterations)
  {
    return false;
  }
  return true;
}

///Move the portal closer to the surface of the CSO, attempting to increase the
///accuracy of the closest features of the two shapes
bool Mpr::Refine(bool toSurface)
{
  Vec3 toOrigin = -mCsoCenter;
  Vec3 u, w, n;

  for(uint i = 0; i < cRefinementIterations; ++i)
  {
    //Direction becomes face normal of portal, away from origin
    mDirection = Geometry::GenerateNormal(mCsoSupport[0], mCsoSupport[1], 
                                          mCsoSupport[2]);

    //Usable face is the portal face
    if(!toSurface && OriginInsideTetrahedron())
    {
      return true;
    }

    //Find support in direction of portal
    Support(3);

    if(OriginOutsideTetrahedron())
    {
      return false;
    }

    if(PortalCloseToSurface())
    {
      return true;
    }
    
    //From center to new support point
    n = mCsoSupport[3] + toOrigin;

    /*
                   v2
                   ^
                  /|\
                 / | \
                /  |  \
               /   |   \
              /    O vN \
             /   /   \   \
            /  /       \  \
           / /           \ \
        v3 ----------------- v1
    */

    Vec3 planeNormals[3] = { mCsoSupport[0] + toOrigin,
                             mCsoSupport[1] + toOrigin,
                             mCsoSupport[2] + toOrigin };
    planeNormals[0] = Cross(planeNormals[0], n);
    planeNormals[1] = Cross(planeNormals[1], n);
    planeNormals[2] = Cross(planeNormals[2], n);

    real planeDistances[3] = { Dot(planeNormals[0], toOrigin),
                               Dot(planeNormals[1], toOrigin),
                               Dot(planeNormals[2], toOrigin) };

    //Choose new portal!

    //Check in the positive region of v2 x vN and the negative region of
    //v1 x vN for the origin
    if(planeDistances[1] > cRefinementEpsilon && 
       planeDistances[0] < cRefinementEpsilon)
    {
      //It's in the region defined by v1, v2, and vN
      Assign(3, 2);
      continue;
    }
    
    //Check in the positive region of v1 x vN and the negative region of
    //v3 x vN for the origin
    if(planeDistances[0] > cRefinementEpsilon && 
       planeDistances[2] < cRefinementEpsilon)
    {
      //It's in the region defined by v3, v1, and vN
      Assign(3, 1);
      continue;
    }

    //Check in the positive region of v3 x vN and the negative region of
    //v2 x vN for the origin
    if(planeDistances[2] > cRefinementEpsilon && 
       planeDistances[1] < cRefinementEpsilon)
    {
      //It's in the region defined by v2, v3, and vN
      Assign(3, 0);
    }
  }

  if(!toSurface && OriginInsideTetrahedron())
  {
    return true;
  }

  return false;
}

///Initialize the algorithm before it is run
void Mpr::Init(const SupportShape* shapeA, const SupportShape* shapeB, 
               AlgorithmType algorithmType)
{
  ErrorIf(shapeA == nullptr, "Physics::Mpr - Invalid shape pointer passed to the "\
                          "MPR collision detection algorithm.");
  ErrorIf(shapeB == nullptr, "Physics::Mpr - Invalid shape pointer passed to the "\
                          "MPR collision detection algorithm.");
  mShapes[cLeftShape] = shapeA;
  mShapes[cRightShape] = shapeB;

  mAlgorithmType = algorithmType;
  mTranslation.ZeroOut();
  mLeftOffset.ZeroOut();
  mTranslationSupport.ZeroOut();
}

///Swap the two support points at the given indexes
void Mpr::Swap(uint a, uint b)
{
  Vec3 temp = mCsoSupport[a];
  mCsoSupport[a] = mCsoSupport[b];
  mCsoSupport[b] = temp;

  temp = mLeftSupport[a];
  mLeftSupport[a] = mLeftSupport[b];
  mLeftSupport[b] = temp;

  temp = mRightSupport[a];
  mRightSupport[a] = mRightSupport[b];
  mRightSupport[b] = temp;
}

///Assign the point at the source index to the point at the destination index
void Mpr::Assign(uint source, uint destination)
{
  mCsoSupport[destination] = mCsoSupport[source];
  mLeftSupport[destination] = mLeftSupport[source];
  mRightSupport[destination] = mRightSupport[source];
}

///Determine if origin is inside the tetrahedron built from support points
bool Mpr::OriginInsideTetrahedron(void)
{
  //If the result is positive, the origin is inside of the face
  real dot = Dot(mDirection, mCsoSupport[0]);
  return dot > -cContainmentEpsilon;
}

///Determine if origin is outside the tetrahedron built from support points
bool Mpr::OriginOutsideTetrahedron(void)
{
  //If the result is positive, the origin is outside tetrahedron but inside
  //the CSO
  real dot = Dot(mDirection, mCsoSupport[3]);
  return dot < cContainmentEpsilon;
}

///Determine if the origin is close to the portal's surface
bool Mpr::PortalCloseToSurface(void)
{
  real dot = Dot(mDirection, mCsoSupport[3]) - Dot(mDirection, mCsoSupport[0]);
  return dot < real(0.01);
}

///Find the point furthest in the direction of either the origin or the 
///translation vector's endpoint
void Mpr::TranslationSupport(void)
{
  real dotTranslation = Dot(mTranslation, mDirection);
  if(Math::IsNegative(dotTranslation))
  {
    mTranslationSupport.ZeroOut();
  }
  else
  {
    mTranslationSupport = mTranslation;
  }
}

///Fill the manifold with all of the information about the collision
void Mpr::FillManifold(Intersection::Manifold& manifold)
{
  DrawDebug(uint(-1));


  //Last search direction is the normal
  real depth = Dot(mDirection, mCsoSupport[0]);
  manifold.PointAt(0).Depth = depth;

  Normalize(mDirection);
  manifold.Normal = mDirection;

  //Get the point on the portal's face
  Vec3 point = mDirection * depth;

  //Calculate the barycentric coordinates of the origin projected onto the
  //portal's face
  Geometry::BarycentricTriangle(point, mCsoSupport[0], mCsoSupport[1], 
                                mCsoSupport[2], &point);

  Vec3 contactPoint = mLeftSupport[0] * point.x;
       contactPoint = Math::MultiplyAdd(contactPoint, mLeftSupport[1], point.y);
       contactPoint = Math::MultiplyAdd(contactPoint, mLeftSupport[2], point.z);
  manifold.PointAt(0).Points[cLeftShape] = contactPoint;

  contactPoint = mRightSupport[0] * point.x;
  contactPoint = Math::MultiplyAdd(contactPoint, mRightSupport[1], point.y);
  contactPoint = Math::MultiplyAdd(contactPoint, mRightSupport[2], point.z);
  manifold.PointAt(0).Points[cRightShape] = contactPoint;
  manifold.PointCount = 1;
}

bool Mpr::PortalFaceCheck(uint pointA, uint pointB, uint offPoint, 
                          bool& originRayNotInsidePortal)
{
  Vec3 vecA = mCsoSupport[pointA] - mCsoCenter;
  Vec3 vecB = mCsoSupport[pointB] - mCsoCenter;
  Vec3 normal = Cross(vecA, vecB);
  real dot = -Dot(normal, mCsoCenter);
  if(dot > -cDiscoveryEpsilon)
  {
    mDirection = normal;
    if(!Support(offPoint))
    {
      return false;
    }

    Swap(pointA, pointB);

    originRayNotInsidePortal = true;
  }
  else
  {
    originRayNotInsidePortal = false;
  }
  return true;
}

void Mpr::DrawDebug(uint debugFlag) const
{
//   //whether or not to draw everything on top
//  bool onTop = (debugFlag & Zero::Physics::DebugFlags::DrawOnTop) == 1;
// 
//   //draw the contact points if the flag is set.
//  if((debugFlag & Zero::Physics::DebugFlags::DrawMpr) == 0)
//   {
//     return;
//   }
// 
// #define DrawSphere(p, r, c, d)                             \
//   Zero::gDebugDraw->Add(Zero::Debug::Sphere(p, r).Color(c) \
//                                               .OnTop(true) \
//                                               .Duration(d))
// 
//   real radius = real(0.1);
//   real duration = real(0.1);
//   DrawSphere(mCenter[0], radius, Color::Pink, duration);
//   DrawSphere(mCenter[1], radius, Color::Pink, duration);
//   DrawSphere(mCsoCenter, radius, Color::Pink, duration);
// 
//   DrawSphere(mCsoSupport[0], radius, Color::Magenta, duration);
//   DrawSphere(mCsoSupport[1], radius, Color::Cyan, duration);
//   DrawSphere(mCsoSupport[2], radius, Color::Yellow, duration);
// 
//   DrawSphere(mLeftSupport[0], radius, Color::Magenta, duration);
//   DrawSphere(mLeftSupport[1], radius, Color::Cyan, duration);
//   DrawSphere(mLeftSupport[2], radius, Color::Yellow, duration);
// 
//   DrawSphere(mRightSupport[0], radius, Color::Magenta, duration);
//   DrawSphere(mRightSupport[1], radius, Color::Cyan, duration);
//   DrawSphere(mRightSupport[2], radius, Color::Yellow, duration);
// 
// #undef DrawSphere
// 
// 
// #define DrawLine(a, b, color)\
//   Zero::gDebugDraw->Add(Zero::Debug::Line((a), (b)).Color((color)).OnTop(true));
// 
//   DrawLine(mCsoCenter, mCsoSupport[0], Color::White);
//   DrawLine(mCsoCenter, mCsoSupport[1], Color::White);
//   DrawLine(mCsoCenter, mCsoSupport[2], Color::White);
// 
// #define DrawTriangle(a, b, c, color)\
//   DrawLine((a), (b), color);        \
//   DrawLine((b), (c), color);        \
//   DrawLine((c), (a), color);
// 
//   DrawTriangle(mCsoSupport[0], mCsoSupport[1], mCsoSupport[2], Color::Blue);
//   DrawTriangle(mLeftSupport[0], mLeftSupport[1], mLeftSupport[2], Color::Green);
//   DrawTriangle(mRightSupport[0], mRightSupport[1], mRightSupport[2], Color::Pink);
// #undef DrawTriangle
// #undef DrawLine
}

}//namespace Intersection
