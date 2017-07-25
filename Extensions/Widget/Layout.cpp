///////////////////////////////////////////////////////////////////////////////
///
/// \file Layout.cpp
/// Implementation of the Layout widget support class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

uint GetAxis(DockArea::Enum dockArea)
{
  switch(dockArea)
  {
  case DockArea::Left:
  case DockArea::Right:
    return Math::cX;
  case DockArea::TopTool:
  case DockArea::BotTool:
  case DockArea::Top:
  case DockArea::Bottom:
    return Math::cY;
  default: return 0;
  }
}

int GetSign(DockArea::Enum dockArea)
{
  switch(dockArea)
  {
  case DockArea::TopTool:
  case DockArea::Left:
  case DockArea::Top:
    return -1;
  case DockArea::BotTool:
  case DockArea::Right:
  case DockArea::Bottom:
    return 1;
  default:
    return 0;
  }
}

bool NeedsLayout(Widget& widget)
{
  return widget.mActive && !widget.mNotInLayout;
}

struct FilteredChildren
{
  WidgetListRange mChildren;

  FilteredChildren(Composite* widget)
  {
    // Remove all invalid children from the end
    WidgetListRange children = widget->GetChildren();
    while(!children.Empty() && !NeedsLayout(children.Back()))
      children.PopBack();
    mChildren = children;
    SkipInvalid();
  }

  Widget& Front()
  {
    return mChildren.Front();
  }

  bool Empty()
  {
    return mChildren.Empty();
  }

  void PopFront()
  {
    mChildren.PopFront();
    SkipInvalid();
  }

  void SkipInvalid()
  {
    while(!mChildren.Empty())
    {
      if(NeedsLayout(mChildren.Front()))
        return;
      mChildren.PopFront();
    }
  }
};

void UpdateNotInLayout(Composite* widget)
{
  WidgetListRange children = widget->GetChildren();
  while(!children.Empty())
  {
    if(children.Front().mActive && children.Front().mNotInLayout)
      children.Front().UpdateTransformExternal();
    children.PopFront();
  }
}

LayoutResult AspectLayout(Vec2 aspect, Vec2 size)
{
  LayoutResult lr;
  float sourceRatio = aspect.x / aspect.y;
  float screenRatio = size.x / size.y;
  Vec2 scaleAspect = sourceRatio < screenRatio ? Vec2(sourceRatio / screenRatio, 1) : Vec2(1, screenRatio / sourceRatio);
  lr.Size =  size * scaleAspect;
  lr.Translation = ToVector3((size - lr.Size) * 0.5f);
  return lr;
}

Vec2 MaxMeasure(Composite* widget, LayoutArea data)
{
  Vec2 neededSize = Vec2(0,0);
  FilteredChildren children(widget);
  while(!children.Empty())
  {
    Widget& child = children.Front();
    Vec2 childSize = child.Measure(data);
    neededSize.x = Math::Max(neededSize.x, childSize.x);
    neededSize.y = Math::Max(neededSize.y, childSize.y);
    children.PopFront();
  }
  return neededSize;
}

//******************************************************************************
void CalculateAlignment(SizeAxis::Type axis, uint alignment,
  Vec2 areaSize, Vec3 areaPos, Vec2 &childSize, Vec3 &childTranslation)
{
  switch (alignment)
  {
  case HorizontalAlignment::Left:
    //case VerticalAlignment::Top:
    childTranslation[axis] = areaPos[axis];
    break;
  case HorizontalAlignment::Right:
    //case VerticalAlignment::Bottom:
    childTranslation[axis] = areaPos[axis] + (areaSize[axis] - childSize[axis]);
    break;
  case HorizontalAlignment::Center:
    //case VerticalAlignment::Center:
    childTranslation[axis] = areaPos[axis] + (areaSize[axis] / 2.0f) - (childSize[axis] / 2.0f);
    break;
  }
}

//******************************************************************************
void ApplyPadding(Thickness padding, LayoutArea &area)
{
  area.Offset += Vec3(padding.TopLeft());
  area.Size -= padding.Size();
}

//******************************************************************************
void RemovePadding(Thickness padding, LayoutArea &area)
{
  area.Offset -= Vec3(padding.TopLeft());
  area.Size += padding.Size();
}

//------------------------------------------------------------------ Layout Area
//******************************************************************************
ZilchDefineType(Layout, builder, type)
{
  ZilchBindDestructor();

  ZilchBindMethod(Measure);
  ZilchBindMethod(DoLayout);
  
  ZilchBindFieldProperty(Padding);
}

//******************************************************************************
Layout::Layout(Thickness padding)
  : Padding(padding)
  , mDebug(false)
{
}

//------------------------------------------------------------------ Layout Area
//******************************************************************************
ZilchDefineType(LayoutArea, builder, type)
{
}

//------------------------------------------------------------------ Fill 
//******************************************************************************
ZilchDefineType(FillLayout, builder, type)
{
}

//******************************************************************************
FillLayout::FillLayout(Thickness padding)
  : Layout(padding)
{
}

//******************************************************************************
Vec2 FillLayout::Measure(Composite* widget, LayoutArea data)
{
  return MaxMeasure(widget, data) + Padding.Size();
}

//******************************************************************************
Vec2 FillLayout::DoLayout(Composite* widget, LayoutArea data)
{
  if (mDebug)
    __debugbreak();

  WidgetListRange children = widget->GetChildren();

  ApplyPadding(Padding, data);
  Vec3 pos = data.Offset;
  Vec2 size = data.Size;

  while (!children.Empty())
  {
    Widget& child = children.Front();
    children.PopFront();

    if (child.GetNotInLayout())
    {
      child.UpdateTransformExternal();
      continue;
    }

    SizePolicies policy = child.GetSizePolicy();

    LayoutArea data;
    Vec2 childSize = child.Measure(data);
    Vec3 childPos = pos;

    if (policy.XPolicy == SizePolicy::Flex)
    {
      childSize.x = size.x;
      childPos.x = pos.x;
    }
    else
    {
      if (policy.XPolicy == SizePolicy::Fixed)
        childSize.x = policy.Size.x;

      CalculateAlignment(SizeAxis::X, child.mHorizontalAlignment, size, pos, childSize, childPos);
    }

    if (policy.YPolicy == SizePolicy::Flex)
    {
      childSize.y = size.y;
      childPos.y = pos.y;
    }
    else 
    {
      if (policy.YPolicy == SizePolicy::Fixed)
        childSize.y = policy.Size.y;

      CalculateAlignment(SizeAxis::Y, child.mVerticalAlignment, size, pos, childSize, childPos);
    }

    child.SetTranslationAndSize(childPos, childSize);
    child.UpdateTransformExternal();
  }
  
  RemovePadding(Padding, data);
  return data.Size;
}

//----------------------------------------------------------------- Stack Layout
//******************************************************************************
ZilchDefineType(StackLayout, builder, type)
{
}

//******************************************************************************
StackLayout::StackLayout(Thickness padding)
  : Layout(padding)
{
  Direction = LayoutDirection::TopToBottom;
  Spacing = Vec2::cZero;
}

//******************************************************************************
StackLayout::StackLayout(LayoutDirection::Enum direction, Vec2 spacing, Thickness padding)
  : Layout(padding)
{
  Direction = direction;
  Spacing = spacing;
}

//******************************************************************************
Vec2 StackLayout::Measure(Composite* widget, LayoutArea data)
{
  // Axis of Stacking 
  int stackAxis = GetAxis((DockArea::Enum)Direction);
  int opAxis = !stackAxis;

  Vec2 neededSize = Vec2(0,0);

  FilteredChildren children(widget);
  while(!children.Empty())
  {
    Widget& child = children.Front();
    children.PopFront();

    bool lastWidget = children.Empty();

    Vec2 childSize = child.Measure(data);

    // Opposite axis is max of all sizes
    neededSize[opAxis] = Math::Max(neededSize[opAxis], childSize[opAxis]);
    neededSize[stackAxis] += childSize[stackAxis];

    // Only add spacing between widgets
    if(!lastWidget)
      neededSize[stackAxis] += Spacing[stackAxis];
  }

  // Add the padding
  return neededSize + Padding.Size();
}

//******************************************************************************
float StackLayout::ComputeFlexRatio(float fixedSize, float totalFlex, float flexMinSize, float totalSize)
{
  float extraSize = totalSize - fixedSize - flexMinSize;
  // Only flex if there is extra space including min size used by flex controls
  if(extraSize > 0.0f && totalFlex > 0.0f)
    return extraSize / totalFlex;
  else
    return 0.0f;
}

//******************************************************************************
Vec2 StackLayout::DoLayout(Composite* widget, LayoutArea data)
{
  if (mDebug)
    __debugbreak();

  // Get all children of the widget
  UpdateNotInLayout(widget);

  FilteredChildren children(widget);
    
  // Axis of Stacking 
  int stackAxis = GetAxis((DockArea::Enum)Direction);

  // Opposite axis of stack 
  // controls will be filled to this axis
  int opAxis = !stackAxis;

  // Direction of layout
  int direction = GetSign((DockArea::Enum)Direction);

  float fixedSize = 0.0f;
  float totalFlex = 0.0f;
  float flexMinSize = 0.f;

  FilteredChildren firstPass(widget);
  while(!firstPass.Empty())
  {
    Widget& child = firstPass.Front();
      
    Vec2 childSize = child.Measure(data);

    if(child.mSizePolicy.Policy[stackAxis] == SizePolicy::Flex)
    {
      totalFlex += child.mSizePolicy.Size[stackAxis];
      flexMinSize += childSize[stackAxis];
    }
    else
      fixedSize += childSize[stackAxis];

    firstPass.PopFront();

    // Only add padding between widgets
    if(!firstPass.Empty())
      fixedSize += Spacing[stackAxis];
  }

  ApplyPadding(Padding, data);
  Vec2 areaSize = data.Size;
  Vec3 offset = data.Offset;

  float totalSize = areaSize[stackAxis];

  float flexRemainder = 0.0f;

  // Flex ratio
  float flexRatio = ComputeFlexRatio(fixedSize, totalFlex, flexMinSize, totalSize);

  while(!children.Empty())
  {
    Widget& child = children.Front();
    children.PopFront();

    SizePolicies policy = child.GetSizePolicy();

    bool lastWidget = children.Empty();

    // Measure the child
    data.Size = areaSize;
    Vec2 childSize = child.Measure(data);
    Vec3 childTranslation = offset;

    SizePolicy::Type stackPolicy = policy.Policy[stackAxis];
    SizePolicy::Type opPolicy = policy.Policy[opAxis];

    // Stack axis logic
    if(stackPolicy == SizePolicy::Flex)
    {
      float size = (policy.Size[stackAxis] * flexRatio);

      // Add in the previous remainder
      size += flexRemainder;

      float flooredSize = Math::Floor(size);
      childSize[stackAxis] = flooredSize + childSize[stackAxis];

      // Calculate the new remainder
      flexRemainder = (size - flooredSize) + 0.0001f;

      //childSize[stackAxis] = SnapToPixels(policy.Size[stackAxis] * flexRatio) + childSize[stackAxis];
    }
    else if (stackPolicy == SizePolicy::Fixed)
    {
      childSize[stackAxis] = policy.Size[stackAxis];
    }

    // Opposite axis logic
    if (opPolicy == SizePolicy::Flex)
    {
      childSize[opAxis] = areaSize[opAxis];
    }
    else
    {
      // if fixed force the size to the policy size
      if (opPolicy == SizePolicy::Fixed)
        childSize[opAxis] = policy.Size[opAxis];

      uint alignment = stackAxis ? child.mHorizontalAlignment : child.mVerticalAlignment;
      CalculateAlignment(opAxis, alignment, areaSize, offset, childSize, childTranslation);
    }

    // If the direction is reversed flip the translation
    if(direction > 0)
      childTranslation[stackAxis] = areaSize[stackAxis] - childSize[stackAxis] - offset[stackAxis];

    child.SetTranslationAndSize(childTranslation, childSize);
    child.UpdateTransformExternal();

    offset[stackAxis] += childSize[stackAxis];

    if(!lastWidget)
      offset[stackAxis] += Spacing[stackAxis];
  }

  data.Size[stackAxis] = offset[stackAxis];
  data.Size[opAxis] = areaSize[opAxis];

  // Remove padding
  RemovePadding(Padding, data);

  return data.Size;
}

//******************************************************************************
Layout* CreateStackLayout(LayoutDirection::Enum direction, Vec2Param spacing, const Thickness& padding)
{
  return new StackLayout(direction, spacing, padding);
}

//******************************************************************************
Layout* CreateStackLayout()
{
  return new StackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero);
}

//******************************************************************************
StackLayout* StackLayout::CreateRowLayout()
{
  return new StackLayout(LayoutDirection::LeftToRight, Vec2::cZero,
    Thickness::cZero);
}

//------------------------------------------------------------- Edge Dock Layout
//******************************************************************************
ZilchDefineType(EdgeDockLayout, builder, type)
{
}

//******************************************************************************
Vec2 EdgeDockLayout::Measure(Composite* widget, LayoutArea data)
{
  return widget->GetSize();
}

//******************************************************************************
Vec2 EdgeDockLayout::DoLayout(Composite* widget, LayoutArea data)
{
  UpdateNotInLayout(widget);

  FilteredChildren children(widget);

  Vec4 area = Vec4(data.Offset.x, data.Offset.y, 
                    data.Size.x, data.Size.y);

  while(!children.Empty())
  {
    Widget& child = children.Front();
    children.PopFront();

    if(child.GetNotInLayout())
    {
      child.UpdateTransformExternal();
      continue;
    }

    Vec2 size = child.mSize;
    DockMode::Enum mode = child.GetDockMode();
    Vec3 childPos(0, 0, 0);

    if(mode & DockMode::DockTop)
      childPos.x = 0;
      
    if(mode & DockMode::DockLeft)
      childPos.y = 0;

    if(mode & DockMode::DockBottom)
      childPos.y = area[SlicesIndex::Bottom] - size.y;

    if(mode & DockMode::DockRight)
      childPos.x = area[SlicesIndex::Right] - size.x;
      
    child.SetTranslationAndSize(childPos, size);
    child.UpdateTransformExternal();
  }
  return data.Size;
}

//------------------------------------------------------------------ Dock Layout
//******************************************************************************
ZilchDefineType(DockLayout, builder, type)
{
}

//******************************************************************************
DockLayout::DockLayout(Thickness padding)
  : Layout(padding)
{
}

//******************************************************************************
Vec2 DockLayout::Measure(Composite* widget, LayoutArea data)
{
  return MaxMeasure(widget, data) + Padding.Size();
}

//******************************************************************************
Vec2 DockLayout::DoLayout(Composite* widget, LayoutArea data)
{
  UpdateNotInLayout(widget);

  FilteredChildren children(widget);    

  ApplyPadding(Padding, data);
  Vec3 offset = data.Offset;
  Vec2 size = data.Size;

  Vec4 area = Vec4(offset.x, offset.y,
    offset.x + size.x, offset.y + size.y);

  while(!children.Empty())
  {
    Widget& child = children.Front();
    children.PopFront();

    bool lastControl = children.Empty();

    Vec2 size = child.Measure(data);
    DockMode::Enum mode = child.GetDockMode();

    Vec2 areaSize = Vec2::cZero;
    Vec3 areaPos = Vec3::cZero;

    if(lastControl)
    {
      areaPos = Vec3(area[SlicesIndex::Left], area[SlicesIndex::Top], 0.0f);
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
        case DockMode::DockNone:
        case DockMode::DockFill:
        case DockMode::DockTop:
        {
          float moveY = size.y;
          areaPos = Vec3(area[SlicesIndex::Left], area[SlicesIndex::Top], 0.0f);
          area[SlicesIndex::Top] += moveY;
          areaSize = Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], moveY);
        }
        break;

        //--------------------------------------------------------------------
        case DockMode::DockBottom:
        {
          float moveY = size.y;
          areaPos = Vec3(area[SlicesIndex::Left], area[SlicesIndex::Bottom] - moveY, 0.0f);
          area[SlicesIndex::Bottom] -= moveY;
          areaSize = Vec2(area[SlicesIndex::Right] - area[SlicesIndex::Left], moveY);
        }        
        break;

        //--------------------------------------------------------------------
        case DockMode::DockLeft:
        {
          float moveX = size.x;
          areaPos = Vec3(area[SlicesIndex::Left], area[SlicesIndex::Top], 0);
          area[SlicesIndex::Left] += moveX;
          areaSize = Vec2(moveX, area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);
        }
        break;

        //--------------------------------------------------------------------
        case DockMode::DockRight:
        {
          float moveX = size.x;
          areaPos = Vec3(area[SlicesIndex::Right] - moveX, area[SlicesIndex::Top], 0.0f);
          area[SlicesIndex::Right] -= moveX;
          areaSize = Vec2(moveX, area[SlicesIndex::Bottom] - area[SlicesIndex::Top]);          
        }
        break;
      }
    }

    Vec3 childPos = areaPos;
    Vec2 childSize = size;

    SizePolicies policy = child.GetSizePolicy();

    if (policy.XPolicy == SizePolicy::Flex)
    {
      childSize.x = areaSize.x;
    }
    else
    {
      if (policy.XPolicy == SizePolicy::Fixed)
        childSize.x = policy.Size.x;
      CalculateAlignment(SizeAxis::X, child.mHorizontalAlignment, areaSize, areaPos, childSize, childPos);
    }

    if (policy.YPolicy == SizePolicy::Flex)
    {
      childSize.y = areaSize.y;
    }
    else
    {
      if (policy.YPolicy == SizePolicy::Fixed)
        childSize.y = policy.Size.y;
      CalculateAlignment(SizeAxis::Y, child.mVerticalAlignment, areaSize, areaPos, childSize, childPos);
    }

    child.SetTranslationAndSize(childPos, childSize);
    child.UpdateTransformExternal();
  }

  RemovePadding(Padding, data);
  return data.Size;
}

//----------------------------------------------------------------- Ratio Layout
//******************************************************************************
ZilchDefineType(RatioLayout, builder, type)
{
}

//******************************************************************************
RatioLayout::RatioLayout(Thickness padding)
  : Layout(padding)
{
}

//******************************************************************************
Vec2 RatioLayout::Measure(Composite* widget, LayoutArea data)
{
  return MaxMeasure(widget, data);
}

//******************************************************************************
Vec2 RatioLayout::DoLayout(Composite* widget, LayoutArea data)
{
  WidgetListRange children = widget->GetChildren();

  ApplyPadding(Padding, data);
  Vec3 pos = data.Offset;
  Vec2 size = data.Size;
  float screenRatio = size.x / size.y;

  while(!children.Empty())
  {
    Widget& child = children.Front();
    Vec2 childSize = child.GetSize();

    SizePolicies policy = child.GetSizePolicy();

    if (policy.XPolicy == SizePolicy::Fixed)
      childSize.x = policy.Size.x;

    if (policy.YPolicy == SizePolicy::Fixed)
      childSize.y = policy.Size.y;

    float sourceRatio = childSize.x / childSize.y;

    Vec2 scaleAspect = sourceRatio < screenRatio ? Vec2(sourceRatio / screenRatio, 1) : Vec2(1, screenRatio / sourceRatio);

    Vec2 newSize = size * scaleAspect;

    // Default to center the child
    Vec3 offset = ToVector3(SnapToPixels((size - newSize) * 0.5f));

    uint hAlignment = child.mHorizontalAlignment;
    if (policy.XPolicy == SizePolicy::Fixed)
    {
      hAlignment = HorizontalAlignment::Center;
      newSize.x = policy.Size.x;
    }

    uint vAlignment = child.mVerticalAlignment;
    if (policy.YPolicy == SizePolicy::Fixed)
    {
      vAlignment = VerticalAlignment::Center;
      newSize.y = policy.Size.y;
    }

    CalculateAlignment(SizeAxis::X, child.mHorizontalAlignment, size, pos, newSize, offset);
    CalculateAlignment(SizeAxis::Y, child.mVerticalAlignment, size, pos, newSize, offset);

    child.SetTranslationAndSize(offset, newSize);
    child.UpdateTransformExternal();
    children.PopFront();
  }

  data.Size = size;
  RemovePadding(Padding, data);
  return size;
}

//------------------------------------------------------------------ Grid Layout
//******************************************************************************
ZilchDefineType(GridLayout, builder, type)
{
}

//******************************************************************************
GridLayout::GridLayout(Thickness padding)
  : Layout(padding)
{
  CellSize = 10.0f;
}

//******************************************************************************
Vec2 GridLayout::Measure(Composite* widget, LayoutArea data)
{
  return MaxMeasure(widget, data);
}

//******************************************************************************
bool GridNodeComparer(const Widget &lhs, const Widget &rhs)
{
  return lhs.mSize.x * lhs.mSize.y > rhs.mSize.x * rhs.mSize.y;
}

//******************************************************************************
Vec2 GridLayout::DoLayout(Composite* widget, LayoutArea data)
{

  ApplyPadding(Padding, data);
  Vec3 pos = data.Offset;
  mSize = data.Size;

  mPlacedTiles.Clear();

  // cell-clamp the children making them fit cleanly into cells
  forRange(Widget &child, widget->GetChildren())
  {
    Vec2 childSize = child.GetSize();
    CellClamp(childSize);
    child.SetSize(childSize);
  }

  // sort from largest to smallest
  widget->mChildren.Sort(GridNodeComparer);

  // position the children in the grid
  forRange(Widget &child, widget->GetChildren())
  {
    Vec2 childSize = child.GetSize();
    CellClamp(childSize);

    TilePlacement placement;
    placement.Size = childSize;
    placement.Widget = &child;
    PlaceTile(placement);

    child.SetTranslationAndSize(ToVector3(placement.Position), placement.Size);
    child.UpdateTransformExternal();

    if (placement.Position.y + placement.Size.y > mSize.y)
      mSize.y = placement.Position.y + placement.Size.y;
  }

  data.Size = mSize;
  RemovePadding(Padding, data);
  widget->SetSize(data.Size);

  return data.Size;
}

//******************************************************************************
void GridLayout::PlaceTile(TilePlacement &toPlace)
{
  float x = Padding.Left;
  float y = Padding.Top;

  while (true)
  {
    toPlace.Position = Vec2(x, y);
    int overlapIndex = FindOverlappingTile(toPlace);

    // not overlapping any tile, valid spot found
    if (overlapIndex == -1)
    {
      mPlacedTiles.PushBack(toPlace);
      return;
    }

    TilePlacement &overlapTile = mPlacedTiles[overlapIndex];
    x = overlapTile.Position.x + overlapTile.Size.x;

    if (x + toPlace.Size.x > mSize.x)
    {
      x = Padding.Left;
      y += CellSize;
    }

  }

}

//******************************************************************************
int GridLayout::FindOverlappingTile(TilePlacement &placement)
{
  for (uint i = 0; i < mPlacedTiles.Size(); ++i)
  {
    TilePlacement &placed = mPlacedTiles[i];
    if (placed.Overlaps(placement))
      return i;
  }
  return -1;
}

//******************************************************************************
void GridLayout::CellClamp(Vec2 &vec)
{
  vec.x = Math::Ceil(vec.x / CellSize) * CellSize;
  vec.y = Math::Ceil(vec.y / CellSize) * CellSize;
  vec.x = (vec.x < CellSize) ? CellSize : vec.x;
  vec.y = (vec.y < CellSize) ? CellSize : vec.y;
}

//******************************************************************************
bool GridLayout::TilePlacement::Overlaps(const TilePlacement &rhs)
{
  if (Position.x >= rhs.Position.x + rhs.Size.x) return false;
  if (Position.x + Size.x <= rhs.Position.x) return false;
  if (Position.y >= rhs.Position.y + rhs.Size.y) return false;
  if (Position.y + Size.y <= rhs.Position.y) return false;
  return true;
}

Layout* CreateDockLayout()
{
  return new DockLayout();
}

Layout* CreateDockLayout(const Thickness& padding)
{ 
  return new DockLayout(padding);
}

Layout* CreateRowLayout()
{
  return StackLayout::CreateRowLayout();
}

Layout* CreateFillLayout(Thickness padding)
{
  return new FillLayout(padding);
}

Layout* CreateFillLayout(const Thickness& padding)
{
  return new FillLayout(padding);
}

Layout* CreateEdgeDockLayout()
{ 
  return new EdgeDockLayout();
}

Layout* CreateRatioLayout()
{
  return new RatioLayout();
}

LayoutResult RemoveThickness(Thickness thickness, Vec2Param outerSize, Vec3Param offset)
{
  LayoutResult r;
  r.Size.x = outerSize.x - (thickness.Left + thickness.Right);
  r.Size.y = outerSize.y - (thickness.Top + thickness.Bottom);
  r.Translation = Vec3(thickness.TopLeft(), 0) + offset;
  return r;
}

void PlaceWithLayout(LayoutResult& result, Widget* widget)
{
  widget->SetTranslation(result.Translation);
  widget->SetSize(result.Size);
}

}//namespace Zero
