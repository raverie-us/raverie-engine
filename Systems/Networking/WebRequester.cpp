///////////////////////////////////////////////////////////////////////////////
///
/// \file TcpSocket.cpp
/// Implementation of the TcpSocket class.
///
/// Authors: Trevor Sundberg.
/// Copyright 2010-2014, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(WebResponseHeadersInternal);
  DefineEvent(WebResponsePartialDataInternal);
  DefineEvent(WebResponseCompleteInternal);
  DefineEvent(WebResponseHeaders);
  DefineEvent(WebResponsePartialData);
  DefineEvent(WebResponseComplete);
}

ZilchDefineType(AsyncWebRequest, builder, type)
{
  ZeroBindDocumented();
  ZeroBindEvent(Events::WebResponseHeaders, WebResponseEvent);
  ZeroBindEvent(Events::WebResponsePartialData, WebResponseEvent);
  ZeroBindEvent(Events::WebResponseComplete, WebResponseEvent);
  ZilchBindField(mUrl);
  ZilchBindMethod(Run);
  ZilchBindMethod(Cancel);
  ZilchBindMethod(Clear);
  ZilchBindMethod(AddHeader);
  ZilchBindMethod(AddFile);
  ZilchBindMethod(AddField);

  ZilchBindMethod(Create);

  ZilchBindFieldGetterProperty(mTotalDownloaded);
  ZilchBindFieldGetterProperty(mTotalExpected);
  ZilchBindFieldGetterProperty(mProgress);
  ZilchBindFieldGetterProperty(mProgressType);
}

//--------------------------------------------------------------------BlockingWebRequest
AsyncWebRequest* AsyncWebRequest::Create()
{
  return new AsyncWebRequest();
}

AsyncWebRequest::~AsyncWebRequest()
{
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

String AsyncWebRequest::GetData()
{
  return mStoredData.ToString();
}

void AsyncWebRequest::Run()
{
  // Cancel the existing request (this won't do anything if it's not running).
  Cancel();

  // Clear stored data and progress.
  mStoredData.Deallocate();
  mProgress = 0.0f;
  mProgressType = ProgressType::None;

  // This data is immutable while running the request. We should never modify it.
  mRequest.mUrl = mUrl;
  mRequest.mPostData = mPostData;
  mRequest.mAdditionalRequestHeaders = mAdditionalRequestHeaders;

  // Keep ourselves alive while we run the request.
  AddReference();

  mRequest.Run();
}

void AsyncWebRequest::Cancel()
{
  if (!mRequest.IsRunning())
    return;
  
  mRequest.Cancel();
  ++mVersion;

  // Since we were running a request, but are no longer then release our own reference.
  // This might destruct the class upon this call.
  Release();
}

void AsyncWebRequest::Clear()
{
  mResponseCode = WebResponseCode::NoServerResponse;
  mStoredData.Deallocate();
  mUrl.Clear();
  mAdditionalRequestHeaders.Clear();
  mResponseHeaders.Clear();
  mPostData.Clear();
  mTotalDownloaded = 0;
  mTotalExpected = 0;
  mProgress = 0.0f;
  mProgressType = ProgressType::None;
}

AsyncWebRequest::AsyncWebRequest() :
  mStoreData(true),
  mResponseCode(WebResponseCode::NoServerResponse),
  mTotalDownloaded(0),
  mTotalExpected(0),
  mProgress(0.0f),
  mProgressType(ProgressType::None),
  mVersion(0),
  mSendEventsOnRequestThread(false)
{
  mRequest.mUserData = this;
  mRequest.mOnHeadersReceived = &OnHeadersReceived;
  mRequest.mOnDataReceived = &OnDataReceived;
  mRequest.mOnComplete = &OnComplete;

  ConnectThisTo(this, Events::WebResponseHeadersInternal, OnWebResponseHeadersInternal);
  ConnectThisTo(this, Events::WebResponsePartialDataInternal, OnWebResponsePartialDataInternal);
  ConnectThisTo(this, Events::WebResponseCompleteInternal, OnWebResponseCompleteInternal);
}

void AsyncWebRequest::OnHeadersReceived(const Array<String>& headers, WebResponseCode::Enum code, WebRequest* request)
{
  // This can be called within a thread!
  AsyncWebRequest* self = (AsyncWebRequest*)request->mUserData;

  u64 totalBytesExpected = 0;
  forRange(StringParam header, headers)
  {
    static const String cContentLength("Content-Length: ");
    if (header.StartsWith(cContentLength))
    {
      cstr lengthStr = header.Data() + cContentLength.SizeInBytes();
      totalBytesExpected = (u64)atoll(lengthStr);
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
  // If the user cancelled the request, ignore any incoming old events by comparing versions.
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

void AsyncWebRequest::OnDataReceived(const byte* data, size_t size, WebRequest* request)
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

  if (self->mSendEventsOnRequestThread)
    self->DispatchEvent(Events::WebResponsePartialDataInternal, toSend);
  else
    Z::gDispatch->Dispatch(self, Events::WebResponsePartialDataInternal, toSend);
}

void AsyncWebRequest::OnWebResponsePartialDataInternal(WebResponseEvent* event)
{
  // If the user cancelled the request, ignore any incoming old events by comparing versions.
  if (event->mVersion != mVersion)
    return;

  event->mResponseCode = mResponseCode;
  event->mResponseHeaders = mResponseHeaders;

  mTotalDownloaded += event->mData.SizeInBytes();
  event->mTotalDownloaded = mTotalDownloaded;
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

  if (self->mSendEventsOnRequestThread)
    self->DispatchEvent(Events::WebResponseCompleteInternal, toSend);
  else
    Z::gDispatch->Dispatch(self, Events::WebResponseCompleteInternal, toSend);
}

void AsyncWebRequest::OnWebResponseCompleteInternal(WebResponseEvent* event)
{
  // If the user cancelled the request, ignore any incoming old events by comparing versions.
  if (event->mVersion != mVersion)
    return;

  event->mResponseCode = mResponseCode;
  event->mResponseHeaders = mResponseHeaders;

  event->mTotalDownloaded = mTotalDownloaded;
  event->mTotalExpected = mTotalExpected;

  mProgress = 1.0f;
  event->mProgressType = mProgressType;

  if (mStoreData)
    event->mData = mStoredData.ToString();

  DispatchEvent(Events::WebResponseComplete, event);

  // Since we completed the request, release our own reference.
  Release();
}

ZilchDefineType(WebResponseEvent, builder, type)
{
  ZilchBindFieldProperty(mResponseCode);
  ZilchBindFieldProperty(mData);
  ZilchBindGetterProperty(HeaderCount);
  ZilchBindMethod(GetHeader);
  ZilchBindFieldGetterProperty(mTotalDownloaded);
  ZilchBindFieldGetterProperty(mTotalExpected);
  ZilchBindFieldGetterProperty(mProgress);
  ZilchBindFieldGetterProperty(mProgressType);
}

WebResponseEvent::WebResponseEvent() :
  mResponseCode(WebResponseCode::NoServerResponse),
  mTotalDownloaded(0),
  mTotalExpected(0),
  mProgress(0.0f),
  mProgressType(ProgressType::None)
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

ZilchDefineType(WebRequester, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindEvent(Events::WebResponseHeaders, WebResponseEvent);
  ZeroBindEvent(Events::WebResponsePartialData, WebResponseEvent);
  ZeroBindEvent(Events::WebResponseComplete, WebResponseEvent);
  ZilchBindGetterSetterProperty(Url);
  ZilchBindMethod(Run);
  ZilchBindMethod(Clear);
  ZilchBindMethod(AddHeader);
  ZilchBindMethod(AddFile);
  ZilchBindMethod(AddField);
}

void WebRequester::Initialize(CogInitializer& initializer)
{
  ReplaceRequest();
}

void WebRequester::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("Url", mRequest->mUrl, String("http://www.w3.org/"));
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
}

void WebRequester::Clear()
{
  mRequest->Clear();
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
    request->mUrl = previousRequest->mUrl;
    request->mPostData = previousRequest->mPostData;
    request->mAdditionalRequestHeaders = previousRequest->mAdditionalRequestHeaders;
  }

  return previousRequestHandle;
}

void WebRequester::OnForwardEvent(WebResponseEvent* event)
{
  DispatchEvent(event->EventId, event);
}

} // namespace Zero
