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

// Undefining for Windows
#ifdef AddJob
#undef AddJob
#endif

// Defines for CURL
#define SKIP_PEER_VERIFICATION

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

//--------------------------------------------------------------------WebRequestInitializer
WebRequestInitializer::WebRequestInitializer()
{
  curl_global_init(CURL_GLOBAL_ALL);
}

WebRequestInitializer::~WebRequestInitializer()
{
  curl_global_cleanup();
}

//--------------------------------------------------------------------BlockingWebRequest
BlockingWebRequest::BlockingWebRequest() :
  mStoreData(true),
  mResponseCode(WebResponseCode::NoServerResponse),
  mFormPostFile(NULL),
  mLastPtr(NULL)
{
  mCanceled = false;
  mSendEvent = true;

  // Initialize a new curl state object
  CURL* curl = curl_easy_init();

  // If we got the curl object...
  if (curl)
  {
    // Store the curl handle
    mHandle = curl;

    // We always want to follow urls if they redirect
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnDataReceived);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

#ifdef SKIP_PEER_VERIFICATION
    // If you want to connect to a site who isn't using a certificate that is
    // signed by one of the certs in the CA bundle you have, you can skip the
    // verification of the server's certificate. This makes the connection
    // A LOT LESS SECURE.

    // If you have a CA cert for the server stored someplace else than in the
    // default bundle, then the CURLOPT_CAPATH option might come handy for
    // you.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
 
#ifdef SKIP_HOSTNAME_VERIFICATION
    // If the site you're connecting to uses a different host name that what
    // they have mentioned in their server certificate's commonName (or
    // subjectAltName) fields, libcurl will refuse to connect. You can skip
    // this check, but this will make the connection less secure.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
  }
  else
  {
    // We were unable to create the curl object!
    mHandle = NULL;
    DoNotifyError("WebRequest", "Unable to create the web request");
  }
}

BlockingWebRequest::~BlockingWebRequest()
{
  // As long as the curl handle is valid...
  if (mHandle != NULL)
  {
    // Cleanup the curl object
    curl_easy_cleanup((CURL*)mHandle);
  }
}

size_t BlockingWebRequest::OnHeaderReceived(char* data, size_t count, size_t elementSize, BlockingWebRequest* userdata)
{
  // This will always be a full header
  size_t size = count * elementSize;
  userdata->mResponseHeaders.PushBack(String(data, size));
  return size;
}

int BlockingWebRequest::OnDataReceived(char* data, size_t count, size_t elementSize, BlockingWebRequest* request)
{
  if(request->mCanceled)
    return 0;

  // Currently this occurs on the main thread, so its safe for us to just directly send data
  size_t size = count * elementSize;

  // Grab the curl object
  CURL* curl = (CURL*)request->mHandle;

  // Create a zero-style string out of the data
  String strData(data, size);

  // If the user wants us to store data...
  if (request->mStoreData)
  {
    // Add the response to our own stored data
    request->mStoredData.Append(strData);
  }
  
  // Send the web response event
  if(request->mSendEvent)
  {
    WebResponseEvent e;
    e.Data = strData;
    e.ResponseHeaders = request->mResponseHeaders;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &e.ResponseCode);
    request->DispatchEvent(Events::PartialWebResponse, &e);
  }
  
  // Return how much data we consumed
  return size;
}

String BlockingWebRequest::UrlParamEncode(StringParam string)
{
  StringBuilder builder;
  StringIterator it = string.Begin();
  StringIterator end = string.End();
  for (;it < end; ++it)
  {
    Rune rune = *it;

    if(IsAlphaNumeric(rune))
      builder << rune;
    else
      builder << String::Format("%%%02X", rune);
  }
  return builder.ToString();
}

Zero::String BlockingWebRequest::UrlParamDecode(StringParam string)
{
  StringBuilder builder;
  StringIterator it = string.Begin();
  StringIterator end = string.End();

  for (; it < end; ++it)
  {
    Rune rune = *it;
    // if the current rune is a % check for two following numbers
    if (rune == '%')
    {
      // make sure moving forward 2 characters is within the string
      if((it + 3) < end)
      {
        // create a substring of the next two characters should they both be hex numbers
        Rune nextRune1 = *(it + 1);
        Rune nextRune2 = *(it + 2);
        if (IsHex(nextRune1) && IsHex(nextRune2))
        {
          String encodedValue = string.SubString(it + 1, it + 3);
          uint value;
          ToValue(encodedValue, value, 16);
          // if the substring was a valid number take the encoded value
          // otherwise take the % as is
          if (value)
          {
            builder << Rune(value);
            it += 2;
            continue;
          }
        }
      }
    }
    // falls back on the runes value in all cases of it not being an encoded value
    builder << rune;
  }

  return builder.ToString();
}

void BlockingWebRequest::AddFile(StringParam fileName, StringParam formFieldName)
{
  curl_formadd((curl_httppost**)&mFormPostFile,
               (curl_httppost**)&mLastPtr,
               CURLFORM_COPYNAME, formFieldName.c_str(),
               CURLFORM_FILE, fileName.c_str(),
               CURLFORM_END);
}

void BlockingWebRequest::AddField(StringParam name, StringParam content)
{
  curl_formadd((curl_httppost**)&mFormPostFile, 
              (curl_httppost**)&mLastPtr, 
              CURLFORM_COPYNAME, name.c_str(),
              CURLFORM_COPYCONTENTS, content.c_str(), 
              CURLFORM_END);
}

void BlockingWebRequest::ClearPostData()
{
  // Grab the curl object
  CURL* curl = (CURL*)mHandle;

  // Free the post data and null out our handles
  curl_formfree((curl_httppost*)mFormPostFile);
  mFormPostFile = NULL;
  mLastPtr = NULL;

  // Make sure we don't post any data
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, NULL);
}

String BlockingWebRequest::Run()
{
  // Do nothing if we've already been canceled. This can happen if the job
  // is added to the job system, and is canceled before the execute function
  // is called
  if(mCanceled)
    return String();

  // Clear stored data
  mStoredData.Deallocate();

  // Skip out if the handle is not valid...
  if (mHandle == NULL)
    return String();

  // Grab the curl object
  CURL* curl = (CURL*)mHandle;

  // Any extra headers we add to the request (or override)
  curl_slist* headerList = NULL;

  // Set whether we're posting or not
  if (mFormPostFile)
  {
    // Set us into post mode
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, mFormPostFile);

    // We need to set a special header to not expect anything
    static const char expects[] = "Expect:";
    headerList = curl_slist_append(headerList, expects);

    // Set the header
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
  }

  if(!mPostData.Empty())
  {
     curl_easy_setopt(curl, CURLOPT_POST, true);

     for(uint i=0;i<mHeaders.Size();++i)
       headerList = curl_slist_append(headerList, mHeaders[i].c_str());

     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mPostData.c_str());

     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
  }

   curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, OnHeaderReceived);
   curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

  // Set the url we'll be requesting to
  curl_easy_setopt(curl, CURLOPT_URL, mUrl.c_str());

  // Perform the request and grab the return code
  auto result = curl_easy_perform(curl);

  // Release the header list back to CURL
  if (headerList)
    curl_slist_free_all(headerList);

  // Get the resulting full data (only works when mStoreData is set!)
  auto strData = mStoredData.ToString();

  uint responseCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
  mResponseCode = responseCode; 

  if(mSendEvent)
  {
    // Send the web response event
    WebResponseEvent responseEvent;
    responseEvent.Data = strData;
    responseEvent.ResponseHeaders.Swap(mResponseHeaders);
    responseEvent.ResponseCode = responseCode;

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

void BlockingWebRequest::ClearData()
{
  mStoredData.Deallocate();
  mHeaders.Clear();
  mResponseHeaders.Clear();
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
  ZilchBindMethod(SetHeader);
  ZilchBindMethod(SetPostData);
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
  mRequest.mHeaders.Clear();
  mRequest.mPostData = String();
}

void WebRequester::SetHeader(StringParam name, StringParam data)
{
  String headerString =  BuildString(name, ":", data);
  mRequest.mHeaders.PushBack(headerString);
}

void WebRequester::SetPostData(StringParam data)
{
  mRequest.mPostData = data;
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
  Z::gDispatch->Dispatch(ThreadContext::Main, mOrigin, 
    Events::PartialWebResponse, new WebResponseEvent(*event));
}

void WebRequestJob::OnWebResponse(WebResponseEvent* event)
{
  Z::gDispatch->Dispatch(ThreadContext::Main, mOrigin, 
    Events::WebResponse, new WebResponseEvent(*event));
}

} // namespace Zero
