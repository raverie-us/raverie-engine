///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const uint cInvalidFrameId = uint(-1);

Quat Slerp2(const Quat& q1, const Quat& q2, float param);

void PrintTabs(uint tabs)
{
  for(uint i = 0; i < tabs; ++i)
    DebugPrint("|   ");
}

void LerpFrame(AnimationFrame& a, AnimationFrame& b, float t, 
               AnimationFrame& result)
{
  uint numberOfTracks = a.Tracks.Size();
  result.Tracks.Resize(numberOfTracks);

  for(uint i=0;i<numberOfTracks;++i)
  {
    bool aActive = a.Tracks[i].Active;
    bool bActive = b.Tracks[i].Active;
    Any& valA = a.Tracks[i].Value;
    Any& valB = b.Tracks[i].Value;
    Any& dest = result.Tracks[i].Value;
    result.Tracks[i].Active = true;
    if(aActive & bActive)
    {
      if(valA.StoredType == ZilchTypeId(Vec3))
        dest = Math::Lerp(valA.Get<Vec3>(), valB.Get<Vec3>(), t);
      else if (valA.StoredType == ZilchTypeId(Quat))
        dest = Slerp2(valA.Get<Quat>(), valB.Get<Quat>(), t);
      else
        dest = valA;
    }
    else if(aActive)
    {
      dest = valA;
    }
    else
    {
      dest = valB;
    }
  }
}

//--------------------------------------------------------------- Animation Node
//******************************************************************************
AnimationNode::AnimationNode()
{
  mTime = 0.0f;
  mDuration = 0.0f;
  mTimeScale = 1.0f;
  mCollapseToPose = false;
  mCollapseToPoseOnFinish = true;
  mUpdatedFrameId = cInvalidFrameId;
}

//******************************************************************************
ZilchDefineType(AnimationNode, builder, type)
{
  ZeroBindDocumented();
  ZilchBindMethod(Clone);
  ZilchBindMethod(IsActive);
  ZilchBindMethod(PrintNode);
  ZilchBindMethod(CollapseToPose);
  ZilchBindMethod(SetNormalizedTime);
  ZilchBindMethod(GetNormalizedTime);
  ZilchBindFieldProperty(mCollapseToPoseOnFinish);
  ZilchBindFieldProperty(mPaused);
  ZilchBindGetterSetterProperty(Time);
  ZilchBindGetterSetterProperty(Duration);
  ZilchBindFieldProperty(mTimeScale);
}

//******************************************************************************
String AnimationNode::GetDisplayName()
{
  return ZilchVirtualTypeId(this)->Name;
}

//******************************************************************************
void AnimationNode::CollapseToPose()
{
  mCollapseToPose = true;
}

//******************************************************************************
void AnimationNode::SetDuration(float duration)
{
  mDuration = duration;
}

//******************************************************************************
float AnimationNode::GetDuration()
{
  return mDuration;
}

//******************************************************************************
void AnimationNode::SetTime(float time)
{
  mTime = Math::Clamp(time, 0.0f, mDuration);
}

//******************************************************************************
float AnimationNode::GetTime()
{
  return mTime;
}

//******************************************************************************
void AnimationNode::SetNormalizedTime(float normalizedTime)
{
  normalizedTime = Math::Clamp(normalizedTime, 0.0f, 1.0f);
  mTime = normalizedTime * mDuration;
}

//******************************************************************************
float AnimationNode::GetNormalizedTime()
{
  if(mDuration == 0.0f)
    return 0.0f;
  return mTime / mDuration;
}

//******************************************************************************
bool AnimationNode::HasUpdatedThisFrame(uint frameId)
{
  return mUpdatedFrameId == frameId;
}

//******************************************************************************
bool AnimationNode::HasUpdatedAtLeastOnce()
{
  return mUpdatedFrameId != cInvalidFrameId;
}

//-------------------------------------------------------------------- Pose Node
ZilchDefineType(PoseNode, builder, type)
{
}

//******************************************************************************
PoseNode::PoseNode(AnimationFrame& pose)
{
  mFrameData.Tracks.Assign(pose.Tracks.All());
}

//******************************************************************************
AnimationNode* PoseNode::Update(AnimationGraph* animGraph, float dt,
                                uint frameId, EventList eventsToSend)
{
  return this;
}

//******************************************************************************
void PoseNode::PrintNode(uint tabs)
{
  PrintTabs(tabs);
  DebugPrint("Pose");
}

//-------------------------------------------------------------- Basic Animation
ZilchDefineType(BasicAnimation, builder, type)
{
  ZilchBindGetterSetterProperty(Animation)->Add(new EditorResource());
  ZilchBindFieldProperty(mPlayMode);
}

//******************************************************************************
BasicAnimation::BasicAnimation()
{
  mDirection = 1.0f;
  mPlayMode = AnimationPlayMode::PlayOnce;
  mLoopCount = 0;
}

//******************************************************************************
BasicAnimation::BasicAnimation(AnimationGraph* animGraph)
{
  mDirection = 1.0f;
  mPlayMode = AnimationPlayMode::PlayOnce;
  mLoopCount = 0;
  mAnimGraph = animGraph;
}

//******************************************************************************
BasicAnimation::BasicAnimation(AnimationGraph* animGraph, Animation* animation, 
                               float t, AnimationPlayMode::Enum playMode)
{
  mDirection = 1.0f;
  mAnimation = animation;
  mTime = t;
  mPlayMode = playMode;
  mLoopCount = 0;
  mAnimGraph = animGraph;
  SetAnimation(animation);
}

//******************************************************************************
void BasicAnimation::ReLinkAnimations()
{
  // Re-set the animation
  SetAnimation(mAnimation);
}

//******************************************************************************
AnimationNode* BasicAnimation::Update(AnimationGraph* animGraph, float dt,
                                      uint frameId, EventList eventsToSend)
{
  // Return early if we've already been updated
  if(HasUpdatedThisFrame(frameId))
    return this;

  if(mCollapseToPose)
  {
    // If we haven't been updated yet, we need to pull the frame data from
    // the animation. Otherwise, we're just using last frames data
    if(HasUpdatedAtLeastOnce())
      UpdateFrame(animGraph);
    return new PoseNode(mFrameData);
  }

  // Update the frame id
  mUpdatedFrameId = frameId;

  Animation* animation = mAnimation;
  if(animation == nullptr)
    return this;

  mTime += dt * animGraph->GetTimeScale() * mDirection * mTimeScale;

  while((mTime > mDuration && mDuration > 0.0f) || mTime < 0.0f)
  {
    AnimationGraphEvent e;
    e.mAnimation = mAnimation;
    e.mPlayMode = mPlayMode;
    e.mNode = this;

    // If we're playing once, we're done and can delete ourselves
    if(mPlayMode == AnimationPlayMode::PlayOnce)
    {
      // Queue an animation ended event
      e.EventId = Events::AnimationEnded;
      eventsToSend.PushBack(new AnimationGraphEvent(e));

      if (mCollapseToPoseOnFinish)
      {
        // Clamp the time before updating the final frame
        mTime = Math::Clamp(mTime, 0.0f, mDuration);

        // Update the frame before collapsing
        UpdateFrame(animGraph);

        return new PoseNode(mFrameData);
      }
      else
        return nullptr;
    }
    else
    {
      if(mPlayMode == AnimationPlayMode::Pingpong)
      {
        // Don't go outside the range of the animation
        mTime = Math::Clamp(mTime, 0.0f, mDuration);

        // Flip the play direction
        mDirection *= -1.0f;
      }
      else
      {
        // Send animation looped event
        if(mTime > mDuration)
          mTime -= mDuration;
        else
          mTime += mDuration;
      }

      // Queue a loop event
      e.EventId = Events::AnimationLooped;
      eventsToSend.PushBack(new AnimationGraphEvent(e));
    }
  }

  // Update the frame with the animations data
  UpdateFrame(animGraph);

  return this;
}

//******************************************************************************
void BasicAnimation::UpdateFrame(AnimationGraph* animGraph)
{
  TrackParams params;
  params.Direction = TrackParams::Forward;
  params.Type = TrackParams::Game;
  params.Time = mTime;

  mFrameData.Tracks.Resize(animGraph->mBlendTracks.Size());
  mAnimation->UpdateFrame(mPlayData, params, mFrameData);
}

//******************************************************************************
AnimationNode* BasicAnimation::Clone()
{
  BasicAnimation* clone = new BasicAnimation(mAnimGraph);
  clone->mDuration = mDuration;
  clone->mTime = mTime;
  clone->mAnimation = mAnimation;
  clone->mPlayData = mPlayData;
  clone->mDirection = mDirection;
  return clone;
}

//******************************************************************************
bool BasicAnimation::IsPlayingInNode(StringParam animName)
{
  return animName == mAnimation->Name;
}

//******************************************************************************
void BasicAnimation::PrintNode(uint tabs)
{
  PrintTabs(tabs);
  String animName = mAnimation->Name;
  DebugPrint("BasicAnimation: %s, at: %.2gs of %.2gs\n", 
              animName.c_str(), mTime, mDuration);
}

//******************************************************************************
String BasicAnimation::GetDisplayName()
{
  if(Animation* animation = mAnimation)
    return animation->Name;
  return AnimationNode::GetDisplayName();
}

//******************************************************************************
Animation* BasicAnimation::GetAnimation()
{
  return mAnimation;
}

//******************************************************************************
void BasicAnimation::SetAnimation(Animation* animation)
{
  mAnimation = animation;

  mPlayData.Clear();
  mDuration = 0.0f;
  if(animation)
  {
    mDuration = animation->mDuration;
    if(AnimationGraph* animGraph = mAnimGraph)
      animGraph->SetUpPlayData(mAnimation, mPlayData);
  }
}

//******************************************************************************
AnimationNode* BuildBasic(AnimationGraph* animGraph, Animation* animation, float t, 
                          AnimationPlayMode::Enum playMode)
{
  return new BasicAnimation(animGraph, animation, t, playMode);
}

//----------------------------------------------------------------- Direct Blend
ZilchDefineType(DirectBlend, builder, type)
{
}

//******************************************************************************
DirectBlend::DirectBlend()
{

}

//******************************************************************************
String DirectBlend::GetName()
{
  return String("DirectBlend");
}

//******************************************************************************
AnimationNode* DirectBlend::Update(AnimationGraph* animGraph, float dt,
                                   uint frameId, EventList eventsToSend)
{
  // Return early if we've already been updated
  if(HasUpdatedThisFrame(frameId))
  {
    if(AnimationNode* lastReturned = mLastReturned)
      return lastReturned;
    return this;
  }

  // Update the frame id
  mUpdatedFrameId = frameId;

  if(mCollapseToPose)
  {
    // Use the last frames data for the pose. If we haven't been updated yet,
    // we need to update with a dt of 0.0, then create the pose node
    if(HasUpdatedAtLeastOnce())
      return new PoseNode(mFrameData);

    // If we're collapsing to pose, we want to pull all animation data without
    // stepping forward in time
    dt = 0.0f;
  }

  mTime += dt * animGraph->GetTimeScale() * mTimeScale;

  // If the blend is done, return the right branch
  if(mTime > mDuration)
  {
    mLastReturned = mB;
    return CollapseToB(animGraph, frameId, eventsToSend);
  }

  float t = mTime/mDuration;

  // Update the left branch
  mA = mA->Update(animGraph, 0, frameId, eventsToSend);
  if(mA.IsNull())
  {
    mLastReturned = mB;
    return CollapseToB(animGraph, frameId, eventsToSend);
  }

  // Update the right branch
  mB = mB->Update(animGraph, 0, frameId, eventsToSend);
  if(mB.IsNull())
  {
    mLastReturned = mA;
    return CollapseToA(animGraph, frameId, eventsToSend);
  }

  // Interpolate between the two branches
  LerpFrame(mA->mFrameData, mB->mFrameData, t, mFrameData);

  // Now that we've updated our frame data, we can create the pose node
  if(mCollapseToPose)
    return new PoseNode(mFrameData);

  return this;
}

//******************************************************************************
void DirectBlend::PrintNode(uint tabs)
{
  PrintTabs(tabs);
  DebugPrint("Direct-Blend between:\n");
  mA->PrintNode(tabs + 1);
  mB->PrintNode(tabs + 1);
}

//******************************************************************************
AnimationNode* BuildDirectBlend(AnimationGraph* t, AnimationNode* a, AnimationNode* b,
                                float transitionTime)
{
  DirectBlend* direct = new DirectBlend();
  direct->mA = a;
  direct->mB = b;
  direct->SetTime(0);
  direct->SetDuration(transitionTime);
  return direct;
}

//------------------------------------------------------------------ Cross Blend
ZilchDefineType(CrossBlend, builder, type)
{
  ZilchBindFieldProperty(mTimeScaleFrom);
  ZilchBindFieldProperty(mTimeScaleTo);
  ZilchBindFieldProperty(mType);
  ZilchBindFieldProperty(mMode);
  ZilchBindMethod(SyncCadence);
  ZilchBindMethod(SetNormalizedTimeScale);
}

//******************************************************************************
CrossBlend::CrossBlend()
{
  mTimeScaleFrom = 1.0f;
  mTimeScaleTo = 1.0f;
  mType = AnimationBlendType::Standard;
  mMode = AnimationBlendMode::Auto;
}

//******************************************************************************
String CrossBlend::GetName()
{
  return String("CrossBlend");
}

//******************************************************************************
AnimationNode* CrossBlend::Update(AnimationGraph* animGraph, float dt,
                                  uint frameId, EventList eventsToSend)
{
  // Return early if we've already been updated
  if(HasUpdatedThisFrame(frameId))
  {
    if(AnimationNode* lastReturned = mLastReturned)
      return lastReturned;
    return this;
  }

  if(mCollapseToPose)
  {
    // Use the last frames data for the pose. If we haven't been updated yet,
    // we need to update with a dt of 0.0, then create the pose node
    if(HasUpdatedAtLeastOnce())
      return new PoseNode(mFrameData);

    // If we're collapsing to pose, we want to pull all animation data without
    // stepping forward in time
    dt = 0.0f;
  }

  // Update the frame id
  mUpdatedFrameId = frameId;

  // Step forward if we're in auto mode
  if(mMode == AnimationBlendMode::Auto)
  {
    float timeScale = animGraph->GetTimeScale() * mTimeScale;
    mTime += dt * timeScale;
  }

  // The normalized time for this blend
  float blendT = 0.0f;
  if(mDuration != 0.0f)
    blendT = mTime / mDuration;

  // Cross blend will also cross blend the animation speed
  if(mMode == AnimationBlendMode::Auto)
  {
    // If the blend is done, return the right branch
    if(mTime > mDuration)
    {
      mLastReturned = mB;
      return CollapseToB(animGraph, frameId, eventsToSend);
    }
  }

  if(mType == AnimationBlendType::Normalized)
  {
    float rateA = 0.0f;
    if(mA->GetDuration() > 0.0f)
      rateA = 1.0f / mA->GetDuration();

    float rateB = 0.0f;
    if(mB->GetDuration() > 0.0f)
      rateB = 1.0f / mB->GetDuration();

    float percentagePassed = Math::Lerp(rateA, rateB, blendT);

    mTimeScaleFrom = percentagePassed * mA->GetDuration();
    mTimeScaleTo = percentagePassed * mB->GetDuration();
  }

  // Update the left branch
  mA = mA->Update(animGraph, dt * mTimeScaleFrom, frameId, eventsToSend);
  if(!mA)
  {
    mLastReturned = mB;
    return CollapseToB(animGraph, frameId, eventsToSend);
  }

  // Update the right branch
  mB = mB->Update(animGraph, dt * mTimeScaleTo, frameId, eventsToSend);
  if(!mB)
  {
    mLastReturned = mA;
    return CollapseToA(animGraph, frameId, eventsToSend);
  }

  // Interpolate between the two branches
  LerpFrame(mA->mFrameData, mB->mFrameData, blendT, mFrameData);

  // Now that we've updated our frame data, we can create the pose node
  if(mCollapseToPose)
    return new PoseNode(mFrameData);

  return this;
}

//******************************************************************************
void CrossBlend::PrintNode(uint tabs)
{
  PrintTabs(tabs);
  DebugPrint("Cross-Blend between:\n");
  mA->PrintNode(tabs + 1);
  mB->PrintNode(tabs + 1);
}

//******************************************************************************
void CrossBlend::SyncCadence()
{
  float normalizedT = 0.0f;
  if(mA->GetDuration() > 0.0f)
    normalizedT = mA->GetTime() / mA->GetDuration();
  mB->SetTime(normalizedT * mB->GetDuration());
}

//******************************************************************************
void CrossBlend::SetNormalizedTimeScale(float min, float max, float current)
{
  mType = AnimationBlendType::Standard;

  float rateA = 0.0f;
  if(mA->GetDuration() > 0.0f)
    rateA = 1.0f / mA->GetDuration();

  float rateB = 0.0f;
  if(mB->GetDuration() > 0.0f)
    rateB = 1.0f / mB->GetDuration();

  float blendTime = (current - min) / (max - min);

  if (blendTime < 0.0f)
  {
    mTimeScaleFrom = current / min;
    mTimeScaleTo = mB->GetDuration() * mTimeScaleFrom * rateA;
  }
  else if (blendTime > 1.0f)
  {
    mTimeScaleTo = current / max;
    mTimeScaleFrom = mA->GetDuration() * mTimeScaleTo * rateB;
  }
  else
  {
    float rate = Math::Lerp(rateA, rateB, blendTime);
    mTimeScaleFrom = mA->GetDuration() * rate;
    mTimeScaleTo = mB->GetDuration() * rate;
  }

  SetNormalizedTime(Math::Clamp(blendTime, 0.0f, 1.0f));
}

//******************************************************************************
AnimationNode* BuildCrossBlend(AnimationGraph* t, AnimationNode* a, AnimationNode* b,
                               float transitionTime)
{
  CrossBlend* blend = new CrossBlend();

  blend->mA = a;
  blend->mB = b;

  blend->SyncCadence();

  blend->SetTime(0);
  blend->SetDuration(transitionTime);

  return blend;
}

//--------------------------------------------------------------- Selective Node
ZilchDefineType(SelectiveNode, builder, type)
{
  ZilchBindGetterSetterProperty(Root);
}

//******************************************************************************
SelectiveNode::SelectiveNode()
{

}

//******************************************************************************
String SelectiveNode::GetName()
{
  return String("SelectiveNode");
}

//******************************************************************************
AnimationNode* SelectiveNode::Update(AnimationGraph* animGraph, float dt,
                                     uint frameId, EventList eventsToSend)
{
  // Return early if we've already been updated
  if(HasUpdatedThisFrame(frameId))
  {
    if(AnimationNode* lastReturned = mLastReturned)
      return lastReturned;
    return this;
  }

  // Update the frame id
  mUpdatedFrameId = frameId;

  if(mCollapseToPose)
  {
    // Use the last frames data for the pose. If we haven't been updated yet,
    // we need to update with a dt of 0.0, then create the pose node
    if(HasUpdatedAtLeastOnce())
      return new PoseNode(mFrameData);

    // If we're collapsing to pose, we want to pull all animation data without
    // stepping forward in time
    dt = 0.0f;
  }

  // Update the left branch
  if(mA)
    mA = mA->Update(animGraph, dt * mTimeScale, frameId, eventsToSend);

  // Update the right branch
  mB = mB->Update(animGraph, dt * mTimeScale, frameId, eventsToSend);
  if(!mB)
  {
    mLastReturned = mA;
    return CollapseToA(animGraph, frameId, eventsToSend);
  }

  // Overwrite the 'A' track values with 'B' track values
  AnimationFrame& frameA = mA->mFrameData;
  AnimationFrame& frameB = mB->mFrameData;

  uint trackCount = frameA.Tracks.Size();
  mFrameData.Tracks.Resize(trackCount);
  for(uint i = 0; i < trackCount; ++i)
  {
    Any& dest = mFrameData.Tracks[i].Value;
    mFrameData.Tracks[i].Active = true;

    if(mSelectiveBones.FindValue((int)i, uint(-1)) != uint(-1))
    {
      Any& valB = frameB.Tracks[i].Value;
      dest = valB;
    }
    else if(mA)
    {
      Any& valA = frameA.Tracks[i].Value;
      dest = valA;
    }
  }

  // Now that we've updated our frame data, we can create the pose node
  if(mCollapseToPose)
    return new PoseNode(mFrameData);

  return this;
}

//******************************************************************************
AnimationNode* SelectiveNode::Clone()
{
  SelectiveNode* clone = new SelectiveNode();
  clone->mDuration = mDuration;
  clone->mTime = mTime;
  clone->mA = mA->Clone();
  clone->mB = mB->Clone();
  clone->mSelectiveBones = mSelectiveBones;
  return clone;
}

//******************************************************************************
void SelectiveNode::PrintNode(uint tabs)
{
  PrintTabs(tabs);
  DebugPrint("Isolated-Blend between:\n");
  mA->PrintNode(tabs + 1);
  mB->PrintNode(tabs + 1);
}

//******************************************************************************
void GetChildIndices(Cog* object, AnimationGraph* t, HashSet<uint>& indices)
{
  // Walk through the blend tracks and find anything with this object
  BlendTracks::range r = t->mBlendTracks.All();
  for(; !r.Empty(); r.PopFront())
  {
    BlendTrack* track = r.Front().second;
    Transform* currObject = track->Object.Get<Transform*>();
    if(currObject == nullptr)
      continue;
    if(currObject->GetOwner() == object)
      indices.Insert(track->Index);
  }

  // Recursively call each child of the object
  Hierarchy* hierarchy = object->has(Hierarchy);
  if(hierarchy)
  {
    HierarchyList::range range = hierarchy->GetChildren();
    for(; !range.Empty(); range.PopFront())
      GetChildIndices(&range.Front(), t, indices);
  }
}

//******************************************************************************
AnimationGraph* GetAnimationGraph(Cog* object)
{
  if(object == nullptr)
    return nullptr;
  if(AnimationGraph* animGraph = object->has(AnimationGraph))
    return animGraph;
  return GetAnimationGraph(object->GetParent());
}

//******************************************************************************
void SelectiveNode::SetRoot(Cog* root)
{
  mRoot = root;
  AnimationGraph* animGraph = GetAnimationGraph(root);
  GetChildIndices(root, animGraph, mSelectiveBones);
}

//******************************************************************************
Cog* SelectiveNode::GetRoot()
{
  return mRoot;
}

//******************************************************************************
AnimationNode* BuildSelectiveNode(AnimationGraph* t, AnimationNode* a, 
                                  AnimationNode* b, Cog* rootBone)
{
  SelectiveNode* selective = new SelectiveNode();

  selective->mA = a;
  selective->mB = b;
  selective->SetRoot(rootBone);

  return selective;
}

//------------------------------------------------------------------- Chain Node
ZilchDefineType(ChainNode, builder, type)
{
}

//******************************************************************************
ChainNode::ChainNode()
{

}

//******************************************************************************
String ChainNode::GetName()
{
  return String("ChainNode");
}

//******************************************************************************
AnimationNode* ChainNode::Update(AnimationGraph* animGraph, float dt,
                                 uint frameId, EventList eventsToSend)
{
  // Return early if we've already been updated
  if(HasUpdatedThisFrame(frameId))
  {
    if(AnimationNode* lastReturned = mLastReturned)
      return lastReturned;
    return this;
  }

  if(mCollapseToPose)
  {
    // Use the last frames data for the pose. If we haven't been updated yet,
    // we need to update with a dt of 0.0, then create the pose node
    if(HasUpdatedAtLeastOnce())
      return new PoseNode(mFrameData);

    // If we're collapsing to pose, we want to pull all animation data without
    // stepping forward in time
    dt = 0.0f;
  }

  // Update the frame id
  mUpdatedFrameId = frameId;

  if (mA == nullptr)
    return this;

  // Keep around a reference so that it cannot be deleted in the update
  HandleOf<AnimationNode> tempRefA = mA;
  mA = mA->Update(animGraph, dt, frameId, eventsToSend);
  if(!mA)
  {
    mLastReturned = mB;
    return CollapseToB(animGraph, frameId, eventsToSend);
  }

  // Copy over tracks from the current child. This should be optimized
  mFrameData.Tracks.Assign(mA->mFrameData.Tracks.All());

  // Now that we've updated our frame data, we can create the pose node
  if(mCollapseToPose)
    return new PoseNode(mFrameData);

  return this;
}

//******************************************************************************
bool ChainNode::IsPlayingInNode(StringParam animName)
{
  return mA->IsPlayingInNode(animName) || mB->IsPlayingInNode(animName);
}

//******************************************************************************
void ChainNode::PrintNode(uint tabs)
{
  PrintTabs(tabs);
  DebugPrint("Chain A then B:\n");
  mA->PrintNode(tabs + 1);
  mB->PrintNode(tabs + 2);
}

//******************************************************************************
AnimationNode* BuildChainNode(AnimationGraph* t, AnimationNode* a, AnimationNode* b)
{
  ChainNode* chain = new ChainNode();
  chain->mA = a;
  chain->mB = b;
  return chain;
}

}//namespace Zero
