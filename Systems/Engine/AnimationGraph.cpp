///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationGraph.cpp
/// Implementation of the AnimationGraph component class.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------- Animation Graph
AnimationGraph::DebugPreviewFunction AnimationGraph::mOnPreviewPressed = NULL;
AnimationGraph::DebugPreviewFunction AnimationGraph::mOnGraphCreated = NULL;

//******************************************************************************
ZilchDefineType(AnimationGraph, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindGetterSetter(ActiveNode);
  ZilchBindMethod(IsPlayingInGraph);
  ZilchBindMethod(PrintGraph);

  ZilchBindGetterSetterProperty(Active);
  ZilchBindFieldProperty(mTimeScale);

  ZeroBindEvent(Events::AnimationBlendEnded, AnimationGraphEvent);
  ZeroBindEvent(Events::AnimationEnded, AnimationGraphEvent);
  ZeroBindEvent(Events::AnimationLooped, AnimationGraphEvent);
  ZeroBindEvent(Events::AnimationPostUpdate, Event);

  // Node creation functions
  ZilchBindMethod(CreateBasicNode);
  ZilchBindMethod(CreateDirectBlendNode);
  ZilchBindMethod(CreateCrossBlendNode);
  ZilchBindMethod(CreateSelectiveNode);
  ZilchBindMethod(CreateChainNode);

  ZeroBindTag(Tags::Core);

  //ZilchBindMethodProperty(PreviewGraph);
}

//******************************************************************************
AnimationGraph::AnimationGraph()
{
  mFrameId = 0;
}

//******************************************************************************
AnimationGraph::~AnimationGraph()
{
  DeleteObjectsInContainer(mBlendTracks);
}

//******************************************************************************
void AnimationGraph::Serialize(Serializer& stream)
{
  SerializeName(mActive);
  // Loaded for old projects
  SerializeEnumName(AnimationPlayMode, mPlayMode);
  SerializeName(mTimeScale);
  // Loaded for old projects
  SerializeResourceName(mAnimation, AnimationManager);
  SerializeNameDefault(mDebugPreviewId, (u64)0);
}

//******************************************************************************
void AnimationGraph::OnAllObjectsCreated(CogInitializer& initializer)
{
  if(Animation* animation = mAnimation)
  {
    if(animation != AnimationManager::GetDefault())
    {
      // Only add the component if it doesn't already exist
      if(GetOwner()->has(SimpleAnimation) == NULL)
      {
        SimpleAnimation* autoPlay = new SimpleAnimation();
        GetOwner()->AddComponent(autoPlay);
        autoPlay->SetPlayMode(mPlayMode);
        autoPlay->SetAnimation(animation);
        mAnimation = NULL;

        // The space has changed
        GetSpace()->MarkModified();
      }
    }
  }
}

//******************************************************************************
void AnimationGraph::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(initializer.mSpace, Events::LogicUpdate, OnUpdate);

  if(mOnGraphCreated && !GetSpace()->IsEditorMode())
    mOnGraphCreated(this);

  ConnectThisTo(MetaDatabase::GetInstance(), Events::MetaModified, OnMetaModified);
}

//******************************************************************************
void AnimationGraph::SetDefaults()
{
  mActive = true;
  mTimeScale = 1.0f;
  mPlayMode = AnimationPlayMode::PlayOnce;
  mAnimation = AnimationManager::GetDefault();
}

//******************************************************************************
void AnimationGraph::OnUpdate(UpdateEvent* e)
{
  // Do nothing if we aren't active
  if(!mActive)
    return;

  if(mActiveNode)
  {
    // Static so we can re-use memory and avoid extra allocations
    static Array<AnimationGraphEvent*> eventsToSend;
    eventsToSend.Clear();

    // Update the root node
    mActiveNode = mActiveNode->Update(this, e->Dt, mFrameId++, eventsToSend);

    // Apply the frame if we're given anything back
    if(mActiveNode)
      ApplyFrame(mActiveNode->mFrameData);

    // Dispatch all events from the animation graph
    forRange(AnimationGraphEvent* eventToSend, eventsToSend.All())
    {
      GetOwner()->DispatchEvent(eventToSend->EventId, eventToSend);
      delete eventToSend;
    }

    // Send the post animation event
    Event eventToSend;
    GetOwner()->DispatchEvent(Events::AnimationPostUpdate, &eventToSend);

    return;
  }
}

//******************************************************************************
void AnimationGraph::ApplyFrame(AnimationFrame& frame)
{
  forRange(BlendTrack* blendTrack, mBlendTracks.Values())
  {
    ErrorIf(blendTrack->Index  >= frame.Tracks.Size(), "Frame error");
    if(blendTrack->Index  < frame.Tracks.Size())
    {
      AnimationFrameData& frameData = frame.Tracks[ blendTrack->Index ];
      if(frameData.Active)
      {
        Any& newValue = frameData.Value;
        if(!blendTrack->Object.IsNull())
          blendTrack->Property->SetValue(blendTrack->Object, newValue);
      }
    }
    else
    {
    }
  }
}

//******************************************************************************
void AnimationGraph::OnMetaModified(MetaTypeEvent* e)
{
  // The blend tracks store pointers to MetaProperties, and must be deleted
  DeleteObjectsInContainer(mBlendTracks);

  // Re-link all active animations
  if(AnimationNode* root = mActiveNode)
    root->ReLinkAnimations();
}

//******************************************************************************
void AnimationGraph::SetActive(bool value)
{
  mActive = value;
}

//******************************************************************************
void AnimationGraph::ForceUpdate()
{
  if(mActiveNode)
  {
    AnimationFrame frame;

    // Static so we can re-use memory and avoid extra allocations
    static Array<AnimationGraphEvent*> eventsToSend;
    eventsToSend.Clear();

    mActiveNode = mActiveNode->Update(this, 0.0f, mFrameId++, eventsToSend);
    if(mActiveNode)
      ApplyFrame(mActiveNode->mFrameData);
  }
}

//******************************************************************************
void AnimationGraph::SetTimeScale(float scale)
{
  ReturnIf(scale < 0.0f, , "TimeScale must be positive.");

  mTimeScale = scale;
}

//******************************************************************************
float AnimationGraph::GetTimeScale()
{
  return mTimeScale;
}

//******************************************************************************
void AnimationGraph::SetActiveNode(AnimationNode* node)
{
  mActiveNode = node;
}

//******************************************************************************
AnimationNode* AnimationGraph::GetActiveNode()
{
  return mActiveNode;
}

//******************************************************************************
// ExamplePath:  /Stomach/Chest/LArm
// RootPath:     /
Cog* ResolveObjectPath(Cog* object, StringRange data)
{
  // If it's the root, 
  if(data == "/")
    return object;

  StringTokenRange r(data.Data(), cAnimationPathDelimiter);
  String first = r.Front();
  Cog* newObject = object->FindChildByName(first);
  
  //If the first node was not found
  //search for the second node (issue with root objects)
  if(newObject==NULL)
  {
    r.PopFront();

    newObject = object->FindChildByName(r.Front());
    if(newObject==NULL)
      return NULL; 
  }

  r.PopFront();
  if(r.Empty())
    return newObject;
  else
    return ResolveObjectPath(newObject, r.Front());
}

//******************************************************************************
void AnimationGraph::SetUpPlayData(Animation* animation, PlayData& playData)
{
  playData.Clear();
  playData.Resize(animation->mNumberOfTracks);

  //Find all objects this track references
  forRange(ObjectTrack& track, animation->ObjectTracks.All())
  {
    ObjectTrackPlayData& objectData = playData[track.ObjectTrackId];
    Cog* currentObject = objectData.ObjectHandle;
    Cog* object = ResolveObjectPath(mOwner, track.FullPath.All());
    if(object)
    {
      objectData.ObjectHandle = object;

      //Clear All Per instance data
      //Find all objects for each sub track of each track
      objectData.mSubTrackPlayData.Clear();

      forRange(PropertyTrack& subTrack, track.PropertyTracks.All())
      {
        PropertyTrackPlayData& subTrackData = objectData.mSubTrackPlayData.PushBack();
        subTrackData.mComponent = NULL;
        subTrackData.mKeyframeIndex = 0;
        subTrack.LinkInstance(objectData.mSubTrackPlayData.Back(), mBlendTracks, track.FullPath, object);
      }

    }
    else
    {
      DebugPrint("Failed to find object in animation track. %s\n", track.FullPath.c_str());
    }
  }
}

//******************************************************************************
void AnimationGraph::PreviewGraph()
{
  if(mOnPreviewPressed)
    mOnPreviewPressed(this);
}

//******************************************************************************
bool AnimationGraph::IsPlayingInGraph(Animation* animation)
{
  if(mActiveNode)
    return mActiveNode->IsPlayingInNode(animation->Name);
  return false;
}

//******************************************************************************
void AnimationGraph::PrintGraph()
{
  if(mActiveNode)
    mActiveNode->PrintNode(0);
}

//******************************************************************************
BasicAnimation* AnimationGraph::CreateBasicNode(Animation* animation,
                                          AnimationPlayMode::Enum mode)
{
  if (animation == NULL)
    DoNotifyException("Null Animation", "Trying to create an animation node but the animation resource is null.");
  return new BasicAnimation(this, animation, 0.0f, mode);
}

//******************************************************************************
DirectBlend* AnimationGraph::CreateDirectBlendNode()
{
  return new DirectBlend();
}

//******************************************************************************
CrossBlend* AnimationGraph::CreateCrossBlendNode()
{
  return new CrossBlend();
}

//******************************************************************************
SelectiveNode* AnimationGraph::CreateSelectiveNode()
{
  return new SelectiveNode();
}

//******************************************************************************
ChainNode* AnimationGraph::CreateChainNode()
{
  return new ChainNode();
}

//------------------------------------------------------------- Simple Animation
ZilchDefineType(SimpleAnimation, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(AnimationGraph);
  ZilchBindFieldProperty(mPlayMode);
  ZilchBindGetterSetterProperty(Animation)->Add(new EditorResource());

  ZilchBindMethod(PlaySingle);
  ZilchBindMethod(DirectBlend);
  ZilchBindMethod(CrossBlend);
  ZilchBindMethod(PlayIsolatedAnimation);
  ZilchBindMethod(ChainAnimation);
}

//******************************************************************************
void SimpleAnimation::Serialize(Serializer& stream)
{
  SerializeResourceName(mAnimation, AnimationManager);
  SerializeEnumNameDefault(AnimationPlayMode, mPlayMode, AnimationPlayMode::Loop);
}

//******************************************************************************
void SimpleAnimation::Initialize(CogInitializer& initializer)
{
  mAnimGraph = GetOwner()->has(AnimationGraph);
  ReturnIf(mAnimGraph == NULL, , "Missing dependency.");

  // Play the animation
  if(mAnimation)
    PlaySingle(mAnimation, mPlayMode);
}

//******************************************************************************
Animation* SimpleAnimation::GetAnimation()
{
  return mAnimation;
}

//******************************************************************************
void SimpleAnimation::SetAnimation(Animation* animation)
{
  PlaySingle(animation, mPlayMode);
  mAnimation = animation;
}

//******************************************************************************
AnimationPlayMode::Enum SimpleAnimation::GetPlayMode()
{
  return mPlayMode;
}

//******************************************************************************
void SimpleAnimation::SetPlayMode(AnimationPlayMode::Enum mode)
{
  mPlayMode = mode;
}

//******************************************************************************
AnimationNode* SimpleAnimation::PlaySingle(Animation* animation,
                                       AnimationPlayMode::Enum playMode)
{
  ReturnIf(animation == NULL, NULL, "Invalid animation given.");
  AnimationNode* node = BuildBasic(mAnimGraph, animation, 0, playMode);
  mAnimGraph->SetActiveNode(node);
  return node;
}

//******************************************************************************
AnimationNode* SimpleAnimation::DirectBlend(Animation* animation, float transitionTime, AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if(activeNode && animation)
  {
    AnimationNode* end = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    activeNode = BuildDirectBlend(mAnimGraph, activeNode, end, transitionTime);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

//******************************************************************************
AnimationNode* SimpleAnimation::CrossBlend(Animation* animation, float transitionTime, AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if(activeNode && animation)
  {
    AnimationNode* end = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    activeNode = BuildCrossBlend(mAnimGraph, activeNode, end, transitionTime);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

//******************************************************************************
AnimationNode* SimpleAnimation::PlayIsolatedAnimation(Animation* animation, Cog* rootBone, AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if(activeNode && animation)
  {
    AnimationNode* isolated = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    AnimationNode* blendingTo = BuildDirectBlend(mAnimGraph, activeNode->Clone(), isolated, 0.15f);
    activeNode = BuildSelectiveNode(mAnimGraph, activeNode, blendingTo, rootBone);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

//******************************************************************************
AnimationNode* SimpleAnimation::ChainAnimation(Animation* animation, AnimationPlayMode::Enum playMode)
{
  AnimationNode* activeNode = mAnimGraph->GetActiveNode();

  if(activeNode && animation)
  {
    AnimationNode* b = BuildBasic(mAnimGraph, animation, 0.0f, playMode);
    activeNode = BuildChainNode(mAnimGraph, activeNode, b);
    mAnimGraph->SetActiveNode(activeNode);
    return activeNode;
  }

  return NULL;
}

}//namespace Zero
