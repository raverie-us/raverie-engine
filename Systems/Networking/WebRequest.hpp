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

  /// Run the request on the given url and receive data back in the 'WebResponse' event.
  /// This will clear any stored data from previous requests, and if StoreData is set
  /// it will return the entire response as a string (if not it will be empty).
  String Run();

  /// Cancels the web request.
  void Cancel();

  /// Clears any stored data, post data (including files), headers, cancelled flag, etc.
  void Clear();

  /// Add a file to be uploaded to a post request.
  void AddFile(StringParam formFieldName, StringParam fileName);

  /// Add a field to a post request.
  void AddField(StringParam name, StringParam content);

  /// The url we're going to fetch data from.
  String mUrl;

  /// Additional headers.
  Array<String> mHeaders;

  /// All data to be added to the post section of a request.
  Array<Os::WebPostData> mPosts;

  /// Whether or not we store the entire response or just send it out.
  /// By default, this is true for convenience.
  bool mStoreData;

  /// Is an event sent out when this completes? This should almost never be altered and is
  /// used primarily for the version selector which needs to run before zero is loaded.
  bool mSendEvent;

  /// Whether or not the request has been externally canceled.
  Atomic<bool> mCanceled;

  /// The response code that was received from the server.
  Os::WebResponseCode::Type mResponseCode;

  /// Array of response headers.
  Array<String> mResponseHeaders;

  /// Contains all data concatenated together from a response (only used when mStoreData is set)
  StringBuilder mStoredData;

private:

  // Occurs when we receive http headers
  static void OnHeadersReceived(const Array<String>& headers, Os::WebResponseCode::Enum code, void* userData);

  // Occurs when we receive data
  static void OnDataReceived(const byte* data, size_t size, void* userData);
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
  Os::WebResponseCode::Type ResponseCode;

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
  void AddHeader(StringParam name, StringParam data);

  /// Add a file to be uploaded to a post request.
  void AddFile(StringParam formFieldName, StringParam fileName);

  /// Add a field to a post request.
  void AddField(StringParam formFieldName, StringParam content);

  /// Run the web request (we should get data back in a WebResponse event).
  void Run();

private:

  /// Occurs when the request gives us a response (with data).
  void OnWebResponse(WebResponseEvent* e);

private:

  /// The object we use to perform web requests.
  BlockingWebRequest mRequest;

  size_t mDeprecatedPostDataIndex;
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
