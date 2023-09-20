// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Location
{

bool IsCardinal(Location::Enum location)
{
  switch (location)
  {
  case Location::CenterLeft:
  case Location::TopCenter:
  case Location::CenterRight:
  case Location::BottomCenter:
    return true;
  default:
    return false;
  }
}

int GetCardinalAxis(Location::Enum location)
{
  switch (location)
  {
  case Location::CenterLeft:
  case Location::CenterRight:
    return 0;
  case Location::TopCenter:
  case Location::BottomCenter:
    return 1;
  default:
    DoNotifyException("Location not supported.", "The given location is not a cardinal axis");
    return -1;
  }
}

Vec2 GetDirection(Location::Enum location)
{
  switch (location)
  {
  case Location::TopLeft:
    return Vec2(-0.5f, 0.5f);
  case Location::TopCenter:
    return Vec2(0, 0.5f);
  case Location::TopRight:
    return Vec2(0.5f, 0.5f);

  case Location::CenterLeft:
    return Vec2(-0.5f, 0);
  case Location::CenterRight:
    return Vec2(0.5f, 0);
  case Location::Center:
  default:
    return Vec2(0, 0);

  case Location::BottomLeft:
    return Vec2(-0.5f, -0.5f);
  case Location::BottomCenter:
    return Vec2(0, -0.5f);
  case Location::BottomRight:
    return Vec2(0.5f, -0.5f);
  }
}

Vec2 GetDirection(Location::Enum from, Location::Enum to)
{
  Vec2 centerToFrom = GetDirection(from);
  Vec2 centerToTo = GetDirection(to);
  return centerToTo - centerToFrom;
}

Location::Enum GetOpposite(Location::Enum location)
{
  switch (location)
  {
  case Location::TopLeft:
    return Location::BottomRight;
  case Location::TopCenter:
    return Location::BottomCenter;
  case Location::TopRight:
    return Location::BottomLeft;

  case Location::CenterLeft:
    return Location::CenterRight;
  case Location::CenterRight:
    return Location::CenterLeft;
  case Location::Center:
  default:
    return Location::Center;

  case Location::BottomLeft:
    return Location::TopRight;
  case Location::BottomCenter:
    return Location::TopCenter;
  case Location::BottomRight:
    return Location::TopLeft;
  }
}

} // namespace Location

// Thickness
const Thickness Thickness::cZero(0, 0, 0, 0);

RaverieDefineType(Thickness, builder, type)
{
  type->CreatableInScript = true;

  RaverieBindDestructor();
  RaverieBindConstructor(float, float, float, float);
  RaverieBindConstructor(Vec4);
  RaverieBindConstructor(float, float);
  RaverieBindConstructor(Vec2);

  RaverieBindMethod(All);

  RaverieBindFieldProperty(Left);
  RaverieBindFieldProperty(Top);
  RaverieBindFieldProperty(Right);
  RaverieBindFieldProperty(Bottom);
  RaverieBindFieldGetter(cZero);

  RaverieBindMethod(Size);
  RaverieBindMethod(TopLeft);
}

Thickness::Thickness() : Left(0), Top(0), Right(0), Bottom(0)
{
}

Thickness::Thickness(float splat) : Left(splat), Top(splat), Right(splat), Bottom(splat)
{
}

Thickness::Thickness(float left, float top, float right, float bottom) :
    Left(left),
    Top(top),
    Right(right),
    Bottom(bottom)
{
}

Thickness::Thickness(Vec4 vector) : Left(vector.x), Top(vector.y), Right(vector.z), Bottom(vector.w)
{
}

Thickness::Thickness(float leftRight, float topBottom) :
    Left(leftRight),
    Top(topBottom),
    Right(leftRight),
    Bottom(topBottom)
{
}

Thickness::Thickness(Vec2 vector) : Left(vector.x), Top(vector.y), Right(vector.x), Bottom(vector.y)
{
}

Thickness Thickness::All(float amount)
{
  return Thickness(amount, amount, amount, amount);
}

Thickness Thickness::operator+(const Thickness& rhs)
{
  return Thickness(Left + rhs.Left, Top + rhs.Top, Right + rhs.Right, Bottom + rhs.Bottom);
}

// Ui Rect
const Rectangle Rectangle::cZero = Rectangle::CenterAndSize(Vec2(0, 0), Vec2(0, 0));

RaverieDefineType(Rectangle, builder, type)
{
  type->CreatableInScript = true;

  RaverieBindFieldProperty(Min);
  RaverieBindFieldProperty(Max);
  RaverieBindGetterProperty(Size);
  RaverieBindMethod(SetSize);
  RaverieBindOverloadedMethod(ResizeToPoint, RaverieInstanceOverload(void, Location::Enum, float));
  RaverieBindOverloadedMethod(ResizeToPoint, RaverieInstanceOverload(void, Location::Enum, Vec2Param));
  RaverieBindOverloadedMethod(ResizeToPoint, RaverieInstanceOverload(void, Location::Enum, Vec2Param, Vec2Param));
  RaverieBindMethod(Expand);

  RaverieBindOverloadedMethod(Transform, RaverieInstanceOverload(void, Mat2Param));
  RaverieBindOverloadedMethod(Transform, RaverieInstanceOverload(void, Mat3Param));
  RaverieBindOverloadedMethod(Transform, RaverieInstanceOverload(void, Mat4Param));
  RaverieBindOverloadedMethod(Transformed, RaverieConstInstanceOverload(Rectangle, Mat2Param));
  RaverieBindOverloadedMethod(Transformed, RaverieConstInstanceOverload(Rectangle, Mat3Param));
  RaverieBindOverloadedMethod(Transformed, RaverieConstInstanceOverload(Rectangle, Mat4Param));

  RaverieBindGetterSetterProperty(TopLeft);
  RaverieBindGetterSetterProperty(TopRight);
  RaverieBindGetterSetterProperty(BottomLeft);
  RaverieBindGetterSetterProperty(BottomRight);
  RaverieBindGetterSetterProperty(Center);

  RaverieBindGetterSetterProperty(Left);
  RaverieBindGetterSetterProperty(Right);
  RaverieBindGetterSetterProperty(Top);
  RaverieBindGetterSetterProperty(Bottom);

  RaverieBindOverloadedMethod(Contains, RaverieConstInstanceOverload(bool, Vec2Param));
  RaverieBindOverloadedMethod(Contains, RaverieConstInstanceOverload(bool, RectangleParam));
  RaverieBindMethodAs(Overlap, "Overlaps");

  Raverie::Function* overlapFn = RaverieBindMethodAs(Overlap, "Overlap");
  overlapFn->Description = "This function is deprecated. Use Overlaps instead";
  overlapFn->AddAttribute(DeprecatedAttribute);

  RaverieBindMethod(RemoveThickness);

  RaverieBindMethod(GetCardinalLocation);
  RaverieBindOverloadedMethod(SetLocation, RaverieInstanceOverload(void, Location::Enum, float));
  RaverieBindMethod(GetLocation);
  RaverieBindOverloadedMethod(SetLocation, RaverieInstanceOverload(void, Location::Enum, Vec2Param));

  type->ToStringFunction = Raverie::BoundTypeToGlobalToString<Rectangle>;
}

Rectangle Rectangle::PointAndSize(Vec2Param point, Vec2Param size)
{
  return MinAndMax(point, point + size);
}

Rectangle Rectangle::CenterAndSize(Vec2Param point, Vec2Param size)
{
  Vec2 halfSize = size * 0.5f;

  return MinAndMax(point - halfSize, point + halfSize);
}

Rectangle Rectangle::MinAndMax(Vec2Param min, Vec2Param max)
{
  Rectangle r;
  r.Min = min;
  r.Max = max;
  return r;
}

Rectangle Rectangle::MinAndMax(Vec3Param min, Vec3Param max)
{
  Rectangle r;
  r.Min = Vec2(min.x, min.y);
  r.Max = Vec2(max.x, max.y);
  return r;
}

bool Rectangle::operator==(RectangleParam rhs) const
{
  return (Min == rhs.Min) && (Max == rhs.Max);
}

void Rectangle::Translate(Vec2Param translation)
{
  Min += translation;
  Max += translation;
}

void Rectangle::Transform(Mat2Param transform)
{
  Mat3 mat3(transform.m00, transform.m01, 0, transform.m10, transform.m11, 0, 0, 0, 1);

  Transform(mat3);
}

void Rectangle::Transform(Mat3Param transform)
{
  Mat2 rotationScale = Math::ToMatrix2(transform);
  for (size_t y = 0; y < 2; ++y)
    for (size_t x = 0; x < 2; ++x)
      rotationScale[y][x] = Math::Abs(rotationScale[y][x]);

  Vec2 center = GetCenter();
  Vec2 halfExtent = GetSize() * 0.5f;

  halfExtent = rotationScale.Transform(halfExtent);
  center = Math::TransformPoint(transform, center);

  Min = center - halfExtent;
  Max = center + halfExtent;
}

void Rectangle::Transform(Mat4Param transform)
{
  Aabb aabb;
  aabb.SetMinAndMax(Vec3(Min), Vec3(Max));
  aabb = aabb.TransformAabb(transform);
  Min = ToVector2(aabb.mMin);
  Max = ToVector2(aabb.mMax);
}

Rectangle Rectangle::Transformed(Mat2Param transform) const
{
  Rectangle other = *this;
  other.Transform(transform);
  return other;
}

Rectangle Rectangle::Transformed(Mat3Param transform) const
{
  Rectangle other = *this;
  other.Transform(transform);
  return other;
}

Rectangle Rectangle::Transformed(Mat4Param transform) const
{
  Rectangle other = *this;
  other.Transform(transform);
  return other;
}

Vec2 Rectangle::GetSize() const
{
  return Max - Min;
}

void Rectangle::SetSize(Location::Enum origin, Vec2Param size)
{
  switch (origin)
  {
  case Location::TopLeft:
  {
    Min.y = Max.y - size.y;
    Max.x = Min.x + size.x;
    return;
  }
  case Location::TopRight:
  {
    Min = Max - size;
    return;
  }
  case Location::BottomLeft:
  {
    Max = Min + size;
    return;
  }
  case Location::BottomRight:
  {
    Min.x = Max.x - size.x;
    Max.y = Min.y + size.y;
    return;
  }
  case Location::CenterLeft:
  {
    Max.x = Min.x + size.x;
    return;
  }
  case Location::TopCenter:
  {
    Min.y = Max.y - size.y;
    return;
  }
  case Location::CenterRight:
  {
    Min.x = Max.x - size.x;
    return;
  }
  case Location::BottomCenter:
  {
    Max.y = Min.y + size.y;
    return;
  }
  case Location::Center:
  {
    Vec2 center = GetCenter();
    Vec2 halfSize = size * 0.5f;
    Min = center - halfSize;
    Max = center + halfSize;
    return;
  }
  }
}

void Rectangle::ResizeToPoint(Location::Enum location, float position)
{
  switch (location)
  {
  case Location::CenterLeft:
  case Location::TopCenter:
  case Location::CenterRight:
  case Location::BottomCenter:
  {
    Vec2 position2 = Vec2::cZero;
    int axis = Location::GetCardinalAxis(location);
    position2[axis] = position;
    ResizeToPoint(location, position2);
    break;
  }
  default:
  {
    DoNotifyException("Location not supported.", "The given location is not a cardinal axis");
  }
  }
}

void Rectangle::ResizeToPoint(Location::Enum location, Vec2Param position)
{
  ResizeToPoint(location, position, Vec2::cZero);
}

void Rectangle::ResizeToPoint(Location::Enum location, Vec2Param position, Vec2Param minSize)
{
  switch (location)
  {
  case Location::TopLeft:
  {
    Min.x = Math::Min(position.x, Max.x - minSize.x);
    Max.y = Math::Max(position.y, Min.y + minSize.y);
    return;
  }
  case Location::TopRight:
  {
    Max = Math::Max(position, Min + minSize);
    return;
  }
  case Location::BottomLeft:
  {
    Min = Math::Min(position, Max - minSize);
    return;
  }
  case Location::BottomRight:
  {
    Min.y = Math::Min(position.y, Max.y - minSize.y);
    Max.x = Math::Max(position.x, Min.x + minSize.x);
    return;
  }
  case Location::CenterLeft:
  {
    Min.x = Math::Min(position.x, Max.x - minSize.x);
    return;
  }
  case Location::TopCenter:
  {
    Max.y = Math::Max(position.y, Min.y + minSize.y);
    return;
  }
  case Location::CenterRight:
  {
    Max.x = Math::Max(position.x, Min.x + minSize.x);
    return;
  }
  case Location::BottomCenter:
  {
    Min.y = Math::Min(position.y, Max.y - minSize.y);
    return;
  }
  case Location::Center:
  {
    DoNotifyException("Location not supported.",
                      "Location.Center is not implemented as it doesn't make "
                      "sense in this context.");
  }
  }
}

void Rectangle::Expand(Vec2Param point)
{
  Min.x = Math::Min(Min.x, point.x);
  Min.y = Math::Min(Min.y, point.y);

  Max.x = Math::Max(Max.x, point.x);
  Max.y = Math::Max(Max.y, point.y);
}

bool Rectangle::Contains(Vec2Param point) const
{
  if (point.x < Min.x)
    return false;
  if (point.x > Max.x)
    return false;
  if (point.y < Min.y)
    return false;
  if (point.y > Max.y)
    return false;
  return true;
}

bool Rectangle::Contains(RectangleParam other) const
{
  return (Min.x <= other.Min.x) && (Min.y <= other.Min.y) && (Max.x >= other.Max.x) && (Max.y >= other.Max.y);
}

bool Rectangle::Overlap(RectangleParam other) const
{
  bool noOverlap = (Min.x > other.Max.x) || (Min.y > other.Max.y) || (other.Min.x > Max.x) || (other.Min.y > Max.y);

  return !noOverlap;
}

void Rectangle::RemoveThickness(const Thickness& thickness)
{
  Min.x += thickness.Left;
  Min.y += thickness.Bottom;
  Max.x -= thickness.Right;
  Max.y -= thickness.Top;
}

Vec2 Rectangle::GetTopLeft() const
{
  return Vec2(Min.x, Max.y);
}

void Rectangle::SetTopLeft(Vec2Param point)
{
  Translate(point - GetTopLeft());
}

Vec2 Rectangle::GetTopRight() const
{
  return Max;
}

void Rectangle::SetTopRight(Vec2Param point)
{
  Translate(point - GetTopRight());
}

Vec2 Rectangle::GetBottomLeft() const
{
  return Min;
}

void Rectangle::SetBottomLeft(Vec2Param point)
{
  Translate(point - GetBottomLeft());
}

Vec2 Rectangle::GetBottomRight() const
{
  return Vec2(Max.x, Min.y);
}

void Rectangle::SetBottomRight(Vec2Param point)
{
  Translate(point - GetBottomRight());
}

Vec2 Rectangle::GetCenter() const
{
  return (Min + Max) * 0.5;
}

void Rectangle::SetCenter(Vec2Param point)
{
  Translate(point - GetCenter());
}

float Rectangle::GetLeft() const
{
  return Min.x;
}

void Rectangle::SetLeft(float left)
{
  Translate(Vec2(left - GetLeft(), 0));
}

float Rectangle::GetRight() const
{
  return Max.x;
}

void Rectangle::SetRight(float right)
{
  Translate(Vec2(right - GetRight(), 0));
}

float Rectangle::GetTop() const
{
  return Max.y;
}

void Rectangle::SetTop(float top)
{
  Translate(Vec2(0, top - GetTop()));
}

float Rectangle::GetBottom() const
{
  return Min.y;
}

void Rectangle::SetBottom(float bottom)
{
  Translate(Vec2(0, bottom - GetBottom()));
}

float Rectangle::GetCardinalLocation(Location::Enum location)
{
  switch (location)
  {
  case Location::CenterLeft:
    return GetLeft();
  case Location::TopCenter:
    return GetTop();
  case Location::CenterRight:
    return GetRight();
  case Location::BottomCenter:
    return GetBottom();
  default:
    DoNotifyException("Location not supported.", "Only cardinal axes are supported.");
  }

  return 0.0f;
}

void Rectangle::SetLocation(Location::Enum location, float value)
{
  switch (location)
  {
  case Location::CenterLeft:
  {
    SetLeft(value);
    break;
  }
  case Location::TopCenter:
  {
    SetTop(value);
    break;
  }
  case Location::CenterRight:
  {
    SetRight(value);
    break;
  }
  case Location::BottomCenter:
  {
    SetBottom(value);
    break;
  }
  default:
    DoNotifyException("Location not supported.", "Only cardinal axes are supported.");
  }
}

Vec2 Rectangle::GetLocation(Location::Enum location) const
{
  Vec2 bottomLeft = GetBottomLeft();
  Vec2 size = GetSize();
  Vec2 toLocation = Location::GetDirection(Location::BottomLeft, location);
  return bottomLeft + (toLocation * size);
}

void Rectangle::SetLocation(Location::Enum location, Vec2Param value)
{
  switch (location)
  {
  case Location::TopLeft:
  {
    SetTopLeft(value);
    break;
  }
  case Location::TopRight:
  {
    SetTopRight(value);
    break;
  }
  case Location::BottomRight:
  {
    SetBottomRight(value);
    break;
  }
  case Location::BottomLeft:
  {
    SetBottomLeft(value);
    break;
  }
  case Location::Center:
  {
    SetCenter(value);
    break;
  }
  default:
    DoNotifyException("Location not supported.", "Only non-cardinal axes are supported.");
  }
}

String ToString(const Rectangle& value, bool shortFormat)
{
  String minStr = ToString(value.Min, shortFormat);
  String maxStr = ToString(value.Max, shortFormat);
  return String::Format("(%s), (%s)", minStr.c_str(), maxStr.c_str());
}

} // namespace Raverie
