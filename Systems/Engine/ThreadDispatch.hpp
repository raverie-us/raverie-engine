///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef uint ThreadContextId;

struct QueuedEvent
{
  Handle Object;
  String EventId;
  Event* EventToSend;
  EventDispatcher* EventDispatcherOn;
};

//-------------------------------------------------------------------ThreadDispatch
class ThreadDispatch
{
public:
  ThreadDispatch();
  ~ThreadDispatch();

  template<typename type>
  void Dispatch(type* object, StringParam eventId, Event* event)
  {
    DispatchOn(object, object->GetDispatcher(), eventId, event);
  }

  void DispatchOn(HandleParam object, EventDispatcher* eventDispatcher, StringParam eventId, Event* event);
  void DispatchEvents();
  void ClearEvents();

private:
  ThreadLock mLock;
  Array<QueuedEvent> mEvents;
};

namespace Z
{
  extern ThreadDispatch* gDispatch;
}


void StartThreadSystem();
void ShutdownThreadSystem();

inline void SendBlockingTaskStart(StringParam taskName)
{
  Z::gDispatch->Dispatch(Z::gEngine, Events::BlockingTaskStart, new BlockingTaskEvent(taskName));
}

inline void SendBlockingTaskFinish()
{
  Z::gDispatch->Dispatch(Z::gEngine, Events::BlockingTaskFinish, new Event());
}

}
