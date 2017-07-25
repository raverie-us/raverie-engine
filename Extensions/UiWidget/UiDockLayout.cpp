///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ Dock Layout
//******************************************************************************
ZilchDefineType(UiDockLayout, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(UiLayout);
  ZilchBindFieldProperty(mSpacing);
}

//******************************************************************************
void UiDockLayout::Initialize(CogInitializer& initializer)
{
  UiLayout::Initialize(initializer);
}

//******************************************************************************
void UiDockLayout::Serialize(Serializer& stream)
{
  UiLayout::Serialize(stream);
  SerializeNameDefault(mSpacing, Vec2::cZero);
}

//******************************************************************************
Vec2 UiDockLayout::Measure(Rect& rect)
{
  return MaxMeasure(rect);
}

//******************************************************************************
Vec2 UiDockLayout::DoLayout(Rect& rect, UiTransformUpdateEvent* e)
{
  // Debug break if set
  if (mDebug)
  {
    ZERO_DEBUG_BREAK;
    mDebug = false;
  }

  // We're in charge of calling UpdateTransform on all of our children,
  // regardless of whether or not they're in the layout
  UpdateNotInLayout(e);

  ApplyPadding(mPadding, rect);
  Vec2 offset = rect.GetPosition();
  Vec2 size = rect.GetSize();

  Vec4 area = Vec4(offset.x, offset.y,
                   offset.x + size.x, offset.y + size.y);

  UiFilteredChildren children = AllWidgetsInLayout();
  while(!children.Empty())
  {
    UiWidget* child = children.Front();
    children.PopFront();

    Vec2 size = child->Measure(rect);
    UiDockMode::Enum mode = child->GetDockMode();

    Vec2 areaSize = Vec2::cZero;
    Vec2 areaPos = Vec2::cZero;

    // Fill the last widget
    bool lastWidget = children.Empty();
    if(lastWidget)
    {
      areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Top]);
      areaSize = Vec2(area[SlicesIndex::Right] - 
                      area[SlicesIndex::Left], 
                      area[SlicesIndex::Bottom] - 
                      area[SlicesIndex::Top]);
    }
    else
    {
      switch (mode)
      {
        //--------------------------------------------------------------------
      case UiDockMode::Top:
        {
          float moveY = size.y;
          areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Top]);
          area[SlicesIndex::Top] += moveY + mSpacing[Axis::Y];
          areaSize = Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], moveY);
        }
        break;

        //--------------------------------------------------------------------
      case UiDockMode::Bottom:
        {
          float moveY = size.y;
          areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Bottom] - moveY);
          area[SlicesIndex::Bottom] -= moveY + mSpacing[Axis::Y];
          areaSize = Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], moveY);
        }
        break;

        //--------------------------------------------------------------------
      case UiDockMode::Left:
        {
          float moveX = size.x;
          areaPos = Vec2(area[SlicesIndex::Left], area[SlicesIndex::Top]);
          area[SlicesIndex::Left] += moveX + mSpacing[Axis::X];
          areaSize = Vec2(moveX, area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);
        }
        break;

        //--------------------------------------------------------------------
      case UiDockMode::Right:
        {
          float moveX = size.x;
          areaPos = Vec2(area[SlicesIndex::Right] - moveX, area[SlicesIndex::Top]);
          area[SlicesIndex::Right] -= moveX + +mSpacing[Axis::X];
          areaSize = Vec2(moveX, area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);          
        }
        break;
      }
    }

    Vec2 childPos = areaPos;
    Vec2 childSize = size;

    if(child->GetSizePolicyX() == UiSizePolicy::Flex)
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
      if(child->GetSizePolicyY() == UiSizePolicy::Fixed)
        childSize.y = child->GetSize().y;
      CalculateAlignment(Axis::Y, child->GetVerticalAlignment(), areaSize, areaPos, childSize, childPos);
    }

    child->SetLocalTranslation(childPos);
    child->SetSize(childSize);
    child->UpdateTransform(e);
  }

  RemovePadding(mPadding, rect);
  return rect.GetSize();
}

}//namespace Zero
