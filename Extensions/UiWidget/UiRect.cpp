////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------------ Ui Rect
const UiRect UiRect::cZero = UiRect::CenterAndSize(Vec2(0, 0), Vec2(0, 0));

//**************************************************************************************************
ZilchDefineType(UiRect, builder, type)
{
  type->CreatableInScript = true;

  ZilchBindFieldProperty(X);
  ZilchBindFieldProperty(Y);
  ZilchBindFieldProperty(SizeX);
  ZilchBindFieldProperty(SizeY);
  ZilchBindGetterSetterProperty(Translation);
  ZilchBindGetterSetterProperty(Size);

  ZilchBindGetterProperty(TopLeft);
  ZilchBindGetterProperty(TopRight);
  ZilchBindGetterProperty(BottomLeft);
  ZilchBindGetterProperty(BottomRight);
  ZilchBindGetterProperty(Center);

  ZilchBindGetterProperty(Left);
  ZilchBindGetterProperty(Right);
  ZilchBindGetterProperty(Top);
  ZilchBindGetterProperty(Bottom);

  ZilchBindMethod(Contains);
  ZilchBindMethod(Overlap);
  ZilchBindMethod(RemoveThickness);
}

//**************************************************************************************************
UiRect UiRect::PointAndSize(Vec2Param point, Vec2Param size)
{
  UiRect r;
  r.X = point.x;
  r.Y = point.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

//**************************************************************************************************
UiRect UiRect::CenterAndSize(Vec2Param point, Vec2Param size)
{
  Vec2 halfSize = size / 2.0f;

  UiRect r;
  r.X = point.x - halfSize.x;
  r.Y = point.y - halfSize.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

//**************************************************************************************************
UiRect UiRect::MinAndMax(Vec2Param min, Vec2Param max)
{
  UiRect r;
  r.X = min.x;
  r.Y = min.y;
  r.SizeX = (max.x - min.x);
  r.SizeY = (max.y - min.y);
  return r;
}

//**************************************************************************************************
bool UiRect::operator==(UiRectParam rhs) const
{
  bool result = Math::Abs(X - rhs.X) < Math::Epsilon();
  result &= Math::Abs(Y - rhs.Y) < Math::Epsilon();
  result &= Math::Abs(SizeX - rhs.SizeX) < Math::Epsilon();
  result &= Math::Abs(SizeY - rhs.SizeY) < Math::Epsilon();

  return result;
}

//**************************************************************************************************
Vec2 UiRect::GetTranslation() const
{
  return GetBottomLeft();
}

//**************************************************************************************************
void UiRect::SetTranslation(Vec2Param translation)
{
  X = translation.x;
  Y = translation.y;
}

//**************************************************************************************************
Vec2 UiRect::GetSize() const
{
  return Vec2(SizeX, SizeY);
}

//**************************************************************************************************
void UiRect::SetSize(Vec2Param size)
{
  SizeX = size.x;
  SizeY = size.y;
}

//**************************************************************************************************
bool UiRect::Contains(Vec2Param point) const
{
  if (point.x < X) return false;
  if (point.x > X + SizeX) return false;
  if (point.y < Y) return false;
  if (point.y > Y + SizeY) return false;
  return true;
}

//**************************************************************************************************
bool UiRect::Overlap(UiRectParam other) const
{
  bool noOverlap = (X > other.X + other.SizeX) ||
                   (Y > other.Y + other.SizeY) ||
                   (other.X > X + SizeX) ||
                   (other.Y > Y + SizeY);

  return !noOverlap;
}

//**************************************************************************************************
void UiRect::RemoveThickness(const Thickness& thickness)
{
  X += thickness.Left;
  Y += thickness.Bottom;
  SizeX -= (thickness.Left + thickness.Right);
  SizeY -= (thickness.Top + thickness.Bottom);
}

}//namespace Zero
