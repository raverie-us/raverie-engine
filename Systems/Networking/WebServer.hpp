///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg.
/// Copyright 2018, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
  DeclareEvent(WebServerRequest);
}

DeclareEnum9(WebServerRequestMethod, Options, Get, Head, Post, Put, Delete, Trace, Connect, Other);

typedef OrderedHashMap<String, String> WebServerHeaderMap;
typedef WebServerHeaderMap::KeyRange WebServerHeaderRange;

/// An event that occurs when we get data from a web server (such as a request).
class WebServerRequestEvent : public Event
{
public:
  ZilchDeclareType(WebServerRequestEvent, TypeCopyMode::ReferenceType);

  /// What kind of request was this?
  WebServerRequestMethod::Enum mMethod;

  /// The string we extracted from the method.
  String mMethodString;

  /// The uri that is being requested.
  String mUri;

  /// Key value pair's of headers (e.g. key "Accept-Language" and value "en-us").
  WebServerHeaderMap mHeaders;

  /// The full data associated with the request.
  String mData;

  /// Any post data submitted by the client.
  String mPostData;

  /// The data we send back in response. This can be filled out manually
  /// and must include the full headers (e.g. "HTTP/1.1 200 OK") or it can
  /// be filled out through any automatic respond function below.
  /// If the response is left empty, we will automatically generate a 404 response.
  String mResponse;

  /// Checks if the given header exists within the request.
  bool HasHeader(StringParam name);

  /// Get a value in the header by name, or returns an empty string if it does not exist.
  String GetHeaderValue(StringParam name);

  /// Get a range of all header names in this request.
  WebServerHeaderRange GetHeaderNames();

  /// Builds the response automatically with the given code, headers, and contents.
  /// The headers that are automatically generated are: Date, Content-Length.
  /// Note that extraHeaders *MUST* use \r\n to separate each header (HTTP 1.1 standard) and *MUST* end with \r\n if provided.
  void Respond(Os::WebResponseCode::Type code, StringParam extraHeaders, StringParam contents);

  /// Builds the response automatically with the given code, headers, and contents.
  /// The headers that are automatically generated are: Date, Content-Length.
  /// Note that extraHeaders *MUST* use \r\n to separate each header (HTTP 1.1 standard) and *MUST* end with \r\n if provided.
  void Respond(StringParam code, StringParam extraHeaders, StringParam contents);
};

class WebServer;

class WebServerConnection
{
public:
  ~WebServerConnection();

  static OsInt ReadThread(void* userData);
  static OsInt WriteThread(void* userData);

  Atomic<bool> mRunning;
  Socket mSocket;
  WebServer* mServer;
  Thread mReadThread;
  Thread mWriteThread;
};

/// Listens on a given port for incoming HTTP traffic and allows the user
/// to respond to any request via events (such as WebServerRequest).
/// The WebServer does not automatically serve files from a directory.
/// It will always generate a 404 error if the user does not provide a response.
class WebServer : public ReferenceCountedThreadSafeId32EventObject
{
public:
  friend class WebServerConnection;

  ZilchDeclareType(WebServer, TypeCopyMode::ReferenceType);

  WebServer();
  ~WebServer();
  
  /// Creates a web-server that can be used in script.
  static WebServer* Create();

  /// Starts listening on the given port for HTTP connections (closes the server if it's already running).
  /// Returns false if it fails to host for any reason, such as another service using the same port.
  bool Host(uint port);

  /// Closes the server and all connections.
  /// This will block until the connections are completed / all threads are shutdown.
  void Close();

  /// Given a WebResponseCode return what the HTTP string would be, e.g. OK turns into "200 OK".
  /// If an invalid or unknown code is provided this will return an empty string.
  static String GetWebResponseCodeString(Os::WebResponseCode::Type code);

protected:
  static void DoNotifyExceptionOnFail(StringParam message, const u32& context, void* userData);
  static OsInt AcceptThread(void* userData);

  Thread mAcceptThread;
  Socket mAcceptSocket;
  Atomic<bool> mLogging;
  Atomic<bool> mRunning;
  ThreadLock mConnectionsLock;
  Array<WebServerConnection*> mConnections;
};

/// A specialized WebServer that provides content from a given directory.
/// Content can be limited by specific file name or by filters.
/// This server can also automatically look for index files, such as index.htm/index.html.
class WebServerDirectory : public WebServer
{
public:
};

} // namespace Zero
