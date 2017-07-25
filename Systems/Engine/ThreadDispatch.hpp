///////////////////////////////////////////////////////////////////////////////
///
/// \file ThreadDispatch.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(ThreadContext, Main, FileIo);
typedef uint ThreadContextId;

struct QueuedEvent
{
  Handle Object;
  String EventId;
  Event* EventToSend;
  EventDispatcher* EventDispatcherOn;
};

class ThreadEvents
{
public:
  ~ThreadEvents();
  Array<QueuedEvent> Events;
};

class ThreadDispatch
{
public:
  ThreadDispatch();
  ~ThreadDispatch();

  template<typename type>
  void Dispatch(ThreadContextId contextId, type* object, StringParam eventId, Event* event)
  {
    DispatchOn(object, contextId, object->GetDispatcher(), eventId, event);
  }

  void AddThreadContext(ThreadContextId id);
  void DispatchOn(HandleParam object, ThreadContextId contextId, EventDispatcher* eventDispatcher, StringParam eventId, Event* event);
  void DispatchEventsFor(ThreadContextId context);

private:
  ThreadLock mLock;
  HashMap<uint, ThreadEvents*> ThreadEventMap;
};

namespace Z
{
  extern ThreadDispatch* gDispatch;
}

void StartThreadSystem();
void ShutdownThreadSystem();

inline void SendBlockingTaskStart(StringParam taskName)
{
  Z::gDispatch->Dispatch(ThreadContext::Main, Z::gEngine, Events::BlockingTaskStart, new BlockingTaskEvent(taskName));
}

inline void SendBlockingTaskFinish()
{
  Z::gDispatch->Dispatch(ThreadContext::Main, Z::gEngine, Events::BlockingTaskFinish, new Event());
}

}
