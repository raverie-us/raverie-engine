///////////////////////////////////////////////////////////////////////////////
///
/// \file Slider.hpp
///  Declaration of the basic Widget system controls.
///
/// Authors: Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------------
namespace Events
{
  /// Called before any manipulation or change happens so that it can be
  /// queried for the old value for undo/redo.
  DeclareEvent(SliderManipulationStarted);
  /// The value has been changed, but it's not the final commit.
  DeclareEvent(SliderIncrementalChange);
  /// The value change is final.
  DeclareEvent(SliderChanged);
}//namespace Events

//------------------------------------------------------------------ ProgressBar
class ProgressBar : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  ProgressBar(Composite* parent);

  /// Composite Interface.
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  /// Percentage.
  void SetPercentage(float percentage);
  float GetPercentage();

  /// Colors.
  void SetPrimaryColor(Vec4Param color);
  void SetBackgroundColor(Vec4Param color);

  /// Used to hide the percentage text.
  void SetTextVisible(bool state);
  void SetProgressBarVisible(bool state);

  Thickness mPadding;

protected:
  /// Whether or not the progress bar is currently hidden.
  bool mProgressBarVisible;

  /// The current percentage of the scroll bar.
  float mPercentage;

  /// Displays the percentage in the middle of the progress bar.
  Text* mPercentageText;

  /// Bar used to display the percentage.
  Element* mProgressBar;

  /// Flat color in the background behind the progress bar.
  Element* mBackground;
};

//----------------------------------------------------------------------- Slider
DeclareEnum3(SliderType, Number, Percentage, Degree);

///Slider control for a simple slide bar.
class Slider : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  Slider(Composite* parent, SliderType::Type sliderType);

  /// The range of values displayed.
  void SetRange(float min, float max);

  /// How much to increment when being modified.
  void SetIncrement(float increment);

  /// Adds a '-' to the middle of the progress slider.
  void SetInvalid();

  /// Percentage value between 0 and 1.
  float GetPercentage();
  void SetPercentage(float percentage, bool sendMessage);

  /// The raw slider value.
  float GetValue();
  void SetValue(float newValue, bool sendEvents);

  /// Composite Interface.
  bool TakeFocusOverride() override;
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

private:
  friend class SliderManipulation;
  void CommitValue(float newValue);

  /// Starts editing text.
  void StartEditText();

  /// Updates the text based on the current value.
  void UpdateText();

  /// Event response.
  void OnMouseDown(MouseEvent* e);
  void OnRightClick(MouseEvent* e);
  void OnKeyDown(KeyboardEvent* e);
  void OnKeyRepeated(KeyboardEvent* e);
  void OnKeyUp(KeyboardEvent* e);
  void OnTextSubmit(Event* e);
  void OnTextFocusLost(Event* e);
  void OnFocusLost(Event* e);

  /// Whether or not the slider is marked as invalid. Once it is set to any
  /// value or is modified, it will become valid.
  bool mInvalid;

  /// The progress bar used to display the current value.
  ProgressBar* mProgressBar;

  /// A border to show whether or not we have focus.
  Element* mBorder;

  /// The progress bar only displays percentages, so if we're not displaying
  /// percentages, we need to disable theirs and use our own.
  Text* mText;

  /// We need to keep track of whether or not the value was nudged (with
  /// keyboard controls) so that when we lose focus, we know whether or
  /// not to commit the final value.
  bool mValueNudged;

  /// Used for when clicked on to type in a specific value.
  TextBox* mEditTextBox;

  /// It's valid for the value to be outside of the min and max range.
  float mValue; 

  /// The min and max values
  float mMinValue;
  float mMaxValue;

  /// How much to increment by
  float mIncrement;

  /// The display type.
  SliderType::Type mType;
};

}//namespace Zero
