///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationGraph.hpp
/// Declaration of the AnimationGraph Composite
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class AnimationEditor;
class GraphDrawer;
class KeyFrame;
struct KeyFrameSelection;
class AnimationScrubber;
class CurveEditor;
class AnimationGraphEditor;
class TrackNode;
class TileViewWidgetFactory;
class LayersDataSource;
class ScrollingGraph;
class KeyFrameEvent;
class AnimationToolBox;

//------------------------------------------------------- Animation Curve Editor
class AnimationCurveEditor : public CurveEditor
{
public:
  /// Typedefs.
  typedef AnimationCurveEditor ZilchSelf;

  /// Constructor.
  AnimationCurveEditor(AnimationGraphEditor* parent);

  bool GetSelectionAabb(Vec2* min, Vec2* max);

  /// CurveEditor Interface.
  Vec2 ToPixelPosition(Vec2 graphPos) override;
  Vec2 ToGraphPosition(Vec2 pixelPos) override;
  Vec2 ClampPixelPosition(Vec2 pixelPos) override;
  void MouseDragUpdate(MouseEvent* e) override;
  String GraphToDisplayTextX(float graphValue) override;
  float DisplayTextToGraphX(StringParam displayText) override;

private:
  void GetSelectedControlPoints(HashSet<ControlPoint*>& set);

  /// Access back to our parent.
  AnimationGraphEditor* mGraph;
};

//------------------------------------------------------- Animation Curve Object
class AnimationCurveObject : public CurveObject
{
public:
  typedef AnimationCurveObject ZilchSelf;

  AnimationCurveObject(AnimationGraphEditor* editor, TrackNode* track);

  /// CurveObject Interface.
  void GetCurve(Vec3Array& curve) override;

  /// The property track we're displaying.
  TrackNode* GetTrack();

private:
  /// Builds the curve from the given track.
  void BuildCurve(TrackNode* track);

  /// Creates a control point from the given key frame.
  void CreateControlPoint(KeyFrame* keyFrame);

  void OnControlPointAdded(CurveEvent* e);

  /// We want to know when the curve was modified to update the animation.
  void OnControlPointModified(CurveEvent* e);

  /// When a control point is deleted in the curve editor, we want to
  /// remove the corresponding key frame from the animation.
  void OnControlPointDeleted(CurveEvent* e);

  /// When a key frame is added, modified, or deleted, we need to update
  /// the corresponding control point.
  void OnKeyFrameAdded(KeyFrameEvent* event);
  void OnKeyFrameModified(KeyFrameEvent* event);
  void OnKeyFrameDeleted(KeyFrameEvent* event);

  /// We want to delete ourself when the track we're displaying is deleted.
  void OnTrackDeleted(TrackEvent* event);

  /// We want to know about changes being made to the animation so that we can
  /// update the data we're displaying. However, when we're modifying the
  /// animation ourselves, the events we're receiving can be ignored
  /// because we're already handling the changes to the animation.
  bool mIgnoreAnimationEvents;

  /// The property track we're displaying.
  TrackNode* mTrack;

  /// Access back to our parent.
  AnimationGraphEditor* mGraph;

  /// When a key frame is modified on the animation, we need to be able to
  /// look up the control point in the curve that it corresponds to.
  HashMap<KeyFrame*, ControlPoint*> mKeysToControlPoints;
};

//-------------------------------------------------------------- Animation Graph
class AnimationGraphEditor : public Composite
{
public:
  /// Type defines.
  typedef AnimationGraphEditor ZilchSelf;

  /// Constructor.
  AnimationGraphEditor(Composite* parent,AnimationEditor* editor,
                       ScrollingGraph* graphData);

  void SetScrubber(AnimationScrubber* scrubber);

  /// We need to pull the curve editor buttons from the toolbox.
  void SetToolBox(AnimationToolBox* toolBox);

  /// Information about the animation we're modifying.
  void SetAnimationEditorData(AnimationEditorData* editorData);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Hides all data relevant to the animation being edited, but retains
  /// the data in attempt to avoid re-allocations.
  void Hide();

  /// Re-Displays all hidden information.
  void Show();

  /// Conversion functions.
  Vec2 ToPixelPosition(Vec2Param graphPos);
  Vec2 ToGraphPosition(Vec2Param pixelPos);

  /// Focus' on the curves of all the selected graphs.
  void FocusOnSelectedCurves(IntVec2Param axes);

private:
  friend class AnimationEditor;
  friend class GraphDrawer;
  friend class MaterialTrackViewer;
  friend class GraphManipulator;
  friend class AnimationCurveEditor;
  friend class AnimationCurveObject;

  /// Finds a color for the given track.
  ByteColor GetTrackColor(TrackNode* track, HashSet<ByteColor>& takenColors,
                          uint& colorIndex);

  /// Rebuilds all curve editors based on the visible property tracks.
  void RebuildCurves();

  /// Zooms to the mouse on the given axis.
  void Zoom(MouseEvent* e, Vec2Param axis);

  /// Event response.
  void OnSelectionModified(Event* e);
  void OnAnimationModified(Event* e);
  void OnMiddleMouseDown(MouseEvent* e);
  void OnMouseScroll(MouseEvent* e);
  void OnMouseScrollNegativeArea(MouseEvent* e);
  void OnMouseMove(MouseEvent* e);
  void OnMouseEnterCurve(CurveEvent* e);
  void OnMouseExitCurve(CurveEvent* e);
  void OnKeyDown(KeyboardEvent* e);

  /// We want to take soft focus when the mouse enters for keyboard input.
  void OnMouseEnter(MouseEvent* e);

  /// Whether or not information of the animation is drawn.
  bool mEnabled;

  /// Draws the graph behind the curves.
  GraphDrawer* mGraphDrawer;

  /// This will manage the multiple curves we have (one for each track).
  AnimationCurveEditor* mCurveEditor;

  /// Displays red over the negative x axis.
  Element* mNegativeArea;

  /// Element displayed behind the graph (fills up the entire size).
  Element* mBackground;

  /// Editor data.
  AnimationEditor* mEditor;
  AnimationEditorData* mEditorData;
  ScrollingGraph* mGraphData;
  AnimationScrubber* mScrubber;
};

}//namespace Zero
