// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Fill out necessary tooltip information for a Composite, if available.
bool GetToolTipText(int index, ListSource* source, StringBuilder* toolTipText);

DeclareEnum4(IndicatorSide, Left, Top, Right, Bottom);

struct ToolTipPlacement
{
  ToolTipPlacement();

  /// Sets the screen rect and sets the hotspot to the center of the rect.
  void SetScreenRect(const WidgetRect& rect);

  void
  SetPriority(IndicatorSide::Type pri0, IndicatorSide::Type pri1, IndicatorSide::Type pri2, IndicatorSide::Type pri3);

  /// The tooltip will be placed pointing at the edges of this rect.
  WidgetRect mScreenRect;

  /// The tooltip will point to this hotspot inside the rect.
  Vec2 mHotSpot;

  /// The priority of edges to place on.
  IndicatorSide::Type mPriority[4];
};

DeclareEnum6(ToolTipColorScheme, Default, Gray, Red, Yellow, Green, Orange);

class ToolTip : public Composite
{
public:
  RaverieDeclareType(ToolTip, TypeCopyMode::ReferenceType);

  /// Constructor.
  ToolTip(Widget* source);
  /// This will set the text and try to fit the tooltip around the given
  /// widget with default values.
  ToolTip(Widget* source, StringParam text);

  /// Composite Interface.
  void UpdateTransform() override;
  void SizeToContents() override;

  /// Make all multi-line text-objects in the ToolTip's content-hierarchy
  /// conform to the ToolTip's max best fit width, if applicable.
  void ForceBestFitText(Composite* composite, Vec2 padding);

  /// Sets the widget to be displayed in the center of the tooltip.
  /// GetMinSize will be called on the given widget to lay out the tooltip.
  void SetContent(Widget* content);

  /// If the content isn't already text, create a stack of one multitext object
  /// and set it as the content.  Else set the text on the already present
  /// multitext object.
  Text* SetText(StringParam text);
  /// If the content isn't already text, create a multitext stack.
  /// Then, add a new multitext object to the content's stack.
  /// Else, just to the add operation only.
  Text* AddText(StringParam text, Vec4Param color);
  /// Clear stack of multitext objects.
  void ClearText();

  /// Simple compound setter.  Calls 'SetText' with the additional ability
  /// to set the tooltip's position.
  void SetTextAndPlace(StringParam text, RectParam placementRect);

  /// Sets the arrow to point at the giving screen position.
  /// Returns whether or not the tooltip will be off the screen at all.
  bool SetArrowTipTranslation(Vec3Param screenPos);

  /// Attempts to find a position defined by the given placement objects.
  void SetArrowTipTranslation(ToolTipPlacement& placement);

  /// Whether or not to destroy the object when the mouse moves off the source.
  void SetDestroyOnMouseExit(bool state);

  void SetColorScheme(ToolTipColorScheme::Enum color);

  /// Further offset the ToolTip's arrow from its current location, where ever
  /// it is. If the arrow is offset outside the ToolTip's rect, then the arrow's
  /// visibility will be disabled. Return value denotes arrow visibility.
  bool TranslateArrowOffset(Vec2Param translation);

  /// Current offset of the ToolTip's arrow relative to the ToolTip rect's
  /// center, on each axis.
  Vec2 GetArrowOffset();

  /// Size of the content inside the ToolTip.
  Vec2 GetContentSize();

  /// Size of the content inside the ToolTip, including content padding.
  Vec2 GetPaddedContentSize();

  void BestFitTextToMaxWidth(bool enabled, float maxBestFitTextWidth = 0.0f);

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

  /// If the text wraps, then the width of this tooltip will never be more or
  /// less than the best fit width.
  /// If the text doesn't wrap, then the width of this tooltip can be less-than
  /// or equal to the best fit width.
  bool mBestFitText;
  float mMaxBestFitTextWidth;

  /// Offset used when shifting the tooltip onto the screen.
  Vec2 mArrowOffset;

  /// The border and arrows displayed behind the content.
  Element* mBackground;
  Element* mBorder;
  Element* mArrow;
  Element* mArrowBorder;

  /// The content contained in the tooltip.
  Widget* mContent;
  /// The content, but only if using text objects.
  Composite* mTextStack;

  /// Whether or not to destroy the object when the mouse moves off the source.
  bool mDestroyOnMouseExitSource;

  /// Source to the source widget.
  HandleOf<Widget> mSource;
};

} // namespace Raverie
