// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(UiFocusGainedPreview);
DefineEvent(UiFocusLost);
DefineEvent(UiFocusGained);
DefineEvent(UiFocusLostHierarchy);
DefineEvent(UiFocusGainedHierarchy);
DefineEvent(UiFocusReset);

} // namespace Events

RaverieDefineType(UiFocusEvent, builder, type)
{
  RaverieBindDocumented();
  RaverieBindFieldGetter(mReceivedFocus);
  RaverieBindFieldGetter(mLostFocus);
}

UiFocusEvent::UiFocusEvent(UiWidget* focusGained, UiWidget* focusLost) :
    mReceivedFocus(focusGained),
    mLostFocus(focusLost)
{
}

} // namespace Raverie
