// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Widget;

namespace SlicesIndex
{
enum Enum
{
  Left,
  Top,
  Right,
  Bottom,
};
} // namespace SlicesIndex

DeclareEnum3(VerticalAlignment, Top, Bottom, Center);
DeclareEnum3(HorizontalAlignment, Left, Right, Center);

struct WidgetRect;
typedef const WidgetRect& RectParam;

struct WidgetRect
{
  static const WidgetRect cZero;

  static WidgetRect PointAndSize(Vec2Param point, Vec2Param size);
  static WidgetRect CenterAndSize(Vec2Param point, Vec2Param size);
  static WidgetRect MinAndMax(Vec2Param min, Vec2Param max);

  float X;
  float Y;
  float SizeX;
  float SizeY;

  bool operator==(RectParam rhs) const;

  Vec2 GetPosition() const
  {
    return Vec2(X, Y);
  }
  Vec2 GetSize() const
  {
    return Vec2(SizeX, SizeY);
  }

  void SetTranslation(Vec2Param translation);
  void SetSize(Vec2Param size);

  void Expand(const WidgetRect& other);
  bool Contains(Vec2Param point) const;
  bool Overlap(const WidgetRect& other) const;

  void RemoveThickness(const Thickness& thickness);

  Vec2 TopLeft() const
  {
    return Vec2(X, Y);
  }
  Vec2 TopRight() const
  {
    return Vec2(X + SizeX, Y);
  }
  Vec2 BottomLeft() const
  {
    return Vec2(X, Y + SizeY);
  }
  Vec2 BottomRight() const
  {
    return Vec2(X + SizeX, Y + SizeY);
  }
  Vec2 Center() const
  {
    return Vec2(X + SizeX * 0.5f, Y + SizeY * 0.5f);
  }

  float Left() const
  {
    return X;
  }
  float Right() const
  {
    return X + SizeX;
  }
  float Top() const
  {
    return Y;
  }
  float Bottom() const
  {
    return Y + SizeY;
  }
};

void PlaceWithRect(const WidgetRect& rect, Widget* widget);
void PlaceCenterToRect(const WidgetRect& rect, Widget* widget, Vec2Param offset = Vec2::cZero);

Mat4 Invert2D(Mat4Param matrix);
void Build2dTransform(Mat4& matrix, Vec3Param translation, float rotation);

float SnapToPixels(float v);
Vec2 SnapToPixels(Vec2Param v);
Vec3 SnapToPixels(Vec3Param v);

inline float Pixels(float x)
{
  return x;
}
inline Vec2 Pixels(float x, float y)
{
  return Vec2(x, y);
}
inline Vec3 Pixels(float x, float y, float z)
{
  return Vec3(x, y, z);
}
inline Vec4 Pixels(float x, float y, float z, float w)
{
  return Vec4(x, y, z, w);
}

// Utility functions
Vec2 ExpandSizeByThickness(Thickness thickness, Vec2Param size);

WidgetRect RemoveThicknessRect(Thickness thickness, Vec2Param outerSize);

} // namespace Raverie
