// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(WebResponseHeadersInternal);
DeclareEvent(WebResponsePartialDataInternal);
DeclareEvent(WebResponseCompleteInternal);
DeclareEvent(WebResponseHeaders);
DeclareEvent(WebResponsePartialData);
DeclareEvent(WebResponseComplete);
} // namespace Events

class WebResponseEvent;

/// Runs a single web-request at a time asynchronously.
/// If a web request is in progress and another is requested, it will cancel the
/// first. To run multiple requests at a time, create multiple AsyncWebRequests.
/// The WebResponseComplete event will be sent at the end of a request (even if
/// it fails or errors), however it may not be sent if Cancel is called.
class AsyncWebRequest : public ReferenceCountedThreadSafeId32EventObject
{
public:
  RaverieDeclareType(AsyncWebRequest, TypeCopyMode::ReferenceType);

  /// Constructs the AsyncWebRequest.
  static AsyncWebRequest* Create();

  /// Destructor cancels any existing request.
  ~AsyncWebRequest();

  /// Run the request on the given url and receive data back in the
  /// 'WebResponse' event. This will clear any stored data from previous
  /// requests, and if StoreData is set it will return the entire response as a
  /// string (if not it will be empty).
  void Run();

  /// Returns if the web request is currently running.
  bool GetIsRunning();

  /// Cancels the web request.
  void Cancel();

  /// Clears all request and response data.
  void ClearAll();

  /// Clears all request data (url, post data, additional headers, etc).
  void ClearRequestData();

  /// Clears any data that was gotten from a response (headers, stored data,
  /// progress, etc).
  void ClearResponseData();

  /// Add a header to the web request .
  void AddHeader(StringParam name, StringParam data);

  /// Add a file to be uploaded to a post request.
  void AddFile(StringParam formFieldName, StringParam fileName);

  /// Add a field to a post request.
  void AddField(StringParam name, StringParam content);

  /// This should only be called when shutting down the program.
  static void CancelAllActiveRequests();

  /// Return the buffer of stored data (only valid if StoredData is true).
  /// Note that if this is called in the middle of a partial data event,
  /// this will return all the concatenated data up to that point.
  String GetData();

  /// The url includes the protocol, host and resource we're requesting.
  String mUrl;

  /// Adding post data means this will be a post request.
  Array<WebPostData> mPostData;

  /// Any additional headers we want to include with the request.
  Array<String> mAdditionalRequestHeaders;

  /// Whether or not we store the entire response or just send it out.
  /// By default, this is true for convenience.
  bool mStoreData;

  /// The response code that was received from the server.
  WebResponseCode::Enum mResponseCode;

  /// Array of response headers.
  Array<String> mResponseHeaders;

  /// Contains all data concatenated together from a response (only used when
  /// mStoreData is set).
  StringBuilder mStoredData;

  /// How much data we've downloaded so far in bytes.
  u64 mTotalDownloaded;

  /// How much data we expect to download in bytes.
  /// If this value is 0, it means the server did not send us a content length.
  u64 mTotalExpected;

  /// Our progress in the web request. Once complete, progress will be 1.
  float mProgress;

  /// Type of progress. Some servers don't send the content length,
  /// so we don't know how much we have to download.
  ProgressType::Enum mProgressType;

  /// A human readable error message if any occurred.
  /// This will be an empty string if there was no error.
  String mError;

  /// If this option is set, it means we send events on
  /// the web request thread (during the web request callbacks).
  /// Set this option before running the request and do not change it
  /// afterwards. Care must be taken to ensure that no properties or calls are
  /// made at the same time on another thread (such as the main thread) when
  /// using this option. For non threaded platforms, this will still be on the
  /// main thread. The default is false.
  bool mSendEventsOnRequestThread;

  /// If this value is non-zero, we will automatically cache a file in a
  /// temporary location for this many seconds when the response is OK.
  /// Subsequent requests to the same URL will automatically return the last
  /// cached request. If the request fails in any way, then we also return the
  /// last cached request even if it's expired. Note that this is forced
  /// caching, and does not query the url to see if a newer version exists.
  /// WARNING: This only caches by url, and does NOT use post data or request
  /// headers to differentiate a response. Note that we will only get the
  /// WebResponseComplete event and no headers/partial data if the cache is
  /// found. Also note that the cache finding is not executed on another thread,
  /// so mSendEventsOnRequestThread has no effect. Currently can only cache data
  /// when StoreData is set to true. Default is 0 (no caching).
  u64 mForceCacheSeconds;

  /// We need an intrusive list of all web requests so that when the engine
  /// shuts down we can cancel all of them (because otheriwse they will keep
  /// running and try to use the Z::gDispatch).
  IntrusiveLink(AsyncWebRequest, link);

private:
  AsyncWebRequest();

  // If a file in the cache exists, this will send a completed response and
  // return true.
  bool SendCompletedCacheResponse(bool ignoreTime);

  // Occurs when we receive http headers.
  static void OnHeadersReceived(const Array<String>& headers, WebResponseCode::Enum code, WebRequest* request);
  void OnWebResponseHeadersInternal(WebResponseEvent* event);

  // Occurs when we receive data.
  static void OnDataReceived(const byte* data, size_t size, u64 totalDownloaded, WebRequest* request);
  void OnWebResponsePartialDataInternal(WebResponseEvent* event);

  // Occurs when the web request is complete (can be due to an error).
  static void OnComplete(Status& status, WebRequest* request);
  void OnWebResponseCompleteInternal(WebResponseEvent* event);

  WebRequest mRequest;

  /// Every time a request is cancelled we increment the version
  /// on the AsyncWebRequest to ensure we don't receive old events.
  Atomic<uint> mVersion;

  /// All living / active AsyncWebRequests (and a thread lock to go with it).
  static InList<AsyncWebRequest> mActiveRequests;
  static ThreadLock mActiveRequestsLock;
};

/// An event that occurs when we get data back from a web request.
class WebResponseEvent : public Event
{
public:
  friend class AsyncWebRequest;
  RaverieDeclareType(WebResponseEvent, TypeCopyMode::ReferenceType);

  WebResponseEvent();

  /// The AsyncWebRequest that generated the request.
  HandleOf<AsyncWebRequest> mAsyncWebRequest;

  /// The response code we received from the server.
  WebResponseCode::Enum mResponseCode;

  /// Any headers we received from the server.
  Array<String> mResponseHeaders;

  /// On WebResponsePartialData this will be the chunk of data received.
  /// On WebResponseComplete, if StoreData is true then this will be the
  /// complete response from the server, otherwise it will be empty.
  String mData;

  /// How much data we've downloaded so far.
  u64 mTotalDownloaded;

  /// How much data we expect to download.
  /// If this value is 0, it means the server did not send us a content length.
  u64 mTotalExpected;

  /// Our progress in the web request. Once complete, progress will be 1.
  float mProgress;

  /// Type of progress. Some servers don't send the content length,
  /// so we don't know how much we have to download.
  ProgressType::Enum mProgressType;

  /// A human readable error message if any occurred.
  /// This will be an empty string if there was no error.
  String mError;

  /// Gets the string value of header by index (e.g. "Content-Length: 1234");
  StringParam GetHeader(uint index);

  /// Get how many headers were returned from the server.
  uint GetHeaderCount();

  /// Reads the Data as if it were a Json document.
  /// This should only be called on a WebResponseComplete event when StoredData
  /// is true. The memory returned must be deleted by the user.
  JsonValue* ReadJson(Status& status);

private:
  /// Every time a request is cancelled we increment the version
  /// on the AsyncWebRequest to ensure we don't receive old events.
  uint mVersion;
};

/// A component we can use to facilitate web requests.
/// Only the url is serialized (the post data and headers are not saved).
class WebRequester : public Component
{
public:
  RaverieDeclareType(WebRequester, TypeCopyMode::ReferenceType);

  WebRequester();
  ~WebRequester();

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// Get and set the url that we make requests to.
  String GetUrl();
  void SetUrl(StringParam url);

  /// Runs and returns an asynchronous web request.
  /// The WebRequester guarantees it will keep the AsyncWebRequest alive
  /// until the request is completed or cancelled.
  HandleOf<AsyncWebRequest> Run();

  /// Clears any stored data, post data (including files), headers, url,
  /// response code, etc.
  void Clear();

  /// Cancels all active requests.
  void CancelActiveRequests();

  /// Add a header to the web request .
  void AddHeader(StringParam name, StringParam data);

  /// Add a file to be uploaded to a post request.
  void AddFile(StringParam formFieldName, StringParam fileName);

  /// Add a field to a post request.
  void AddField(StringParam formFieldName, StringParam content);

private:
  /// Forward all events that we get.
  void OnForwardEvent(WebResponseEvent* event);

  /// Creates a new request internally, listens for events, and copies data from
  /// the last one. Returns the previous AsyncWebRequest handle.
  HandleOf<AsyncWebRequest> ReplaceRequest();

  /// Walk the active requests and cleanup those that are no longer active.
  void CleanDeadRequests();

  /// Keep all the active requests alive so we can get events from them.
  Array<HandleOf<AsyncWebRequest>> mActiveRequests;

  /// Note that the request is always valid. One is created in the constructor
  /// and each time we run another one is created in it's place.
  HandleOf<AsyncWebRequest> mRequest;

  /// We can't immediately create the request so we need
  /// to serialize the url using our own data member.
  /// Do NOT use this to set the url!
  String mSerializedUrl;

  /// Does the WebRequester cancel all active
  /// requests that it created upon its destruction?
  /// If this is set to false, the user can keep
  /// AsyncWebRequests alive by holding a reference to them.
  bool mCancelOnDestruction;
};

} // namespace Raverie
