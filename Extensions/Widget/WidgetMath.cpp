///////////////////////////////////////////////////////////////////////////////
///
/// \file WidgetMath.cpp
///  Implementation of the basic Widget layout helpers.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------- Margins
const Thickness Thickness::cZero(0,0,0,0);

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

Thickness::Thickness()
  : Left(0), Top(0), Right(0), Bottom(0)
{

}

Thickness::Thickness(float left, float top, float right, float bottom)
  : Left(left), Top(top), Right(right), Bottom(bottom)
{

}

Thickness::Thickness(Vec4 vector)
: Left(vector.x), Top(vector.y), Right(vector.z), Bottom(vector.w)
{

}

Thickness::Thickness(float leftRight, float topBottom)
  : Left(leftRight), Top(topBottom), Right(leftRight), Bottom(topBottom)
{

}

Thickness::Thickness(Vec2 vector)
 : Left(vector.x), Top(vector.y), Right(vector.x), Bottom(vector.y)
{

}

Thickness Thickness::All(float amount)
{
  return Thickness(amount, amount, amount, amount);
}

// Add Thickness
Thickness Thickness::operator+(const Thickness& rhs)
{
  return Thickness(Left + rhs.Left, Top + rhs.Top, Right + rhs.Right, Bottom + rhs.Bottom);
}

//------------------------------------------------------------------------- Rect

ZilchDefineType(Rect, builder, type)
{
  type->CreatableInScript = true;

  ZilchBindFieldProperty(X);
  ZilchBindFieldProperty(Y);
  //ZilchBindFieldProperty(Position);
  ZilchBindFieldProperty(SizeX);
  ZilchBindFieldProperty(SizeY);
  //ZilchBindFieldProperty(Size);

  ZilchBindMethod(SetTranslation);
  ZilchBindMethod(SetSize);
  ZilchBindMethod(Expand);
  ZilchBindMethod(Contains);
  ZilchBindMethod(Overlap);
  ZilchBindMethod(RemoveThickness);
  ZilchBindMethod(TopLeft);
  ZilchBindMethod(TopRight);
  ZilchBindMethod(BottomLeft);
  ZilchBindMethod(BottomRight);
  ZilchBindMethod(Center);

  ZilchBindMethod(Left);
  ZilchBindMethod(Right);
  ZilchBindMethod(Top);
  ZilchBindMethod(Bottom);
}

Rect Rect::CenterAndSize(Vec2Param point, Vec2Param size)
{
  Vec2 halfSize = size / 2.0f;

  Rect r;
  r.X = point.x - halfSize.x;
  r.Y = point.y - halfSize.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

Rect Rect::PointAndSize(Vec2Param point, Vec2Param size)
{
  Rect r;
  r.X = point.x;
  r.Y = point.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

Rect Rect::MinAndMax(Vec2Param min, Vec2Param max)
{
  Rect r;
  r.X = min.x;
  r.Y = min.y;
  r.SizeX = (max.x - min.x);
  r.SizeY = (max.y - min.y);
  return r;
}

void Rect::SetTranslation(Vec2Param translation)
{
  X = translation.x;
  Y = translation.y;
}

void Rect::SetSize(Vec2Param size)
{
  SizeX = size.x;
  SizeY = size.y;
}

void Rect::Expand(const Rect& other)
{
  float nx = Math::Min(X, other.X);
  float ny = Math::Min(Y, other.Y);
  SizeX = Math::Max(Right(), other.Right()) - nx;
  SizeY = Math::Max(Bottom(), other.Bottom()) - ny;
  X = nx;
  Y = ny;
}

bool Rect::Contains(Vec2Param point) const
{
  if(point.x < X) return false;
  if(point.x > X + SizeX) return false;
  if(point.y < Y) return false;
  if(point.y > Y + SizeY) return false;
  return true;
}

bool Rect::Overlap(const Rect& other) const
{
  bool nooverlap = (X > other.X + other.SizeX) || 
                   (Y > other.Y + other.SizeY) ||
                   (other.X > X + SizeX) || 
                   (other.Y > Y + SizeY);

  return !nooverlap;
}

void Rect::RemoveThickness(const Thickness& thickness)
{
  X += thickness.Left;
  Y += thickness.Top;
  SizeX -= (thickness.Left + thickness.Right);
  SizeY -= (thickness.Top + thickness.Bottom);
}

void PlaceWithRect(const Rect& rect, Widget* widget)
{
  widget->SetTranslation(ToVector3(rect.TopLeft()));
  widget->SetSize(rect.GetSize());
}

void PlaceCenterToRect(const Rect& rect, Widget* widget, Vec2Param offset)
{
  Vec2 translation = rect.TopLeft();
  if(widget->mOrigin == DisplayOrigin::Center)
    translation += rect.GetSize() * 0.5f;
  else
    translation += (rect.GetSize() * 0.5f) - (widget->GetSize() * 0.5f);
  widget->SetTranslation(ToVector3(translation + offset));
}

Vec3 SnapToPixels(Vec3Param v)
{
  return Vec3(SnapToPixels(v.x), SnapToPixels(v.y), SnapToPixels(v.z));
}

Vec2 SnapToPixels(Vec2Param v)
{
  return Vec2(SnapToPixels(v.x), SnapToPixels(v.y));
}

float SnapToPixels(float v)
{
  return Math::Floor(v + 0.5f);
}

void Build2dTransform(Mat4& m, Vec3Param t, float r)
{
  float cos = Math::Cos(r);
  float sin = Math::Sin(r);

  m.m00 = cos;
  m.m01 = sin;
  m.m02 = 0.0f;
  m.m03 = 0.0f;

  m.m10 = -sin;
  m.m11 = cos;
  m.m12 = 0.0f;
  m.m13 = 0.0f;

  m.m20 = 0.0f;
  m.m21 = 0.0f;
  m.m22 = 1.0f;
  m.m23 = 0.0f;

  m.m30 = t.x;
  m.m31 = t.y;
  m.m32 = t.z;
  m.m33 = 1.0f;
}

Mat4 Invert2D(Mat4Param mat)
{
  Mat4 inverted;
  inverted.SetIdentity();

  float x = mat.m30;
  float y = mat.m31;
  float det = mat.m00 * mat.m11 - mat.m01 * mat.m10;
  if (det == 0.0f)
  {
    inverted.m30 = -x;
    inverted.m31 = -y;
  }
  else
  {
    float invDet = 1.0f / det;
    inverted.m00 = mat.m11 * invDet;
    inverted.m01 = -mat.m01 * invDet;
    inverted.m11 = mat.m00 * invDet;
    inverted.m10 = -mat.m10 * invDet;
    inverted.m30 = ((mat.m10 * y) - (x * mat.m11)) * invDet;
    inverted.m31 = ((x * mat.m01) - (mat.m00 * y)) * invDet;
  }
  return inverted;
}


Vec2 ExpandSizeByThickness(Thickness thickness, Vec2Param size)
{
  Vec2 addBorder(thickness.Left + thickness.Right,
               thickness.Top + thickness.Bottom);
  return addBorder + size;
}

Rect RemoveThicknessRect(Thickness thickness, Vec2Param size)
{
  Rect r;
  r.SizeX = size.x - (thickness.Left + thickness.Right);
  r.SizeY = size.y - (thickness.Top + thickness.Bottom);
  r.X = thickness.Left;
  r.Y = thickness.Top;
  return r;
}

}
