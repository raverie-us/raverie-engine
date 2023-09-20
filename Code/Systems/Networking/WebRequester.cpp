// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(WebResponseHeadersInternal);
DefineEvent(WebResponsePartialDataInternal);
DefineEvent(WebResponseCompleteInternal);
DefineEvent(WebResponseHeaders);
DefineEvent(WebResponsePartialData);
DefineEvent(WebResponseComplete);
} // namespace Events

static const String cCacheDirectory("ZeroCache");

RaverieDefineType(AsyncWebRequest, builder, type)
{
  RaverieBindDocumented();
  RaverieBindEvent(Events::WebResponseHeaders, WebResponseEvent);
  RaverieBindEvent(Events::WebResponsePartialData, WebResponseEvent);
  RaverieBindEvent(Events::WebResponseComplete, WebResponseEvent);
  RaverieBindField(mUrl);
  RaverieBindMethod(Run);
  RaverieBindMethod(Cancel);
  RaverieBindMethod(ClearAll);
  RaverieBindMethod(ClearRequestData);
  RaverieBindMethod(ClearResponseData);
  RaverieBindMethod(AddHeader);
  RaverieBindMethod(AddFile);
  RaverieBindMethod(AddField);
  RaverieBindGetterProperty(IsRunning);

  RaverieBindMethod(Create);

  RaverieBindFieldGetterProperty(mStoreData);

  RaverieBindFieldGetterProperty(mTotalDownloaded);
  RaverieBindFieldGetterProperty(mTotalExpected);
  RaverieBindFieldGetterProperty(mProgress);
  RaverieBindFieldGetterProperty(mProgressType);
  RaverieBindFieldGetterProperty(mError);
}

InList<AsyncWebRequest> AsyncWebRequest::mActiveRequests;
ThreadLock AsyncWebRequest::mActiveRequestsLock;

AsyncWebRequest* AsyncWebRequest::Create()
{
  return new AsyncWebRequest();
}

AsyncWebRequest::~AsyncWebRequest()
{
  mActiveRequestsLock.Lock();
  mActiveRequests.Erase(this);
  mActiveRequestsLock.Unlock();
}

void AsyncWebRequest::AddHeader(StringParam name, StringParam data)
{
  String headerString = BuildString(name, ": ", data);
  mAdditionalRequestHeaders.PushBack(headerString);
}

void AsyncWebRequest::AddFile(StringParam formFieldName, StringParam fileName)
{
  WebPostData& data = mPostData.PushBack();
  data.mName = formFieldName;

  size_t fileSize = 0;
  byte* fileMemory = ReadFileIntoMemory(fileName.c_str(), fileSize);
  data.mValue.SetData(fileMemory, fileSize, true);
  data.mFileName = fileName;
}

void AsyncWebRequest::AddField(StringParam formFieldName, StringParam content)
{
  WebPostData& data = mPostData.PushBack();
  data.mName = formFieldName;

  byte* copy = (byte*)zAllocate(content.SizeInBytes());
  memcpy(copy, content.Data(), content.SizeInBytes());
  data.mValue.SetData(copy, content.SizeInBytes(), true);
}

void AsyncWebRequest::CancelAllActiveRequests()
{
  // Make a copy of the list so we don't need to worry about
  // the destructor locking the list.
  Array<HandleOf<AsyncWebRequest>> requestsToCancel;
  mActiveRequestsLock.Lock();
  forRange (AsyncWebRequest& request, mActiveRequests)
    requestsToCancel.PushBack(&request);
  mActiveRequestsLock.Unlock();

  // Iterate through the list and cancel each request.
  forRange (HandleOf<AsyncWebRequest>& request, requestsToCancel)
    request->Cancel();

  // Destructing the temporary array should release the last handles to the
  // requests. If any others are still being held onto, they at least should not
  // be running.
}

String AsyncWebRequest::GetData()
{
  return mStoredData.ToString();
}

String GetCacheFile(StringParam url)
{
  // Make sure the URL is encoded to a valid file-name.
  // We don't use post data or request headers here.
  String fileName = UrlParamEncode(url);
  String cacheDirectory = FilePath::Combine(GetTemporaryDirectory(), cCacheDirectory);
  CreateDirectoryAndParents(cacheDirectory);
  String cacheFile = FilePath::Combine(cacheDirectory, fileName);
  return cacheFile;
}

void AsyncWebRequest::Run()
{
  // Cancel the existing request (this won't do anything if it's not running).
  Cancel();

  // Clear stored data and progress.
  ClearResponseData();

  // If we allow caching then we need to check if we already cached this
  // file/url...
  if (mForceCacheSeconds != 0)
  {
    // If we successfully loaded a file from the cache, then send the completed
    // event and return.
    if (SendCompletedCacheResponse(false))
      return;
  }

  // This data is immutable while running the request. We should never modify
  // it.
  mRequest.mUrl = mUrl;
  mRequest.mPostData = mPostData;
  mRequest.mAdditionalRequestHeaders = mAdditionalRequestHeaders;

  mRequest.Run();
}

bool AsyncWebRequest::GetIsRunning()
{
  return mRequest.IsRunning();
}

void AsyncWebRequest::Cancel()
{
  if (!mRequest.IsRunning())
    return;

  mRequest.Cancel();
  ++mVersion;
}

void AsyncWebRequest::ClearAll()
{
  ClearRequestData();
  ClearResponseData();
}

void AsyncWebRequest::ClearRequestData()
{
  mUrl.Clear();
  mAdditionalRequestHeaders.Clear();
  mPostData.Clear();
}

void AsyncWebRequest::ClearResponseData()
{
  mResponseCode = WebResponseCode::NoServerResponse;
  mStoredData.Deallocate();
  mResponseHeaders.Clear();
  mTotalDownloaded = 0;
  mTotalExpected = 0;
  mProgress = 0.0f;
  mProgressType = ProgressType::None;
  mError = String();
}

AsyncWebRequest::AsyncWebRequest() :
    mStoreData(true),
    mResponseCode(WebResponseCode::NoServerResponse),
    mTotalDownloaded(0),
    mTotalExpected(0),
    mProgress(0.0f),
    mProgressType(ProgressType::None),
    mVersion(0),
    mSendEventsOnRequestThread(false),
    mForceCacheSeconds(0)
{
  mActiveRequestsLock.Lock();
  mActiveRequests.PushBack(this);
  mActiveRequestsLock.Unlock();

  mRequest.mUserData = this;
  mRequest.mOnHeadersReceived = &OnHeadersReceived;
  mRequest.mOnDataReceived = &OnDataReceived;
  mRequest.mOnComplete = &OnComplete;

  ConnectThisTo(this, Events::WebResponseHeadersInternal, OnWebResponseHeadersInternal);
  ConnectThisTo(this, Events::WebResponsePartialDataInternal, OnWebResponsePartialDataInternal);
  ConnectThisTo(this, Events::WebResponseCompleteInternal, OnWebResponseCompleteInternal);
}

bool AsyncWebRequest::SendCompletedCacheResponse(bool ignoreTime)
{
  String cacheFile = GetCacheFile(mUrl);

  if (FileExists(cacheFile))
  {
    // If we're withing the forced caching time...
    s64 now = (s64)Time::GetTime();
    s64 modifiedTime = (s64)GetFileModifiedTime(cacheFile);
    s64 timeSinceModification = now - modifiedTime;
    bool isWithinCacheTime = timeSinceModification <= (s64)mForceCacheSeconds;
    if (ignoreTime || isWithinCacheTime)
    {
      // Attempt to read the file contents.
      DataBlock block = ReadFileIntoDataBlock(cacheFile.c_str());
      if (block.Data)
      {
        // Set all the event data.
        WebResponseEvent* toSend = new WebResponseEvent();
        toSend->mAsyncWebRequest = this;
        toSend->mVersion = mVersion;
        toSend->mTotalExpected = block.Size;
        toSend->mTotalDownloaded = block.Size;
        toSend->mProgress = 1.0f;
        toSend->mProgressType = ProgressType::Normal;
        toSend->mResponseCode = WebResponseCode::OK;
        toSend->mData = String((cstr)block.Data, block.Size);

        // Set all of our data.
        mResponseCode = WebResponseCode::OK;
        mTotalDownloaded = block.Size;
        mTotalExpected = block.Size;
        mProgress = 1.0f;
        mProgressType = ProgressType::Normal;
        mError = String();
        if (mStoreData)
          mStoredData.Append(toSend->mData);

        DispatchEvent(Events::WebResponseComplete, toSend);
        return true;
      }
    }
  }

  return false;
}

void AsyncWebRequest::OnHeadersReceived(const Array<String>& headers, WebResponseCode::Enum code, WebRequest* request)
{
  // This can be called within a thread!
  AsyncWebRequest* self = (AsyncWebRequest*)request->mUserData;

  Matches matches;
  u64 totalBytesExpected = 0;
  forRange (StringParam header, headers)
  {
    // Create a case insensitive regex to search for content length.
    static const Regex cContentLengthRegex("content-length:\\s*([0-9]*)\\s*", RegexFlavor::EcmaScript, false);

    cContentLengthRegex.Search(header, matches);
    if (matches.Size() == 2)
    {
      totalBytesExpected = (u64)atoll(matches[1].mBegin);
      break;
    }
  }

  WebResponseEvent* toSend = new WebResponseEvent();
  toSend->mAsyncWebRequest = self;
  toSend->mVersion = self->mVersion;
  toSend->mTotalExpected = totalBytesExpected;
  toSend->mProgressType = totalBytesExpected ? ProgressType::Normal : ProgressType::Indeterminate;
  toSend->mResponseCode = code;
  toSend->mResponseHeaders = headers;

  if (self->mSendEventsOnRequestThread)
    self->DispatchEvent(Events::WebResponseHeadersInternal, toSend);
  else
    Z::gDispatch->Dispatch(self, Events::WebResponseHeadersInternal, toSend);
}

void AsyncWebRequest::OnWebResponseHeadersInternal(WebResponseEvent* event)
{
  // If the user cancelled the request, ignore any incoming old events by
  // comparing versions.
  if (event->mVersion != mVersion)
    return;

  mResponseCode = event->mResponseCode;
  mResponseHeaders = event->mResponseHeaders;

  mTotalDownloaded = 0;
  mTotalExpected = event->mTotalExpected;

  mProgress = 0.0f;
  mProgressType = event->mProgressType;

  DispatchEvent(Events::WebResponseHeaders, event);
}

void AsyncWebRequest::OnDataReceived(const byte* data, size_t size, u64 totalDownloaded, WebRequest* request)
{
  // This can be called within a thread!
  AsyncWebRequest* self = (AsyncWebRequest*)request->mUserData;

  // Create a zero-style string out of the data
  String strData((const char*)data, size);

  // The main thread will fill out the response headers and request code.
  WebResponseEvent* toSend = new WebResponseEvent();
  toSend->mAsyncWebRequest = self;
  toSend->mVersion = self->mVersion;
  toSend->mData = strData;

  // We always get total downloaded from the callback because in some
  // implementations (Emscripten) we don't support partial binary downloads
  // (only get the entire buffer at the end) but it does at least give us the
  // total downloaded amount.
  toSend->mTotalDownloaded = totalDownloaded;

  if (self->mSendEventsOnRequestThread)
    self->DispatchEvent(Events::WebResponsePartialDataInternal, toSend);
  else
    Z::gDispatch->Dispatch(self, Events::WebResponsePartialDataInternal, toSend);
}

void AsyncWebRequest::OnWebResponsePartialDataInternal(WebResponseEvent* event)
{
  // If the user cancelled the request, ignore any incoming old events by
  // comparing versions.
  if (event->mVersion != mVersion)
    return;

  event->mResponseCode = mResponseCode;
  event->mResponseHeaders = mResponseHeaders;

  // We can't just accumulate mData.SizeInBytes() because we may not actually
  // get partial data on some platforms (however we are guaranteed to get the
  // total downloaded amount as a number on all platforms).
  mTotalDownloaded = event->mTotalDownloaded;
  event->mTotalExpected = mTotalExpected;

  mProgress = mTotalExpected ? (float)(mTotalDownloaded / (double)mTotalExpected) : 0.0f;
  event->mProgress = mProgress;
  event->mProgressType = mProgressType;

  if (mStoreData)
    mStoredData.Append(event->mData);

  DispatchEvent(Events::WebResponsePartialData, event);
}

void AsyncWebRequest::OnComplete(Status& status, WebRequest* request)
{
  // This can be called within a thread!
  AsyncWebRequest* self = (AsyncWebRequest*)request->mUserData;

  // The main thread will fill out everything else.
  WebResponseEvent* toSend = new WebResponseEvent();
  toSend->mAsyncWebRequest = self;
  toSend->mVersion = self->mVersion;
  toSend->mProgress = 1.0f;
  toSend->mError = status.Message;

  if (self->mSendEventsOnRequestThread)
    self->DispatchEvent(Events::WebResponseCompleteInternal, toSend);
  else
    Z::gDispatch->Dispatch(self, Events::WebResponseCompleteInternal, toSend);
}

void AsyncWebRequest::OnWebResponseCompleteInternal(WebResponseEvent* event)
{
  // If the user cancelled the request, ignore any incoming old events by
  // comparing versions.
  if (event->mVersion != mVersion)
    return;

  event->mResponseCode = mResponseCode;
  event->mResponseHeaders = mResponseHeaders;

  event->mTotalDownloaded = mTotalDownloaded;
  event->mTotalExpected = mTotalExpected;

  mProgress = 1.0f;
  event->mProgressType = mProgressType;

  mError = event->mError;

  if (mStoreData)
  {
    event->mData = mStoredData.ToString();

    // If the request came in OK from the server with no errors...
    if (event->mResponseCode == WebResponseCode::OK && event->mError.Empty())
    {
      // If we allow caching then we need to check if we already cached this
      // file/url...
      if (mForceCacheSeconds != 0)
      {
        String cacheFile = GetCacheFile(mUrl);
        WriteStringRangeToFile(cacheFile, event->mData);
      }
    }
    else
    {
      // We failed the request, but if we have caching enabled then attempt to
      // load the file from the cache and send it as a complete event (even
      // though the request failed).
      if (mForceCacheSeconds != 0)
      {
        // Early out, since we don't want to send the actual event.
        if (SendCompletedCacheResponse(true))
          return;
      }
    }
  }

  DispatchEvent(Events::WebResponseComplete, event);
}

RaverieDefineType(WebResponseEvent, builder, type)
{
  RaverieBindField(mAsyncWebRequest);
  RaverieBindFieldProperty(mResponseCode);
  RaverieBindFieldProperty(mData);
  RaverieBindGetterProperty(HeaderCount);
  RaverieBindMethod(GetHeader);
  RaverieBindFieldGetterProperty(mTotalDownloaded);
  RaverieBindFieldGetterProperty(mTotalExpected);
  RaverieBindFieldGetterProperty(mProgress);
  RaverieBindFieldGetterProperty(mProgressType);
  RaverieBindFieldGetterProperty(mError);
}

WebResponseEvent::WebResponseEvent() : mResponseCode(WebResponseCode::NoServerResponse), mTotalDownloaded(0), mTotalExpected(0), mProgress(0.0f), mProgressType(ProgressType::None)
{
}

StringParam WebResponseEvent::GetHeader(uint index)
{
  if (index >= mResponseHeaders.Size())
    DoNotifyException("WebResponseEvent", "Header index was out of bounds");

  return mResponseHeaders[index];
}

uint WebResponseEvent::GetHeaderCount()
{
  return (uint)mResponseHeaders.Size();
}

JsonValue* WebResponseEvent::ReadJson(Status& status)
{
  if (mData.Empty())
  {
    status.SetFailed("Data was empty");
    return nullptr;
  }

  static const String cOrigin = "WebResponseEvent";
  CompilationErrors errors;
  JsonValue* value = Raverie::JsonReader::ReadIntoTreeFromString(errors, mData, cOrigin, nullptr);
  if (!value)
  {
    status.SetFailed("Failed to parse JSON");
    return nullptr;
  }

  return value;
}

RaverieDefineType(WebRequester, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
  RaverieBindEvent(Events::WebResponseHeaders, WebResponseEvent);
  RaverieBindEvent(Events::WebResponsePartialData, WebResponseEvent);
  RaverieBindEvent(Events::WebResponseComplete, WebResponseEvent);
  RaverieBindGetterSetterProperty(Url);
  RaverieBindMethod(Run);
  RaverieBindMethod(Clear);
  RaverieBindMethod(CancelActiveRequests);
  RaverieBindMethod(AddHeader);
  RaverieBindMethod(AddFile);
  RaverieBindMethod(AddField);

  RaverieBindMemberProperty(mCancelOnDestruction);
}

WebRequester::WebRequester() : mCancelOnDestruction(true)
{
}

WebRequester::~WebRequester()
{
  if (mCancelOnDestruction)
    CancelActiveRequests();
}

void WebRequester::Initialize(CogInitializer& initializer)
{
  ReplaceRequest();
  mRequest->mUrl = mSerializedUrl;
}

void WebRequester::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("Url", mSerializedUrl, String(Urls::cUserWebRequesterDefault));
  SerializeNameDefault(mCancelOnDestruction, true);
}

HandleOf<AsyncWebRequest> WebRequester::Run()
{
  // Run the request and create a new one in it's place.
  // When the request runs, we'll get events about its status.
  mRequest->Run();
  return ReplaceRequest();
}

String WebRequester::GetUrl()
{
  return mRequest->mUrl;
}

void WebRequester::SetUrl(StringParam url)
{
  mRequest->mUrl = url;
  mSerializedUrl = url;
}

void WebRequester::Clear()
{
  mRequest->ClearAll();
}

void WebRequester::CancelActiveRequests()
{
  // Cancel all active web requests.
  forRange (HandleOf<AsyncWebRequest>& request, mActiveRequests)
    request->Cancel();
  mActiveRequests.Clear();
}

void WebRequester::AddHeader(StringParam name, StringParam data)
{
  mRequest->AddHeader(name, data);
}

void WebRequester::AddFile(StringParam fileName, StringParam formFieldName)
{
  mRequest->AddFile(formFieldName, fileName);
}

void WebRequester::AddField(StringParam formFieldName, StringParam content)
{
  mRequest->AddField(formFieldName, content);
}

void WebRequester::OnForwardEvent(WebResponseEvent* event)
{
  DispatchEvent(event->EventId, event);

  if (event->EventId == Events::WebResponseComplete)
    CleanDeadRequests();
}

HandleOf<AsyncWebRequest> WebRequester::ReplaceRequest()
{
  HandleOf<AsyncWebRequest> previousRequestHandle = mRequest;
  AsyncWebRequest* previousRequest = previousRequestHandle;

  mRequest = AsyncWebRequest::Create();
  AsyncWebRequest* request = mRequest;

  ConnectThisTo(request, Events::WebResponseHeaders, OnForwardEvent);
  ConnectThisTo(request, Events::WebResponsePartialData, OnForwardEvent);
  ConnectThisTo(request, Events::WebResponseComplete, OnForwardEvent);

  // If we had a request previously, then copy the old data over.
  if (previousRequest)
  {
    // We only need to keep the previous requests alive (the current is kept
    // alive by mRequest).
    mActiveRequests.PushBack(previousRequestHandle);

    request->mUrl = previousRequest->mUrl;
    request->mPostData = previousRequest->mPostData;
    request->mAdditionalRequestHeaders = previousRequest->mAdditionalRequestHeaders;
  }

  return previousRequestHandle;
}

void WebRequester::CleanDeadRequests()
{
  for (size_t i = 0; i < mActiveRequests.Size();)
  {
    HandleOf<AsyncWebRequest>& request = mActiveRequests[i];
    if (!request->GetIsRunning())
      mActiveRequests.EraseAt(i);
    else
      ++i;
  }
}

} // namespace Raverie
