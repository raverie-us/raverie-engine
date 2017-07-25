///////////////////////////////////////////////////////////////////////////////
///
/// \file GraphWidget.cpp
/// Implementation of the GraphWidget.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace GraphWidgetUi
{
const cstr cLocation = "EditorUi/ResourceEditors/SampleCurve";
Tweakable(Vec4,  PrimaryLineColor,   Vec4(1,1,1,1), cLocation);
Tweakable(float, PrimaryLineWidth,   1.0f,          cLocation);
Tweakable(Vec4,  SecondaryLineColor, Vec4(1,1,1,1), cLocation);
Tweakable(float, SecondaryLineWidth, 1.0f,          cLocation);
}

GraphWidget::GraphWidget(Composite* parent)
  : Widget(parent)
{
  mWidthRange[MIN] = 0.0f;
  mWidthRange[MAX] = 1.0f;
  mHeightRange[MIN] = 0.0f;
  mHeightRange[MAX] = 1.0f;
  
  mDrawLabels = true;
  mHideLastLabel = false;
  mFlags.SetAll();
}

void AddHashLines(Array<Vec3>& lines, Vec3 pos, float size, Vec3 dir, Vec3 hashDir, uint count, uint offset, float spacing)
{
  for (uint i = offset; i < count + 1; ++i)
  {
    Vec3 startPos = SnapToPixels(dir * (i * spacing));
    Vec3 endPos = startPos + hashDir * size;
    lines.PushBack(pos + startPos);
    lines.PushBack(pos + endPos);
  }
}

void GraphWidget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Array<StreamedVertex>& streamedVertices = frameBlock.mRenderQueues->mStreamedVertices;

  // Get the spacing for each
  float widthRange = GetWidthRange();
  float widthSpacing = GetWidthSpacing();
  float heightRange = GetHeightRange();
  float heightSpacing = GetHeightSpacing();

  // Hot-spots
  Vec3 upperLeft = Vec3::cZero;
  Vec3 upperRight = upperLeft + Vec3(mSize.x, 0, 0);
  Vec3 lowerLeft = upperLeft + Vec3(0, mSize.y, 0);
  Vec3 lowerRight = lowerLeft + Vec3(mSize.x, 0, 0);

  // Offsets to deal with rendering issues? Would investigate further, but old UI should be
  // removed somewhat soon
  upperLeft += Vec3(0.5f, 0, 0);
  lowerLeft += Vec3(0.5f, -0.5f, 0);
  lowerRight += Vec3(0, -0.5f, 0);

  Vec4 primaryColor = GraphWidgetUi::PrimaryLineColor;
  Vec4 secondaryColor = GraphWidgetUi::SecondaryLineColor;
  //float primaryWidth = GraphWidgetUi::PrimaryLineWidth;
  //float secondaryWidth = GraphWidgetUi::SecondaryLineWidth;

  Array<Vec3> primaryLines;
  Array<Vec3> secondaryLines;

  // Border lines
  primaryLines.PushBack(upperLeft);
  primaryLines.PushBack(upperRight);
  primaryLines.PushBack(upperRight);
  primaryLines.PushBack(lowerRight);
  primaryLines.PushBack(lowerRight);
  primaryLines.PushBack(lowerLeft);
  primaryLines.PushBack(lowerLeft);
  primaryLines.PushBack(upperLeft);

  // Horizontal lines
  if (mFlags.IsSet(GraphAxes::AxisX))
  {
    float hashCountf = (widthRange / widthSpacing);
    uint hashCount = (uint)hashCountf;
    float spacing = mSize.y / hashCountf;

    AddHashLines(primaryLines, lowerLeft, mSize.x, -Vec3::cYAxis, Vec3::cXAxis, hashCount, 1, spacing);
    AddHashLines(secondaryLines, lowerLeft + Vec3(0, -spacing * 0.5f, 0), mSize.x, -Vec3::cYAxis, Vec3::cXAxis, hashCount - 1, 0, spacing);
  }
  
  // Vertical lines
  if (mFlags.IsSet(GraphAxes::AxisY))
  {
    float hashCountf = (heightRange / heightSpacing);
    uint hashCount = (uint)hashCountf;
    float spacing = mSize.x / hashCountf;

    AddHashLines(primaryLines, lowerLeft, mSize.y, Vec3::cXAxis, -Vec3::cYAxis, hashCount, 1, spacing);
    AddHashLines(secondaryLines, lowerLeft + Vec3(spacing * 0.5f, 0, 0), mSize.y, Vec3::cXAxis, -Vec3::cYAxis, hashCount - 1, 0, spacing);
  }

  // Render data for lines
  ViewNode& lineNode = AddRenderNodes(viewBlock, frameBlock, clipRect, TextureManager::Find("White"));
  lineNode.mStreamedVertexType = PrimitiveType::Lines;

  for (uint i = 0; i < secondaryLines.Size(); ++i)
  {
    StreamedVertex vertex(Math::TransformPoint(lineNode.mLocalToView, secondaryLines[i]), Vec2::cZero, secondaryColor);
    streamedVertices.PushBack(vertex);
  }

  for (uint i = 0; i < primaryLines.Size(); ++i)
  {
    StreamedVertex vertex(Math::TransformPoint(lineNode.mLocalToView, primaryLines[i]), Vec2::cZero, primaryColor);
    streamedVertices.PushBack(vertex);
  }

  lineNode.mStreamedVertexCount = streamedVertices.Size() - lineNode.mStreamedVertexStart;

  if (mDrawLabels)
  {
    RenderFont* font = FontManager::GetInstance()->GetRenderFont("NotoSans-Regular", 11, 0);
    ViewNode& textNode = AddRenderNodes(viewBlock, frameBlock, clipRect, font->mTexture);
    FontProcessor fontProcessor(frameBlock.mRenderQueues, &textNode, primaryColor);

    if (mFlags.IsSet(GraphAxes::AxisX))
    {
      float hashCountf = (widthRange / widthSpacing);
      uint hashCount = (uint)hashCountf;
      float spacing = mSize.y / hashCountf;
      String format = GetLabelFormat(widthSpacing);

      uint count = mHideLastLabel ? hashCount : hashCount + 1;
      for (uint i = 0; i < count; ++i)
      {
        Vec3 pos = lowerLeft - Vec3(0, spacing * i, 0);
        float val = mWidthRange[MIN] + float(i) * widthSpacing;
        String text = String::Format(format.c_str(), val);

        Vec2 textSize = font->MeasureText(text);
        pos.x -= textSize.x + Pixels(2);
        pos.y -= textSize.y * 0.5f;
        AddTextRange(fontProcessor, font, text, Math::ToVector2(SnapToPixels(pos)), TextAlign::Left, Vec2(1, 1), mSize, false);
      }
    }

    if (mFlags.IsSet(GraphAxes::AxisY))
    {
      float hashCountf = (heightRange / heightSpacing);
      uint hashCount = (uint)hashCountf;
      float spacing = mSize.x / hashCountf;
      String format = GetLabelFormat(heightSpacing);

      uint count = mHideLastLabel ? hashCount : hashCount + 1;
      for (uint i = 0; i < hashCount + 1; ++i)
      {
        Vec3 pos = lowerLeft + Vec3(spacing * i, 0, 0);
        float val = mHeightRange[MIN] + float(i) * heightSpacing;
        String text = String::Format(format.c_str(), val);

        Vec2 textSize = font->MeasureText(text);
        pos.x -= textSize.x * 0.5f;
        pos.y += textSize.y * 0.5f;
        AddTextRange(fontProcessor, font, text, Math::ToVector2(SnapToPixels(pos)), TextAlign::Left, Vec2(1, 1), mSize, false);
      }
    }
  }
}

Vec2 GraphWidget::ToGraphPositionScaled(Vec2Param pixelPos)
{
  Vec2 range(GetHeightRange(), GetWidthRange());
  return ToGraphPosition(pixelPos) * range;
}

Vec2 GraphWidget::ToPixelPositionScaled(Vec2Param graphPos)
{
  Vec2 local(graphPos.x, -graphPos.y);
  Vec2 origin = GetGridOrigin();
  Vec2 range(GetHeightRange(), GetWidthRange());
  Vec2 world = origin + mSize / range * local;
  return world;
}

Vec2 GraphWidget::ToGraphPosition(Vec2Param pixelPos)
{
  Vec2 origin = GetGridOrigin();
  Vec2 local = (pixelPos - origin) / mSize;
  return Vec2(local.x, -local.y);
}

Vec2 GraphWidget::ToPixelPosition(Vec2Param graphPos)
{
  Vec2 local(graphPos.x, -graphPos.y);
  Vec2 origin = GetGridOrigin();
  Vec2 world = origin + mSize * local;
  return world;
}

bool GraphWidget::WorldPointInGraph(Vec2Param worldPos)
{
  // Get the local position
  Vec2 local = ToGraphPosition(worldPos);

  // Check if it's in the current range
  Vec2 range(GetHeightRange(), GetWidthRange());

  Vec2 extents = local / range;

  return !(extents.x < 0.0f || extents.x > 1.0f ||
           extents.y < 0.0f || extents.y > 1.0f);
}

Vec2 GraphWidget::ClampPos(Vec2Param worldPos)
{
  // Get the local position
  Vec2 local = ToGraphPosition(worldPos);

  local.x = Math::Clamp(local.x, 0.0f, 1.0f);
  local.y = Math::Clamp(local.y, 0.0f, 1.0f);
  return ToPixelPosition(local);
}

void GraphWidget::SetWidthMin(float min)
{
  mWidthRange[MIN] = min;
}

void GraphWidget::SetWidthMax(float max)
{
  mWidthRange[MAX] = max;
}

void GraphWidget::SetWidthRange(float min, float max)
{
  SetWidthMin(min);
  SetWidthMax(max);
}

void GraphWidget::SetHeightMin(float min)
{
  mHeightRange[MIN] = min;
}

void GraphWidget::SetHeightMax(float max)
{
  mHeightRange[MAX] = max;
}

void GraphWidget::SetHeightRange(float min, float max)
{
  SetHeightMin(min);
  SetHeightMax(max);
}

bool GraphWidget::GetDrawAxisX()
{
  return mFlags.IsSet(GraphAxes::AxisX);
}

void GraphWidget::SetDrawAxisX(bool state)
{
  return mFlags.SetState(GraphAxes::AxisX,state);
}

bool GraphWidget::GetDrawAxisY()
{
  return mFlags.IsSet(GraphAxes::AxisY);
}

void GraphWidget::SetDrawAxisY(bool state)
{
  return mFlags.SetState(GraphAxes::AxisY,state);
}

Vec2 GraphWidget::GetGridOrigin()
{
  return Vec2(0, mSize.y);
}

float GraphWidget::GetWidthRange()
{
  return mWidthRange[MAX] - mWidthRange[MIN];
}

float GraphWidget::GetHeightRange()
{
  return mHeightRange[MAX] - mHeightRange[MIN];
}

float GraphWidget::GetWidthSpacing()
{
  return GetIntervalSize(GetWidthRange(), mSize.y);
}

float GraphWidget::GetHeightSpacing()
{
  return GetIntervalSize(GetHeightRange(), mSize.x);
}

String GraphWidget::GetLabelFormat(float spacing)
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

float GetNextStep(int index)
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

float GraphWidget::GetIntervalSize(float range, float space)
{
  static const float cMinDistance = Pixels(40);
  static const float cMaxDistance = Pixels(300);

  // I don't know why...
  space -= 1.0f;

  uint hashCount;
  float dist;

  // This is a temporary fix, because the below function can infinite loop in certain cases
  int count = 0;
  const int cMaxCount = 100;

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
    dist = space / hashCount;

    if(dist > cMinDistance)
    {
      // If we're both over the min and under the max, we found our step
      if(dist < cMaxDistance)
        break;
      else
        step = GetNextStep(--index);
    }
    else
    {
      // Get the next step
      step = GetNextStep(++index);
    }
    ++count;

    if (count >= cMaxCount)
      break;
  }

   return step;
}

}// namespace Zero
