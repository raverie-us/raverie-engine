///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------- Stack Layout
// Direction of 
DeclareEnum4(UiStackLayoutDirection, TopToBottom, BottomToTop,
                                     LeftToRight, RightToLeft);

class UiStackLayout : public UiLayout
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// UiLayout Interface.
  Vec2 Measure(Rect& rect) override;
  Vec2 DoLayout(Rect& rect, UiTransformUpdateEvent* e) override;

  float ComputeFlexRatio(float fixedSize, float totalFlex, float flexMinSize,
                         float totalSize);

  /// The stack direction the child Widgets will be placed in.
  UiStackLayoutDirection::Enum GetStackDirection();
  void SetStackDirection(UiStackLayoutDirection::Enum direction);

  /// The amount of pixels in between each child Widget.
  Vec2 GetSpacing();
  void SetSpacing(Vec2Param spacing);
  
private:
  UiStackLayoutDirection::Enum mStackDirection;
  Vec2 mSpacing;
};

}//namespace Zero
