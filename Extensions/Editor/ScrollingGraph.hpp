///////////////////////////////////////////////////////////////////////////////
///
/// \file ScrollingGraph.hpp
/// Declaration of ScrollingGraph helper class.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Actions;

//-------------------------------------------------------------- Scrolling Graph
/// Used to help making graphs easier to create
class ScrollingGraph
{
public:
  ScrollingGraph(Vec2Param hashSize);

  /// Converts from a pixel position (in widget coordinates) to a
  /// local position on the graph.
  Vec2 ToGraphSpace(Vec2Param pixelPos);
  Vec2 ToPixels(Vec2Param graphPos);

  float GetDisplayInterval(float clientArea, uint axis);
  Vec2 GetDisplayInterval(Vec2 clientArea);

  /// Snap to nearest displayed hash.
  Vec2 SnapGraphPos(Vec2Param graphPos, Vec2Param clientArea);
  Vec2 SnapPixelPos(Vec2Param pixelPos, Vec2Param clientArea);

  /// Scrolls in the given direction.
  void ScrollGraph(Vec2Param graphSpaceScrollDirection);
  void ScrollPixels(Vec2Param pixelScrollDirection);

  void PanToTranslation(Vec2Param graphPos, float animTime = 0.4f);

  /// Frames the given aabb in view.
  void Frame(Vec2Param min, Vec2Param max,
             Vec2Param clientSize, IntVec2 axes = IntVec2(1,1),
             Vec2 pixelPadding = Pixels(0, 0), float animTime = 0.4f);

  //------------------------------------------------------------------ Hash Mark
  struct HashMark
  {
    /// Position in graph space.
    float Position;
    /// Formatted label of the position.
    String Label;
  };

  //---------------------------------------------------------------------- range
  struct range
  {
    range(float origin, float range, float spacing,
          bool halfHash, float gridScale);
    HashMark Front();
    void PopFront();
    bool Empty();

  private:
    friend class ScrollingGraph;
    float mOffset;
    uint mHashCount;
    uint mCurrentHash;
    bool mHalfHash;
    float mOrigin;
    float mGridScale;

    /// The spacing in graph space.
    float mSpacing;
  };

  range GetWidthHashes(float clientWidth, bool halfHashes = false);
  range GetHeightHashes(float clientHeight, bool halfHashes = false);

  String GetWidthFormat(float clientWidth);
  String GetHeightFormat(float clientHeight);

  Vec2 GetTranslation();
  void SetTranslation(Vec2Param translation);

  Vec2 GetZoom();
  void SetZoom(Vec2Param zoom);
  void ZoomAtPosition(Vec2Param position, Vec2Param zoom);

  /// A scalar to the values in the grid. For example, if the values
  /// you're giving to the grid is between 0-1, but you want to display them
  /// between 0-60, you would set this value to 60 (on whichever axis).
  /// This allows you to not have to worry about conversions every time
  /// you interface with this object.
  Vec2 mGridScale;

  /// In graph space.
  Vec2 Translation;

  /// Used to clamp how much you can zoom in / out.
  Vec2 ZoomMin, ZoomMax;

  /// If supplied, it allows us to animate when focusing on an area.
  Actions* mActions;

private:
  friend struct range;
  static String GetLabelFormat(float spacing);
  float GetNextStep(int index);
  float GetIntervalSize(float range, float space);

  Vec2 Zoom;

  /// The size (in pixels) that represents 1 unit when the zoom is at 1.0.
  Vec2 mPixelsPerUnit;
};

}//namespace Zero
