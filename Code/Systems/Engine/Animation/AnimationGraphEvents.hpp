// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(AnimationBlendEnded);
DeclareEvent(AnimationEnded);
DeclareEvent(AnimationLooped);
DeclareEvent(AnimationPostUpdate);
} // namespace Events

class Animation;
class AnimationNode;

class AnimationGraphEvent : public Event
{
public:
  ZilchDeclareType(AnimationGraphEvent, TypeCopyMode::ReferenceType);

  /// Only set when the animation node has a single animation.
  HandleOf<Animation> mAnimation;
  AnimationPlayMode::Enum mPlayMode;
  HandleOf<AnimationNode> mNode;
};

} // namespace Zero

#include "AnimationNode.inl"
