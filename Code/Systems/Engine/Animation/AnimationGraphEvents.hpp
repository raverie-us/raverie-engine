// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
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
  RaverieDeclareType(AnimationGraphEvent, TypeCopyMode::ReferenceType);

  /// Only set when the animation node has a single animation.
  HandleOf<Animation> mAnimation;
  AnimationPlayMode::Enum mPlayMode;
  HandleOf<AnimationNode> mNode;
};

} // namespace Raverie

#include "AnimationNode.inl"
