// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
// Focus on Object
DeclareEvent(UiFocusGainedPreview);
DeclareEvent(UiFocusGained);
DeclareEvent(UiFocusLost);

DeclareEvent(UiFocusLostHierarchy);
DeclareEvent(UiFocusGainedHierarchy);

// Any focus operation should be canceled. This happens when
DeclareEvent(UiFocusReset);

} // namespace Events

class UiFocusEvent : public Event
{
public:
  /// Meta Initialization.
  RaverieDeclareType(UiFocusEvent, TypeCopyMode::ReferenceType);

  /// Constructor.
  UiFocusEvent(UiWidget* focusGained, UiWidget* focusLost);

  /// The object that is gaining focus.
  HandleOf<UiWidget> mReceivedFocus;

  /// The object that has lost focus, or will lose focus (in the case of
  /// preview).
  HandleOf<UiWidget> mLostFocus;

  /// On UiFocusGainedPreview, allows you to cancel the focus gain.
  bool mCancelFocus;
};

} // namespace Raverie
