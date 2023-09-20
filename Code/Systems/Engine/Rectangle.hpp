// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Location
DeclareEnum9(Location, TopLeft, TopCenter, TopRight, CenterLeft, Center, CenterRight, BottomLeft, BottomCenter, BottomRight);

// Extensions
namespace Location
{
// Returns whether or not the given location is a cardinal direction
// (CenterLeft, TopCenter, CenterRight, BottomCenter).
bool IsCardinal(Location::Enum location);

// Returns the axis of the given location. This is only valid for the cardinal
// directions (CenterLeft, TopCenter, CenterRight, BottomCenter).
int GetCardinalAxis(Location::Enum location);

// Returns the direction of the given location from the center.
Vec2 GetDirection(Location::Enum location);

// Returns the direction from 'from' to 'to'.
Vec2 GetDirection(Location::Enum from, Location::Enum to);

// Returns the location on the opposite side of the given location.
Location::Enum GetOpposite(Location::Enum location);
} // namespace Location

// Thickness
struct Thickness
{
  RaverieDeclareType(Thickness, TypeCopyMode::ValueType);

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

  Vec2 Size() const
  {
    return Vec2(Width(), Height());
  }
  Vec2 SizeX() const
  {
    return Vec2(Left, Right);
  }
  Vec2 SizeY() const
  {
    return Vec2(Top, Bottom);
  }
  Vec2 TopLeft() const
  {
    return Vec2(Left, Top);
  }
  Vec2 TopRight() const
  {
    return Vec2(Right, Top);
  }
  Vec2 BottomLeft() const
  {
    return Vec2(Left, Bottom);
  }
  Vec2 BottomRight() const
  {
    return Vec2(Right, Bottom);
  }
  float Width() const
  {
    return Left + Right;
  }
  float Height() const
  {
    return Top + Bottom;
  }

  static const Thickness cZero;
};

// Rectangle
struct Rectangle;
typedef const Rectangle& RectangleParam;

/// Two dimensional, unrotated rectangle.
struct Rectangle
{
  RaverieDeclareType(Rectangle, TypeCopyMode::ValueType);

  static Rectangle PointAndSize(Vec2Param point, Vec2Param size);
  static Rectangle CenterAndSize(Vec2Param point, Vec2Param size);
  static Rectangle MinAndMax(Vec2Param min, Vec2Param max);
  static Rectangle MinAndMax(Vec3Param min, Vec3Param max);

  bool operator==(RectangleParam rhs) const;

  /// Translates the rectangle by the passed in vector.
  void Translate(Vec2Param translation);

  /// Applies transformation to the Rectangle. Note that Rectangle is
  /// non-rotated, so this will result in the Aabb around the rotated rectangle.
  void Transform(Mat2Param transform);

  /// Applies transformation to the Rectangle. Note that Rectangle is
  /// non-rotated, so this will result in the Aabb around the rotated rectangle.
  /// The given transform is assumed to be a 2D transformation.
  void Transform(Mat3Param transform);

  /// Takes a full 3D transformation matrix and brings it down to a 2D matrix.
  /// Applies transformation to the Rectangle. Note that Rectangle is
  /// non-rotated, so this will result in the Aabb around the rotated rectangle.
  void Transform(Mat4Param transform);

  /// Returns a copy of ourself transformed by the given matrix. Note that
  /// Rectangle is non-rotated, so this will result in the Aabb around the
  /// rotated rectangle.
  Rectangle Transformed(Mat2Param transform) const;

  /// Returns a copy of ourself transformed by the given matrix. Note that
  /// Rectangle is non-rotated, so this will result in the Aabb around the
  /// rotated rectangle. The given transform is assumed to be a 2D
  /// transformation.
  Rectangle Transformed(Mat3Param transform) const;

  /// Takes a full 3D transformation matrix and brings it down to a 2D matrix.
  Rectangle Transformed(Mat4Param transform) const;

  Vec2 GetSize() const;
  void SetSize(Location::Enum origin, Vec2Param size);
  void ResizeToPoint(Location::Enum location, float position);
  void ResizeToPoint(Location::Enum location, Vec2Param position);
  void ResizeToPoint(Location::Enum location, Vec2Param position, Vec2Param minSize);

  void Expand(Vec2Param point);

  /// Returns true if the given point is inside the Rectangle.
  bool Contains(Vec2Param point) const;

  /// Returns true if this Rectangle completely contains the given Rectangle.
  bool Contains(RectangleParam other) const;

  /// Returns true if this Rectangle and the given Rectangle overlap in any way.
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

  float GetCardinalLocation(Location::Enum location);
  void SetLocation(Location::Enum location, float value);
  Vec2 GetLocation(Location::Enum location) const;
  void SetLocation(Location::Enum location, Vec2Param value);

  Vec2 Min;
  Vec2 Max;
  static const Rectangle cZero;
};

String ToString(const Rectangle& value, bool shortFormat = false);

} // namespace Raverie
