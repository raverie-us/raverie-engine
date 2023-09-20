// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
RaverieDeclareEvent(TranslationError, TranslationErrorEvent);
RaverieDeclareEvent(ValidationError, ValidationErrorEvent);
} // namespace Events

/// An error event for when translation fails
class TranslationErrorEvent : public Raverie::EventData
{
public:
  RaverieDeclareType(TranslationErrorEvent, Raverie::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Raverie::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Raverie::CodeLocation mLocation;
};

/// An error even dispatched during validation. Mostly the same as a translation
/// error event, but this also contains a call stack to trace where an error
/// occurred.
class ValidationErrorEvent : public Raverie::EventData
{
  RaverieDeclareType(ValidationErrorEvent, Raverie::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Raverie::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Raverie::CodeLocation mLocation;

  Array<Raverie::CodeLocation> mCallStack;
};

/// Event handler for sending shader compilation errors as well as translation
/// errors.
class ShaderCompilationErrors : public Raverie::EventHandler
{
public:
  ShaderCompilationErrors();

  void SendTranslationError(Raverie::CodeLocation& location, StringParam message);
  void SendTranslationError(Raverie::CodeLocation& location, StringParam shortMsg, StringParam fullMsg);

  void ListenForRaverieErrors(Raverie::CompilationErrors& raverieErrors);
  void ListenForTypeParsed(Raverie::CompilationErrors& raverieErrors);
  void ForwardErrorEvent(Raverie::ErrorEvent* e);
  void ForwardGenericEvent(Raverie::EventData* e);

  /// Was an error triggered during compilation (either from translation or
  /// raverie)
  bool mErrorTriggered;
  bool mEmitMultipleErrors;
};

} // namespace Raverie
