// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class UiDockLayout : public UiLayout
{
public:
  /// Meta Initialization.
  ZilchDeclareType(UiDockLayout, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// UiLayout Interface.
  Vec2 Measure(Rectangle& rect) override;
  void DoLayout(Rectangle& rect, UiTransformUpdateEvent* e) override;

  Vec2 mSpacing;
};

} // namespace Zero
