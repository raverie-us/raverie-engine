///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationEditor.hpp
/// Declaration of AnimationEditor Composite
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Typedefs
class Animation;
class AnimationEditor;
class TreeView;
class AnimationTrackSource;
struct TrackSelection;
class RichAnimation;
class TrackNode;
class TreeEvent;
class DataEvent;
class AnimationEditorData;
class TrackEvent;

void RegisterAnimationTrackViewEditors();

//--------------------------------------------------------- Animation Track View
class AnimationTrackView : public Composite
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  AnimationTrackView(Composite* parent, AnimationEditor* editor);

  /// Composite interface.
  void UpdateTransform() override;

  /// Builds the tree view from the given animation info.
  void SetAnimationEditorData(AnimationEditorData* editorData);

  /// Sets the selection.
  void SetSelection(Array<TrackNode*>& selection);

  /// Open all
  void FocusOnObject(TrackNode* track);

  /// Hides all data relevant to the animation being edited, but retains
  /// the data in attempt to avoid re-allocations.
  void Hide();

  /// Re-Displays all hidden information.
  void Show();

private:
  void UpdateToolTip();

  /// We want to know when the selection is modified outside of the tree view.
  void OnSelectionModified(Event* e);

  /// Focus on the track when double clicked.
  void OnDataActivated(DataEvent* e);

  /// We want to bring up a context menu when they right click on a row.
  void OnTreeRightClick(TreeEvent* e);

  /// Renaming object tracks.
  void OnRename(Event* e);

  /// Enable / Disable through the right click menu.
  void OnToggleEnable(Event* e);

  /// Clears all key frames on the track.
  void OnClearKeys(Event* e);

  /// When the delete option is pressed in the context menu.
  void OnDeleteTrack(Event* e);

  /// Called when the selection on the tree has changed.
  void OnTracksSelected(Event* e);

  /// We want to focus on the track that was added.
  void OnTrackAdded(TrackEvent* e);

  TreeView* mTree;
  AnimationTrackSource* mSource;

  Element* mBackground;

  /// Context row.
  DataIndex mCommandIndex;

  /// Text displayed when the animation is empty.
  HandleOf<ToolTip> mToolTip;

  AnimationEditor* mEditor;
  AnimationEditorData* mEditorData;
};

}//namespace Zero
