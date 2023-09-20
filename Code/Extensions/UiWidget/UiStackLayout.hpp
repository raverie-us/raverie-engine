// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Direction of
DeclareEnum4(UiStackLayoutDirection, TopToBottom, BottomToTop, LeftToRight, RightToLeft);

class UiStackLayout : public UiLayout
{
public:
  /// Meta Initialization.
  RaverieDeclareType(UiStackLayout, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// UiLayout Interface.
  Vec2 Measure(Rectangle& rect) override;
  void DoLayout(Rectangle& rect, UiTransformUpdateEvent* e) override;

  float ComputeFlexRatio(float fixedSize, float totalFlex, float flexMinSize, float totalSize);

  /// The stack direction the child Widgets will be placed in.
  UiStackLayoutDirection::Enum GetStackDirection();
  void SetStackDirection(UiStackLayoutDirection::Enum direction);

  /// The amount of space, in pixels, between each child Widget.
  Vec2 GetSpacing();
  void SetSpacing(Vec2Param spacing);

private:
  UiStackLayoutDirection::Enum mStackDirection;
  Vec2 mSpacing;
};

} // namespace Raverie
