// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
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
  RaverieDeclareType(FileEditEvent, TypeCopyMode::ReferenceType);
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
  RaverieDeclareType(EventDirectoryWatcher, TypeCopyMode::ReferenceType);

  EventDirectoryWatcher(StringParam directory);

  OsInt FileCallBack(DirectoryWatcher::FileOperationInfo& info);
  DirectoryWatcher mWatcher;
};

} // namespace Raverie
