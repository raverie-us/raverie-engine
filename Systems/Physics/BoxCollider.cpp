//////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const Vec3 BoxCollider::mMinAllowedSize = Vec3(0.0001f);
const Vec3 BoxCollider::mMaxAllowedSize = Vec3(1000000.0f);

ZilchDefineType(BoxCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDocumented();

  ZilchBindGetterSetter(HalfSize)->ZeroSerialize(Vec3(0.5f));
  // @MetaSerialization: Property needs to cause rescans
  ZilchBindGetterSetterProperty(Size);
  ZilchBindGetter(WorldSize);
}

BoxCollider::BoxCollider()
{
  mType = cBox;
}

void BoxCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  
  stream.SerializeFieldDefault("HalfSize", mLocalHalfSize, Vec3(real(0.5f)));
  // To deal with bad serialized values (most before the setter was fixed)
  mLocalHalfSize = Math::Clamp(mLocalHalfSize, mMinAllowedSize, mMaxAllowedSize);
}

void BoxCollider::DebugDraw()
{
  Collider::DebugDraw();
  gDebugDraw->Add(Debug::Obb(GetWorldTranslation(), mWorldHalfSize, GetWorldRotation()).Color(Color::Plum).BackShade(true));
}

void BoxCollider::CacheWorldValues()
{
  Vec3 worldScale = GetWorldScale();
  worldScale = Math::Abs(worldScale);
  mWorldHalfSize = mLocalHalfSize * worldScale;
}

void BoxCollider::ComputeWorldAabbInternal()
{
  SetWorldAabbFromHalfExtents(mWorldHalfSize);
}

real BoxCollider::ComputeWorldVolumeInternal()
{
  return real(8.0f) * mWorldHalfSize[0] * mWorldHalfSize[1] * mWorldHalfSize[2];
}

void BoxCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  Vec3 size = GetWorldSize();
  real diagonal = real(1.0f / 12.0f) * mass;
  real x = diagonal * (size.y * size.y + size.z * size.z);
  real y = diagonal * (size.x * size.x + size.z * size.z);
  real z = diagonal * (size.x * size.x + size.y * size.y);
  localInvInertia.SetIdentity();
  localInvInertia.Scale(real(1.0f) / x, real(1.0f) / y, real(1.0f) / z);
}

void BoxCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportObb(direction, GetWorldTranslation(), mWorldHalfSize, GetWorldRotation(), support);
}

Vec3 BoxCollider::GetHalfSize()
{
  return mLocalHalfSize;
}

void BoxCollider::SetHalfSize(Vec3Param localHalfSize)
{
  mLocalHalfSize = Math::Clamp(localHalfSize, mMinAllowedSize, mMaxAllowedSize);

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

Vec3 BoxCollider::GetSize() const
{
  return mLocalHalfSize * real(2.0f);
}

void BoxCollider::SetSize(Vec3Param localSize)
{
  // This is just a more convenient view of half-size (which is actually saved)
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "HalfSize", GetHalfSize());

  Vec3 localHalfSize = localSize * real(0.5f);
  mLocalHalfSize = Math::Clamp(localHalfSize, mMinAllowedSize, mMaxAllowedSize); 
  
  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

Vec3 BoxCollider::GetWorldSize() const
{
  return real(2) * mWorldHalfSize;
}

}//namespace Zero
