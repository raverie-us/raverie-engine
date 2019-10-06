// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include <curl/curl.h>

// Defines for CURL
#define SKIP_PEER_VERIFICATION
#define SKIP_HOSTNAME_VERIFICATION

namespace Zero
{
class WebRequestSharedData
{
public:
  WebRequestSharedData() : mCurl(nullptr), mTotalDownloaded(0), mCalledReceivedHeaders(false)
  {
  }

  // Things from the main thread (copied).
  String mUrl;
  Array<WebPostData> mPostData;
  Array<String> mAdditionalRequestHeaders;
  String mPostDataWithBoundaries;

  // Things set by the thread.
  CURL* mCurl;
  u64 mTotalDownloaded;
  Array<String> mHeaders;
  bool mCalledReceivedHeaders;

  ThreadLock mRequestLock;
  WebRequest* mRequest;
};

typedef std::shared_ptr<WebRequestSharedData> WebRequestSharedPointer;

class WebRequestPrivateData
{
public:
  Thread mThread;
  WebRequestSharedPointer mShared;
};

static size_t OnWebRequestHeaderReceived(char* data, size_t size, size_t nitems, void* userData)
{
  WebRequestSharedData* shared = (WebRequestSharedData*)userData;
  CURL*& curl = shared->mCurl;

  size_t totalSize = size * nitems;

  // Split the headers by the standard HTTP \r\n.
  String strData(data, totalSize);

  // Ignore the HTTP line in the headers (it's a header, but not a Name:Value
  // header).
  if (strData.StartsWith("HTTP/"))
    return totalSize;

  // There should only actually be one header line per callback,
  // but we split because it's the easiest thing to do.
  static const String cHttpNewline("\r\n");
  Array<String> headers;
  forRange (StringRange singleHeader, strData.Split(cHttpNewline))
    shared->mHeaders.PushBack(singleHeader);

  return totalSize;
}

static size_t OnWebRequestDataReceived(char* data, size_t size, size_t nmemb, void* userData)
{
  WebRequestSharedData* shared = (WebRequestSharedData*)userData;
  CURL*& curl = shared->mCurl;

  if (!shared->mCalledReceivedHeaders)
  {
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    {
      Lock lock(shared->mRequestLock);
      WebRequest* request = shared->mRequest;
      if (!request)
        return 0;
      if (request->mOnHeadersReceived)
        request->mOnHeadersReceived(shared->mHeaders, (WebResponseCode::Enum)responseCode, request);
    }

    shared->mCalledReceivedHeaders = true;
  }

  size_t totalSize = size * nmemb;

  shared->mTotalDownloaded += (u64)totalSize;

  {
    Lock lock(shared->mRequestLock);
    WebRequest* request = shared->mRequest;
    if (!request)
      return 0;
    if (request->mOnDataReceived)
      request->mOnDataReceived((byte*)data, totalSize, shared->mTotalDownloaded, request);
  }

  // Return how much data we consumed
  return totalSize;
}

OsInt WebRequestThread(void* userData)
{
  // Keep a the shared pointer on the stack to ensure it's alive.
  std::shared_ptr<WebRequestSharedData> shared = *(WebRequestSharedPointer*)userData;
  delete (WebRequestSharedPointer*)userData;
  shared->mTotalDownloaded = 0;
  shared->mHeaders.Clear();
  shared->mCalledReceivedHeaders = false;

  // Initialize a new curl state object
  CURL*& curl = shared->mCurl;
  curl = curl_easy_init();

  Status status;

  // If we couldn't create the curl object...
  if (!curl)
  {
    Lock lock(shared->mRequestLock);
    WebRequest* request = shared->mRequest;
    if (!request)
      return 0;
    if (request->mOnComplete)
    {
      status.SetFailed("Unable to create the curl object");
      request->mOnComplete(status, request);
    }
    return -1;
  }


  // Keep this alive for the entire curl call.
  String userAgent = WebRequest::GetUserAgent();

  // We always want to follow urls if they redirect
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &OnWebRequestHeaderReceived);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, shared.get());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &OnWebRequestDataReceived);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, shared.get());
  curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
  curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);

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
  // subjectAltName) fields, curl will refuse to connect. You can skip
  // this check, but this will make the connection less secure.
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

  // Any extra headers we add to the request (or override)
  curl_slist* headerList = nullptr;

  forRange (StringParam header, shared->mAdditionalRequestHeaders)
    headerList = curl_slist_append(headerList, header.c_str());

  if (!shared->mPostData.Empty())
  {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, shared->mPostDataWithBoundaries.c_str());

    // Set us into post mode
    curl_easy_setopt(curl, CURLOPT_POST, true);

    // We need to set a special header to not expect anything
    static cstr cExpects = "Expect:";
    headerList = curl_slist_append(headerList, cExpects);
  }

  // Set the headers that we send.
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

  // Set the url we'll be requesting to.
  curl_easy_setopt(curl, CURLOPT_URL, shared->mUrl.c_str());

  char errorBuffer[CURL_ERROR_SIZE];
  errorBuffer[0] = '\0';
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

  // Perform the request and grab the return code (callbacks occur at this
  // time).
  CURLcode result = curl_easy_perform(curl);
  if (result != CURLE_OK)
  {
    if (strlen(errorBuffer) != 0)
      status.SetFailed(errorBuffer);
    else
      status.SetFailed(String::Format("Curl error %d: %s", (int)result, curl_easy_strerror(result)));
  }


  // Release the header list back to CURL
  if (headerList)
    curl_slist_free_all(headerList);

  curl_easy_cleanup(curl);

  {
    Lock lock(shared->mRequestLock);
    WebRequest* request = shared->mRequest;
    if (!request)
      return 0;
    if (request->mOnComplete)
      request->mOnComplete(status, request);
  }

  return 0;
}

WebRequest::WebRequest() :
    mOnHeadersReceived(nullptr),
    mOnDataReceived(nullptr),
    mOnComplete(nullptr),
    mUserData(nullptr)
{
  ZeroConstructPrivateData(WebRequestPrivateData);
}

WebRequest::~WebRequest()
{
  ZeroGetPrivateData(WebRequestPrivateData);

  // Cancel any active requests.
  Cancel();

  // Wait for the thread to complete before we destruct ourselves.
  if (self->mThread.IsValid())
    self->mThread.WaitForCompletion();

  ZeroDestructPrivateData(WebRequestPrivateData);
}

void WebRequest::Run()
{
  ZeroGetPrivateData(WebRequestPrivateData);
  auto shared = std::make_shared<WebRequestSharedData>();
  shared->mUrl = mUrl;
  shared->mPostData = mPostData;
  shared->mAdditionalRequestHeaders = mAdditionalRequestHeaders;
  shared->mPostDataWithBoundaries = GetPostDataWithBoundaries();
  shared->mRequest = this;
  self->mShared = shared;

  self->mThread.Initialize(&WebRequestThread, new WebRequestSharedPointer(shared), "WebRequest");
}

void WebRequest::Cancel()
{
  ZeroGetPrivateData(WebRequestPrivateData);
  if (!IsRunning())
    return;

  {
    Lock lock(self->mShared->mRequestLock);
    self->mShared->mRequest = nullptr;
  }
  self->mShared = nullptr;
}

bool WebRequest::IsRunning()
{
  ZeroGetPrivateData(WebRequestPrivateData);
  return self->mThread.IsValid() && !self->mThread.IsCompleted();
}

void WebRequest::Initialize()
{
  CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
  ErrorIf(code != CURLE_OK, "Curl was unable to initialize");
}

void WebRequest::Shutdown()
{
  curl_global_cleanup();
}

} // namespace Zero
