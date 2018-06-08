///////////////////////////////////////////////////////////////////////////////
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
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

}// namespace Zero
