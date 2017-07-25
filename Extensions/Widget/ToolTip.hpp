///////////////////////////////////////////////////////////////////////////////
///
/// \file ToolTip.hpp
///  Declaration of the ToolTip Widget.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------- Tool Tip Placement
DeclareEnum4(IndicatorSide, Left, Top, Right, Bottom);

struct ToolTipPlacement
{
  ToolTipPlacement();

  /// Sets the screen rect and sets the hotspot to the center of the rect.
  void SetScreenRect(const Rect& rect);

  void SetPriority(IndicatorSide::Type pri0, IndicatorSide::Type pri1, 
                   IndicatorSide::Type pri2, IndicatorSide::Type pri3);
  
  /// The tooltip will be placed pointing at the edges of this rect.
  Rect mScreenRect;

  /// The tooltip will point to this hotspot inside the rect.
  Vec2 mHotSpot;

  /// The priority of edges to place on.
  IndicatorSide::Type mPriority[4];
};

//--------------------------------------------------------------------- Tool Tip
DeclareEnum6(ToolTipColor, Default, Gray, Red, Yellow, Green, Orange);

class ToolTip : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  ToolTip(Widget* source);
  /// This will set the text and try to fit the tooltip around the given
  /// widget with default values.
  ToolTip(Widget* source, StringParam text);

  /// Composite Interface.
  void UpdateTransform() override;
  void SizeToContents() override;

  /// Sets the widget to be displayed in the center of the tooltip.
  /// GetMinSize will be called on the given widget to lay out the tooltip.
  void SetContent(Widget* content);

  /// Creates a text object and sets it as the content.
  Text* SetText(StringParam text);

  /// Sets the arrow to point at the giving screen position.
  /// Returns whether or not the tooltip will be off the screen at all.
  bool SetArrowTipTranslation(Vec3Param screenPos);

  /// Attempts to find a position defined by the given placement objects.
  void SetArrowTipTranslation(ToolTipPlacement& placement);

  /// Whether or not to destroy the object when the mouse moves off the source.
  void SetDestroyOnMouseExit(bool state);

  void SetColor(ToolTipColor::Enum color);

  /// The side the tooltip is meant to be on.
  IndicatorSide::Type mSide;

  /// The color of the border.
  Vec4 mBackgroundColor;
  Vec4 mBorderColor;

  /// Padding for the content
  Thickness mContentPadding;

private:
  /// Shared constructor.
  void Initialize(Widget* source);
  
  /// We want to destroy ourself if our source is destroyed or the mouse exits.
  void OnMouseUpdate(MouseEvent* event);

  /// Offset used when shifting the tooltip onto the screen.
  Vec2 mArrowOffset;

  /// The border and arrows displayed behind the content.
  Element* mBackground;
  Element* mBorder;
  Element* mArrow;
  Element* mArrowBorder;

  /// The content contained in the tooltip.
  Widget* mContent;

  /// Whether or not to destroy the object when the mouse moves off the source.
  bool mDestroyOnMouseExitSource;

  /// Source to the source widget.
  HandleOf<Widget> mSource;
};

}//namespace Zero
