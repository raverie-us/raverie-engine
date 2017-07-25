/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_WEB_SOCKET_HPP
#define ZILCH_WEB_SOCKET_HPP

namespace Zilch
{
  ZilchDeclareStaticLibrary(WebSockets, ZilchNoNamespace, ZeroShared);

  namespace Events
  {
    // Sent when the listener accepts a connection
    // This event MUST be handled, otherwise the accepted web-sockets will leak
    ZilchDeclareEvent(WebSocketAcceptedConnection, WebSocketEvent);

    // Sent any time the threaded web-socket receives data
    ZilchDeclareEvent(WebSocketReceivedData, WebSocketEvent);

    // Sent any time the threaded web-socket encounters an error
    // This should generally always be followed by a disconnect event
    ZilchDeclareEvent(WebSocketError, WebSocketEvent);

    // Sent any time the threaded web-socket is closed
    // If an error occurs, the web-socket is automatically closed and this event will be sent
    ZilchDeclareEvent(WebSocketDisconnected, WebSocketEvent);
  }

  // Describes the raw types of packets we can receive
  // The only packets that the user will recieve is Text and Binary
  // (all other packets are handled internally)
  namespace WebSocketPacketType
  {
    enum Enum
    {
      Invalid       = -1,
      Continuation  = 0x00,
      Text          = 0x01,
      Binary        = 0x02,
      // Reserved Non-Control Frames 0x03-0x07
      Close         = 0x08,
      Ping          = 0x09,
      Pong          = 0x0A
      // Reserved Control Frames 0x0B-0x0F
    };
  }

  // Forward declarations
  class ThreadedWebSocketConnection;

  // An event sent out whenever a web socket connection changes or receives data
  class ZeroShared WebSocketEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Default constructor
    WebSocketEvent();

    // The connection involved in the event
    ThreadedWebSocketConnection* Connection;

    // If we received data, this will contain the data we received (otherwise will be empty if not applicable)
    WebSocketPacketType::Enum PacketType;

    // Any data that was received by the connection (or empty if not applicable)
    // The data can be binary or text, depending on PacketType
    String Data;

    // If any error occurred, this status will hold the error message and state
    Status ErrorStatus;
  };

  // A connection that we can communicate on (could be from cient to server, or server to client)
  // With the blocking version, the user must properly respond to the Close and Ping messages
  // The threaded version internally takes care of these messages
  class ZeroShared BlockingWebSocketConnection
  {
  public:

    // Constructor
    BlockingWebSocketConnection();
    
    // All the values we read from the HTTP headers
    HashMap<String, String> Headers;

    // Send a full packet to the remote end
    // This function will block until the entire packet is sent
    // It is safe to call this function from another thread (only one thread at a time though)
    void SendFullPacket(Status& status, const byte* data, size_t length, WebSocketPacketType::Enum packetType);

    // Receives an entire packet of data into an array
    // This function will block until the entire packet is received, or an error occurs
    // It is safe to call this function from another thread (only one thread at a time though)
    // Note: We use strings both as text and binary blobs of data
    // If we return an 'Invalid' packet, it means the connection was disconnected or an error occurred (check status)
    // The packet types we receive can be Text, Binary, Close, or Ping
    // Close must be responeded to by sending a Close message back, and Ping must be responded to by sending back a Pong
    WebSocketPacketType::Enum ReceiveFullPacket(Status& status, String& dataOut);
    
    // Checks if the connection is initialized
    bool IsValid();

    // The connection to the remote host (we can send and receive on this connection)
    Socket RemoteSocket;

    //******** Internal ********//
    
    // As we read data, we place it into this buffer
    Array<byte> ReadData;
  };

  // Listens for incoming web-socket connections
  // This class should only be initialized and closed once (it should not be reused)
  class ZeroShared BlockingWebSocketListener
  {
  public:

    // Constructor
    BlockingWebSocketListener();

    // Host a server on a given port and initialize the internal socket
    void Initialize(Status& status, int port);

    // Closes the socket
    void Close(Status& status);

    // Checks if the object is initialized
    bool IsValid();

    // Blocks until we receive an incoming connection
    // This method performs the full web-socket authentication and will not complete
    // until either the connection fails, or the socket is acceptped
    // Ideally the server should be run on another thread due to blocking
    void Accept(Status& status, BlockingWebSocketConnection& connectionOut);

    // The socket that we listen for incoming connections on
    Socket ListenerSocket;
  };

  // A threaded version of the blocking web-socket connection
  // This class can only be initialized from a ThreadedWebSocketListener
  // This class maintains a send and receive thread, and when updated we pull data
  // Either the user or the ThreadedWebSocketServer must periodically call Update from the owning thread
  // All send and update functions are safe to call from the owning thread (not multiple!)
  // Once this connection has been terminated, it may not be used again
  class ZeroShared ThreadedWebSocketConnection : public EventHandler
  {
  public:
    // sends WebSocketReceivedData : WebSocketEvent;
    // sends WebSocketError : WebSocketEvent;
    // sends WebSocketDisconnected : WebSocketEvent;

    // Default constructor
    ThreadedWebSocketConnection();

    // Destructor (terminates threads and closes the socket)
    ~ThreadedWebSocketConnection();

    // Send a single message
    // You should only ever send the Text or Binary packet types
    void SendPacket(StringParam message, WebSocketPacketType::Enum packetType);
    
    // Checks if the connection is initialized
    bool IsValid();

    // Shuts down a connection and closes the socket
    // Only the first call will actually terminate the socket (multiple calls allowed)
    void Close();

    // Pumps both received messages and events such as a disconnect
    // If a thread terminates early, or the socket becomes no longer writable, Close will be called and an event will be sent
    void Update();
    
    //******** Internal ********//

    // Thread entrypoints for receive, and send
    static OsInt ReceiveEntryPoint(void* context);
    static OsInt SendEntryPoint(void* context);

    // Sets the web-socket and spins up the send/receive threads
    // We can only be initialized by a listener (after our blocking WebSocket has been initialized)
    void Initialize();
    
    // The thread we receive data on (receive is a blocking call)
    Thread ReceiveThread;

    // We must lock the array of receive messages/errors it before reading or modifying it
    ThreadLock IncomingLock;

    // This array is locked by the web-socket receiving thread (and send thread when errors occur)
    // Any read in messages are enqued here and dispatched when the connection is updated
    // Note: We use strings both as text and binary blobs
    Array<WebSocketEvent> ThreadIncomingEvents;

    // This array of received messages is maintained by the owning thread, and is regularly swapped with the 'ThreadReceiveMessages'
    Array<WebSocketEvent> OwnerIncomingEvents;

    // The send outgoing messages on (send is a blocking call)
    Thread SendThread;

    // We must lock the array of send messages it before reading or modifying it
    ThreadLock SendLock;

    // This array is locked by the web-socket sending thread
    // Any messages we want sent should just get added here, and the
    // 'send' event should be signaled once a message is added (or multiple)
    // If the send event is signaled and there are no messages, it means we are being destroyed
    // Note: We use strings both as text and binary blobs
    Array<WebSocketEvent> SendMessages;

    // Every time we add a message to the queue of messages to be sent we
    // signal this event, this will wake up the send thread
    // The send thread will then swap array pointers with the 'SendMessages',
    // which will very quickly pull all the messages off and clear messages to be sent
    // If the send event is signaled and there are no messages, it means we are being destroyed
    OsEvent SendEvent;

    // The web socket we communicate on (where we send our messages, and receive from)
    // All operations are done on other threads (that we properly lock and make safe to pull into the owning thread)
    BlockingWebSocketConnection BlockingConnection;
  };

  
  // Listens for incoming web-socket connections on a thread
  // Can only be used once (once it is closed, it should be removed)
  // Must be preriodically updated by the owning thread, which will then dispatch events for accepted connections
  class ZeroShared ThreadedWebSocketListener : public EventHandler
  {
  public:
    // sends WebSocketAcceptedConnection : WebSocketEvent;
    // sends WebSocketError : WebSocketEvent;

    // Default constructor
    ThreadedWebSocketListener();

    // Destructor (terminates threads and closes the socket)
    ~ThreadedWebSocketListener();

    // Start listening and accepting connections on a given port
    // This also initializes the listener and should only be called once
    void Initialize(int port);

    // Shuts down a connection and closes the socket
    // Only the first call will actually terminate the socket (multiple calls allowed)
    void Close();

    // Checks if the object is initialized
    bool IsValid();
    
    // Updates the web socket connection, which pumps both received messages and events such as a disconnect
    void Update();
    
    //******** Internal ********//

    // The thread entrypoint we use for accepting socket connections
    static OsInt AcceptEntryPoint(void* context);

    // The current connection that we are processing
    // It is NOT safe to access this connection from the owning thread, unless using the 'AcceptingConnectionLock'
    // Note: The only action we should use the AcceptingConnectionLock for is terminating
    // the socket for the accepting connection upon our destruction
    ThreadedWebSocketConnection* AcceptingConnection;

    // Whenever the accepting thread creates a connection, it needs to lock to ensure that the owning thread
    // is not also accessing the accepting connection
    // If the accepting thread locks and finds the blocking listening socket to be terminated, we will immediately return
    ThreadLock AcceptingConnectionLock;

    // Anytime a connection is accepted or an error occurs, this must be locked (to write to ThreadIncomingEvents)
    ThreadLock IncomingLock;

    // We maintain a list of all connections that we accept and errors that occur
    Array<WebSocketEvent> ThreadIncomingEvents;

    // This array of received messages is maintained by the owning thread, and is regularly swapped with the 'ThreadReceiveMessages'
    Array<WebSocketEvent> OwnerIncomingEvents;

    // The thread we accept connections on
    Thread AcceptThread;

    // We tell the blocking listener to accept connections on the accepting thread
    BlockingWebSocketListener BlockingListener;
  };

  // The web-socket server maintains threaded web-socket connections and invokes
  // callbacks for when connections are received and fully handshook, or closed
  // The thread that owns the server is responsible for occasionally pumping events via Update
  class ZeroShared ThreadedWebSocketServer : public EventHandler
  {
  public:
    // Events will be forwarded from owned connections to the server
    // sends WebSocketAcceptedConnection : WebSocketEvent;
    // sends WebSocketError : WebSocketEvent;
    // sends WebSocketReceivedData : WebSocketEvent;
    // sends WebSocketDisconnected : WebSocketEvent;

    // Default constructor that sets the max connections
    ThreadedWebSocketServer(size_t maxConnections = 64);

    // Destructor (terminates all connections)
    ~ThreadedWebSocketServer();

    // Start listening and accepting connections on a given port
    void Host(int port);

    // Tells us if the server has been initialized (specifically, the listener)
    bool IsValid();

    // Updates the web socket connection, which pumps accepted
    // connections and all sends/receives on stored connections
    void Update();
    
    // Send a message to all connections
    void SendPacketToAll(StringParam message, WebSocketPacketType::Enum packetType);

    // The maximum number of connections we'll accept
    size_t MaximumConnections;

    // The connections we maintain and accept
    Array<ThreadedWebSocketConnection*> Connections;

    
    //******** Internal ********//

    // Occurs when our listener accepts a connection (only when updating the listener)
    void OnAcceptedConnection(WebSocketEvent* event);

    // The listener we use to accept connections
    ThreadedWebSocketListener Listener;
  };
}

#endif
