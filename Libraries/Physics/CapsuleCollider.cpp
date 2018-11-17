///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(CapsuleCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Radius);
  ZilchBindGetterSetterProperty(Height);
  ZilchBindGetterSetterProperty(Direction);
  ZilchBindGetterSetterProperty(ScalingMode);

  ZilchBindGetter(WorldRadius);
  ZilchBindGetter(WorldCylinderHeight);
}

CapsuleCollider::CapsuleCollider()
{
  mType = cCapsule;
}

void CapsuleCollider::Initialize(CogInitializer& initializer)
{
  Collider::Initialize(initializer);
}

void CapsuleCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeNameDefault(mRadius, real(0.5f));
  SerializeNameDefault(mHeight, real(1.0f));
  SerializeEnumNameDefault(AxisDirection, mDirection, AxisDirection::Y);
  SerializeEnumNameDefault(CapsuleScalingMode, mScalingMode, CapsuleScalingMode::PreserveHeight);
}

void CapsuleCollider::DebugDraw()
{
  Collider::DebugDraw();

  Vec3 pointA, pointB;
  ComputeWorldPoints(pointA, pointB);
  gDebugDraw->Add(Debug::Capsule(pointA, pointB, GetWorldRadius()).Color(Color::Lime).OnTop(true));
}

void CapsuleCollider::CacheWorldValues()
{
  uint rIndex0, rIndex1;
  GetRadiiIndices(rIndex0, rIndex1);
  uint heightIndex = GetHeightIndex();

  // Get the scale for our height and radius (the radius scale
  // is the larger of the two relevant world-scale values)
  Vec3 worldScale = GetWorldScale();
  real worldHeightScale = worldScale[heightIndex];
  real worldRadiusScale = Math::Max(worldScale[rIndex0], worldScale[rIndex1]);

  if(mScalingMode == CapsuleScalingMode::PreserveScale)
  {
    mWorldRadius = worldRadiusScale * mRadius;
    // To preserve scale we need to compute the capsule's entire height (local height + the two radius end caps)
    // and then scale that up to world space. Since the radius should only be affected by the radius' scale then 
    // the height needs to account for the missing total height scale. This means we can simply find the cylinder's
    // height by removing the actual world radius values.
    real localHeight = mHeight + 2 * mRadius;
    real worldHeight = worldHeightScale * localHeight;
    mWorldCylinderHeight = worldHeight - 2 * mWorldRadius;
  }
  else
  {
    // Otherwise we simply scale each value up
    mWorldRadius = worldRadiusScale * mRadius;
    mWorldCylinderHeight = worldHeightScale * mHeight;
  }
}

void CapsuleCollider::ComputeWorldAabbInternal()
{
  Vec3 pointA, pointB;
  ComputeWorldPoints(pointA, pointB);
  real radius = GetWorldRadius();

  // The easiest way to compute the tight-fit aabb of a capsule is to first look at the
  // capsule as a line segment. It's easy to compute the line segment of the capsule as
  // just the min/max on each axis for both points. After this we can "add" in the radius of the capsule.
  // The min will always shrink by the radius while the max will grow by the radius.
  mAabb.Compute(pointA);
  mAabb.Expand(pointB);
  mAabb.mMin -= Vec3(radius);
  mAabb.mMax += Vec3(radius);
}

void CapsuleCollider::ComputeWorldBoundingSphereInternal()
{
  Vec3 pointA, pointB;
  ComputeWorldPoints(pointA, pointB);
  real radius = GetWorldRadius();

  // The minimum bounding sphere will always be at the mid-point of the two sphere centers.
  // The radius is then simply the radius of this segment plus the sphere-cap radius.
  mBoundingSphere.mCenter = (pointA + pointB) * 0.5f;
  mBoundingSphere.mRadius = Math::Length(pointA - pointB) * 0.5f + radius;
}

real CapsuleCollider::ComputeWorldVolumeInternal()
{
  real radius = GetWorldRadius();
  real height = GetWorldCylinderHeight();
  real rSquared = radius * radius;

  real cylinderVolume = Math::cPi * rSquared * height;
  real hemiSphereVolume = real(2.0f / 3.0) * Math::cPi * rSquared * radius;

  return cylinderVolume + 2 * hemiSphereVolume;
}

void CapsuleCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  // A capsule's inertia is pieced together from two hemisphere's inertia
  // and a cylinder's inertia using the parallel axis theorem
  real worldRadius = GetWorldRadius();
  real worldHeight = GetWorldCylinderHeight();
  real rSquared = worldRadius * worldRadius;
  real hSquared = worldHeight * worldHeight;

  // Unfortunately, we need to have piece-wise mass for the hemisphere and cylinder
  // so we can't use the passed in mass (currently this is just the total mass so there's
  // no real problem other than performance with recomputing it)
  real density = GetMaterial()->mDensity;
  real cylinderVolume = Math::cPi * rSquared * worldHeight;
  real hemiSphereVolume = real(2.0f / 3.0f) * Math::cPi * rSquared * worldRadius;
  real cylinderMass = cylinderVolume * density;
  real hemiSphereMass = hemiSphereVolume * density;

  // Compute the unique hemisphere and cylinder inertia terms (the hemisphere is strangely enough symmetric)
  real hemiSphereInertia = real(2.0f / 5.0f) * hemiSphereMass * rSquared;
  real cylinderRadiusInertia = real(1.0f / 2.0f) * cylinderMass * rSquared;
  real cylinderHeightInertia = (cylinderMass / 12.0f) * (hSquared + 3 * rSquared);

  // The height is just the sum of the cylinder and the two radii (no extra terms show up)
  real heightInertia = cylinderHeightInertia + 2 * hemiSphereInertia;
  // The radius (the local 'x' and 'z' axes) axis gets a large amount of extra terms since it
  // should be harder to rotate about the x or z axes than a cylinder and a sphere put together
  // (since the sphere's are offset from the center of the cylinder)
  real radiusInertiaOffsetTerm = hemiSphereMass * worldHeight * ((2 * worldHeight + 3 * worldRadius) / 8.0f);
  real radiusInertia = cylinderRadiusInertia + 2 * (hemiSphereInertia + radiusInertiaOffsetTerm);
  real invHeightInertia = real(1.0f) / heightInertia;
  real invRadiusInertia = real(1.0f) / radiusInertia;

  // Get what local axes define the radius and height so we can set the correct local inertia
  uint rIndex0, rIndex1;
  GetRadiiIndices(rIndex0, rIndex1);
  uint heightIndex = GetHeightIndex();

  localInvInertia.SetIdentity();
  localInvInertia[rIndex0][rIndex0] = invRadiusInertia;
  localInvInertia[heightIndex][heightIndex] = invHeightInertia;
  localInvInertia[rIndex1][rIndex1] = invRadiusInertia;
}

void CapsuleCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  Vec3 pointA, pointB;
  ComputeWorldPoints(pointA, pointB);
  Geometry::SupportCapsule(direction, pointA, pointB, GetWorldRadius(), support);
}

real CapsuleCollider::GetRadius() const
{
  return mRadius;
}

void CapsuleCollider::SetRadius(real radius)
{
  const real minRadius = real(0.0001f);
  mRadius = Math::Max(radius, minRadius); 

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

real CapsuleCollider::GetHeight() const
{
  return mHeight;
}

void CapsuleCollider::SetHeight(real height)
{
  const real minHeight = real(0.0001f);
  mHeight = Math::Max(height, minHeight); 

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

AxisDirection::Enum CapsuleCollider::GetDirection() const
{
  return mDirection;
}

void CapsuleCollider::SetDirection(AxisDirection::Enum direction)
{
  // Safeguard against bad values
  if(direction >= AxisDirection::Size)
    direction = AxisDirection::Z;
  mDirection = direction;

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

CapsuleScalingMode::Enum CapsuleCollider::GetScalingMode() const
{
  return mScalingMode;
}

void CapsuleCollider::SetScalingMode(CapsuleScalingMode::Enum mode)
{
  mScalingMode = mode;
  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

void CapsuleCollider::GetRadiiIndices(uint& rIndex0, uint& rIndex1) const
{
  // Direction defines the height index so the radii indices are the other two
  rIndex0 = (mDirection + 1) % 3;
  rIndex1 = (mDirection + 2) % 3;
}

uint CapsuleCollider::GetHeightIndex() const
{
  return mDirection;
}

real CapsuleCollider::GetWorldRadius() const
{
  return mWorldRadius;
}

real CapsuleCollider::GetWorldCylinderHalfHeight() const
{
  return GetWorldCylinderHeight() * real(0.5f);
}

real CapsuleCollider::GetWorldCylinderHeight() const
{
  return mWorldCylinderHeight;
}

void CapsuleCollider::ComputeWorldPoints(Vec3Ref pointA, Vec3Ref pointB) const
{
  // The world-space vector that defines our height axis can be retrieved 
  // from the rotation matrix's basis vectors. This axis can then be scaled by the
  // half height so we can easily get the two sphere center points.
  Mat3 rot = GetWorldRotation();
  Vec3 worldTranslation = GetWorldTranslation();
  Vec3 heightAxis = rot.GetBasis(GetHeightIndex());
  real cylinderHalfHeight = GetWorldCylinderHalfHeight();
  Vec3 worldHalfHeight = heightAxis * cylinderHalfHeight;
  pointA = worldTranslation + worldHalfHeight;
  pointB = worldTranslation - worldHalfHeight;
}

}//namespace Zero
