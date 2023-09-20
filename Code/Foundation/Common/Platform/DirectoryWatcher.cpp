// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

DirectoryWatcher::DirectoryWatcher(cstr directoryToWatch, CallbackFunction callback, void* callbackInstance)
{
  RaverieCStringCopy(mDirectoryToWatch, File::MaxPath, directoryToWatch, strlen(directoryToWatch));

  mCallbackInstance = callbackInstance;
  mCallback = callback;

  if (ThreadingEnabled)
  {
    mWorkThread.Initialize(Thread::ObjectEntryCreator<DirectoryWatcher, &DirectoryWatcher::RunThreadEntryPoint>,
                           this,
                           "DirectoryWatcherWorker");
    mCancelEvent.Initialize(true, false);
  }
}

DirectoryWatcher::~DirectoryWatcher()
{
  Shutdown();
}

void DirectoryWatcher::Shutdown()
{
  if (ThreadingEnabled)
  {
    mCancelEvent.Signal();
    mWorkThread.WaitForCompletion();
  }
}

} // namespace Raverie
