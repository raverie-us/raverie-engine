///////////////////////////////////////////////////////////////////////////////
///
/// \file Simplex.hpp
/// .
///
/// Authors: Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Intersection
{

class Gjk;
class Epa;

struct CSOVertex
{
  CSOVertex(void) {}
  CSOVertex(Vec3 a, Vec3 b, Vec3 c) : objA(a), objB(b), cso(c) {}
  Vec3 objA, objB, cso;
};

//--------------------------------------------------------------------- Simplex
class Simplex
{
public:

  friend class Gjk;
  friend class Epa;

  Simplex(void);

  void Clear(void);
  bool ContainsOrigin(void);
  Vec3 GetSupportVector(void);
  void AddPoint(const CSOVertex &point);
  void Update(void);

  Zero::Array<CSOVertex> GetPoints(void);

private:

  void HandleLineSegment(void);
  void HandleTriangle(void);
  void HandleTetrahedron(void);
  bool TestTriangle(uint i0, uint i1, uint i2, bool &leftEdge, bool &rightEdge);
  void ComputeSupportVector(void);

  Vec3 mSupportVector;
  CSOVertex mPoints[4];
  unsigned mCount;
  bool mContainsOrigin;
};

}// namespace Intersection
