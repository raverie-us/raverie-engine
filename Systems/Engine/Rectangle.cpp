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
Rectangle Rectangle::PointAndSize(Vec2Param point, Vec2Param size)
{
  Rectangle r;
  r.X = point.x;
  r.Y = point.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

//**************************************************************************************************
Rectangle Rectangle::CenterAndSize(Vec2Param point, Vec2Param size)
{
  Vec2 halfSize = size / 2.0f;

  Rectangle r;
  r.X = point.x - halfSize.x;
  r.Y = point.y - halfSize.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

//**************************************************************************************************
Rectangle Rectangle::MinAndMax(Vec2Param min, Vec2Param max)
{
  Rectangle r;
  r.X = min.x;
  r.Y = min.y;
  r.SizeX = (max.x - min.x);
  r.SizeY = (max.y - min.y);
  return r;
}

//**************************************************************************************************
bool Rectangle::operator==(UiRectParam rhs) const
{
  bool result = Math::Abs(X - rhs.X) < Math::Epsilon();
  result &= Math::Abs(Y - rhs.Y) < Math::Epsilon();
  result &= Math::Abs(SizeX - rhs.SizeX) < Math::Epsilon();
  result &= Math::Abs(SizeY - rhs.SizeY) < Math::Epsilon();

  return result;
}

//**************************************************************************************************
Vec2 Rectangle::GetTranslation() const
{
  return GetBottomLeft();
}

//**************************************************************************************************
void Rectangle::SetTranslation(Vec2Param translation)
{
  X = translation.x;
  Y = translation.y;
}

//**************************************************************************************************
Vec2 Rectangle::GetSize() const
{
  return Vec2(SizeX, SizeY);
}

//**************************************************************************************************
void Rectangle::SetSize(Vec2Param size)
{
  SizeX = size.x;
  SizeY = size.y;
}

//**************************************************************************************************
bool Rectangle::Contains(Vec2Param point) const
{
  if (point.x < X) return false;
  if (point.x > X + SizeX) return false;
  if (point.y < Y) return false;
  if (point.y > Y + SizeY) return false;
  return true;
}

//**************************************************************************************************
bool Rectangle::Overlap(UiRectParam other) const
{
  bool noOverlap = (X > other.X + other.SizeX) ||
                   (Y > other.Y + other.SizeY) ||
                   (other.X > X + SizeX) ||
                   (other.Y > Y + SizeY);

  return !noOverlap;
}

//**************************************************************************************************
void Rectangle::RemoveThickness(const Thickness& thickness)
{
  X += thickness.Left;
  Y += thickness.Bottom;
  SizeX -= (thickness.Left + thickness.Right);
  SizeY -= (thickness.Top + thickness.Bottom);
}

}//namespace Zero
