///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------PerformanceGraphWidget
PerformanceGraphWidget::PerformanceGraphWidget(Composite* parent)
  : Widget(parent)
{
}

void AddLine(Vec3 pos0, Vec3 pos1, Vec4 color, Array<StreamedVertex>& vertices)
{
  vertices.PushBack(StreamedVertex(pos0, Vec2(), color));
  vertices.PushBack(StreamedVertex(pos1, Vec2(), color));
}

void PerformanceGraphWidget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  mGraphPos = mTranslation + Vec3(1, 1, 0);
  mGraphSize = Vec3(mSize) + Vec3(-2, -2, 0);

  uint averageFreq = 1;
  static uint sUpdateCount = averageFreq - 1;

  Array<Profile::Record*>::range records = Profile::ProfileSystem::Instance->GetRecords();
  if (records.Empty())
    return;

  Profile::Record* mainRecord = records.Front();
  mainRecord->Update();

  ++sUpdateCount;
  if (sUpdateCount == averageFreq)
  {
    sUpdateCount = 0;
    mainRecord->AverageRunningSample();
    Profile::Record::sSampleIndex = (Profile::Record::sSampleIndex + 1) % Profile::Record::cSampleCount;
  }

  static Array<StreamedVertex> lines;
  static Array<StreamedVertex> triangles;
  lines.Clear();
  triangles.Clear();

  uint recordCount = 1;
  float samplesTotal[Profile::Record::cSampleCount] = {};
  forRange (Profile::Record& record, mainRecord->GetChildren())
  {
    DrawSamples(viewBlock, frameBlock, lines, triangles, record.mSamples, record.GetColor());
    ++recordCount;

    for (uint i = 0; i < Profile::Record::cSampleCount; ++i)
      samplesTotal[i] += record.mSamples[i];
  }
  DrawSamples(viewBlock, frameBlock, lines, triangles, samplesTotal, Color::White, true);

  CreateRenderData(viewBlock, frameBlock, clipRect, triangles, PrimitiveType::Triangles);


  Vec4 color = ToFloatColor(Color::Black);
  Vec3 tl = mGraphPos;
  Vec3 tr = mGraphPos + Vec3(mGraphSize.x, 0, 0);
  Vec3 bl = mGraphPos + Vec3(0, mGraphSize.y, 0);
  Vec3 br = mGraphPos + mGraphSize;

  AddLine(tl, tr, color, lines);
  AddLine(tr, br, color, lines);
  AddLine(br, bl, color, lines);
  AddLine(bl, tl, color, lines);

  AddLine(tl + Vec3(0, mGraphSize.y * 0.333f, 0), tr + Vec3(0, mGraphSize.y * 0.333f, 0), color, lines);
  AddLine(tl + Vec3(0, mGraphSize.y * 0.667f, 0), tr + Vec3(0, mGraphSize.y * 0.667f, 0), color, lines);

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);

  RenderFont* font = FontManager::GetInstance()->GetRenderFont("NotoSans-Regular", 11, 0);
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, font->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(Color::Black));

  float seconds = Profile::ProfileSystem::Instance->GetTimeInSeconds((Profile::ProfileTime)mainRecord->SmoothAverage());
  float fps = (seconds != 0.0f) ? Math::Round(1.0f / seconds) : 1.0f;
  String text = String::Format(" FPS: %0.0f", fps);
  AddTextRange(fontProcessor, font, text, Vec2(0, 0), TextAlign::Left, Vec2(1, 1), mSize, true);

  text = String("25.0ms (40fps) ");
  ProcessTextRange(fontProcessor, font, text, ToVector2(mGraphPos), TextAlign::Right, Vec2(1, 1), Vec2(mGraphSize.x, font->mLineHeight));

  text = String("16.7ms (60fps) ");
  ProcessTextRange(fontProcessor, font, text, ToVector2(mGraphPos) + Vec2(0, Math::Round(mGraphSize.y * 0.333f)), TextAlign::Right, Vec2(1, 1), Vec2(mGraphSize.x, font->mLineHeight));

  text = String("8.3ms (120fps) ");
  ProcessTextRange(fontProcessor, font, text, ToVector2(mGraphPos) + Vec2(0, Math::Round(mGraphSize.y * 0.667f)), TextAlign::Right, Vec2(1, 1), Vec2(mGraphSize.x, font->mLineHeight));

  Vec2 namePos = Vec2(0, 20.0f * recordCount);
  forRange (Profile::Record& record, mainRecord->GetChildren())
  {
    float seconds = Profile::ProfileSystem::Instance->GetTimeInSeconds((Profile::ProfileTime)record.SmoothAverage());
    text = String::Format(" %0.1fms %s", seconds * 1000.0f, record.GetName());
    fontProcessor.mVertexColor = ToFloatColor(record.GetColor());
    AddTextRange(fontProcessor, font, text, namePos, TextAlign::Left, Vec2(1, 1), mSize, true);
    namePos -= Vec2(0, 20);
  }

  // total
  {
    Profile::Record& record = *mainRecord;
    float seconds = Profile::ProfileSystem::Instance->GetTimeInSeconds((Profile::ProfileTime)samplesTotal[Profile::Record::sSampleIndex - 1]);
    text = String::Format(" %0.1fms %s Total", seconds * 1000.0f, record.GetName());
    fontProcessor.mVertexColor = ToFloatColor(record.GetColor());
    AddTextRange(fontProcessor, font, text, namePos, TextAlign::Left, Vec2(1, 1), mSize, true);
    namePos -= Vec2(0, 20);
  }
}

Vec3 PerformanceGraphWidget::GetPosition(float sample, uint sampleNumber)
{
  float xPercent = sampleNumber / (float)(Profile::Record::cSampleCount - 1);

  float seconds = Profile::ProfileSystem::Instance->GetTimeInSeconds((Profile::ProfileTime)sample);
  float yPercent = seconds / 0.025f;

  float x = mGraphPos.x + mGraphSize.x * xPercent;
  float y = mGraphPos.y + mGraphSize.y * (1.0f - yPercent);
  return Vec3(x, y, 0);
}

void PerformanceGraphWidget::DrawSamples(ViewBlock& viewBlock, FrameBlock& frameBlock, Array<StreamedVertex>& lines, Array<StreamedVertex>& triangles, float* samples, ByteColor byteColor, bool linesOnly)
{
  const uint sampleCount = Profile::Record::cSampleCount;
  uint sampleIndex = Profile::Record::sSampleIndex;

  Vec4 color = ToFloatColor(byteColor);
  Vec4 colorAlpha = color;
  colorAlpha.w = 0.25f;

  for (uint i = 0; i < sampleCount - 1; ++i)
  {
    uint i0 = (sampleIndex + i) % sampleCount;
    uint i1 = (sampleIndex + i + 1) % sampleCount;

    StreamedVertex v0(GetPosition(samples[i0], i),     Vec2(0, 0), colorAlpha);
    StreamedVertex v1(GetPosition(0, i),               Vec2(0, 1), colorAlpha);
    StreamedVertex v2(GetPosition(0, i + 1),           Vec2(1, 1), colorAlpha);
    StreamedVertex v3(GetPosition(samples[i1], i + 1), Vec2(1, 0), colorAlpha);

    if (linesOnly == false)
    {
      triangles.PushBack(v0);
      triangles.PushBack(v1);
      triangles.PushBack(v2);
      triangles.PushBack(v2);
      triangles.PushBack(v3);
      triangles.PushBack(v0);
    }

    v0.mColor = color;
    v3.mColor = color;
    lines.PushBack(v0);
    lines.PushBack(v3);
  }
}

float PerformanceGraphWidget::DrawProfileGraph(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Vec3 position, Profile::Record* record, float parentSize, float parentTotal, int level)
{
  float average = (float)record->SmoothAverage();
  float timeInS = Profile::ProfileSystem::Instance->GetTimeInSeconds((Profile::ProfileTime)record->SmoothAverage());

  Vec3 pos = position;
  Vec2 size((average / parentTotal) * parentSize, Pixels(20));
  Vec4 color = ToFloatColor(record->GetColor());

  StreamedVertex v0(SnapToPixels(pos + Vec3(0,      0,      0)), Vec2(0, 0), color);
  StreamedVertex v1(SnapToPixels(pos + Vec3(0,      size.y, 0)), Vec2(0, 1), color);
  StreamedVertex v2(SnapToPixels(pos + Vec3(size.x, size.y, 0)), Vec2(1, 1), color);
  StreamedVertex v3(SnapToPixels(pos + Vec3(size.x, 0,      0)), Vec2(1, 0), color);

  Array<StreamedVertex> vertices;
  vertices.PushBack(v0);
  vertices.PushBack(v1);
  vertices.PushBack(v2);
  vertices.PushBack(v2);
  vertices.PushBack(v3);
  vertices.PushBack(v0);

  CreateRenderData(viewBlock, frameBlock, clipRect, vertices, PrimitiveType::Triangles);

  RenderFont* font = FontManager::GetInstance()->GetRenderFont("NotoSans-Regular", 11, 0);

  String text;
  if (level == 0)
  {
    float fps = 0.0f;
    if (timeInS != 0.0f)
      fps = 1.0f / timeInS;
    fps = Math::Round(fps);
    text = String::Format("%s %0.0f %0.1f ms", record->GetName(), fps, timeInS* 1000.0f);
  }
  else
  {
    text = String::Format("%s %0.1f ms", record->GetName(), timeInS * 1000.0f);
  }

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, font->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(Color::Black));
  AddTextRange(fontProcessor, font, text, ToVector2(pos), TextAlign::Left, Vec2(1, 1), mSize, true);

  position.y += size.y;
  double totalTime = 0.0f;

  forRange (Profile::Record& childRecord, record->GetChildren())
  {
    position.x += DrawProfileGraph(viewBlock, frameBlock, clipRect, position, &childRecord, size.x, average, level + 1);
  }
  return size.x;
}

double PerformanceGraphWidget::RecursiveAverage(Profile::Record* record)
{
  Profile::Record::RangeType records = record->GetChildren();
  double totalTime = 0.0f;
  if(records.Empty())
  {
    return record->SmoothAverage();
  }
  else
  {
    while(!records.Empty())
    { 
      Profile::Record* childRecord = &records.Front();
      totalTime += RecursiveAverage(childRecord);
      records.PopFront();
    }
    return totalTime;
  }

}

//-------------------------------------------------------------------MemoryGraphWidget
MemoryGraphWidget::MemoryGraphWidget(Composite* parent)
  : Widget(parent)
{
}

void MemoryGraphWidget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);
  DrawMemoryGraph(Vec3(1, 0, 0), Memory::GetRoot(), mSize.x, 0, viewBlock, frameBlock, parentTx, clipRect);
}

void DrawRect(const Rect& rect, Array<StreamedVertex>& triangles, Vec4Param solidColor, Array<StreamedVertex>& lines, Vec4Param lineColor)
{
  Vec3 bl = Vec3(rect.BottomLeft());
  Vec3 br = Vec3(rect.BottomRight());
  Vec3 tl = Vec3(rect.TopLeft());
  Vec3 tr = Vec3(rect.TopRight());
  triangles.PushBack(StreamedVertex(bl, Vec2::cZero, solidColor));
  triangles.PushBack(StreamedVertex(br, Vec2::cZero, solidColor));
  triangles.PushBack(StreamedVertex(tr, Vec2::cZero, solidColor));
  triangles.PushBack(StreamedVertex(bl, Vec2::cZero, solidColor));
  triangles.PushBack(StreamedVertex(tr, Vec2::cZero, solidColor));
  triangles.PushBack(StreamedVertex(tl, Vec2::cZero, solidColor));

  // Bottom
  lines.PushBack(StreamedVertex(bl, Vec2::cZero, lineColor));
  lines.PushBack(StreamedVertex(br, Vec2::cZero, lineColor));
  // Right
  lines.PushBack(StreamedVertex(br, Vec2::cZero, lineColor));
  lines.PushBack(StreamedVertex(tr, Vec2::cZero, lineColor));
  // Top
  lines.PushBack(StreamedVertex(tr, Vec2::cZero, lineColor));
  lines.PushBack(StreamedVertex(tl, Vec2::cZero, lineColor));
  // Left
  lines.PushBack(StreamedVertex(tl, Vec2::cZero, lineColor));
  lines.PushBack(StreamedVertex(bl, Vec2::cZero, lineColor));
}

float MemoryGraphWidget::DrawMemoryGraph(Vec3 position, Memory::Graph* memoryNode, float parentSize, float parentTotal,
  ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, Rect clipRect)
{
  float ysize = Pixels(20);
  Memory::Stats stat;
  memoryNode->Compute(stat);

  float localKB = stat.BytesAllocated / 1024.0f;

  float xsize = parentTotal != 0 ? (localKB / parentTotal) * parentSize 
    : parentSize;

  String text = String::Format("%s %.2f KB", memoryNode->GetName(), localKB);

  Vec4 boxColor = ToFloatColor(Color::Red);
  Vec4 lineColor = ToFloatColor(Color::Black);
  position = SnapToPixels(position);
  Rect rect = Rect::PointAndSize(ToVector2(position), Vec2(xsize, ysize));
  
  // Create and render bordered boxes
  Array<StreamedVertex> triangles;
  Array<StreamedVertex> lines;
  DrawRect(rect, triangles, boxColor, lines, lineColor);
  CreateRenderData(viewBlock, frameBlock, clipRect, triangles, PrimitiveType::Triangles);
  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);

  // Compute the clip rect for the text
  Vec3 clipPosition = Math::TransformPointCol(parentTx, position);
  Rect subClipRect = Rect::PointAndSize(ToVector2(clipPosition), Vec2(xsize, ysize));
  // Render the text
  RenderFont* font = FontManager::GetInstance()->GetRenderFont("NotoSans-Regular", 11, 0);
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, subClipRect, font->mTexture);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(Color::Black));
  AddTextRange(fontProcessor, font, text, ToVector2(position), TextAlign::Left, Vec2(1, 1), mSize, true);


  position.y += ysize;

  Vec3 subP = position;
  float move = 0.0f;

  Memory::Graph::RangeType records = memoryNode->GetChildren();
  while(!records.Empty())
  {
    Memory::Graph& r = records.Front();
    float move = DrawMemoryGraph(subP, &r, xsize, localKB, viewBlock, frameBlock, parentTx, clipRect);
    subP.x += move;
    records.PopFront();
  }
  return xsize;
}

}//namespace Zero
