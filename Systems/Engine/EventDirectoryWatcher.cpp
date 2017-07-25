///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(FileModified);
  DefineEvent(FileCreated);
  DefineEvent(FileDeleted);
  DefineEvent(FileRenamed);
}

ZilchDefineType(EventDirectoryWatcher, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

ZilchDefineType(FileEditEvent, builder, type)
{
}

EventDirectoryWatcher::EventDirectoryWatcher(StringParam directory)
  :mWatcher(directory.c_str(), DirectoryWatcher::CallBackCreator<EventDirectoryWatcher, &EventDirectoryWatcher::FileCallBack>, this)
{
}

OsInt EventDirectoryWatcher::FileCallBack(DirectoryWatcher::FileOperationInfo& info)
{
  FileEditEvent* event = new FileEditEvent();
  event->FileName = info.FileName;
  event->OldFileName = info.OldFileName;
  event->TimeStamp = Time::Clock();

  String eventName = Events::FileModified;
  if(info.Operation == DirectoryWatcher::Added)
    eventName = Events::FileCreated;
  if(info.Operation == DirectoryWatcher::Removed)
    eventName = Events::FileDeleted;
  if(info.Operation == DirectoryWatcher::Renamed)
    eventName = Events::FileRenamed;

  Z::gDispatch->DispatchOn(this, ThreadContext::Main, this->GetDispatcher(), eventName, event);
  return 0;
}

}
