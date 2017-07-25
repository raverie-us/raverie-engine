///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
ZilchDefineEvent(TranslationError);
}//namespace Events

 //-------------------------------------------------------------------TranslationErrorEvent
ZilchDefineType(TranslationErrorEvent, builder, type)
{
}

String TranslationErrorEvent::GetFormattedMessage(Zilch::MessageFormat::Enum format)
{
  return mLocation.GetFormattedStringWithMessage(format, mFullMessage);
}

//-------------------------------------------------------------------ShaderCompilationErrors
ShaderCompilationErrors::ShaderCompilationErrors()
{
  mErrorTriggered = false;
  mEmitMultipleErrors = false;
}

void ShaderCompilationErrors::SendTranslationError(Zilch::CodeLocation& location, StringParam message)
{
  SendTranslationError(location, message, message);
}

void ShaderCompilationErrors::SendTranslationError(Zilch::CodeLocation& location, StringParam shortMsg, StringParam fullMsg)
{
  // Check if this is the first error being sent and if not check if we send multiple errors
  if(mErrorTriggered && !mEmitMultipleErrors)
    return;

  mErrorTriggered = true;

  TranslationErrorEvent toSend;
  toSend.EventName = Events::TranslationError;
  toSend.mShortMessage = shortMsg;
  toSend.mFullMessage = fullMsg;
  toSend.mLocation = location;
  EventSend(this, toSend.EventName, &toSend);
}

void ShaderCompilationErrors::ListenForZilchErrors(Zilch::CompilationErrors& zilchErrors)
{
  EventConnect(&zilchErrors, Zilch::Events::CompilationError, &ShaderCompilationErrors::ForwardEvent, this);
}

void ShaderCompilationErrors::ForwardEvent(Zilch::ErrorEvent* e)
{
  mErrorTriggered = true;
  EventSend(this, e->EventName, e);
}

}//namespace Zero
