///////////////////////////////////////////////////////////////////////////////
///
/// \file Gjk.hpp
/// Interface for the GJK collision detection algorithm.
///
/// Authors: Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Intersection
{
class SupportShape;

//-------------------------------------------------------------------------- Gjk
class Gjk
{
public:

  static const float sEpsilon;

  Type Test(const SupportShape* shapeA, const SupportShape* shapeB, Manifold* manifold = nullptr, unsigned maxIter = 20);
  Type TestDebug(const SupportShape* shapeA, const SupportShape* shapeB, Manifold* manifold = nullptr, unsigned maxIter = 20);

  void DrawDebug(uint debugFlag);

  Simplex GetSimplex(void);

  Vec3 mSupportVector;
private:

  void DrawTriangle(Vec3 p0, Vec3 p1, Vec3 p2);
  void DrawCSO(void);
  void Initialize(const SupportShape *shapeA, const SupportShape *shapeB);
  CSOVertex ComputeSupport(Vec3 supportVector);
  void ComputeCSO(void);
  bool ComputeContactData(Manifold *manifold, unsigned maxExpands = 20, bool debug = false);
  void CompleteSimplex(void);

  const SupportShape *mShapeA;
  const SupportShape *mShapeB;
  Simplex mSimplex;
  Epa mEpa;

  Zero::Array<Vec3> mCSO;
};

}// namespace Intersection
