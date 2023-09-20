// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

const WidgetRect WidgetRect::cZero = WidgetRect::CenterAndSize(Vec2(0, 0), Vec2(0, 0));

WidgetRect WidgetRect::PointAndSize(Vec2Param point, Vec2Param size)
{
  WidgetRect r;
  r.X = point.x;
  r.Y = point.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

WidgetRect WidgetRect::CenterAndSize(Vec2Param point, Vec2Param size)
{
  Vec2 halfSize = size / 2.0f;

  WidgetRect r;
  r.X = point.x - halfSize.x;
  r.Y = point.y - halfSize.y;
  r.SizeX = size.x;
  r.SizeY = size.y;
  return r;
}

WidgetRect WidgetRect::MinAndMax(Vec2Param min, Vec2Param max)
{
  WidgetRect r;
  r.X = min.x;
  r.Y = min.y;
  r.SizeX = (max.x - min.x);
  r.SizeY = (max.y - min.y);
  return r;
}

bool WidgetRect::operator==(RectParam rhs) const
{
  bool result = Math::Abs(X - rhs.X) < Math::Epsilon();
  result &= Math::Abs(Y - rhs.Y) < Math::Epsilon();
  result &= Math::Abs(SizeX - rhs.SizeX) < Math::Epsilon();
  result &= Math::Abs(SizeY - rhs.SizeY) < Math::Epsilon();

  return result;
}

void WidgetRect::SetTranslation(Vec2Param translation)
{
  X = translation.x;
  Y = translation.y;
}

void WidgetRect::SetSize(Vec2Param size)
{
  SizeX = size.x;
  SizeY = size.y;
}

void WidgetRect::Expand(const WidgetRect& other)
{
  float nx = Math::Min(X, other.X);
  float ny = Math::Min(Y, other.Y);
  SizeX = Math::Max(Right(), other.Right()) - nx;
  SizeY = Math::Max(Bottom(), other.Bottom()) - ny;
  X = nx;
  Y = ny;
}

bool WidgetRect::Contains(Vec2Param point) const
{
  if (point.x < X)
    return false;
  if (point.x > X + SizeX)
    return false;
  if (point.y < Y)
    return false;
  if (point.y > Y + SizeY)
    return false;
  return true;
}

bool WidgetRect::Overlap(const WidgetRect& other) const
{
  bool nooverlap = (X > other.X + other.SizeX) || (Y > other.Y + other.SizeY) || (other.X > X + SizeX) || (other.Y > Y + SizeY);

  return !nooverlap;
}

void WidgetRect::RemoveThickness(const Thickness& thickness)
{
  X += thickness.Left;
  Y += thickness.Top;
  SizeX -= (thickness.Left + thickness.Right);
  SizeY -= (thickness.Top + thickness.Bottom);
}

void PlaceWithRect(const WidgetRect& rect, Widget* widget)
{
  widget->SetTranslation(ToVector3(rect.TopLeft()));
  widget->SetSize(rect.GetSize());
}

void PlaceCenterToRect(const WidgetRect& rect, Widget* widget, Vec2Param offset)
{
  Vec2 translation = rect.TopLeft();
  if (widget->mOrigin == DisplayOrigin::Center)
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
  Vec2 addBorder(thickness.Left + thickness.Right, thickness.Top + thickness.Bottom);
  return addBorder + size;
}

WidgetRect RemoveThicknessRect(Thickness thickness, Vec2Param size)
{
  WidgetRect r;
  r.SizeX = size.x - (thickness.Left + thickness.Right);
  r.SizeY = size.y - (thickness.Top + thickness.Bottom);
  r.X = thickness.Left;
  r.Y = thickness.Top;
  return r;
}

} // namespace Raverie
