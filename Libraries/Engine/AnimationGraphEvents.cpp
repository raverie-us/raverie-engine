// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(AnimationBlendEnded);
DefineEvent(AnimationEnded);
DefineEvent(AnimationLooped);
DefineEvent(AnimationPostUpdate);
} // namespace Events

ZilchDefineType(AnimationGraphEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldGetter(mAnimation);
  ZilchBindFieldGetter(mNode);
  ZilchBindFieldGetter(mPlayMode);
}

} // namespace Zero
