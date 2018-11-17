///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

real GetAnimationTime(Cog* cog)
{
  real t = 0;
  TimeSpace* time = cog->GetSpace()->has(TimeSpace);
  if(time != nullptr)
    t = time->mRealTimePassed;
  return t;
}

void DebugDrawDisc(Vec3Param center, Vec3Param axis0, Vec3Param axis1, real radius0, real radius1)
{
  // Draw 128 line segments to represent the ellipse
  real delta = Math::cTwoPi / real(128);
  real prev = real(0);
  for(; prev <= Math::cTwoPi; prev += delta)
  {
    // Could cache the last value and not re-compute it, but this is debug drawing so I don't care right now...
    real next = prev + delta;
    Vec3 p0 = center + Math::Cos(prev) * axis0 * radius0 + Math::Sin(prev) * axis1 * radius1;
    Vec3 p1 = center + Math::Cos(next) * axis0 * radius0 + Math::Sin(next) * axis1 * radius1;
    gDebugDraw->Add(Debug::Line(p0, p1).Color(Color::Plum));
  }
}

void DrawRing(Vec3Param center, Vec3Param axis, real radius, size_t subDivisions, real time, ByteColor color)
{
  // Compute a basis for the ring
  Vec3 axis0, axis1;
  Math::GenerateOrthonormalBasis(axis, &axis0, &axis1);

  // Compute the arc-length of each line segment
  real globalDetail = Math::cTwoPi / 128;
  // Compute the total arc-length of each sub-division
  real subArcAngle = Math::cTwoPi / subDivisions;
  // Take off an arbitrary 5 degrees so there's room for the arrow-head
  subArcAngle -= Math::DegToRad(5);

  for(size_t i = 0; i <= subDivisions; ++i)
  {
    // Compute the starting angle of this sub-division
    real start = time + (i * Math::cTwoPi / subDivisions);
    real prev = start;
    for(real deltaAngle = 0; deltaAngle <= subArcAngle; deltaAngle += globalDetail)
    {
      real next = start + deltaAngle;

      Vec3 p0 = center + radius * (Math::Cos(prev) * axis0 + Math::Sin(prev) * axis1);
      Vec3 p1 = center + radius * (Math::Cos(next) * axis0 + Math::Sin(next) * axis1);

      // Draw a line with the given color
      Debug::Line line(p0, p1);
      line.Color(color);
      // If this is the last segment then add an arrow head
      if(deltaAngle + globalDetail > subArcAngle)
        line.HeadSize(0.1f);
      gDebugDraw->Add(line);

      prev = next;
    }
  }
}

void GetPenumbraDebugDrawValues(real minDistance, real maxDistance, real& minForce, real& maxForce)
{
  // Get a signed normalized force for both the min and max distances
  minForce /= Math::Abs(minForce);
  maxForce /= Math::Abs(maxForce);
  real deltaDistance = maxDistance - minDistance;
  // Arbitrary scaling on the vectors. Draw a length of 1 unless that'll cause visual overlaps.
  // In this case choose the length based upon the distance to the next "ring" minus some buffer.
  if(minForce < 0)
    minForce *= Math::Min(1.0f, minDistance * 0.75f);
  else
    minForce *= Math::Min(1.0f, deltaDistance * 0.4f);
  if(maxForce < 0)
    maxForce *= Math::Min(1.0f, deltaDistance * 0.4f);
}

Cylinder GetSupportShapeCylinder(const Intersection::SupportShape& supportShape, Vec3Param primaryAxis)
{
  // We want to approximate a cylinder from the given support shape. To start
  // we need to build a basis from the primary axis.
  Vec3 axis0, axis1;
  Math::GenerateOrthonormalBasis(primaryAxis, &axis0, &axis1);

  // Get the center of the shape
  Vec3 center;
  supportShape.GetCenter(&center);

  // We can then find the points furthest in each basis direction so we can figure out the height and
  // radius of the cylinder (assume symmetric shapes for now).
  Vec3 supportPointPrimary, supportPoint0, supportPoint1;
  supportShape.Support(primaryAxis, &supportPointPrimary);
  supportShape.Support(axis0, &supportPoint0);
  supportShape.Support(axis1, &supportPoint1);
  // The cylinder's half-height vector can now be found by projecting its support point onto the flow direction.
  Vec3 cylinderHalfHeightAxis = Math::ProjectOnVector(supportPointPrimary - center, primaryAxis);
  // The radius can be found by projecting the other two points and then taking the max length between them
  real radius0 = Math::Length(Math::ProjectOnPlane(supportPoint0 - center, primaryAxis));
  real radius1 = Math::Length(Math::ProjectOnPlane(supportPoint1 - center, primaryAxis));

  Cylinder result;
  result.Radius = Math::Max(radius0, radius1);
  result.PointA = center - cylinderHalfHeightAxis;
  result.PointB = center + cylinderHalfHeightAxis;
  return result;
}

Cylinder GetCogCylinder(Cog* cog, Vec3Param primaryAxis)
{
  // If there's a collider then use it's support shape
  if(Collider* collider = cog->has(Collider))
    return GetSupportShapeCylinder(collider->GetSupportShape(), primaryAxis);

  // Otherwise, start with a default sized aabb. Use the transform to position the aabb if possible.
  Aabb aabb(Vec3::cZero, Vec3(0.5f));
  if(Transform* transform = cog->has(Transform))
    aabb.SetCenterAndHalfExtents(transform->GetWorldTranslation(), transform->GetWorldScale());

  Intersection::SupportShape supportShape = Intersection::MakeSupport(&aabb);
  return GetSupportShapeCylinder(supportShape, primaryAxis);
}

} // namespace Zero
