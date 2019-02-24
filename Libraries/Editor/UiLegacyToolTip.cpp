// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(UiLegacyToolTip, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  ZilchBindMethod(SetPlacement);
  ZilchBindMethod(SetPriority);

  ZilchBindSetter(Padding);

  ZilchBindMethod(SetColorScheme);
  ZilchBindSetter(BackgroundColor);
  ZilchBindSetter(BorderColor);

  ZilchBindMethod(ClearText);
  ZilchBindMethod(AddText);
}

UiLegacyToolTip::UiLegacyToolTip()
{
  ToolTip* toolTip = mToolTip = new ToolTip(Z::gEditor->GetRootWidget());

  toolTip->SetDestroyOnMouseExit(false);

  // Defaults below

  toolTip->mContentPadding = Thickness(2);

  toolTip->SetColorScheme(ToolTipColorScheme::Default);

  mPriority[0] = IndicatorSide::Right;
  mPriority[1] = IndicatorSide::Left;
  mPriority[2] = IndicatorSide::Bottom;
  mPriority[3] = IndicatorSide::Top;

  mPlacementRect = WidgetRect::MinAndMax(Vec2::cZero, Vec2::cZero);
}

UiLegacyToolTip::~UiLegacyToolTip()
{
  mToolTip.SafeDestroy();
}

void UiLegacyToolTip::SetPlacement(CameraViewport* viewport, RectangleParam localRect)
{
  mPlacementRect = viewport->mViewport->GetScreenRect();
  mPlacementRect.X += localRect.GetTopLeft().x;
  mPlacementRect.Y -= localRect.GetTopLeft().y;

  Vec2 size = localRect.GetSize();
  mPlacementRect.SizeX = size.x;
  mPlacementRect.SizeY = size.y;

  ToolTipPlacement placement;
  placement.SetScreenRect(mPlacementRect);
  placement.SetPriority(mPriority[0], mPriority[1], mPriority[2], mPriority[2]);

  mToolTip->SetArrowTipTranslation(placement);
}

void UiLegacyToolTip::SetPriority(IndicatorSide::Enum p0,
                                  IndicatorSide::Enum p1,
                                  IndicatorSide::Enum p2,
                                  IndicatorSide::Enum p3)
{
  mPriority[0] = p0;
  mPriority[1] = p1;
  mPriority[2] = p2;
  mPriority[3] = p3;

  ToolTipPlacement placement;
  placement.SetScreenRect(mPlacementRect);
  placement.SetPriority(p0, p1, p2, p3);

  ToolTip* toolTip = mToolTip;
  toolTip->SetArrowTipTranslation(placement);
}

void UiLegacyToolTip::SetColorScheme(ToolTipColorScheme::Enum color)
{
  mToolTip->SetColorScheme(color);
}

void UiLegacyToolTip::SetBackgroundColor(Vec4Param color)
{
  ToolTip* toolTip = mToolTip;
  toolTip->mBackgroundColor = color;
  toolTip->MarkAsNeedsUpdate();
}

void UiLegacyToolTip::SetBorderColor(Vec4Param color)
{
  ToolTip* toolTip = mToolTip;
  toolTip->mBorderColor = color;
  toolTip->MarkAsNeedsUpdate();
}

void UiLegacyToolTip::SetPadding(const Thickness& padding)
{
  ToolTip* toolTip = mToolTip;
  toolTip->mContentPadding = padding;
  toolTip->MarkAsNeedsUpdate();
}

void UiLegacyToolTip::ClearText()
{
  mToolTip->ClearText();
}

void UiLegacyToolTip::AddText(StringParam text, Vec4Param color)
{
  if (mToolTip.IsNull())
    return;

  ToolTip* toolTip = mToolTip;
  toolTip->AddText(text, color);

  // Must redo placement, as the tooltip size probably changed when adding a
  // line.
  ToolTipPlacement placement;
  placement.SetScreenRect(mPlacementRect);
  placement.SetPriority(mPriority[0], mPriority[1], mPriority[2], mPriority[2]);

  toolTip->SetArrowTipTranslation(placement);
}

} // namespace Zero
