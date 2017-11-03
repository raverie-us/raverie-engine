////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

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
typedef const Rectangle& UiRectParam;

struct Rectangle
{
  ZilchDeclareType(TypeCopyMode::ValueType);

  static Rectangle PointAndSize(Vec2Param point, Vec2Param size);
  static Rectangle CenterAndSize(Vec2Param point, Vec2Param size);
  static Rectangle MinAndMax(Vec2Param min, Vec2Param max);

  bool operator==(UiRectParam rhs) const;

  Vec2 GetTranslation() const;
  void SetTranslation(Vec2Param translation);

  Vec2 GetSize() const;
  void SetSize(Vec2Param size);

  bool Contains(Vec2Param point) const;
  bool Overlap(UiRectParam other) const;

  void RemoveThickness(const Thickness& thickness);

  Vec2 GetTopLeft() const { return Vec2(X, Y + SizeY); }
  Vec2 GetTopRight() const { return Vec2(X + SizeX, Y + SizeY); }
  Vec2 GetBottomLeft() const { return Vec2(X, Y); }
  Vec2 GetBottomRight() const { return Vec2(X + SizeX, Y); }
  Vec2 GetCenter() const { return Vec2(X + SizeX * 0.5f, Y + SizeY * 0.5f); }

  float GetLeft() const { return X; }
  float GetRight() const { return X + SizeX; }
  float GetTop() const { return Y + SizeY; }
  float GetBottom() const { return Y; }

  float X;
  float Y;
  float SizeX;
  float SizeY;
  static const Rectangle cZero;
};

}//namespace Zero
