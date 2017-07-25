///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean, Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Utility/Typedefs.hpp"

namespace Zero
{

#define SOCKET_LIBRARY_SUPPORTS_IPV6

/// Maximum IPv4 numeric address host string length (including null terminator)
static const size_t Ipv4StringLength = 16;
/// Maximum IPv6 numeric address host string length (including null terminator)
static const size_t Ipv6StringLength = 46;

/// IPv4 address structure size
static const size_t Ipv4AddressBytes = 4;
/// IPv6 address structure size
static const size_t Ipv6AddressBytes = 16;

/// IPv4 socket address structure size
static const size_t Ipv4SocketAddressBytes    = 16;
/// IPv6 socket address structure size
static const size_t Ipv6SocketAddressBytes    = 28;
/// Socket address storage structure size
static const size_t SocketAddressStorageBytes = 128;

/// Maximum fully qualified domain name string length (including null terminator)
static const size_t MaxFullyQualifiedDomainNameStringLength = 1025;
/// Maximum host name string length (including null terminator)
static const size_t MaxHostNameStringLength                 = 255;
/// Maximum service name string length (including null terminator)
static const size_t MaxServiceNameStringLength              = 32;

/// Any available port
static const ushort AnyPort = 0;

/// IPv4 minimum packet header size
static const size_t Ipv4MinHeaderBytes = 20;
/// IPv4 maximum packet header size
static const size_t Ipv4MaxHeaderBytes = 60;
/// UDP datagram header size
static const size_t UdpHeaderBytes     = 8;

/// UDP maximum datagram length after header size
static const size_t DatagramMtuBytes = 65536 - UdpHeaderBytes;
/// Ethernet v2 MTU
static const size_t EthernetMtuBytes = 1500;
/// IPv4 minimum reassembly buffer size
static const size_t Ipv4MinMtuBytes  = 576;

// TODO: Add more IPv6 constants

} // namespace Zero
