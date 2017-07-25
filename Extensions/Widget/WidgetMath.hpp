///////////////////////////////////////////////////////////////////////////////
///
/// \file WidgetMath.hpp
///  Declaration of the basic Widget layout helpers.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
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
}//namespace SlicesIndex

DeclareEnum3(VerticalAlignment, Top, Bottom, Center);
DeclareEnum3(HorizontalAlignment, Left, Right, Center);

//---------------------------------------------------------------------- Thickness
struct Thickness
{
  ZilchDeclareType(TypeCopyMode::ValueType);

  Thickness();

  Thickness(float left, float top, float right, float bottom);
  explicit Thickness(Vec4 vector);

  Thickness(float leftRight, float topBottom);
  explicit Thickness(Vec2 vector);

  // Thickness equal to value in all directions
  static Thickness All(float amount);

  // Add Thickness
  Thickness operator+(const Thickness& rhs);

  float Left;
  float Top;
  float Right;
  float Bottom;

  Vec2 Size() const  {return Vec2(Width(), Height()); }
  Vec2 TopLeft() const {return Vec2(Left, Top); }
  Vec2 TopRight() const {return Vec2(Right, Top); }
  Vec2 BottomLeft() const {return Vec2(Left, Bottom); }
  Vec2 BottomRight() const {return Vec2(Right, Bottom); }
  float Width() const {return Left + Right;}
  float Height() const {return Top + Bottom;}

  static const Thickness cZero;
};

//------------------------------------------------------------------------- Rect
struct Rect
{
  ZilchDeclareType(TypeCopyMode::ValueType);

  static Rect PointAndSize(Vec2Param point, Vec2Param size);
  static Rect CenterAndSize(Vec2Param point, Vec2Param size);
  static Rect MinAndMax(Vec2Param min, Vec2Param max);

  float X;
  float Y;
  float SizeX;
  float SizeY;

  Vec2 GetPosition() const { return Vec2(X, Y); }
  Vec2 GetSize() const { return Vec2(SizeX, SizeY); }

  void SetTranslation(Vec2Param translation);
  void SetSize(Vec2Param size);

  void Expand(const Rect& other);
  bool Contains(Vec2Param point) const;
  bool Overlap(const Rect& other) const;

  void RemoveThickness(const Thickness& thickness);

  Vec2 TopLeft() const {return Vec2(X, Y); }
  Vec2 TopRight() const {return Vec2(X + SizeX,Y); }
  Vec2 BottomLeft() const {return Vec2(X, Y + SizeY); }
  Vec2 BottomRight() const {return Vec2(X + SizeX, Y + SizeY); }
  Vec2 Center() const {return Vec2(X + SizeX * 0.5f, Y + SizeY * 0.5f);}

  float Left() const {return X;}
  float Right() const {return X + SizeX;}
  float Top() const {return Y;}
  float Bottom() const {return Y + SizeY;}
};

void PlaceWithRect(const Rect& rect, Widget* widget);
void PlaceCenterToRect(const Rect& rect, Widget* widget, Vec2Param offset = Vec2::cZero);

Mat4 Invert2D(Mat4Param matrix);
void Build2dTransform(Mat4& matrix, Vec3Param translation, float rotation);

float SnapToPixels(float v);
Vec2 SnapToPixels(Vec2Param v);
Vec3 SnapToPixels(Vec3Param v);

inline float Pixels(float x){return x;}
inline Vec2 Pixels(float x, float y){return Vec2(x, y);}
inline Vec3 Pixels(float x, float y, float z){return Vec3(x, y, z);}
inline Vec4 Pixels(float x, float y, float z, float w){return Vec4(x, y, z, w);}

//Utility functions
Vec2 ExpandSizeByThickness(Thickness thickness, Vec2Param size);

Rect RemoveThicknessRect(Thickness thickness, Vec2Param outerSize);

}
