// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
ZilchDeclareEvent(TranslationError, TranslationErrorEvent);
ZilchDeclareEvent(ValidationError, ValidationErrorEvent);
} // namespace Events

/// An error event for when translation fails
class TranslationErrorEvent : public Zilch::EventData
{
public:
  ZilchDeclareType(TranslationErrorEvent, Zilch::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Zilch::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Zilch::CodeLocation mLocation;
};

/// An error even dispatched during validation. Mostly the same as a translation
/// error event, but this also contains a call stack to trace where an error
/// occurred.
class ValidationErrorEvent : public Zilch::EventData
{
  ZilchDeclareType(ValidationErrorEvent, Zilch::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Zilch::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Zilch::CodeLocation mLocation;

  Array<Zilch::CodeLocation> mCallStack;
};

/// Event handler for sending shader compilation errors as well as translation
/// errors.
class ShaderCompilationErrors : public Zilch::EventHandler
{
public:
  ShaderCompilationErrors();

  void SendTranslationError(Zilch::CodeLocation& location, StringParam message);
  void SendTranslationError(Zilch::CodeLocation& location, StringParam shortMsg, StringParam fullMsg);

  void ListenForZilchErrors(Zilch::CompilationErrors& zilchErrors);
  void ListenForTypeParsed(Zilch::CompilationErrors& zilchErrors);
  void ForwardErrorEvent(Zilch::ErrorEvent* e);
  void ForwardGenericEvent(Zilch::EventData* e);

  /// Was an error triggered during compilation (either from translation or
  /// zilch)
  bool mErrorTriggered;
  bool mEmitMultipleErrors;
};

} // namespace Zero
