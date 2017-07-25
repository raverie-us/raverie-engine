///////////////////////////////////////////////////////////////////////////////
///
/// \file GraphView.cpp
/// Implementation of the GraphView.
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{


const uint maxSamples = 60;

void GraphEntry::Update()
{
  float newSample = Sampler->Sample();

  if(newSample > data.MaxValue)
    data.MaxValue = newSample;
  if(newSample < data.MinValue)
    data.MinValue = newSample;

  Values.PushBack(newSample);

  if(Values.Size() > maxSamples)
    Values.Erase(Values.Data());
}

float GraphEntry::Normalize(float value)
{
  float r = data.MaxValue - data.MinValue;
  return (value - data.MinValue) / r;
}

GraphView::GraphView(Composite* parent)
  :Widget(parent)
{
  mColors.PushBack( Color::Purple );
  mColors.PushBack( Color::Khaki );
  mColors.PushBack( Color::YellowGreen);
  mColors.PushBack( Color::Cyan );
  mColors.PushBack( Color::Blue );
  mColors.PushBack( Color::Red );

  mFont = FontManager::GetInstance()->GetRenderFont("NotoSans-Regular", 11, 0);

  mLabels = true;

  mMouseOverLabel = -1;

  //set the size policy to auto expand our size layout
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MouseMove, OnMouseMove);
  //ConnectThisTo(mGraph, Events::MouseScroll, OnMouseScroll);
  //ConnectThisTo(mSetKey, Events::ButtonPressed, OnSetKeyPressed);
}

void GraphView::AddEntry(GraphEntry* entry)
{
  //get a color for this entry and remove it from the list of available colors
  entry->Color = mColors.Back();
  mColors.PopBack();

  entry->Label.Translation = GetLabelPosition(Entries.Size());
  entry->Label.Size = mFont->MeasureText(entry->Label.Name, 1.0f);
  Entries.PushBack(entry);
}

void GraphView::AddSampler(DataSampler* sampler)
{
  GraphEntry* entry = new GraphEntry();
  entry->Sampler = sampler;
  sampler->Setup(entry->data, entry->Label);

  AddEntry(entry);
}

void GraphView::OnSetKeyPressed(ObjectEvent* event)
{

}

void GraphView::OnMouseDown(MouseEvent* event)
{
  
}

void GraphView::OnMouseMove(MouseEvent* event)
{
  Vec3 widgetPos = GetTranslation();
  Vec2 mousePos = event->Position - Vec2(widgetPos.x, widgetPos.y);
  
  DebugPrint("[%g, %g]\n", mousePos.x, mousePos.y);
  // Draw the label for each graph entry
  for(uint i = 0; i < Entries.Size(); ++i)
  {
    EntryLabel& label = Entries[i]->Label;
    if(label.PointContains(mousePos))
    {
      mMouseOverLabel = i;
      return;
    }
  }
}

void GraphView::OnClose(ObjectEvent* event)
{

}

void GraphView::OnMouseScroll(MouseEvent* event)
{
  
}

void GraphView::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  static RenderFont* font = mFont;

  Vec2 size = mSize;
  // Draw the grid
  DrawGrid(size, font, viewBlock, frameBlock, clipRect);

  // Draw the color coded labels at the upper 
  // left and the value for the grid lines
  if(mLabels)
    DrawLabels(font, viewBlock, frameBlock, clipRect);

  // Draw the graph
  DrawLineGraph(size, viewBlock, frameBlock, clipRect);
}

Vec3 GraphView::GetLabelPosition(uint labelNumber)
{
  return Vec3(25, float(labelNumber) * mFont->mFontHeight, 0.0f);
}

void GraphView::DrawLabels(RenderFont* font, ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, font->mTexture);
  
  // Draw the label for each graph entry
  for(uint i = 0; i < Entries.Size(); ++i)
  {
    GraphEntry* entry = Entries[i];
    EntryLabel& label = entry->Label;
    float last = entry->Values.Empty() ?  0.0f : entry->Values.Back();
    String text = String::Format("%s %g", label.Name.c_str(), last);

    FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(entry->Color));
    AddTextRange(fontProcessor, font, text, ToVector2(label.Translation), TextAlign::Left, Vec2(1, 1), mSize);
    //if(i == mMouseOverLabel)
    //  label.DrawSelection(render);
  }
}

void GraphView::DrawLineGraph(Vec2Param size, ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  // For each entry, draw it's graph on the grid
  const float sampleSpacing = size.x / maxSamples;

  const float startPosX = 0.0f;

  static Array<StreamedVertex> lines;
  lines.Clear();
  
  // Walk through each entry
  forRange(GraphEntry* entry, Entries.All())
  {
    // Get a new sample from the entry
    entry->Update();

    Vec4 lineColor = ToFloatColor(entry->Color);

    // Plot each value for this entry on the graph
    Array<float>::range values = entry->Values.All();

    // Do nothing if there are no samples
    if(!values.Empty())
    {
      float prev = values.Front();
      values.PopFront();

      // Normalize the entry if specified
      if(entry->data.AutoNormalized)
        prev = entry->Normalize(prev);

      float samplePos = startPosX;
      float scaleY = -size.y;

      // Walk each sample and draw a line from sample to sample
      forRange(float current, values)
      {
        // Normalize the current entry if specified
        if(entry->data.AutoNormalized)
          current = entry->Normalize(current);

        // Move to the next position along the x-axis
        float nextPos = samplePos + sampleSpacing;

        // Draw the line segment
        Vec3 start(samplePos, (prev * scaleY) + size.y, 0);
        Vec3 end(nextPos, (current * scaleY) + size.y, 0);

        lines.PushBack(StreamedVertex(start, Vec2(), lineColor));
        lines.PushBack(StreamedVertex(end, Vec2(), lineColor));

        samplePos = nextPos;
        prev = current;
      }
    }

    // Draw all of the lines for this entry
    CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
  }
}

void GraphView::DrawGrid(Vec2Param size, RenderFont* font, ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, font->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(Color::Black));

  const float spacing = 1.0f / 100.0f;
  // Draw the value that each grid line represents
  for(uint i = 0; i <= 100; i += 10)
  {
    String num = String::Format("%d", i);
    Vec3 pos = Vec3(0, mSize.y - i * spacing * mSize.y, 0);
    AddTextRange(fontProcessor, font, num, ToVector2(mTranslation) + ToVector2(pos), TextAlign::Left, Vec2(1, 1), mSize, false);
  }

  static Array<StreamedVertex> lines;
  lines.Clear();
  // Draw the horizontal grid lines which are used to help visualize
  // What a given value on a grid line is
  Vec4 lineColor = ToFloatColor(Color::DimGray);
  for(uint i = 0; i <= 100; i += 10)
  {
    Vec3 pos = SnapToPixels(Vec3(0, mSize.y - i * spacing * mSize.y, 0));
    Vec3 endPos = pos;
    endPos.x = mSize.x;
    lines.PushBack(StreamedVertex(pos, Vec2(), lineColor));
    lines.PushBack(StreamedVertex(endPos, Vec2(), lineColor));
  }
  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
}

void GraphView::DrawPieGraph()
{

}

}//namespace Zero
