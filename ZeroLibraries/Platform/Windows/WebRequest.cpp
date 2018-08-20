///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

class WebHandle
{
public:
  WebHandle() :
    mHandle(nullptr)
  {
  }
  WebHandle(HINTERNET handle) :
    mHandle(handle)
  {
  }

  ~WebHandle()
  {
    WinHttpCloseHandle(mHandle);
  }

  operator HINTERNET()
  {
    return mHandle;
  }

  HINTERNET mHandle;
};

OsInt WebRequestThread(void* request)
{
  WebRequest* self = (WebRequest*)request;
  Status status;

  WString wurl = Widen(self->mUrl);
  URL_COMPONENTS urlComp;
  ZeroMemory(&urlComp, sizeof(urlComp));
  urlComp.dwStructSize = sizeof(urlComp);
  urlComp.dwSchemeLength = (DWORD)-1;
  urlComp.dwHostNameLength = (DWORD)-1;
  urlComp.dwUrlPathLength = (DWORD)-1;

  BOOL urlResult = WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComp);

  if (!urlResult)
  {
    FillWindowsErrorStatus(status, "WinHttpCrackUrl");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  if (self->mCancel)
    return 0;

  WebHandle hSession = WinHttpOpen(
    L"WebRequest",
    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME,
    WINHTTP_NO_PROXY_BYPASS,
    0);

  if (!hSession)
  {
    FillWindowsErrorStatus(status, "WinHttpOpen");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  if (self->mCancel)
    return 0;

  WString hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);

  WebHandle hConnect = WinHttpConnect(
    hSession,
    hostName.c_str(),
    INTERNET_DEFAULT_HTTPS_PORT,
    0);

  if (!hConnect)
  {
    FillWindowsErrorStatus(status, "WinHttpConnect");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  if (self->mCancel)
    return 0;

  LPCWSTR verb = L"GET";
  if (!self->mPostData.Empty())
    verb = L"POST";

  WString urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

  WebHandle hRequest = WinHttpOpenRequest(
    hConnect,
    verb,
    urlPath.c_str(),
    nullptr,
    WINHTTP_NO_REFERER,
    WINHTTP_DEFAULT_ACCEPT_TYPES,
    WINHTTP_FLAG_SECURE);

  if (!hRequest)
  {
    FillWindowsErrorStatus(status, "WinHttpOpenRequest");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  if (self->mCancel)
    return 0;

  WString headers;

  // Turn the additional headers into newline separated strings
  StringBuilder builder;

  forRange(StringParam header, self->mAdditionalRequestHeaders)
  {
    builder.Append(header);
    builder.Append("\r\n");
  }

  const char* boundary = "----------ZeroEngine43476095-a5a0-4190-a9b5-bce2d2de5eef$";

  builder.Append("Content-Type:multipart/form-data; boundary=");
  builder.Append(boundary);
  builder.Append("\r\n");

  headers = Widen(builder.ToString());

  StringBuilder post;

  forRange(WebPostData& postData, self->mPostData)
  {
    post.Append("--");
    post.Append(boundary);
    post.Append("\r\n");
    post.Append("Content-Disposition: form-data; name=\"");
    post.Append(postData.mName);
    post.Append("\"");

    if (!postData.mFileName.Empty())
    {
      post.Append("; filename=\"");
      post.Append(postData.mFileName);
      post.Append("\"");
    }

    post.Append("\r\n\r\n");

    // We rely on the fact that this can write binary data
    post.Write(postData.mValue.GetBegin(), postData.mValue.Size());

    post.Append("\r\n");
  }

  post.Append("--");
  post.Append(boundary);
  post.Append("--");

  String postString = post.ToString();

  BOOL bResults = WinHttpSendRequest(
    hRequest,
    headers.c_str(),
    -1,
    (void*)postString.Data(),
    postString.SizeInBytes(),
    postString.SizeInBytes(),
    0);

  if (!bResults)
  {
    FillWindowsErrorStatus(status, "WinHttpSendRequest");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  if (self->mCancel)
    return 0;

  bResults = WinHttpReceiveResponse(hRequest, nullptr);

  if (!bResults)
  {
    FillWindowsErrorStatus(status, "WinHttpReceiveResponse");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  if (self->mCancel)
    return 0;

  // Call it once just to get the size, then create it again
  DWORD dwHeaderSize = 0;
  WinHttpQueryHeaders(
    hRequest,
    WINHTTP_QUERY_RAW_HEADERS,
    WINHTTP_HEADER_NAME_BY_INDEX,
    nullptr,
    &dwHeaderSize,
    WINHTTP_NO_HEADER_INDEX);

  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    WCHAR* lpBuffer = new WCHAR[dwHeaderSize / sizeof(WCHAR)];

    bResults = WinHttpQueryHeaders(
      hRequest,
      WINHTTP_QUERY_RAW_HEADERS,
      WINHTTP_HEADER_NAME_BY_INDEX,
      lpBuffer,
      &dwHeaderSize,
      WINHTTP_NO_HEADER_INDEX);

    if (!bResults)
    {
      FillWindowsErrorStatus(status, "WinHttpQueryHeaders");
      if (self->mOnComplete)
        self->mOnComplete(status, self);
      return -1;
    }

    if (self->mCancel)
      return 0;

    Array<String> headers;

    while (*lpBuffer != 0)
    {
      headers.PushBack(Narrow(lpBuffer));
      lpBuffer += wcslen(lpBuffer) + 1;
    }

    DWORD dwStatusCode = 0;
    DWORD dwStatusCodeSize = sizeof(dwStatusCode);
    bResults = WinHttpQueryHeaders(
      hRequest,
      WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
      WINHTTP_HEADER_NAME_BY_INDEX,
      &dwStatusCode,
      &dwStatusCodeSize,
      WINHTTP_NO_HEADER_INDEX);

    if (!bResults)
    {
      FillWindowsErrorStatus(status, "WinHttpQueryHeaders");
      if (self->mOnComplete)
        self->mOnComplete(status, self);
      return -1;
    }

    if (self->mCancel)
      return 0;

    if (self->mOnHeadersReceived)
      self->mOnHeadersReceived(headers, (WebResponseCode::Enum)dwStatusCode, self);
  }
  else
  {
    FillWindowsErrorStatus(status, "WinHttpQueryHeaders");
    if (self->mOnComplete)
      self->mOnComplete(status, self);
    return -1;
  }

  Array<byte> buffer;

  DWORD dwSize = 0;
  do
  {
    // Check for available data
    dwSize = 0;

    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
    {
      FillWindowsErrorStatus(status, "WinHttpQueryDataAvailable");
      if (self->mOnComplete)
        self->mOnComplete(status, self);
      return -1;
    }

    if (self->mCancel)
      return 0;

    buffer.Resize(dwSize);

    DWORD dwDownloaded = 0;
    if (!WinHttpReadData(hRequest, (LPVOID)buffer.Data(), dwSize, &dwDownloaded))
    {
      FillWindowsErrorStatus(status, "WinHttpReadData");
      if (self->mOnComplete)
        self->mOnComplete(status, self);
      return -1;
    }

    if (self->mCancel)
      return 0;

    if (self->mOnDataReceived)
      self->mOnDataReceived(buffer.Data(), (size_t)dwDownloaded, self);
  } while (dwSize > 0);

  if (self->mCancel)
    return 0;

  // We finished the web request.
  if (self->mOnComplete)
    self->mOnComplete(status, self);
  return 0;
}

WebRequest::WebRequest() :
  mOnHeadersReceived(nullptr),
  mOnDataReceived(nullptr),
  mOnComplete(nullptr),
  mUserData(nullptr),
  mCancel(false)
{
  ZeroConstructPrivateData(Thread);
}

WebRequest::~WebRequest()
{
  ZeroGetPrivateData(Thread);

  // Cancel any active requests.
  Cancel();

  // Wait for the thread to complete before we destruct ourselves.
  if (self->IsValid())
    self->WaitForCompletion();

  ZeroDestructPrivateData(Thread);
}

void WebRequest::Run()
{
  ZeroGetPrivateData(Thread);
  self->Initialize(&WebRequestThread, this, "WebRequest");
}

void WebRequest::Cancel()
{
  ZeroGetPrivateData(Thread);
  if (!IsRunning())
    return;

  mCancel = true;
  self->WaitForCompletion();
  mCancel = false;
}

bool WebRequest::IsRunning()
{
  ZeroGetPrivateData(Thread);
  return self->IsValid() && !self->IsCompleted();
}

} // namespace Zero
