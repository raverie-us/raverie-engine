///////////////////////////////////////////////////////////////////////////////
///
/// \file EngineEvents.hpp
/// Declaration of the engine events.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(GeneralEngine);
  DeclareEvent(ScriptInitialize);
}//namespace Events


//------------------------------------------------------------------- Text Event
class TextEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  TextEvent(StringParam text) : Text(text) {}
  String Text;
};

//------------------------------------------------------------------ Error Event
class TextErrorEvent : public TextEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  TextErrorEvent(StringParam text, int code) : TextEvent(text), Code(code) {}
  int Code;
};

//--------------------------------------------------------------- Progress Event
DeclareEnum3(ProgressType, Normal, Indeterminate, None);

class ProgressEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ProgressEvent();
  ProgressType::Enum ProgressType;
  String Operation;
  String CurrentTask;
  String ProgressLine;
  float Percentage;
};

//----------------------------------------------------------- BlockingTask Event
class BlockingTaskEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BlockingTaskEvent(StringParam taskName = String()) : mTaskName(taskName) {}
  String mTaskName;
};

}//namespace Zero
