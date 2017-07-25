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

Vec2 OffsetOfOffset(Location::Enum baseOrign, Location::Enum cornerOrigin)
{
  Vec2 offset = ToOffset(baseOrign);
  Vec2 offsetAgain = ToOffset(cornerOrigin);
  return offset - offsetAgain;
}

Vec2 ToOffset(Location::Enum orign)
{
  const float base = 0.5f;
  switch(orign)
  {
  case Location::TopLeft:
    return Vec2( 0.5f, -0.5f);
  case Location::TopCenter:
    return Vec2( 0, -0.5f);
  case Location::TopRight:
    return Vec2( -0.5f, -0.5f);

  case Location::CenterLeft:
    return Vec2(  0.5f, 0);
  case Location::CenterRight:
    return Vec2( -0.5f, 0);
  case Location::Center:
  default:
    return Vec2(0, 0);

  case Location::BottomLeft:
    return Vec2( 0.5f, 0.5f);
  case Location::BottomCenter:
    return Vec2( 0,    0.5f);
  case Location::BottomRight:
    return Vec2( -0.5f, 0.5f);
  }
}

ZilchDefineType(AreaEvent, builder, type)
{
}

ZilchDefineType(Area, builder, type)
{
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindEvent(Events::AreaChanged, AreaEvent);
  ZeroBindDependency(Transform);
 
  ZilchBindDefaultConstructor();
  ZilchBindGetterSetterProperty(Origin)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetter(TopLeft);
  ZilchBindGetter(TopCenter);
  ZilchBindGetter(TopRight);
  ZilchBindGetter(CenterLeft);
  ZilchBindGetter(Center);
  ZilchBindGetter(CenterRight);
  ZilchBindGetter(BottomLeft);
  ZilchBindGetter(BottomCenter);
  ZilchBindGetter(BottomRight);
  ZilchBindGetterSetterProperty(Size)->AddAttribute(PropertyAttributes::cLocalModificationOverride);

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
  mOrigin = origin;
  DoAreaChanged();
}

Vec2 Area::LocalOffsetOf(Location::Enum location)
{
  Vec2 offset = OffsetOfOffset(mOrigin, location);
  return offset * mSize;
}

Aabb Area::GetLocalAabb()
{
  Vec3 offset = Vec3(ToOffset(mOrigin), 0);
  Vec3 size = Vec3(mSize, AreaThickness);

  return Aabb(offset * size, size * 0.5f);
}

Aabb Area::GetAabb()
{
  Vec3 offset = Vec3(ToOffset(mOrigin), 0);
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
