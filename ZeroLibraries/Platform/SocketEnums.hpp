///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean, Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Utility/EnumDeclaration.hpp"

namespace Zero
{

/// Socket address type designated by protocol
namespace SocketAddressFamily
{
  enum Enum
  {
    Unspecified      = 0,   /// Unspecified SocketAddressFamily
    Unix             = 1,   /// UNIX Interprocess Communication Protocols (Pipes)
    InternetworkV4   = 2,   /// Internetwork Version 4 (IPv4) Protocols
    ImpLink          = 3,   /// ARPANET IMP Addresses
    Pup              = 4,   /// PUP Protocols
    Chaos            = 5,   /// MIT CHAOS Protocols
    Ns               = 6,   /// IPX (XEROX NS) Protocols
    Ipx              = Ns,  /// IPX (XEROX NS) Protocols
    Iso              = 7,   /// OSI (ISO) Protocols
    Osi              = Iso, /// OSI (ISO) Protocols
    Ecma             = 8,   /// European Computer Manufacturers
    DataKit          = 9,   /// DataKit Protocols
    Ccitt            = 10,  /// CCITT Protocols
    Sna              = 11,  /// IBM SNA Protocol
    DecNet           = 12,  /// DECnet Protocol
    DataLink         = 13,  /// DataLink Interface
    Lat              = 14,  /// LAT Protocol
    Hyperchannel     = 15,  /// NSC Hyperchannel
    AppleTalk        = 16,  /// AppleTalk Protocols
    NetBios          = 17,  /// NetBios Addresses
    VoiceView        = 18,  /// VoiceView Protocols
    Firefox          = 19,  /// Firefox Protocols
    Unknown1         = 20,  /// Unknown Protocol
    Banyan           = 21,  /// Banyan Protocols
    Atm              = 22,  /// Native ATM Services
    InternetworkV6   = 23,  /// Internetwork Version 6 (IPv6) Protocols
    Cluster          = 24,  /// Microsoft Wolfpack Protocol
    Ieee12844        = 25,  /// IEEE 1284.4 WG Protocol
    Irda             = 26,  /// IrDA Protocols
    NetworkDesigners = 28   /// Network Designers OSI and Gateway Protocols
  };
  typedef uint Type;
}

/// Socket address resolution behavior flags
namespace SocketAddressResolutionFlags
{
  enum Enum
  {
    None                      = 0,          /// No SocketAddressResolutionFlags
    AnyAddress                = 0x00000001, /// Given an empty host, the host will resolve to the 'any' address. Sockets bound to the 'any' address may be used to accept connections on any of the host's socket addresses
    RequestCannonName         = 0x00000002, /// Request the canonical name in the first ai_canonname member
    NumericHost               = 0x00000004, /// Host name is numeric address string
    NumericService            = 0x00000008, /// Service name is a port number string
    RequestIpv6and4           = 0x00000100, /// Request both IPv6 and IPv4 addresses with RequestIpv4Mapped
    ResolveIfGlobalAddress    = 0x00000400, /// Resolves only if a global address is configured
    RequestIpv4Mapped         = 0x00000800, /// On IPv6 request failure, request an IPv4-mapped IPv6 address
    ResolveNonAuthoritative   = 0x00004000, /// Allow address resolution from a non-authoritative namespace provider
    ResolveSecure             = 0x00008000, /// Enable address resolution from a secure channel
    RequestPreferredNames     = 0x00010000, /// Request address resolution for a preferred user name
    RequestQualifiedName      = 0x00020000, /// Request the fully qualified domain name in the first ai_canonname member, must not be set with RequestCannonName
    FileServerHint            = 0x00040000, /// Hint that the host being queried is a file server
    DisableInternationalNames = 0x00080000  /// Disable internationalized domain name handling
  };
  typedef uint Type;
}

/// Socket name resolution behavior flags
namespace SocketNameResolutionFlags
{
  enum Enum
  {
    None                       = 0,    /// No SocketNameResolutionFlags
    NoFullyQualifiedDomainName = 0x01, /// Local hosts have their relative distinguished name (RDN) returned instead of their FQDN
    NumericHost                = 0x02, /// Return the numeric address string of a host instead of it's host name
    ErrorIfHostNotInDNS        = 0x04, /// Results in an error if the host is not resolved by DNS
    NumericService             = 0x08, /// Return the port number string of a service instead of it's service name
    DatagramService            = 0x10  /// Indicates the specified service is a datagram service (some services use different port numbers for UDP vs TCP)
  };
  typedef uint Type;
}

/// Socket protocol
namespace SocketProtocol
{
  enum Enum
  {
    Ip                                = 0,   /// Internet Protocol
    Ipv6HopOptions                    = Ip,  /// Internet Protocol Version 6 (IPv6) Hop-By-Hop Options
    Unspecified                       = Ip,  /// Unspecified Protocol
    Icmp                              = 1,   /// Internet Control Message Protocol
    Igmp                              = 2,   /// Internet Group Management Protocol
    Ggp                               = 3,   /// Gateway-to-Gateway Protocol
    Ipv4                              = 4,   /// Internet Protocol Version 4 (IPv4)
    Stream                            = 5,   /// Stream Protocol
    Tcp                               = 6,   /// Transmission Control Protocol
    Cbt                               = 7,   /// Core Based Trees Protocol
    Egp                               = 8,   /// Exterior Gateway Protocol
    Igp                               = 9,   /// Private Interior Gateway Protocol
    Pup                               = 12,  /// PARC Universal Packet Protocol
    Udp                               = 17,  /// User Datagram Protocol
    Idp                               = 22,  /// Internet Datagram Protocol
    Rdp                               = 27,  /// Reliable Data Protocol
    Ipv6                              = 41,  /// Internet Protocol Version 6 (IPv6)
    Ipv6RoutingHeader                 = 43,  /// Internet Protocol Version 6 (IPv6) Routing Header
    Ipv6FragmentHeader                = 44,  /// Internet Protocol Version 6 (IPv6) Fragment Header
    IpSecEncapsulatingSecurityPayload = 50,  /// Internet Protocol Security Encapsulating Security Payload
    IpSecAuthenticationHeader         = 51,  /// Internet Protocol Security Authentication Header
    IcmpV6                            = 58,  /// Internet Control Message Protocol Version 6 (ICMPv6)
    Ipv6NoNextHeader                  = 59,  /// Internet Protocol Version 6 (IPv6) No Next Header
    Ipv6DestinationOptions            = 60,  /// Internet Protocol Version 6 (IPv6) Destination Options
    Nd                                = 77,  /// Net Disk Protocol
    Iclfxbm                           = 78,  /// Wideband monitoring
    Pim                               = 103, /// Protocol Independent Multicast
    Pgm                               = 113, /// Pragmatic General Multicast
    L2tp                              = 115, /// Level 2 Tunneling Protocol
    Sctp                              = 132, /// Stream Control Transmission Protocol
    Raw                               = 255  /// Raw Internet Protocol
  };
  typedef uint Type;
}

/// Socket protocol type
DeclareEnum6(SocketType,
  Unspecified,      /// Unspecified SocketType
  Stream,           /// Reliable Connection-Based Streams (such as TCP)
  Datagram,         /// Unreliable Connectionless Datagrams (such as UDP)
  RawDatagram,      /// Raw Unreliable Connectionless Datagrams
  ReliableDatagram, /// Reliable Connectionless Datagrams
  StreamPacket);    /// Reliable Connection-Based Stream Packets

/// Socket operation set
DeclareEnum3(SocketIo,
  Read,  /// Read (Receive) Operation
  Write, /// Write (Send) Operation
  Both); /// Read and Write Operations

/// Socket capability set
DeclareEnum3(SocketSelect,
  Read,   /// Check Socket Readability
  Write,  /// Check Socket Writability
  Error); /// Check Socket Errors

/// Socket operation behavior flags
namespace SocketFlags
{
  enum Enum
  {
    None                 = 0,         /// No SocketFlags
    OutOfBand            = (1 << 0),  /// Send/Receive Out-Of-Band Data
    Peek                 = (1 << 1),  /// Peek Incoming Data
    DontRoute            = (1 << 2),  /// Send Without Routing
    WaitAll              = (1 << 3),  /// Wait Until Packet Is Completely Full
    Interrupt            = (1 << 4),  /// Send/Receive Inside Interrupt Context
    Truncated            = (1 << 8),  /// Packet Data Was/May Be Truncated
    ControlDataTruncated = (1 << 9),  /// Control Data Was/May Be Truncated
    Broadcast            = (1 << 10), /// Broadcast Data
    Multicast            = (1 << 11), /// Multicast Data
    Partial              = (1 << 15)  /// Partially Send/Receive Data
  };
  typedef uint Type;
}

/// Socket behavior option
/// Applies to all sockets unless otherwise specified
namespace SocketOption
{
  enum Enum
  {
    DebugOutput         = 0x0001,        /// Enable debug output? (Get/Set : bool)
    IsListening         = 0x0002,        /// Socket is in listening mode? Valid for connection-based protocols (Get : bool)
    ReuseAddress        = 0x0004,        /// Allow binding to an address and port already in use? (Get/Set : bool)
    KeepAlive           = 0x0008,        /// Socket connections should use keep-alive? Valid for connection-based protocols (Get/Set : bool)
    DontRoute           = 0x0010,        /// Send without routing? Valid for message-oriented protocols (Get/Set : bool)
    CanBroadcast        = 0x0020,        /// Socket is configured to broadcast? Valid for protocols that support broadcast (Get/Set : bool)
    SendLoopback        = 0x0040,        /// Send data using the loopback adapter? (Get/Set : bool)
    Linger              = 0x0080,        /// Socket should remain open for a set duration after being closed? Valid for connection-based protocols (Get/Set : linger)
    OutOfBandInline     = 0x0100,        /// Return Out-Of-Band data inline with regular data? (Get/Set : bool)
    DontLinger          = ~Linger,       /// Socket should remain open for a set duration after being closed? Valid for connection-based protocols (Get/Set : bool)
    ExclusiveAddress    = ~ReuseAddress, /// Socket has exclusive use of the address and port it's bound to? Must be set before calling bind (Get/Set : bool)
    SendBufferSize      = 0x1001,        /// Socket buffer size reserved for sending data (Get/Set : uint)
    ReceiveBufferSize   = 0x1002,        /// Socket buffer size reserved for receiving data (Get/Set : uint)
    SendLowWatermark    = 0x1003,        /// Minimum number of bytes to process for send operations (Get/Set : uint)
    ReceiveLowWatermark = 0x1004,        /// Minimum number of bytes to process for receive operations (Get/Set : uint)
    SendTimeout         = 0x1005,        /// Blocking send call timeout in milliseconds, zero indicates no timeout (Get/Set : uint)
    ReceiveTimeout      = 0x1006,        /// Blocking receive call timeout in milliseconds, zero indicates no timeout (Get/Set : uint)
    ErrorCode           = 0x1007,        /// The last error code set on the socket (Get : uint)
    SocketType          = 0x1008,        /// Socket type (Get : uint)
    SocketState         = 0x1009,        /// Current socket state (Get : CSADDR_INFO)
    MaxMessageSize      = 0x2003,        /// Max outgoing message size in bytes. Valid for message-oriented protocols (Get : uint)
    ProviderConfig      = 0x3001,        /// Socket service provider configuration (Get/Set : char*)
    ConditionalAccept   = 0x3002,        /// Accept/reject incoming connections by the application? Valid for connection-based protocols (Get/Set : bool)
    PauseAccept         = 0x3003,        /// Pause accepting connections. Valid for connection-based protocols (Get/Set : bool)
    CompartmentId       = 0x3004,        /// Socket compartment (Get/Set : uint)
    RandomizePort       = 0x3005,        /// Randomize wildcard port assignment (Get/Set : bool)
    PortScalability     = 0x3006         /// Maximize local port scalability by allocating wildcard ports multiple times for different local address port pairs? (Get/Set : bool)
  };
  typedef uint Type;
}

/// IPv4 Socket behavior option
/// Applies to all IPv4 sockets unless otherwise specified
namespace SocketIpv4Option
{
  enum Enum
  {
    Options                                 = 1,  /// All IPv4 options (Get/Set : char*)
    IncludeHeader                           = 2,  /// Application provides the IP header? Valid only for raw sockets (Get/Set : bool)
    TypeOfService                           = 3,  /// Type of service setting (Get/Set : uint)
    TimeToLive                              = 4,  /// Time To Live (TTL, aka hop limit) of packets (Get/Set : uint)
    MulticastInterface                      = 9,  /// Multicast traffic interface (Get/Set : uint)
    MulticastTimeToLive                     = 10, /// Time To Live (TTL, aka hop limit) of multicast packets (Get/Set : uint)
    MulticastLoopback                       = 11, /// Allow local, outgoing multicast data to be received using the loopback adapter? (Get/Set : bool)
    AddMulticastGroupMembership             = 12, /// Add membership to the specified multicast group (Set : ip_mreq)
    RemoveMulticastGroupMembership          = 13, /// Remove membership from the specified multicast group (Set : ip_mreq)
    DontFragment                            = 14, /// Don't fragment messages. Valid for message-oriented protocols (Get/Set : bool)
    AddMulticastGroupAndSourceMembership    = 15, /// Add membership to the specified multicast group and accept data from the supplied multicast source address (Set : ip_mreq_source)
    RemoveMulticastGroupAndSourceMembership = 16, /// Remove membership from the specified multicast group and ignore data from the supplied multicast source address (Set : ip_mreq_source)
    RemoveMulticastSourceMembership         = 17, /// Ignore data from the supplied multicast source address (Set : ip_mreq_source)
    AddMulticastSourceMembership            = 18, /// Accept data from the supplied multicast source address (Set : ip_mreq_source)
    ReturnPacketInfo                        = 19, /// Return the packet information when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReturnTimeToLive                        = 21, /// Return the Time To Live (TTL, aka hop limit) when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReceiveBroadcast                        = 22, /// Allow broadcast reception? (Get/Set : bool)
    ReturnArrivalInterface                  = 24, /// Return the arrival interface when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReturnDestinationAddress                = 25, /// Return the destination address when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    EnableInterfaceList                     = 28, /// Enable an interface list
    AddInterfaceListEntry                   = 29, /// Add an interface list entry
    RemoveInterfaceListEntry                = 30, /// Remove an interface list entry
    UnicastInterface                        = 31, /// Unicast traffic interface (Get/Set : uint)
    Ipv6RoutingHeader                       = 32, /// IPv6 routing header
    ReturnRoutingHeader                     = 38, /// Return the routing header when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    TrafficClass                            = 39, /// Packet traffic class
    ReturnTrafficClass                      = 40, /// Return the packet traffic class when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReturnOriginalArrivalInterface          = 47  /// Return the original arrival interface when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
  };
  typedef uint Type;
}

/// IPv6 Socket behavior option
/// Applies to all IPv6 sockets unless otherwise specified
namespace SocketIpv6Option
{
  enum Enum
  {
    HopOptions                     = 1,  /// All IPv6 hop-by-hop options (Get/Set : char*)
    IncludeHeader                  = 2,  /// Application provides the IP header? Valid only for datagram and raw sockets (Get/Set : bool)
    UnicastTimeToLive              = 4,  /// Time To Live (TTL, aka hop limit) of unicast packets (Get/Set : uint)
    MulticastInterface             = 9,  /// Multicast traffic interface (Get/Set : uint)
    MulticastTimeToLive            = 10, /// Time To Live (TTL, aka hop limit) of multicast packets (Get/Set : uint)
    MulticastLoopback              = 11, /// Allow local, outgoing multicast data to be received using the loopback adapter? (Get/Set : bool)
    AddMulticastGroupMembership    = 12, /// Add membership to the specified multicast group (Set : ipv6_mreq)
    RemoveMulticastGroupMembership = 13, /// Remove membership from the specified multicast group (Set : ipv6_mreq)
    DontFragment                   = 14, /// Don't fragment messages. Valid for message-oriented protocols (Get/Set : bool)
    ReturnPacketInfo               = 19, /// Return the packet information when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReturnTimeToLive               = 21, /// Return the Time To Live (TTL, aka hop limit) when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ProtectionLevel                = 23, /// Restrict the scope of a listening socket (Get/Set : int)
    ReturnArrivalInterface         = 24, /// Return the arrival interface when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReturnDestinationAddress       = 25, /// Return the destination address when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ChecksumOffset                 = 26, /// Offset to checksum for outgoing packets. Valid only for raw sockets
    Ipv6Only                       = 27, /// Restrict communication to IPv6 only? (Sockets bound to IPv6 addresses have the capability to communicate with IPv4 mapped addresses) (Get/Set : bool)
    EnableInterfaceList            = 28, /// Enable an interface list
    AddInterfaceListEntry          = 29, /// Add an interface list entry
    RemoveInterfaceListEntry       = 30, /// Remove an interface list entry
    UnicastInterface               = 31, /// Unicast traffic interface (Get/Set : uint)
    Ipv6RoutingHeader              = 32, /// IPv6 routing header
    ReturnRoutingHeader            = 38, /// Return the routing header when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    TrafficClass                   = 39, /// Packet traffic class
    ReturnTrafficClass             = 40, /// Return the packet traffic class when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
    ReturnOriginalArrivalInterface = 47  /// Return the original arrival interface when a packet is received, in the WSAMSG structure returned by WSARecvMsg? Valid only for datagram or raw sockets (Get/Set : bool)
  };
  typedef uint Type;
}

/// TCP Socket behavior option
/// Applies to all TCP sockets unless otherwise specified
namespace SocketTcpOption
{
  enum Enum
  {
    NoDelay                       = 1, /// Disable Nagle's algorithm (send packets without waiting for data to coalesce)? (Get/Set : bool)
    ExpeditedImplemented          = 2, /// Service provider implemented expedited data as specified in RFC 1122? (Get/Set : bool)
    IdleDurationBeforeKeepAlive   = 3, /// Idle duration, in seconds, before keep alive begins (Get/Set : uint)
    MaxSegmentSize                = 4, /// Max outgoing segment size in bytes (Get/Set : uint)
    RetransmitDurationBeforeClose = 5, /// Retransmit duration, in seconds, before the connection is closed (Get/Set : uint)
    UrgentImplemented             = 6  /// Service provider implemented urgent data as specified in RFC 1122? (Get/Set : bool)
  };
  typedef uint Type;
}

/// UDP Socket behavior option
/// Applies to all UDP sockets unless otherwise specified
namespace SocketUdpOption
{
  enum Enum
  {
    NoChecksum       = 1, /// Send datagrams with a zero checksum? (Get/Set : bool)
    ChecksumCoverage = 20 /// Send datagrams with a checksum? (Get/Set : bool)
  };
  typedef uint Type;
}

/// Internet protocol version
DeclareEnum4(InternetProtocol,
  Unspecified, /// Unspecified internet protocol version
  V4,          /// Internetwork protocol version 4
  V6,          /// Internetwork protocol version 6
  Both);       /// Internetwork protocol version 4 and 6

/// Internet protocol version range
static const InternetProtocol::Enum InternetProtocolMin = InternetProtocol::Unspecified;
static const InternetProtocol::Enum InternetProtocolMax = InternetProtocol::Both;

} // namespace Zero
