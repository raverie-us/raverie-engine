///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationSettings.cpp
/// Implementation of settings for the animator.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------ Animation Options
AnimationSettings::EditFpsPresetArray AnimationSettings::mEditFpsPresets;

//******************************************************************************
void GetEditFpsPresets(HandleParam instance, Property* property, 
                       Array<String>& strings)
{
  strings.Resize(AnimationSettings::mEditFpsPresets.Size());

  for(uint i = 0; i < AnimationSettings::mEditFpsPresets.Size(); ++i)
    strings[i] = AnimationSettings::mEditFpsPresets[i].first;
}

//******************************************************************************
ZilchDefineType(AnimationSettings, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindGetterSetterProperty(EditFps)->Add(new EditorIndexedStringArray(GetEditFpsPresets));
  ZilchBindFieldProperty(mTimeDisplay);
  ZilchBindFieldProperty(mSnappingX);
  ZilchBindFieldProperty(mSnappingY);
  ZilchBindFieldProperty(mAutoKey);
  ZilchBindFieldProperty(mAutoFocus);
  ZilchBindFieldProperty(mOnionSkinning);
  ZilchBindFieldProperty(mPlaybackSpeed);
  ZilchBindFieldProperty(mPreviewMode);

  mEditFpsPresets.Resize(4);
  mEditFpsPresets[0] = EditFpsPreset("12fps", 12.0f);
  mEditFpsPresets[1] = EditFpsPreset("24fps", 24.0f);
  mEditFpsPresets[2] = EditFpsPreset("30fps", 30.0f);
  mEditFpsPresets[3] = EditFpsPreset("60fps", 60.0f);
}

//******************************************************************************
AnimationSettings::AnimationSettings()
{
  mEditFps = 30.0f;
  mTimeDisplay = TimeDisplay::Frames;
  mSnappingX = true;
  mSnappingY = false;
  mOnionSkinning = false;
  mAutoKey = false;
  mAutoFocus = true;
  mStartTime = 0.0f;
  mEndTime = 30.0f;
  mPlaybackSpeed = 1.0f;
  mPreviewMode = AnimationPlayMode::Loop;
}

//******************************************************************************
void AnimationSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mEditFps, 30.0f);
  SerializeEnumName(TimeDisplay, mTimeDisplay);
  SerializeNameDefault(mSnappingX, true);
  SerializeNameDefault(mAutoKey, false);
  SerializeNameDefault(mAutoFocus, true);
  SerializeNameDefault(mStartTime, 0.0f);
  SerializeNameDefault(mEndTime, 30.0f);
  SerializeNameDefault(mPlaybackSpeed, 1.0f);
  SerializeEnumName(AnimationPlayMode, mPreviewMode);
}

//******************************************************************************
void AnimationSettings::SetEditFps(uint index)
{
  mEditFps = mEditFpsPresets[index].second;
}

//******************************************************************************
uint AnimationSettings::GetEditFps()
{
  for(uint i = 0; i < mEditFpsPresets.Size(); ++i)
  {
    if(mEditFps == mEditFpsPresets[i].second)
      return i;
  }

  return 0;
}

//------------------------------------------------------ Animation Settings View
//******************************************************************************
AnimationSettingsView::AnimationSettingsView(Composite* parent, AnimationEditor* editor)
 : Composite(parent), mEditor(editor)
{
  SetClipping(true);

  // Create a filled background element
  mBackground = CreateAttached<Element>("GraphBackground");

  mGroup = new Composite(this);
  mGroup->SetLayout(CreateStackLayout());

  mRichAnimProperties = new PropertyView(mGroup);
  mRichAnimProperties->SetSize(Pixels(100, 20));
  mRichAnimProperties->mNamePercent = 0.55f;

  mSettingsProperties = new PropertyView(mGroup);
  mSettingsProperties->mNamePercent = 0.55f;

}

//******************************************************************************
void AnimationSettingsView::SetAnimationEditorData(AnimationEditorData* editorData)
{
  mRichAnimProperties->SetObject(editorData->mRichAnimation);
  mRichAnimProperties->Invalidate();
  
  mSettingsProperties->SetObject(mEditor->GetSettings());
  mSettingsProperties->Invalidate();
}

//******************************************************************************
void AnimationSettingsView::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetVisible(true);
  
  Thickness borderThickness = Thickness::All(5);
  LayoutResult lr = RemoveThickness(borderThickness, mSize);
  mGroup->SetTranslation(lr.Translation);
  mGroup->SetSize(lr.Size);

//   mRichAnimProperties->SetTranslation(Pixels(5,5,0));
//   mRichAnimProperties->SetSize(Pixels(10,10));
//   mRichAnimProperties->Refresh();
// 
//   mSettingsProperties->SetTranslation(Pixels(5,5,0));
//   mSettingsProperties->SetSize(mSize - Pixels(10,10));
//   mSettingsProperties->Refresh();

  Composite::UpdateTransform();
}

}//namespace Zero
