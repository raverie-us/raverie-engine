///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationControls.cpp
/// Implementation of AnimationControls helper class.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace AnimControlsUi
{
const cstr cLocation = "EditorUi/AnimationEditor/AnimControls";
Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
}

void CreateSpacer(Composite* parent, float size) 
{
  Spacer* spacer = new Spacer(parent); 
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, size);
}

void CreateButton(IconButton*& buttonVar, StringParam icon, Composite* parent, Vec2 size) 
{
  buttonVar = new IconButton(parent);
  buttonVar->SetIcon(icon);
  buttonVar->SetSizing(SizeAxis::X, SizePolicy::Fixed, size.x);
}

//------------------------------------------------------------- Animation Button
//******************************************************************************
AnimationButton::AnimationButton(Composite* parent, StringParam element) 
  : Composite(parent)
{
  mImage = CreateAttached<Element>(element);

  ConnectThisTo(mImage, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(mImage, Events::MouseExit, OnMouseExit);
}

//******************************************************************************
void AnimationButton::SetElement(StringParam element)
{
  BaseDefinition* definition = mDefSet->GetDefinition(element);
  mImage->ChangeDefinition(definition);
}

//******************************************************************************
void AnimationButton::OnMouseEnter(MouseEvent* e)
{
  Vec4 color = mColor * Vec4(0.8f, 0.7f, 0.7f, 1);
  mColor = color;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationButton::OnMouseExit(MouseEvent* e)
{
  mColor = Vec4(1,1,1,1);
  MarkAsNeedsUpdate();
}

//----------------------------------------------------------- Animation Selector
//******************************************************************************
AnimationSelector::AnimationSelector(Composite* parent, AnimationEditor* editor)
  : Composite(parent), mEditor(editor)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(3,0), Thickness::cZero));

  mAnimationBox = new TextBox(this);
  mAnimationBox->SetText("No Animation");
  mAnimationBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(214));
  ConnectThisTo(mAnimationBox, Events::LeftClick, OnAnimationBoxLeftClick);

  IconButton* button = new IconButton(this);
  button->SetIcon("AnimPlus");
  ConnectThisTo(button, Events::ButtonPressed, OnAddButtonPressed);
}

//******************************************************************************
void AnimationSelector::OnAnimationBoxLeftClick(MouseEvent* e)
{
  /// Don't do anything unless we're in a somewhat valid state
  ErrorState::Type state = mEditor->GetErrorState();
  if(state != ErrorState::None && state != ErrorState::ReadOnly)
    return;

  FloatingSearchView* searchView = mActiveSearch;
  if(searchView==NULL)
  {
    FloatingSearchView* viewPopUp = new FloatingSearchView(this);
    Vec3 mousePos = ToVector3(e->GetMouse()->GetClientPosition());
    SearchView* searchView = viewPopUp->mView;
    viewPopUp->SetSize(Pixels(300,400));
    viewPopUp->ShiftOntoScreen(mousePos);
    viewPopUp->UpdateTransformExternal();

    searchView->mSearch->ActiveTags.Insert("Resources");
    searchView->mSearch->ActiveTags.Insert("Animation");
    searchView->mSearch->SearchProviders.PushBack(GetResourceSearchProvider()) ;

    searchView->TakeFocus();
    viewPopUp->UpdateTransformExternal();
    searchView->Search(String());
    ConnectThisTo(searchView, Events::SearchCompleted, OnAnimationSelected);

    mActiveSearch = viewPopUp;
  }
}

//******************************************************************************
void AnimationSelector::OnAnimationSelected(SearchViewEvent* e)
{
  Animation* animation = (Animation*)e->Element->Data;
  mEditor->SetAnimation(animation);

  mActiveSearch.SafeDestroy();
}

//******************************************************************************
void AnimationSelector::OnAddButtonPressed(ObjectEvent* e)
{
  mEditor->OpenAnimationAddWindow();
}


//******************************************************************************
void AnimationSelector::Hide()
{
  mAnimationBox->SetText("No Animation");
}

//******************************************************************************
void AnimationSelector::Show()
{
  if(AnimationEditorData* editorData = mEditor->GetEditorData())
  {
    Resource* animation = editorData->mAnimation;
    mAnimationBox->SetText(animation->Name);
  }
}

//----------------------------------------------------------- Animation Tool Box
AnimationToolBox::AnimationToolBox(Composite* parent, AnimationEditor* editor)
  : Composite(parent), mEditor(editor)
{
  mSettings = mEditor->GetSettings();
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(4,0), Thickness(Pixels(20, 0, 0, 0))));

  mCurveToolBar = new CurveEditing::CurveEditorToolbar(this);

  Spacer* spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(4));

  // Auto Key
  mAutoKey = new ToggleIconButton(this);
  mAutoKey->SetToolTip("Auto Key");
  mAutoKey->SetEnabledIcon("AnimatorAutoKey");
  mAutoKey->SetDisabledIcon("AnimatorAutoKeyDisabled");
  mAutoKey->SetEnabled(false);
  mAutoKey->mIgnoreClicks = true;
  mAutoKey->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  ConnectThisTo(mAutoKey, Events::ButtonPressed, OnAutoKeyPressed);

  // Auto Focus
  mAutoFocus = new ToggleIconButton(this);
  mAutoFocus->SetToolTip("Auto Focus");
  mAutoFocus->SetEnabledIcon("AnimatorAutoFocus");
  mAutoFocus->SetDisabledIcon("AnimatorAutoFocusDisabled");
  mAutoFocus->SetEnabled(false);
  mAutoFocus->mIgnoreClicks = true;
  mAutoFocus->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  ConnectThisTo(mAutoFocus, Events::ButtonPressed, OnAutoFocusPressed);

  // Snapping X
  mSnappingX = new ToggleIconButton(this);
  mSnappingX->SetToolTip("Snapping X");
  mSnappingX->SetEnabledIcon("AnimatorSnappingX");
  mSnappingX->SetDisabledIcon("AnimatorSnappingXDisabled");
  mSnappingX->SetEnabled(false);
  mSnappingX->mIgnoreClicks = true;
  mSnappingX->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  ConnectThisTo(mSnappingX, Events::ButtonPressed, OnSnappingXPressed);

  // Snapping Y
  mSnappingY = new ToggleIconButton(this);
  mSnappingY->SetToolTip("Snapping Y");
  mSnappingY->SetEnabledIcon("AnimatorSnappingY");
  mSnappingY->SetDisabledIcon("AnimatorSnappingYDisabled");
  mSnappingY->SetEnabled(false);
  mSnappingY->mIgnoreClicks = true;
  mSnappingY->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  ConnectThisTo(mSnappingY, Events::ButtonPressed, OnSnappingYPressed);

  // Disabled until implemented
  mSnappingY->SetActive(false);

  // Onion Skinning
  mOnionSkinning = new ToggleIconButton(this);
  mOnionSkinning->SetToolTip("Onion Skinning");
  mOnionSkinning->SetEnabledIcon("OnionSkinning");
  mOnionSkinning->SetDisabledIcon("OnionSkinningDisabled");
  mOnionSkinning->SetEnabled(false);
  mOnionSkinning->mIgnoreClicks = true;
  mOnionSkinning->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  ConnectThisTo(mOnionSkinning, Events::ButtonPressed, OnOnionSkinningPressed);

  spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(4));

  mEditFpsBox = new ComboBox(this);
  mSource = new StringSource();
  forRange(auto entry, AnimationSettings::mEditFpsPresets.All())
  {
    mSource->Strings.PushBack(entry.first);
  }
  mEditFpsBox->SetListSource(mSource);
  mEditFpsBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(70));
  ConnectThisTo(mEditFpsBox, Events::ItemSelected, OnEditFpsSelected);

  // Disabled until more clear
  mEditFpsBox->SetActive(false);

  spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(4));

  mTimeDisplaySelector = new SelectorButton(this);
  mTimeDisplaySelector->CreateButtons(TimeDisplay::Names, TimeDisplay::Size);
  ConnectThisTo(mTimeDisplaySelector, Events::ItemSelected, OnTimeDisplaySelected);

  spacer = new Spacer(this);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(4));

  // Linear Tangents
  mLinearTangents = new IconButton(this);
  mLinearTangents->SetToolTip("Linear Tangents");
  mLinearTangents->SetIcon("LinearTangents");
  mLinearTangents->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));

  // Split Tangents
  mSplitTangents = new IconButton(this);
  mSplitTangents->SetToolTip("Split Tangents");
  mSplitTangents->SetIcon("SplitTangents");
  mSplitTangents->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));

  // Weighted Tangents
  mWeightedTangents = new IconButton(this);
  mWeightedTangents->SetToolTip("Weighted Tangents");
  mWeightedTangents->SetIcon("WeightedTangents");
  mWeightedTangents->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));

  mStatusText = new Label(this, cText);
  mStatusText->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  mStatusText->mText->mAlign = TextAlign::Right;
  SetStatusText(String());
}

//******************************************************************************
AnimationToolBox::~AnimationToolBox()
{
  SafeDelete(mSource);
}

//******************************************************************************
void AnimationToolBox::SetStatusText(StringParam text, ByteColor color)
{
  Vec4 newColor = ToFloatColor(color);
  if(text.Empty())
  {
    newColor = mStatusText->GetColor();
    newColor.w = 0.0f;
  }
  else
  {
    mStatusText->SetText(text);
  }

  mStatusText->GetActions()->Cancel();
  ActionSequence* seq = new ActionSequence(mStatusText, ActionExecuteMode::FrameUpdate);
  seq->Add(AnimatePropertyGetSet(Widget, Color, Ease::Quad::InOut, mStatusText, 0.16f, newColor));
}

//******************************************************************************
void AnimationToolBox::Hide()
{
  mAutoFocus->mIgnoreClicks = true;
  mAutoKey->mIgnoreClicks = true;
  mSnappingX->mIgnoreClicks = true;
  mSnappingY->mIgnoreClicks = true;
  mOnionSkinning->mIgnoreClicks = true;

  mAutoFocus->SetEnabled(false);
  mAutoKey->SetEnabled(false);
  mSnappingX->SetEnabled(false);
  mSnappingY->SetEnabled(false);
  mOnionSkinning->SetEnabled(false);
}

//******************************************************************************
void AnimationToolBox::Show()
{
  mAutoFocus->mIgnoreClicks = false;
  mAutoKey->mIgnoreClicks = false;
  mSnappingX->mIgnoreClicks = false;
  mSnappingY->mIgnoreClicks = false;
  mOnionSkinning->mIgnoreClicks = false;

  AnimationSettings* settings = mEditor->GetSettings();
  mAutoFocus->SetEnabled(settings->mAutoFocus);
  mAutoKey->SetEnabled(settings->mAutoKey);
  mSnappingX->SetEnabled(settings->mSnappingX);
  mSnappingY->SetEnabled(settings->mSnappingY);
  mOnionSkinning->SetEnabled(settings->mOnionSkinning);

  mEditFpsBox->SetSelectedItem(settings->GetEditFps(), false);
  mTimeDisplaySelector->SetSelectedItem((uint)settings->mTimeDisplay, false);
}

//******************************************************************************
void AnimationToolBox::OnAutoFocusPressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->mAutoFocus = mAutoFocus->GetEnabled();
}

//******************************************************************************
void AnimationToolBox::OnAutoKeyPressed(Event* e)
{
  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->mAutoKey = mAutoKey->GetEnabled();
}

//******************************************************************************
void AnimationToolBox::OnSnappingXPressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->mSnappingX = mSnappingX->GetEnabled();
}

//******************************************************************************
void AnimationToolBox::OnSnappingYPressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->mSnappingY = mSnappingY->GetEnabled();
}

//******************************************************************************
void AnimationToolBox::OnOnionSkinningPressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->mOnionSkinning = mOnionSkinning->GetEnabled();
}

//******************************************************************************
void AnimationToolBox::OnEditFpsSelected(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->SetEditFps((uint)mEditFpsBox->GetSelectedItem());
}

//******************************************************************************
void AnimationToolBox::OnTimeDisplaySelected(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  AnimationEditorData* data = mEditor->GetEditorData();
  mSettings->mTimeDisplay = (TimeDisplay::Enum)mTimeDisplaySelector->GetSelectedItem();
}

//------------------------------------------------------------- Dual Play Button
//******************************************************************************
PlayControls::PlayControls(Composite* parent, AnimationControls* controls)
  : Composite(parent), mControls(controls)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));

  mPlayLeft = new IconButton(this);
  mPlayLeft->SetIcon("AnimPlayLeft");
  mPlayLeft->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(31));
  ConnectThisTo(mPlayLeft, Events::ButtonPressed, OnPlayLeftPressed);

  mPlayRight = new IconButton(this);
  mPlayRight->SetIcon("AnimPlay");
  mPlayRight->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(31));
  ConnectThisTo(mPlayRight, Events::ButtonPressed, OnPlayRightPressed);

  mPause = new IconButton(this);
  mPause->SetIcon("AnimPause");
  mPause->SetActive(false);
  mPause->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(62));
  ConnectThisTo(mPause, Events::ButtonPressed, OnPausePressed);
}

//******************************************************************************
void PlayControls::Pause()
{
  mPlayLeft->SetActive(true);
  mPlayRight->SetActive(true);
  mPause->SetActive(false);

  // Update the mode and button
  mControls->mPreviewDirection = PreviewDirection::Paused;
}

//******************************************************************************
void PlayControls::OnPlayLeftPressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mControls->mEditor->GetErrorState() != ErrorState::None)
    return;

  mPlayLeft->SetActive(false);
  mPlayRight->SetActive(false);
  mPause->SetActive(true);

  mControls->mPreviewDirection = PreviewDirection::Backward;
  mControls->mCurrTime = mControls->mScrubber->GetPlayHead();
}

//******************************************************************************
void PlayControls::OnPlayRightPressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mControls->mEditor->GetErrorState() != ErrorState::None)
    return;

  mPlayLeft->SetActive(false);
  mPlayRight->SetActive(false);
  mPause->SetActive(true);

  mControls->mPreviewDirection = PreviewDirection::Forward;

  // If we're at or passed the end of the animation, start it over
  float duration = mControls->mEditorData->mRichAnimation->mDuration;
  if(mControls->mScrubber->GetPlayHead() >= duration)
    mControls->mCurrTime = 0.0f;
  else
    mControls->mCurrTime = mControls->mScrubber->GetPlayHead();
}

//******************************************************************************
void PlayControls::OnPausePressed(Event* e)
{
  // Do nothing if we're in an error state
  if(mControls->mEditor->GetErrorState() != ErrorState::None)
    return;

  Pause();

  // Snap to the nearest key frame
  float editFps = mControls->mEditor->GetSettings()->mEditFps;
  float time = Math::Floor(mControls->mCurrTime * editFps + 0.5f) / editFps;
  mControls->UpdateToTime(time, false);
}

//----------------------------------------------------------- Animation Controls
//******************************************************************************
AnimationControls::AnimationControls(Composite* parent, AnimationEditor* editor)
  : Composite(parent)
{
  mEditor = editor;
  mScrubber = NULL;
  mEditorData = NULL;
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0,2), Thickness(Pixels(3, 4, 3, 4))));

  mBackground = CreateAttached<Element>("ScrubBackground");
  mBackground->SetNotInLayout(true);

  mPreviewDirection = PreviewDirection::Paused;
  const Vec2 cButtonSize = Pixels(40, 20);
  
  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(2, 0), Thickness::cZero));
  buttonRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(20));
  buttonRow->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  {
    CreateButton(mButtonBegin, "AnimRewindFull", buttonRow, cButtonSize);
    CreateButton(mButtonShiftLeft, "AnimRewind", buttonRow, cButtonSize);
  
    mPlayControls = new PlayControls(buttonRow, this);
    mPlayControls->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(62));

    CreateButton(mButtonShiftRight, "AnimFastForward", buttonRow, cButtonSize);
    CreateButton(mButtonEnd, "AnimFastForwardFull", buttonRow, cButtonSize);
  }
  
  mPlayModeSelector = new SelectorButton(this);
  mPlayModeSelector->CreateButtons(AnimationPlayMode::Names, AnimationPlayMode::Size);
  mPlayModeSelector->SetSelectedItem(AnimationPlayMode::Loop, false);
  mPlayModeSelector->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  mPlayModeSelector->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(20));
  ConnectThisTo(mPlayModeSelector, Events::ItemSelected, OnPlaybackModeSelected);

  mPlaybackSpeedSlider = new Slider(this, SliderType::Number);
  mPlaybackSpeedSlider->SetRange(0, 2);
  mPlaybackSpeedSlider->SetValue(1, false);
  mPlaybackSpeedSlider->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  mPlaybackSpeedSlider->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
  mPlaybackSpeedSlider->SetIncrement(0.01f);
  ConnectThisTo(mPlaybackSpeedSlider, Events::SliderIncrementalChange, OnPlaybackSliderChanged);
  ConnectThisTo(mPlaybackSpeedSlider, Events::SliderChanged, OnPlaybackSliderChanged);
  ConnectThisTo(mPlaybackSpeedSlider, Events::MouseEnter, OnMouseEnterPlaybackSpeed);
  ConnectThisTo(mPlaybackSpeedSlider, Events::MouseExit, OnMouseExitPlaybackSpeed);

  // We need to update the animation on frame update if we're playing
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
  
  //// Connect to events so we know when the buttons are pressed
  ConnectThisTo(mButtonBegin, Events::LeftMouseDown, OnBeginPressed);
  ConnectThisTo(mButtonShiftLeft, Events::LeftMouseDown, OnShiftLeftPressed);
  ConnectThisTo(mButtonShiftRight, Events::LeftMouseDown, OnShiftRightPressed);
  ConnectThisTo(mButtonEnd, Events::LeftMouseDown, OnEndPressed);
}

//******************************************************************************
void AnimationControls::SetScrubber(AnimationScrubber* scrubber)
{
  mScrubber = scrubber;
}

//******************************************************************************
void AnimationControls::SetAnimationEditorData(AnimationEditorData* editorData)
{
  mEditorData = editorData;
  mPlayControls->Pause();
}

//******************************************************************************
void AnimationControls::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetColor(AnimControlsUi::BackgroundColor);

  Composite::UpdateTransform();
}

//******************************************************************************
void AnimationControls::Hide()
{
  mPlayControls->Pause();
}

//******************************************************************************
void AnimationControls::Show()
{

}

//******************************************************************************
void AnimationControls::OnUpdate(UpdateEvent* event)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  // Do nothing if we're paused
  if(mPreviewDirection == PreviewDirection::Paused)
    return;

  // Account for the direction we're playing in
  float direction = 1.0f;
  if(mPreviewDirection == PreviewDirection::Backward)
    direction = -1.0f;

  AnimationSettings* settings = mEditor->GetSettings();
  float playbackSpeed = settings->mPlaybackSpeed;

  // Step forward or backwards
  mCurrTime += event->Dt * direction * playbackSpeed;

  // Update the scrubber
  bool looping = (settings->mPreviewMode == AnimationPlayMode::Loop);
  bool wrapped = UpdateToTime(mCurrTime, looping, true);

  if(wrapped)
  {
    // If we've hit the end (or start) of the animation and we're in single
    // mode, pause the animation
    if(settings->mPreviewMode == AnimationPlayMode::PlayOnce)
    {
      mPlayControls->Pause();
    }
    // If we're in ping-pong mode, play in the opposite direction
    else if(settings->mPreviewMode == AnimationPlayMode::Pingpong)
    {
      if(mPreviewDirection == PreviewDirection::Forward)
        mPreviewDirection = PreviewDirection::Backward;
      else
        mPreviewDirection = PreviewDirection::Forward;
    }
    // If we're in loop mode, do nothing as the UpdateToTime function
    // wrapped the current time for us
  }
}

//******************************************************************************
void AnimationControls::OnBeginPressed(MouseEvent* event)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  // Move the scrubber to the beginning
  if(mPreviewDirection == PreviewDirection::Paused)
    UpdateToTime(0.0f, false);
}

//******************************************************************************
void AnimationControls::OnShiftLeftPressed(MouseEvent* event)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  // Shift left one second (one frame if snapping is enabled)
  if(mPreviewDirection == PreviewDirection::Paused)
  {
    float editFps = mEditor->GetSettings()->mEditFps;
    UpdateToTime(mScrubber->GetPlayHead() - 1.0f / editFps, false);
  }
}

//******************************************************************************
void AnimationControls::OnShiftRightPressed(MouseEvent* event)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  // Shift right one second (one frame if snapping is enabled)
  if(mPreviewDirection == PreviewDirection::Paused)
  {
    float editFps = mEditor->GetSettings()->mEditFps;
    UpdateToTime(mScrubber->GetPlayHead() + 1.0f / editFps, false);
  }
}

//******************************************************************************
void AnimationControls::OnEndPressed(MouseEvent* event)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  // Move the scrubber to the end of the animation
  if(mPreviewDirection == PreviewDirection::Paused)
    UpdateToTime(mEditorData->mRichAnimation->mDuration, false);
}

//******************************************************************************
void AnimationControls::OnPlaybackModeSelected(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  mEditor->GetSettings()->mPreviewMode = mPlayModeSelector->GetSelectedItem();
}

//******************************************************************************
void AnimationControls::OnPlaybackSliderChanged(Event* e)
{
  // Do nothing if we're in an error state
  if(mEditor->GetErrorState() != ErrorState::None)
    return;

  mEditor->GetSettings()->mPlaybackSpeed = mPlaybackSpeedSlider->GetValue();
}

//******************************************************************************
bool AnimationControls::UpdateToTime(float time, bool wrap, bool clampToDuration)
{
  bool wrapped = false;

  float duration = mEditorData->mRichAnimation->mDuration;

  // Check the left bound of the animation
  if(time < 0.0f)
  {
    if(wrap)
      time = duration + time;
    wrapped = true;
  }
  // Check the right bound of the animation
  else if(time > duration)
  {
    if(wrap)
      time = time - duration;
    wrapped = true;
  }

  // Clamp the time to the bounds of the animation
  mCurrTime = Math::Max(time, 0.0f);
  if(clampToDuration)
    mCurrTime = Math::Min(mCurrTime, duration);

  // Set the play head on the scrubber
  mScrubber->SetPlayHead(mCurrTime, true);

  return wrapped;
}

//******************************************************************************
void AnimationControls::OnMouseEnterPlaybackSpeed(MouseEvent* e)
{
  // Create the tooltip
  ToolTip* toolTip = new ToolTip(this);
  toolTip->SetText("Preview Time Scale");

  // Position the tooltip
  ToolTipPlacement placement;
  placement.SetScreenRect(mPlaybackSpeedSlider->GetScreenRect());
  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Top, 
                        IndicatorSide::Left, IndicatorSide::Bottom);
  toolTip->SetArrowTipTranslation(placement);

  mPlaybackSpeedTooltip = toolTip;
}

//******************************************************************************
void AnimationControls::OnMouseExitPlaybackSpeed(MouseEvent* e)
{
  mPlaybackSpeedTooltip.SafeDestroy();
}

}//namespace Zero
