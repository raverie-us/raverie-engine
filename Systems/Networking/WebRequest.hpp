///////////////////////////////////////////////////////////////////////////////
///
/// \file TcpSocket.hpp
/// Declaration of the TcpSocket class.
///
/// Authors: Trevor Sundberg.
/// Copyright 2010-2014, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(PartialWebResponse);
  DeclareEvent(WebResponse);
}

namespace WebResponseCode
{
  enum Enum
  {
    NoServerResponse              = 0,

    // HTTP Log Codes:
    // Informational:
    Continue                      = 100,
    SwitchingProtocols            = 101,
    // Success:
    OK                            = 200,
    Created                       = 201,
    Accepted                      = 202,
    NonauthoritativeInformation   = 203,
    NoContent                     = 204,
    ResetContent                  = 205,
    PartialContent                = 206,
    // Redirection:
    MovedPermanently              = 301,
    ObjectMovedTemporarily        = 302,
    SeeOther                      = 303,
    NotModified                   = 304,
    TemporaryRedirect             = 307,
    // Client Error:
    BadRequest                    = 400,
    AccessDenied                  = 401,
    Forbidden                     = 403,
    NotFound                      = 404,
    HTTPVerbNotAllowed            = 405,
    ClientBrowserRejectsMIME      = 406,
    ProxyAuthenticationRequired   = 407,
    PreconditionFailed            = 412,
    RequestEntityTooLarge         = 413,
    RequestURITooLarge            = 414,
    UnsupportedMediaType          = 415,
    RequestedRangeNotSatisfiable  = 416,
    ExecutionFailed               = 417,
    LockedError                   = 423,
    // Server Error
    InternalServerError           = 500,
    UnimplementedHeaderValueUsed  = 501,
    GatewayProxyReceivedInvalid   = 502,
    ServiceUnavailable            = 503,
    GatewayTimedOut               = 504,
    HTTPVersionNotSupported       = 505
    // cURL also returns FTP and SMTP codes
    // Add if necessary
  };
  typedef int Type;
}

/// Curl is not safe to perform global initialization when any threads are active (not just any threads using curl).
/// To ensure this we MUST globally initialize curl at the beginning of the application's lifetime. It's possible to
/// use a statically initialized class which will unsure this happens once before main, but we also have to avoid
/// performing this in a dll's static initializer otherwise we may have a loader lock deadlock happen.
/// Hence this is the responsibility of each application to ensure this is called once in main.
class WebRequestInitializer
{
public:
  WebRequestInitializer();
  ~WebRequestInitializer();
};

class WebResponseEvent;

/// Lets us send out web requests generically.
class BlockingWebRequest : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  BlockingWebRequest();

  /// Destructor.
  ~BlockingWebRequest();

  /// Encodes a string to be safely passed as a parameter
  /// in a Url using Percent-encoding.
  static String UrlParamEncode(StringParam string);

  /// Decodes a string from a Url using Percent-encoding.
  static String UrlParamDecode(StringParam string);

  /// Add a file to be uploaded to a post request.
  void AddFile(StringParam fileName, StringParam formFieldName);

  /// Add a field to a post request.
  void AddField(StringParam name, StringParam content);

  /// Clear any attached post data (such as files).
  void ClearPostData();

  /// Run the request on the given url and receive data back in the 'WebResponse' event.
  /// This will clear any stored data from previous requests, and if mStoreData is set
  /// it will return the entire response as a string (if not it will be empty).
  String Run();

  /// Cancels the web request.
  void Cancel();

  // Clears any stored data
  void ClearData();

  /// The url we're going to fetch data from.
  String mUrl;

  /// String based post data.
  String mPostData;

  WebResponseCode::Type mResponseCode;

  /// Additional headers.
  Array<String> mHeaders;

  /// Array of response headers.
  Array<String> mResponseHeaders;

  /// Whether or not we store the entire response or just send it out.
  /// By default, this is true for convenience.
  bool mStoreData;

  /// Is an event sent out when this completes? This should almost never be altered and is
  /// used primarily for the version selector which needs to run before zero is loaded.
  bool mSendEvent;

private:

  // Occurs when we receive data
  static int OnDataReceived(char* data, size_t count, size_t elementSize, BlockingWebRequest* request);

  static size_t OnHeaderReceived(char* data, size_t count, size_t elementSize, BlockingWebRequest* userdata);

private:

  // Contains all data concatenated together from a request (only used when mStoreData is set)
  StringBuilder mStoredData;

  // The interface we use to send the data (eg CURL)
  void* mHandle;

  // Other needed information for posting
  void* mFormPostFile;
  void* mLastPtr;

  // Whether or not the request has been externally canceled.
  bool mCanceled;
};

class WebRequestJob;

/// A web requester that will make a non-blocking threaded request.
/// WARNING: This object MUST continue to exist until it receives the final.
/// WebResponse event since we don't currently have thread safe handles.
class ThreadedWebRequest : public ThreadSafeId32EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ThreadedWebRequest();
  ~ThreadedWebRequest();

  /// Run the request on the given url and receive data back in the 'WebResponse' event.
  void Run();
  void Cancel();

  /// The url we're going to fetch data from.
  String mUrl;

private:
  WebRequestJob* mJob;
};

/// An event that occurs when we get data back from a web request.
class WebResponseEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The response code we received from the server.
  WebResponseCode::Type ResponseCode;

  Array<String> ResponseHeaders;

  /// The data associated with the response.
  String Data;
};


/// A component we can use to facilitate web requests.
class WebRequester : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// Get and set the url that we make requests to.
  String GetUrl();
  void SetUrl(StringParam url);

  /// Clear headers and post data.
  void Clear();

  /// Add a header to the web request .
  void SetHeader(StringParam name, StringParam data);

  /// Add Post data to the request, this will also change
  /// the request to a post request.
  void SetPostData(StringParam data);

  /// Run the web request (we should get data back in a WebResponse event).
  void Run();

private:

  /// Occurs when the request gives us a response (with data).
  void OnWebResponse(WebResponseEvent* e);

private:

  /// The object we use to perform web requests.
  BlockingWebRequest mRequest;

};

class WebRequestJob : public Job
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  int Execute();

  ThreadedWebRequest* mOrigin;
  BlockingWebRequest mWebRequest;
  EventReceiver mReceiver;

  EventReceiver* GetReceiver() { return &mReceiver; }

  void OnPartialWebResponse(WebResponseEvent* event);
  void OnWebResponse(WebResponseEvent* event);
};

} // namespace Zero
