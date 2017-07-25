///////////////////////////////////////////////////////////////////////////////
///
/// \file TcpSocket.cpp
/// Implementation of the TcpSocket class.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "TcpSocket.hpp"
#include "Engine/EngineEvents.hpp"
#include "Engine/Engine.hpp"
#include "Serialization/Binary.hpp"
#include "Diagnostic/Diagnostic.hpp"

// Defines
#define infinite_loop for(;;)


// The general max size of temporary buffers
const size_t BufferSize = 8192;

// Type-defines
typedef size_t ChunkLengthType;

namespace Zero
{
ZeroDeclareTypeImpl(ConnectionData);
ZeroDeclareTypeImpl(ConnectionEvent);
ZeroDeclareTypeImpl(ReceivedDataEvent);

// Constructor
ConnectionData::ConnectionData()
{
  Index = InvalidIndex;
}

void ConnectionEvent::InitializeMeta(MetaType* meta)
{

}

void ReceivedDataEvent::InitializeMeta(MetaType* meta)
{

}


// Initialize the meta for connection data
void ConnectionData::InitializeMeta(MetaType* meta)
{
  BindMemberProperty(Host);
  BindMemberProperty(Address);
  BindMemberProperty(Port);
}

// Constructor
ConnectionEvent::ConnectionEvent(const ConnectionData* connectionInfo)
{

}

// Constructor
ReceivedDataEvent::ReceivedDataEvent(const ConnectionData* connectionInfo, const byte* data, size_t size) : ConnectionEvent(connectionInfo)
{
  // Resize the data buffer to be the size of the data we're getting
  Data.resize(size);

  // Copy over the data
  memcpy(Data.data(), data, size);
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
  return false;
}

// Define the meta-implementations
ZeroDeclareTypeImpl(TcpSocket);

void TcpSocket::InitializeMeta(MetaType* meta)
{

}

// Constructor (no protocols)
TcpSocket::TcpSocket(String debugName)
{
  Initialize();
}

// Constructor (basic protocol)
TcpSocket::TcpSocket(Protocol::Type protocols, String debugName)
{
  Initialize();
}

// Constructor (protocol setup info)
TcpSocket::TcpSocket(const ProtocolSetup& setup, String debugName)
{
  Initialize();
}


// Initialize the socket (called by the constructors)
void TcpSocket::Initialize()
{

}

// Destructor
TcpSocket::~TcpSocket()
{

}

// Attempt to connect to a server on the given port
void TcpSocket::Connect(String host, int port)
{

}

// Attempt to connect to a host IP on the given port (the IP buffer is a 4 byte buffer that stores the IP address, NOT a string)
//void TcpSocket::Connect(unsigned long ip, int port)
//{
//}

// Internal connection function
void TcpSocket::InternalConnect(const ConnectionData& info)
{

}

// Listen for incoming connections
bool TcpSocket::Listen(int port, uint maxConnections)
{


  // Return a success!
  return false;
}

// Close all activity (whether listening or connected to a server)
void TcpSocket::Close()
{

}

// Send a message to a specific connection index
void TcpSocket::SendBufferTo(const byte* buffer, size_t size, size_t index)
{

}

void TcpSocket::SendBufferToAll(const byte* buffer, size_t size)
{

}

// Send a message to all connections except a particular connection index
void TcpSocket::SendBufferToAllExcept(const byte* buffer, size_t size, size_t exceptIndex)
{

}

// Send an event to a specific connection index
void TcpSocket::SendTo(StringParam eventId, SendableEvent* event, uint index)
{
}
// Send an event to all connections
void TcpSocket::SendToAll(StringParam eventId, SendableEvent* event)
{

}
// Send an event to all connections except a particular connection index
void TcpSocket::SendToAllExcept(StringParam eventId, SendableEvent* event, uint exceptIndex)
{

}

// Make a user data packet and output the buffer
void TcpSocket::MakeUserPacket(const byte* data, size_t size, PodArray<byte>& dataOut)
{

}

// Make a packet given a buffer
void TcpSocket::MakeEventPacket(SendableEvent* event, PodArray<byte>& dataOut)
{

}

// Setup a packet (mainly the header)
void TcpSocket::SetupPacket(MessageType::Enum messageType, PodArray<byte>& dataOut)
{

}

// Finish setting up the packet
void TcpSocket::FinalizePacket(PodArray<byte>& dataOut)
{

}

// Extract a message into a buffer and return the number of bytes written
size_t TcpSocket::ExtractIntoBuffer(const BinaryBufferSaver& messageConst, byte* buffer, size_t bufferSize)
{

  // Return the size / amount of data extracted
  return 0;
}

// Handle incoming connections
void TcpSocket::HandleIncomingConnections()
{

}

// Handle outgoing connections
void TcpSocket::HandleOutgoingConnections()
{

}

// Handle incoming data
void TcpSocket::HandleIncomingData()
{

}

// Do the actual receiving of data (returns true if we should move on to the next socket)
TcpSocket::ReceiveState TcpSocket::ReceiveData(SocketData& socketData, size_t index)
{

  // Otherwise, we recieved data (we should continue pumping)
  return TcpSocket::cDataReceived;
}

// Handle chunks if we need to
void TcpSocket::HandleChunks(SocketData& socketData, const byte* buffer, size_t size)
{

}

// Determine what to do with the recieved data
void TcpSocket::HandleReceivedData(const SocketData& socketData, const byte* buffer, size_t size)
{

}

// Handle any protocols that we might have set
void TcpSocket::HandleProtocols(const SocketData& socketData, const byte* buffer, size_t size)
{

}

// Handle the event protocol
void TcpSocket::HandleEventProtocol(const SocketData& socketData, const byte* buffer, size_t size)
{

}

// Check if we are currently connected to anyone
bool TcpSocket::IsConnected()
{
  return mConnections.size() > 0;
}

// Get the number of connections we have
uint TcpSocket::GetConnectionCount()
{
  return mConnections.size();
}

// Get the GUID for this client
NetGuid TcpSocket::GetGuid()
{
  return mGuid;
}

Array<NetGuid>::range TcpSocket::GetTrackedGuids()
{
  return mTrackedGuids.all();
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
}

// Close a particular connection
void TcpSocket::CloseConnection(uint index)
{

}

// Broadcast an error event
void TcpSocket::DispatchError()
{

}

// Validates a server socket or creates one if it's invalid
bool TcpSocket::ValidateServer()
{

  // Otherwise, it should be valid
  return true;
}

// Create a new socket with all the options we want
SocketHandle TcpSocket::CreateSocket()
{

  // Return the socket (whether it was created or not
  return 0;
}

// Setup all the socket options that we want
void TcpSocket::SetSocketOptions(SocketHandle socket)
{

}

// Resolve a name into an IP address
unsigned long TcpSocket::ResolveHost(String host)
{
  return 0;
}


// Get an error string for a particular error code
String TcpSocket::GetErrorString(int errorCode)
{
  // Return our own string result
  return "Error";
}

// Check if the last error was a problem or not
bool TcpSocket::IsError(int result)
{
  // Otherwise, we didn't have an error!
  return true;
}

}
