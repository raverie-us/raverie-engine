///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationControls.hpp
/// Declaration of AnimationControls Composite
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
class AnimationScrubber;
class RichAnimation;
class UpdateEvent;
class TextBox;
class Text;
class AnimationEditorData;
class ToggleIconButton;
class SelectorButton;
class AnimationSettings;
class StringSource;

namespace CurveEditing
{
  class CurveEditorToolbar;
}

//------------------------------------------------------------- Animation Button
class AnimationButton : public Composite
{
public:
  typedef AnimationButton ZilchSelf;

  AnimationButton(Composite* parent, StringParam element);
  void SetElement(StringParam element);
  void OnMouseEnter(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  Element* mImage;
};

//----------------------------------------------------------- Animation Selector
class AnimationSelector : public Composite
{
public:
  typedef AnimationSelector ZilchSelf;

  AnimationSelector(Composite* parent, AnimationEditor* editor);

  /// We want to change the selected animation from clicking on this text box.
  void OnAnimationBoxLeftClick(MouseEvent* e);
  void OnAnimationSelected(SearchViewEvent* e);

  /// Open the add dialog when the add button is pressed.
  void OnAddButtonPressed(ObjectEvent* e);

  void Hide();
  void Show();

  /// Displays the currently selected animation.
  TextBox* mAnimationBox;
  HandleOf<FloatingSearchView> mActiveSearch;
  AnimationEditor* mEditor;
};

//----------------------------------------------------------- Animation Tool Box
class AnimationToolBox : public Composite
{
public:
  typedef AnimationToolBox ZilchSelf;

  AnimationToolBox(Composite* parent, AnimationEditor* editor);
  ~AnimationToolBox();

  /// Sets the status text in the lower left corner.
  void SetStatusText(StringParam text, ByteColor color = Color::Gray);

  void Hide();
  void Show();

private:
  /// Event response.
  void OnAutoFocusPressed(Event* e);
  void OnAutoKeyPressed(Event* e);
  void OnSnappingXPressed(Event* e);
  void OnSnappingYPressed(Event* e);
  void OnOnionSkinningPressed(Event* e);
  void OnEditFpsSelected(Event* e);
  void OnTimeDisplaySelected(Event* e);

public:
  CurveEditing::CurveEditorToolbar* mCurveToolBar;
  ToggleIconButton* mAutoFocus;
  ToggleIconButton* mAutoKey;
  ToggleIconButton* mSnappingX;
  ToggleIconButton* mSnappingY;
  ToggleIconButton* mOnionSkinning;
  IconButton* mLinearTangents;
  IconButton* mSplitTangents;
  IconButton* mWeightedTangents;
  ComboBox* mEditFpsBox;
  SelectorButton* mTimeDisplaySelector;

  AnimationSettings* mSettings;
  AnimationEditor* mEditor;
  StringSource* mSource;

  /// A status at the bottom
  Label* mStatusText;
};

//---------------------------------------------------------------- Play Controls
class AnimationControls;
class PlayControls : public Composite
{
public:
  typedef PlayControls ZilchSelf;
  PlayControls(Composite* parent, AnimationControls* controls);

  void Pause();

  /// Event Response.
  void OnPlayLeftPressed(Event* e);
  void OnPlayRightPressed(Event* e);
  void OnPausePressed(Event* e);

  IconButton* mPause;
  IconButton* mPlayLeft;
  IconButton* mPlayRight;

  AnimationControls* mControls;
};

//----------------------------------------------------------- Animation Controls
DeclareEnum3(PreviewDirection, Paused, Forward, Backward);
class AnimationControls : public Composite
{
public:
  typedef AnimationControls ZilchSelf;

  AnimationControls(Composite* parent, AnimationEditor* editor);

  void SetScrubber(AnimationScrubber* scrubber);

  void SetAnimationEditorData(AnimationEditorData* editorData);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Hides all data relevant to the animation being edited, but retains
  /// the data in attempt to avoid re-allocations.
  void Hide();

  /// Re-Displays all hidden information.
  void Show();

private:
  friend class AnimationEditor;
  friend class PlayControls;

  /// Event Response.
  void OnUpdate(UpdateEvent* event);

  /// Button event response.
  void OnBeginPressed(MouseEvent* event);
  void OnShiftLeftPressed(MouseEvent* event);
  void OnShiftRightPressed(MouseEvent* event);
  void OnEndPressed(MouseEvent* event);

  void OnPlaybackModeSelected(Event* e);
  void OnPlaybackSliderChanged(Event* e);

  /// Updates the scrubber to the given time.  If wrapping is enabled,
  /// it will wrap around if the given time is out of the bounds of the
  /// currently selected animation.
  /// Returns whether or not it wrapped, or if it would have wrapped
  /// if wrap had been set.
  bool UpdateToTime(float time, bool wrap, bool clampToDuration = false);
  
  void OnMouseEnterPlaybackSpeed(MouseEvent* e);
  void OnMouseExitPlaybackSpeed(MouseEvent* e);

  HandleOf<ToolTip> mPlaybackSpeedTooltip;

  /// Top row of buttons.
  IconButton* mButtonBegin;
  IconButton* mButtonShiftLeft;
  PlayControls* mPlayControls;
  IconButton* mButtonShiftRight;
  IconButton* mButtonEnd;
  IconButton* mButtonSettings;
  
  SelectorButton* mPlayModeSelector;

  /// Controls the playback speed.
  Slider* mPlaybackSpeedSlider;

  /// Second row for animations
  TextBox* mAnimationBox;

  /// Adds a new animation.
  AnimationButton* mAddButton;

  /// Displays the object with the AnimationGraph component that we're editing.
  Label* mRootLabel;
  Label* mObjectLabel;

  /// Background.
  Element* mBackground;

  /// We need the scrubber to update the location of the play head.
  AnimationScrubber* mScrubber;

  /// The current play mode.
  PreviewDirection::Type mPreviewDirection;

  /// The current time in the playing animation.
  float mCurrTime;

  AnimationEditor* mEditor;
  AnimationEditorData* mEditorData;
};

}//namespace Zero
