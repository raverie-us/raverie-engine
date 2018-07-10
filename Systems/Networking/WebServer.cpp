///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg.
/// Copyright 2018, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(WebServerRequest);
}

// HTTP explicitly uses \r\n.
static const String cHttpNewline("\r\n");

// These values must correspond with the enum values in WebServerRequestMethod.
static const String cMethods[] =
{
  "OPTIONS",
  "GET",
  "HEAD",
  "POST",
  "PUT",
  "DELETE",
  "TRACE",
  "CONNECT",
  "OTHER"
};

ZilchDefineType(WebServerRequestEvent, builder, type)
{
  ZilchBindFieldProperty(mData);
  ZilchBindFieldProperty(mResponse);
  ZilchBindMethod(HasHeader);
  ZilchBindMethod(GetHeaderValue);
  ZilchBindMethod(GetHeaderNames);
  ZilchBindOverloadedMethod(Respond, ZilchInstanceOverload(void, Os::WebResponseCode::Type, StringParam, StringParam));
  ZilchBindOverloadedMethod(Respond, ZilchInstanceOverload(void, StringParam, StringParam, StringParam));
}

bool WebServerRequestEvent::HasHeader(StringParam name)
{
  return mHeaders.ContainsKey(name);
}

String WebServerRequestEvent::GetHeaderValue(StringParam name)
{
  return mHeaders.FindValue(name, String());
}

OrderedHashMap<String, String>::KeyRange WebServerRequestEvent::GetHeaderNames()
{
  return mHeaders.Keys();
}

void WebServerRequestEvent::Respond(Os::WebResponseCode::Type code, StringParam extraHeaders, StringParam contents)
{
  return Respond(WebServer::GetWebResponseCodeString(code), extraHeaders, contents);
}

void WebServerRequestEvent::Respond(StringParam code, StringParam extraHeaders, StringParam contents)
{
  if (code.Empty())
  {
    DoNotifyException("WebServerEvent", "A web response code was not provided (string was empty).");
    return;
  }

  if (!extraHeaders.Empty() && !extraHeaders.EndsWith(cHttpNewline))
  {
    DoNotifyException("WebServerEvent", "The 'extraHeaders' was non-empty and must end with '\\r\\n'.");
    return;
  }

  StringBuilder builder;
  builder.Append("HTTP/1.1 ");
  builder.Append(code);
  builder.Append(cHttpNewline);

  //builder.Append("Date: ");
  //builder.Append(gHttpNewline);

  builder.Append("Content-Length: ");
  builder.AppendFormat("%llu", (unsigned long long)contents.SizeInBytes());
  builder.Append(cHttpNewline);
  
  builder.Append(extraHeaders);

  // At the very end we need two newlines. One is either provided before
  // extra headers, or by extra headers, and then we provide this one.
  builder.Append(cHttpNewline);

  builder.Append(contents);

  mResponse = builder.ToString();
}

WebServerConnection::~WebServerConnection()
{
  // The destructor is only ever called by the WebServer, and we know mRunning on the
  // WebServer will be set to false when closing all connections, so we can wait on our threads.
  mReadThread.WaitForCompletion();
  mWriteThread.WaitForCompletion();
}

// Method   : Reading GET/POST/PUT first line of the HTTP request.
// Headers  : Reading headers such as "Accept-Language: en-us\r\n".
// Content  : Reading the content length from the headers if it exists.
// PostData : Reading everything after the \r\n\r\n (should have read a content length).
DeclareEnum4(ReadStage, Method, Headers, Content, PostData);

OsInt WebServerConnection::ReadThread(void* userData)
{
  WebServerConnection* self = (WebServerConnection*)userData;

  StringBuilder builder;

  // Look for the request method line, such as "GET /index.htm HTTP/1.1\r\n"
  static const size_t cMethodMatchCount = 4;
  static const Regex cMethodRegex("^([A-Z]+)\\s+(.*)\\s+HTTP/[0-9\\.]+\r\n(\r\n)?");

  // Look for the headers, such as "Accept-Language: en-us\r\n"
  static const size_t cHeaderMatchCount = 4;
  static const Regex cHeaderRegex("^([^:]+)\\s*:\\s*([^\r\n]*)\r\n(\r\n)?");

  // Preemptively create the event so we can fill it out
  WebServerRequestEvent* toSend = new WebServerRequestEvent();

  int contentLength = 0;

  // This designates what part of the http request we're reading.
  ReadStage::Enum stage = ReadStage::Method;

  String unparsedContent;
  
  while (self->mRunning && self->mServer->mRunning)
  {
    byte buffer[4096];
    Status status;
    size_t amount = self->mSocket.Receive(status, buffer, sizeof(buffer));

    // If the connection is gracefully closed, or we had an error, then terminate the connection.
    if (amount == 0 || status.Failed())
    {
      // Since the connection is considered terminated, we're no 
      // longer running (the write thread should also shut-down).
      self->mRunning = false;
      delete toSend;
      return 0;
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
        toSend->mUri = uri;

        // Was this the last line in the header? We know this because we'll see \r\n\r\n.
        // Note that this is extremely unlikely to not get any other headers, and only the method request line.
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
      // Note that there could still be more incoming even when we finish this loop!
      for (;;)
      {
        cHeaderRegex.Search(unparsedContent, matches, RegexFlags::None);

        if (matches.Size() == cHeaderMatchCount)
        {
          // Add the header to the event.
          String key = matches[1];
          String value = matches[2];
          toSend->mHeaders[key] = value;

          // Was this the last line in the header? We know this because we'll see \r\n\r\n.
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
      String contentLengthString = toSend->mHeaders.FindValue("Content-Length", String());

      if (!contentLengthString.Empty())
        contentLength = atoi(contentLengthString.c_str());

      // If we didn't have the header, or for some reason the content length was unparsable or 0, then we're done!
      if (contentLength == 0)
      {
        Z::gDispatch->Dispatch(self->mServer, Events::WebServerRequest, toSend);
        return 0;
      }
      else
      {
        // Otherwise we move onto POST data parsing.
        stage = ReadStage::PostData;
      }
    }

    if (stage == ReadStage::PostData)
    {
      // If we have enough data from the client to make-up all the post data, then dispatch it!
      if ((int)unparsedContent.SizeInBytes() >= contentLength)
      {
        Z::gDispatch->Dispatch(self->mServer, Events::WebServerRequest, toSend);
        return 0;
      }
    }
  }

  // If we got here the server stopped running (which means we're also not running).
  self->mRunning = false;
  delete toSend;
  return 0;
}

OsInt WebServerConnection::WriteThread(void* userData)
{
  WebServerConnection* self = (WebServerConnection*)userData;
  return 0;
}

ZilchDefineType(WebServer, builder, type)
{
  ZeroBindEvent(Events::WebServerRequest, WebServerRequestEvent);
  ZilchBindMethod(Create);
  ZilchBindMethod(Host);
  ZilchBindMethod(Close);
  ZilchBindMethod(GetWebResponseCodeString);
}

WebServer::WebServer()
{
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

  // We don't need to synchronize on mConnections because we know the accept thread is stopped.
  forRange(WebServerConnection* connection, mConnections)
  {
    delete connection;
  }
}

String WebServer::GetWebResponseCodeString(Os::WebResponseCode::Type code)
{
  switch (code)
  {
    case Os::WebResponseCode::Continue                      : return "100 Continue"; // 100
    case Os::WebResponseCode::SwitchingProtocols            : return "101 Switching Protocols"; // 101
    case Os::WebResponseCode::OK                            : return "200 OK"; // 200
    case Os::WebResponseCode::Created                       : return "201 Created"; // 201
    case Os::WebResponseCode::Accepted                      : return "202 Accepted"; // 202
    case Os::WebResponseCode::NonauthoritativeInformation   : return "203 Non-Authoritative Information"; // 203
    case Os::WebResponseCode::NoContent                     : return "204 No Content"; // 204
    case Os::WebResponseCode::ResetContent                  : return "205 Reset Content"; // 205
    case Os::WebResponseCode::PartialContent                : return "206 Partial Content"; // 206
    case Os::WebResponseCode::MovedPermanently              : return "301 Moved Permanently"; // 301
    case Os::WebResponseCode::ObjectMovedTemporarily        : return "302 Found"; // 302
    case Os::WebResponseCode::SeeOther                      : return "303 See Other"; // 303
    case Os::WebResponseCode::NotModified                   : return "304 Not Modified"; // 304
    case Os::WebResponseCode::TemporaryRedirect             : return "307 Temporary Redirect"; // 307
    case Os::WebResponseCode::PermanentRedirect             : return "308 Permanent Redirect"; // 308
    case Os::WebResponseCode::BadRequest                    : return "400 Bad Request"; // 400
    case Os::WebResponseCode::AccessDenied                  : return "401 Unauthorized"; // 401
    case Os::WebResponseCode::Forbidden                     : return "403 Forbidden"; // 403
    case Os::WebResponseCode::NotFound                      : return "404 Not Found"; // 404
    case Os::WebResponseCode::HTTPVerbNotAllowed            : return "405 Method Not Allowed"; // 405
    case Os::WebResponseCode::ClientBrowserRejectsMIME      : return "406 Not Acceptable"; // 406
    case Os::WebResponseCode::ProxyAuthenticationRequired   : return "407 Proxy Authentication Required"; // 407
    case Os::WebResponseCode::PreconditionFailed            : return "412 Precondition Failed"; // 412
    case Os::WebResponseCode::RequestEntityTooLarge         : return "413 Payload Too Large"; // 413
    case Os::WebResponseCode::RequestURITooLarge            : return "414 URI Too Long"; // 414
    case Os::WebResponseCode::UnsupportedMediaType          : return "415 Unsupported Media Type"; // 415
    case Os::WebResponseCode::RequestedRangeNotSatisfiable  : return "416 Requested Range Not Satisfiable"; // 416
    case Os::WebResponseCode::ExecutionFailed               : return "417 Expectation Failed"; // 417
    case Os::WebResponseCode::LockedError                   : return "423 Locked"; // 423
    case Os::WebResponseCode::InternalServerError           : return "500 Internal Server Error"; // 500
    case Os::WebResponseCode::UnimplementedHeaderValueUsed  : return "501 Not Implemented"; // 501
    case Os::WebResponseCode::GatewayProxyReceivedInvalid   : return "502 Bad Gateway"; // 502
    case Os::WebResponseCode::ServiceUnavailable            : return "503 Service Unavailable"; // 503
    case Os::WebResponseCode::GatewayTimedOut               : return "504 Gateway Timeout"; // 504
    case Os::WebResponseCode::HTTPVersionNotSupported       : return "505 HTTP Version Not Supported"; // 505
    default: return String();
  }
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

    // If we got a valid socket then throw it on the connections list and start up threads for the socket.
    if (status.Succeeded() && acceptedSocket.IsOpen())
    {
      WebServerConnection* connection = new WebServerConnection();
      connection->mRunning = true;
      connection->mSocket = ZeroMove(acceptedSocket);
      connection->mServer = self;

      connection->mReadThread.Initialize(&WebServerConnection::ReadThread, connection, "WebServerConnectionRead");
      connection->mWriteThread.Initialize(&WebServerConnection::WriteThread, connection, "WebServerConnectionWrite");

      self->mConnectionsLock.Lock();
      self->mConnections.PushBack(connection);
      self->mConnectionsLock.Unlock();
    }
  }

  return 0;
}

} // namespace Zero
