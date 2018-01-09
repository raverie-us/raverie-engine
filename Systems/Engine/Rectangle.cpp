////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------------- Thickness
const Thickness Thickness::cZero(0, 0, 0, 0);

//**************************************************************************************************
ZilchDefineType(Thickness, builder, type)
{
  type->CreatableInScript = true;

  ZilchBindDestructor();
  ZilchBindConstructor(float, float, float, float);
  ZilchBindConstructor(Vec4);
  ZilchBindConstructor(float, float);
  ZilchBindConstructor(Vec2);

  ZilchBindMethod(All);

  ZilchBindFieldProperty(Left);
  ZilchBindFieldProperty(Top);
  ZilchBindFieldProperty(Right);
  ZilchBindFieldProperty(Bottom);
  ZilchBindFieldGetter(cZero);

  ZilchBindMethod(Size);
  ZilchBindMethod(TopLeft);
}

//**************************************************************************************************
Thickness::Thickness()
  : Left(0), Top(0), Right(0), Bottom(0)
{

}

//**************************************************************************************************
Thickness::Thickness(float splat)
  : Left(splat), Top(splat), Right(splat), Bottom(splat)
{
}

//**************************************************************************************************
Thickness::Thickness(float left, float top, float right, float bottom)
  : Left(left), Top(top), Right(right), Bottom(bottom)
{

}

//**************************************************************************************************
Thickness::Thickness(Vec4 vector)
  : Left(vector.x), Top(vector.y), Right(vector.z), Bottom(vector.w)
{

}

//**************************************************************************************************
Thickness::Thickness(float leftRight, float topBottom)
  : Left(leftRight), Top(topBottom), Right(leftRight), Bottom(topBottom)
{

}

//**************************************************************************************************
Thickness::Thickness(Vec2 vector)
  : Left(vector.x), Top(vector.y), Right(vector.x), Bottom(vector.y)
{

}

//**************************************************************************************************
Thickness Thickness::All(float amount)
{
  return Thickness(amount, amount, amount, amount);
}

//**************************************************************************************************
Thickness Thickness::operator+(const Thickness& rhs)
{
  return Thickness(Left + rhs.Left, Top + rhs.Top, Right + rhs.Right, Bottom + rhs.Bottom);
}

//------------------------------------------------------------------------------------------ Ui Rect
const Rectangle Rectangle::cZero = Rectangle::CenterAndSize(Vec2(0, 0), Vec2(0, 0));

//**************************************************************************************************
ZilchDefineType(Rectangle, builder, type)
{
  type->CreatableInScript = true;

  ZilchBindFieldProperty(Min);
  ZilchBindFieldProperty(Max);
  ZilchBindGetterProperty(Size);
  ZilchBindMethod(SetSize);
  ZilchBindOverloadedMethod(ResizeToPoint, ZilchInstanceOverload(void, Location::Enum, Vec2Param));
  ZilchBindOverloadedMethod(ResizeToPoint, ZilchInstanceOverload(void, Location::Enum, Vec2Param, Vec2Param));
  ZilchBindMethod(Expand);

  ZilchBindOverloadedMethod(Transform, ZilchInstanceOverload(void, Mat2Param));
  ZilchBindOverloadedMethod(Transform, ZilchInstanceOverload(void, Mat3Param));
  ZilchBindOverloadedMethod(Transform, ZilchInstanceOverload(void, Mat4Param));
  ZilchBindOverloadedMethod(Transformed, ZilchConstInstanceOverload(Rectangle, Mat2Param));
  ZilchBindOverloadedMethod(Transformed, ZilchConstInstanceOverload(Rectangle, Mat3Param));
  ZilchBindOverloadedMethod(Transformed, ZilchConstInstanceOverload(Rectangle, Mat4Param));

  ZilchBindGetterSetterProperty(TopLeft);
  ZilchBindGetterSetterProperty(TopRight);
  ZilchBindGetterSetterProperty(BottomLeft);
  ZilchBindGetterSetterProperty(BottomRight);
  ZilchBindGetterSetterProperty(Center);

  ZilchBindGetterSetterProperty(Left);
  ZilchBindGetterSetterProperty(Right);
  ZilchBindGetterSetterProperty(Top);
  ZilchBindGetterSetterProperty(Bottom);

  ZilchBindMethod(Contains);
  ZilchBindMethod(Overlap);
  ZilchBindMethod(RemoveThickness);
}

//**************************************************************************************************
Rectangle Rectangle::PointAndSize(Vec2Param point, Vec2Param size)
{
  return MinAndMax(point, point + size);
}

//**************************************************************************************************
Rectangle Rectangle::CenterAndSize(Vec2Param point, Vec2Param size)
{
  Vec2 halfSize = size * 0.5f;

  return MinAndMax(point - halfSize, point + halfSize);
}

//**************************************************************************************************
Rectangle Rectangle::MinAndMax(Vec2Param min, Vec2Param max)
{
  Rectangle r;
  r.Min = min;
  r.Max = max;
  return r;
}

//**************************************************************************************************
bool Rectangle::operator==(RectangleParam rhs) const
{
  return (Min == rhs.Min) && (Max == rhs.Max);
}

//**************************************************************************************************
void Rectangle::Translate(Vec2Param translation)
{
  Min += translation;
  Max += translation;
}

//**************************************************************************************************
void Rectangle::Transform(Mat2Param transform)
{
  Mat3 mat3(transform.m00, transform.m01, 0,
            transform.m10, transform.m11, 0,
            0,             0,             1);

  Transform(mat3);
}

//**************************************************************************************************
void Rectangle::Transform(Mat3Param transform)
{
  Mat2 rotationScale = Math::ToMatrix2(transform);
  for (size_t y = 0; y < 2; ++y)
    for (size_t x = 0; x < 2; ++x)
      rotationScale[y][x] = Math::Abs(rotationScale[y][x]);

  Vec2 center = GetCenter();
  Vec2 halfExtent = GetSize() * 0.5f;

  halfExtent = rotationScale.Transform(halfExtent);
  center = Math::TransformPoint(transform, center);

  Min = center - halfExtent;
  Max = center + halfExtent;
}

//**************************************************************************************************
void Rectangle::Transform(Mat4Param transform)
{
  Aabb aabb;
  aabb.SetMinAndMax(Vec3(Min), Vec3(Max));
  aabb.Transform(transform);
  Min = ToVector2(aabb.mMin);
  Max = ToVector2(aabb.mMax);
}

//**************************************************************************************************
Rectangle Rectangle::Transformed(Mat2Param transform) const
{
  Rectangle other = *this;
  other.Transform(transform);
  return other;
}

//**************************************************************************************************
Rectangle Rectangle::Transformed(Mat3Param transform) const
{
  Rectangle other = *this;
  other.Transform(transform);
  return other;
}

//**************************************************************************************************
Rectangle Rectangle::Transformed(Mat4Param transform) const
{
  Rectangle other = *this;
  other.Transform(transform);
  return other;
}

//**************************************************************************************************
Vec2 Rectangle::GetSize() const
{
  return Max - Min;
}

//**************************************************************************************************
void Rectangle::SetSize(Location::Enum origin, Vec2Param size)
{
  switch(origin)
  {
  case Location::TopLeft:
  {
    Min.y = Max.y - size.y;
    Max.x = Min.x + size.x;
    return;
  }
  case Location::TopRight:
  {
    Min = Max - size;
    return;
  }
  case Location::BottomLeft:
  {
    Max = Min + size;
    return;
  }
  case Location::BottomRight:
  {
    Min.x = Max.x - size.x;
    Max.y = Min.y + size.y;
    return;
  }
  case Location::CenterLeft:
  {
    Max.x = Min.x + size.x;
    return;
  }
  case Location::TopCenter:
  {
    Min.y = Max.y - size.y;
    return;
  }
  case Location::CenterRight:
  {
    Min.x = Max.x - size.x;
    return;
  }
  case Location::BottomCenter:
  {
    Max.y = Min.y + size.y;
    return;
  }
  case Location::Center:
  {
    Vec2 center = GetCenter();
    Vec2 halfSize = size * 0.5f;
    Min = center - halfSize;
    Max = center + halfSize;
    return;
  }
  }
}

//**************************************************************************************************
void Rectangle::ResizeToPoint(Location::Enum location, Vec2Param position)
{
  ResizeToPoint(location, position, Vec2::cZero);
}

void Rectangle::ResizeToPoint(Location::Enum location, Vec2Param position, Vec2Param minSize)
{
  switch (location)
  {
  case Location::TopLeft:
  {
    Min.x = Math::Min(position.x, Max.x - minSize.x);
    Max.y = Math::Max(position.y, Min.y + minSize.y);
    return;
  }
  case Location::TopRight:
  {
    Max = Math::Max(position, Min + minSize);
    return;
  }
  case Location::BottomLeft:
  {
    Min = Math::Min(position, Max - minSize);
    return;
  }
  case Location::BottomRight:
  {
    Min.y = Math::Min(position.y, Max.y - minSize.y);
    Max.x = Math::Max(position.x, Min.x + minSize.x);
    return;
  }
  case Location::CenterLeft:
  {
    Min.x = Math::Min(position.x, Max.x - minSize.x);
    return;
  }
  case Location::TopCenter:
  {
    Max.y = Math::Max(position.y, Min.y + minSize.y);
    return;
  }
  case Location::CenterRight:
  {
    Max.x = Math::Max(position.x, Min.x + minSize.x);
    return;
  }
  case Location::BottomCenter:
  {
    Min.y = Math::Min(position.y, Max.y - minSize.y);
    return;
  }
  case Location::Center:
  {
    DoNotifyException("Location not supported.", "Location.Center is not implemented as it doesn't make sense in this context.");
  }
  }
}

//**************************************************************************************************
void Rectangle::Expand(Vec2Param point)
{
  Min.x = Math::Min(Min.x, point.x);
  Min.y = Math::Min(Min.y, point.y);

  Max.x = Math::Max(Max.x, point.x);
  Max.y = Math::Max(Max.y, point.y);
}

//**************************************************************************************************
bool Rectangle::Contains(Vec2Param point) const
{
  if (point.x < Min.x)return false;
  if (point.x > Max.x) return false;
  if (point.y < Min.y) return false;
  if (point.y > Max.y) return false;
  return true;
}

//**************************************************************************************************
bool Rectangle::Overlap(RectangleParam other) const
{
  bool noOverlap = (Min.x > other.Max.x) ||
                   (Min.y > other.Max.y) ||
                   (other.Min.x > Max.x) ||
                   (other.Min.y > Max.y);

  return !noOverlap;
}

//**************************************************************************************************
void Rectangle::RemoveThickness(const Thickness& thickness)
{
  Min.x += thickness.Left;
  Min.y += thickness.Bottom;
  Max.x -= thickness.Right;
  Max.y -= thickness.Top;
}

//**************************************************************************************************
Vec2 Rectangle::GetTopLeft() const
{
  return Vec2(Min.x, Max.y);
}

//**************************************************************************************************
void Rectangle::SetTopLeft(Vec2Param point)
{
  Translate(point - GetTopLeft());
}

//**************************************************************************************************
Vec2 Rectangle::GetTopRight() const
{
  return Max;
}

//**************************************************************************************************
void Rectangle::SetTopRight(Vec2Param point)
{
  Translate(point - GetTopRight());
}

//**************************************************************************************************
Vec2 Rectangle::GetBottomLeft() const
{
  return Min;
}

//**************************************************************************************************
void Rectangle::SetBottomLeft(Vec2Param point)
{
  Translate(point - GetBottomLeft());
}

//**************************************************************************************************
Vec2 Rectangle::GetBottomRight() const
{
  return Vec2(Max.x, Min.y);
}

//**************************************************************************************************
void Rectangle::SetBottomRight(Vec2Param point)
{
  Translate(point - GetBottomRight());
}

//**************************************************************************************************
Vec2 Rectangle::GetCenter() const
{
  return (Min + Max) * 0.5;
}

//**************************************************************************************************
void Rectangle::SetCenter(Vec2Param point)
{
  Translate(point - GetCenter());
}

//**************************************************************************************************
float Rectangle::GetLeft() const
{
  return Min.x;
}

//**************************************************************************************************
void Rectangle::SetLeft(float left)
{
  Translate(Vec2(left - GetLeft(), 0));
}

//**************************************************************************************************
float Rectangle::GetRight() const
{
  return Max.x;
}

//**************************************************************************************************
void Rectangle::SetRight(float right)
{
  Translate(Vec2(right - GetRight(), 0));
}

//**************************************************************************************************
float Rectangle::GetTop() const
{
  return Max.y;
}

//**************************************************************************************************
void Rectangle::SetTop(float top)
{
  Translate(Vec2(0, top - GetTop()));
}

//**************************************************************************************************
float Rectangle::GetBottom() const
{
  return Min.y;
}

//**************************************************************************************************
void Rectangle::SetBottom(float bottom)
{
  Translate(Vec2(0, bottom - GetBottom()));
}

}//namespace Zero
