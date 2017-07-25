///////////////////////////////////////////////////////////////////////////////
///
/// \file OnionSkinning.cpp
/// Implementation of the OnionSkinning class.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
void MakeInvisible(Cog* cog)
{
  //if(Model* model = cog->has(Model))
  //  model->mVisible = false;

  //if(Sprite* sprite = cog->has(Sprite))
  //  sprite->mVisible = false;

  forRange(Cog& child, cog->GetChildren())
  {
    MakeInvisible(&child);
  }
}

//--------------------------------------------------------------- Onion Skinning
//******************************************************************************
OnionSkinning::OnionSkinning(AnimationEditor* editor)
{
  mEditor = editor;
  mObjectCount = 5;
  mStepTime = 0.3f;
}

//******************************************************************************
void OnionSkinning::Update()
{
  // Clear any old objects
  Clear();

  // Pull needed information from the editor
  AnimationEditorData* editorData = mEditor->GetEditorData();
  AnimationSettings* settings = mEditor->GetSettings();
  AnimationScrubber* scrubber = mEditor->GetScrubber();

  // Do nothing if onion skinning is disabled
  if(!settings->mOnionSkinning)
    return;

  // Grab the objects needed from the editor
  Cog* animGraphObject = mEditor->GetAnimationGraphObject();
  Cog* selected = mEditor->GetSelectedObject();

  // Can't do anything without both objects
  if(animGraphObject == NULL || selected == NULL)
    return;

  // We need to get the Transform track for the currently selected object
  RichAnimation* richAnimation = editorData->mRichAnimation;

  // Find the object track
  String objectPath = GetObjectPath(selected, animGraphObject);
  TrackNode* objectTrack = richAnimation->GetObjectTrack(objectPath, false);

  // Nothing to do if there is no object track
  if(objectTrack == NULL)
    return;

  // Create all the preview objects
  CreateObjects();

  // Apply the translation track
  bool translationApplied = ApplyTrack(ZilchTypeId(Transform), "Translation");

  // If there was no translation track, no reason to show any other tracks
  if(!translationApplied)
  {
    Clear();
    return;
  }
  ApplyTrack(ZilchTypeId(Transform), "Rotation");
  ApplyTrack(ZilchTypeId(Transform), "Scale");
}

//******************************************************************************
void OnionSkinning::Clear()
{
  // Destroy all objects
  forRange(Cog* cog, mPreviewObjects.All())
  {
    if(cog)
      cog->Destroy();
  }

  mPreviewObjects.Clear();
}

//******************************************************************************
void OnionSkinning::CreateObjects()
{
  Cog* selected = mEditor->GetSelectedObject();

  // Save out the current object so we can copy it
  DataBlock data = Cog::SaveToDataBlock(selected);

  // Create all objects
  for(uint i = 0; i < mObjectCount; ++i)
  {
    // Create the object
    Cog* newObject = Cog::CreateFromDataBlock(selected->GetSpace(), data);

    // Attach to our parent if we're not a root object
    if(Cog* parent = selected->GetParent())
      newObject->AttachTo(parent);

    // We don't want it to show up in the editor or be selectable
    newObject->mFlags.SetFlag(CogFlags::Transient | CogFlags::Locked | CogFlags::ObjectViewHidden);

    // Make it invisible
    MakeInvisible(newObject);

    mPreviewObjects.PushBack(newObject);
  }
}

//******************************************************************************
bool OnionSkinning::ApplyTrack(BoundType* componentType, StringParam propertyName)
{
  RichAnimation* richAnim = mEditor->GetEditorData()->mRichAnimation;

  // We don't need to check these objects at this point because this function
  // should not be called unless they're valid
  Cog* animGraphObject = mEditor->GetAnimationGraphObject();
  Cog* selected = mEditor->GetSelectedObject();

  // Get the property track to sample the value
  TrackNode* track = richAnim->GetPropertyTrack(selected, animGraphObject,
                                            componentType, propertyName, false);

  // If the property track doesn't exist, there's nothing for us to do
  if(track == NULL)
    return false;

  // Grab the meta type so we can get the meta property
  Property* metaProperty = componentType->GetProperty(propertyName);

  // Apply the track to all objects
  for(uint i = 0; i < mObjectCount; ++i)
  {
    Cog* object = mPreviewObjects[i];

    // Get the sample time for this object
    float t = GetObjectSampleTime(i);

    // Sample the track
    Any sampleValue = track->SampleTrack(t);

    // Grab the component to set the property on
    Component* component = object->QueryComponentType(componentType);
    metaProperty->SetValue(component, sampleValue);
  }

  return true;
}

//******************************************************************************
float OnionSkinning::GetObjectSampleTime(uint objectIndex)
{
  // Offset it by the scrubber time
  AnimationScrubber* scrubber = mEditor->GetScrubber();
  float playHead = scrubber->GetPlayHead();
  float duration = mEditor->GetEditorData()->mRichAnimation->mDuration;

  // [0, 1]
  float t = (float)objectIndex / (float)(mObjectCount - 1);

  // [0, duration]
  t *= duration;

  // [-duration / 2, duration / 2]
  t -= (duration * 0.5f);

  // Scale
  t *= mStepTime;

  // Center around play head
  t += playHead;

  return t;
}

}//namespace Zero
