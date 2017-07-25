///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
ZilchDeclareEvent(TranslationError, TranslationErrorEvent);
}//namespace Events

//-------------------------------------------------------------------TranslationErrorEvent
// An error event for when translation fails
class TranslationErrorEvent : public Zilch::EventData
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);
  String GetFormattedMessage(Zilch::MessageFormat::Enum format);

  String mShortMessage;
  String mFullMessage;
  Zilch::CodeLocation mLocation;
};

//-------------------------------------------------------------------ShaderCompilationErrors
// Event handler for sending shader compilation errors as well as translation errors.
class ShaderCompilationErrors : public Zilch::EventHandler
{
public:
  ShaderCompilationErrors();

  void SendTranslationError(Zilch::CodeLocation& location, StringParam message);
  void SendTranslationError(Zilch::CodeLocation& location, StringParam shortMsg, StringParam fullMsg);

  void ListenForZilchErrors(Zilch::CompilationErrors& zilchErrors);
  void ForwardEvent(Zilch::ErrorEvent* e);

  /// Was an error triggered during compilation (either from translation or zilch)
  bool mErrorTriggered;
  bool mEmitMultipleErrors;
};

}//namespace Zero
