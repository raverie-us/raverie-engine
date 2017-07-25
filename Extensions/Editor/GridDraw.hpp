///////////////////////////////////////////////////////////////////////////////
///
/// \file GridDraw.hpp
/// Declaration of the GridDraw class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class UpdateEvent;

/// A component used for drawing a grid
class GridDraw : public Component
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Constants
  static const size_t NumAxes = 3;

  // Component Interface
  void Initialize(CogInitializer& initializer);
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Update event
  void OnFrameUpdate(UpdateEvent* e);

  // Set the axis.
  void SetAxis(AxisDirection::Enum axis);

  bool mActive;

private:

  static void DrawLine();
  void Draw();
public:

  /// Is the grid offset by half a unit?
  bool mHalfCellOffset;

  /// The deltas in the grid (how far apart we draw grid lines)
  float mCellSize;

  /// Changes the line color to 
  bool mDrawAxisOrigins;

  /// Always draw the grid in editor (not just when it's selected)
  bool mAlwaysDrawInEditor;

  /// Draw the grid in the game.
  bool mDrawInGame;

  /// Color of grid lines
  Vec4 mGridColor;
  
  // Color of highlighted grid lines
  Vec4 mGridHighlight;

  /// How often should cells be activated
  int mHighlightInterval;

  /// Move with the editor camera?
  bool mFollowEditorCamera;

  /// The number of lines to draw
  int mLines;

  AxisDirection::Enum mAxis;

};

}//namespace Zero
