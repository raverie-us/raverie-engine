///////////////////////////////////////////////////////////////////////////////
///
/// \file WidgetEvents.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
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

}//namespace Events

//------------------------------------------------------------------ Focus Event
class UiFocusEvent : public Event
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  UiFocusEvent(Cog* focusGained, Cog* focusLost)
    : mReceivedFocus(focusGained), mLostFocus(focusLost)
  {
  }

  /// The object that is gaining focus.
  CogId mReceivedFocus;

  /// The object that has lost focus, or will lose focus (in the case of preview).
  CogId mLostFocus;

  /// On UiFocusGainedPreview, allows you to cancel the focus gain.
  bool mCancelFocus;
};

}//namespace Zero
