///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationScrubber.hpp
/// Declaration of the AnimationScrubber Composite
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
class ScrubberDrawer;
class AnimationScrubber;
struct KeyFrameSelection;
class AnimationEditorData;
class ScrollingGraph;
class KeyFrameEvent;

// Constants
const String cKeyFrameImage = "KeyFrame";
const String cKeyFrameImageSelected = "KeyFrameSelected";
const String cTextFont = "NotoSans-Regular";
const String cBackgroundElement = "ScrubBackground";

// Events
namespace Events
{
  DeclareEvent(PlayHeadModified);
}

//-------------------------------------------------------------------- Key Frame
class KeyFrameIcon : public Composite
{
public:
  /// Typedefs.
  typedef KeyFrameIcon ZilchSelf;

  /// Constructor.
  KeyFrameIcon(AnimationScrubber* scrubber, float time);

  /// Widget Interface.
  void UpdateTransform() override;
  void OnDestroy() override;

  /// Event response.
  void OnMouseEnter(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);
  void OnLeftMouseDown(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnLeftMouseDrag(MouseEvent* e);

  /// We want to bring up a menu when the user right clicks on the key frame.
  void OnRightClick(MouseEvent* e);
  void OnRightMouseUp(MouseEvent* e);

  /// When the delete option is pressed in the context menu.
  void OnDelete(ObjectEvent* e);

  /// Deletes all key frames this icon represents.
  void Delete();

  /// Moves all key frames to the given location. Does not mark the animation
  /// as modified.
  void MoveTo(float time);

  /// Should be called after being moved.  This will push 
  /// all data to the animation.
  void FinishMove();

  /// Hides / shows the icon.
  void Hide();
  void Show();

  /// Attempts to remove the given key frame. 
  bool AttemptRemove(KeyFrame* keyFrame);

  KeyFrameIcon* Duplicate();

  float mTime;

  /// The key frames that this icon is displaying.
  Array<KeyFrame*> mKeyFrames;

  /// The icon.
  Element* mIcon;

  /// Pointer back to our parent.
  AnimationScrubber* mScrubber;
};

//----------------------------------------------------------- Animation Scrubber
class AnimationScrubber : public Composite
{
public:
  /// Typedefs.
  typedef AnimationScrubber ZilchSelf;

  /// Constructor.
  AnimationScrubber(Composite* parent, AnimationEditor* editor,
                    ScrollingGraph* graphData);

  /// Information about the animation we're modifying.
  void SetAnimationEditorData(AnimationEditorData* editorData);

  /// The position of the scrub (Play Head)
  float GetPlayHead();
  void SetPlayHead(float t, bool sendsEvent = true);

  /// Clears all key frames.
  void DestroyKeyFrames();

  /// Zooms and offsets the scrubber to get all loaded key frames in view.
  void FrameSelectedKeyFrames(KeyFrameSelection& selection);

  /// Hides all data relevant to the animation being edited, but retains
  /// the data in attempt to avoid re-allocations.
  void Hide();

  /// Re-Displays all hidden information.
  void Show();

  /// Conversions from time to local space of the composite.
  float ToTime(float pixelPos);
  float ToPixels(float worldPos);

  bool mShowGhostPlayHead;
  float mGhostPlayHead;

private:
  friend class ScrubberManipulator;
  friend class ScrubberSelection;
  friend class KeyFrameManipulator;
  friend class ScrubberDrawer;
  friend class GraphDrawer;
  friend class DopeGraphDrawer;
  friend class KeyFrameIcon;
  friend class AnimationEditor;

  void DeleteSelectedKeys();

  /// Attempts to find a key icon to add this key frame to, otherwise it will
  /// create a new key icon.
  void AddKeyFrame(KeyFrame* keyFrame);

  /// Destroys and re-creates all key frames.
  void RebuildKeyFrames();

  /// Event Response.
  void OnSelectionModified(Event* event);
  void OnTrackDeleted(TrackEvent* event);
  void OnKeyFrameAdded(KeyFrameEvent* event);
  void OnKeyFrameModified(KeyFrameEvent* event);
  void OnKeyFrameDeleted(KeyFrameEvent* event);
  void OnKeyDown(KeyboardEvent* e);

  /// Mouse event response.
  void OnLeftMouseClick(MouseEvent* event);
  void OnLeftMouseDrag(MouseEvent* event);
  void OnRightClick(MouseEvent* event);
  void OnMiddleMouseDown(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  void MouseDragUpdate(MouseEvent* e);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Updates the key icon in the array map with its updated time.
  void ReInsert(KeyFrameIcon* icon);

  /// Whether or not the composite is enabled.
  bool mEnabled;

  /// The position of the play head.
  float mPlayHead;

  /// Used to animate the sliding.
  float mTargetSlidePosition;

  /// We want to know about changes being made to the animation so that we can
  /// update the data we're displaying. However, when we're modifying the
  /// animation ourselves, the events we're receiving can be ignored
  /// because we're already handling the changes to the animation.
  bool mIgnoreAnimationEvents;

  /// Selected icons.
  HashSet<KeyFrameIcon*> mSelection;

  /// Key frames.
  ArrayMultiMap<float, KeyFrameIcon*> mKeyFrames;

  /// Used to display selections.
  Element* mSelectBox;

  /// Element displayed behind the scrubber (fills up the entire size).
  Element* mBackground;

  /// Displays red over the negative x axis.
  Element* mNegativeArea;

  /// Draws the hash marks behind the key frames.
  ScrubberDrawer* mDrawer;

  /// Editor data.
  AnimationEditor* mEditor;
  AnimationEditorData* mEditorData;
  ScrollingGraph* mGraphData;
};

namespace AnimScrubberUi
{
DeclareTweakable(Vec4, PlayHeadColor);
DeclareTweakable(Vec4, GhostPlayHeadColor);
}

}//namespace Zero
