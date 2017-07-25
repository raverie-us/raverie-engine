///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(FileModified);
  DeclareEvent(FileCreated);
  DeclareEvent(FileDeleted);
  DeclareEvent(FileRenamed);
}

class FileEditEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// If it was a rename, this should be set to the old file name
  String OldFileName;
  String FileName;

  /// The time at which the event happened.
  TimeType TimeStamp;
};

// Watches a directory and sends out events on the main thread.
class EventDirectoryWatcher : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EventDirectoryWatcher(StringParam directory);

  OsInt FileCallBack(DirectoryWatcher::FileOperationInfo& info);
  DirectoryWatcher mWatcher;
};

}
