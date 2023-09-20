// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Area;

namespace Events
{
DeclareEvent(AreaChanged);
}

/// Sent when an area component's size or origin changes.
class AreaEvent : public Event
{
public:
  RaverieDeclareType(AreaEvent, TypeCopyMode::ReferenceType);

  /// The area component that triggered this event.
  Area* mArea;
};

class Area : public Component
{
public:
  RaverieDeclareType(Area, TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void SetDefaults() override;
  void DebugDraw() override;

  /// Location of the Origin of the Area
  Location::Enum GetOrigin();
  void SetOrigin(Location::Enum type);

  /// Size of the Area
  Vec2 GetSize();
  void SetSize(Vec2 newSize);

  /// Local space Aabb for the area
  Aabb GetLocalAabb();
  /// World space Aabb for the area
  Aabb GetAabb();

  /// Offset of the given location in local space
  Vec2 LocalOffsetOf(Location::Enum location);
  Vec2 OffsetOfOffset(Location::Enum location);

  // Interanls
  Vec2 GetLocalTranslation();
  void SetLocalTranslation(Vec2Param translation);
  Vec2 GetWorldTranslation();
  void SetWorldTranslation(Vec2Param worldTranslation);

  /// Rectangle representing the area relative to parent.
  Rectangle GetLocalRectangle();
  void SetLocalRectangle(RectangleParam rectangle);

  /// Rectangle representing the area in world space.
  Rectangle GetWorldRectangle();
  void SetWorldRectangle(RectangleParam rectangle);

  Vec2 GetLocalLocation(Location::Enum location);
  void SetLocalLocation(Location::Enum location, Vec2Param localTranslation);
  Vec2 GetWorldLocation(Location::Enum location);
  void SetWorldLocation(Location::Enum location, Vec2Param worldTranslation);

#define OffsetOfGetter(location)                                                                                                                                                                       \
  Vec2 Get##location()                                                                                                                                                                                 \
  {                                                                                                                                                                                                    \
    return LocalOffsetOf(Location::location);                                                                                                                                                          \
  }
  OffsetOfGetter(TopLeft) OffsetOfGetter(TopCenter) OffsetOfGetter(TopRight) OffsetOfGetter(CenterLeft) OffsetOfGetter(Center) OffsetOfGetter(CenterRight) OffsetOfGetter(BottomLeft)
      OffsetOfGetter(BottomCenter) OffsetOfGetter(BottomRight)
#undef OffsetOfGetter

#define LocationGetterSetter(location)                                                                                                                                                                 \
  Vec2 GetLocal##location()                                                                                                                                                                            \
  {                                                                                                                                                                                                    \
    return GetLocalLocation(Location::location);                                                                                                                                                       \
  }                                                                                                                                                                                                    \
  void SetLocal##location(Vec2Param localTranslation)                                                                                                                                                  \
  {                                                                                                                                                                                                    \
    SetLocalLocation(Location::location, localTranslation);                                                                                                                                            \
  }                                                                                                                                                                                                    \
  Vec2 GetWorld##location()                                                                                                                                                                            \
  {                                                                                                                                                                                                    \
    return GetWorldLocation(Location::location);                                                                                                                                                       \
  }                                                                                                                                                                                                    \
  void SetWorld##location(Vec2Param worldTranslation)                                                                                                                                                  \
  {                                                                                                                                                                                                    \
    SetWorldLocation(Location::location, worldTranslation);                                                                                                                                            \
  }

          LocationGetterSetter(TopLeft) LocationGetterSetter(TopCenter) LocationGetterSetter(TopRight) LocationGetterSetter(CenterLeft) LocationGetterSetter(Center) LocationGetterSetter(CenterRight)
              LocationGetterSetter(BottomLeft) LocationGetterSetter(BottomCenter) LocationGetterSetter(BottomRight)
#undef LocationGetterSetter

                  float GetLocalTop();
  void SetLocalTop(float localTop);
  float GetWorldTop();
  void SetWorldTop(float worldTop);
  float GetLocalRight();
  void SetLocalRight(float localRight);
  float GetWorldRight();
  void SetWorldRight(float worldRight);
  float GetLocalBottom();
  void SetLocalBottom(float localBottom);
  float GetWorldBottom();
  void SetWorldBottom(float worldBottom);
  float GetLocalLeft();
  void SetLocalLeft(float localLeft);
  float GetWorldLeft();
  void SetWorldLeft(float worldLeft);

  // Internals
  void DoAreaChanged();
  Vec2 mSize;
  Location::Enum mOrigin;
  Transform* mTransform;
};

} // namespace Raverie
