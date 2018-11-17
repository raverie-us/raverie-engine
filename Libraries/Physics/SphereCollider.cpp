///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const real SphereCollider::mMinAllowedRadius = real(0.0001f);
const real SphereCollider::mMaxAllowedRadius = real(1000000.0f);

ZilchDefineType(SphereCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindInterface(Collider);
  ZilchBindGetterSetterProperty(Radius);
  ZilchBindGetter(WorldRadius);
}

SphereCollider::SphereCollider()
{
  mType = cSphere;
}

void SphereCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeNameDefault(mRadius, real(0.5f));

  // To deal with bad serialized values (most before the setter was fixed)
  mRadius = Math::Clamp(mRadius, mMinAllowedRadius, mMaxAllowedRadius);
}

void SphereCollider::DebugDraw()
{
  Collider::DebugDraw();
  gDebugDraw->Add(Debug::Sphere(GetWorldTranslation(), GetWorldRadius()).Color(Color::Plum).OnTop(true));
}

void SphereCollider::CacheWorldValues()
{
  Vec3 worldScale = GetWorldScale();
  real largestScale = Math::Max(Math::Max(worldScale[0], worldScale[1]), worldScale[2]);
  mWorldRadius = mRadius * largestScale;
}

void SphereCollider::ComputeWorldAabbInternal()
{
  // We don't want the default world aabb logic. Since we're a sphere, the world-space
  // aabb doesn't change, it's always the min aabb that encompasses our radius.
  real radius = GetWorldRadius();
  Vec3 worldCenter = GetWorldTranslation();
  mAabb.SetCenterAndHalfExtents(worldCenter, Vec3(radius));
}

void SphereCollider::ComputeWorldBoundingSphereInternal()
{
  // We also don't want the default world bounding sphere logic.
  // Obviously we know best (being a sphere) what our bounding sphere is.
  mBoundingSphere.mCenter = GetWorldTranslation();
  mBoundingSphere.mRadius = GetWorldRadius();
}
real SphereCollider::ComputeWorldVolumeInternal()
{
  real radius = GetWorldRadius();
  return real(4.0f / 3.0) * Math::cPi * radius * radius * radius;
}

void SphereCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  real radius = GetWorldRadius();
  real diagonal = real(2.0f / 5.0f) * mass * radius * radius;
  localInvInertia.SetIdentity();
  localInvInertia.m00 = real(1.0f) / diagonal;
  localInvInertia.m11 = localInvInertia.m00;
  localInvInertia.m22 = localInvInertia.m00;
}

void SphereCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportSphere(direction, GetWorldTranslation(), GetWorldRadius(), support);
}

real SphereCollider::GetRadius() const
{
  return mRadius;
}

void SphereCollider::SetRadius(real radius)
{
  mRadius = Math::Clamp(radius, mMinAllowedRadius, mMaxAllowedRadius); 

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

real SphereCollider::GetWorldRadius() const
{
  return mWorldRadius;
}

}//namespace Zero
