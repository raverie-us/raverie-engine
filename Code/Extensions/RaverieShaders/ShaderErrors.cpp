// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
RaverieDefineEvent(TranslationError);
RaverieDefineEvent(ValidationError);
} // namespace Events

RaverieDefineType(TranslationErrorEvent, builder, type)
{
}

String TranslationErrorEvent::GetFormattedMessage(Raverie::MessageFormat::Enum format)
{
  return mLocation.GetFormattedStringWithMessage(format, mFullMessage);
}

RaverieDefineType(ValidationErrorEvent, builder, type)
{
}

String ValidationErrorEvent::GetFormattedMessage(Raverie::MessageFormat::Enum format)
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

void ShaderCompilationErrors::SendTranslationError(Raverie::CodeLocation& location, StringParam message)
{
  SendTranslationError(location, message, message);
}

void ShaderCompilationErrors::SendTranslationError(Raverie::CodeLocation& location,
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

void ShaderCompilationErrors::ListenForRaverieErrors(Raverie::CompilationErrors& raverieErrors)
{
  EventConnect(&raverieErrors, Raverie::Events::CompilationError, &ShaderCompilationErrors::ForwardErrorEvent, this);
}

void ShaderCompilationErrors::ListenForTypeParsed(Raverie::CompilationErrors& raverieErrors)
{
  EventConnect(&raverieErrors, Raverie::Events::TypeParsed, &ShaderCompilationErrors::ForwardGenericEvent, this);
}

void ShaderCompilationErrors::ForwardErrorEvent(Raverie::ErrorEvent* e)
{
  mErrorTriggered = true;
  EventSend(this, e->EventName, e);
}

void ShaderCompilationErrors::ForwardGenericEvent(Raverie::EventData* e)
{
  EventSend(this, e->EventName, e);
}

} // namespace Raverie
