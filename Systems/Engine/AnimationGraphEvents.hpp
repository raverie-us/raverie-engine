///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DeclareEvent(AnimationBlendEnded);
  DeclareEvent(AnimationEnded);
  DeclareEvent(AnimationLooped);
  DeclareEvent(AnimationPostUpdate);
}

class Animation;
class AnimationNode;

//-------------------------------------------------------- Animation Graph Event
class AnimationGraphEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Only set when the animation node has a single animation.
  HandleOf<Animation> mAnimation;
  AnimationPlayMode::Enum mPlayMode;
  HandleOf<AnimationNode> mNode;
};

}// namespace Zero
