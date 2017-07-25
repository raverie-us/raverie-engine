///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Thread.hpp"
#include "ThreadSync.hpp"
#include "File.hpp"

namespace Zero
{
class Thread;
class Event;

class ZeroShared DirectoryWatcher
{
public:
  
  enum FileOperation
  {
    Added,
    Removed,
    Modified,
    Renamed
  };

  struct FileOperationInfo
  {
    FileOperation Operation;
    String OldFileName;
    String FileName;
  };

  typedef OsInt (*CallbackFunction)(void* callbackInstance, FileOperationInfo& info);

  DirectoryWatcher(cstr directoryToWatch, CallbackFunction callback, void* callbackInstance);
  ~DirectoryWatcher();
  void Shutdown();

  template<typename classType, OsInt (classType::*MemberFunction)(FileOperationInfo& info)>
  static OsInt CallBackCreator(void* objectInstance, FileOperationInfo& info)
  {
    classType* object = (classType*)objectInstance;
    OsInt returnValue = (object->*MemberFunction)(info);
    return returnValue;
  }

private:
  // Note: The directory watcher currently has no private data
  // because it stores everything on the stack of its thread
  char mDirectoryToWatch[File::MaxPath];
  CallbackFunction mCallback;
  void* mCallbackInstance;
  OsInt RunThreadEntryPoint();
  Thread mWorkThread;
  OsEvent mCancelEvent;
};

}//namespace Zero
