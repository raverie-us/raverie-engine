// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(UiDockLayout, builder, type)
{
  RaverieBindDocumented();
  RaverieBindComponent();
  RaverieBindInterface(UiLayout);
  RaverieBindFieldProperty(mSpacing);
}

void UiDockLayout::Initialize(CogInitializer& initializer)
{
  UiLayout::Initialize(initializer);
}

void UiDockLayout::Serialize(Serializer& stream)
{
  UiLayout::Serialize(stream);
  SerializeNameDefault(mSpacing, Vec2::cZero);
}

Vec2 UiDockLayout::Measure(Rectangle& rect)
{
  return MaxMeasure(rect);
}

UiWidget* FindPreviousInLayout(UiWidget* widget)
{
  do
  {
    widget = widget->GetPreviousSibling();
  } while (widget && !widget->GetInLayout());

  return widget;
}

float GetChildFlex(UiWidget* widget, uint axis)
{
  UiWidget* current = widget->mParent->GetLastDirectChild();
  if (current->GetInLayout() == false)
    current = FindPreviousInLayout(current);

  float flexSize = 0.0f;

  while (current != widget && current)
  {
    float currentFlexSize = current->GetFlexSize()[axis];
    uint currentAxis = UiDockMode::GetAxis(current->GetDockMode());

    if (currentAxis == axis)
      flexSize += currentFlexSize;
    else
      flexSize = Math::Max(flexSize, currentFlexSize);

    current = FindPreviousInLayout(current);
  }

  return flexSize;
}

float CalculateFlexedSize(UiWidget* widget, float flexSize, float totalSize)
{
  uint axis = UiDockMode::GetAxis(widget->GetDockMode());

  float remainingFlex = GetChildFlex(widget, axis);
  float totalFlex = flexSize + remainingFlex;
  float flexRatio = totalSize / totalFlex;
  return Math::Floor(flexSize * flexRatio);
}

void UiDockLayout::DoLayout(Rectangle& rect, UiTransformUpdateEvent* e)
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

  rect.RemoveThickness(mPadding);
  Vec2 offset = rect.GetBottomLeft();
  Vec2 size = rect.GetSize();

  Vec4 area = Vec4(offset.x, offset.y, offset.x + size.x, offset.y + size.y);

  UiFilteredChildren children = AllWidgetsInLayout();
  while (!children.Empty())
  {
    UiWidget* child = children.Front();
    children.PopFront();

    Vec2 size = child->Measure(rect);
    UiDockMode::Enum mode = child->GetDockMode();

    Vec2 areaSize = Vec2::cZero;
    Vec2 areaPos = Vec2::cZero;

    // Fill the last widget
    bool lastWidget = children.Empty();
    if (lastWidget)
    {
      areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Top]);
      areaSize =
          Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);
    }
    else
    {
      switch (mode)
      {
      case UiDockMode::Bottom:
      {
        if (child->GetSizePolicyY() == UiSizePolicy::Flex)
        {
          float flexSize = child->GetFlexSize().y;
          float totalSize = area[SlicesIndex::Bottom] - area[SlicesIndex::Top];
          size.y = CalculateFlexedSize(child, flexSize, totalSize);
        }

        float moveY = size.y;
        areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Top]);
        area[SlicesIndex::Top] += moveY + mSpacing[Axis::Y];
        areaSize = Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], moveY);
      }
      break;

      case UiDockMode::Top:
      {
        if (child->GetSizePolicyY() == UiSizePolicy::Flex)
        {
          float flexSize = child->GetFlexSize().y;
          float totalSize = area[SlicesIndex::Bottom] - area[SlicesIndex::Top];
          size.y = CalculateFlexedSize(child, flexSize, totalSize);
        }

        float moveY = size.y;
        areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Bottom] - moveY);
        area[SlicesIndex::Bottom] -= moveY + mSpacing[Axis::Y];
        areaSize = Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], moveY);
      }
      break;

      case UiDockMode::Left:
      {
        if (child->GetSizePolicyX() == UiSizePolicy::Flex)
        {
          float flexSize = child->GetFlexSize().x;
          float totalSize = area[SlicesIndex::Right] - area[SlicesIndex::Left];
          size.x = CalculateFlexedSize(child, flexSize, totalSize);
        }
        float moveX = size.x;
        areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Top]);
        area[SlicesIndex::Left] += moveX + mSpacing[Axis::X];
        areaSize = Vec2(moveX, area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);
      }
      break;

      case UiDockMode::Right:
      {
        if (child->GetSizePolicyX() == UiSizePolicy::Flex)
        {
          float flexSize = child->GetFlexSize().x;
          float totalSize = area[SlicesIndex::Right] - area[SlicesIndex::Left];
          size.x = CalculateFlexedSize(child, flexSize, totalSize);
        }
        float moveX = size.x;
        areaPos = Vec2(area[SlicesIndex::Right] - moveX, area[SlicesIndex::Top]);
        area[SlicesIndex::Right] -= moveX + mSpacing[Axis::X];
        areaSize = Vec2(moveX, area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);
      }
      break;
      }
    }

    Vec2 childPos = areaPos;
    Vec2 childSize = size;

    if (child->GetSizePolicyX() == UiSizePolicy::Flex)
    {
      childSize.x = areaSize.x;
    }
    else
    {
      if (child->GetSizePolicyX() == UiSizePolicy::Fixed)
        childSize.x = child->GetSize().x;
      CalculateAlignment(Axis::X, child->GetHorizontalAlignment(), areaSize, areaPos, childSize, childPos);
    }

    if (child->GetSizePolicyY() == UiSizePolicy::Flex)
    {
      childSize.y = areaSize.y;
    }
    else
    {
      if (child->GetSizePolicyY() == UiSizePolicy::Fixed)
        childSize.y = child->GetSize().y;
      CalculateAlignment(Axis::Y, child->GetVerticalAlignment(), areaSize, areaPos, childSize, childPos);
    }

    child->SetSize(childSize);
    child->SetLocalBottomLeft(childPos);
    child->Update(e);
  }
}

} // namespace Raverie
