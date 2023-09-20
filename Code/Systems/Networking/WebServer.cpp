// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
namespace Events
{
DefineEvent(WebServerRequestRaw);
DefineEvent(WebServerRequest);
DefineEvent(WebServerUnhandledRequest);
} // namespace Events

// HTTP explicitly uses \r\n.
static const String cHttpNewline("\r\n");

// These values must correspond with the enum values in WebServerRequestMethod.
static const String cMethods[] = {"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"};

RaverieDefineType(WebServerRequestEvent, builder, type)
{
  RaverieBindFieldGetter(mWebServer);
  RaverieBindFieldGetter(mMethod);
  RaverieBindFieldGetter(mMethodString);
  RaverieBindFieldGetter(mOriginalUri);
  RaverieBindFieldGetter(mDecodedUri);
  RaverieBindFieldGetter(mPostData);
  RaverieBindFieldGetter(mData);
  RaverieBindMethod(HasHeader);
  RaverieBindMethod(GetHeaderValue);
  RaverieBindMethod(GetHeaderNames);
  RaverieBindOverloadedMethod(Respond, RaverieInstanceOverload(void, WebResponseCode::Enum, StringParam, StringParam));
  RaverieBindOverloadedMethod(Respond, RaverieInstanceOverload(void, StringParam, StringParam, StringParam));
  RaverieBindOverloadedMethod(Respond, RaverieInstanceOverload(void, StringParam));
}

WebServerRequestEvent::WebServerRequestEvent(WebServerConnection* connection) : mWebServer(connection->mWebServer), mConnection(connection), mMethod(WebServerRequestMethod::Other)
{
}

WebServerRequestEvent::~WebServerRequestEvent()
{
  if (!mConnection)
    return;

  String contents = String::Format("404 Not Found (Timestamp: %lld, Clock: %lld)", (long long)Time::GetTime(), (long long)Time::Clock());
  Respond(WebResponseCode::NotFound, String(), contents);
}

bool WebServerRequestEvent::HasHeader(StringParam name)
{
  return mHeaders.ContainsKey(name.ToLower());
}

String WebServerRequestEvent::GetHeaderValue(StringParam name)
{
  return mHeaders.FindValue(name.ToLower(), String());
}

OrderedHashMap<String, String>::KeyRange WebServerRequestEvent::GetHeaderNames()
{
  return mHeaders.Keys();
}

void WebServerRequestEvent::Respond(WebResponseCode::Enum code, StringParam extraHeaders, StringParam contents)
{
  return Respond(WebServer::GetWebResponseCodeString(code), extraHeaders, contents);
}

void WebServerRequestEvent::Respond(StringParam code, StringParam extraHeaders, StringParam contents)
{
  if (code.Empty())
  {
    DoNotifyException("WebServerRequestEvent", "A web response code was not provided (string was empty).");
    return;
  }

  if (!extraHeaders.Empty() && !extraHeaders.EndsWith(cHttpNewline))
  {
    DoNotifyException("WebServerRequestEvent", "The 'extraHeaders' was non-empty and must end with '\\r\\n'.");
    return;
  }

  StringBuilder builder;
  builder.Append("HTTP/1.1 ");
  builder.Append(code);
  builder.Append(cHttpNewline);

  // builder.Append("Date: ");
  // builder.Append(gHttpNewline);

  builder.Append("content-length: ");
  builder.AppendFormat("%llu", (unsigned long long)contents.SizeInBytes());
  builder.Append(cHttpNewline);

  builder.Append(extraHeaders);

  // At the very end we need two newlines. One is either provided before
  // extra headers, or by extra headers, and then we provide this one.
  builder.Append(cHttpNewline);

  builder.Append(contents);

  Respond(builder.ToString());
}

void WebServerRequestEvent::Respond(StringParam response)
{
  if (!mConnection)
  {
    DoNotifyException("WebServerRequestEvent", "Cannot send multiple responses to a WebServer request");
    return;
  }

  mConnection->mWriteLock.Lock();
  mConnection->mWriteData.Insert(mConnection->mWriteData.Begin(), response.Data(), response.EndData());
  mConnection->mWriteComplete = true;
  mConnection->mWriteLock.Unlock();
  mConnection->mWriteSignal.Signal();

  mConnection = nullptr;
}

WebServerConnection::WebServerConnection(WebServer* server) : mWebServer(server), mWriteComplete(false)
{
  mWriteSignal.Initialize();
}

WebServerConnection::~WebServerConnection()
{
}

// Method   : Reading GET/POST/PUT first line of the HTTP request.
// Headers  : Reading headers such as "Accept-Language: en-us\r\n".
// Content  : Reading the content length from the headers if it exists.
// PostData : Reading everything after the \r\n\r\n (should have read a content
// length).
DeclareEnum4(ReadStage, Method, Headers, Content, PostData);

OsInt WebServerConnection::ReadWriteThread(void* userData)
{
  WebServerConnection* self = (WebServerConnection*)userData;
  WebServer* webServer = self->mWebServer;

  StringBuilder builder;

  // Look for the request method line, such as "GET /index.htm HTTP/1.1\r\n"
  static const size_t cMethodMatchCount = 4;
  static const Regex cMethodRegex("^([A-Z]+)\\s+(.*)\\s+HTTP/[0-9\\.]+\r\n(\r\n)?");

  // Look for the headers, such as "Accept-Language: en-us\r\n"
  static const size_t cHeaderMatchCount = 4;
  static const Regex cHeaderRegex("^([^:]+)\\s*:\\s*([^\r\n]*)\r\n(\r\n)?");

  // Preemptively create the event so we can fill it out
  WebServerRequestEvent* toSend = new WebServerRequestEvent(self);

  bool running = true;
  int contentLength = 0;

  // This designates what part of the http request we're reading.
  ReadStage::Enum stage = ReadStage::Method;

  String unparsedContent;

  while (webServer->mRunning)
  {
    byte buffer[4096];
    Status status;
    size_t amount = self->mSocket.Receive(status, buffer, sizeof(buffer));

    // If the connection is gracefully closed, or we had an error, then
    // terminate the connection.
    if (amount == 0 || status.Failed() || !webServer->mRunning)
    {
      // Mark the event's connection as null so it doesn't try to send a 404
      // response in it's destructor.
      toSend->mConnection = nullptr;

      // Since the connection is considered terminated, we're no
      // longer running (the write thread should also shut-down).
      running = false;
      delete toSend;
      toSend = nullptr;
      break;
    }

    builder.Append((cstr)buffer, amount);

    // This isn't the most efficient way of doing this,
    // but the performance of the web server isn't critical.
    unparsedContent = BuildString(unparsedContent, String((cstr)buffer, amount));

    Matches matches;

    if (stage == ReadStage::Method)
    {
      cMethodRegex.Search(unparsedContent, matches, RegexFlags::None);

      if (matches.Size() == cMethodMatchCount)
      {
        WebServerRequestMethod::Enum method = WebServerRequestMethod::Other;
        String methodString = matches[1];
        String uri = matches[2];

        for (size_t i = 0; i < WebServerRequestMethod::Size; ++i)
        {
          if (cMethods[i] == methodString)
            method = (WebServerRequestMethod::Enum)i;
        }

        toSend->mMethod = method;
        toSend->mMethodString = methodString;
        toSend->mOriginalUri = uri;
        toSend->mDecodedUri = UrlParamDecode(uri);

        // Was this the last line in the header? We know this because we'll see
        // \r\n\r\n. Note that this is extremely unlikely to not get any other
        // headers, and only the method request line.
        if (!matches[3].Empty())
          stage = ReadStage::Content;
        else
          stage = ReadStage::Headers;

        unparsedContent = unparsedContent.SubString(matches[0].End(), unparsedContent.End());
      }
    }

    if (stage == ReadStage::Headers)
    {
      // Loop until we don't find any headers in the unparsed data.
      // Note that there could still be more incoming even when we finish this
      // loop!
      for (;;)
      {
        cHeaderRegex.Search(unparsedContent, matches, RegexFlags::None);

        if (matches.Size() == cHeaderMatchCount)
        {
          // Add the header to the event (keys are case-insensitive, and values
          // have optional whitespace).
          String key = matches[1].ToLower();
          String value = matches[2].Trim();
          toSend->mHeaders[key] = value;

          // Was this the last line in the header? We know this because we'll
          // see \r\n\r\n.
          if (!matches[3].Empty())
            stage = ReadStage::Content;

          unparsedContent = unparsedContent.SubString(matches[0].End(), unparsedContent.End());
        }
        else
        {
          // We didn't find another header.
          break;
        }
      }
    }

    if (stage == ReadStage::Content)
    {
      // We only care about post data if there was a Content-Length field.
      static const String cContentLength("content-length");
      String contentLengthString = toSend->GetHeaderValue(cContentLength);

      if (!contentLengthString.Empty())
        contentLength = atoi(contentLengthString.c_str());

      // If we didn't have the header, or for some reason the content length was
      // unparsable or 0, then we're done!
      if (contentLength == 0)
      {
        toSend->mData = builder.ToString();
        Z::gDispatch->Dispatch(webServer, Events::WebServerRequestRaw, toSend);
        toSend = nullptr;
        break;
      }
      else
      {
        // Otherwise we move onto POST data parsing.
        stage = ReadStage::PostData;
      }
    }

    if (stage == ReadStage::PostData)
    {
      // If we have enough data from the client to make-up all the post data,
      // then dispatch it!
      if ((int)unparsedContent.SizeInBytes() >= contentLength)
      {
        toSend->mData = builder.ToString();
        toSend->mPostData = unparsedContent;
        Z::gDispatch->Dispatch(webServer, Events::WebServerRequestRaw, toSend);
        toSend = nullptr;
        break;
      }
    }
  }

  ErrorIf(toSend != nullptr, "We should have sent or deleted the event by this point");

  if (running)
  {
    Array<byte> writeData;

    // Loop until we're no longer running, or an error occurs.
    for (;;)
    {
      self->mWriteSignal.Wait();

      if (!webServer->mRunning)
        break;

      // Steal any data that needs to be written.
      self->mWriteLock.Lock();
      writeData.Swap(self->mWriteData);
      running = !self->mWriteComplete;
      self->mWriteLock.Unlock();

      byte* buffer = writeData.Data();
      size_t size = writeData.Size();

      // Write out all the data we have.
      while (size != 0)
      {
        Status status;
        size_t amount = self->mSocket.Send(status, buffer, size);

        if (amount == 0 || status.Failed())
        {
          running = false;
          break;
        }

        size -= amount;
        buffer += amount;
      }

      // Clear the data but keep the buffer allocated (this makes the swap more
      // efficient).
      writeData.Clear();

      // If we wrote all the data, then we're done with this thread!
      if (!running)
        break;
    }
  }

  // Since we got here, it's time to erase/delete this connection.
  webServer->mConnectionsLock.Lock();
  webServer->mConnections.EraseValue(self);
  webServer->mConnectionCountEvent.DecrementCount();
  webServer->mConnectionsLock.Unlock();

  // Since we're done with the connection, delete it.
  // The server won't touch the connection.
  delete self;
  return 0;
}

RaverieDefineType(WebServer, builder, type)
{
  RaverieBindEvent(Events::WebServerRequest, WebServerRequestEvent);
  RaverieBindEvent(Events::WebServerUnhandledRequest, WebServerRequestEvent);
  RaverieBindMethod(Create);
  RaverieBindMethod(Host);
  RaverieBindMethod(Close);
  RaverieBindMethod(GetWebResponseCodeString);
  RaverieBindMethod(UrlParamEncode);
  RaverieBindMethod(UrlParamDecode);
  RaverieBindMethod(MapExtensionToMimeType);
  RaverieBindMethod(GetMimeTypeFromExtension);
  RaverieBindMethod(ClearMimeTypes);
  RaverieBindFieldProperty(mPath);
}

WebServer::WebServer()
{
  ConnectThisTo(this, Events::WebServerRequestRaw, OnWebServerRequestRaw);
}

WebServer::~WebServer()
{
  Close();
}

WebServer* WebServer::Create()
{
  return new WebServer();
}

bool WebServer::Host(uint port)
{
  // Close the web server if we're already running.
  Close();

  Status status;
  mAcceptSocket.Open(status, SocketAddressFamily::InternetworkV4, SocketType::Stream, SocketProtocol::Tcp);
  if (status.Failed())
    return false;

  SocketAddress address;
  address.SetIpv4(status, String(), port, SocketAddressResolutionFlags::AnyAddress);
  if (status.Failed())
    return false;

  mAcceptSocket.Bind(status, address);
  if (status.Failed())
    return false;

  mAcceptSocket.Listen(status, Socket::GetMaxListenBacklog());
  if (status.Failed())
    return false;

  mRunning = true;
  mAcceptThread.Initialize(&AcceptThread, this, "WebServerAccept");
  return true;
}

void WebServer::Close()
{
  if (!mRunning)
    return;

  mRunning = false;
  mAcceptSocket.Close();
  mAcceptThread.WaitForCompletion();
  mAcceptThread.Close();

  mConnectionsLock.Lock();
  forRange (WebServerConnection* connection, mConnections)
  {
    // Close the socket to resume from any blocking reads.
    connection->mSocket.Close();

    // Signal the writes so that we'll resume from waiting.
    connection->mWriteSignal.Signal();
  }
  mConnectionsLock.Unlock();

  // Wait until all the connections are finished.
  mConnectionCountEvent.Wait();
}

String WebServer::GetWebResponseCodeString(WebResponseCode::Enum code)
{
  switch (code)
  {
  case WebResponseCode::Continue:
    return "100 Continue"; // 100
  case WebResponseCode::SwitchingProtocols:
    return "101 Switching Protocols"; // 101
  case WebResponseCode::OK:
    return "200 OK"; // 200
  case WebResponseCode::Created:
    return "201 Created"; // 201
  case WebResponseCode::Accepted:
    return "202 Accepted"; // 202
  case WebResponseCode::NonauthoritativeInformation:
    return "203 Non-Authoritative Information"; // 203
  case WebResponseCode::NoContent:
    return "204 No Content"; // 204
  case WebResponseCode::ResetContent:
    return "205 Reset Content"; // 205
  case WebResponseCode::PartialContent:
    return "206 Partial Content"; // 206
  case WebResponseCode::MovedPermanently:
    return "301 Moved Permanently"; // 301
  case WebResponseCode::ObjectMovedTemporarily:
    return "302 Found"; // 302
  case WebResponseCode::SeeOther:
    return "303 See Other"; // 303
  case WebResponseCode::NotModified:
    return "304 Not Modified"; // 304
  case WebResponseCode::TemporaryRedirect:
    return "307 Temporary Redirect"; // 307
  case WebResponseCode::PermanentRedirect:
    return "308 Permanent Redirect"; // 308
  case WebResponseCode::BadRequest:
    return "400 Bad Request"; // 400
  case WebResponseCode::AccessDenied:
    return "401 Unauthorized"; // 401
  case WebResponseCode::Forbidden:
    return "403 Forbidden"; // 403
  case WebResponseCode::NotFound:
    return "404 Not Found"; // 404
  case WebResponseCode::HTTPVerbNotAllowed:
    return "405 Method Not Allowed"; // 405
  case WebResponseCode::ClientBrowserRejectsMIME:
    return "406 Not Acceptable"; // 406
  case WebResponseCode::ProxyAuthenticationRequired:
    return "407 Proxy Authentication Required"; // 407
  case WebResponseCode::PreconditionFailed:
    return "412 Precondition Failed"; // 412
  case WebResponseCode::RequestEntityTooLarge:
    return "413 Payload Too Large"; // 413
  case WebResponseCode::RequestURITooLarge:
    return "414 URI Too Long"; // 414
  case WebResponseCode::UnsupportedMediaType:
    return "415 Unsupported Media Type"; // 415
  case WebResponseCode::RequestedRangeNotSatisfiable:
    return "416 Requested Range Not Satisfiable"; // 416
  case WebResponseCode::ExecutionFailed:
    return "417 Expectation Failed"; // 417
  case WebResponseCode::LockedError:
    return "423 Locked"; // 423
  case WebResponseCode::InternalServerError:
    return "500 Internal Server Error"; // 500
  case WebResponseCode::UnimplementedHeaderValueUsed:
    return "501 Not Implemented"; // 501
  case WebResponseCode::GatewayProxyReceivedInvalid:
    return "502 Bad Gateway"; // 502
  case WebResponseCode::ServiceUnavailable:
    return "503 Service Unavailable"; // 503
  case WebResponseCode::GatewayTimedOut:
    return "504 Gateway Timeout"; // 504
  case WebResponseCode::HTTPVersionNotSupported:
    return "505 HTTP Version Not Supported"; // 505
  default:
    return String();
  }
}

String WebServer::UrlParamEncode(StringParam string)
{
  return Raverie::UrlParamEncode(string);
}

String WebServer::UrlParamDecode(StringParam string)
{
  return Raverie::UrlParamDecode(string);
}

String StripDotFromExtension(StringParam extension)
{
  if (extension.StartsWith("."))
    return extension.SubString(extension.Begin() + 1, extension.End());
  return extension;
}

void WebServer::MapExtensionToMimeType(StringParam extension, StringParam mimeType)
{
  String extensionWithoutDot = StripDotFromExtension(extension);
  mExtensionToMimeType[extensionWithoutDot] = mimeType;
}

String WebServer::GetMimeTypeFromExtension(StringParam extension)
{
  String extensionWithoutDot = StripDotFromExtension(extension);
  return mExtensionToMimeType.FindValue(extensionWithoutDot, String());
}

void WebServer::ClearMimeTypes()
{
  mExtensionToMimeType.Clear();
}

String WebServer::SanitizeForHtml(StringParam text)
{
  StringBuilder builder;
  forRange (Rune rune, text)
  {
    switch (rune.value)
    {
    case '&':
      builder.Append("&amp;");
      break;
    case '<':
      builder.Append("&lt;");
      break;
    case '>':
      builder.Append("&gt;");
      break;
    case '"':
      builder.Append("&quot;");
      break;
    case '\'':
      builder.Append("&#39;");
      break;
    default:
      builder.Append(rune);
      break;
    }
  }
  String result = builder.ToString();
  return result;
}

void WebServer::OnWebServerRequestRaw(WebServerRequestEvent* event)
{
  // First let the user try and handle it.
  DispatchEvent(Events::WebServerRequest, event);

  // If the users responded to the event then the connection would be null,
  // nothing else for us to do!
  if (!event->mConnection)
    return;

  // If the user specified a mapped path, then look for files or directories
  // there.
  if (!mPath.Empty())
  {
    // Turn the URI into a relative file path by using an un-rooted URI and
    // replacing the slashes with our os path separator.
    String localPath = FilePath::Normalize(FilePath::Combine(mPath, event->mDecodedUri));

    // If we have a file on disk, attempt to open it so we can send it.
    if (FileExists(localPath))
    {
      String fileData = ReadFileIntoString(localPath.c_str());
      String headers;

      // If we have a MIME type for the file, then let the requester know.
      String mimeType = GetMimeTypeFromExtension(FilePath::GetExtension(localPath));
      if (!mimeType.Empty())
        headers = BuildString("Content-Type: ", mimeType, "\r\n");

      event->Respond(WebResponseCode::OK, headers, fileData);
    }
    else if (DirectoryExists(localPath))
    {
      String uriWithSlash = event->mDecodedUri;
      if (!uriWithSlash.EndsWith("/"))
        uriWithSlash = BuildString(uriWithSlash, "/");

      // Build an HTML page that shows the name of the current directory and
      // then lists all files.
      String sanitizedServerPath = SanitizeForHtml(uriWithSlash);
      StringBuilder builder;
      builder.Append("<html><head><title>");
      builder.Append(sanitizedServerPath);
      builder.Append("</title></head><body><h1>");
      builder.Append(sanitizedServerPath);
      builder.Append("</h1>");

      // Sort the file names so they'll be in order.
      Array<String> sortedFileNames;

      // Add this so users can go back one directory.
      sortedFileNames.PushBack("..");

      for (FileRange fileRange(localPath); !fileRange.Empty(); fileRange.PopFront())
        sortedFileNames.PushBack(fileRange.Front());
      Sort(sortedFileNames.All());

      // Build the listing for each file in sorted order.
      forRange (StringParam fileName, sortedFileNames)
      {
        String serverPath = BuildString(uriWithSlash, fileName);

        builder.Append("<a href='");
        builder.Append(serverPath);
        builder.Append("'>");
        builder.Append(SanitizeForHtml(fileName));
        builder.Append("</a><br>");
      }

      builder.Append("</body></html>");
      String html = builder.ToString();
      event->Respond(WebResponseCode::OK, String(), html);
    }
  }

  // If we don't have a connection, it means we responded already.
  if (!event->mConnection)
    return;

  // One last chance for the user to handle the request before it automatically
  // generates a 404.
  DispatchEvent(Events::WebServerUnhandledRequest, event);
}

void WebServer::DoNotifyExceptionOnFail(StringParam message, const u32& context, void* userData)
{
  DoNotifyException("WebServer", message);
}

OsInt WebServer::AcceptThread(void* userData)
{
  WebServer* self = (WebServer*)userData;

  while (self->mRunning)
  {
    Socket acceptedSocket;

    Status status;
    self->mAcceptSocket.Accept(status, &acceptedSocket);

    // If we got a valid socket then throw it on the connections list and start
    // up threads for the socket.
    if (status.Succeeded() && acceptedSocket.IsOpen())
    {
      WebServerConnection* connection = new WebServerConnection(self);
      connection->mSocket = RaverieMove(acceptedSocket);

      connection->mReadWriteThread.Initialize(&WebServerConnection::ReadWriteThread, connection, "WebServerConnectionReadWrite");

      self->mConnectionsLock.Lock();
      self->mConnections.PushBack(connection);
      self->mConnectionCountEvent.IncrementCount();
      self->mConnectionsLock.Unlock();
    }
  }

  return 0;
}

} // namespace Raverie
