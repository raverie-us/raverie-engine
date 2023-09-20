// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

class WebRequestPrivateData
{
public:
  WebRequestPrivateData() : mIsRunning(false)
  {
  }

  bool mIsRunning;
};

extern "C" EMSCRIPTEN_KEEPALIVE void EmscriptenWebRequestOnHeadersReceived(char* responseHeaders,
                                                                           int responseCode,
                                                                           WebRequest* request)
{
  Array<String> headers;
  if (request->mOnHeadersReceived)
  {
    String responseHeadersStr(responseHeaders);
    StringSplitRange headerLines = responseHeadersStr.Split("\r\n");
    forRange (StringRange headerLine, headerLines)
      headers.PushBack(headerLine);

    request->mOnHeadersReceived(headers, (WebResponseCode::Enum)responseCode, request);
  }

  free(responseHeaders);
}

extern "C" EMSCRIPTEN_KEEPALIVE void
EmscriptenWebRequestOnDataReceived(byte* data, size_t length, size_t totalDownloaded, WebRequest* request)
{
  // Parameter 'data' is a JavaScript owned Uint8Array (no need to call free).
  if (request->mOnDataReceived)
    request->mOnDataReceived(data, length, (u64)totalDownloaded, request);
}

extern "C" EMSCRIPTEN_KEEPALIVE void EmscriptenWebRequestOnComplete(char* errorMessage, WebRequest* request)
{
  if (request->mOnComplete)
  {
    Status status;
    if (errorMessage)
      status.SetFailed(errorMessage);
    request->mOnComplete(status, request);
  }

  ((WebRequestPrivateData*)request->mPrivateData)->mIsRunning = false;

  free(errorMessage);
}

EM_JS(void,
      EmscriptenWebRequestRun,
      (bool isPost, const char* url, const char* headers, const char* postData, WebRequest* request),
      { xhrRun(isPost, UTF8ToString(url), UTF8ToString(headers), UTF8ToString(postData), request); });

EM_JS(void, EmscriptenWebRequestCancel, (WebRequest * request), { xhrCancel(request); });

WebRequest::WebRequest() :
    mOnHeadersReceived(nullptr),
    mOnDataReceived(nullptr),
    mOnComplete(nullptr),
    mUserData(nullptr)
{
  RaverieConstructPrivateData(WebRequestPrivateData);
}

WebRequest::~WebRequest()
{
  RaverieDestructPrivateData(WebRequestPrivateData);
}

void WebRequest::Run()
{
  RaverieGetPrivateData(WebRequestPrivateData);

  Cancel();

  String headers = GetNewlineSeparatedHeaders();
  String postData = GetPostDataWithBoundaries();

  self->mIsRunning = true;
  EmscriptenWebRequestRun(!mPostData.Empty(), mUrl.c_str(), headers.c_str(), postData.c_str(), this);
}

void WebRequest::Cancel()
{
  RaverieGetPrivateData(WebRequestPrivateData);

  if (!self->mIsRunning)
    return;

  self->mIsRunning = false;
  EmscriptenWebRequestCancel(this);
}

bool WebRequest::IsRunning()
{
  RaverieGetPrivateData(WebRequestPrivateData);

  return self->mIsRunning;
}

void WebRequest::Initialize()
{
}

void WebRequest::Shutdown()
{
}

} // namespace Raverie
