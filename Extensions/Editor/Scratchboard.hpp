///////////////////////////////////////////////////////////////////////////////
///
/// \file Scratchboard.hpp
/// Declaration of the Scratchboard Composite.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class ScratchboardDrawer;
class MouseManipulation;

//----------------------------------------------------------------- Scratchboard
class Scratchboard : public Composite
{
public:
  typedef Scratchboard ZilchSelf;

  /// Constructor.
  Scratchboard(Composite* parent);

  /// Scrolls the graph in the given direction.
  void Scroll(Vec3Param direction);

  /// Animates the graph to center on the given point in world space.
  void CenterToPoint(Vec3Param graphPosition, float panTime);

  /// Makes the given aabb visible in the window by translating and zooming.
  void Frame(Aabb& worldAabb, float panTime);

  /// Creates a mouse manipulator to drag the given object 
  MouseManipulation* StartObjectDrag(Mouse* mouse, Widget* object, 
                        bool snapping = false, float snapFidelity = Pixels(10));

  /// Returns whether or not the given point is visible in the graph.
  bool WithinView(Vec3Param graphPosition);

  /// Transforms the given position from world space to the local space
  /// of this composite (screen position)
  Vec3 ToPixelPosition(Vec3Param graphPosition);

  /// Transforms the given position from local space (screen position) to
  /// the world space of the scratchboard
  Vec3 ToGraphPosition(Vec3Param pixelPosition);

  /// Sets the amount of pixels between each hash mark on the grid.
  void SetGridSize(float size);

  /// Gets the amount of pixels between each hash mark on the grid.
  float GetGridSize() const;

  /// Enables / Disables dragging.
  void SetDragging(bool state);

  /// Snaps a point to the grid.
  Vec3 SnapToGrid(Vec3Param graphPos);

  /// Returns the client area that all objects are attached to.
  Composite* GetClientArea();

  /// Composite Interface.
  void AttachChildWidget(Widget* widget, AttachType::Enum attachType) override;
  void UpdateTransform() override;

private:
  /// Event Response.
  void OnMiddleMouseDown(MouseEvent* e);

  /// Clamps the view to make sure at least one object is visible at all times.
  void ClampViewToElements();

  /// Returns an Aabb of all child objects in the scratchboard.
  Aabb GetObjectsAabb();

private:
  /// Whether or not dragging is enabled.
  bool mDraggingEnabled;

  /// Makes it so that you cannot move around so that the view leaves the 
  /// aabb of all objects.
  bool mKeepElementsInView;

  /// Draws the small light hashes of the grid.
  ScratchboardDrawer* mLightGridDrawer;

  /// Draws the larger bold hashes of the grid.
  ScratchboardDrawer* mBoldGridDrawer;

  /// All objects are attached to this composite.
  Composite* mClientArea;
};

}//namespace Zero
