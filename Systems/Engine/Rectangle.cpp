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
  ZilchBindMethod(Expand);

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
bool Rectangle::operator==(UiRectParam rhs) const
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
    Vec2 topLeft = GetTopLeft();
    Min = Vec2(topLeft.x, size.y);
    Max = Vec2(size.x, topLeft.y);
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
    Vec2 bottomRight = GetBottomRight();
    Min = Vec2(bottomRight.x - size.x, bottomRight.y);
    Max = Vec2(bottomRight.x, bottomRight.y + size.y);
    return;
  }
  }

  DoNotifyException("Location not supported.", "Only TopLeft, TopRight, BottomLeft, and BottomRight"
                    " are currently supported.");
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
bool Rectangle::Overlap(UiRectParam other) const
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
