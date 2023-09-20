// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Whether or not the given widget should be in the layout
bool NeedsLayout(UiWidget* widget)
{
  return widget->GetActive() && widget->GetInLayout() && !widget->GetOwner()->GetMarkedForDestruction();
}

RaverieDefineType(UiLayout, builder, type)
{
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDependency(UiWidget);

  RaverieBindGetterSetterProperty(PaddingLeft);
  RaverieBindGetterSetterProperty(PaddingTop);
  RaverieBindGetterSetterProperty(PaddingRight);
  RaverieBindGetterSetterProperty(PaddingBottom);
}

void UiLayout::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("PaddingLeft", mPadding.Left, 0.0f);
  stream.SerializeFieldDefault("PaddingTop", mPadding.Top, 0.0f);
  stream.SerializeFieldDefault("PaddingRight", mPadding.Right, 0.0f);
  stream.SerializeFieldDefault("PaddingBottom", mPadding.Bottom, 0.0f);
}

void UiLayout::Initialize(CogInitializer& initializer)
{
  mDebug = false;
  mWidget = GetOwner()->has(UiWidget);
}

float UiLayout::GetPaddingLeft()
{
  return mPadding.Left;
}

float UiLayout::GetPaddingTop()
{
  return mPadding.Top;
}

float UiLayout::GetPaddingRight()
{
  return mPadding.Right;
}

float UiLayout::GetPaddingBottom()
{
  return mPadding.Bottom;
}

void UiLayout::SetPaddingLeft(float val)
{
  mPadding.Left = Snap(val, cUiWidgetSnapSize);
  mWidget->MarkAsNeedsUpdate();
}

void UiLayout::SetPaddingTop(float val)
{
  mPadding.Top = Snap(val, cUiWidgetSnapSize);
  mWidget->MarkAsNeedsUpdate();
}

void UiLayout::SetPaddingRight(float val)
{
  mPadding.Right = Snap(val, cUiWidgetSnapSize);
  mWidget->MarkAsNeedsUpdate();
}

void UiLayout::SetPaddingBottom(float val)
{
  mPadding.Bottom = Snap(val, cUiWidgetSnapSize);
  mWidget->MarkAsNeedsUpdate();
}

void UiLayout::Debug()
{
  mDebug = true;
}

void UiLayout::UpdateNotInLayout(UiTransformUpdateEvent* e)
{
  forRange (Cog& child, GetOwner()->GetChildren())
  {
    if (UiWidget* widget = child.has(UiWidget))
    {
      if (widget->GetActive() && !widget->GetInLayout())
      {
        widget->SizeToContentsIfAuto();
        widget->Update(e);
      }
    }
  }
}

void UiLayout::CalculateAlignment(Axis::Type axis,
                                  uint alignment,
                                  Vec2Param areaSize,
                                  Vec2Param areaPos,
                                  Vec2Param childSize,
                                  Vec2Ref childTranslation)
{
  switch (alignment)
  {
  case UiHorizontalAlignment::Left:
    // case UiVerticalAlignment::Bottom:
    childTranslation[axis] = areaPos[axis];
    break;
  case UiHorizontalAlignment::Right:
    // case UiVerticalAlignment::Top:
    childTranslation[axis] = areaPos[axis] + (areaSize[axis] - childSize[axis]);
    break;
  case UiHorizontalAlignment::Center:
    // case UiVerticalAlignment::Center:
    childTranslation[axis] = areaPos[axis] + (areaSize[axis] / 2.0f) - (childSize[axis] / 2.0f);
    break;
  }
}

Vec2 UiLayout::MaxMeasure(Rectangle& rect)
{
  Vec2 neededSize = Vec2(0, 0);

  forRange (UiWidget* child, AllWidgetsInLayout())
  {
    Vec2 childSize = child->Measure(rect) + child->GetMargins().Size();
    neededSize.x = Math::Max(neededSize.x, childSize.x);
    neededSize.y = Math::Max(neededSize.y, childSize.y);
  }

  return neededSize + mPadding.Size();
}

UiLayout::UiFilteredChildren UiLayout::AllWidgetsInLayout()
{
  return UiFilteredChildren(mWidget);
}

UiLayout::UiFilteredChildren::UiFilteredChildren(UiWidget* widget)
{
  mRange = widget->GetChildren();
  SkipInvalid();
}

UiWidget* UiLayout::UiFilteredChildren::Front()
{
  return &mRange.Front();
}

bool UiLayout::UiFilteredChildren::Empty()
{
  return mRange.Empty();
}

void UiLayout::UiFilteredChildren::PopFront()
{
  mRange.PopFront();
  SkipInvalid();
}

void UiLayout::UiFilteredChildren::SkipInvalid()
{
  while (!mRange.Empty())
  {
    if (NeedsLayout(&mRange.Front()))
      return;
    mRange.PopFront();
  }
}

} // namespace Raverie
