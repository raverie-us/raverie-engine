///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------ Dock Layout
class UiDockLayout : public UiLayout
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// UiLayout Interface.
  Vec2 Measure(Rect& rect) override;
  Vec2 DoLayout(Rect& rect, UiTransformUpdateEvent* e) override;

  Vec2 mSpacing;
};

}//namespace Zero
