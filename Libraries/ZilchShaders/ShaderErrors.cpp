// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
ZilchDefineEvent(TranslationError);
ZilchDefineEvent(ValidationError);
} // namespace Events

ZilchDefineType(TranslationErrorEvent, builder, type)
{
}

String TranslationErrorEvent::GetFormattedMessage(Zilch::MessageFormat::Enum format)
{
  return mLocation.GetFormattedStringWithMessage(format, mFullMessage);
}

ZilchDefineType(ValidationErrorEvent, builder, type)
{
}

String ValidationErrorEvent::GetFormattedMessage(Zilch::MessageFormat::Enum format)
{

  StringBuilder builder;
  // Write the full message with the location
  String msg = BuildString(mShortMessage, "\n", mFullMessage);
  builder.Append(mLocation.GetFormattedStringWithMessage(format, msg));
  // Append all call stack locations to the message (to trace the error)
  for (size_t i = 0; i < mCallStack.Size(); ++i)
  {
    builder.AppendFormat("%s:\n", mCallStack[i].GetFormattedString(format).c_str());
  }

  return builder.ToString();
}

ShaderCompilationErrors::ShaderCompilationErrors()
{
  mErrorTriggered = false;
  mEmitMultipleErrors = false;
}

void ShaderCompilationErrors::SendTranslationError(Zilch::CodeLocation& location, StringParam message)
{
  SendTranslationError(location, message, message);
}

void ShaderCompilationErrors::SendTranslationError(Zilch::CodeLocation& location,
                                                   StringParam shortMsg,
                                                   StringParam fullMsg)
{
  // Check if this is the first error being sent and if not check if we send
  // multiple errors
  if (mErrorTriggered && !mEmitMultipleErrors)
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
  EventConnect(&zilchErrors, Zilch::Events::CompilationError, &ShaderCompilationErrors::ForwardErrorEvent, this);
}

void ShaderCompilationErrors::ListenForTypeParsed(Zilch::CompilationErrors& zilchErrors)
{
  EventConnect(&zilchErrors, Zilch::Events::TypeParsed, &ShaderCompilationErrors::ForwardGenericEvent, this);
}

void ShaderCompilationErrors::ForwardErrorEvent(Zilch::ErrorEvent* e)
{
  mErrorTriggered = true;
  EventSend(this, e->EventName, e);
}

void ShaderCompilationErrors::ForwardGenericEvent(Zilch::EventData* e)
{
  EventSend(this, e->EventName, e);
}

} // namespace Zero
