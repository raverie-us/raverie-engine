///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Whether or not the given widget should be in the layout
bool NeedsLayout(UiWidget* widget)
{
  return widget->GetActive() && widget->GetInLayout() && !widget->GetOwner()->GetMarkedForDestruction();
}

//----------------------------------------------------------------------- Layout
//******************************************************************************
ZilchDefineType(UiLayout, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(UiWidget);

  ZilchBindGetterSetterProperty(PaddingLeft);
  ZilchBindGetterSetterProperty(PaddingTop);
  ZilchBindGetterSetterProperty(PaddingRight);
  ZilchBindGetterSetterProperty(PaddingBottom);

  // Only show the debug button if we're in Visual Studio
  if(Os::IsDebuggerAttached())
    ZilchBindMethodProperty(Debug);
}

//******************************************************************************
void UiLayout::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("PaddingLeft",   mPadding.Left,   0.0f);
  stream.SerializeFieldDefault("PaddingTop",    mPadding.Top,    0.0f);
  stream.SerializeFieldDefault("PaddingRight",  mPadding.Right,  0.0f);
  stream.SerializeFieldDefault("PaddingBottom", mPadding.Bottom, 0.0f);
}

//******************************************************************************
void UiLayout::Initialize(CogInitializer& initializer)
{
  mDebug = false;
  mWidget = GetOwner()->has(UiWidget);
}

//******************************************************************************
float UiLayout::GetPaddingLeft()
{
  return mPadding.Left;
}

//******************************************************************************
float UiLayout::GetPaddingTop()
{
  return mPadding.Top;
}

//******************************************************************************
float UiLayout::GetPaddingRight()
{
  return mPadding.Right;
}

//******************************************************************************
float UiLayout::GetPaddingBottom()
{
  return mPadding.Bottom;
}

//******************************************************************************
void UiLayout::SetPaddingLeft(float val)
{
  mPadding.Left = val;
  mWidget->MarkAsNeedsUpdate();
}

//******************************************************************************
void UiLayout::SetPaddingTop(float val)
{
  mPadding.Top = val;
  mWidget->MarkAsNeedsUpdate();
}

//******************************************************************************
void UiLayout::SetPaddingRight(float val)
{
  mPadding.Right = val;
  mWidget->MarkAsNeedsUpdate();
}

//******************************************************************************
void UiLayout::SetPaddingBottom(float val)
{
  mPadding.Bottom = val;
  mWidget->MarkAsNeedsUpdate();
}

//******************************************************************************
void UiLayout::Debug()
{
  mDebug = true;
}

//******************************************************************************
void UiLayout::UpdateNotInLayout(UiTransformUpdateEvent* e)
{
  forRange(Cog& child, GetOwner()->GetChildren())
  {
    if(UiWidget* widget = child.has(UiWidget))
    {
      if(widget->GetActive() && !widget->GetInLayout())
        widget->UpdateTransform(e);
    }
  }
}

//******************************************************************************
void UiLayout::CalculateAlignment(Axis::Type axis, uint alignment,
                                  Vec2Param areaSize, Vec2Param areaPos,
                                  Vec2Param childSize, Vec2Ref childTranslation)
{
  switch (alignment)
  {
  case UiHorizontalAlignment::Left:
    //case UiVerticalAlignment::Top:
    childTranslation[axis] = areaPos[axis];
    break;
  case UiHorizontalAlignment::Right:
    //case UiVerticalAlignment::Bottom:
    childTranslation[axis] = areaPos[axis] + (areaSize[axis] - childSize[axis]);
    break;
  case UiHorizontalAlignment::Center:
    //case UiVerticalAlignment::Center:
    childTranslation[axis] = areaPos[axis] + (areaSize[axis] / 2.0f) - (childSize[axis] / 2.0f);
    break;
  }
}

//******************************************************************************
Vec2 UiLayout::MaxMeasure(Rect& rect)
{
  Vec2 neededSize = Vec2(0,0);

  forRange(UiWidget* child, AllWidgetsInLayout())
  {
    Vec2 childSize = child->Measure(rect) + child->GetMargins().Size();
    neededSize.x = Math::Max(neededSize.x, childSize.x);
    neededSize.y = Math::Max(neededSize.y, childSize.y);
  }

  return neededSize + mPadding.Size();
}

//******************************************************************************
void UiLayout::ApplyPadding(Thickness& padding, Rect& area)
{
  Vec2 padTopLeft = padding.TopLeft();
  Vec2 padSize = padding.Size();
  area.X += padTopLeft.x;
  area.Y += padTopLeft.y;
  area.SizeX -= padSize.x;
  area.SizeY -= padSize.y;
}

//******************************************************************************
void UiLayout::RemovePadding(Thickness& padding, Rect& area)
{
  Vec2 padTopLeft = padding.TopLeft();
  Vec2 padSize = padding.Size();

  area.X -= padTopLeft.x;
  area.Y -= padTopLeft.y;
  area.SizeX += padSize.x;
  area.SizeY += padSize.y;
}

//******************************************************************************
UiLayout::UiFilteredChildren UiLayout::AllWidgetsInLayout()
{
  return UiFilteredChildren(mWidget);
}

//------------------------------------------------------------ Filtered Children
//******************************************************************************
UiLayout::UiFilteredChildren::UiFilteredChildren(UiWidget* widget)
{
  mRange = widget->GetChildren();
  SkipInvalid();
}

//******************************************************************************
UiWidget* UiLayout::UiFilteredChildren::Front()
{
  return &mRange.Front();
}

//******************************************************************************
bool UiLayout::UiFilteredChildren::Empty()
{
  return mRange.Empty();
}

//******************************************************************************
void UiLayout::UiFilteredChildren::PopFront()
{
  mRange.PopFront();
  SkipInvalid();
}

//******************************************************************************
void UiLayout::UiFilteredChildren::SkipInvalid()
{
  while(!mRange.Empty())
  {
    if(NeedsLayout(&mRange.Front()))
      return;
    mRange.PopFront();
  }
}

}//namespace Zero
