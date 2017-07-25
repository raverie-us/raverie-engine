///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationEditorData.cpp
/// Implementation of AnimationEditorData.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(TrackSelectionModified);
}

//******************************************************************************
String GetObjectNameFromPath(StringParam objectPath)
{
  // This uses a '.' because the Animation data file uses '.' as a delimiter.
  StringTokenRange r = StringTokenRange(objectPath, '.');

  // Default case
  String last = String();

  while(!r.Empty())
  {
    last = r.Front();
    r.PopFront();
  }

  return last;
}

//-------------------------------------------------------- Animation Editor Data
ZilchDefineType(AnimationEditorData, builder, type)
{
}

//******************************************************************************
AnimationEditorData::AnimationEditorData(AnimationEditor* editor,
                                         Cog* animGraphObject,
                                         Animation* animation,
                                         ScrollingGraph* graphData)
{
  mEditor = editor;
  mGraphData = graphData;
  mAnimation = animation;

  ContentItem* contentItem = animation->mContentItem;
  RichAnimationBuilder* builder = contentItem->has(RichAnimationBuilder);

  if(builder == nullptr)
  {
     mRichAnimation = ConvertToRichAnimation(animation);
     //contentItem->AddComponent(mRichAnimation);
     //contentItem->SaveContent();
  }
  else
  {
    // The Load the rich animation from file
    String path = animation->mContentItem->GetFullPath();

    mRichAnimation = new RichAnimation();
    LoadFromDataFile(*mRichAnimation, path);
    mRichAnimation->Initialize();
  }

  ConnectThisTo(mRichAnimation, Events::TrackDeleted, OnTrackDeleted);
  ConnectThisTo(mRichAnimation, Events::AnimationModified, OnAnimationModified);
}

//******************************************************************************
void AnimationEditorData::OnAnimationModified(Event* e)
{
  BakeToAnimation();
  mEditor->UpdateToScrubber();
}

//******************************************************************************
void AnimationEditorData::OnTrackDeleted(TrackEvent* e)
{
  // We only want to send the selection modified event if the deleted
  // track was visible
  if(mVisiblePropertyTracks.Contains(e->mTrack))
  {
    // Erase the track from the visible list
    mVisiblePropertyTracks.Erase(e->mTrack);
    Event e;
    GetDispatcher()->Dispatch(Events::TrackSelectionModified, &e);
  }
}

//******************************************************************************
void AnimationEditorData::BakeToAnimation()
{
  Animation* animation = mAnimation;
  if(animation == nullptr)
    return;

  mRichAnimation->BakeToAnimation(animation);

  if(animation->mContentItem)
    animation->mContentItem->SaveContent();
}

//******************************************************************************
void AnimationEditorData::SetSelection(Array<TrackNode*>& selection)
{
  // Clear the previous selection
  mVisiblePropertyTracks.Clear();

  // Add all tracks
  for(uint i = 0; i < selection.Size(); ++i)
    mVisiblePropertyTracks.Insert(selection[i]);

  // Signal that the selection was modified
  Event e;
  GetDispatcher()->Dispatch(Events::TrackSelectionModified, &e);
}

//******************************************************************************
void AnimationEditorData::AddToSelection(TrackNode* track)
{
  // If it's a vector type track, just add the children to the selection
  if(track->Type == TrackType::Property && !track->Children.Empty())
  {
    // Add all children to the visible tracks
    forRange(TrackNode* child, track->Children.All())
    {
      mVisiblePropertyTracks.Insert(child);
    }
  }
  else
  {
    // Add the track to the selection
    mVisiblePropertyTracks.Insert(track);
  }

  // Signal that the selection was modified
  Event e;
  GetDispatcher()->Dispatch(Events::TrackSelectionModified, &e);
}

//******************************************************************************
void AnimationEditorData::SaveRichAnimation()
{
  // Save the rich animation
  if(Animation* animation = mAnimation)
  {
    String file = animation->mContentItem->GetFullPath();
    SaveToDataFile(*mRichAnimation, file);
  }
}

//******************************************************************************
void LinearizeTrack(TrackNode* track)
{
  TrackNode::KeyFrames::ArrayType& keyFrames = track->mKeyFrames.mArray;

  uint count = keyFrames.Size();
  for(uint i = 0; i < count; ++i)
  {
    // Grab all valid key frames
    KeyFrame* previous = (i > 0) ? keyFrames[i - 1].second : nullptr;
    KeyFrame* current = keyFrames[i].second;
    KeyFrame* next = (i < count - 1) ? keyFrames[i + 1].second : nullptr;

    //using namespace CurveEditing;
    //current->mEditorFlags |= (CurveEditorFlags::LinearIn | CurveEditorFlags::LinearOut);
    //current->mEditorFlags |= CurveEditorFlags::TangentsSplit;

    // Can only do this for float types
    if(current->GetValue().Is<float>())
    {
      Vec2 currentValue = current->GetGraphPosition();

      // Update the in tangent
      if(previous)
      {
        Vec2 previousValue = previous->GetGraphPosition();
        current->SetTangentIn((previousValue - currentValue).Normalized() * 0.03f);
      }

      // Update the out tangent
      if(next)
      {
        Vec2 nextValue = next->GetGraphPosition();
        current->SetTangentOut((nextValue - currentValue).Normalized() * 0.03f);
      }
    }
  }

  // Linearize all children
  forRange(TrackNode* child, track->Children.All())
  {
    LinearizeTrack(child);
  }
}

//******************************************************************************
void LoadKeyFramesIntoTrack(TrackNode* track, PropertyTrack* animTrack)
{
  Array<float> times;
  Array<Any> values;
  animTrack->GetKeyTimes(times);
  animTrack->GetKeyValues(values);

  for(uint i = 0; i < times.Size(); ++i)
    track->CreateKeyFrame(times[i], values[i]);

  // Linearize the track
  LinearizeTrack(track);
}

//******************************************************************************
RichAnimation* ConvertToRichAnimation(Animation* animation)
{
  ErrorIf(animation == nullptr, "Invalid animation.");

  RichAnimation* richAnim = new RichAnimation();

  // Set the duration on the animation
  richAnim->mDuration = animation->GetDuration();

  // Create the root node
  richAnim->mRoot = new TrackNode("Root", String(), TrackType::Object,
                                  nullptr, nullptr, richAnim);

  HashMap<String, TrackNode*> objectTracks;
  // Create a node for each object track
  forRange(ObjectTrack& objectTrack, animation->ObjectTracks.All())
  {
    // Create the object node
    String objectName = GetObjectNameFromPath(objectTrack.FullPath);
    TrackNode* objectNode = new TrackNode(objectName, objectTrack.FullPath,
                                       TrackType::Object, nullptr, nullptr, richAnim);
    objectTracks.Insert(objectNode->Path, objectNode);

    // Used to store all the property tracks for each unique component
    typedef HashMap<String, Array<PropertyTrack*>> ComponentTrackMap;
    ComponentTrackMap componentTracks;

    // We want to group all property tracks under the same component
    forRange(PropertyTrack& propertyTrack, objectTrack.PropertyTracks.All())
    {
      // The name of the component
      String componentName = ComponentNameFromPath(propertyTrack.Name);
      ErrorIf(componentName.Empty(), "Invalid component name for property track.");

      // Attempt to find the array
      Array<PropertyTrack*>* tracks = componentTracks.FindPointer(componentName);

      // If it wasn't inserted, we want to Insert it
      if(tracks == nullptr)
      {
        componentTracks.Insert(componentName, Array<PropertyTrack*>());
        tracks = componentTracks.FindPointer(componentName);
      }

      // Add the track
      tracks->PushBack(&propertyTrack);
    }

    // Create a component node for each unique component
    ComponentTrackMap::range r = componentTracks.All();
    while(!r.Empty())
    {
      // Create the component node
      String componentName = r.Front().first;
      String componentPath = componentName;
      TrackNode* componentNode = new TrackNode(componentName, componentPath,
                              TrackType::Component, nullptr, objectNode, richAnim);

      // Add each property track
      Array<PropertyTrack*>& propertyTracks = r.Front().second;
      for(uint i = 0; i < propertyTracks.Size(); ++i)
      {
        PropertyTrack* propertyTrack = propertyTracks[i];

        // Create the property node
        String propertyName = PropertyNameFromPath(propertyTrack->Name);
        String propertyPath = BuildString(componentName, ".", propertyName);
        BoundType* targetMeta = MetaDatabase::GetInstance()->FindType(componentNode->Name);
        TrackNode* propertyNode = new TrackNode(propertyName, propertyPath,
                                                TrackType::Property, targetMeta,
                                                componentNode, richAnim);
        LoadKeyFramesIntoTrack(propertyNode, propertyTrack);
      }

      r.PopFront();
    }
  }

  typedef Pair<String, TrackNode*> TrackPair;
  forRange(TrackPair& currTrackPair, objectTracks.All())
  {
    TrackNode* currTrack = currTrackPair.second;
    if(currTrack->Name == currTrack->Path)
    {
      richAnim->mRoot->AddChild(currTrack);
      continue;
    }

    char delimiter = cAnimationPathDelimiter;

    Pair<StringRange,StringRange> splitPath = SplitOnLast(currTrack->Path, delimiter);

    // Is there a parent part in the path?
    if(!splitPath.second.Empty())
    {
      String parentPath = splitPath.first;
      TrackNode* parent = objectTracks.FindValue(parentPath, nullptr);
      ErrorIf(parent == nullptr, "Object Track not found.");
      if(parent)
        parent->AddChild(currTrack);
    }

  }

  richAnim->UpdateDuration();

  return richAnim;
}

}//namespace Zero
