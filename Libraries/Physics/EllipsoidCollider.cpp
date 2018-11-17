///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(EllipsoidCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Radii);
  ZilchBindGetter(WorldRadii);
}

EllipsoidCollider::EllipsoidCollider()
{
  mType = cEllipsoid;
}

void EllipsoidCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeNameDefault(mRadii, Vec3(0.5f));
}

void EllipsoidCollider::DebugDraw()
{
  Collider::DebugDraw();

  Vec3 pos = GetWorldTranslation();
  Mat3 rotation = GetWorldRotation();
  Vec3 worldRadii = GetWorldRadii();

  // Draw only one disc on the xz plane
  DebugDrawDisc(pos, rotation.BasisX(), rotation.BasisZ(), worldRadii.x, worldRadii.z);

  // To help visualize, draw 6 rings around the y-axis. To do this we
  // interpolate across the x-z ellipse to get an axis and radius.
  real increment = Math::cPi / 6.0f;
  for(real t = 0; t < Math::cPi; t += increment)
  {
    // Compute the local space point of the ellipse on the x-z plane
    Vec3 xAxis = Math::Cos(t) * rotation.BasisX() * worldRadii.x;
    Vec3 zAxis = Math::Sin(t) * rotation.BasisZ() * worldRadii.z;
    // Get the axis and radius along this axis
    Vec3 axis = xAxis + zAxis;
    real radius = axis.AttemptNormalize();
    // Draw a disc along this axis and the y basis vector
    DebugDrawDisc(pos, axis, rotation.BasisY(), radius, worldRadii.y);
  }
}

void EllipsoidCollider::ComputeWorldAabbInternal()
{
  SetWorldAabbFromHalfExtents(GetWorldRadii());
}

real EllipsoidCollider::ComputeWorldVolumeInternal()
{
  Vec3 worldRadii = GetWorldRadii();
  return real(4.0 / 3.0) * Math::cPi * worldRadii.x * worldRadii.y * worldRadii.z;
}

void EllipsoidCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  Vec3 radii = GetWorldRadii();
  real xSquared = radii.x * radii.x;
  real ySquared = radii.y * radii.y;
  real zSquared = radii.z * radii.z;
  real fifthMass = (mass / real(5.0));
  localInvInertia.SetIdentity();
  localInvInertia.m00 = real(1.0) / ((ySquared + zSquared) * fifthMass);
  localInvInertia.m11 = real(1.0) / ((xSquared + zSquared) * fifthMass);
  localInvInertia.m22 = real(1.0) / ((xSquared + ySquared) * fifthMass);
}

void EllipsoidCollider::ComputeWorldBoundingSphereInternal()
{
  // Get the maximum of the radii
  Vec3 worldRadii = GetWorldRadii();
  real maxRadius = Math::Max(Math::Max(worldRadii.x, worldRadii.y), worldRadii.z);
  mBoundingSphere.mCenter = GetWorldTranslation();
  mBoundingSphere.mRadius = maxRadius;
}

void EllipsoidCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportEllipsoid(direction, GetWorldTranslation(), GetWorldRadii(), GetWorldRotation(), support);
}

Vec3 EllipsoidCollider::GetRadii(void) const
{
  return mRadii;
}

void EllipsoidCollider::SetRadii(Vec3Param radii)
{
  const float minScale = 0.0001f;
  mRadii = Math::Max(radii, Vec3(minScale)); 

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

Vec3 EllipsoidCollider::GetWorldRadii() const
{
  return mRadii * GetWorldScale();
}

}//namespace Zero
