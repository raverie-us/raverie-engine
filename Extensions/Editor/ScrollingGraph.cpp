///////////////////////////////////////////////////////////////////////////////
///
/// \file ScrollingGraph.hpp
/// Declaration of ScrollingGraph helper class.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
ScrollingGraph::ScrollingGraph(Vec2Param hashSize)
{
  mPixelsPerUnit = hashSize;
  Zoom = Vec2(1,1);
  Translation = Vec2(0,0);
  mActions = NULL;
  mGridScale = Vec2(1,1);

  const float cDefaultZoomBounds = 10000.0f;
  ZoomMin = Vec2(1, 1) * 1.0f / cDefaultZoomBounds;
  ZoomMax = Vec2(1, 1) * cDefaultZoomBounds;
}

//******************************************************************************
Vec2 ScrollingGraph::ToGraphSpace(Vec2Param pixelPos)
{
  Vec2 hashSize = mPixelsPerUnit * Zoom;

  Vec2 graphPos = pixelPos / hashSize;
  graphPos -= Translation;
  return graphPos;
}

//******************************************************************************
Vec2 ScrollingGraph::ToPixels(Vec2Param graphPos)
{
  Vec2 hashSize = mPixelsPerUnit * Zoom;

  Vec2 pixelPos = graphPos + Translation;
  pixelPos *= hashSize;
  return pixelPos;
}

//******************************************************************************
float ScrollingGraph::GetDisplayInterval(float clientArea, uint axis)
{
  // The range of units in pixels we're displaying
  float displayRange = (clientArea / mPixelsPerUnit[axis]) / Zoom[axis];
  displayRange *= mGridScale[axis];

  return GetIntervalSize(displayRange, clientArea);
}

//******************************************************************************
Vec2 ScrollingGraph::GetDisplayInterval(Vec2 clientArea)
{
  float intervalX = GetDisplayInterval(clientArea.x, 0);
  float intervalY = GetDisplayInterval(clientArea.y, 1);
  return Vec2(intervalX, intervalY);
}

//******************************************************************************
Vec2 ScrollingGraph::SnapGraphPos(Vec2Param graphPos, Vec2Param clientArea)
{
  Vec2 displayInterval = GetDisplayInterval(clientArea);
  float x = Math::Round(graphPos.x * mGridScale.x / displayInterval.x) / mGridScale.x * displayInterval.x;
  float y = Math::Round(graphPos.y * mGridScale.y / displayInterval.y) / mGridScale.y * displayInterval.y;

  return Vec2(x, y);
}

//******************************************************************************
Vec2 ScrollingGraph::SnapPixelPos(Vec2Param pixelPos, Vec2Param clientArea)
{
  // Convert to graph space
  Vec2 graphPos = ToGraphSpace(pixelPos);

  // Snap to the display intervals
  Vec2 snappedPos = SnapGraphPos(graphPos, clientArea);

  // Bring back to pixels
  Vec2 snappedPixels = ToPixels(snappedPos);

  return snappedPixels;
}

//******************************************************************************
void ScrollingGraph::ScrollGraph(Vec2Param graphSpaceScrollDirection)
{
  // Origin is already in graph space, so just add to it
  Translation += graphSpaceScrollDirection;
}

//******************************************************************************
void ScrollingGraph::ScrollPixels(Vec2Param pixelScrollDirection)
{
  // Convert to graph space and scroll
  Vec2 hashSize = mPixelsPerUnit * Zoom;
  Vec2 graphSpaceScrollDirection = pixelScrollDirection / hashSize;
  
  ScrollGraph(graphSpaceScrollDirection);
}

//******************************************************************************
void ScrollingGraph::PanToTranslation(Vec2Param graphPos, float animTime)
{
  if(mActions)
  {
    // Clear anything that was happening before
    mActions->Cancel();

    // We want the origin and zoom to animate at the same time
    ActionGroup* group = new ActionGroup();

    // Animate the Translation
    Action* origin = AnimatePropertyGetSet(ScrollingGraph, Translation, 
                                       Ease::Quad::Out, this, animTime, graphPos);
    group->Add(origin);

    // Add the action group
    mActions->Add(group, ActionExecuteMode::FrameUpdate);
  }
  else
  {
    Translation = graphPos;
  }
}

//******************************************************************************
void ScrollingGraph::Frame(Vec2Param min, Vec2Param max,
                           Vec2Param clientSize, IntVec2 axes, 
                           Vec2 pixelPadding, float animTime)
{
  // Calculate the zoom without the padding
  Vec2 targetZoom = Zoom;
  targetZoom = (clientSize / mPixelsPerUnit) / (max - min);

  // Now calculate the padding based on the new zoom
  Vec2 hashSize = mPixelsPerUnit * targetZoom;
  Vec2 graphPadding = pixelPadding / hashSize;

  Vec2 targetOrigin = Translation;
  targetOrigin = -min + graphPadding;
  targetZoom = (clientSize / mPixelsPerUnit) / (graphPadding * 2.0f + max - min);

  targetOrigin.x = Math::Min(targetOrigin.x, 0.0f);
  
  if(axes.x == 0)
  {
    targetOrigin.x = Translation.x;
    targetZoom.x = Zoom.x;
  }
  if(axes.y == 0)
  {
    targetOrigin.y = Translation.y;
    targetZoom.y = Zoom.y;
  }

  if(mActions)
  {
    // Clear anything that was happening before
    mActions->Cancel();

    // We want the origin and zoom to animate at the same time
    ActionGroup* group = new ActionGroup();

    // Animate the Origin
    Action* origin = AnimatePropertyGetSet(ScrollingGraph, Translation, 
                                 Ease::Quad::Out, this, animTime, targetOrigin);
    group->Add(origin);

    // Animate the Zoom
    Action* zoom = AnimatePropertyGetSet(ScrollingGraph, Zoom, Ease::Quad::Out,
                                         this, animTime, targetZoom);
    group->Add(zoom);

    // Add the action group
    mActions->Add(group, ActionExecuteMode::FrameUpdate);
  }
  else
  {
    Translation = targetOrigin;
    Zoom = targetZoom;
  }
}

//******************************************************************************
ScrollingGraph::range ScrollingGraph::GetWidthHashes(float clientWidth,
                                                     bool halfHashes)
{
  // The range of units in local space we're displaying
  float displayRange = (clientWidth / mPixelsPerUnit.x) / Zoom.x;
  displayRange *= mGridScale.x;

  float spacing = GetIntervalSize(displayRange, clientWidth);
  return range(Translation.x, displayRange, spacing, halfHashes, mGridScale.x);
}

//******************************************************************************
ScrollingGraph::range ScrollingGraph::GetHeightHashes(float clientHeight,
                                                      bool halfHashes)
{
  // The range of units in local space we're displaying
  float displayRange = (clientHeight / mPixelsPerUnit.y) / Zoom.y;
  displayRange *= mGridScale.y;

  float spacing = GetIntervalSize(displayRange, clientHeight);
  return range(Translation.y, displayRange, spacing, halfHashes, mGridScale.y);
}

//******************************************************************************
String ScrollingGraph::GetWidthFormat(float clientWidth)
{
  // The range of units in local space we're displaying
  float displayRange = (clientWidth / mPixelsPerUnit.x) / Zoom.x;
  displayRange *= mGridScale.x;

  float spacing = GetIntervalSize(displayRange, clientWidth);
  return GetLabelFormat(spacing);
}

//******************************************************************************
String ScrollingGraph::GetHeightFormat(float clientHeight)
{
  // The range of units in local space we're displaying
  float displayRange = (clientHeight / mPixelsPerUnit.y) / Zoom.y;
  displayRange *= mGridScale.y;

  float spacing = GetIntervalSize(displayRange, clientHeight);
  return GetLabelFormat(spacing);
}

//******************************************************************************
Vec2 ScrollingGraph::GetTranslation()
{
  return Translation;
}

//******************************************************************************
void ScrollingGraph::SetTranslation(Vec2Param translation)
{
  Translation = translation;
}

//******************************************************************************
Vec2 ScrollingGraph::GetZoom()
{
  return Zoom;
}

//******************************************************************************
void ScrollingGraph::SetZoom(Vec2Param zoom)
{
  Zoom.x = Math::Clamp(zoom.x, ZoomMin.x, ZoomMax.x);
  Zoom.y = Math::Clamp(zoom.y, ZoomMin.y, ZoomMax.y);
}

//******************************************************************************
void ScrollingGraph::ZoomAtPosition(Vec2Param pixelPos, Vec2Param zoom)
{
  // To zoom at a given position, we want to calculate the position on the
  // graph where the given position is, zoom, recalculate the position,
  // and translate by the difference in position to stay at the same spot

  // Calculate the position in graph space
  Vec2 graphPos = ToGraphSpace(pixelPos);

  // Zoom
  SetZoom(Zoom + zoom);

  // Recalculate
  Vec2 newGraphPos = ToGraphSpace(pixelPos);

  // Translate by the difference
  Translation += (newGraphPos - graphPos);
}

//******************************************************************************
String ScrollingGraph::GetLabelFormat(float spacing)
{
  uint decimals;
  if(spacing > 0.9f)
    decimals = 0;
  else if(spacing > 0.09f)
    decimals = 1;
  else if(spacing > 0.009)
    decimals = 2;
  else if(spacing > 0.0009)
    decimals = 3;
  else
    decimals = 4;

  return String::Format("%%0.%if", decimals);
}

//******************************************************************************
float ScrollingGraph::GetNextStep(int index)
{
  // If it is even
  if (index % 2 == 0)
  {
    int i = index / 2;
    return (float)Math::Pow(10.0f, (float)i);
  }
  else
  {
    int i = (index + 1) / 2;
    return (float)Math::Pow(10.0f, (float)i) / 2.0f;
  }
}

//******************************************************************************
float ScrollingGraph::GetIntervalSize(float range, float clientSpace)
{
  const float cMinDistance = Pixels(35);
  const float cMaxDistance = Pixels(350);

  uint hashCount;
  float dist;

  // At index = -2, the step is 0.1 (which by default we want to 
  // start searching from)
  int index = -2;
  float step = GetNextStep(index);
  while(true)
  {
    // Get the amount of hash marks that will fit
    hashCount = (uint)(range / step);

    if(hashCount == 0)
      return range;

    // Get the distance between each hash
    dist = clientSpace / hashCount;

    if(dist > cMaxDistance)
      step = GetNextStep(--index);
    else if(dist < cMinDistance)
      step = GetNextStep(++index);
    else
      break;
  }

  return step;
}

//------------------------------------------------------------------------ range
//******************************************************************************
ScrollingGraph::range::range(float origin, float range, float spacing,
                             bool halfHash, float gridScale)
{
  mOrigin = origin * gridScale;
  mOffset = Math::FMod(mOrigin, spacing);
  mHashCount = (uint)(range / spacing) + 1;
  mCurrentHash = 0;
  mSpacing = spacing;
  mHalfHash = halfHash;
  mGridScale = gridScale;
}

//******************************************************************************
ScrollingGraph::HashMark ScrollingGraph::range::Front()
{
  String format = GetLabelFormat(mSpacing);
  float graphPosition = -mOrigin + mSpacing * float(mCurrentHash) + mOffset;

  if(mHalfHash)
    graphPosition += mSpacing * 0.5f;

  HashMark entry;
  entry.Position = graphPosition / mGridScale;
  entry.Label = String::Format(format.c_str(), graphPosition);

  return entry;
}

//******************************************************************************
void ScrollingGraph::range::PopFront()
{
  ++mCurrentHash;
}

//******************************************************************************
bool ScrollingGraph::range::Empty()
{
  if(mHalfHash)
    return mCurrentHash == mHashCount;
  return mCurrentHash == (mHashCount + 1);
}

}//namespace Zero
