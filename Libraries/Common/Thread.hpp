// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Is threading enabled on this platform?
ZeroShared extern const bool ThreadingEnabled;

/// Thread class manages Os threads.
class ZeroShared Thread
{
public:
  typedef OsInt (*EntryFunction)(void*);

  // Construct a thread object does not create the thread.
  // Initialize and then resume to run the thread.
  Thread();
  ~Thread();

  // Is this a valid thread or uninitialized.
  bool IsValid();

  // Initializes the thread but does not run it.
  bool Initialize(EntryFunction entryFunction, void* instance, StringParam threadName);

  // Close the thread handle. The thread should have been shut down before
  // calling this function.
  void Close();

  // Block waiting for the thread to complete.
  // The thread should either complete in a reasonable way or
  // be signaled to be closed.
  OsInt WaitForCompletion();
  // Wait for the thread to complete up to a certain number of milliseconds.
  // The thread should either complete in a reasonable way or
  // be signaled to be closed.
  OsInt WaitForCompletion(unsigned long milliseconds);

  // Is the thread completed?
  bool IsCompleted();

  // Removes the thread from this object.
  OsHandle Detach();

  // Get the OsHandle to the thread.
  OsHandle GetThreadHandle();

  // Get the thread id of this thread
  size_t GetThreadId();

  // Get the current thread id of the running thread
  static size_t GetCurrentThreadId();

  // Is this the thread that the platform library was initialized on?
  static bool IsMainThread();

  // The id of the main thread that the platform library was initialized on
  static size_t MainThreadId;

  // Template Helper for creating Entry Functions
  // From member functions
  template <typename classType, OsInt (classType::*MemberFunction)()>
  static OsInt ObjectEntryCreator(void* objectInstance)
  {
    classType* object = (classType*)objectInstance;
    OsInt returnValue = (object->*MemberFunction)();
    return returnValue;
  }

private:
  String mThreadName;
  ZeroDeclarePrivateData(Thread, 20);
};

} // namespace Zero
