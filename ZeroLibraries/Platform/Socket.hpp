///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean, Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "String/String.hpp"
#include "Utility/Status.hpp"
#include "Containers/Array.hpp"
#include "Containers/BitStream.hpp"
#include "PrivateImplementation.hpp"
#include "SocketEnums.hpp"
#include "SocketConstants.hpp"
#include "Platform/OsHandle.hpp"

namespace Zero
{

/// Converts a host byte order value to a network byte order value
/// On little endian platforms, flips byte order to big endian
/// On big endian platforms, does nothing
/// (Named htons/htonl on most platforms)
ZeroShared u16 HostToNetworkShort(u16 hostShort);
ZeroShared s16 HostToNetworkShort(s16 hostShort);
ZeroShared u32 HostToNetworkLong(u32 hostLong);
ZeroShared s32 HostToNetworkLong(s32 hostLong);

/// Converts a network byte order value to a host byte order value
/// On little endian platforms, flips byte order to little endian
/// On big endian platforms, does nothing
/// (Named ntohs/ntohl on most platforms)
ZeroShared u16 NetworkToHostShort(u16 networkShort);
ZeroShared s16 NetworkToHostShort(s16 networkShort);
ZeroShared u32 NetworkToHostLong(u32 networkLong);
ZeroShared s32 NetworkToHostLong(s32 networkLong);

//---------------------------------------------------------------------------------//
//                                SocketAddress                                    //
//---------------------------------------------------------------------------------//

/// Network host identifier
class ZeroShared SocketAddress
{
public:
  /// Creates an empty socket address
  SocketAddress();

  /// Copy Constructor
  SocketAddress(const SocketAddress& rhs);

  /// Copy Assignment Operator
  SocketAddress& operator =(const SocketAddress& rhs);

  /// Comparison Operators
  bool operator ==(const SocketAddress& rhs) const;
  bool operator !=(const SocketAddress& rhs) const;
  bool operator  <(const SocketAddress& rhs) const;

  /// Returns true if the socket address is not empty, else false
  operator bool(void) const;

  /// Returns true if the socket address is empty, else false
  bool IsEmpty() const;

  /// Returns the socket address family
  SocketAddressFamily::Enum GetAddressFamily() const;

  /// Sets the socket address to the most preferred IPv4 socket address resolved from a host name and port number as specified
  /// Host provided may contain either a host name (like "localhost" or "www.example.com") or numeric address string (like "127.0.0.1" or "::1")
  /// Port provided must contain a port number
  /// Will block until address resolution completes or times out
  /// Note: Preferred socket address order is documented in RFC 3484
  void SetIpv4(Status& status, StringParam host, uint port);
  void SetIpv4(Status& status, StringParam host, uint port, SocketAddressResolutionFlags::Enum addressResolutionFlags);

  /// Sets the socket address to the most preferred IPv6 socket address resolved from a host name and port number as specified
  /// Host provided may contain either a host name (like "localhost" or "www.example.com") or numeric address string (like "127.0.0.1" or "::1")
  /// Port provided must contain a port number
  /// Will block until address resolution completes or times out
  /// Note: Preferred socket address order is documented in RFC 3484
  void SetIpv6(Status& status, StringParam host, uint port);
  void SetIpv6(Status& status, StringParam host, uint port, SocketAddressResolutionFlags::Enum addressResolutionFlags);

  /// Sets the IPv4/IPv6 socket address port
  void SetIpPort(Status& status, uint port);
  /// Returns the IPv4/IPv6 socket address port, else 0
  uint GetIpPort(Status& status) const;

  /// Clears the socket address
  void Clear();

  /// Address data
  ZeroDeclarePrivateDataBytes(SocketAddressStorageBytes);
};

/// Serializes a socket address (currently only defined for InternetworkV4 and InternetworkV6 socket addresses)
/// Returns the number of bits serialized if successful, else 0
ZeroShared Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, SocketAddress& socketAddress);

/// Resolves the most preferred socket address from a host name and service name as specified
/// Host provided may contain either a host name (like "localhost" or "www.example.com") or numeric address string (like "127.0.0.1" or "::1")
/// Service provided may contain either a service name (like "http") or port number string (like "80")
/// Will block until address resolution completes or times out
/// Note: Preferred socket address order is documented in RFC 3484
/// (Named getaddrinfo on most platforms)
ZeroShared SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                              SocketProtocol::Enum protocol, SocketType::Enum type, SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                              SocketProtocol::Enum protocol, SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                              SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service);

/// Resolves all associated socket addresses (in preferred socket address order) from a host name and service name as specified
/// Host provided may contain either a host name (like "localhost" or "www.example.com") or numeric address string (like "127.0.0.1" or "::1")
/// Service provided may contain either a service name (like "http") or port number string (like "80")
/// Will block until address resolution completes or times out
/// Note: Preferred socket address order is documented in RFC 3484
/// (Named getaddrinfo on most platforms)
ZeroShared Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                                          SocketProtocol::Enum protocol, SocketType::Enum type, SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                                          SocketProtocol::Enum protocol, SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                                          SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressResolutionFlags::Enum addressResolutionFlags);
ZeroShared Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service);

/// Resolves the host name and service name from a socket address as specified
/// Host returned may contain either a host name (like "localhost" or "www.example.com") or numeric address string (like "127.0.0.1" or "::1")
/// Service returned may contain either a service name (like "http") or port number string (like "80")
/// Will block until name resolution completes or times out
/// (Named getnameinfo on most platforms)
ZeroShared Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address, SocketNameResolutionFlags::Enum nameResolutionFlags);
ZeroShared Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address);

/// Converts a valid socket address to a numeric address string, else String()
/// Note: Only translates the address host portion, does not translate port numbers
/// (Named inet_ntop on most platforms)
ZeroShared String SocketAddressToString(Status& status, SocketAddressFamily::Enum addressFamily, const SocketAddress& address);
ZeroShared String SocketAddressToString(SocketAddressFamily::Enum addressFamily, const SocketAddress& address);

/// Converts a valid numeric address string to a socket address, else SocketAddress()
/// Note: Only translates the address host portion, does not translate port numbers
/// (Named inet_pton on most platforms)
ZeroShared SocketAddress StringToSocketAddress(Status& status, SocketAddressFamily::Enum addressFamily, StringParam address);
ZeroShared SocketAddress StringToSocketAddress(SocketAddressFamily::Enum addressFamily, StringParam address);

/// Returns true if the socket address represents a valid IPv4 host, else false
/// This does not imply that the host exists, only that the socket address is well formed
ZeroShared bool IsValidIpv4Address(const SocketAddress& address);
/// Returns true if the socket address represents a valid IPv6 host, else false
/// This does not imply that the host exists, only that the socket address is well formed
ZeroShared bool IsValidIpv6Address(const SocketAddress& address);

/// Returns true if the numeric address string represents a valid IPv4 host, else false
/// This does not imply that the host exists, only that the numeric address string is well formed
ZeroShared bool IsValidIpv4Address(StringParam address);
/// Returns true if the numeric address string represents a valid IPv6 host, else false
/// This does not imply that the host exists, only that the numeric address string is well formed
ZeroShared bool IsValidIpv6Address(StringParam address);

/// Converts a valid IPv4 socket address to a numeric address string, else String()
ZeroShared String Ipv4AddressToString(const SocketAddress& address);
/// Converts a valid IPv6 socket address to a numeric address string, else String()
ZeroShared String Ipv6AddressToString(const SocketAddress& address);

/// Returns a port as a numeric string, else String()
ZeroShared String PortToString(uint port);

/// Converts a valid IPv4 socket address to a numeric address string with appended port number, else String()
ZeroShared String Ipv4AddressToStringWithPort(const SocketAddress& address);
/// Converts a valid IPv6 socket address to a numeric address string with appended port number, else String()
ZeroShared String Ipv6AddressToStringWithPort(const SocketAddress& address);

/// Converts a valid IPv4 numeric address string to a socket address, else SocketAddress()
ZeroShared SocketAddress StringToIpv4Address(StringParam address);
ZeroShared SocketAddress StringToIpv4Address(StringParam address, ushort port);

/// Converts a valid IPv6 numeric address string to a socket address, else SocketAddress()
ZeroShared SocketAddress StringToIpv6Address(StringParam address);
ZeroShared SocketAddress StringToIpv6Address(StringParam address, ushort port);

//---------------------------------------------------------------------------------//
//                                    Socket                                       //
//---------------------------------------------------------------------------------//

/// Network host endpoint
/// Facilitates interprocess communication
class ZeroShared Socket
{
public:
  //
  // Static Member Functions
  //

  /// Returns the maximum number of connections the OS can backlog while listening
  static size_t GetMaxListenBacklog();

  /// Returns true if the error code is a common receive error, else false
  /// These errors are considered continuable and should not be reported via asserts
  static bool IsCommonReceiveError(int extendedErrorCode);

  /// Returns true if the error code is a common accept error, else false
  /// These errors are considered continuable and should not be reported via asserts
  static bool IsCommonAcceptError(int extendedErrorCode);

  /// Returns true if the error code is a common connect error, else false
  /// These errors are considered continuable and should not be reported via asserts
  static bool IsCommonConnectError(int extendedErrorCode);

  /// Returns true if the platform's underlying socket library is initialized (reference count greater than zero), else false
  static bool IsSocketLibraryInitialized();

  /// Initializes the platform's underlying socket library (acquiring the resources needed to provide socket functionality)
  /// Safe to call multiple times (even without matching Uninitialize calls), internally reference counted
  /// If already initialized (reference count greater than zero), only increments the reference count
  /// If not yet initialized (reference count of zero), initializes the socket library and if successful, increments the reference count
  /// Note: Manually initializing and uninitializing the platform socket library is optional (as we automatically do so internally when needed),
  ///       however, it's still recommended to wrap your usage of the socket API starting with InitializeSocketLibrary and ending with UninitializeSocketLibrary
  ///       in order to avoid unnecessarily initializing/uninitializing the socket library repetitively during usage of the socket API
  static void InitializeSocketLibrary(Status& status);

  /// Uninitializes the platform's underlying socket library (releasing the resources needed to provide socket functionality)
  /// Safe to call multiple times (even without matching Initialize calls), internally reference counted
  /// If not initialized (reference count of zero), does nothing
  /// If initialized more than once (reference count greater than one), decrements the reference count
  /// If initialized once (reference count of one), decrements the reference count and uninitializes the socket library
  /// Note: Manually initializing and uninitializing the platform socket library is optional (as we automatically do so internally when needed),
  ///       however, it's still recommended to wrap your usage of the socket API starting with InitializeSocketLibrary and ending with UninitializeSocketLibrary
  ///       in order to avoid unnecessarily initializing/uninitializing the socket library repetitively during usage of the socket API
  static void UninitializeSocketLibrary(Status& status);

  //
  // Non-Static Member Functions
  //

  /// Creates a closed socket
  Socket();

  /// Destroys the socket (closes the socket if still open)
  ~Socket();

  /// Move Constructor
  Socket(MoveReference<Socket> rhs);

  /// Move Assignment Operator
  Socket& operator =(MoveReference<Socket> rhs);

  /// Returns true if the socket is opened, else false
  /// This does not imply that the socket is connected or bound, only that we have a valid socket handle
  bool IsOpen() const;

  /// Returns the socket address family if open, else SocketAddressFamily::Unspecified
  SocketAddressFamily::Enum GetAddressFamily() const;

  /// Returns the socket type if open, else SocketType::Unspecified
  SocketType::Enum GetType() const;

  /// Returns the socket protocol if open, else SocketProtocol::Unspecified
  SocketProtocol::Enum GetProtocol() const;

  /// Returns true if the socket is bound to a local address, else false
  bool IsBound() const;

  /// Returns the local address this socket is bound to, else SocketAddress()
  SocketAddress GetBoundLocalAddress() const;

  /// Returns true if the socket is set in listening mode, else false
  bool IsListening() const;

  /// Returns true if the socket is set in blocking mode, else false
  bool IsBlocking() const;

  /// Returns true if there is a remote address currently associated with this socket, else false
  /// This does not imply that the connection to the remote address is currently valid, only that the most recent connect operation succeeded and hasn't been shutdown since
  bool HasConnectedRemoteAddress() const;

  /// Returns the remote address currently associated with this socket, else SocketAddress()
  /// For connectionless sockets (Datagram socket types), this will be the remote address specified in the last connect call
  /// For connection-based sockets (Stream socket types), this will be the remote address specified in the last connect call after a connection has been successfully established
  /// This does not imply that the connection to the remote address is currently valid, only that the most recent connect operation succeeded and hasn't been shutdown since
  SocketAddress GetConnectedRemoteAddress() const;

  /// Opens the closed socket as a host for the specified protocol (closes the socket if already open)
  void Open(Status& status, SocketAddressFamily::Enum addressFamily, SocketType::Enum type, SocketProtocol::Enum protocol);

  /// Associates the open socket with the specified local address
  /// For IP addresses, if the host or port are left empty the OS will automatically determine their values
  void Bind(Status& status, const SocketAddress& localAddress);

  /// Sets the bound socket to listening mode, ready to accept incoming connections
  /// Valid only for connection-based sockets
  void Listen(Status& status, uint backlog);

  /// Sets the open socket's blocking mode
  /// If enabled the socket will block on non-immediate operation calls (such as send and receive)
  void SetBlocking(Status& status, bool blocking);

  /// Accepts an incoming connection on the listening socket
  /// Outputs a bound socket connected to the remote host
  /// Valid only for connection-based sockets
  void Accept(Status& status, Socket* connectionOut);

  /// Attempts to connect to the specified remote address on the non-listening socket
  /// Will block until the connect attempt completes (unless the socket is set to non-blocking)
  /// For connectionless sockets this only scopes send and receive operations to this address
  void Connect(Status& status, const SocketAddress& remoteAddress);

  /// Shuts down the specified connected socket operations
  /// Safe to call from other threads (blocking calls will simply terminate with an error)
  /// The socket is still considered open after this call, to free socket resources use close
  void Shutdown(Status& status, SocketIo::Enum io);

  /// Closes the open socket and frees associated resources
  /// Safe to call from other threads (blocking calls will simply terminate with an error)
  /// The socket is considered closed after this call
  void Close(Status& status);

  /// Sends data on the connected socket to the connected remote address
  /// Will block if the send buffer is full (unless the socket is set to non-blocking)
  /// Returns the number of bytes sent (0 if an error occurs, status will contain the error)
  size_t Send(Status& status, const byte* data, size_t dataLength, SocketFlags::Enum flags = SocketFlags::None);

  /// Sends data on the open socket to the specified remote address
  /// Will block if the send buffer is full (unless the socket is set to non-blocking)
  /// Returns the number of bytes sent (0 if an error occurs, status will contain the error)
  size_t SendTo(Status& status, const byte* data, size_t dataLength, const SocketAddress& to, SocketFlags::Enum flags = SocketFlags::None);

  /// Receives data on the connected socket from the connected remote address
  /// Will block if the receive buffer is empty (unless the socket is set to non-blocking)
  /// Returns the number of bytes received (0 if an error occurs, status will contain the error)
  size_t Receive(Status& status, byte* dataOut, size_t dataLength, SocketFlags::Enum flags = SocketFlags::None);

  /// Receives data on the open socket from any remote address
  /// Will block if the receive buffer is empty (unless the socket is set to non-blocking)
  /// Returns the number of bytes received (0 if an error occurs, status will contain the error)
  size_t ReceiveFrom(Status& status, byte* dataOut, size_t dataLength, SocketAddress& from, SocketFlags::Enum flags = SocketFlags::None);

  /// Returns true if the specified socket capability is ready for use, else false
  /// In a high efficiency situation, mechanisms other than select should be used
  bool Select(Status& status, SocketSelect::Enum selectMode, float timeoutSeconds) const;

  /// Gets the current value of the specified socket option
  template <typename Option, typename T>
  void GetSocketOption(Status& status, Option option, T& value) const
  {
    size_t valueLength = sizeof(value);
    GetSocketOption(status, option, &value, &valueLength);
  }
  template <typename Option>
  void GetSocketOption(Status& status, Option option, bool& value) const
  {
    size_t temp       = size_t(value);
    size_t tempLength = sizeof(temp);
    GetSocketOption(status, option, &temp, &tempLength);
    value = bool(temp);
  }
  void GetSocketOption(Status& status, SocketOption::Enum option, void* value, size_t* valueLength) const;
  void GetSocketOption(Status& status, SocketIpv4Option::Enum option, void* value, size_t* valueLength) const;
  void GetSocketOption(Status& status, SocketIpv6Option::Enum option, void* value, size_t* valueLength) const;
  void GetSocketOption(Status& status, SocketTcpOption::Enum option, void* value, size_t* valueLength) const;
  void GetSocketOption(Status& status, SocketUdpOption::Enum option, void* value, size_t* valueLength) const;

  /// Sets the current value of the specified socket option
  template <typename Option, typename T>
  void SetSocketOption(Status& status, Option option, T& value)
  {
    size_t valueLength = sizeof(value);
    SetSocketOption(status, option, &value, valueLength);
  }
  template <typename Option>
  void SetSocketOption(Status& status, Option option, bool& value)
  {
    size_t temp       = size_t(value);
    size_t tempLength = sizeof(temp);
    SetSocketOption(status, option, &temp, tempLength);
    value = bool(temp);
  }
  void SetSocketOption(Status& status, SocketOption::Enum option, const void* value, size_t valueLength);
  void SetSocketOption(Status& status, SocketIpv4Option::Enum option, const void* value, size_t valueLength);
  void SetSocketOption(Status& status, SocketIpv6Option::Enum option, const void* value, size_t valueLength);
  void SetSocketOption(Status& status, SocketTcpOption::Enum option, const void* value, size_t valueLength);
  void SetSocketOption(Status& status, SocketUdpOption::Enum option, const void* value, size_t valueLength);

  /// Native socket handle
  OsHandle                  mHandle;
  /// Socket address family
  SocketAddressFamily::Enum mAddressFamily;
  /// Socket protocol type
  SocketType::Enum          mType;
  /// Socket protocol
  SocketProtocol::Enum      mProtocol;
  /// Is this socket set in listening mode?
  bool                      mIsListening;
  /// Is this socket set in blocking mode?
  bool                      mIsBlocking;

private:
  /// No Copy Constructor
  Socket(const Socket&);
  /// No Copy Assignment Operator
  Socket& operator=(const Socket&);

  /// Friends
  template<typename type>
  friend struct MoveWithoutDestructionOperator;
};

/// Socket Move-Without-Destruction Operator
template<>
struct ZeroShared MoveWithoutDestructionOperator<Socket>
{
  static inline void MoveWithoutDestruction(Socket* dest, Socket* source)
  {
    // Construct new socket in place at destination
    new(dest) Socket(ZeroMove(*source));

    // We actually do need to destroy the source socket
    // in order to uninitialize the socket library as needed
    source->~Socket();
  }
};

/// Queries the socket library for the current local socket address associated with the specified socket
/// Returns the local address the socket is bound to, else SocketAddress()
/// (Named getsockname on most platforms)
ZeroShared SocketAddress QueryLocalSocketAddress(Status& status, const Socket& socket);

/// Queries the socket library for the current remote socket address associated with the specified socket
/// Returns the remote address currently associated with this socket, else SocketAddress()
/// For connectionless sockets (Datagram socket types), this will be the remote address specified in the last connect call
/// For connection-based sockets (Stream socket types), this will be the remote address specified in the last connect call after a connection has been successfully established
/// This does not imply that the connection to the remote address is currently valid, only that the most recent connect operation succeeded and hasn't been shutdown since
/// (Named getpeername on most platforms)
ZeroShared SocketAddress QueryRemoteSocketAddress(Status& status, const Socket& socket);

} // namespace Zero
