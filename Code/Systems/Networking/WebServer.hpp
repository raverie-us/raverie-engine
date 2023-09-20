// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
namespace Events
{
// This is the request that is actually sent from the thread (not exposed to
// script).
DeclareEvent(WebServerRequestRaw);

// After the WebServer intercepts it it will sent it to script as this event.
// This occurs before the server tries to do directory or file mapping.
DeclareEvent(WebServerRequest);

// If the users don't hand it via 'WebServerRequest', and the WebServer doesn't
// handle it, then this is sent one last time.
DeclareEvent(WebServerUnhandledRequest);
} // namespace Events

DeclareEnum9(WebServerRequestMethod, Options, Get, Head, Post, Put, Delete, Trace, Connect, Other);

typedef OrderedHashMap<String, String> WebServerHeaderMap;
typedef WebServerHeaderMap::KeyRange WebServerHeaderRange;

class WebServer;
class WebServerConnection;

/// An event that occurs when we get data from a web server (such as a request).
/// If no Respond function is called on the event then we will automatically
/// generate a 404 response.
class WebServerRequestEvent : public Event
{
public:
  RaverieDeclareType(WebServerRequestEvent, TypeCopyMode::ReferenceType);

  WebServerRequestEvent(WebServerConnection* connection);

  /// Automatically generates a 404 response if no Respond has been called.
  ~WebServerRequestEvent();

  /// The web server that this event originated from.
  WebServer* mWebServer;

  /// What kind of request was this?
  WebServerRequestMethod::Enum mMethod;

  /// The string we extracted from the method.
  String mMethodString;

  /// The original uri that is being requested, with escaped characters such as
  /// spaces as '%20'.
  String mOriginalUri;

  /// The uri after we un-escaped the percent-encoded characters.
  String mDecodedUri;

  /// Key value pair's of headers (e.g. key "Accept-Language" and value
  /// "en-us").
  WebServerHeaderMap mHeaders;

  /// Any post data submitted by the client.
  String mPostData;

  /// The full data associated with the request.
  String mData;

  /// Checks if the given header exists within the request.
  bool HasHeader(StringParam name);

  /// Get a value in the header by name, or returns an empty string if it does
  /// not exist. All header values are automatically trimmed (no optional
  /// whitespace prefix/suffix).
  String GetHeaderValue(StringParam name);

  /// Get a range of all header names in this request.
  /// All header names are lowercase because they are case insensitive.
  WebServerHeaderRange GetHeaderNames();

  /// Builds the response automatically with the given code, headers, and
  /// contents. The headers that are automatically generated are: Date,
  /// Content-Length. Note that extraHeaders *MUST* use \r\n to separate each
  /// header (HTTP 1.1 standard) and *MUST* end with \r\n if provided.
  void Respond(WebResponseCode::Enum code, StringParam extraHeaders, StringParam contents);

  /// Builds the response automatically with the given code, headers, and
  /// contents. The headers that are automatically generated are: Date,
  /// Content-Length. Note that extraHeaders *MUST* use \r\n to separate each
  /// header (HTTP 1.1 standard) and *MUST* end with \r\n if provided.
  void Respond(StringParam code, StringParam extraHeaders, StringParam contents);

  /// Send a manually filled out HTTP response which includes
  /// the status and full headers (e.g. "HTTP/1.1 200 OK").
  void Respond(StringParam response);

  // Internal
  /// The connection that this event originated from. We clear the event once we
  /// have responded.
  WebServerConnection* mConnection;
};

class WebServerConnection
{
public:
  WebServerConnection(WebServer* server);
  ~WebServerConnection();

  static OsInt ReadWriteThread(void* userData);

  WebServer* mWebServer;
  Socket mSocket;
  Thread mReadWriteThread;
  OsEvent mWriteSignal;
  ThreadLock mWriteLock;
  Array<byte> mWriteData;
  bool mWriteComplete;
};

/// Listens on a given port for incoming HTTP traffic and allows the user
/// to respond to any request via events (such as WebServerRequest).
/// The WebServer does not automatically serve files from a directory.
/// It will always generate a 404 error if the user does not provide a response.
class WebServer : public ReferenceCountedThreadSafeId32EventObject
{
public:
  friend class WebServerConnection;

  RaverieDeclareType(WebServer, TypeCopyMode::ReferenceType);

  WebServer();
  ~WebServer();

  /// Creates a web-server that can be used in script.
  static WebServer* Create();

  /// Starts listening on the given port for HTTP connections (closes the server
  /// if it's already running). Returns false if it fails to host for any
  /// reason, such as another service using the same port.
  bool Host(uint port);

  /// Closes the server and all connections.
  /// This will block until the connections are completed / all threads are
  /// shutdown.
  void Close();

  /// Replaces & > < " ' characters with &amp; &lt; &gt; &quot; &#39; and
  /// returns the string.
  static String SanitizeForHtml(StringParam text);

  /// Given a WebResponseCode return what the HTTP string would be, e.g. OK
  /// turns into "200 OK". If an invalid or unknown code is provided this will
  /// return an empty string.
  static String GetWebResponseCodeString(WebResponseCode::Enum code);

  /// Encodes a string to be safely passed as a parameter in a url using
  /// percent-encoding.
  static String UrlParamEncode(StringParam string);

  /// Decodes a string from a url using percent-encoding.
  static String UrlParamDecode(StringParam string);

  /// Maps an extension (e.g. "html" or ".html") to a MIME type (e.g.
  /// "text/html"). This is only used when the Path is set, but can be quried by
  /// the user via 'GetMimeTypeFromExtension'. The extension may or may not have
  /// the '.' in it. The MIME type can include multiple parts such as
  /// "text/html; charset=utf-8". This will overwrite a MIME type if one is
  /// already mapped for the given extension.
  void MapExtensionToMimeType(StringParam extension, StringParam mimeType);

  /// Returns a MIME type for a given extension which may include the '.' (e.g.
  /// "html" or ".html"). If the MIME type isn't known, then an empty string
  /// will be returned. Mime types and extensions can be added via
  /// 'MapExtensionToMimeType'.
  String GetMimeTypeFromExtension(StringParam extension);

  /// Clears the map of extensions to MIME types.
  void ClearMimeTypes();

  /// A single path that we can point at on the local file system where we map
  /// requests. The web server will first send 'WebServerRequest', then check
  /// for a file or directory within this path, then if nothing is found (or the
  /// path is empty) the web server will send 'WebServerUnhandledRequest'.
  String mPath;

private:
  void OnWebServerRequestRaw(WebServerRequestEvent* event);
  static void DoNotifyExceptionOnFail(StringParam message, const u32& context, void* userData);
  static OsInt AcceptThread(void* userData);

  Thread mAcceptThread;
  Socket mAcceptSocket;
  Atomic<bool> mLogging;
  Atomic<bool> mRunning;
  ThreadLock mConnectionsLock;
  Array<WebServerConnection*> mConnections;
  CountdownEvent mConnectionCountEvent;

  // Maps the extension (without '.') to a MIME type.
  HashMap<String, String> mExtensionToMimeType;
};

/// A specialized WebServer that provides content from a given directory.
/// Content can be limited by specific file name or by filters.
/// This server can also automatically look for index files, such as
/// index.htm/index.html.
class WebServerDirectory : public WebServer
{
public:
};

} // namespace Raverie
