///////////////////////////////////////////////////////////////////////////////
///
/// \file Slider.cpp
///  Declaration of the basic Widget system controls.
///
/// Authors: Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ProgressBarUi
{
const cstr cLocation = "EditorUi/Controls/ProgressBar";
Tweakable(Vec4,  BackgroundColor,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  ProgressBarColor, Vec4(1,1,1,1), cLocation);
Tweakable(float, Padding,          Pixels(2),     cLocation);
}

namespace SliderUi
{
const cstr cLocation = "EditorUi/Controls/Slider";
Tweakable(Vec4, FocusBorderColor, Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  DefineEvent(SliderManipulationStarted);
  DefineEvent(SliderIncrementalChange);
  DefineEvent(SliderChanged);
}//namespace Events

//------------------------------------------------------------------ ProgressBar
ZilchDefineType(ProgressBar, builder, type)
{
}

//******************************************************************************
ProgressBar::ProgressBar(Composite* parent)
  : Composite(parent)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mProgressBar = CreateAttached<Element>(cWhiteSquare);
  mPercentageText = new Text(this, cText);
  mPercentageText->SetInteractive(false);
  mProgressBarVisible = true;
  mPercentage = 0.0f;

  SetPrimaryColor(ProgressBarUi::ProgressBarColor);
  SetBackgroundColor(ProgressBarUi::BackgroundColor);

  mPadding = Thickness::All(ProgressBarUi::Padding);
}

//******************************************************************************
void ProgressBar::UpdateTransform()
{
  // Background takes up the full size
  mBackground->SetSize(mSize);

  // Shrink the progress bar by the padding
  LayoutResult lr = RemoveThickness(mPadding, mSize);
  // Shrink it by the percentage
  lr.Size.x *= mPercentage;

  // If the size is 0, it will display in a weird way, so set it invisible
  // This should be temporary until the display bug is fixed
  mProgressBar->SetVisible(mProgressBarVisible);
  if(lr.Size.x == 0.0f)
    mProgressBar->SetVisible(false);
  
  PlaceWithLayout(lr, mProgressBar);

  // Set the text if it's visible
  if(mPercentageText->GetActive())
  {
    String percentText = String::Format("%.0f%%", mPercentage * 100.0f);
    mPercentageText->SetText(percentText);
  }

  // Center the text
  Vec2 textSize = mPercentageText->GetMinSize();
  Vec3 textPos = ToVector3((mSize * 0.5f) - (textSize * 0.5f));
  mPercentageText->SetTranslation(SnapToPixels(textPos));
  mPercentageText->SetSize(textSize);

  // Base update transform
  Composite::UpdateTransform();
}

//******************************************************************************
Vec2 ProgressBar::GetMinSize()
{
  return mPercentageText->GetMinSize();
}

//******************************************************************************
void ProgressBar::SetPercentage(float percentage)
{
  mPercentage = percentage;
  MarkAsNeedsUpdate();
}

//******************************************************************************
float ProgressBar::GetPercentage()
{
  return mPercentage;
}

//******************************************************************************
void ProgressBar::SetPrimaryColor(Vec4Param color)
{
  mProgressBar->SetColor(color);
}

//******************************************************************************
void ProgressBar::SetBackgroundColor(Vec4Param color)
{
  mBackground->SetColor(color);
}

//******************************************************************************
void ProgressBar::SetTextVisible(bool state)
{
  mPercentageText->SetActive(state);
}

//******************************************************************************
void ProgressBar::SetProgressBarVisible(bool state)
{
  mProgressBarVisible = state;
}

//---------------------------------------------------------- Slider Manipulation
class SliderManipulation : public MouseManipulation
{
public:
  //****************************************************************************
  SliderManipulation(Mouse* mouse, Composite* owner, Slider* sliderTarget)
    : MouseManipulation(mouse, owner)
  {
    mSliderTarget = sliderTarget;
    UpdateToMouse(mouse);
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    UpdateToMouse(event->GetMouse());
  }

  //****************************************************************************
  void UpdateToMouse(Mouse* mouse)
  {
    Vec2 mousePos = mSliderTarget->ToLocal(mouse->GetClientPosition());
    float val = mousePos.x / mSliderTarget->mSize.x;
    mSliderTarget->SetPercentage(val, true);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* event) override
  {
    mSliderTarget->CommitValue(mSliderTarget->GetValue());
    mSliderTarget->TryTakeFocus();
    this->Destroy();
  }

  /// The slider we're modifying.
  Slider* mSliderTarget;
};

//----------------------------------------------------------------------- Slider
ZilchDefineType(Slider, builder, type)
{
}

//******************************************************************************
Slider::Slider(Composite* parent, SliderType::Type sliderType)
  : Composite(parent)
{
  mType = sliderType;

  // Create the progress bar
  mProgressBar = new ProgressBar(this);

  // Create a border for when we have focus
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBorder->SetInteractive(false);

  // Create the text object to display over the center of the bar
  mText = new Text(this, cText);
  mText->SetInteractive(false);

  mEditTextBox = nullptr;

  mInvalid = false;
  mValueNudged = false;

  // Default to 0
  mValue = 0.0f;
  mMinValue = 0.0f;
  mMaxValue = 1.0f;
  mIncrement = 0.1f;

  // Event connections
  ConnectThisTo(mProgressBar, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(mProgressBar, Events::RightClick,    OnRightClick);
  ConnectThisTo(this,         Events::KeyDown,       OnKeyDown);
  ConnectThisTo(this,         Events::KeyRepeated,   OnKeyRepeated);
  ConnectThisTo(this,         Events::KeyUp,         OnKeyUp);
  ConnectThisTo(this,         Events::FocusLost,     OnFocusLost);
}

//******************************************************************************
void Slider::SetRange(float min, float max)
{
  mMinValue = min;
  mMaxValue = max;
}

//******************************************************************************
void Slider::SetIncrement(float increment)
{
  mIncrement = increment;
}

//******************************************************************************
void Slider::SetInvalid()
{
  mProgressBar->SetProgressBarVisible(false);
  mProgressBar->SetTextVisible(false);
  mText->SetText("-");
  mInvalid = true;
}

//******************************************************************************
float Slider::GetPercentage()
{
  // Clamp the percentage
  float valueRange = (mMaxValue - mMinValue);
  float percentage = (mValue - mMinValue) / valueRange;
  return Math::Clamp(percentage, 0.0f, 1.0f);
}

//******************************************************************************
void Slider::SetPercentage(float percentage, bool sendMessage)
{
  // Clamp the percentage
  percentage = Math::Clamp(percentage, 0.0f, 1.0f);

  // Set the new value based on the percentage
  float valueRange = (mMaxValue - mMinValue);
  float newValue = mMinValue + valueRange * percentage;
  SetValue(newValue, sendMessage);
}

//******************************************************************************
float Slider::GetValue()
{
  return mValue;
}

//******************************************************************************
float SnapIncrement(float input, float increment)
{
  float normalized = input / increment;
  float rounded = floor(normalized + 0.5f) * increment;
  return rounded;
}

//******************************************************************************
void Slider::SetValue(float newValue, bool sendEvents)
{
  // We're now considered valid
  mInvalid = false;

  // Snap the value
  mValue = SnapIncrement(newValue, mIncrement);

  // Update the progress bar
  mProgressBar->SetPercentage(GetPercentage());

  // Send the event if specified
  if(sendEvents)
  {
    ObjectEvent eventToSend(this);
    GetDispatcher()->Dispatch(Events::SliderIncrementalChange, &eventToSend);
  }

  // Update the text to the new value
  UpdateText();

  MarkAsNeedsUpdate();
}

//******************************************************************************
bool Slider::TakeFocusOverride()
{
  this->HardTakeFocus();
  return true;
}

//******************************************************************************
void Slider::UpdateTransform()
{
  // Fill the progress bar
  mProgressBar->SetSize(mSize);

  // Only show the border if we have focus
  mBorder->SetSize(mSize);
  mBorder->SetColor(SliderUi::FocusBorderColor);
  mBorder->SetVisible(HasFocus());

  // If they're editing the value in a text box, no need
  // to update everything else
  if(mEditTextBox)
  {
    mEditTextBox->SetSize(mSize);
  }
  else
  {
    // If we're marked as invalid, don't touch the progress bar or text objects
    if(!mInvalid)
    {
      mProgressBar->SetProgressBarVisible(true);

      // Just use the progress bar text if we're already in Percentage mode
      bool useProgressBarText = (mType == SliderType::Percentage);
      mProgressBar->SetTextVisible(useProgressBarText);
      mText->SetActive(!useProgressBarText);
    }

    // Center the text
    Vec2 textSize = mText->GetMinSize();
    Vec3 textPos = ToVector3((mSize * 0.5f) - (textSize * 0.5f));
    mText->SetTranslation(SnapToPixels(textPos));
    mText->SetSize(textSize);
  }

  Composite::UpdateTransform();
}

//******************************************************************************
Vec2 Slider::GetMinSize()
{
  return mText->GetMinSize();
}

//******************************************************************************
void Slider::CommitValue(float newValue)
{
  SetValue(newValue, false);
  ObjectEvent eventToSend(this);
  GetDispatcher()->Dispatch(Events::SliderChanged, &eventToSend);
}

//******************************************************************************
void Slider::StartEditText()
{
  if(mEditTextBox == nullptr)
  {
    // Create a text box over the slider to edit the value
    mEditTextBox = new TextBox(this);
    mEditTextBox->HideBackground(true);
    mEditTextBox->SetText(String::Format("%g", mValue));
    mEditTextBox->SetEditable(true);
    mEditTextBox->TakeFocus();

    // We need to know when the text was submitted to set the new value
    ConnectThisTo(mEditTextBox, Events::TextSubmit, OnTextSubmit);
    ConnectThisTo(mEditTextBox, Events::FocusLost, OnTextFocusLost);

    // We don't want to see the progress bar or any other text objects
    mProgressBar->SetProgressBarVisible(false);
    mProgressBar->SetTextVisible(false);
    mText->SetActive(false);
  }
}

//******************************************************************************
void Slider::UpdateText()
{
  // Set the text
  String valueText = String::Format("%g", mValue);

  // If it's in degrees, add the degree symbol
  if(mType == SliderType::Degree)
    valueText = BuildString(valueText, "o");

  mText->SetText(valueText);
}

//******************************************************************************
void Slider::OnMouseDown(MouseEvent* e)
{
  ObjectEvent eventToSend(this);
  GetDispatcher()->Dispatch(Events::SliderManipulationStarted, &eventToSend);

  // Start the slider manipulation to edit the slider
  new SliderManipulation(e->GetMouse(), this, this);
}

//******************************************************************************
void Slider::OnRightClick(MouseEvent* e)
{
  StartEditText();
}

//******************************************************************************
void Slider::OnKeyDown(KeyboardEvent* e)
{
  // Do nothing if the event was already handled
  if(e->Handled)
    return;

  TabJump(this, e);

  switch(e->Key)
  {
    // Start editing the text
    case Keys::Enter:
    {
      StartEditText();
      break;
    }
    // Move the value to the left
    case Keys::Left:
    {
      ObjectEvent eventToSend(this);
      GetDispatcher()->Dispatch(Events::SliderManipulationStarted, &eventToSend);
      OnKeyRepeated(e);
      break;
    }
    // Move the value to the right
    case Keys::Right:
    {
      ObjectEvent eventToSend(this);
      GetDispatcher()->Dispatch(Events::SliderManipulationStarted, &eventToSend);
      OnKeyRepeated(e);
      break;
    }
  }
}

//******************************************************************************
void Slider::OnKeyRepeated(KeyboardEvent* e)
{
  switch(e->Key)
  {
    // Move the value to the left
  case Keys::Left:
    {
      // Clamp the value and set it
      float newValue = mValue - mIncrement;
      newValue = Math::Max(mMinValue, newValue);
      SetValue(newValue, true);
      mValueNudged = true;
      break;
    }
    // Move the value to the right
  case Keys::Right:
    {
      // Clamp the value and set it
      float newValue = mValue + mIncrement;
      newValue = Math::Min(mMaxValue, newValue);
      SetValue(newValue, true);
      mValueNudged = true;
      break;
    }
  }
}

//******************************************************************************
void Slider::OnKeyUp(KeyboardEvent* e)
{
  if(mValueNudged && (e->Key == Keys::Left || e->Key == Keys::Right))
  {
    CommitValue(mValue);
    mValueNudged = false;
  }
}

//******************************************************************************
void Slider::OnTextSubmit(Event* e)
{
  ObjectEvent eventToSend(this);
  GetDispatcher()->Dispatch(Events::SliderManipulationStarted, &eventToSend);

  // Set the new value
  String textValue = mEditTextBox->GetText();
  float newValue;
  Zero::ToValue(textValue.All(), newValue);
  CommitValue(newValue);

  // Destroy the text box
  mEditTextBox->Destroy();
  mEditTextBox = nullptr;

  /// Re-enable the progress bar
  mProgressBar->SetProgressBarVisible(true);

  MarkAsNeedsUpdate();
}

//******************************************************************************
void Slider::OnTextFocusLost(Event* e)
{
  if(mEditTextBox)
  {
    // Destroy the text box
    mEditTextBox->Destroy();
    mEditTextBox = nullptr;

    /// Re-enable the progress bar
    mProgressBar->SetProgressBarVisible(true);
  }

  MarkAsNeedsUpdate();
}

//******************************************************************************
void Slider::OnFocusLost(Event* e)
{
  if(mValueNudged)
    CommitValue(mValue);
}

}//namespace Zero
