////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------------------------ Ui Rect
struct UiRect;
typedef const UiRect& UiRectParam;

struct UiRect
{
  ZilchDeclareType(TypeCopyMode::ValueType);

  static UiRect PointAndSize(Vec2Param point, Vec2Param size);
  static UiRect CenterAndSize(Vec2Param point, Vec2Param size);
  static UiRect MinAndMax(Vec2Param min, Vec2Param max);

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
  static const UiRect cZero;
};

}//namespace Zero
