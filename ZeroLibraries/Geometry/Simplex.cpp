///////////////////////////////////////////////////////////////////////////////
///
/// \file Simplex.cpp
/// .
///
/// Authors: Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Intersection
{

Simplex::Simplex(void)
{
  Clear();
}

void Simplex::Clear(void)
{
  mCount = 0;
  mContainsOrigin = false;
}

bool Simplex::ContainsOrigin(void)
{
  return mContainsOrigin;
}

Vec3 Simplex::GetSupportVector(void)
{
  return mSupportVector;
}

void Simplex::AddPoint(const CSOVertex &point)
{
  for (uint i = 0; i < mCount; ++i)
  {
    if (point.cso == mPoints[i].cso)
      return;
  }

  mPoints[mCount] = point;
  ++mCount;
}

void Simplex::Update(void)
{
  switch (mCount)
  {
    // Closest geometry on a point simplex
    // is the simplex itself
    case 2: HandleLineSegment(); break;
    case 3: HandleTriangle();    break;
    case 4: HandleTetrahedron(); break;
  }

  ComputeSupportVector();
}

Zero::Array<CSOVertex> Simplex::GetPoints(void)
{
  Zero::Array<CSOVertex> points;
  for (unsigned i = 0; i < mCount; ++i)
    points.PushBack(mPoints[i]);
  return points;
}

void Simplex::HandleLineSegment()
{
  if ((mPoints[1].cso - mPoints[0].cso).Dot(-mPoints[1].cso) > 0.0f)
  {
    // Closest geometry is point p1
    mPoints[0] = mPoints[1];
    --mCount;
  }

  // Closest geometry is line segment p0p1
}

void Simplex::HandleTriangle(void)
{
  Vec3 p0p2 = mPoints[2].cso - mPoints[0].cso;
  Vec3 p1p2 = mPoints[2].cso - mPoints[1].cso;
  Vec3 normal = p0p2.Cross(p1p2);

  Vec3 dir1 = normal.Cross(p0p2);
  Vec3 dir2 = p1p2.Cross(normal);

  bool inside1 = dir1.Dot(-mPoints[0].cso) <= 0.0f;
  bool inside2 = dir2.Dot(-mPoints[1].cso) <= 0.0f;

  // Closest geometry is triangle p0p1p2
  if (inside1 && inside2)
    return;

  if (!inside1 && p0p2.Dot(-mPoints[2].cso) <= 0.0f)
  {
    // Closest geometry is line segment p0p1
    mPoints[1] = mPoints[2];
    --mCount;
  }
  else if (!inside2 && p1p2.Dot(-mPoints[2].cso) <= 0.0f)
  {
    // Closest geometry is line segment p1p2
    mPoints[0] = mPoints[2];
    --mCount;
  }
  else
  {
    // Closest geometry is point p2
    mPoints[0] = mPoints[2];
    mCount -= 2;
  }
}

void Simplex::HandleTetrahedron(void)
{
  bool edge0 = true;
  bool edge1 = true;
  bool edge2 = true;

  if (TestTriangle(0, 1, 3, edge0, edge1))
  {
    // Closest geometry is triangle p0p1p3
    mPoints[2] = mPoints[3];
    --mCount;
    return;
  }

  if (TestTriangle(1, 2, 3, edge1, edge2))
  {
    // Closest geometry is triangle p1p2p3
    mPoints[0] = mPoints[3];
    --mCount;
    return;
  }

  if (TestTriangle(2, 0, 3, edge2, edge0))
  {
    // Closest geometry is triangle p2p0p3
    mPoints[1] = mPoints[3];
    --mCount;
    return;
  }

  if (edge0 && edge1 && edge2)
  {
    // Closest geometry is point p3
    mPoints[0] = mPoints[3];
    mCount -= 3;
  }
  else if (edge0)
  {
    // Closest geometry is line segment p0p3
    mPoints[1] = mPoints[3];
    mCount -= 2;
  }
  else if (edge1)
  {
    // Closest geometry is line segment p1p3
    mPoints[0] = mPoints[3];
    mCount -= 2;
  }
  else if (edge2)
  {
    // Closest geometry is line segment p2p3
    mPoints[0] = mPoints[2];
    mPoints[1] = mPoints[3];
    mCount -= 2;
  }

  // Closest geometry is tetrahedron
}

bool Simplex::TestTriangle(uint i0, uint i1, uint i2, bool &leftEdge, bool &rightEdge)
{
  Vec3 p0 = mPoints[i0].cso;
  Vec3 p1 = mPoints[i1].cso;
  Vec3 p2 = mPoints[i2].cso;

  Vec3 N = (p1 - p0).Cross(p2 - p0);

  Vec3 e1 = (p2 - p1).Cross(N);
  Vec3 e2 = (p0 - p2).Cross(N);

  bool leftInside = e2.Dot(-p2) <= 0.0f;
  bool rightInside = e1.Dot(-p2) <= 0.0f;

  // Set possible edges
  leftEdge = leftEdge && !leftInside;
  rightEdge = rightEdge && !rightInside;

  // Origin behind triangle
  if (N.Dot(-p0) <= 0.0f)
    return false;

  // Triangle is closest geometry
  if (leftInside && rightInside)
    return true;

  return false;
}

void Simplex::ComputeSupportVector(void)
{
  // Support is in the opposite direction of point
  if (mCount == 1)
  {
    mSupportVector = -mPoints[0].cso;
  }
  // Support is perpendicular to the line segment
  else if (mCount == 2)
  {
    Vec3 lineDir = mPoints[1].cso - mPoints[0].cso;
    Vec3 originDir = -mPoints[0].cso;
    Vec3 perpendicular = lineDir.Cross(originDir);
    mSupportVector = perpendicular.Cross(lineDir);
  }
  // Support is normal to the triangle
  else if (mCount == 3)
  {
    Vec3 p0p1 = mPoints[1].cso - mPoints[0].cso;
    Vec3 p0p2 = mPoints[2].cso - mPoints[0].cso;
    Vec3 normal = p0p1.Cross(p0p2);
    normal.Normalize();

    real distance = normal.Dot(-mPoints[0].cso);
    mSupportVector = normal * distance;

    // Correct triangle winding order when origin is on the wrong side
    if (distance < 0.0f)
    {
      mPoints[3] = mPoints[1];
      mPoints[1] = mPoints[2];
      mPoints[2] = mPoints[3];
    }
  }
  else if (mCount == 4)
  {
    mSupportVector.ZeroOut();
  }

  // If the result is the zero vector, then the origin is contained in our simplex
  if (mSupportVector.Length() == 0.0f)
    mContainsOrigin = true;
  else
    mSupportVector.Normalize();
}

}// namespace Intersection
