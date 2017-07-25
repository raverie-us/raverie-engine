///////////////////////////////////////////////////////////////////////////////
///
/// \file TcpSocket.cpp
/// Implementation of the TcpSocket class.
///
/// Authors: Trevor Sundberg.
/// Copyright 2010-2011, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include <WinSock2.h>

// Defines
#define infinite_loop for(;;)

// Include the winsock library
#pragma comment(lib, "ws2_32.lib")

// Make sure our socket definition is the proper size
StaticAssert(SocketSize, sizeof(Zero::SocketHandle) >= sizeof(SOCKET), "The socket sizes do not match (they must so we don't have to include winsock in our header!)");

// The general max size of temporary buffers
const size_t BufferSize = 8192;

// Type-defines
typedef size_t ChunkLengthType;

namespace Zero
{

// Random number generator for guids
Math::Random TcpSocket::GuidGenerator((int)time(nullptr) + (int)clock() * 137);

namespace Events
{
  DefineEvent(SocketError);
  DefineEvent(ConnectionCompleted);
  DefineEvent(ConnectionFailed);
  DefineEvent(Disconnected);
  DefineEvent(ReceivedData);
}

ZilchDefineType(ConnectionData, builder, type)
{
  ZilchBindFieldProperty(Host);
  ZilchBindFieldProperty(Address);
  ZilchBindFieldProperty(Port);
  ZilchBindFieldProperty(Index);
  ZilchBindFieldProperty(Incoming);
}

// Constructor
ConnectionData::ConnectionData()
{
  Index = InvalidIndex;
  Incoming = false;
}

ZilchDefineType(ConnectionEvent, builder, type)
{
  ZilchBindFieldProperty(Host);
  ZilchBindFieldProperty(Address);
  ZilchBindFieldProperty(Port);
  ZilchBindFieldProperty(Index);
  ZilchBindFieldProperty(Incoming);
}

// Constructor
ConnectionEvent::ConnectionEvent(const ConnectionData* connectionInfo)
{
  // Set the connection data
  Host      = connectionInfo->Host;
  Address   = connectionInfo->Address;
  Port      = connectionInfo->Port;
  Index     = connectionInfo->Index;
  Incoming  = connectionInfo->Incoming;
}

ZilchDefineType(ReceivedDataEvent, builder, type)
{
  ZilchBindFieldProperty(Buffer);
}

ReceivedDataEvent::ReceivedDataEvent(const ConnectionData* connectionInfo, const byte* data, size_t size)
  : ConnectionEvent(connectionInfo)
{
  // Resize the data buffer to be the size of the data we're getting
  Data.Resize(size);

  // Copy over the data
  memcpy(Data.Data(), data, size);

  Buffer = StringRange((const char*)data, (const char*)data, (const char*)data + size);
}

// Constructor
ProtocolSetup::ProtocolSetup()
{
  Protocols = 0;
  ChunkType = ChunkType::LengthEncoded;
}

// Is a protocol set that requires message headers and length encoded chunks?
bool ProtocolSetup::RequiresCustomData()
{
  return (Protocols & Protocol::FlagsThatNeedCustomData) != 0;
}

// Define the meta-implementations
ZilchDefineType(TcpSocket, builder, type)
{
  // Make constructible in script
  type->CreatableInScript = true;

  ZeroBindDocumented();

  ZilchBindMethod(Connect);
  ZilchBindOverloadedMethod(Listen, ZilchInstanceOverload(bool, int, uint));
  ZilchBindOverloadedMethod(Listen, ZilchInstanceOverload(bool, int, uint, TcpSocketBind::Enum));
  ZilchBindMethod(Close);
  ZilchBindMethod(CloseConnection);
  ZilchBindMethod(SendTo);
  ZilchBindMethod(SendToAll);
  ZilchBindMethod(SendToAllExcept);
  ZilchBindMethod(SendToAllAndSelf);
  ZilchBindMethod(IsConnected);

  ZilchBindGetterProperty(OutgoingConnectionCount);
  ZilchBindGetterProperty(IncomingConnectionCount);
  ZilchBindGetterProperty(ConnectionCount);

  ZeroBindEvent(Events::SocketError, TextErrorEvent);
  ZeroBindEvent(Events::ConnectionCompleted, ConnectionEvent);
  ZeroBindEvent(Events::ConnectionFailed, ConnectionEvent);
  ZeroBindEvent(Events::Disconnected, ConnectionEvent);
  ZeroBindEvent(Events::ReceivedData, ReceivedDataEvent);
}

TcpSocket::TcpSocket()
{
  mProtocolSetup.Protocols = Protocol::Chunks | Protocol::Events;
  mDataVersion = DataVersion::Current;
  Initialize();
}

// Constructor (no protocols)
TcpSocket::TcpSocket(StringParam debugName)
{
  mDebugName = debugName;
  mDataVersion = DataVersion::Current;
  Initialize();
}

// Constructor (basic protocol)
TcpSocket::TcpSocket(Protocol::Type protocols, StringParam debugName)
{
  mDebugName = debugName;
  mDataVersion = DataVersion::Current;
  mProtocolSetup.Protocols = protocols;
  Initialize();
}

// Constructor (protocol setup info)
TcpSocket::TcpSocket(const ProtocolSetup& setup, StringParam debugName)
{
  mDebugName = debugName;
  mDataVersion = DataVersion::Current;
  mProtocolSetup = setup;
  Initialize();
}

// Initialize the socket (called by the constructors)
void TcpSocket::Initialize()
{
  // Do we have any protocols enabled?
  if (mProtocolSetup.Protocols != 0)
  {
    // If we enabled any protocol that would require an extra message header (any time we add custom data)
    if (mProtocolSetup.RequiresCustomData())
    {
      // We always enable the protocol of using length encoded chunks (don't want to get half an event!)
      // Note: This may override what the user specified, but it's really important otherwise we might get garbled data
      mProtocolSetup.Protocols |= Protocol::Chunks;
      mProtocolSetup.ChunkType = ChunkType::LengthEncoded;
    }

    // If we enabled the Guid protocol...
    if (mProtocolSetup.Protocols & Protocol::Guid)
    {
      // We should be generating a 64-bit random number to avoid collisions
      mGuid = (NetGuid)GuidGenerator.Uint64();
    }
    else
    {
      mGuid = 0;
    }
  }

  // Set the maximum number of connections
  mMaxIncomingConnections = 0;

  // Store winsock startup data
  WSADATA wsaData;

  // If the startup resulted in an error
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
  {
    // Dispatch an error
    DispatchError();
  }
  else
  {
    // Connect to the frame update event
    ConnectThisTo(Z::gEngine, Events::EngineUpdate, Update);
  }

  // Set the socket to an invalid socket
  mServer = INVALID_SOCKET;

  // Initialize all the statistics to zero
  mSendCount = 0;
  mSendSize = 0;
  mReceiveCount = 0;
  mReceieveSize = 0;
}

// Destructor
TcpSocket::~TcpSocket()
{
  // Close out of the socket
  Close();

  // Cleanup winsock references
  WSACleanup();
}

// Attempt to connect to a server on the given port
void TcpSocket::Connect(StringParam host, int port)
{
  // Fill out a connection data structure
  ConnectionData info;
  info.Address = ResolveHost(host);
  info.Port = port;
  info.Host = host;

  // Do the connection
  InternalConnect(info);
}

// Resolve an IP address into a name
String ResolveIP(in_addr address)
{
  auto host = gethostbyaddr((char*) &address.S_un.S_addr, sizeof(address.S_un.S_addr), AF_INET);

  if (host != nullptr)
  {
    return host->h_name;
  }
  else
  {
    return inet_ntoa(address);
  }
}

// Internal connection function
void TcpSocket::InternalConnect(const ConnectionData& info)
{
  // Setup a host address structure
  sockaddr_in hostAddress;
  hostAddress.sin_family = AF_INET;
  hostAddress.sin_addr.s_addr = info.Address;
  hostAddress.sin_port = htons(info.Port);

  // Setup a new socket
  SOCKET newSocket = (SOCKET)CreateSocket();
  
  ConnectionEvent e(&info);
  // If we got a valid socket
  if (newSocket == INVALID_SOCKET)
  {
    // Dispatch an error
    DispatchError();

    // Dispatch a connection failed event
    mDispatcher.Dispatch(Events::ConnectionFailed, &e);
  }
  else
  {
    // Start the connection process
    if (IsError(connect(newSocket, (SOCKADDR*) &hostAddress, sizeof(hostAddress))))
    {
      // Dispatch an error
      DispatchError();

      // Dispatch a connection failed event
      mDispatcher.Dispatch(Events::ConnectionFailed, &e);

      // Close the socket
      closesocket(newSocket);
    }
    else
    {
      // Create a socket data structure to be added to the pending connections list
      SocketData newSocketData;
      newSocketData.ConnectionInfo = info;
      newSocketData.Handle = newSocket;

      // Add the socket to the pending connections list
      mPendingOutgoingConnections.PushBack(newSocketData);
    }
  }
}

bool TcpSocket::Listen(int port, uint maxConnections)
{
  return Listen(port, maxConnections, TcpSocketBind::Any);
}

// Listen for incoming connections
bool TcpSocket::Listen(int port, uint maxConnections, TcpSocketBind::Enum bindTo)
{
  // Quit out if we have an invalid socket (or it can't be created)
  if (ValidateServer() == false)
    return false;

  // Create a socket address info that lets us bind to the local network adapter (probably the primary)
  sockaddr_in service;
  service.sin_family      = AF_INET;
  service.sin_port        = htons(port);
  if(bindTo == TcpSocketBind::Loopback)
    service.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  else
    service.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket to the local port
  if (IsError(bind((SOCKET)mServer, (SOCKADDR*) &service, sizeof(service))))
  {
    // Dispatch out an error and return a failure
    DispatchError();
    return false;
  }

  // Use the defined constant for maximum connections
  if (maxConnections == MaxPossibleConnections)
    maxConnections = SOMAXCONN;

  // Set the maximum number of connections
  if (IsError(listen((SOCKET)mServer, maxConnections)))
  {
    // Dispatch out an error and return a failure
    DispatchError();
    return false;
  }

  // Store the maximum number of connections
  mMaxIncomingConnections = maxConnections;

  // Return a success!
  return true;
}

// Close all activity (whether listening or connected to a server)
void TcpSocket::Close()
{
  // No longer need server socket
  closesocket((SOCKET)mServer);

  // Set the socket to an invalid socket
  mServer = INVALID_SOCKET;

  // Loop through all the connections and close them
  for (size_t i = 0; i < mConnections.Size(); ++i)
    CloseConnection(i);
}

void TcpSocket::RawSend(SocketData& socketData, const byte* data, size_t size)
{
  // Ignore this send if it's to an invalid socket
  if(socketData.Handle == 0)
    return;

  // Assume we sent nothing...
  size_t dataSent = 0;

  // If we have no queued data, try to send this immediately!
  if(socketData.PartialSentData.Empty())
  {
    // Send the data to the current socket
    int result = send((SOCKET)socketData.Handle, (const char*)data, size, 0);
    VerifyResult(result);

    // If we didn't send all the data, figure out how much we did send
    if(result != SOCKET_ERROR)
      dataSent = (size_t)result;
  }
  
  // Compute how much data we did not sent (needs to be queued)
  size_t dataNotSent = size - dataSent;

  // Resize the buffer to hold the data we didn't send yet...
  size_t lastSize = socketData.PartialSentData.Size();
  socketData.PartialSentData.Resize(lastSize + dataNotSent);
  memcpy(socketData.PartialSentData.Data() + lastSize, data + dataSent, dataNotSent);

  // Just check if the sent data is building up too much
  if (socketData.PartialSentData.Size() > 65535)
    DoNotifyWarning("TcpSocket", "Too much data is being sent on the TcpSocket");
}

// Send a message to a specific connection index
void TcpSocket::SendBufferTo(const byte* buffer, size_t size, size_t index)
{
  // Make sure that the index we were given was valid
  ReturnIf(index >= mConnections.Size(),, "The given connection index was invalid");

  // Make the user packet (which may do nothing if there no header needed)
  PodArray<byte> data;
  MakeUserPacket(buffer, size, data);

  // Send the data to each connection
  RawSend(mConnections[index], data.Data(), data.Size());

  // Statistics
  TrackSend(data.Data(), data.Size(), mConnections[index].Handle);
}

// Send a message to all connections
void TcpSocket::SendBufferToAll(const byte* buffer, size_t size)
{
  // Make the user packet (which may do nothing if there no header needed)
  PodArray<byte> data;
  MakeUserPacket(buffer, size, data);

  // Loop through all the connections
  for (size_t i = 0; i < mConnections.Size(); ++i)
  {
    // Send the data to each connection
    RawSend(mConnections[i], data.Data(), data.Size());

    // Statistics
    TrackSend(data.Data(), data.Size(), mConnections[i].Handle);
  }
}

// Send a message to all connections except a particular connection index
void TcpSocket::SendBufferToAllExcept(const byte* buffer, size_t size, size_t exceptIndex)
{
  // Make sure that the exceptIndex we were given was valid
  ReturnIf(exceptIndex >= mConnections.Size(),, "The given connection index was invalid");

  // Make the user packet (which may do nothing if there no header needed)
  PodArray<byte> data;
  MakeUserPacket(buffer, size, data);

  // Loop through all the connections
  for (size_t i = 0; i < mConnections.Size(); ++i)
  {
    // Send it to everyone except the given connection index
    if (i != exceptIndex)
    {
      // Send the data to each connection
      RawSend(mConnections[i], data.Data(), data.Size());

      // Statistics
      TrackSend(data.Data(), data.Size(), mConnections[i].Handle);
    }
  }
}

// Send an event to a specific connection index
void TcpSocket::SendTo(StringParam eventId, SendableEvent* event, uint index)
{
  // Set the event Id
  event->EventId = eventId;

  // Make sure that the index we were given was valid
  ReturnIf(index >= mConnections.Size(),, "The given connection index was invalid");

  // Store a buffer for the message
  PodArray<byte> data;
  MakeEventPacket(event, data);

  // Send the data to each connection
  RawSend(mConnections[index], data.Data(), data.Size());

  // Statistics
  TrackSend(data.Data(), data.Size(), mConnections[index].Handle);
}

// Send an event to all connections
void TcpSocket::SendToAll(StringParam eventId, SendableEvent* event)
{
  // Set the event Id
  event->EventId = eventId;

  // Store a buffer for the message
  PodArray<byte> data;
  MakeEventPacket(event, data);

  // Loop through all the connections
  for (size_t i = 0; i < mConnections.Size(); ++i)
  {
    // Send the data to each connection
    RawSend(mConnections[i], data.Data(), data.Size());

    // Statistics
    TrackSend(data.Data(), data.Size(), mConnections[i].Handle);
  }
}

void TcpSocket::SendToAllAndSelf(StringParam eventId, SendableEvent* event)
{
  SendToAll(eventId, event);
  this->DispatchEvent(eventId, event);
}

// Send an event to all connections except a particular connection index
void TcpSocket::SendToAllExcept(StringParam eventId, SendableEvent* event, uint exceptIndex)
{
  // Set the event Id
  event->EventId = eventId;

  // Make sure that the exceptIndex we were given was valid
  ReturnIf(exceptIndex >= mConnections.Size(),, "The given connection index was invalid");

  // Store a buffer for the message
  PodArray<byte> data;
  MakeEventPacket(event, data);

  // Loop through all the connections
  for (size_t i = 0; i < mConnections.Size(); ++i)
  {
    // Send it to everyone except the given connection index
    if (i != exceptIndex)
    {
      // Send the data to each connection
      RawSend(mConnections[i], data.Data(), data.Size());

      // Statistics
      TrackSend(data.Data(), data.Size(), mConnections[i].Handle);
    }
  }
}

// Make a user data packet and output the buffer
void TcpSocket::MakeUserPacket(const byte* data, size_t size, PodArray<byte>& dataOut)
{
  // Setup the packet
  SetupPacket(TCPSocketMessageType::UserData, dataOut);

  // Get the size of the data before we do anything to it
  size_t lastSize = dataOut.Size();

  // Copy the user data into the out buffer
  dataOut.Resize(lastSize + size);
  memcpy(dataOut.Data() + lastSize, data, size);

  // Finalize the packet
  FinalizePacket(dataOut);
}

// Make a guid packet
void TcpSocket::MakeGuidPacket(NetGuid guid, PodArray<byte>& dataOut)
{
  // Setup the packet
  SetupPacket(TCPSocketMessageType::Guid, dataOut);

  // Get the size of the data before we do anything to it
  size_t lastSize = dataOut.Size();

  // Copy the user data into the out buffer
  dataOut.Resize(lastSize + sizeof(NetGuid));
  memcpy(dataOut.Data() + lastSize, &guid, sizeof(NetGuid));

  // Finalize the packet
  FinalizePacket(dataOut);
}

// Make a packet given a buffer
void TcpSocket::MakeEventPacket(SendableEvent* event, PodArray<byte>& dataOut)
{
  // Make sure the protocol is enabled
  ErrorIf((mProtocolSetup.Protocols & Protocol::Events) == 0,
    "In order to send packet-objects, the 'PacketObjects' protocol must be enabled by passing it in to the TcpSocket constructor");

  // Setup the packet
  SetupPacket(TCPSocketMessageType::Event, dataOut);

  // Create a buffer saver to serialize the event
  //BinaryBufferSaver saver;
  TextSaver saver;
  saver.OpenBuffer(mDataVersion);
  SendableEvent::Save(event, saver);

  size_t lastSize = dataOut.Size();
  dataOut.Resize(lastSize + saver.GetBufferSize() + 1);
  saver.ExtractInto(dataOut.Data() + lastSize, saver.GetBufferSize());

  dataOut.Back() = '\0';

  // Extract the event data into the out buffer
  //size_t lastSize = dataOut.Size();
  //dataOut.Resize(lastSize + saver.GetSize());
  //ExtractIntoBuffer(saver, dataOut.Data() + lastSize, saver.GetSize());

  // Finalize the packet
  FinalizePacket(dataOut);
}

// Setup a packet (mainly the header)
void TcpSocket::SetupPacket(TCPSocketMessageType::Enum messageType, PodArray<byte>& dataOut)
{
  // Reserve space for the packet (it could be bigger than this size though!)
  dataOut.Reserve(BufferSize);

  // If we enabled the Chunks protocol and we're using length encoding...
  if (mProtocolSetup.Protocols & Protocol::Chunks && mProtocolSetup.ChunkType == ChunkType::LengthEncoded)
  {
    // Add the size of the length to the header (we don't yet write the size because we don't know it!)
    dataOut.Resize(sizeof(ChunkLengthType));
  }

  // If we enabled any protocol that would require an extra message header (any time we add custom data)
  if (mProtocolSetup.RequiresCustomData())
  {
    // We use a single character header when protocols are enabled, so we know what kind of packets they are (users can still send their own data!)
    dataOut.PushBack((byte)messageType);
  }
}

// Finish setting up the packet
void TcpSocket::FinalizePacket(PodArray<byte>& dataOut)
{
  // If we enabled the Chunks protocol and we're using length encoding...
  if (mProtocolSetup.Protocols & Protocol::Chunks && mProtocolSetup.ChunkType == ChunkType::LengthEncoded)
  {
    // If we have length encoded chunks, then the head will always be the size of the packet
    *((ChunkLengthType*)dataOut.Data()) = dataOut.Size();
  }
}

// Extract a message into a buffer and return the number of bytes written
size_t TcpSocket::ExtractIntoBuffer(const BinaryBufferSaver& messageConst, byte* buffer, size_t bufferSize)
{
  // Get rid of the const (HACK) 
  BinaryBufferSaver& message = const_cast<BinaryBufferSaver&>(messageConst);

  // Get the size of the message
  size_t size = message.GetSize();

  // Extract into the buffer
  message.ExtractInto(buffer, bufferSize);

  // Return the size / amount of data extracted
  return size;
}

// Track a send
void TcpSocket::TrackSend(const byte* data, size_t size, SocketHandle socket)
{
  return;

  // Statistics
  ++mSendCount;
  mSendSize += size;

  // Printing
  //ZPrint("%s Send Count (%d) Size (%d) - Sent data of size %d to socket %d\n", this->mDebugName.c_str(), mSendCount, mSendSize, size, socket);

  PrintData("Sent", data, size);
}

// Track a receive
void TcpSocket::TrackReceive(const byte* data, size_t size, SocketHandle socket)
{
  return;

  // Statistics
  ++mReceiveCount;
  mReceieveSize += size;

  // Printing
  //ZPrint("%s Receive Count (%d) Size (%d) - Received data of size %d from socket %d\n", this->mDebugName.c_str(), mReceiveCount, mReceieveSize, size, socket);

  PrintData("Received", data, size);
}

// Print our data
void TcpSocket::PrintData(const char* mode, const byte* data, size_t size)
{
  Array<char> ascii;
  ascii.Resize(size + 1);
  char* text = ascii.Data();
  ascii.Back() = '\0';

  for (size_t i = 0; i < size; ++i)
  {
    unsigned char c = data[i];
    if (c != '\0' && c != '\n' && c != '\r')
    {
      text[i] = c;
    }
    else
    {
      text[i] = '_';
    }
  }

  ZPrint("%s: - %s\n", mode, text);
}

SocketData& TcpSocket::FindOpenOrCreateSocketData()
{
  // Find an available connection
  for(size_t i = 0; i < mConnections.Size(); ++i)
  {
    // Look for an open connection, and take its place!
    SocketData& socketData = mConnections[i];
    if(socketData.Handle == 0)
      return socketData;
  }

  // We only set the index the first time we create it (and never clear it)
  SocketData& newSocketData = mConnections.PushBack();
  newSocketData.ConnectionInfo.Index = mConnections.Size() - 1;
  return newSocketData;
}

// Handle incoming connections
void TcpSocket::HandleIncomingConnections()
{
  // Get the number of connections we have (this could be more optimized...)
  size_t incomingConnections = GetIncomingConnectionCount();
  
  // If we already hit our max connections, don't run anything
  if(incomingConnections >= mMaxIncomingConnections)
    return;

  // Store socket address information
  sockaddr_in sockAddress;
  memset(&sockAddress, 0, sizeof(sockAddress));
  int sockAddrSize = sizeof(sockaddr_in);

  // Accept any incoming connections
  SocketHandle newSocket = accept((SOCKET)mServer, (SOCKADDR*) &sockAddress, &sockAddrSize);

  // If the socket is valid..
  if (newSocket != INVALID_SOCKET)
  {
    // Now that we created a socket, make sure to set all the options we want
    SetSocketOptions(newSocket);

    // Get the IP as an integer
    unsigned long ip = sockAddress.sin_addr.s_addr;
      
    ResolveIP(sockAddress.sin_addr);

    // Find an open connection slot or create one
    SocketData& newSocketData = this->FindOpenOrCreateSocketData();

    // Create a socket data structure to be added to the pending connections list
    newSocketData.ConnectionInfo.Host     = ResolveIP(sockAddress.sin_addr);
    newSocketData.ConnectionInfo.Address  = ip;
    newSocketData.ConnectionInfo.Port     = htons(sockAddress.sin_port);
    newSocketData.ConnectionInfo.Incoming = true;
    newSocketData.Handle = newSocket;

    ConnectionEvent e(&newSocketData.ConnectionInfo);
    // Dispatch an event to inform the user of a connection
    mDispatcher.Dispatch(Events::ConnectionCompleted, &e);

    // If the guid broadcaster protocol is enabled
    if (mProtocolSetup.Protocols & Protocol::GuidBroadcaster)
    {
      // Loop through all the known guids
      auto guids = mTrackedGuids.All();
      for (; !guids.Empty(); guids.PopFront())
      {
        // Get the current guid
        NetGuid guid = guids.Front();

        // We need to tell the other end what our guid is
        PodArray<byte> data; 
        MakeGuidPacket(guid, data);

        // Send the data to each connection
        RawSend(newSocketData, data.Data(), data.Size());

        // Statistics
        TrackSend(data.Data(), data.Size(), newSocket);
      }
    }
  }
}

// Handle outgoing connections
void TcpSocket::HandleOutgoingConnections()
{
  // Make the 'select' call instant (no timeout)
  timeval time;
  time.tv_sec = 0;
  time.tv_usec = 0;

  // Loop through all pending connections to see if they've connected
  for (size_t i = 0; i < mPendingOutgoingConnections.Size();)
  {
    int writeCount = 0;
    int errorCount = 0;

    // Get the socket data
    SocketData socketData = mPendingOutgoingConnections[i];

    {
      // Create a set for all sockets
      fd_set writableSet;
      writableSet.fd_count = 1;

      // Test the socket for readability using select
      memcpy(writableSet.fd_array, &socketData.Handle, sizeof(Zero::SocketHandle));
      writeCount = select(0, nullptr, &writableSet, nullptr, &time);
    }
    {
      // Create a set for all sockets
      fd_set errorSet;
      errorSet.fd_count = 1;

      // Test the socket for readability using select
      memcpy(errorSet.fd_array, &socketData.Handle, sizeof(Zero::SocketHandle));
      errorCount = select(0, nullptr, nullptr, &errorSet, &time);
    }
    
    // If we had an error
    if (errorCount == 1)
    {
      ConnectionEvent e(&socketData.ConnectionInfo);
      // Dispatch a connection failed event
      mDispatcher.Dispatch(Events::ConnectionFailed, &e);

      // Erase the current pending connection (iterates forward)
      mPendingOutgoingConnections.EraseAt(i);
    }
    // If we have a writable socket
    else if (writeCount == 1)
    {
      // Find an open connection slot or create one
      SocketData& newSocketData = this->FindOpenOrCreateSocketData();
      socketData.ConnectionInfo.Index = newSocketData.ConnectionInfo.Index;
      socketData.ConnectionInfo.Incoming = false;
      newSocketData = socketData;
      
      // If we enabled the Guid protocol...
      if (mProtocolSetup.Protocols & Protocol::Guid)
      {
        // We need to tell the other end what our guid is
        PodArray<byte> data;
        MakeGuidPacket(mGuid, data);
        
        // Send the data to each connection
        RawSend(socketData, data.Data(), data.Size());

        // Statistics
        TrackSend(data.Data(), data.Size(), socketData.Handle);
      }

      ConnectionEvent e(&socketData.ConnectionInfo);
      // Dispatch an event to inform the user of a connection
      mDispatcher.Dispatch(Events::ConnectionCompleted, &e);
      
      // Remove it from the pending list since we processed this connection
      mPendingOutgoingConnections.EraseAt(i);
    }
    else
    {
      ++i;
    }
  }
}


void TcpSocket::HandleOutgoingData()
{
  // Loop through all the connections and send any data that needs to be sent
  for (size_t i = 0; i < mConnections.Size(); ++i)
  {
    // Store the current socket
    SocketData& socketData = mConnections[i];

    // If the socket has nothing on it, just skip it
    if(socketData.Handle == 0)
    {
      // Just make sure the data is cleared, and skip this index
      socketData.PartialSentData.Clear();
      continue;
    }

    // Make sure that the index inside the socket data is correct
    ErrorIf(socketData.ConnectionInfo.Index != i, "The intrusive connection index doesn't align with its spot in the array");

    // If we don't have any data to send, skip this!
    if(socketData.PartialSentData.Empty())
      continue;

    PodArray<byte>& partialData = socketData.PartialSentData;
    size_t amountToSend = partialData.Size();

    // Send the data to the current socket
    int result = send((SOCKET)socketData.Handle, (const char*)partialData.Data(), amountToSend, 0);
    VerifyResult(result);

    // If we sent all the data, clear the partial data
    if (result == amountToSend)
    {
      partialData.Clear();
      continue;
    }

    if (result == SOCKET_ERROR)
      continue;

    // We sent some of the data, remove that range
    PodArray<byte>::range range(partialData.Begin(), partialData.Begin() + result);
    socketData.PartialReceivedData.Erase(range);
  }
}

void TcpSocket::HandleIncomingData()
{
  // Pump IO for all connected sockets
  for (size_t i = 0; i < mConnections.Size();)
  {
    // Store the current socket
    SocketData& socketData = mConnections[i];

    // If the socket has nothing on it, just skip it
    if(socketData.Handle == 0)
    {
      // Just make sure the data is cleared, and skip this index
      socketData.PartialReceivedData.Clear();
      ++i;
      continue;
    }

    // Make sure that the index inside the socket data is correct
    ErrorIf(socketData.ConnectionInfo.Index != i, "The intrusive connection index doesn't align with its spot in the array");

    // A boolean that specifies if a connection was erased
    bool connectionErased = false;

    // Store the receive-state of the connection
    ReceiveState state;

    // The maximum number of times we pump receive before we move on
    const size_t MaxReceives = 16;
    size_t numReceives = 0;

    // Loop until we hit a "would block" error
    do
    {
      // Attempt to receive data from the connection
      state = ReceiveData(socketData, i);

      // Increment the number of receives we have
      ++numReceives;
    }
    // Loop until we hit something other than data received...
    while (state == cDataReceived && numReceives < MaxReceives);

    // If we should move on to the next connection...
    if (state == cNextConnection)
    {
      // Increment the iterator
      ++i;
    }
    // Otherwise, if the connection is to be closed...
    else if (state == cCloseConnection)
    {
      // Close the connection (we don't need to increment the iterator since we push everything back)
      CloseConnection(i);
    }
  }
}

// Do the actual receiving of data (returns true if we should move on to the next socket)
TcpSocket::ReceiveState TcpSocket::ReceiveData(SocketData& socketData, size_t index)
{
  // Keep a buffer for received data
  byte buffer[BufferSize];

  // Receive data from each of the connections
  int result = recv((SOCKET)socketData.Handle, (char*)buffer, BufferSize, 0);

  // If we didn't get an error...
  if (result != SOCKET_ERROR)
  {
    // If the connection was closed...
    if (result == 0)
    {
      // Remove the connection
      return cCloseConnection;
    }
    // Otherwise, it must be good data!
    else
    {
      // Statistics
      TrackReceive(buffer, result, socketData.Handle);

      // If we're using the chunks protocol
      if (mProtocolSetup.Protocols & Protocol::Chunks)
      {
        // Handle chunks
        HandleChunks(socketData, buffer, result);
      }
      else
      {
        // Otherwise, we're not chunking up data so just receive it as normal
        HandleReceivedData(socketData, buffer, result);
      }
    }
  }
  else
  {
    // Break out if we hit a "would block" error
    if (WSAGetLastError() == WSAEWOULDBLOCK)
    {
      // We can continue to the next connection
      return cNextConnection;
    }

    // Otherwise, dispatch an error
    DispatchError();

    // Remove the connection
    return cCloseConnection;
  }
  
  // Otherwise, we received data (we should continue pumping)
  return cDataReceived;
}

// Handle chunks if we need to
void TcpSocket::HandleChunks(SocketData& socketData, const byte* buffer, size_t size)
{
  // Resize the buffer to hold packet data
  size_t lastSize = socketData.PartialReceivedData.Size();
  socketData.PartialReceivedData.Resize(socketData.PartialReceivedData.Size() + size);
  memcpy(socketData.PartialReceivedData.Data() + lastSize, buffer, size);

  // Did we handle a chunk?
  bool handledChunk;

  do
  {
    // Assume we will not handle a chunk
    handledChunk = false;

    // If we use length encoding...
    if (mProtocolSetup.ChunkType == ChunkType::LengthEncoded)
    {
      // Loop until we don't have room to read the encoded length...
      while (socketData.PartialReceivedData.Size() >= sizeof(ChunkLengthType))
      {
        // Read the length
        ChunkLengthType packetSize = *(ChunkLengthType*)socketData.PartialReceivedData.Data();

        // If we have the entire chunk's data...
        if (socketData.PartialReceivedData.Size() >= packetSize)
        {
          // Handle the received data (not including the chunk size)
          HandleReceivedData(socketData, socketData.PartialReceivedData.Data() + sizeof(ChunkLengthType), packetSize - sizeof(ChunkLengthType));

          // Erase the packet from the received data
          PodArray<byte>::range range(socketData.PartialReceivedData.Begin(), socketData.PartialReceivedData.Begin() + packetSize);
          socketData.PartialReceivedData.Erase(range);
        }
        else
        {
          // Break out of the loop since there is nothing left to read
          break;
        }
      }
    }
    // If we're using delimiting...
    else if (mProtocolSetup.ChunkType == ChunkType::Delimiter)
    {
      // Loop through all the received data
      for (size_t i = 0; i < socketData.PartialReceivedData.Size(); ++i)
      {
        // Do we have a match? (assume so until we find otherwise)
        bool match = true;

        // Loop through the delimiter bytes
        for (size_t j = 0; j < mProtocolSetup.Delimiter.Size() && (i + j) < socketData.PartialReceivedData.Size(); ++j)
        {
          // Does this byte of the delimiter deviate from the received data?
          if (socketData.PartialReceivedData[i + j] != mProtocolSetup.Delimiter[j])
          {
            // If so, we have no match
            match = false;
            break;
          }
        }

        // If we have a match...
        if (match)
        {
          // Handle the received data up to this delimiter (not including the delimiter)
          HandleReceivedData(socketData, socketData.PartialReceivedData.Data(), i);

          // Erase the packet from the received data
          PodArray<byte>::range range(socketData.PartialReceivedData.Begin(), socketData.PartialReceivedData.Begin() + i);
          socketData.PartialReceivedData.Erase(range);
          break;
        }
      }
    }
  }
  // Loop until we don't handle a chunk
  while (handledChunk == true);
}

// Determine what to do with the received data
void TcpSocket::HandleReceivedData(const SocketData& socketData, const byte* buffer, size_t size)
{
  // If the protocol we're using is to send packet objects...
  if (mProtocolSetup.RequiresCustomData())
  {
    // Since we have protocols enabled, we need to handle packets different (basically we need to read attached header data)
    HandleProtocols(socketData, buffer, size);
  }
  else
  {
    ReceivedDataEvent e(&socketData.ConnectionInfo, buffer, size);
    // Dispatch an event that we received data
    mDispatcher.Dispatch(Events::ReceivedData, &e);
  }
}

// Handle any protocols that we might have set
void TcpSocket::HandleProtocols(const SocketData& socketData, const byte* buffer, size_t size)
{
  // If we got an event message
  switch (buffer[0])
  {
    // If it's an event packet...
    case TCPSocketMessageType::Event:
    {
      // If we have the events protocol set...
      if (mProtocolSetup.Protocols & Protocol::Events)
      {
        // Pass the message off (strip the 1 byte header)
        HandleEventProtocol(socketData, buffer + 1, size - 1);
      }
      else
      {
        // Throw a warning since we never should have gotten here
        Warn("A packet was received for the event protocol, but the event protocol was not enabled (though other protocols were!)");
      }
      break;
    }

    // If it's a guid packet...
    case TCPSocketMessageType::Guid:
    {
      // If we have the guid protocol set...
      if (mProtocolSetup.Protocols & (Protocol::Guid | Protocol::GuidBroadcaster))
      {
        // Pass the message off (strip the 1 byte header)
        HandleGuidProtocol(socketData, buffer + 1, size - 1);
      }
      else
      {
        // Throw a warning since we never should have gotten here
        Warn("A packet was received for the guid protocol, but the guid protocol was not enabled (though other protocols were!)");
      }
      break;
    }

    // If it's data that was sent by the user (not an event...)
    case TCPSocketMessageType::UserData:
    {
      ReceivedDataEvent e(&socketData.ConnectionInfo, buffer + 1, size - 1);
      // Dispatch an event that we received data
      mDispatcher.Dispatch(Events::ReceivedData, &e);
      break;
    }
  }
}

// Handle the guid protocol
void TcpSocket::HandleGuidProtocol(const SocketData& socketData, const byte* buffer, size_t size)
{
  // Read the guid from the buffer
  NetGuid guid = *(const NetGuid*)buffer;

  // If the guid broadcaster protocol is enabled
  if (mProtocolSetup.Protocols & Protocol::GuidBroadcaster)
  {
    // We need to tell the other end what our guid is
    PodArray<byte> data; 
    MakeGuidPacket(guid, data);

    // Loop through all the connections
    for (size_t i = 0; i < mConnections.Size(); ++i)
    {
      // Send the data to each connection
      RawSend(mConnections[i], data.Data(), data.Size());

      // Statistics
      TrackSend(data.Data(), data.Size(), mConnections[i].Handle);
    }
  }

  // Add the guid to the tracked guids (both the hoster and the one using the protocol do this)
  // The hoster needs to know so that if anyone joins later, it can tell them who was already there
  mTrackedGuids.PushBack(guid);
  Sort(mTrackedGuids.All());
}

// Handle the event protocol
void TcpSocket::HandleEventProtocol(const SocketData& socketData, const byte* buffer, size_t size)
{
  // Create a binary deserializer (remove the const since the buffer loader doesn't respect it)
  //BinaryBufferLoader loader;
  DataTreeLoader loader;
  //loader.SetBuffer((byte*)buffer, size);
  Status status;
  loader.OpenBuffer(status, StringRange((const char*) buffer));

  // Deserialize the event (this should always work!)
  SendableEvent* event = SendableEvent::Load(loader);

  // Attach connection data as a component
  event->Connection = &socketData.ConnectionInfo;

  // Dispatch an event that we received data
  mDispatcher.Dispatch(event->EventId, event);

  // Free the event object
  event->Delete();
}

// Check if we are currently connected to anyone
bool TcpSocket::IsConnected()
{
  return mConnections.Size() > 0;
}

uint TcpSocket::GetConnectionCount()
{
  return mConnections.Size();
}

uint TcpSocket::GetIncomingConnectionCount()
{
  size_t count = 0;
  for(size_t i = 0; i < mConnections.Size(); ++i)
  {
    if(mConnections[i].ConnectionInfo.Incoming)
      ++count;
  }

  return count;
}

uint TcpSocket::GetOutgoingConnectionCount()
{
  size_t count = 0;
  for(size_t i = 0; i < mConnections.Size(); ++i)
  {
    if(!mConnections[i].ConnectionInfo.Incoming)
      ++count;
  }

  return count;
}

// Get the Guid for this client
NetGuid TcpSocket::GetGuid()
{
  return mGuid;
}

// Get a range of all tracked guids
Array<NetGuid>::range TcpSocket::GetTrackedGuids()
{
  return mTrackedGuids.All();
}

// Occurs when the engine updates
void TcpSocket::Update(UpdateEvent* event)
{
  // Handle incoming connections
  HandleIncomingConnections();

  // Handle outgoing connections
  HandleOutgoingConnections();

  // Handle all incoming data
  HandleIncomingData();

  // Handle all outgoing data (buffered data that couldn't be sent...)
  HandleOutgoingData();
}

// Close a particular connection
void TcpSocket::CloseConnection(uint index)
{
  // Get the socket data
  SocketData& socketData = mConnections[index];

  // If we have no socket at this location, ignore this!
  if (socketData.Handle == 0)
    return;

  // Close the connection and remove it from the list
  closesocket((SOCKET)socketData.Handle);
  
  // We need to make a copy of the connection data so that when we dispatch the 'Disconnected event'
  // we don't need to worry about them touching the socket or closing the connection
  ConnectionData tempCopy = socketData.ConnectionInfo;

  // Clear out this socket to be re-used later
  socketData.Handle = 0;
  socketData.PartialReceivedData.Clear();
  socketData.PartialSentData.Clear();
  socketData.ConnectionInfo.Address = 0;
  socketData.ConnectionInfo.Host = String();
  socketData.ConnectionInfo.Port = 0;
  socketData.ConnectionInfo.Incoming = false;

  ConnectionEvent e(&tempCopy);
  // Dispatch a connection failed event
  mDispatcher.Dispatch(Events::Disconnected, &e);
}

// Broadcast an error event
void TcpSocket::DispatchError()
{
  // Get the error code
  int errorCode = WSAGetLastError();

  // Get the error string
  String error = GetErrorString(errorCode);

  TextErrorEvent e(error, errorCode);
  // Dispatch an error
  mDispatcher.Dispatch(Events::SocketError, &e);
}

// Validates a server socket or creates one if it's invalid
bool TcpSocket::ValidateServer()
{
  // If the socket is invalid...
  if (mServer == INVALID_SOCKET)
  {
    // Create the socket
    mServer = CreateSocket();

    // Return true if the server is not invalid, false otherwise
    return mServer != INVALID_SOCKET;
  }

  // Otherwise, it should be valid
  return true;
}

// Create a new socket with all the options we want
SocketHandle TcpSocket::CreateSocket()
{
  // Create the socket
  SocketHandle newSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // If for some reason the socket is invalid...
  if (newSocket == INVALID_SOCKET)
  {
    // Dispatch an error and the socket was not valid
    DispatchError();
  }
  else
  {
    // Now that we created a socket, make sure to set all the options we want
    SetSocketOptions(newSocket);
  }

  // Return the socket (whether it was created or not
  return newSocket;
}

// Setup all the socket options that we want
void TcpSocket::SetSocketOptions(SocketHandle socket)
{
  // Make sure to set it so the socket won't buffer up small packets (turn off the Nagle algorithm)
  BOOL noDelay = 1;
  setsockopt((SOCKET)socket, IPPROTO_TCP, TCP_NODELAY, (char*) &noDelay, sizeof(noDelay));

  // Turn socket blocking off
  u_long noBlocking = 1;
  ioctlsocket((SOCKET)socket, FIONBIO, &noBlocking);
}

// Resolve a name into an IP address
unsigned long TcpSocket::ResolveHost(StringParam host)
{
  // Is the string an IP address?
  unsigned long ip = inet_addr(host.c_str());

  // If we didn't parse into an IP...
  if (ip == INADDR_NONE)
  {
    // Is the string a host name?
    hostent* result = gethostbyname(host.c_str());

    // If no result was returned from the name resolution
    if (result == nullptr)
    {
      // Dispatch an error
      DispatchError();
    }
    else
    {
      // set the first IP that was found
      ip = *(unsigned long*)result->h_addr_list[0];
    }
  }

  // Return the IP that we parsed
  return ip;
}

// Get an error string for a particular error code
String TcpSocket::GetErrorString(int errorCode)
{
  // Allocate the error string and store it in a temporary
  LPSTR errorString = nullptr;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, (LPSTR)&errorString, 0, 0);
  
  // Copy the string to our own string
  String result = errorString;

  // Free the one allocated by the system
  LocalFree(errorString);

  // Return our own string result
  return result;
}

// Check if the last error was a problem or not
bool TcpSocket::IsError(int result)
{
  // If the result was an error value...
  if (result == SOCKET_ERROR)
  {
    // Get the last error code
    unsigned long error = WSAGetLastError();

    // Return if it's not the would block error
    return error != WSAEWOULDBLOCK;
  }

  // Otherwise, we didn't have an error!
  return false;
}

// Verify that nothing went wrong
void TcpSocket::VerifyResult(int result)
{
  // If there was an error
  if (IsError(result))
  {
    // Get the last error code
    unsigned long error = WSAGetLastError();

    // Skip the 'connection disconnected error'
    if(error == WSAECONNABORTED || error == WSAECONNRESET || error == WSAENETRESET)
      return;

    char errorMessage[1024] = {0};
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, errorMessage, 1023, nullptr);
    Warn("SocketHandle failure '%s'", errorMessage);
    DoNotifyWarning("TcpSocket", String::Format("A socket call returned a failing result of '%s'", errorMessage).c_str());
  }
}

} // namespace Zero
