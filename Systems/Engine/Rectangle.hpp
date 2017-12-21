////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum9(Location, TopLeft,    TopCenter,    TopRight,
                       CenterLeft, Center,       CenterRight,
                       BottomLeft, BottomCenter, BottomRight);

//---------------------------------------------------------------------------------------- Thickness
struct Thickness
{
  ZilchDeclareType(TypeCopyMode::ValueType);

  Thickness();
  Thickness(float splat);

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

  Vec2 Size() const { return Vec2(Width(), Height()); }
  Vec2 TopLeft() const { return Vec2(Left, Top); }
  Vec2 TopRight() const { return Vec2(Right, Top); }
  Vec2 BottomLeft() const { return Vec2(Left, Bottom); }
  Vec2 BottomRight() const { return Vec2(Right, Bottom); }
  float Width() const { return Left + Right; }
  float Height() const { return Top + Bottom; }

  static const Thickness cZero;
};

//---------------------------------------------------------------------------------------- Rectangle
struct Rectangle;
typedef const Rectangle& RectangleParam;

struct Rectangle
{
  ZilchDeclareType(TypeCopyMode::ValueType);

  static Rectangle PointAndSize(Vec2Param point, Vec2Param size);
  static Rectangle CenterAndSize(Vec2Param point, Vec2Param size);
  static Rectangle MinAndMax(Vec2Param min, Vec2Param max);

  bool operator==(RectangleParam rhs) const;
  
  /// Translates the rectangle by the passed in vector.
  void Translate(Vec2Param translation);

  Vec2 GetSize() const;
  void SetSize(Location::Enum origin, Vec2Param size);
  void ResizeToPoint(Location::Enum location, Vec2Param position);
  void ResizeToPoint(Location::Enum location, Vec2Param position, Vec2Param minSize);
  
  void Expand(Vec2Param point);

  bool Contains(Vec2Param point) const;
  bool Overlap(RectangleParam other) const;

  void RemoveThickness(const Thickness& thickness);

  Vec2 GetTopLeft() const;
  void SetTopLeft(Vec2Param point);
  Vec2 GetTopRight() const;
  void SetTopRight(Vec2Param point);
  Vec2 GetBottomLeft() const;
  void SetBottomLeft(Vec2Param point);
  Vec2 GetBottomRight() const;
  void SetBottomRight(Vec2Param point);
  Vec2 GetCenter() const;
  void SetCenter(Vec2Param point);

  float GetLeft() const;
  void SetLeft(float left);
  float GetRight() const;
  void SetRight(float right);
  float GetTop() const;
  void SetTop(float top);
  float GetBottom() const;
  void SetBottom(float bottom);

  Vec2 Min;
  Vec2 Max;
  static const Rectangle cZero;
};

}//namespace Zero
