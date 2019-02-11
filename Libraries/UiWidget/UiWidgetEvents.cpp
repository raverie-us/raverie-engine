// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
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

ZilchDefineType(UiFocusEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldGetter(mReceivedFocus);
  ZilchBindFieldGetter(mLostFocus);
}

UiFocusEvent::UiFocusEvent(UiWidget* focusGained, UiWidget* focusLost) :
    mReceivedFocus(focusGained),
    mLostFocus(focusLost)
{
}

} // namespace Zero
