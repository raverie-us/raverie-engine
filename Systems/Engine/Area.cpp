///////////////////////////////////////////////////////////////////////////////
///
/// \file Area.hpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(AreaChanged);
}

ZilchDefineType(AreaEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(mArea);
}

ZilchDefineType(Area, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindEvent(Events::AreaChanged, AreaEvent);
  ZeroBindDependency(Transform);
 
  ZilchBindGetterSetterProperty(Origin)->ZeroLocalModificationOverride();
  ZilchBindGetter(TopLeft);
  ZilchBindGetter(TopCenter);
  ZilchBindGetter(TopRight);
  ZilchBindGetter(CenterLeft);
  ZilchBindGetter(Center);
  ZilchBindGetter(CenterRight);
  ZilchBindGetter(BottomLeft);
  ZilchBindGetter(BottomCenter);
  ZilchBindGetter(BottomRight);
  ZilchBindGetterSetterProperty(Size)->ZeroLocalModificationOverride();

  ZilchBindMethod(LocalOffsetOf);
}

void Area::SetDefaults()
{
  mSize = Vec2(1, 1);
  mOrigin = Location::Center;
}

void Area::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
}

void Area::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSize, Vec2(1,1));
  SerializeEnumName(Location, mOrigin);
}

Vec2 Area::GetSize()
{
  return mSize;
}

void Area::SetSize(Vec2 newSize)
{
  Math::Clamp(&newSize, 0.0f, 10000.0f);
  mSize = newSize;
  DoAreaChanged();
}

Location::Enum Area::GetOrigin()
{
  return mOrigin;
}

void Area::SetOrigin(Location::Enum origin)
{
  if(mOrigin != origin)
  {
    mOrigin = origin;
    DoAreaChanged();
  }
}

Vec2 Area::LocalOffsetOf(Location::Enum location)
{
  return OffsetOfOffset(location) * mSize;
}

Vec2 Area::OffsetOfOffset(Location::Enum location)
{
  return Location::GetDirection(mOrigin, location);
}

Aabb Area::GetLocalAabb()
{
  Vec3 offset = Vec3(-Location::GetDirection(mOrigin), 0);
  Vec3 size = Vec3(mSize, AreaThickness);

  return Aabb(offset * size, size * 0.5f);
}

Aabb Area::GetAabb()
{
  Vec3 offset = Vec3(-Location::GetDirection(mOrigin), 0);
  Vec3 size = Vec3(mSize, AreaThickness);

  return FromTransformAndExtents(mTransform, size * 0.5f,  offset * size);
}

void Area::DoAreaChanged()
{
  TransformUpdateInfo info;
  info.TransformFlags = TransformUpdateFlags::Translation;
  GetOwner()->TransformUpdate(info);

  AreaEvent event;
  event.mArea = this;
  this->GetOwner()->DispatchEvent(Events::AreaChanged, &event);
}

void Area::DebugDraw()
{

}

}
