// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Profile
{
class Record;
} // namespace Profile

class DisplayRender;
class DisplayCustomDraw;
class Composite;

/// Displays stacked line graphs of the top-level records from the
/// profiling system. Should eventually be made more generic.
class PerformanceGraphWidget : public Widget
{
public:
  PerformanceGraphWidget(Composite* parent);

  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;
  float DrawProfileGraph(ViewBlock& viewBlock,
                         FrameBlock& frameBlock,
                         WidgetRect clipRect,
                         Vec3 position,
                         Profile::Record* record,
                         float parentSize,
                         float parentTotal,
                         int level = 0);

  static double RecursiveAverage(Profile::Record* record);

  Vec3 GetPosition(float sample, uint sampleNumber);
  void DrawSamples(ViewBlock& viewBlock,
                   FrameBlock& frameBlock,
                   Array<StreamedVertex>& lines,
                   Array<StreamedVertex>& triangles,
                   float* samples,
                   ByteColor byteColor,
                   bool linesOnly = false);

  Vec3 mGraphPos;
  Vec3 mGraphSize;
};

/// Displays bar-graphs of current memory usage. Should eventually be
/// refactored.
class MemoryGraphWidget : public Widget
{
public:
  MemoryGraphWidget(Composite* parent);

  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;

  float DrawMemoryGraph(Vec3 position,
                        Memory::Graph* memoryNode,
                        float parentSize,
                        float parentTotal,
                        ViewBlock& viewBlock,
                        FrameBlock& frameBlock,
                        Mat4Param parentTx,
                        WidgetRect clipRect);
};

} // namespace Zero
