// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(ScriptInitialize);
} // namespace Events

class TextEvent : public Event
{
public:
  RaverieDeclareType(TextEvent, TypeCopyMode::ReferenceType);
  TextEvent(StringParam text) : Text(text)
  {
  }
  String Text;
};

class TextErrorEvent : public TextEvent
{
public:
  RaverieDeclareType(TextErrorEvent, TypeCopyMode::ReferenceType);
  TextErrorEvent(StringParam text, int code) : TextEvent(text), Code(code)
  {
  }
  int Code;
};

class BlockingTaskEvent : public Event
{
public:
  RaverieDeclareType(BlockingTaskEvent, TypeCopyMode::ReferenceType);

  BlockingTaskEvent(StringParam taskName = String()) : mTaskName(taskName)
  {
  }
  String mTaskName;
};

} // namespace Raverie
