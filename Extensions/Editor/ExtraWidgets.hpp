///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Profile
{
class Record;
}//namespace Profile

class DisplayRender;
class DisplayCustomDraw;
class Composite;

//------------------------------------------------------------------------------PerformanceGraphWidget
/// Displays stacked line graphs of the top-level records from the
/// profiling system. Should eventually be made more generic.
class PerformanceGraphWidget : public Widget
{
public:
  PerformanceGraphWidget(Composite* parent);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;
  float DrawProfileGraph(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Vec3 position, Profile::Record* record, float parentSize, float parentTotal, int level = 0);

  static double RecursiveAverage(Profile::Record* record);

  Vec3 GetPosition(float sample, uint sampleNumber);
  void DrawSamples(ViewBlock& viewBlock, FrameBlock& frameBlock, Array<StreamedVertex>& lines, Array<StreamedVertex>& triangles, float* samples, ByteColor byteColor, bool linesOnly = false);

  Vec3 mGraphPos;
  Vec3 mGraphSize;
};

//------------------------------------------------------------------------------MemoryGraphWidget
/// Displays bar-graphs of current memory usage. Should eventually be refactored.
class MemoryGraphWidget : public Widget
{
public:
  MemoryGraphWidget(Composite* parent);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  float DrawMemoryGraph(Vec3 position, Memory::Graph* memoryNode, float parentSize, float parentTotal,
    ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, Rect clipRect);
};

}//namespace Zero
