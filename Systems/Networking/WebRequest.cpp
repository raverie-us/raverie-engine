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
  DefineEvent(PartialWebResponse);
  DefineEvent(WebResponse);
}

ZilchDefineType(BlockingWebRequest, builder, type)
{
  ZilchBindFieldProperty(mUrl);
}

//--------------------------------------------------------------------BlockingWebRequest
BlockingWebRequest::BlockingWebRequest() :
  mStoreData(true),
  mSendEvent(true),
  mResponseCode(Os::WebResponseCode::NoServerResponse),
  mCanceled(false)
{
}

BlockingWebRequest::~BlockingWebRequest()
{
}

void BlockingWebRequest::OnHeadersReceived(const Array<String>& headers, Os::WebResponseCode::Enum code, void* userData)
{
  BlockingWebRequest* self = (BlockingWebRequest*)userData;

  if (self->mCanceled)
    return;

  self->mResponseHeaders = headers;
  self->mResponseCode = code;
}

void BlockingWebRequest::OnDataReceived(const byte* data, size_t size, void* userData)
{
  BlockingWebRequest* self = (BlockingWebRequest*)userData;

  if(self->mCanceled)
    return;

  // Create a zero-style string out of the data
  String strData((const char*)data, size);

  // If the user wants us to store data...
  if (self->mStoreData)
  {
    // Add the response to our own stored data
    self->mStoredData.Append(strData);
  }
  
  // Send the web response event
  if(self->mSendEvent)
  {
    WebResponseEvent e;
    e.Data = strData;
    e.ResponseHeaders = self->mResponseHeaders;
    e.ResponseCode = self->mResponseCode;
    self->DispatchEvent(Events::PartialWebResponse, &e);
  }
}

void BlockingWebRequest::AddFile(StringParam formFieldName, StringParam fileName)
{
  Os::WebPostData& data = mPosts.PushBack();
  data.mName = formFieldName;

  size_t fileSize = 0;
  byte* fileMemory = ReadFileIntoMemory(fileName.c_str(), fileSize);
  data.mValue.SetData(fileMemory, fileSize, true);
  data.mFileName = fileName;
}

void BlockingWebRequest::AddField(StringParam formFieldName, StringParam content)
{
  Os::WebPostData& data = mPosts.PushBack();
  data.mName = formFieldName;

  byte* copy = (byte*)zAllocate(content.SizeInBytes());
  memcpy(copy, content.Data(), content.SizeInBytes());
  data.mValue.SetData(copy, content.SizeInBytes(), true);
}

String BlockingWebRequest::Run()
{
  // Do nothing if we've already been canceled. This can happen if the job
  // is added to the job system, and is canceled before the execute function
  // is called
  if (mCanceled)
    return String();

  // Clear stored data
  mStoredData.Deallocate();

  Status status;
  Os::WebRequest(
    status,
    mUrl,
    mPosts,
    mHeaders,
    &BlockingWebRequest::OnHeadersReceived,
    &BlockingWebRequest::OnDataReceived,
    this);

  if (status.Failed())
  {
    DoNotifyException("Web Request", status.Message);
    return String();
  }

  // Get the resulting full data (only works when mStoreData is set!)
  auto strData = mStoredData.ToString();

  if(mSendEvent)
  {
    // Send the web response event
    WebResponseEvent responseEvent;
    responseEvent.Data = strData;
    responseEvent.ResponseHeaders.Swap(mResponseHeaders);
    responseEvent.ResponseCode = mResponseCode;

    if(!mCanceled)
      this->DispatchEvent(Events::WebResponse, &responseEvent);
  }

  // Return the stored data
  return strData;
}

void BlockingWebRequest::Cancel()
{
  mCanceled = true;
}

void BlockingWebRequest::Clear()
{
  mStoredData.Deallocate();
  mHeaders.Clear();
  mResponseHeaders.Clear();
  mPosts.Clear();
  mCanceled = false;
}

ZilchDefineType(ThreadedWebRequest, builder, type)
{
}

ThreadedWebRequest::ThreadedWebRequest()
{
  mJob = NULL;
}

ThreadedWebRequest::~ThreadedWebRequest()
{
  if(mJob)
  {
    mJob->mWebRequest.Cancel();
    mJob->mOsEvent->Wait();
    delete mJob;
  }
}

void ThreadedWebRequest::Run()
{
  mJob = new WebRequestJob();
  mJob->mWebRequest.mUrl = this->mUrl;
  mJob->mOrigin = this;
  mJob->mDeletedOnCompletion = false;
  // We need to wait on the job to finish before destructing ourself
  mJob->InitializeOsEvent();
  Z::gJobs->AddJob(mJob);
}

void ThreadedWebRequest::Cancel()
{
  if(mJob)
    mJob->mWebRequest.Cancel();
}

ZilchDefineType(WebResponseEvent, builder, type)
{
  ZilchBindFieldProperty(ResponseCode);
  ZilchBindFieldProperty(Data);
}

ZilchDefineType(WebRequester, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZilchBindGetterSetterProperty(Url);
  ZeroBindEvent(Events::WebResponse, WebResponseEvent);
  ZilchBindMethod(Clear);
  ZilchBindMethod(Run);
  ZilchBindMethod(AddHeader);
  ZilchBindMethod(AddFile);
  ZilchBindMethod(AddField);
}

void WebRequester::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnWebResponse);
}

void WebRequester::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("Url", mRequest.mUrl, String("http://www.w3.org/"));
}

void WebRequester::Run()
{
  mRequest.Run();
}

String WebRequester::GetUrl()
{
  return mRequest.mUrl;
}

void WebRequester::SetUrl(StringParam url)
{
  mRequest.mUrl = url;
}

void WebRequester::Clear()
{
  mRequest.Clear();
}

void WebRequester::AddHeader(StringParam name, StringParam data)
{
  String headerString =  BuildString(name, ":", data);
  mRequest.mHeaders.PushBack(headerString);
}

void WebRequester::AddFile(StringParam fileName, StringParam formFieldName)
{
  mRequest.AddFile(formFieldName, fileName);
}

void WebRequester::AddField(StringParam formFieldName, StringParam content)
{
  mRequest.AddField(formFieldName, content);
}

void WebRequester::OnWebResponse(WebResponseEvent* event)
{
  // Just forward the data to our own object
  this->GetOwner()->DispatchEvent(Events::WebResponse, event);
}

ZilchDefineType(WebRequestJob, builder, type)
{
}

int WebRequestJob::Execute()
{
  ConnectThisTo(&mWebRequest, Events::PartialWebResponse, OnPartialWebResponse);
  ConnectThisTo(&mWebRequest, Events::WebResponse, OnWebResponse);

  mWebRequest.Run();
  return true;
}

void WebRequestJob::OnPartialWebResponse(WebResponseEvent* event)
{
  Z::gDispatch->Dispatch(mOrigin, 
    Events::PartialWebResponse, new WebResponseEvent(*event));
}

void WebRequestJob::OnWebResponse(WebResponseEvent* event)
{
  Z::gDispatch->Dispatch(mOrigin, 
    Events::WebResponse, new WebResponseEvent(*event));
}

} // namespace Zero
