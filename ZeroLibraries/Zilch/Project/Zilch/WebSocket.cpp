/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

// Using directives
using namespace Zero;

// Note: The entire implementation here is the WebSocket protocol:
// https://tools.ietf.org/html/rfc6455

namespace Zilch
{
  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(WebSocketAcceptedConnection);
    ZilchDefineEvent(WebSocketReceivedData);
    ZilchDefineEvent(WebSocketError);
    ZilchDefineEvent(WebSocketDisconnected);
  }

  //***************************************************************************
  ZilchDefineStaticLibrary(WebSockets)
  {
    ZilchInitializeType(WebSocketEvent);
  }

  //***************************************************************************
  ZilchDefineType(WebSocketEvent, builder, type)
  {
  }

  //***************************************************************************
  WebSocketEvent::WebSocketEvent() :
    Connection(nullptr),
    PacketType(WebSocketPacketType::Invalid)
  {
  }

  //***************************************************************************
  BlockingWebSocketConnection::BlockingWebSocketConnection()
  {
  }
  
  //***************************************************************************
  void BlockingWebSocketConnection::SendFullPacket(Status& status, const byte* data, size_t length, WebSocketPacketType::Enum packetType)
  {
    // The header has a maximum size without the mask, lets build that header
    byte header[10] = {0};
    size_t headerSize = 0;

    // Write the first two bytes (flags / opcode, and the payload)
    // Since we're always sending a full packet (not truncated or fragments) then this is always a FIN packet
    header[0] |= 0x80;

    // Set the opcode bit depending on if we're in text or binary or some other control opcode
    header[0] |= (byte)packetType;

    // We don't bother to set the mask bit, so just set the second byte as the size (and onward if we need it)
    if (length < 126)
    {
      header[1] = (byte)length;
      headerSize = 2;
    }
    // If we're using 2 bytes to describe the size (1 byte just to say we're using 2 bytes...)
    else if (length <= 0xFFFF)
    {
      header[1] = 126;

      *((unsigned short*)(header + 2)) = NetworkFlip((unsigned short)length);
      headerSize = 4;
    }
    // If we're using 8 bytes to describe the size (1 byte just to say we're using 8 bytes...)
    else
    {
      header[1] = 127;
      *((unsigned long long*)(header + 2)) = NetworkFlip((unsigned long long)length);
      headerSize = 10;
    }

    // Send the header off first (this will block until it sends the entire thing!)
    int sentSizeHeader = this->RemoteSocket.Send(status, header, (int)headerSize, SocketFlags::None);
    ErrorIf(sentSizeHeader != 0 && sentSizeHeader != (int)headerSize, "We should always send the entire header, send should block until its all sent!");
    if (status.Failed())
      return;

    // Now send the rest of the payload data (could be done in one send, no problem though!)
    int sentSizeData = this->RemoteSocket.Send(status, data, (int)length, SocketFlags::None);
    ErrorIf(sentSizeData != 0 && sentSizeData != (int)length, "We should always send the entire data, send should block until its all sent!");
    if (status.Failed())
      return;
  }
  
  //***************************************************************************
  bool BlockingWebSocketConnection::IsValid()
  {
    return this->RemoteSocket.IsOpen();
  }

  //***************************************************************************
  WebSocketPacketType::Enum BlockingWebSocketConnection::ReceiveFullPacket(Status& status, String& dataOut)
  {
    // All the information we need for a web-socket header
    bool readHeader = false;
    size_t payloadSize = 0;
    char mask[4] = {0};
    bool fin = false;
    bool rsv1 = false;
    bool rsv2 = false;
    bool rsv3 = false;
    WebSocketPacketType::Enum opcode = WebSocketPacketType::Invalid;

    // Clear the data, just so that if the user is reusing a buffer, they don't get confused
    dataOut.Clear();

    // Read until we hit a full packet (return inside loop below)
    for (;;)
    {
      // The default buffer size we use (big enough for a tcp window)
      const int ReceiveBufferSize = 4096;

      // Read the data from the socket
      byte buffer[ReceiveBufferSize];
      int amountReceived = this->RemoteSocket.Receive(status, buffer, ReceiveBufferSize, SocketFlags::None);
      
      // If the receive call failed, or we gracefully disconnected...
      if (status.Failed() || amountReceived == 0)
        return WebSocketPacketType::Invalid;

      // Add the data to a remaining buffer
      this->ReadData.Insert(this->ReadData.End(), buffer, buffer + amountReceived);
      byte* data = this->ReadData.Data();

      // The minimum amount of data a packet can be is 6, but the header can be larger because of extended sizes
      // This includes the opcode/starting bits (1), the payload length (1), and the mask (4)
      if (readHeader == false && this->ReadData.Size() >= 6)
      {
        // First, read the header byte which tells us all the options
        byte headerByte = data[0];
        fin  = (headerByte & 0x80) != 0;
        rsv1 = (headerByte & 0x40) != 0;
        rsv2 = (headerByte & 0x20) != 0;
        rsv3 = (headerByte & 0x10) != 0;
        opcode = (WebSocketPacketType::Enum)(headerByte & 0x0F);
        
        // Read the size of our packet
        byte sizeByte = data[1];

        // The highest bit is always set, basically clear it
        bool masking = (sizeByte & 0x80) != 0;
        sizeByte &= 0x7F;
        
        // We read the opcode/bits and the payload length, which is 2 bytes
        size_t position = 2;
        payloadSize = sizeByte;

        // Assume right now that we successfully read the header (we may still fail below if we don't have enough header data)
        readHeader = true;
        
        // If the size is 126, then it means we have an extra 16 bits of data (2 bytes)
        if (sizeByte == 126)
        {
          // We're reading an extended payload size of 2 bytes, which means we must have at least 8 bytes (6 + 2) of data in total for the whole header
          if (this->ReadData.Size() >= 8)
          {
            payloadSize = NetworkFlip((unsigned short)(data[2] + (data[3] << 8)));
            position = 4;
          }
          else
          {
            // We couldn't read the extended payload, because we didn't have enough data read for it
            readHeader = false;
          }
        }
        // If the size is 127, then we have an extra 64 bits of data (8 bytes)
        else if (sizeByte == 127)
        {
          // We're reading an extended payload size of 8 bytes, which means we must have at least 14 bytes (6 + 8) of data in total for the whole header
          if (this->ReadData.Size() >= 14)
          {
            payloadSize = (size_t)NetworkFlip((unsigned long long)(data[2] + (data[3] << 8) + (data[4] << 16) + (data[5] << 24) + ((unsigned long long)data[6] << 32) + ((unsigned long long)data[7] << 40) + ((unsigned long long)data[8] << 48) + ((unsigned long long)data[9] << 56)));
            position = 10;
          }
          else
          {
            // We couldn't read the extended payload, because we didn't have enough data read for it
            readHeader = false;
          }
        }

        // If we successfully read in all the header data...
        if (readHeader)
        {
          // Last but not least, read the mask!
          memcpy(mask, data + position, 4);
          position += 4;

          // Remove the header from the data
          this->ReadData.Erase(Array<byte>::range(data, data + position));
        }
      }

      // If we already read the header and we have enough data to read the entire packet...
      if (readHeader && this->ReadData.Size() >= payloadSize)
      {
        // Get a pointer to the data (for convenience)
        byte* readData = this->ReadData.Data();

        // Directly create a string node that we'll Assign to a string (where we copy our data into)
        Zero::StringNode* node = String::AllocateNode(payloadSize);

        // Loop through the payload and apply the mask to it
        for (size_t i = 0; i < payloadSize; ++i)
        {
          // Grab the current mask character (rotates by 4 over and over)
          // Apply this mask to the current byte we're reading, and put it into the user output
          char maskChar = mask[i % 4];
          node->Data[i] = readData[i] ^ maskChar;
        }

        // Now output the string from the node we just created
        dataOut = String(node);

        // We're done, we read a full packet, remove the data we read so that the next packet can be processed
        this->ReadData.Erase(Array<byte>::range(readData, readData + payloadSize));
        return opcode;
      }
    }
  }

  //***************************************************************************
  BlockingWebSocketListener::BlockingWebSocketListener()
  {
  }
  
  //***************************************************************************
  void BlockingWebSocketListener::Initialize(Status& status, int port)
  {
    // Web sockets are strictly TCP
    this->ListenerSocket.Open(status, SocketAddressFamily::InternetworkV4, SocketType::Stream, SocketProtocol::Tcp);

    // First create a local socket address, bound to any network adapter (let the OS choose)
    // Use the port that the user passed in
    SocketAddress localAddress;
    localAddress.SetIpv4(status, String(), ushort(port), SocketAddressResolutionFlags::AnyAddress);

    if (status.Failed())
      return;

    // Next, bind our listener socket to that address
    this->ListenerSocket.Bind(status, localAddress);

    if (status.Failed())
      return;

    // Now listen on the socket, which should allow incoming connections to be accepted
    this->ListenerSocket.Listen(status, Socket::GetMaxListenBacklog());
  }
  
  //***************************************************************************
  void BlockingWebSocketListener::Close(Status& status)
  {
    this->ListenerSocket.Close(status);
  }

  //***************************************************************************
  bool BlockingWebSocketListener::IsValid()
  {
    return this->ListenerSocket.IsOpen();
  }

  //***************************************************************************
  void BlockingWebSocketListener::Accept(Status& status, BlockingWebSocketConnection& connectionOut)
  {
    // Attempt to accept a connection
    this->ListenerSocket.Accept(status, &connectionOut.RemoteSocket);

    if (status.Failed())
      return;

    // The default buffer size we use (big enough for a tcp window)
    const int ReceiveBufferSize = 4096;

    // Even though it's not the most efficient approach, we're going to take all data we receive
    // and stuff it into this array before we try and parse it (keeps data contiguous)
    // A better data structure would be able to work with the temporary received data, and then
    // only Append what isn't read at the end
    Array<byte> remainingHeaderData;

    // Whether or not we read the full header (denoted when we read two newlines in a row)
    bool readFullHttpHeader = false;

    // We've accepted a connection, but we don't know if this is a proper websocket yet...
    do
    {
      // Read the data from the socket
      byte buffer[ReceiveBufferSize];
      int amountReceived = connectionOut.RemoteSocket.Receive(status, buffer, ReceiveBufferSize, SocketFlags::None);
      
      // If the receive call failed, or we gracefully disconnected...
      if (status.Failed() || amountReceived == 0)
        return;

      // Add the data to a remaining buffer
      remainingHeaderData.Insert(remainingHeaderData.End(), buffer, buffer + amountReceived);

      // If we process the get request header
      bool getRequest = false;

      // Since the HTTP protocol is line based, store the last line we stopped on
      const char* lastLineStart = (char*)remainingHeaderData.Data();

      // Get a pointer to the remaining data, for convenience
      char* data = (char*)remainingHeaderData.Data();

      // Track whether we already hit a newline (used to break out from the HTTP request - two newlines)
      bool justHitNewline = false;

      // Loop through remaining data...
      for (size_t i = 0; i < remainingHeaderData.Size(); ++i)
      {
        // Read a single character
        char c = data[i];

        // If the character is a newline...
        if (c == '\n')
        {
          // If we hit a newline now and we had just hit a newline before...
          if (justHitNewline)
          {
            // We read the entire HTTP header!
            readFullHttpHeader = true;
            break;
          }

          // Mark that we just hit a newline (gets reset in the 'else' clause below)
          justHitNewline = true;

          // Create a range that points from the start of the current line to the end (where we are now, the newline!)
          StringRange range(lastLineStart, data + i);

          // If we have yet to read that this is a get-request...
          if (getRequest == false)
          {
            // Read the get-request line
            if (String::StartsWith(range, "GET / HTTP/1.1"))
            {
              getRequest = true;
            }
            else
            {
              // This is NOT a get request, or it's an old version!
              connectionOut.RemoteSocket.Close(status);

              // Return that the http request was not valid (regardless of if closing the socket failed)
              status.SetFailed("Not a valid HTTP 1.1 client request");
              return;
            }
          }
          else
          {
            // Look for the HTTP header divider character ':'
            StringRange foundDivider = range.FindFirstOf(Zero::Rune(':'));
            if (!foundDivider.Empty())
            {
              // Parse the key and value part of the header line
              // The +2 is for the ':' and the space after it
              StringRange httpHeaderKey(range.Begin(), foundDivider.Begin());
              StringRange httpHeaderValue(foundDivider.End(), range.End());

              // Trim the trailing '\r' if it exists (it should, but some non-compliant browsers don't send it)
              if (httpHeaderValue.Back() == '\r')
                httpHeaderValue.PopBack();

              // Finally, map the key to the value so we can lookup any header values later
              connectionOut.Headers.Insert(httpHeaderKey, httpHeaderValue);
            }
            else
            {
              // We got some bad data, or just something we didn't yet handle!
              status.SetFailed(BuildString("Invalid line found in the http header request: ", range));
              return;
            }
          }

          // Skip the newline character (hence the +1)
          lastLineStart = data + i + 1;
        }
        else if (c != '\r')
        {
          // As long as we're not hitting the carriage return, mark this as no longer a newline
          justHitNewline = false;
        }
      }

      // Erase all the data we processed already
      remainingHeaderData.Erase(Array<byte>::range((byte*)data, (byte*)lastLineStart));
    }
    // Loop until we read the full header
    while (readFullHttpHeader == false);


    // A special guid appended to the 'accept' message that the server sends back (just to identify web sockets)
    static const String ServerGuid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    static const String SecWebSocketKey("Sec-WebSocket-Key");

    // Make sure we got a web socket key request
    String* key = connectionOut.Headers.FindPointer(SecWebSocketKey);
    if (key == nullptr)
    {
      status.SetFailed("We did not receive the 'Sec-WebSocket-Key' parameter, and "
                       "therefore the connection was not a valid web socket connection");
      return;
    }

    // As per RFC-6455 (page 7)
    // We take the above guid and concatenate it with the given key
    String concatenatedKey = BuildString(*key, ServerGuid);

    // Get the sha1 on of the concatenated key
    byte finalSha1[Sha1Builder::Sha1ByteSize];
    Sha1Builder sha1Builder;
    sha1Builder.Append((byte*)concatenatedKey.Data(), concatenatedKey.SizeInBytes());
    sha1Builder.OutputHash(finalSha1);

    // Lastly, we need to base64 encode the sha1 hash
    String base64Encoded = Base64Encoder::Encode(finalSha1, Sha1Builder::Sha1ByteSize);

    // Send the response to the web socket request
    String response = BuildString
    (
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: ",
      base64Encoded,
      "\r\n\r\n"
    );
    
    // This should be the last thing we have to send!
    connectionOut.RemoteSocket.Send(status, (byte*)response.Data(), (int)response.SizeInBytes(), SocketFlags::None);
  }
  
  //***************************************************************************
  void ThreadedWebSocketConnection::SendPacket(StringParam message, WebSocketPacketType::Enum packetType)
  {
    // First initiale the lock around the send messages queue, Append the message
    // then unlock it so the 'SendThread' can resume its work
    this->SendLock.Lock();

    // We just use the event structure because it has all the data we need
    WebSocketEvent& toSend = this->SendMessages.PushBack();
    toSend.Data = message;
    toSend.PacketType = packetType;

    // We need to tell the send thread that it now has data to send (unblocks it)
    // This MUST happen inside the locks, otherwise it's possibly to consume
    // all messages in the send thread before the singal, and cause the send thread to exit
    this->SendEvent.Signal();
    this->SendLock.Unlock();
  }

  //***************************************************************************
  ThreadedWebSocketConnection::ThreadedWebSocketConnection()
  {
  }

  //***************************************************************************
  ThreadedWebSocketConnection::~ThreadedWebSocketConnection()
  {
    // Closing automatically terminates threads and the socket
    // It's also safe to call more than once
    this->Close();
  }

  //***************************************************************************
  bool ThreadedWebSocketConnection::IsValid()
  {
    return this->BlockingConnection.IsValid();
  }
  
  //***************************************************************************
  void ThreadedWebSocketConnection::Close()
  {
    // If the connection is already closed... early out
    if (this->IsValid() == false)
      return;

    // Shutdown the remote connection, which should signal that we're ending the connection
    WebSocketEvent shutdownEvent;
    shutdownEvent.Connection = this;
    this->BlockingConnection.RemoteSocket.Shutdown(shutdownEvent.ErrorStatus, SocketIo::Both);

    // Close the remote connection
    WebSocketEvent closeEvent;
    closeEvent.Connection = this;
    this->BlockingConnection.RemoteSocket.Close(closeEvent.ErrorStatus);

    // Clear out the messages to be sent
    this->SendLock.Lock();
    this->SendMessages.Clear();
    this->SendLock.Unlock();

    // Signal the send event, if it gets signaled and there's no messages, it means we're closing
    this->SendEvent.Signal();
    this->SendThread.WaitForCompletion();
    
    // Wait until the receive thread ends...
    this->ReceiveThread.WaitForCompletion();

    // If the shutdown status failed for any reason, dispatch an event out
    if (shutdownEvent.ErrorStatus.Failed())
      EventSend(this, Events::WebSocketError, &shutdownEvent);

    // If the close status failed for any reason, dispatch an event out
    if (closeEvent.ErrorStatus.Failed())
      EventSend(this, Events::WebSocketError, &closeEvent);

    // After the entire closure of the socket, send out a disconnected event
    WebSocketEvent disconnectEvent;
    disconnectEvent.Connection = this;
    EventSend(this, Events::WebSocketDisconnected, &disconnectEvent);
  }

  //***************************************************************************
  void ThreadedWebSocketConnection::Update()
  {
    // Lock the recieve buffer and bring all messages to the owning recieve message array
    this->IncomingLock.Lock();
    this->OwnerIncomingEvents.Swap(this->ThreadIncomingEvents);
    this->IncomingLock.Unlock();
    
    // Walk through all messages and deliver them to the user
    for (size_t i = 0; i < this->OwnerIncomingEvents.Size(); ++i)
    {
      // Grab the current message and send out a received event
      WebSocketEvent* event = &this->OwnerIncomingEvents[i];

      // Depending on what happened, send out either an error message or a received data message
      // Note: Errors can originate both from the send and receive thread
      if (event->ErrorStatus.Failed())
      {
        // Only send out the error if its a real error (we don't consider receive/close errors as bad errors)
        if (Socket::IsCommonReceiveError(event->ErrorStatus.Context) == false)
          EventSend(this, Events::WebSocketError, event);
      }
      else
      {
        // Based on the packet type..
        // Note: We don't currently handle pong because we don't care (nor do we send out pings)
        switch (event->PacketType)
        {
          // If we received either text or binary data, let the user know
          case WebSocketPacketType::Binary:
          case WebSocketPacketType::Text:
            EventSend(this, Events::WebSocketReceivedData, event);
            break;

          // A continuation of a packet is a partial packet...
          case WebSocketPacketType::Continuation:
            Error("WebSocket Continuation packets are not currently handled");
            break;

          // We must respond with a pong
          case WebSocketPacketType::Ping:
            this->SendPacket(event->Data, WebSocketPacketType::Pong);
            break;

          // When the socket is closed, we must send a close response and close our own socket
          case WebSocketPacketType::Close:
            this->SendPacket(event->Data, WebSocketPacketType::Close);
            break;
        }
      }
    }

    // Clear out the owning messages
    this->OwnerIncomingEvents.Clear();

    // Now, if we ever experienced a disconnect, either via a non-writable socket or a thread terminates
    // then we'll attempt to close the socket
    // The receive thread can terminate on any error or upon a gracefull disconnect (receives 0 data)
    // The send thread can terminate on any error or upon sending the final web-socket close packet
    if (this->ReceiveThread.IsCompleted() || this->SendThread.IsCompleted())
      this->Close();
  }

  //***************************************************************************
  void ThreadedWebSocketConnection::Initialize()
  {
    // The send event will be used to signal the send thread that we have data outgoing
    this->SendEvent.Initialize();
    
    // Initialize all of our threads
    this->ReceiveThread.Initialize(ReceiveEntryPoint, this, "WebSocketReceive");
    this->SendThread.Initialize(SendEntryPoint, this, "WebSocketSend");

    // Start the send and recieve threads
    this->SendThread.Resume();
    this->ReceiveThread.Resume();
  }

  //***************************************************************************
  OsInt ThreadedWebSocketConnection::ReceiveEntryPoint(void* context)
  {
    // The context we pass in is our 'this' pointer
    ThreadedWebSocketConnection* self = (ThreadedWebSocketConnection*)context;

    // Receive messages until the socket is closed or we disconnect
    ZilchLoop
    {
      // Create an event that we'll queue up for the owning thread to receive
      WebSocketEvent receivedEvent;
      receivedEvent.Connection = self;

      // Receive an entire packet of text, this text should be json data
      receivedEvent.PacketType = self->BlockingConnection.ReceiveFullPacket(receivedEvent.ErrorStatus, receivedEvent.Data);
      
      // Lock the recieve buffer and push the message into it
      // The message may be an actual packet, or an error message
      self->IncomingLock.Lock();
      self->ThreadIncomingEvents.PushBack(receivedEvent);
      self->IncomingLock.Unlock();

      // If we failed to for any reason (or disconnected gracefully), exit out of the thread
      if (receivedEvent.PacketType == WebSocketPacketType::Invalid)
        return 0;
    }
  }

  //***************************************************************************
  OsInt ThreadedWebSocketConnection::SendEntryPoint(void* context)
  {
    // The context we pass in is our 'this' pointer
    ThreadedWebSocketConnection* self = (ThreadedWebSocketConnection*)context;

    // We pull messages from the debugger's owning thread into our thread
    Array<WebSocketEvent> messageQueue;

    ZilchLoop
    {
      // Wait for any messages to be put on the queue
      self->SendEvent.Wait();

      // Make sure OUR message queue is empty (this is safe to do outside the lock)
      ErrorIf(messageQueue.Empty() == false,
        "We should have sent all messages before attempting to swap with the main thread");

      // We got some messages, lock and swap with our own message buffer
      self->SendLock.Lock();

      // If we have no messages but the send event was signaled
      if (self->SendMessages.Empty())
      {
        // Make sure to unlock and early return out
        self->SendLock.Unlock();
        return 0;
      }

      // Very quickly swap our entire send messages array with theirs
      // Note: This should always clear our their array because ours should be empty
      messageQueue.Swap(self->SendMessages);

      // Due to a race condition that can happen where the main thread queues up
      // a message between the Wait and the Lock, then we will have the event set in the next loop,
      // yet we will have cleared all the messages (swapped)
      self->SendEvent.Reset();

      // We're done, that was fast!
      self->SendLock.Unlock();

      // Send every message we have
      for (size_t i = 0; i < messageQueue.Size(); ++i)
      {
        // Grab the current message
        WebSocketEvent& message = messageQueue[i];

        // This is only queued up in the case that an error occurs
        WebSocketEvent errorEvent;
        errorEvent.Connection = self;

        // Send the full packet over
        self->BlockingConnection.SendFullPacket(errorEvent.ErrorStatus, (const byte*)message.Data.Data(), message.Data.SizeInBytes(), message.PacketType);

        // If we encountered an error when sending...
        if (errorEvent.ErrorStatus.Failed())
        {
          // Lock the recieve buffer and push the error into it
          self->IncomingLock.Lock();
          self->ThreadIncomingEvents.PushBack(errorEvent);
          self->IncomingLock.Unlock();

          // We always exit the send thread in the event of an error
          return 0;
        }

        // If the last packet was a close packet, we must not send any more data, as per the web-socket RFC
        if (message.PacketType == WebSocketPacketType::Close)
          return 0;
      }

      // Clear out the queue
      messageQueue.Clear();
    }
  }

  
  //***************************************************************************
  ThreadedWebSocketListener::ThreadedWebSocketListener() :
    AcceptingConnection(nullptr)
  {
  }
  
  //***************************************************************************
  ThreadedWebSocketListener::~ThreadedWebSocketListener()
  {
    this->Close();
  }
  
  //***************************************************************************
  void ThreadedWebSocketListener::Initialize(int port)
  {
    // Starts listening on the given port
    WebSocketEvent hostEvent;
    this->BlockingListener.Initialize(hostEvent.ErrorStatus, port);

    // If the hosting status failed for any reason, dispatch an event out
    if (hostEvent.ErrorStatus.Failed())
      EventSend(this, Events::WebSocketError, &hostEvent);

    // Create the web socket accepting thread
    this->AcceptThread.Initialize(AcceptEntryPoint, this, "WebSocketAccept");

    // Start the accepting thread
    this->AcceptThread.Resume();
  }

  //***************************************************************************
  bool ThreadedWebSocketListener::IsValid()
  {
    return this->BlockingListener.IsValid();
  }

  //***************************************************************************
  void ThreadedWebSocketListener::Close()
  {
    // If the connection is already closed... early out
    if (this->BlockingListener.ListenerSocket.IsOpen() == false)
      return;

    // We only want to terminate the accepting listener socket while inside the lock, to prevent a race condition
    // We also want to terminate the socket for any connection that is currently being accepted (if it exists)
    this->AcceptingConnectionLock.Lock();
    
    // Close the remote connection, which should signal that we're ending the connection
    WebSocketEvent listenerCloseEvent;
    this->BlockingListener.ListenerSocket.Close(listenerCloseEvent.ErrorStatus);
    
    // Blocking connection close event
    WebSocketEvent acceptingCloseEvent;

    // If the accepting connection is valid (we have one that is currently being accepted
    if (this->AcceptingConnection != nullptr && this->AcceptingConnection->BlockingConnection.IsValid())
      this->AcceptingConnection->BlockingConnection.RemoteSocket.Close(acceptingCloseEvent.ErrorStatus);

    // Let the accepting thread resume
    this->AcceptingConnectionLock.Unlock();

    // The accept thread should encounter an error (accept will unblock)
    // and then because the listener descriptor was closed and made invalid by calling 'Close', the thread should terminate
    // Wait until the accept thread ends...
    this->AcceptThread.WaitForCompletion();

    // If the listener close status failed for any reason, dispatch an event out
    if (listenerCloseEvent.ErrorStatus.Failed())
      EventSend(this, Events::WebSocketError, &listenerCloseEvent);

    // If the accepting connection's close status failed for any reason, dispatch an event out
    if (acceptingCloseEvent.ErrorStatus.Failed())
      EventSend(this, Events::WebSocketError, &acceptingCloseEvent);
  }
  
  //***************************************************************************
  void ThreadedWebSocketListener::Update()
  {
    // Lock the recieve buffer and bring all messages to the owning recieve message array
    this->IncomingLock.Lock();
    this->OwnerIncomingEvents.Swap(this->ThreadIncomingEvents);
    this->IncomingLock.Unlock();
    
    // Walk through all messages and deliver them to the user
    for (size_t i = 0; i < this->OwnerIncomingEvents.Size(); ++i)
    {
      // Grab the current message and send out a received event
      WebSocketEvent* event = &this->OwnerIncomingEvents[i];

      // Depending on what happened, send out either an error message or a accepted connection message
      if (event->Connection != nullptr)
        EventSend(this, Events::WebSocketAcceptedConnection, event);
      else
        EventSend(this, Events::WebSocketError, event);
    }

    // Clear out the owning messages
    this->OwnerIncomingEvents.Clear();

    // Now, if we ever experienced a disconnect, either via a non-writable socket or a thread terminates
    // Then we'll attempt to close the socket
    if (this->AcceptThread.IsCompleted())
      this->Close();
  }

  //***************************************************************************
  OsInt ThreadedWebSocketListener::AcceptEntryPoint(void* context)
  {
    // The context we pass in is our 'this' pointer
    ThreadedWebSocketListener* self = (ThreadedWebSocketListener*)context;

    ZilchLoop
    {
      // An event we send out if an error occurs
      WebSocketEvent acceptEvent;

      // Before accepting any connections, we need to check if we're terminating, and also let the owning
      // thread know the current connection we're accepting (so if the owning thread destructs this object, it
      // can also close the accepting connection)
      self->AcceptingConnectionLock.Lock();

      // If we no longer have a valid blocking listener, just exit out
      if (self->BlockingListener.IsValid() == false)
      {
        // Make sure to unlock, and terminate this thread
        self->AcceptingConnectionLock.Unlock();
        return 0;
      }
      
      // Create a new connection that can be accepted, and let the owning thread know what it is
      // Again, this is so the owning thread can cancel both the listening and accepting sockets so we don't deadlock on receieve
      ThreadedWebSocketConnection* connection = new ThreadedWebSocketConnection();
      self->AcceptingConnection = connection;

      // Let the owning thread resume
      self->AcceptingConnectionLock.Unlock();

      // Accept an incoming connection (blocks until the connection is fully acked according to web-sockets)
      // Will unblock if the sockets are terminated by the owning thread
      self->BlockingListener.Accept(acceptEvent.ErrorStatus, connection->BlockingConnection);
      
      // Now that we've finished accepting (could have failed, or could be a valid connection)
      // we are no longer accepting this 'connection'
      self->AcceptingConnectionLock.Lock();

      // Clear out the accepting connection so the owning thread won't try and close it
      self->AcceptingConnection = nullptr;

      // Let the owning thread resume
      self->AcceptingConnectionLock.Unlock();

      // If we didn't fail to accept a connection, then tell the main thread about the connection
      if (acceptEvent.ErrorStatus.Succeeded())
      {
        // This is an accepted connection with no errors!
        acceptEvent.Connection = connection;

        // Initialize the connection, which generally spins up the send/receive threads
        connection->Initialize();
      
        // Lock the recieve buffer and push the connection into it
        self->IncomingLock.Lock();
        self->ThreadIncomingEvents.PushBack(acceptEvent);
        self->IncomingLock.Unlock();
      }
      else
      {
        // Any failure to connect should destroy the connection
        delete connection;

        // If the extended error code was set, it means we ran into a true socket error (or the socket was closed) so terminate the connection
        if (acceptEvent.ErrorStatus.Context != 0 && Socket::IsCommonAcceptError(acceptEvent.ErrorStatus.Context) == false)
        {
          // We only dispatch the error message if it's not a close event
          if (Socket::IsCommonReceiveError(acceptEvent.ErrorStatus.Context) == false)
          {
            // Lock the recieve buffer and push the error into it
            self->IncomingLock.Lock();
            self->ThreadIncomingEvents.PushBack(acceptEvent);
            self->IncomingLock.Unlock();
          }

          // We always exit the send thread in the event of an error
          return 0;
        }
      }
    }
  }

  //***************************************************************************
  ThreadedWebSocketServer::ThreadedWebSocketServer(size_t maxConnections) :
    MaximumConnections(maxConnections)
  {
    // We want to know when connections are accepted, and when errors occur with the listener
    EventConnect(&this->Listener, Events::WebSocketAcceptedConnection, &ThreadedWebSocketServer::OnAcceptedConnection, this);
    EventForward(&this->Listener, Events::WebSocketError, this);
  }

  //***************************************************************************
  ThreadedWebSocketServer::~ThreadedWebSocketServer()
  {
    // Loop through all the connections we have and update them
    for (size_t i = 0; i < this->Connections.Size(); ++i)
    {
      // Grab the current connection
      ThreadedWebSocketConnection* connection = this->Connections[i];
      delete connection;
    }
  }

  //***************************************************************************
  void ThreadedWebSocketServer::Host(int port)
  {
    this->Listener.Initialize(port);
  }
  
  //***************************************************************************
  bool ThreadedWebSocketServer::IsValid()
  {
    return this->Listener.IsValid();
  }

  //***************************************************************************
  void ThreadedWebSocketServer::Update()
  {
    // Update the listener, which will dispatch events
    this->Listener.Update();

    // Loop through all the connections we have and update them
    for (size_t i = 0; i < this->Connections.Size();)
    {
      // Grab the current connection
      ThreadedWebSocketConnection* connection = this->Connections[i];

      // Update the connection, which could actually close the socket
      connection->Update();

      // If the connection is closed, we should remove it!
      if (connection->IsValid() == false)
      {
        // Swap with the last connection and then pop the back
        this->Connections[i] = this->Connections.Back();
        this->Connections.PopBack();
      }
      else
      {
        // Otherwise, the connection was fine so just walk to the next one
        ++i;
      }
    }
  }

  //***************************************************************************
  void ThreadedWebSocketServer::SendPacketToAll(StringParam message, WebSocketPacketType::Enum packetType)
  {
    // Loop through all the connections and send the message to each
    for (size_t i = 0; i < this->Connections.Size(); ++i)
    {
      // Grab the current connection and send the message
      ThreadedWebSocketConnection* connection = this->Connections[i];
      connection->SendPacket(message, packetType);
    }
  }
  
  //***************************************************************************
  void ThreadedWebSocketServer::OnAcceptedConnection(WebSocketEvent* event)
  {
    // If we're already at (or exceeded) our max connections...
    if (this->Connections.Size() >= this->MaximumConnections)
    {
      // Terminate the accepted connection immediately (don't add it to our list)
      delete event->Connection;
      event->Connection = nullptr;
      return;
    }

    // Add the connection to our own tracked list
    this->Connections.PushBack(event->Connection);

    // Forward the event on us, so anyone listening can see we got a new connection
    EventSend(this, event->EventName, event);

    // Forward all the errors, disconnect, and receive data events
    EventForward(event->Connection, Events::WebSocketError, this);
    EventForward(event->Connection, Events::WebSocketDisconnected, this);
    EventForward(event->Connection, Events::WebSocketReceivedData, this);
  }
}
