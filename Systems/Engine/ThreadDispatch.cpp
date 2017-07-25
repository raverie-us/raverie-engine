///////////////////////////////////////////////////////////////////////////////
///
/// \file ThreadDispatch.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
ThreadDispatch* gDispatch = nullptr;
}

ThreadEvents::~ThreadEvents()
{
  forRange(QueuedEvent& queuedEvent, Events.All())
  {
    delete queuedEvent.EventToSend;
  }
  Events.Clear();
}

ThreadDispatch::ThreadDispatch()
{
  Z::gDispatch = this;
  AddThreadContext(ThreadContext::Main);
  AddThreadContext(ThreadContext::FileIo);
}

ThreadDispatch::~ThreadDispatch()
{
  DeleteObjectsInContainer(ThreadEventMap);
}

void ThreadDispatch::AddThreadContext(ThreadContextId id)
{
  mLock.Lock();
  ThreadEventMap[id] = new ThreadEvents();
  mLock.Unlock();
}

void ThreadDispatch::DispatchOn(HandleParam object, ThreadContextId contextId, EventDispatcher* eventDispatcher, StringParam eventId, Event* event)
{
  //If this is null, ThreadDispatch has been shutdown. To prevent
  //crashes with destruction order just do nothing.
  if(this == nullptr)
    return;

  QueuedEvent queuedEvent;
  queuedEvent.Object = object;
  queuedEvent.EventToSend = event;
  queuedEvent.EventDispatcherOn = eventDispatcher;
  queuedEvent.EventId  = eventId;

  mLock.Lock();
  ThreadEventMap[contextId]->Events.PushBack(queuedEvent);
  mLock.Unlock();
}

void ThreadDispatch::DispatchEventsFor(ThreadContextId contextId)
{
  Array<QueuedEvent> eventsToDispatch;

  //To avoid dead lock pull out all message before dispatching
  //(dispatching may add more events)
  mLock.Lock();
  eventsToDispatch.Swap(ThreadEventMap[contextId]->Events);
  mLock.Unlock();

  forRange(QueuedEvent& queuedEvent, eventsToDispatch.All())
  {
    //Check to see if the object is still alive
    if(queuedEvent.Object.IsNull() == false)
      queuedEvent.EventDispatcherOn->Dispatch(queuedEvent.EventId, queuedEvent.EventToSend);

    //delete the event
    delete queuedEvent.EventToSend;
  }
  eventsToDispatch.Clear();
}



void StartThreadSystem()
{
  Z::gDispatch = new ThreadDispatch();
  Z::gJobs = new JobSystem();
}

void ShutdownThreadSystem()
{
  // This is important that the jobs are deleted first, because the job threads could be using the gDispatch
  SafeDelete(Z::gJobs);
  SafeDelete(Z::gDispatch);
}

}
