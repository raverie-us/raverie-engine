// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace WebResponseCode
{
enum Enum
{
  Invalid = -1,
  NoServerResponse = 0,

  // HTTP Log Codes:
  // Informational:
  Continue = 100,
  SwitchingProtocols = 101,
  // Success:
  OK = 200,
  Created = 201,
  Accepted = 202,
  NonauthoritativeInformation = 203,
  NoContent = 204,
  ResetContent = 205,
  PartialContent = 206,
  // Redirection:
  MovedPermanently = 301,
  ObjectMovedTemporarily = 302,
  SeeOther = 303,
  NotModified = 304,
  TemporaryRedirect = 307,
  PermanentRedirect = 308,
  // Client Error:
  BadRequest = 400,
  AccessDenied = 401,
  Forbidden = 403,
  NotFound = 404,
  HTTPVerbNotAllowed = 405,
  ClientBrowserRejectsMIME = 406,
  ProxyAuthenticationRequired = 407,
  PreconditionFailed = 412,
  RequestEntityTooLarge = 413,
  RequestURITooLarge = 414,
  UnsupportedMediaType = 415,
  RequestedRangeNotSatisfiable = 416,
  ExecutionFailed = 417,
  LockedError = 423,
  // Server Error
  InternalServerError = 500,
  UnimplementedHeaderValueUsed = 501,
  GatewayProxyReceivedInvalid = 502,
  ServiceUnavailable = 503,
  GatewayTimedOut = 504,
  HTTPVersionNotSupported = 505
};
typedef int Type;
} // namespace WebResponseCode

class WebRequest;

typedef void (*WebRequestHeadersFn)(const Array<String>& headers, WebResponseCode::Enum code, WebRequest* request);
typedef void (*WebRequestDataFn)(const byte* data, size_t size, u64 totalDownloaded, WebRequest* request);
typedef void (*WebRequestCompleteFn)(Status& status, WebRequest* request);

class WebPostData
{
public:
  // The name of the input field in the submitted form
  String mName;

  // Leave this empty if this is not a file
  String mFileName;

  // File contents or a value
  ByteBufferBlock mValue;
};

// Performs a single web request asynchronously.
// The mOnComplete callback will always be called (even when errors occur)
// except if it is cancelled.
class WebRequest
{
public:
  WebRequest();

  // Destructing the web request cancels any active
  // requests and waits for the thread to complete.
  ~WebRequest();

  // Perform an HTTP request (assumed GET, unless POST data is provided).
  // Once this is called, no data on the WebRequest should be changed as it may
  // be accessed by other threads (consider it immutable).
  void Run();

  // Callable by any thread and signals that this web request is to be
  // cancelled. The below data will become mutable again after this call. Does
  // nothing if no web request is currently running.
  void Cancel();

  // Returns true if a web request is actively running, false otherwise.
  bool IsRunning();

  // Initializes the WebRequest data globally.
  // Must be called at the start when no other threads are running.
  static void Initialize();
  // Cleans up the web request at the end.
  static void Shutdown();

  // Shared:
  // Return the boundary used in multipart/form requests.
  String GetBoundary();
  // Return the content type used in multipart/form requests, which includes the
  // boundary. This always ends in the HTTP newline (\r\n).
  String GetContentTypeHeader();
  // Concatenates all post data and adds boundaries.
  String GetPostDataWithBoundaries();
  // Gets the headers concatenated with the standard HTTP newline (\r\n).
  // This also includes the content type header at the beginning.
  String GetNewlineSeparatedHeaders();
  // The user agent we use for this version of the executable (use if possible).
  static String GetUserAgent();

  // The data and callbacks must be set before calling Run.
  String mUrl;
  Array<WebPostData> mPostData;
  Array<String> mAdditionalRequestHeaders;

  // May be called on another thread.
  WebRequestHeadersFn mOnHeadersReceived;
  WebRequestDataFn mOnDataReceived;
  WebRequestCompleteFn mOnComplete;
  void* mUserData;

  ZeroDeclarePrivateDataBytes(128);
};

} // namespace Zero
