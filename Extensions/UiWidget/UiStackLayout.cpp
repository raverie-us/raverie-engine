///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
Axis::Enum GetAxis(UiStackLayoutDirection::Enum direction)
{
  switch(direction)
  {
  case UiStackLayoutDirection::TopToBottom:
  case UiStackLayoutDirection::BottomToTop:
    return Axis::Y;

  case UiStackLayoutDirection::LeftToRight:
  case UiStackLayoutDirection::RightToLeft:
    return Axis::X;

  default:
    return Axis::X;
  }
}

//******************************************************************************
int GetSign(UiStackLayoutDirection::Enum direction)
{
  switch(direction)
  {
  case UiStackLayoutDirection::TopToBottom:
  case UiStackLayoutDirection::LeftToRight:
    return 1;
  case UiStackLayoutDirection::BottomToTop:
  case UiStackLayoutDirection::RightToLeft:
    return -1;
  default:
    return 0;
  }
}

//----------------------------------------------------------------- Stack Layout
//******************************************************************************
ZilchDefineType(UiStackLayout, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(UiLayout);
  ZilchBindGetterSetterProperty(StackDirection);
  ZilchBindGetterSetterProperty(Spacing);
}

//******************************************************************************
void UiStackLayout::Serialize(Serializer& stream)
{
  UiLayout::Serialize(stream);
  SerializeEnumName(UiStackLayoutDirection, mStackDirection);
  SerializeNameDefault(mSpacing, Vec2::cZero);
}

//******************************************************************************
void UiStackLayout::Initialize(CogInitializer& initializer)
{
  mWidget = GetOwner()->has(UiWidget);
}

//******************************************************************************
Vec2 UiStackLayout::Measure(Rect& rect)
{
  // Axis of Stacking 
  int stackAxis = GetAxis(mStackDirection);
  int opAxis = !stackAxis;

  Vec2 neededSize = Vec2::cZero;

  UiFilteredChildren r = AllWidgetsInLayout();
  while(!r.Empty())
  {
    UiWidget* child = r.Front();
    r.PopFront();

    // Measure for the minimum child size
    Vec2 childSize = child->Measure(rect);

    // Opposite axis is max of all sizes
    neededSize[opAxis] = Math::Max(neededSize[opAxis], childSize[opAxis]);
    neededSize[stackAxis] += childSize[stackAxis];

    // Only add spacing between widgets
    bool lastWidget = r.Empty();
    if(!lastWidget)
      neededSize[stackAxis] += mSpacing[stackAxis];
  }

  // Add the padding
  return neededSize + mPadding.Size();
}

//******************************************************************************
Vec2 UiStackLayout::DoLayout(Rect& rect, UiTransformUpdateEvent* e)
{
  // Debug break if set
  if(mDebug)
  {
    ZERO_DEBUG_BREAK;
    mDebug = false;
  }

  float snapSize = mWidget->GetSnapSize();

  // We're in charge of calling UpdateTransform on all of our children,
  // regardless of whether or not they're in the layout
  UpdateNotInLayout(e);

  // Axis of Stacking
  Axis::Enum stackAxis = GetAxis(mStackDirection);

  // Opposite axis of stack controls will be filled to this axis
  Axis::Enum opAxis = stackAxis == Axis::X ? Axis::Y : Axis::X;

  // Direction of layout
  bool reverse = (GetSign(mStackDirection) < 0);
  float direction = reverse ? -1.0f : 1.0f;

  float fixedSize = 0.0f;
  float totalFlex = 0.0f;
  float flexMinSize = 0.0f;

  // Do a first pass over the children to calculate the sizes we need
  UiFilteredChildren firstPass = AllWidgetsInLayout();
  while(!firstPass.Empty())
  {
    UiWidget* child = firstPass.Front();
    firstPass.PopFront();

    // Minimum size for the child
    Vec2 childSize = child->Measure(rect);

    if(child->GetSizePolicy(stackAxis) == UiSizePolicy::Flex)
    {
      totalFlex += child->GetFlexSize()[stackAxis];
      flexMinSize += childSize[stackAxis];
    }
    else
    {
      fixedSize += childSize[stackAxis];
    }

    // Margins are a fixed size, so they need to be accounted for
    fixedSize += child->GetMargins().Size()[stackAxis];

    // Only add padding between widgets
    bool lastWidget = firstPass.Empty();
    if(!lastWidget)
      fixedSize += mSpacing[stackAxis];
  }

  // Used to determine the starting location based on the current stack
  // direction. This is an optimization to avoid branching.
  Vec2 paddings[4] = { mPadding.TopLeft(), mPadding.BottomLeft(),
                       mPadding.TopLeft(), mPadding.TopRight() };

  // Remove the padding
  Vec2 initialSize = rect.GetSize();
  ApplyPadding(mPadding, rect);
  Vec2 areaSize = rect.GetSize();
  Vec2 offset = paddings[mStackDirection];

  // If we're laying out objects in reverse, we need to start from the end
  // on the stack axis
  if(reverse)
    offset[stackAxis] = initialSize[stackAxis] - paddings[mStackDirection][stackAxis];

  float totalSize = areaSize[stackAxis];

  // Flex ratio
  float flexRatio = ComputeFlexRatio(fixedSize, totalFlex, flexMinSize, totalSize);

  // When we all the flex objects can't be evenly distributed within the size
  // allocated for flex objects, we need to Assign them slightly "incorrect" sizes.
  // Example:
  // We have 300 pixels to Assign to two widgets (both with a flex ratio of 1).
  // There is a 1 pixel spacing (now 299 pixels for the flex widgets).
  // Both widgets will get assigned 149.5 pixels. If we were to call SnapToPixels,
  // they would both be given 150 pixels, going over our size limit (301 total).
  // This can also happen in the other direction (going under our size limit),
  // which can cause a small jitter when resizing windows with stack layouts.
  // To fix this issue, we're going to pass on the remainder of unused size
  // to the next widget. In the case above, the first would get assigned 149
  // pixels, and the 0.5 would get passed on to the next, which would get 150.
  float flexRemainder = 0.0f;

  UiFilteredChildren secondPass = AllWidgetsInLayout();
  while(!secondPass.Empty())
  {
    UiWidget* child = secondPass.Front();
    secondPass.PopFront();

    const Thickness& childMargins = child->GetMargins();

    // If we're laying out left to right, we want to apply the left margins
    // first, place the widget, then apply the right margins.
    // If we're laying out right to left, we want to do the opposite.
    // These are lookup tables for which margins to apply given the 
    // stack direction. This is an optimization to avoid branching.
    float marginsStart[4] = { childMargins.Top,    childMargins.Bottom,
                              childMargins.Left,   childMargins.Right };
    float marginsEnd[4] = {   childMargins.Bottom, childMargins.Top,
                              childMargins.Right,  childMargins.Left };

    // Before placing the widget, move over by the margins
    offset[stackAxis] += marginsStart[mStackDirection] * direction;

    // Measure the child
    rect.SizeX = areaSize.x;
    rect.SizeY = areaSize.y;
    Vec2 childSize = child ->Measure(rect);
    Vec2 childTranslation = offset;

    UiSizePolicy::Enum stackPolicy = child->GetSizePolicy(stackAxis);
    UiSizePolicy::Enum opPolicy = child->GetSizePolicy(opAxis);

    // Stack axis logic
    if(stackPolicy == UiSizePolicy::Flex)
    {
      float size = (child->GetFlexSize()[stackAxis] * flexRatio);// +childSize[stackAxis];

      // Add in the previous remainder
      size += flexRemainder;

      float flooredSize = Math::Floor(size / snapSize) * snapSize;
      childSize[stackAxis] = flooredSize;

      // Calculate the new remainder
      flexRemainder = (size - flooredSize) + 0.0001f;
    }
    else if (stackPolicy == UiSizePolicy::Fixed)
    {
      childSize[stackAxis] = child->GetSize()[stackAxis];
    }

    // Opposite axis logic
    if (opPolicy == UiSizePolicy::Flex)
    {
      childSize[opAxis] = areaSize[opAxis];

      // Margins on the opposite axis will shrink flex objects
      childSize[opAxis] -= childMargins.Size()[opAxis];
    }
    else
    {
      // if fixed force the size to the policy size
      if (opPolicy == UiSizePolicy::Fixed)
        childSize[opAxis] = child->GetSize()[opAxis];

      uint alignment = stackAxis ? child->GetHorizontalAlignment() : child->GetVerticalAlignment();
      CalculateAlignment(opAxis, alignment, areaSize, offset, childSize, childTranslation);
    }

    // Shift ourselves along the off axis based on our margins
    if(opAxis == 0)
    {
      if(child->GetHorizontalAlignment() != UiHorizontalAlignment::Left)
        childTranslation[opAxis] -= childMargins.Right;
      if(child->GetHorizontalAlignment() != UiHorizontalAlignment::Right)
        childTranslation[opAxis] += childMargins.Left;
    }
    else
    {
      if(child->GetVerticalAlignment() != UiVerticalAlignment::Top)
        childTranslation[opAxis] -= childMargins.Bottom;
      if(child->GetVerticalAlignment() != UiVerticalAlignment::Bottom)
        childTranslation[opAxis] += childMargins.Top;
    }

    // When laying out forward, the position we're calculating is in the top 
    // left. When in reverse, it's the opposite on the stack axis, so we need
    // calculate the actual top left corner for where it should be placed
    if(reverse)
      childTranslation[stackAxis] -= childSize[stackAxis];

    child->SetLocalTranslation(childTranslation);
    child->SetSize(childSize);
    child->UpdateTransform(e);

    offset[stackAxis] += childSize[stackAxis] * direction;

    // Shift by the margins that affect the translation of the next object
    offset[stackAxis] += marginsEnd[mStackDirection] * direction;

    bool lastWidget = secondPass.Empty();
    if(!lastWidget)
      offset[stackAxis] += mSpacing[stackAxis] * direction;
  }

  // Put these in an if until the rect is refactored...
  if(stackAxis == 0)
    rect.SizeX = offset[stackAxis];
  else
    rect.SizeY = offset[stackAxis];
  if (opAxis == 0)
    rect.SizeX = areaSize[opAxis];
  else
    rect.SizeY = areaSize[opAxis];

  // Remove padding
  RemovePadding(mPadding, rect);

  return rect.GetSize();
}

//******************************************************************************
float UiStackLayout::ComputeFlexRatio(float fixedSize, float totalFlex,
                                      float flexMinSize, float totalSize)
{
  float extraSize = totalSize - fixedSize;// -flexMinSize;
  // Only flex if there is extra space including min size used by flex controls
  if(extraSize > 0.0f && totalFlex > 0.0f)
    return extraSize / totalFlex;
  else
    return 0.0f;
}

//******************************************************************************
UiStackLayoutDirection::Enum UiStackLayout::GetStackDirection()
{
  return mStackDirection;
}

//******************************************************************************
void UiStackLayout::SetStackDirection(UiStackLayoutDirection::Enum direction)
{
  mStackDirection = direction;
  mWidget->MarkAsNeedsUpdate();
}

//******************************************************************************
Vec2 UiStackLayout::GetSpacing()
{
  return mSpacing;
}

//******************************************************************************
void UiStackLayout::SetSpacing(Vec2Param spacing)
{
  mSpacing = spacing;
  mWidget->MarkAsNeedsUpdate();
}

}//namespace Zero
