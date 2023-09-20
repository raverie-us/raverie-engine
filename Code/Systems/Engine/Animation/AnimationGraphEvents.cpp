// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(AnimationBlendEnded);
DefineEvent(AnimationEnded);
DefineEvent(AnimationLooped);
DefineEvent(AnimationPostUpdate);
} // namespace Events

RaverieDefineType(AnimationGraphEvent, builder, type)
{
  RaverieBindDocumented();
  RaverieBindFieldGetter(mAnimation);
  RaverieBindFieldGetter(mNode);
  RaverieBindFieldGetter(mPlayMode);
}

} // namespace Raverie
