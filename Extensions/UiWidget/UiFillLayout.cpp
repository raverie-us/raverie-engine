///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ Fill Layout
//******************************************************************************
ZilchDefineType(UiFillLayout, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(UiLayout);
}

//******************************************************************************
void UiFillLayout::Initialize(CogInitializer& initializer)
{
  UiLayout::Initialize(initializer);
}

//******************************************************************************
Vec2 UiFillLayout::Measure(Rect& rect)
{
  return MaxMeasure(rect);
}

//******************************************************************************
Vec2 UiFillLayout::DoLayout(Rect& rect, UiTransformUpdateEvent* e)
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

  // Apply the padding before laying out the children
  ApplyPadding(mPadding, rect);
  Vec2 pos = rect.GetPosition();
  Vec2 size = rect.GetSize();

  // Layout each child
  forRange(UiWidget* child, AllWidgetsInLayout())
  {
    // Do nothing if it's not in the layout
    if(!child->GetInLayout())
      continue;

    // Measure the child object
    Rect tempRect;
    Vec2 childSize = child->Measure(tempRect);

    const Thickness& childMargins = child->GetMargins();

    // Add the margins
    childSize += child->GetMargins().Size();

    Vec2 childPos = pos;

    if(child->GetSizePolicyX() == UiSizePolicy::Flex)
    {
      childSize.x = size.x;
      childPos.x = pos.x;
    }
    else
    {
      if(child->GetSizePolicyX() == UiSizePolicy::Fixed)
        childSize.x = child->GetSize().x + childMargins.Width();

      CalculateAlignment(Axis::X, child->GetHorizontalAlignment(), size, pos, childSize, childPos);
    }

    if(child->GetSizePolicyY() == UiSizePolicy::Flex)
    {
      childSize.y = size.y;
      childPos.y = pos.y;
    }
    else 
    {
      if(child->GetSizePolicyY() == UiSizePolicy::Fixed)
        childSize.y = child->GetSize().y + childMargins.Height();

      CalculateAlignment(Axis::Y, child->GetVerticalAlignment(), size, pos, childSize, childPos);
    }

    Rect childRect = Rect::PointAndSize(childPos, childSize);

    childRect.RemoveThickness(childMargins);

    child->SetLocalTranslation(childRect.GetPosition());
    child->SetSize(childRect.GetSize());
    child->UpdateTransform(e);
  }

  RemovePadding(mPadding, rect);
  return rect.GetSize();
}

}//namespace Zero
