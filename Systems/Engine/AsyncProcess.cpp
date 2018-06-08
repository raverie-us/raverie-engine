///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(PartialStandardOutputResponse);
DefineEvent(PartialStandardErrorResponse);
DefineEvent(StandardOutputFinished);
DefineEvent(StandardErrorFinished);
}//namespace Events

 //-------------------------------------------------------------------AsyncProcessEvent
ZilchDefineType(AsyncProcessEvent, builder, type)
{
  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
  ZeroBindDocumented();

  ZilchBindField(mBytes);
  ZilchBindField(mStreamType);
}

//-------------------------------------------------------------------AsyncProcess
ZilchDefineType(AsyncProcess, builder, type)
{
  ZilchBindDestructor();
  ZeroBindDocumented();

  ZeroBindEvent(Events::PartialStandardOutputResponse, AsyncProcessEvent);
  ZeroBindEvent(Events::PartialStandardErrorResponse, AsyncProcessEvent);
  ZeroBindEvent(Events::StandardOutputFinished, Event);
  ZeroBindEvent(Events::StandardErrorFinished, Event);

  ZilchBindMethod(Create);
  ZilchFullBindMethod(builder, type, &AsyncProcess::Start, ZilchInstanceOverload(bool, ProcessStartInfo&), "Start", "startInfo");
  ZilchBindMethod(IsRunning);
  ZilchFullBindMethod(builder, type, &AsyncProcess::WaitForClose, ZilchInstanceOverload(int), "WaitForClose", "");
  ZilchFullBindMethod(builder, type, &AsyncProcess::WaitForClose, ZilchInstanceOverload(int, int), "WaitForClose", "milliseconds");
  ZilchBindMethod(Close);
  ZilchBindMethod(Terminate);
  ZilchBindGetterSetter(StoreStandardOutputData);
  ZilchBindGetterSetter(StoreStandardErrorData);
  ZilchBindField(mStandardInput);
  ZilchBindGetter(StandardOutput);
  ZilchBindGetter(StandardError);
}

AsyncProcess* AsyncProcess::Create()
{
  // This function is currently required to create reference counted objects from script.
  AsyncProcess* result = new AsyncProcess();
  return result;
}

AsyncProcess::AsyncProcess()
{
  ConnectThisTo(this, Events::StandardOutputFinished, OnFinished);
  ConnectThisTo(this, Events::StandardErrorFinished, OnFinished);
}

AsyncProcess::~AsyncProcess()
{
  Close();
}

bool AsyncProcess::Start(ProcessStartInfo& startInfo)
{
  Status status;
  Start(status, startInfo);

  // If we failed to open the process then throw an exception and return.
  if(status.Failed())
  {
    ExecutableState::CallingState->ThrowException(status.Message);
    return false;
  }

  return true;
}

void AsyncProcess::Start(Status& status, ProcessStartInfo& startInfo)
{
  // Close the previous running thread if there was one
  Close();
  // Since we have a new process start info, different streams
  // could have data cached. Reconnect to the partial data events.
  ResetEventConnections();
  // Reset the thread states (not running, etc...)
  for(size_t i = 0; i < 2; ++i)
    mThreads[i].Reset();

  mProcess.Start(status, startInfo);
  // If we failed to open the process then return
  if(status.Failed())
    return;

  // Start the standard output thread if necessary
  if(startInfo.mRedirectStandardOutput)
  {
    ThreadInfo& threadInfo = mThreads[StreamType::StandardOutput];
    threadInfo.mIsRunning = true;
    mProcess.OpenStandardOut(threadInfo.mFileStream);
    threadInfo.mThread.Initialize(AsyncProcess::StandardOutputThreadFn, this, "StandardOutputThread");
  }
  // Start the standard error thread if necessary
  if(startInfo.mRedirectStandardError)
  {
    ThreadInfo& threadInfo = mThreads[StreamType::StandardError];
    threadInfo.mIsRunning = true;
    mProcess.OpenStandardError(threadInfo.mFileStream);
    threadInfo.mThread.Initialize(AsyncProcess::StandardErrorThreadFn, this, "StandardErrorThread");
  }
  // Redirect standard input if necessary
  if(startInfo.mRedirectStandardInput)
  {
    mStandardInput = ZilchAllocate(FileStream);
    FileStream* standardInput = mStandardInput;
    mProcess.OpenStandardIn(standardInput->InternalFile);
    standardInput->Capabilities = StreamCapabilities::Write;
  }
}

bool AsyncProcess::IsRunning()
{
  // If the process is still running or any of the output streams are running
  // then so are we. (This blocks until we have all data from the streams).
  bool standardOutputRunning = mThreads[StreamType::StandardOutput].IsRunning();
  bool standardErrorRunning = mThreads[StreamType::StandardError].IsRunning();
  return mProcess.IsRunning() || standardOutputRunning || standardErrorRunning;
}

int AsyncProcess::WaitForClose()
{
  int result = mProcess.WaitForClose();
  mThreads[StreamType::StandardOutput].mThread.WaitForCompletion();
  mThreads[StreamType::StandardError].mThread.WaitForCompletion();
  return result;
}

int AsyncProcess::WaitForClose(int milliseconds)
{
  // This will wait up to 3 * milliseconds.
  int result = mProcess.WaitForClose(milliseconds);
  mThreads[StreamType::StandardOutput].mThread.WaitForCompletion(milliseconds);
  mThreads[StreamType::StandardError].mThread.WaitForCompletion(milliseconds);
  return result;
}

void AsyncProcess::Close()
{
  // Close the active streams
  CloseInternal();
  mProcess.Close();
}

void AsyncProcess::Terminate()
{
  // Close the active streams
  CloseInternal();
  mProcess.Terminate();
}

String AsyncProcess::GetStandardOutput()
{
  return mThreads[StreamType::StandardOutput].mResults;
}

String AsyncProcess::GetStandardError()
{
  return mThreads[StreamType::StandardError].mResults;
}

bool AsyncProcess::GetStoreStandardOutputData()
{
  return mThreads[StreamType::StandardOutput].mShouldStoreResults;
}

void AsyncProcess::SetStoreStandardOutputData(bool state)
{
  mThreads[StreamType::StandardOutput].mShouldStoreResults = state;
}

bool AsyncProcess::GetStoreStandardErrorData()
{
  return mThreads[StreamType::StandardError].mShouldStoreResults;
}

void AsyncProcess::SetStoreStandardErrorData(bool state)
{
  mThreads[StreamType::StandardError].mShouldStoreResults = state;
}

void AsyncProcess::CloseInternal()
{
  // Drop a reference to standard input
  mStandardInput = nullptr;

  // Close all output thread/streams
  for(size_t i = 0; i < 2; ++i)
    mThreads[i].mFileStream.Close();
  // Wait for all of the threads to finish
  for(size_t i = 0; i < 2; ++i)
    mThreads[i].mThread.WaitForCompletion();
  
  // Clear our event list. Sending any buffered events could be problematic as all
  // of the threads are closed which could confuse users, especially during destruction.
  mEventDispatchList.ClearEvents();
}

void AsyncProcess::ResetEventConnections()
{
  // Stop listening to partial response events
  EventDispatcher* dispatcher = GetDispatcher();
  dispatcher->DisconnectEvent(Events::PartialStandardOutputResponse, this);
  dispatcher->DisconnectEvent(Events::PartialStandardErrorResponse, this);

  // Reconnect to the appropriate partial response events
  if(mThreads[StreamType::StandardOutput].mShouldStoreResults)
  {
    ConnectThisTo(this, Events::PartialStandardOutputResponse, OnPartialResponse);
  }
  if(mThreads[StreamType::StandardError].mShouldStoreResults)
  {
    ConnectThisTo(this, Events::PartialStandardErrorResponse, OnPartialResponse);
  }
}

void AsyncProcess::OnPartialResponse(AsyncProcessEvent* e)
{
  // Accumulate the results for the given stream type
  ThreadInfo& threadInfo = mThreads[e->mStreamType];
  threadInfo.mResults = BuildString(threadInfo.mResults, e->mBytes);
}
void AsyncProcess::OnFinished(AsyncProcessEvent* e)
{
  // Mark the thread as being done. We have all data from the stream now.
  mThreads[e->mStreamType].mIsRunning = false;
}

AsyncProcess::ThreadInfo::ThreadInfo()
{
  mIsRunning = false;
  mShouldStoreResults = true;
}

bool AsyncProcess::ThreadInfo::IsRunning()
{
  return mIsRunning;
}

void AsyncProcess::ThreadInfo::Reset()
{
  mIsRunning = false;
  mResults = "";
}

OsInt AsyncProcess::StandardOutputThreadFn(void* threadStartData)
{
  // Start reading from standard output
  AsyncProcess* asyncProcess = (AsyncProcess*)threadStartData;
  return RunThread(asyncProcess, StreamType::StandardOutput, Events::PartialStandardOutputResponse, Events::StandardOutputFinished);
}

OsInt AsyncProcess::StandardErrorThreadFn(void* threadStartData)
{
  // Start reading from standard error
  AsyncProcess* asyncProcess = (AsyncProcess*)threadStartData;
  return RunThread(asyncProcess, StreamType::StandardError, Events::PartialStandardErrorResponse, Events::StandardErrorFinished);
}

OsInt AsyncProcess::RunThread(AsyncProcess* asyncProcess, StreamType::Enum streamType, StringParam partialResponseEventName, StringParam finishedEventName)
{
  ThreadInfo& threadInfo = asyncProcess->mThreads[streamType];
  const size_t bufferSize = AsyncProcess::mReadBufferSize;
  byte buffer[bufferSize];
  Status status;

  // Keep reading as long as the process is running.
  while(threadInfo.mFileStream.IsOpen())
  {
    // Peek until there's no more data or the pipe failed (was closed)
    for(;;)
    {
      bool hasData = threadInfo.mFileStream.HasData(status);
      // Failure here typically means the pipe was closed by the main thread (destruction).
      if(status.Failed())
        return 0;
      if(hasData)
        break;
      Os::Sleep(10);
    }

    // Read up to the max buffer size.
    size_t bytesRead = threadInfo.mFileStream.Read(status, buffer, bufferSize);
    // There was no more data to read
    if(bytesRead == 0)
      break;

    // Send out the partial data as an event
    AsyncProcessEvent* toSend = new AsyncProcessEvent();
    toSend->mBytes = String((char*)buffer, bytesRead);
    toSend->mStreamType = streamType;
    asyncProcess->mEventDispatchList.Dispatch(asyncProcess, partialResponseEventName, toSend);
  }

  // The process has finished but we need to signal back to the main thread that there's no more
  // data to be read from the stream. We may have just sent data that will come back to AsyncProcess
  // the next time the main thread runs. To make sure this data has been received we send out another
  // event that will correctly signal that the main thread has all data for this stream.
  AsyncProcessEvent* finalEvent = new AsyncProcessEvent();
  finalEvent->mStreamType = streamType;
  asyncProcess->mEventDispatchList.Dispatch(asyncProcess, finishedEventName, finalEvent);
  return 0;
}

}// namespace Zero
