///////////////////////////////////////////////////////////////////////////////
///
/// \file TcpSocket.hpp
/// Declaration of the TcpSocket class.
///
/// Authors: Trevor Sundberg.
/// Copyright 2010-2011, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// The events we use or send out
namespace Events
{
  DeclareEvent(SocketError);
  DeclareEvent(ConnectionCompleted);
  DeclareEvent(ConnectionFailed);
  DeclareEvent(Disconnected);
  DeclareEvent(ReceivedData);
}

// Forward declaration
class SendableEvent;
class UpdateEvent;
class BinaryBufferSaver;
class GameSession;

//-------------------------------------------------------------- Connection Data
// All the data we need to know about connections
struct ConnectionData
{
  // Connection data requires a meta so it can be attached as a component to events
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // A constant that is set when a connection index is not initialized
  static const uint InvalidIndex = (uint)-1;

  // Constructor
  ConnectionData();

  // Connection data
  String Host;
  uint Address;
  uint Port;
  uint Index;
  bool Incoming;
};

//------------------------------------------------------------- Connection Event
// A connection event Contains basic information about a connection (IP address, host-name, port, index, etc)
class ConnectionEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Constructor
  ConnectionEvent(const ConnectionData* connectionInfo);

  // Connection data available to the user
  String Host;
  uint Address;
  uint Port;
  uint Index;
  bool Incoming;
};

//---------------------------------------------------------- Received Data Event
// A received-data event stores information such as who the data came from, and the data itself
class ReceivedDataEvent : public ConnectionEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Constructor
  ReceivedDataEvent(const ConnectionData* connectionInfo, const byte* data, size_t size);

  // The data that was received (packet headers are not included in this data)
  PodArray<byte> Data;

  // This is only so we can make this available to script
  String Buffer;
};

// Type-defines
typedef u64 SocketHandle;
typedef u32 NetGuid;

//------------------------------------------------------------------ SocketHandle Data
// A structure to represent socket data
struct SocketData
{
  ConnectionData ConnectionInfo;

  // A socket id of 0 indicates that it this slot is not being used
  SocketHandle Handle;

  // Received data that is being buffered because it was not a full packet
  PodArray<byte> PartialReceivedData;

  // Any data that wasn't sent will get queued up here
  PodArray<byte> PartialSentData;
};

// Any extra protocols we'd like to tack on top of the socket
// Guid   - Assigns Guids to every peer in a topology (useful for lock-step and peer-to-peer networks)
// Events - Allows the sockets to send serializable events that get broadcasted out on the other end
// Chunks - Data is always received in full chunks rather than broken up packets (always enabled if any other protocol is enabled)
// Otherwise, data is simply sent as is over a TCP connection, which is useful when implementing other protocols
namespace Protocol
{
  enum Enum
  {
    Events = 1,
    Chunks = 2,
    Guid = 4,
    GuidBroadcaster = 8
  };

  // The flags that require custom data
  const uint FlagsThatNeedCustomData = Chunks | Guid | GuidBroadcaster;

  // We use a uint type since this is a bit field
  typedef uint Type;
}

// The message types we use (for our own protocols)
DeclareEnum3(TCPSocketMessageType, Event, Guid, UserData);

// In chunks mode there are two ways of doing chunks, by delimiter or by length encoding
DeclareEnum2(ChunkType, LengthEncoded, Delimiter);

//--------------------------------------------------------------- Protocol Setup
// A structure that we create to setup protocol options
struct ProtocolSetup
{
  // Constructor
  ProtocolSetup();

  // Is a protocol set that requires message headers and length encoded chunks?
  bool RequiresCustomData();

  // The protocols that we want enabled
  Protocol::Type Protocols;
  
  // Only applies if Protocol::Chunks is set
  // This specifies the type of chunking we'd like to do (whether length encoded or delimiter)
  ChunkType::Enum ChunkType;

  // Only applies if Protocol::Chunks is set and the chunk type is set to delimiter
  // The delimiter that separates whole packet chunks from each other
  PodArray<byte> Delimiter;
};

DeclareEnum2(TcpSocketBind, Any, Loopback);

//------------------------------------------------------------------- TCP SocketHandle
/// Manages all the client/server/peer connections .
class TcpSocket : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constants.
  static const uint MaxPossibleConnections = (uint)-1;

  /// Constructor.
  TcpSocket();

  /// Constructor (no protocols).
  TcpSocket(StringParam debugName);

  /// Constructor (basic protocol).
  TcpSocket(Protocol::Type protocols, StringParam debugName);

  /// Constructor (protocol setup info).
  TcpSocket(const ProtocolSetup& setup, StringParam debugName);

  /// Destructor.
  ~TcpSocket();

  /// Attempt to connect to a host on the given port.
  void Connect(StringParam host, int port);

  /// Listen for incoming connections.
  bool Listen(int port, uint maxConnections = MaxPossibleConnections);
  bool Listen(int port, uint maxConnections, TcpSocketBind::Enum bindTo);

  /// Get the number of connections we have.
  uint GetConnectionCount();

  /// Get the number of incoming connections.
  uint GetIncomingConnectionCount();

  /// Get the number of outgoing connections.
  uint GetOutgoingConnectionCount();

  /// Close all activity (whether listening or connected to a server).
  void Close();

  /// Send a message to a specific connection index.
  void SendBufferTo(const byte* buffer, size_t size, size_t index);
  /// Send a message to all connections.
  void SendBufferToAll(const byte* buffer, size_t size);
  /// Send a message to all connections except a particular connection index.
  void SendBufferToAllExcept(const byte* buffer, size_t size, size_t exceptIndex);

  /// Send an event to a specific connection index.
  void SendTo(StringParam eventId, SendableEvent* event, uint index);
  /// Send an event to all connections.
  void SendToAll(StringParam eventId, SendableEvent* event);
  /// Send an event to all connections except a particular connection index.
  void SendToAllExcept(StringParam eventId, SendableEvent* event, uint exceptIndex);
  /// Send an event to all connections and dispatch on self.
  void SendToAllAndSelf(StringParam eventId, SendableEvent* event);

  /// Check if we are currently connected to anyone.
  bool IsConnected();

  /// Get the Guid for this client.
  NetGuid GetGuid();

  /// Get a range of all tracked guids.
  Array<NetGuid>::range GetTrackedGuids();

private:
  
  // Initialize the socket (called by the constructors)
  void Initialize();

  // Internal connection function
  void InternalConnect(const ConnectionData& info);

  // Occurs when the engine updates
  void Update(UpdateEvent* event);

  // Dispatch an error event
  void DispatchError();

  // Validates a server socket or creates one if it's invalid
  bool ValidateServer();

  // Resolve a name into an IP address
  unsigned long ResolveHost(StringParam host);

  // Create a new socket with all the options we want
  SocketHandle CreateSocket();

  // Setup all the socket options that we want
  static void SetSocketOptions(SocketHandle socket);

  // Get an error string for a particular error code
  static String GetErrorString(int errorCode);

  // Check if the last error was a problem or not
  bool IsError(int result);

  // Verify that nothing went wrong
  void VerifyResult(int result);

  // Close a particular connection
  void CloseConnection(uint index);

  // Extract a message into a buffer and return the number of bytes written
  static size_t ExtractIntoBuffer(const BinaryBufferSaver& message, byte* buffer, size_t bufferSize);

  // Send directly to a particular socket (if there's any queued data, this will fail and instead queue up the data)
  // If not all data is sent, it will also queue up the data (which we attempt to send in HandleOutgoingData)
  void RawSend(SocketData& socketData, const byte* data, size_t size);

  // Track a send
  void TrackSend(const byte* data, size_t size, SocketHandle socket);

  // Track a receive
  void TrackReceive(const byte* data, size_t size, SocketHandle socket);

  // Print our data
  void PrintData(const char* mode, const byte* data, size_t size);

  // Re-uses a socket data structure or creates a new one
  // Returns nullptr if we reach the connection limit
  SocketData& FindOpenOrCreateSocketData();

  // Handle incoming connections
  void HandleIncomingConnections();

  // Handle outgoing connections
  void HandleOutgoingConnections();

  // Handle incoming data
  void HandleIncomingData();

  // Handle outgoing data
  void HandleOutgoingData();

  // Handle any protocols that we might have set
  void HandleProtocols(const SocketData& socketData, const byte* buffer, size_t size);

  // Handle the event protocol
  void HandleEventProtocol(const SocketData& socketData, const byte* buffer, size_t size);

  // Handle the guid protocol
  void HandleGuidProtocol(const SocketData& socketData, const byte* buffer, size_t size);

  // Tells us the state of receiving data from a particular connection
  enum ReceiveState
  {
    cDataReceived,
    cNextConnection,
    cCloseConnection,
  };

  // Do the actual receiving of data (returns true if we should move on to the next socket)
  ReceiveState ReceiveData(SocketData& socketData, size_t index);

  // Handle chunks if we need to
  void HandleChunks(SocketData& socketData, const byte* buffer, size_t size);

  // Determine what to do with the received data
  void HandleReceivedData(const SocketData& socketData, const byte* buffer, size_t size);

  // Setup a packet (mainly the header)
  void SetupPacket(TCPSocketMessageType::Enum messageType, PodArray<byte>& dataOut);

  // Finish setting up the packet
  void FinalizePacket(PodArray<byte>& dataOut);

  // Make a packet given a buffer (the buffer is expected to be of size BufferSize)
  inline void MakeEventPacket(SendableEvent* event, PodArray<byte>& dataOut);

  // Make a user data packet and output the buffer
  inline void MakeUserPacket(const byte* data, size_t size, PodArray<byte>& dataOut);

  // Make a guid packet
  inline void MakeGuidPacket(NetGuid guid, PodArray<byte>& dataOut);

public:

  // What version of serialization to use when sending events. Defaulted to the
  // current version but may need to be set to old versions for legacy.
  DataVersion::Enum mDataVersion;

private:

  // Random number generator for guids
  static Math::Random GuidGenerator;

  // Only a valid Guid if using the Guid protocol
  NetGuid mGuid;

  // Store a list of connections
  // We never remove entries (but we do re-use slots as the connections get closed)
  Array<SocketData> mConnections;

  // Store a list of pending connections
  Array<SocketData> mPendingOutgoingConnections;

  // All the guids for other clients in a network
  Array<NetGuid> mTrackedGuids;

  // Store the maximum number of allowed incoming connections
  size_t mMaxIncomingConnections;

  // The socket we use as the main server or client socket
  SocketHandle mServer;

  // Store the extra protocols we're using
  ProtocolSetup mProtocolSetup;

  // A name used just for debugging
  String mDebugName;

  // The number of packets sent
  int mSendCount;

  // The amount of data sent
  int mSendSize;

  // The number of packets received
  int mReceiveCount;

  // The amount of data received
  int mReceieveSize;
};

} // namespace Zero
