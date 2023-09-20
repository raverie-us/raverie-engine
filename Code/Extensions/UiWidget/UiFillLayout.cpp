// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(UiFillLayout, builder, type)
{
  RaverieBindDocumented();
  RaverieBindComponent();
  RaverieBindInterface(UiLayout);

  RaverieBindMethod(FillToParent);
  RaverieBindMethod(FillToRectangle);
}

void UiFillLayout::Initialize(CogInitializer& initializer)
{
  UiLayout::Initialize(initializer);
}

Vec2 UiFillLayout::Measure(Rectangle& rect)
{
  return MaxMeasure(rect);
}

void UiFillLayout::DoLayout(Rectangle& rect, UiTransformUpdateEvent* e)
{
  // Debug break if set
  if (mDebug)
  {
    RaverieDebugBreak();
    mDebug = false;
  }

  // We're in charge of calling UpdateTransform on all of our children,
  // regardless of whether or not they're in the layout
  UpdateNotInLayout(e);

  // Apply the padding before laying out the children
  rect.RemoveThickness(mPadding);

  // Layout each child
  forRange (UiWidget* child, AllWidgetsInLayout())
  {
    // Do nothing if it's not in the layout
    if (!child->GetInLayout())
      continue;

    FillToRectangle(rect, child);
    child->Update(e);
  }
}

void UiFillLayout::FillToParent(UiWidget* child)
{
  if (UiWidget* parent = child->mParent)
  {
    Rectangle rect = parent->GetBodyRectangle();
    FillToRectangle(rect, child);
  }
}

void UiFillLayout::FillToRectangle(RectangleParam rect, UiWidget* widget)
{
  // Measure the child object
  Rectangle tempRect;
  Vec2 childSize = widget->Measure(tempRect);

  const Thickness& childMargins = widget->GetMargins();

  // Add the margins
  childSize += childMargins.Size();

  Vec2 pos = rect.GetBottomLeft();
  Vec2 size = rect.GetSize();

  Vec2 childPos = pos;

  if (widget->GetSizePolicyX() == UiSizePolicy::Flex)
  {
    childSize.x = size.x;
    childPos.x = pos.x;
  }
  else
  {
    if (widget->GetSizePolicyX() == UiSizePolicy::Fixed)
      childSize.x = widget->GetSize().x + childMargins.Width();

    CalculateAlignment(Axis::X, widget->GetHorizontalAlignment(), size, pos, childSize, childPos);
  }

  if (widget->GetSizePolicyY() == UiSizePolicy::Flex)
  {
    childSize.y = size.y;
    childPos.y = pos.y;
  }
  else
  {
    if (widget->GetSizePolicyY() == UiSizePolicy::Fixed)
      childSize.y = widget->GetSize().y + childMargins.Height();

    CalculateAlignment(Axis::Y, widget->GetVerticalAlignment(), size, pos, childSize, childPos);
  }

  Rectangle childRect = Rectangle::PointAndSize(childPos, childSize);

  childRect.RemoveThickness(childMargins);

  widget->SetSize(childRect.GetSize());
  widget->SetLocalBottomLeft(childRect.GetBottomLeft());
}

} // namespace Raverie
