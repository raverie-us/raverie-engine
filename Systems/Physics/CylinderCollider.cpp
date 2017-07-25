///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(CylinderCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDocumented();
  
  ZilchBindGetterSetterProperty(Radius);
  ZilchBindGetterSetterProperty(Height);
  ZilchBindGetterSetterProperty(Direction);

  ZilchBindGetter(WorldRadius);
  ZilchBindGetter(WorldHeight);
}

CylinderCollider::CylinderCollider()
{
  mType = cCylinder;
}

void CylinderCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeNameDefault(mRadius, real(0.5f));
  SerializeNameDefault(mHeight, real(1.0f));
  SerializeEnumNameDefault(AxisDirection, mDirection, AxisDirection::Y);
}

void CylinderCollider::DebugDraw()
{
  Collider::DebugDraw();

  Vec3 pointA, pointB;
  ComputeWorldPoints(pointA, pointB);
  gDebugDraw->Add(Debug::Cylinder(pointA, pointB, GetWorldRadius()).Color(Color::Plum));
}

void CylinderCollider::CacheWorldValues()
{
  uint rIndex0, rIndex1;
  GetRadiiIndices(rIndex0, rIndex1);
  uint heightIndex = GetHeightIndex();

  // Get the scale for our height and radius (the radius scale
  // is the larger of the two relevant world-scale values)
  Vec3 worldScale = GetWorldScale();
  real worldHeightScale = worldScale[heightIndex];
  real worldRadiusScale = Math::Max(worldScale[rIndex0], worldScale[rIndex1]);

  mWorldHeight = worldHeightScale * mHeight;
  mWorldRadius = worldRadiusScale * mRadius;
}

void CylinderCollider::ComputeWorldAabbInternal()
{
  real worldRadius = GetWorldRadius();
  real worldHalfHeight = GetWorldHalfHeight();

  uint rIndex0, rIndex1;
  GetRadiiIndices(rIndex0, rIndex1);
  uint heightIndex = GetHeightIndex();

  // Compute the aabb of the cylinder as if it was an obb
  Vec3 halfExtents;
  halfExtents[heightIndex] = worldHalfHeight;
  halfExtents[rIndex0] = worldRadius;
  halfExtents[rIndex1] = worldRadius;
  SetWorldAabbFromHalfExtents(halfExtents);
}

void CylinderCollider::ComputeWorldBoundingSphereInternal()
{
  // To compute the minimum bounding sphere we can use the Pythagorean theorem
  // with the height and radius to get the exact sphere radius
  real radius = GetWorldRadius();
  real halfHeight = GetWorldHalfHeight();
  mBoundingSphere.mCenter = GetWorldTranslation();
  mBoundingSphere.mRadius = Math::Sqrt((radius * radius) + (halfHeight * halfHeight));
}

real CylinderCollider::ComputeWorldVolumeInternal()
{
  real radius = GetWorldRadius();
  real height = GetWorldHeight();
  return Math::cPi * radius * radius * height;
}

void CylinderCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  real oneTwelthMass = mass / real(12.0f);
  real worldRadius = GetWorldRadius();
  real worldHeight = GetWorldHeight();
  real rSquared = worldRadius * worldRadius;
  real hSquared = worldHeight * worldHeight;

  // Compute the unique inverted terms
  real heightInvInertia = real(1) / (real(0.5f) * mass * rSquared);
  real radiusInvInertia = real(1) / (oneTwelthMass * (real(3.0f) * rSquared + hSquared));

  // Get what local axes define the radius and height so we can set the correct local inertia
  uint rIndex0, rIndex1;
  GetRadiiIndices(rIndex0, rIndex1);
  uint heightIndex = GetHeightIndex();
  
  localInvInertia.SetIdentity();
  localInvInertia[rIndex0][rIndex0] = radiusInvInertia;
  localInvInertia[heightIndex][heightIndex] = heightInvInertia;
  localInvInertia[rIndex1][rIndex1] = radiusInvInertia;
}

void CylinderCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  Vec3 pointA, pointB;
  ComputeWorldPoints(pointA, pointB);
  Geometry::SupportCylinder(direction, pointA, pointB, GetWorldRadius(), support);
}

real CylinderCollider::GetRadius() const
{
  return mRadius;
}

void CylinderCollider::SetRadius(real radius)
{
  const float minScale = 0.01f;
  mRadius = Math::Max(radius, minScale);

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

real CylinderCollider::GetHeight() const
{
  return mHeight;
}

void CylinderCollider::SetHeight(real height)
{
  const float minScale = 0.01f;
  mHeight = Math::Max(height, minScale); 

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

AxisDirection::Enum CylinderCollider::GetDirection() const
{
  return mDirection;
}

void CylinderCollider::SetDirection(AxisDirection::Enum direction)
{
  // Safeguard against bad values
  if(direction >= AxisDirection::Size)
    direction = AxisDirection::Z;

  mDirection = direction;

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}


void CylinderCollider::GetRadiiIndices(uint& rIndex0, uint& rIndex1) const
{
  rIndex0 = (mDirection + 1) % 3;
  rIndex1 = (mDirection + 2) % 3;
}

uint CylinderCollider::GetHeightIndex() const
{
  return mDirection;
}

real CylinderCollider::GetWorldRadius() const
{
  return mWorldRadius;
}

real CylinderCollider::GetWorldHalfHeight() const
{
  return GetWorldHeight() * real(0.5f);
}

real CylinderCollider::GetWorldHeight() const
{
  return mWorldHeight;
}

void CylinderCollider::ComputeWorldPoints(Vec3Ref pointA, Vec3Ref pointB) const
{
  // The world-space vector that defines our height axis can be retrieved 
  // from the rotation matrix's basis vectors. This axis can then be scaled by the
  // half height so we can easily get the cylinder's two points.
  Mat3 rotation = GetWorldRotation();
  Vec3 worldPos = GetWorldTranslation();
  real cylinderHalfHeight = GetWorldHalfHeight();
  Vec3 heightAxis = rotation.GetBasis(GetHeightIndex());
  Vec3 worldHalfHeight = heightAxis * cylinderHalfHeight;

  pointA = worldPos + worldHalfHeight;
  pointB = worldPos - worldHalfHeight;
}

}//namespace Zero
