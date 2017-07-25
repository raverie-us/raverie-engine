///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationEditorData.cpp
/// Declaration of AnimationEditorData.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace AnimationConstants
{
  const ByteColor cHashColor = Color::Black;
}

/// Forward declarations.
class AnimationEditor;
class Animation;
class RichAnimation;
class AnimationSettings;
class ScrollingGraph;
class TrackNode;
class TrackEvent;

// Events
namespace Events
{
  // Sent when the selected tracks are modified
  DeclareEvent(TrackSelectionModified);
}

//---------------------------------------------------------- Key Frame Selection
struct KeyFrameSelection
{
  Array<uint> Keys;
};

//-------------------------------------------------------------- Track Selection
struct TrackSelection
{
  /// Contains only property tracks
  Array<TrackNode*> Tracks;
};

//-------------------------------------------------------- Animation Editor Data
class AnimationEditorData : public EventObject
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  AnimationEditorData(AnimationEditor* editor, Cog* animGraphObject,
                      Animation* animation, ScrollingGraph* graphData);

  /// When the rich animation is modified, we want to bake it out to
  /// the animation resource.
  void OnAnimationModified(Event* e);

  /// When a track is deleted, we need to remove it from the selection.
  void OnTrackDeleted(TrackEvent* e);

  /// Bakes the RichAnimation to the Animation.
  void BakeToAnimation();

  /// Track Selection.
  void SetSelection(Array<TrackNode*>& selection);

  /// Adds the given track to the selection.
  void AddToSelection(TrackNode* track);

  /// Saves the rich animation to file.
  void SaveRichAnimation();

  /// Used to sync scrolling between the scrubber and the curve editor.
  ScrollingGraph* mGraphData;

  /// The animation being modified.
  HandleOf<Animation> mAnimation;

  /// The rich animation we're editing.
  RichAnimation* mRichAnimation;

  /// We need a pointer back to the editor so that when the rich animation
  /// has been modified, we can tell it to update the animGraph object.
  AnimationEditor* mEditor;

  /// The tracks that are currently being displayed.
  typedef HashSet<TrackNode*> TrackMap;
  TrackMap mVisiblePropertyTracks;
};

/// Coverts the given animation into a new rich animation.
RichAnimation* ConvertToRichAnimation(Animation* animation);

}//namespace Zero
