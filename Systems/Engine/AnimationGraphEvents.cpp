///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(AnimationBlendEnded);
  DefineEvent(AnimationEnded);
  DefineEvent(AnimationLooped);
  DefineEvent(AnimationPostUpdate);
}//namespace Events

 //-------------------------------------------------------- Animation Graph Event
ZilchDefineType(AnimationGraphEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldGetter(mAnimation);
  ZilchBindFieldGetter(mNode);
  ZilchBindFieldGetter(mPlayMode);
}

}//namespace Zero
