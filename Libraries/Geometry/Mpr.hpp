///////////////////////////////////////////////////////////////////////////////
///
/// \file Mpr.hpp
/// Interface for the MPR collision detection algorithm.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Intersection
{

//-------------------------------------------------------------------------- Mpr
class Mpr
{
public:
  Intersection::Type Test(const SupportShape* shapeA, 
                          const SupportShape* shapeB, 
                          Intersection::Manifold* manifold = nullptr);

  ///Check to see if (and when) two moving shapes are intersecting
  Intersection::Type SweptTest(const SupportShape* shapeA, 
                               const SupportShape* shapeB,
                               Intersection::Manifold* manifold = nullptr);

  void DrawDebug(uint debugFlag) const;

private:
  enum AlgorithmType
  {
    Discrete,
    Swept
  };

  ///Calculate the CSO's center bases off of the centers of the two shapes
  void CalculateCenter(void);

  ///Calculate the relative motion of the two shapes
  void CalculateTranslation(void);

  ///Get the point furthest in the stored direction on the CSO and store the 
  ///results in the specified index. Returns false if any error occurs due to
  ///invalid data.
  bool Support(uint index);

  ///Compute the initial portal to start the algorithm
  bool InitialPortal(void);

  ///Iterate over the CSO hull to find a portal which Contains the origin ray
  bool Discover(void);

  ///Move the portal closer to the surface of the CSO, attempting to increase 
  ///the accuracy of the closest features of the two shapes
  bool Refine(bool onSurface);

  ///Initialize the algorithm before it is run
  void Init(const SupportShape* shapeA, const SupportShape* shapeB, 
            AlgorithmType algorithmType);

  ///Swap the two support points at the given indexes
  void Swap(uint a, uint b);

  ///Assign the point at the source index to the point at the destination index
  void Assign(uint source, uint destination);

  ///Determine if origin is inside the tetrahedron built from support points
  bool OriginInsideTetrahedron(void);

  ///Determine if origin is outside the tetrahedron built from support points
  bool OriginOutsideTetrahedron(void);

  ///Determine if the origin is close to the portal's surface
  bool PortalCloseToSurface(void);

  ///Find the point furthest in the direction of either the origin or the 
  ///translation vector's endpoint
  void TranslationSupport(void);

  ///Fill the manifold with all of the information about the collision
  void FillManifold(Intersection::Manifold& manifold);

  bool PortalFaceCheck(uint pointA, uint pointB, uint offPoint, 
                       bool& originRayNotInsidePortal);

  Vec3 mCenter[2];
  Vec3 mCsoCenter;
  Vec3 mDirection;
  Vec3 mTranslation;
  Vec3 mTranslationSupport;
  Vec3 mLeftOffset;
  Vec3 mLeftSupport[4];
  Vec3 mRightSupport[4];
  Vec3 mCsoSupport[4];
  AlgorithmType mAlgorithmType;
  const SupportShape* mShapes[2];
};
}// namespace Intersection
