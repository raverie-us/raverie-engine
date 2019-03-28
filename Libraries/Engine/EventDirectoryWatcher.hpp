// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DeclareEvent(FileModified);
DeclareEvent(FileCreated);
DeclareEvent(FileDeleted);
DeclareEvent(FileRenamed);
} // namespace Events

class FileEditEvent : public Event
{
public:
  ZilchDeclareType(FileEditEvent, TypeCopyMode::ReferenceType);
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
  ZilchDeclareType(EventDirectoryWatcher, TypeCopyMode::ReferenceType);

  EventDirectoryWatcher(StringParam directory);

  OsInt FileCallBack(DirectoryWatcher::FileOperationInfo& info);
  DirectoryWatcher mWatcher;
};

} // namespace Zero
