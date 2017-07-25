///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

DirectoryWatcher::DirectoryWatcher(cstr directoryToWatch, CallbackFunction callback, void* callbackInstance)
{
  ZeroCStringCopy(mDirectoryToWatch, File::MaxPath, directoryToWatch, strlen(directoryToWatch));

  mCallbackInstance = callbackInstance;
  mCallback = callback;

  mWorkThread.Initialize(Thread::ObjectEntryCreator<DirectoryWatcher, &DirectoryWatcher::RunThreadEntryPoint>, this, "DirectoryWatcherWorker");
  mWorkThread.Resume();
  mCancelEvent.Initialize(true, false);
}

DirectoryWatcher::~DirectoryWatcher()
{
  Shutdown();
}

void DirectoryWatcher::Shutdown()
{
  mCancelEvent.Signal();
  mWorkThread.WaitForCompletion();
}

OsInt DirectoryWatcher::RunThreadEntryPoint()
{
  //Do not prevent others from using the directory.
  const OsInt shareAll = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

  //Create a directory handle which in windows is done with CreateFile
  //FILE_LIST_DIRECTORY is an access right required for ReadDirectoryChangesW to operate.
  //FILE_FLAG_OVERLAPPED is required to use overlaps with the handle.
  //FILE_FLAG_BACKUP_SEMANTICS is required to create a directory handle.
  const OsInt flags = FILE_FLAG_OVERLAPPED |FILE_FLAG_BACKUP_SEMANTICS;

  //Create the directory handle
  OsHandle dirHandle = CreateFileW(Widen(mDirectoryToWatch).c_str(), FILE_LIST_DIRECTORY, shareAll, NULL, OPEN_EXISTING,  flags, NULL);
  CheckWin(dirHandle!=INVALID_HANDLE_VALUE,"Failed to create directory handle.");
  if(dirHandle==INVALID_HANDLE_VALUE)
    return (OsInt)-1;

  //4k buffer for directory events.
  const OsInt cBufferSize = 1024 * 4;
  byte fileNotifyBuffer[cBufferSize];

  //Create Io control so the directory watcher can be canceled.
  IoControl readDir;
  InitIoControl(readDir, mCancelEvent);

  //Loop until cancel
  for(;;)
  {
    OsInt bytesReturned = 0;

    //Looking for file updates, additions, and removal.
    OsInt filterFlags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE;

    ReadDirectoryChangesW(dirHandle, fileNotifyBuffer, cBufferSize, TRUE,  filterFlags,  &bytesReturned, IoGetOverlap(readDir), NULL);
    OsInt result = WaitForIoCompletion(readDir);

    if(result == IoFinished)
    {
      //Overlapped Io has finished process events.
      OsInt overlapResult = GetOverlappedResult(dirHandle, IoGetOverlap(readDir), &bytesReturned, TRUE);

      //Iterate through the directory event buffer.
      byte* buffer = (byte*)fileNotifyBuffer;

      String lastRename;

      for(;;)
      {
        const OsInt cFileNameBufferSize = MAX_PATH;
        char asciFilename[cFileNameBufferSize];
        FILE_NOTIFY_INFORMATION& notify = *(FILE_NOTIFY_INFORMATION*)buffer;

        // FileNameLength is in bytes so divide by size of WCHAR
        uint characterLength = notify.FileNameLength / sizeof(WCHAR);
        WideCharToMultiByte(CP_ACP, 0, notify.FileName, notify.FileNameLength, asciFilename, cFileNameBufferSize, "?", NULL);

        FileOperationInfo info;
        info.FileName = String(asciFilename, characterLength);

        switch(notify.Action)
        {
        case FILE_ACTION_ADDED:
          info.Operation = Added;
          (*mCallback)(mCallbackInstance, info);
          break;
        case FILE_ACTION_REMOVED:
          info.Operation = Removed;
          (*mCallback)(mCallbackInstance, info);
          break;
        case FILE_ACTION_MODIFIED:
          info.Operation = Modified;
          (*mCallback)(mCallbackInstance, info);
          break;
        case FILE_ACTION_RENAMED_OLD_NAME:
          lastRename = info.FileName;
          break;
        case FILE_ACTION_RENAMED_NEW_NAME:
          ErrorIf(lastRename.Empty(), "We didn't get an old name event.");
          info.Operation = Modified;
          info.OldFileName = lastRename;
          (*mCallback)(mCallbackInstance, info);
          break;
        }

        if(notify.NextEntryOffset ==0)
        {
          //No more entries in this read.
          break;
        }
        else
        {
          //Move to the next file notification. FILE_NOTIFY_INFORMATION can be variably size
          //due to the filename so the stride between structures is given with the field NextEntryOffset.
          //When the size is zero it is the last message.
          buffer = buffer + notify.NextEntryOffset;
        }
      }
    }
    else if(result == IoTerminated)
    {
      CancelIo(dirHandle);
      CloseHandle(dirHandle);
      CleanUpIoControl(readDir);
      return 0;//Exit the thread
    }
  }
}

}//namespace Zero
