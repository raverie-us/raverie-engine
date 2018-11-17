///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(PartialStandardOutputResponse);
DeclareEvent(PartialStandardErrorResponse);
DeclareEvent(StandardOutputFinished);
DeclareEvent(StandardErrorFinished);
}//namespace Events

DeclareEnum3(StreamType, StandardOutput, StandardError, StandardInput);

//-------------------------------------------------------------------AsyncProcessEvent
/// Sent out when AsyncProcess completes a partial read for
/// a stream or the stream has finished reading all data.
class AsyncProcessEvent : public Event
{
public:
  ZilchDeclareType(AsyncProcessEvent, TypeCopyMode::ReferenceType);

  /// Bytes being read from a stream. Note: These bytes may not form a
  /// valid string if the stream type was non ascii (e.g. utf-8).
  String mBytes;
  /// The type of stream that sent this event.
  StreamType::Enum mStreamType;
};

//-------------------------------------------------------------------AsyncProcess
/// A process class that asynchronously reads from standard output and standard
/// error and sends out partial read events. Additionally, the full buffer can
/// be stored for each stream. This makes it possible to read the output of a
/// process in a single-threaded context without having to block on output.
class AsyncProcess : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(AsyncProcess, TypeCopyMode::ReferenceType);

  /// Construct a new process. This does not start the process.
  static AsyncProcess* Create();
  
  AsyncProcess();
  ~AsyncProcess();

  /// Begins the execution of another process using the given parameters.
  /// Throws an exception if the process cannot be started.
  bool Start(ProcessStartInfo& startInfo);
  /// Begins the execution of another process using the given parameters.
  void Start(Status& status, ProcessStartInfo& startInfo);

  /// Returns true if the process is still running, false otherwise.
  bool IsRunning();
  /// Waits for a process to close, this will block until the process closes.
  int WaitForClose();
  /// Waits for a process to close up to a given number of milliseconds.
  /// This can take up to 3 * milliseconds due to waiting for the output streams to close.
  int WaitForClose(int milliseconds);
  /// Closes the wrapper around the process, does not close the process launched.
  void Close();
  /// Attempts to manually shut down the process. This is not safe for the other process or what it's handling.
  void Terminate();

  /// Should the results from standard output be accumulated and stored? If a lot of data
  /// is output it may be good to turn this off and use the partial data callback events instead.
  bool GetStoreStandardOutputData();
  void SetStoreStandardOutputData(bool state);
  /// Should the results from standard error be accumulated and stored? If a lot of data
  /// is output it may be good to turn this off and use the partial data callback events instead.
  bool GetStoreStandardErrorData();
  void SetStoreStandardErrorData(bool state);

  /// The cached total results from standard output. Will be empty if StoreStandardOutputData is false.
  String GetStandardOutput();
  /// The cached total results from standard error. Will be empty if StoreStandardErrorData is false.
  String GetStandardError();

private:
  /// The amount of data that the threads will block to read.
  const static size_t mReadBufferSize = 64;

  /// Closes active streams and waits for the read threads to finish.
  void CloseInternal();
  /// Stop all old event connections and start new ones
  /// (depends on if data is stored for an output stream).
  void ResetEventConnections();
  /// We got back some data from a thread.
  void OnPartialResponse(AsyncProcessEvent* e);
  /// A thread has finished so we can correctly mark that we have all of it's data.
  void OnFinished(AsyncProcessEvent* e);

  /// Info for a running thread. Helps to generalize thread response functions. 
  struct ThreadInfo
  {
    ThreadInfo();

    bool IsRunning();
    void Reset();

    Thread mThread;
    File mFileStream;
    /// Is the thread currently running? Assures that AsyncProcess gets all data back on the main thread.
    bool mIsRunning;
    /// Should the thread's data be cached in one buffer? Convenience data for the user.
    bool mShouldStoreResults;
    /// The cached output data of the stream.
    String mResults;
  };

  static OsInt StandardOutputThreadFn(void* threadStartData);
  static OsInt StandardErrorThreadFn(void* threadStartData);
  /// Shared thread running function for standard output/error.
  static OsInt RunThread(AsyncProcess* asyncProcess, StreamType::Enum streamType, StringParam partialResponseEventName, StringParam finishedEventName);

  Process mProcess;
  ThreadInfo mThreads[2];
  HandleOf<Zilch::FileStreamClass> mStandardInput;
  ObjectThreadDispatch mEventDispatchList;
};

}// namespace Zero
