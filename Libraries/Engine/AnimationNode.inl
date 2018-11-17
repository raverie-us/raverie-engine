///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//------------------------------------------------------------------- Dual Blend
//******************************************************************************
namespace Zero
{

//******************************************************************************
template <typename DerivedType>
ZilchDefineType(DualBlend<DerivedType>, builder, type)
{
  ZilchBindGetterSetterProperty(From);
  ZilchBindGetterSetterProperty(To);
}

//******************************************************************************
template <typename DerivedType>
DualBlend<DerivedType>::DualBlend()
{
  mDuration = 1.0f;
}

//******************************************************************************
template <typename DerivedType>
DualBlend<DerivedType>::~DualBlend()
{

}

//******************************************************************************
template <typename DerivedType>
void DualBlend<DerivedType>::ReLinkAnimations()
{
  if(AnimationNode* a = mA)
    a->ReLinkAnimations();
  if(AnimationNode* b = mB)
    b->ReLinkAnimations();
}

//******************************************************************************
template <typename DerivedType>
AnimationNode* DualBlend<DerivedType>::Clone()
{
  DerivedType* clone = new DerivedType();
  clone->mDuration = mDuration;
  clone->mTime = mTime;
  clone->mA = mA->Clone();
  clone->mB = mB->Clone();
  return clone;
}

//******************************************************************************
template <typename DerivedType>
AnimationNode* DualBlend<DerivedType>::CollapseToA(AnimationGraph* animGraph, 
                                                uint frameId,
                                                EventList eventsToSend)
{
  AnimationGraphEvent* e = new AnimationGraphEvent();
  e->mPlayMode = AnimationPlayMode::PlayOnce;
  e->mNode = this;
  e->EventId = Events::AnimationBlendEnded;
  eventsToSend.PushBack(e);

  AnimationNode* r = mA->Update(animGraph, 0, frameId, eventsToSend);
  return r;
}

//******************************************************************************
template <typename DerivedType>
AnimationNode* DualBlend<DerivedType>::CollapseToB(AnimationGraph* animGraph, 
                                                   uint frameId,
                                                   EventList eventsToSend)
{
  AnimationGraphEvent* e = new AnimationGraphEvent();
  e->mPlayMode = AnimationPlayMode::PlayOnce;
  e->mNode = this;
  e->EventId = Events::AnimationBlendEnded;
  eventsToSend.PushBack(e);

  AnimationNode* r = mB->Update(animGraph, 0, frameId, eventsToSend);
  return r;
}

//******************************************************************************
template <typename DerivedType>
bool DualBlend<DerivedType>::IsPlayingInNode(StringParam animName)
{
  return mA->IsPlayingInNode(animName) || mB->IsPlayingInNode(animName);
}

//******************************************************************************
template <typename DerivedType>
void DualBlend<DerivedType>::SetFrom(AnimationNode* node)
{
  if(node == this)
  {
    DoNotifyException("Cannot attach node to itself", "This would cause an infinite loop.");
    return;
  }
  mA = node;
}

//******************************************************************************
template <typename DerivedType>
AnimationNode* DualBlend<DerivedType>::GetFrom()
{
  return mA;
}

//******************************************************************************
template <typename DerivedType>
void DualBlend<DerivedType>::SetTo(AnimationNode* node)
{
  if(node == this)
  {
    DoNotifyException("Cannot attach node to itself", "This would cause an infinite loop.");
    return;
  }
  mB = node;
}

//******************************************************************************
template <typename DerivedType>
AnimationNode* DualBlend<DerivedType>::GetTo()
{
  return mB;
}

}//namespace Zero
