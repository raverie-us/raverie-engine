///////////////////////////////////////////////////////////////////////////////
///
/// \file Gjk.cpp
/// GJK algorithm for detecting collision between to convex shapes.
///
/// Authors: Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Intersection
{

const float Gjk::sEpsilon = 0.001f;

Type Gjk::Test(const SupportShape *shapeA, const SupportShape *shapeB, Manifold *manifold, unsigned maxIter)
{
  // Get initial support vector
  Initialize(shapeA, shapeB);

  // If shape centers are on top of each other, default to any direction
  // Any direction is valid unless shape centers are on the surface...
  if (mSupportVector.Length() == 0.0f)
    mSupportVector.Set(1, 0, 0);

  unsigned iter_count = 0;
  while(iter_count < maxIter)
  {
    // Find support point on Minkowski Difference
    CSOVertex support = ComputeSupport(mSupportVector);

    // Check if point is valid
    float proj = mSupportVector.Dot(support.cso);
    if (proj <= 0.0f)
      return Intersection::None;

    // Add point to simplex
    mSimplex.AddPoint(support);

    // Find geometry on simplex that is closest to origin
    mSimplex.Update();

    if (mSimplex.ContainsOrigin())
    {
      ComputeContactData(manifold);
      return Intersection::Other;
    }

    // Get new support vector
    mSupportVector = mSimplex.GetSupportVector();
    ++iter_count;
  }

  return Intersection::None;
}

Type Gjk::TestDebug(const SupportShape *shapeA, const SupportShape *shapeB, Manifold *manifold, unsigned maxIter)
{
  // Get initial support vector
  Initialize(shapeA, shapeB);

  // If shape centers are on top of each other, default to any direction
  // Any direction is valid unless shape centers are on the surface...
  if (mSupportVector.Length() == 0.0f)
    mSupportVector.Set(1, 0, 0);

  unsigned iter_count = 0;
  while(iter_count < maxIter)
  {
    // Find support point on Minkowski Difference
    CSOVertex support = ComputeSupport(mSupportVector);

    // Check if point is valid
    if (mSimplex.mCount)
    {
      // Instead of early out,
      // Only terminate when we've reached the geometry nearest the origin
      float proj = mSupportVector.Dot(support.cso - mSimplex.mPoints[0].cso);
      if (proj <= sEpsilon)
      {
        // Zero::gDebugDraw->Add(Zero::Debug::Sphere(support.cso, 0.02).Color(Color::Black));
        return Intersection::None;
      }
    }

    // Add point to simplex
    mSimplex.AddPoint(support);

    // Find geometry on simplex that is closest to origin
    mSimplex.Update();

    if (mSimplex.ContainsOrigin())
    {
      // Only point is the origin, so objects have intersection of zero
      if (mSimplex.mCount == 1)
        return Intersection::None;

      ComputeContactData(manifold);
      if (manifold->Points[0].Depth > 0.0f)
        return Intersection::Other;
      else
        return Intersection::None;
    }

    // Get new support vector
    mSupportVector = mSimplex.GetSupportVector();
    ++iter_count;
  }

  return Intersection::None;
}

void Gjk::DrawDebug(uint debugFlag)
{
  DrawCSO();
  // mEpa.DrawDebug();

  ByteColor colors[] =
  {
    Color::Red,
    Color::Green,
    Color::Blue,
    Color::White,
  };

  for (uint i = 0; i < mSimplex.mCount; ++i)
  {
    float radius = 0.05f - 0.01f * i;
    Zero::gDebugDraw->Add(Zero::Debug::Sphere(mSimplex.mPoints[i].cso, radius).Color(colors[i]));
    Zero::gDebugDraw->Add(Zero::Debug::Sphere(mSimplex.mPoints[i].objA, radius).Color(colors[i]));
    Zero::gDebugDraw->Add(Zero::Debug::Sphere(mSimplex.mPoints[i].objB, radius).Color(colors[i]));
  }

  Zero::gDebugDraw->Add(Zero::Debug::Line(Vec3(0, 0, 0), mSupportVector).Color(Color::White));

  // mSupportVector = mSimplex.GetSupportVector();

  // for (unsigned i = 0; i < mCSO.Size(); ++i)
  //   Zero::gDebugDraw->Add(Zero::Debug::Sphere(mCSO[i], 0.01f).Color(Color::PapayaWhip));

  // for (unsigned i = 0; i < mSimplex.mCount; ++i)
  // {
  //   Zero::gDebugDraw->Add(Zero::Debug::Sphere(mSimplex.mPoints[i].cso, 0.01f).Color(Color::Blue));
  //   Zero::gDebugDraw->Add(Zero::Debug::Line(mSimplex.mPoints[i].cso, mSimplex.mPoints[(i + 1) % mSimplex.mCount].cso).Color(Color::Blue));
  //   if (mSimplex.mCount == 4)
  //   {
  //     Zero::gDebugDraw->Add(Zero::Debug::Line(mSimplex.mPoints[i].cso, mSimplex.mPoints[(i + 2) % mSimplex.mCount].cso).Color(Color::Blue));
  //     Zero::gDebugDraw->Add(Zero::Debug::Line(mSimplex.mPoints[i].cso, mSimplex.mPoints[(i + 3) % mSimplex.mCount].cso).Color(Color::Blue));
  //   }
  // }

  // Vec3 average = Vec3::cZero;
  // for (unsigned i = 0; i < mSimplex.mCount; ++i)
  //   average += mSimplex.mPoints[i].cso;
  // if (mSimplex.mCount)
  //   average /= (float)mSimplex.mCount;
  // Zero::gDebugDraw->Add(Zero::Debug::Line(average, average + mSupportVector).Color(Color::Red));
}

Simplex Gjk::GetSimplex(void)
{
  return mSimplex;
}

void Gjk::Initialize(const SupportShape *shapeA, const SupportShape *shapeB)
{
  mShapeA = shapeA;
  mShapeB = shapeB;

  Vec3 centerA, centerB;
  mShapeA->GetCenter(&centerA);
  mShapeB->GetCenter(&centerB);

  mSupportVector = centerB - centerA;
  mSimplex.Clear();
}

CSOVertex Gjk::ComputeSupport(Vec3 supportVector)
{
  Vec3 supportA, supportB;
  mShapeA->Support(supportVector, &supportA);
  mShapeB->Support(-supportVector, &supportB);

  return CSOVertex(supportA, supportB, supportA - supportB);
}

void Gjk::DrawTriangle(Vec3 p0, Vec3 p1, Vec3 p2)
{
  Zero::gDebugDraw->Add(Zero::Debug::Triangle(p0, p1, p2).Color(Color::Orange).Border(true).Alpha(50));
}

void Gjk::DrawCSO(void)
{
  Zero::Array<CSOVertex> supports;

  unsigned subdivisions = 24;
  float alphaLimit = Math::cPi - sEpsilon;
  float betaLimit = Math::cPi * 2 - sEpsilon;

  float delta = Math::cPi / subdivisions;
  for (float alpha = delta; alpha < alphaLimit; alpha += delta)
  {
    float alpha2 = (alpha + delta < alphaLimit) ? alpha + delta : delta;
    float sinAlpha1 = Math::Sin(alpha);
    float CosAlpha1 = Math::Cos(alpha);
    float sinAlpha2 = Math::Sin(alpha2);
    float CosAlpha2 = Math::Cos(alpha2);

    for (float beta = 0; beta < betaLimit; beta += delta)
    {
      float beta2 = (beta + delta < betaLimit) ? beta + delta : 0;
      float sinBeta1 = Math::Sin(beta);
      float CosBeta1 = Math::Cos(beta);
      float sinBeta2 = Math::Sin(beta2);
      float CosBeta2 = Math::Cos(beta2);

      Vec3 v0(sinAlpha1 * sinBeta1, CosAlpha1, sinAlpha1 * CosBeta1);
      Vec3 v1(sinAlpha2 * sinBeta1, CosAlpha2, sinAlpha2 * CosBeta1);
      Vec3 v2(sinAlpha2 * sinBeta2, CosAlpha2, sinAlpha2 * CosBeta2);
      Vec3 v3(sinAlpha1 * sinBeta2, CosAlpha1, sinAlpha1 * CosBeta2);
      CSOVertex p0 = ComputeSupport(v0);
      CSOVertex p1 = ComputeSupport(v1);
      CSOVertex p2 = ComputeSupport(v2);
      CSOVertex p3 = ComputeSupport(v3);

      supports.PushBack(p0);

      if (alpha2 > delta)
      {
        DrawTriangle(p0.cso, p2.cso, p1.cso);
        DrawTriangle(p0.cso, p3.cso, p2.cso);
      }
    }
  }

  CSOVertex top = ComputeSupport(Vec3(0, 1, 0));
  CSOVertex bottom = ComputeSupport(Vec3(0, -1, 0));

  unsigned collumns = subdivisions * 2;
  for (unsigned i = 0; i < collumns; ++i)
  {
    unsigned i2 = (i + 1) % (collumns);
    CSOVertex p0 = supports[i];
    CSOVertex p1 = supports[i2];

    unsigned j = supports.Size() - (collumns - i);
    unsigned j2 = supports.Size() - (collumns - i2);
    CSOVertex p2 = supports[j];
    CSOVertex p3 = supports[j2];

    DrawTriangle(top.cso, p1.cso, p0.cso);
    DrawTriangle(p2.cso, p3.cso, bottom.cso);
  }

  supports.PushBack(top);
  supports.PushBack(bottom);

  // for (unsigned i = 0; i < supports.Size(); ++i)
  //   Zero::gDebugDraw->Add(Zero::Debug::Sphere(supports[i].cso, 0.01f).Color(Color::Green));
}

bool Gjk::ComputeContactData(Manifold *manifold, unsigned maxExpands, bool debug)
{
  if (!manifold) return false;
  manifold->PointCount = 0;

  // Handle non tetrahedron
  CompleteSimplex();

  mEpa.Init(mSimplex);

  unsigned count = 0;
  // if (debug)
  // {
  //   bool newPoint = true;
  //   while (count < maxExpands)
  //   {
  //     if (newPoint)
  //     {
  //       mSupportVector = mEpa.GetClosestFaceNormal();
  //       CSOVertex support = ComputeSupport(mSupportVector);
  //       mEpa.DebugPoint(support);
  //       newPoint = false;
  //     }
  //     else
  //     {
  //       if (!mEpa.DebugStep())
  //         newPoint = true;
  //     }

  //     ++count;
  //   }
  //   if (count == maxExpands) return false;
  // }
  // else
  // {
    // Expand until closest face is found
    while (count < maxExpands)
    {
      mSupportVector = mEpa.GetClosestFaceNormal();
      CSOVertex support = ComputeSupport(mSupportVector);
      if (!mEpa.Expand(support))
        break;
      ++count;
    }

    // If we hit max expands, we need to find the new closest face before computing contact data
    if (count == maxExpands)
      mSupportVector = mEpa.GetClosestFaceNormal();
  // }

  // DebugPrint("%d\n", count);

  manifold->PointCount = 1;
  manifold->Normal = mSupportVector;
  manifold->Points[0].Depth = -mEpa.GetClosestDistance();

  Vec3 csoContact = manifold->Normal * manifold->Points[0].Depth;

  CSOVertex face[3];
  mEpa.GetClosestFace(face);

  Vec3 weights;
  Geometry::BarycentricTriangle(csoContact, face[0].cso, face[1].cso, face[2].cso, &weights);
  manifold->Points[0].Points[0] = face[0].objA * weights.x + face[1].objA * weights.y + face[2].objA * weights.z;
  manifold->Points[0].Points[1] = face[0].objB * weights.x + face[1].objB * weights.y + face[2].objB * weights.z;

  return true;
}

void Gjk::CompleteSimplex(void)
{
  // The only way gjk could terminate with a single point simplex
  // is if the first point was the origin,
  // which would fail the projection test and not make it here.
  switch (mSimplex.mCount)
  {
    // Manually expanding the simplex to a tetrahedron if it wasn't already
    // Fall through is intentional to build simplex piece by piece
    case 2:
    {
      // Build six coplanar vectors, all perpendicular to the line
      // It is possible for up to two of these permutations to be degenerate,
      // that case is handled in the loop
      Vec3 lineDir = mSimplex.mPoints[1].cso - mSimplex.mPoints[0].cso;
      Vec3 perpVectors[6] =
      {
        Vec3(lineDir.y, -lineDir.x, 0),
        Vec3(lineDir.z, 0, -lineDir.x),
        Vec3(0, lineDir.z, -lineDir.y),
        Vec3(-lineDir.y, lineDir.x, 0),
        Vec3(-lineDir.z, 0, lineDir.x),
        Vec3(0, -lineDir.z, lineDir.y)
      };

      // No way to intelligently expand a line segment when the origin is on it,
      // so expand until we find a valid support point that can create a triangle
      for (unsigned i = 0; i < 6; ++i)
      {
        if (perpVectors[i].Length() == 0.0f)
          continue;

        perpVectors[i].Normalize();
        CSOVertex support = ComputeSupport(perpVectors[i]);
        if (perpVectors[i].Dot(support.cso - mSimplex.mPoints[0].cso) > 0.0f)
        {
          mSimplex.AddPoint(support);
          break;
        }
      }

      ErrorIf(mSimplex.mCount < 3, "Failed to complete simplex, possibly a degenerate CSO.");
    }

    case 3:
    {
      // Only two directions for trying to expand a triangle, front or back face normals
      Vec3 p0p1 = mSimplex.mPoints[1].cso - mSimplex.mPoints[0].cso;
      Vec3 p0p2 = mSimplex.mPoints[2].cso - mSimplex.mPoints[0].cso;
      Vec3 normal = p0p1.Cross(p0p2);
      normal.Normalize();

      CSOVertex support = ComputeSupport(normal);
      // It is possible that the triangle is on the surface of the cso and the normal points outward
      // if this happens, just need to expand the opposite direction and flip two vertices
      // to maintain the expected winding order in the simplex
      if (normal.Dot(support.cso - mSimplex.mPoints[0].cso) <= 0.0f)
      {
        normal *= -1.0f;
        support = ComputeSupport(normal);
        ErrorIf(normal.Dot(support.cso - mSimplex.mPoints[0].cso) <= 0.0f, "Failed to complete simplex, possibly a degenerate CSO.");

        CSOVertex temp = mSimplex.mPoints[1];
        mSimplex.mPoints[1] = mSimplex.mPoints[2];
        mSimplex.mPoints[2] = temp;
      }
      mSimplex.AddPoint(support);
    }
  }
}

}// namespace Intersection
