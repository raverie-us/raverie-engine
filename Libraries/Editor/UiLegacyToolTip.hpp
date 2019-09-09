// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class UiLegacyToolTip
{
public:
  ZilchDeclareType(UiLegacyToolTip, TypeCopyMode::ReferenceType);

  UiLegacyToolTip();
  ~UiLegacyToolTip();

  /// The CameraViewport passed in - is in the same space as the UIWidget
  /// in which the Rectangle was obtained.  The Rectangle should define
  /// which the boundary about which the ToolTip will be placed.
  /// [ie, left-of, right-of, top-of, bottom-of].
  void SetPlacement(CameraViewport* viewport, RectangleParam localRect);

  void SetPriority(IndicatorSide::Enum p0, IndicatorSide::Enum p1, IndicatorSide::Enum p2, IndicatorSide::Enum p3);

  void SetColorScheme(ToolTipColorScheme::Enum color);
  void SetBackgroundColor(Vec4Param c);
  void SetBorderColor(Vec4Param c);

  void SetPadding(const Thickness& padding);

  void ClearText();
  void AddText(StringParam text, Vec4Param color);

public:
  Vec4 mBorderColor;
  Vec4 mBackgroundColor;
  Thickness mThickness;
  IndicatorSide::Enum mPriority[4];

  WidgetRect mPlacementRect;

  HandleOf<ToolTip> mToolTip;
};

} // namespace Zero
