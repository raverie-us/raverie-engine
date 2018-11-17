///////////////////////////////////////////////////////////////////////////////
///
/// \file ThreadIo.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//Io control structure is used to control
//Overlapped blocking IO. This is a unified
//way of waiting for both the terminate event.
//and for the operation to be complete.
struct IoControl
{
  OVERLAPPED IoOverlap;
  OsHandle TerminateEvent;
  OsHandle IoCompletedEvent;
};

inline void InitIoControl(IoControl& ioControl, OsEvent& terminateEvent)
{
  //Zero the structure
  ZeroMemory(&ioControl,  sizeof(IoControl));
  //Create the event used to signal that the overlapped io is complete.
  ioControl.IoCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  ioControl.IoOverlap.hEvent = ioControl.IoCompletedEvent;
  //Store the terminate event
  ioControl.TerminateEvent = terminateEvent.GetHandle();
}

inline OVERLAPPED* IoGetOverlap(IoControl& ioControl)
{
  return &ioControl.IoOverlap;
}

inline void IoReset(IoControl& ioControl)
{
  ResetEvent(ioControl.IoCompletedEvent);
}

inline void CleanUpIoControl(IoControl& ioControl)
{
  CloseHandle(ioControl.IoCompletedEvent);
}

const OsInt IoFinished = WAIT_OBJECT_0;
const OsInt IoTerminated = WAIT_OBJECT_0+1;

inline OsInt WaitForIoCompletion(IoControl& ioControl)
{
  OsHandle events[2] = {ioControl.IoOverlap.hEvent, ioControl.TerminateEvent};
  return WaitForMultipleObjects(2, events, FALSE, INFINITE);
}

}//namespace Zero
