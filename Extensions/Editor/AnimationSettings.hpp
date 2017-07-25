///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationSettings.cpp
/// Declaration of settings for the animator.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class PropertyView;
class AnimationEditor;
class AnimationEditorData;

//----------------------------------------------------------- Animation Settings
DeclareEnum2(TimeDisplay, Frames, Timecodes);

class AnimationSettings
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  AnimationSettings();

  /// ContentComponent Interface.
  void Serialize(Serializer& stream);

  /// Edit Fps.
  void SetEditFps(uint index);
  uint GetEditFps();

  /// 12/24/30/60 fps
  float mEditFps;

  /// Whether to display frames or time codes on the play head in the scrubber.
  TimeDisplay::Enum mTimeDisplay;

  /// Whether or not the play head is snapped to the current grid resolution.
  bool mSnappingX, mSnappingY;

  /// Whether or not to automatically make key frames
  /// properties are modified.
  bool mAutoKey;

  /// Auto focusing on the curves when the track selection has changed.
  bool mAutoFocus;

  /// Shows a "ghost" of the object being animated.
  bool mOnionSkinning;

  /// The start time of the animation
  float mStartTime;

  /// The end time of the animation
  float mEndTime;

  float mPlaybackSpeed;

  /// The viewing mode of this animation.
  AnimationPlayMode::Type mPreviewMode;

  typedef Pair<String, float> EditFpsPreset;
  typedef Array<EditFpsPreset> EditFpsPresetArray;
  static EditFpsPresetArray mEditFpsPresets;
};

//------------------------------------------------------ Animation Settings View
class AnimationSettingsView : public Composite
{
public:
  AnimationSettingsView(Composite* parent, AnimationEditor* editor);
  void UpdateTransform() override;

  void SetAnimationEditorData(AnimationEditorData* editorData);

private:
  Composite* mGroup;
  PropertyView* mRichAnimProperties;
  PropertyView* mSettingsProperties;
  Element* mBackground;
  AnimationEditor* mEditor;
};

}//namespace Zero
