///////////////////////////////////////////////////////////////////////////////
///
/// \file GraphWidget.hpp
/// Declaration of the GraphWidget.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TextBox;
class RenderFont;

DeclareBitField2(GraphAxes,AxisX,AxisY);

//----------------------------------------------------------------- Graph Widget
class GraphWidget : public Widget
{
public:
  typedef GraphWidget ZilchSelf;

  GraphWidget(Composite* parent);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  /// Point transformations
  Vec2 ToGraphPositionScaled(Vec2Param pixelPos);
  Vec2 ToPixelPositionScaled(Vec2Param graphPos);

  Vec2 ToGraphPosition(Vec2Param pixelPos);
  Vec2 ToPixelPosition(Vec2Param graphPos);

  /// Returns whether or not the given point is inside the graph.
  bool WorldPointInGraph(Vec2Param worldPos);

  Vec2 ClampPos(Vec2Param worldPos);

  /// Sets the width range of the graph.
  void SetWidthMin(float min);
  void SetWidthMax(float max);
  void SetWidthRange(float min, float max);

  /// Sets the height range of the graph.
  void SetHeightMin(float min);
  void SetHeightMax(float max);
  void SetHeightRange(float min, float max);

  /// Determines if the x axis (horizontal) hash lines and labels should be drawn.
  bool GetDrawAxisX();
  void SetDrawAxisX(bool state);
  /// Determines if the y axis (vertical) hash lines and labels should be drawn.
  bool GetDrawAxisY();
  void SetDrawAxisY(bool state);

  bool mDrawLabels;
  bool mHideLastLabel;

private:
  Vec2 GetGridOrigin();

  /// Draws the grid.
  void DrawGrid(RenderFont* font, DisplayRender* render);

public:
  float GetWidthRange();
  float GetHeightRange();
  float GetWidthSpacing();
  float GetHeightSpacing();
  String GetLabelFormat(float spacing);

  /// 
  float GetIntervalSize(float range, float space);

private:

  enum {MIN, MAX};

  /// Width / Height
  float mWidthRange[2];
  float mHeightRange[2];

  BitField<GraphAxes::Enum> mFlags;
};

}//namespace Zero
