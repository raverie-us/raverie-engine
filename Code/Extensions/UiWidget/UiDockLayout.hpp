// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class UiDockLayout : public UiLayout
{
public:
  /// Meta Initialization.
  RaverieDeclareType(UiDockLayout, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// UiLayout Interface.
  Vec2 Measure(Rectangle& rect) override;
  void DoLayout(Rectangle& rect, UiTransformUpdateEvent* e) override;

  Vec2 mSpacing;
};

} // namespace Raverie
