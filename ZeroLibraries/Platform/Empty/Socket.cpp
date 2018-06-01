////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

u16 HostToNetworkShort(u16 hostShort)
{
  Error("Not implemented");
  return u16(0);
}

s16 HostToNetworkShort(s16 hostShort)
{
  Error("Not implemented");
  return s16(0);
}

u32 HostToNetworkLong(u32 hostLong)
{
  Error("Not implemented");
  return u32(0);
}

s32 HostToNetworkLong(s32 hostLong)
{
  Error("Not implemented");
  return s32(0);
}

u16 NetworkToHostShort(u16 networkShort)
{
  Error("Not implemented");
  return u16(0);
}

s16 NetworkToHostShort(s16 networkShort)
{
  Error("Not implemented");
  return s16(0);
}

u32 NetworkToHostLong(u32 networkLong)
{
  Error("Not implemented");
  return u32(0);
}

s32 NetworkToHostLong(s32 networkLong)
{
  Error("Not implemented");
  return s32(0);
}

//---------------------------------------------------------------------------------//
//                                SocketAddress                                    //
//---------------------------------------------------------------------------------//
SocketAddress::SocketAddress()
{
  Error("Not implemented");
}

SocketAddress::SocketAddress(const SocketAddress& rhs)
{
  Error("Not implemented");
}

SocketAddress& SocketAddress::operator =(const SocketAddress& rhs)
{
  Error("Not implemented");
  return *this;
}

bool SocketAddress::operator ==(const SocketAddress& rhs) const
{
  Error("Not implemented");
  return false;
}
bool SocketAddress::operator !=(const SocketAddress& rhs) const
{
  Error("Not implemented");
  return false;
}
bool SocketAddress::operator  <(const SocketAddress& rhs) const
{
  Error("Not implemented");
  return false;
}

SocketAddress::operator bool(void) const
{
  Error("Not implemented");
  return false;
}

bool SocketAddress::IsEmpty() const
{
  Error("Not implemented");
  return false;
}

SocketAddressFamily::Enum SocketAddress::GetAddressFamily() const
{
  Error("Not implemented");
  return SocketAddressFamily::Unspecified;
}

void SocketAddress::SetIpv4(Status& status, StringParam host, uint port)
{
  Error("Not implemented");
}

void SocketAddress::SetIpv4(Status& status, StringParam host, uint port, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
}

void SocketAddress::SetIpv6(Status& status, StringParam host, uint port)
{
  Error("Not implemented");
}
void SocketAddress::SetIpv6(Status& status, StringParam host, uint port, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
}

void SocketAddress::SetIpPort(Status& status, uint port)
{
  Error("Not implemented");
}

uint SocketAddress::GetIpPort(Status& status) const
{
  Error("Not implemented");
  return 0;
}

void SocketAddress::Clear()
{
  Error("Not implemented");
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, SocketAddress& socketAddress)
{
  Error("Not implemented");
  return 0;
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                   SocketProtocol::Enum protocol, SocketType::Enum type, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                   SocketProtocol::Enum protocol, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                   SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service)
{
  Error("Not implemented");
  return SocketAddress();
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                               SocketProtocol::Enum protocol, SocketType::Enum type, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                               SocketProtocol::Enum protocol, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                               SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Error("Not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service)
{
  Error("Not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address, SocketNameResolutionFlags::Enum nameResolutionFlags)
{
  Error("Not implemented");
  return Pair<String, String>();
}

Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address)
{
  Error("Not implemented");
  return Pair<String, String>();
}

String SocketAddressToString(Status& status, SocketAddressFamily::Enum addressFamily, const SocketAddress& address)
{
  Error("Not implemented");
  return String();
}

String SocketAddressToString(SocketAddressFamily::Enum addressFamily, const SocketAddress& address)
{
  Error("Not implemented");
  return String();
}

SocketAddress StringToSocketAddress(Status& status, SocketAddressFamily::Enum addressFamily, StringParam address)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress StringToSocketAddress(SocketAddressFamily::Enum addressFamily, StringParam address)
{
  Error("Not implemented");
  return SocketAddress();
}

bool IsValidIpv4Address(const SocketAddress& address)
{
  Error("Not implemented");
  return false;
}

bool IsValidIpv6Address(const SocketAddress& address)
{
  Error("Not implemented");
  return false;
}

bool IsValidIpv4Address(StringParam address)
{
  Error("Not implemented");
  return false;
}

bool IsValidIpv6Address(StringParam address)
{
  Error("Not implemented");
  return false;
}

String Ipv4AddressToString(const SocketAddress& address)
{
  Error("Not implemented");
  return String();
}

String Ipv6AddressToString(const SocketAddress& address)
{
  Error("Not implemented");
  return String();
}

String PortToString(uint port)
{
  Error("Not implemented");
  return String();
}

String Ipv4AddressToStringWithPort(const SocketAddress& address)
{
  Error("Not implemented");
  return String();
}

String Ipv6AddressToStringWithPort(const SocketAddress& address)
{
  Error("Not implemented");
  return String();
}

SocketAddress StringToIpv4Address(StringParam address)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress StringToIpv4Address(StringParam address, ushort port)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress StringToIpv6Address(StringParam address)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress StringToIpv6Address(StringParam address, ushort port)
{
  Error("Not implemented");
  return SocketAddress();
}

//---------------------------------------------------------------------------------//
//                                    Socket                                       //
//---------------------------------------------------------------------------------//

//
// Static Member Functions
//

size_t Socket::GetMaxListenBacklog()
{
  Error("Not implemented");
  return 0;
}

bool Socket::IsCommonReceiveError(int extendedErrorCode)
{
  Error("Not implemented");
  return false;
}

bool Socket::IsCommonAcceptError(int extendedErrorCode)
{
  Error("Not implemented");
  return false;
}

bool Socket::IsCommonConnectError(int extendedErrorCode)
{
  Error("Not implemented");
  return false;
}

bool Socket::IsSocketLibraryInitialized()
{
  Error("Not implemented");
  return false;
}

void Socket::InitializeSocketLibrary(Status& status)
{
  Error("Not implemented");
}

void Socket::UninitializeSocketLibrary(Status& status)
{
  Error("Not implemented");
}

//
// Non-Static Member Functions
//

Socket::Socket()
{
  Error("Not implemented");
}

Socket::~Socket()
{
  Error("Not implemented");
}

Socket::Socket(MoveReference<Socket> rhs)
{
  Error("Not implemented");
}

Socket& Socket::operator =(MoveReference<Socket> rhs)
{
  Error("Not implemented");
  return *this;
}

bool Socket::IsOpen() const
{
  Error("Not implemented");
  return false;
}

SocketAddressFamily::Enum Socket::GetAddressFamily() const
{
  Error("Not implemented");
  return SocketAddressFamily::Unspecified;
}

SocketType::Enum Socket::GetType() const
{
  Error("Not implemented");
  return SocketType::Unspecified;
}

SocketProtocol::Enum Socket::GetProtocol() const
{
  Error("Not implemented");
  return SocketProtocol::Unspecified;
}

bool Socket::IsBound() const
{
  Error("Not implemented");
  return false;
}

SocketAddress Socket::GetBoundLocalAddress() const
{
  Error("Not implemented");
  return SocketAddress();
}

bool Socket::IsListening() const
{
  Error("Not implemented");
  return false;
}

bool Socket::IsBlocking() const
{
  Error("Not implemented");
  return false;
}

bool Socket::HasConnectedRemoteAddress() const
{
  Error("Not implemented");
  return false;
}

SocketAddress Socket::GetConnectedRemoteAddress() const
{
  Error("Not implemented");
  return SocketAddress();
}

void Socket::Open(Status& status, SocketAddressFamily::Enum addressFamily, SocketType::Enum type, SocketProtocol::Enum protocol)
{
  Error("Not implemented");
}

void Socket::Bind(Status& status, const SocketAddress& localAddress)
{
  Error("Not implemented");
}

void Socket::Listen(Status& status, uint backlog)
{
  Error("Not implemented");
}

void Socket::SetBlocking(Status& status, bool blocking)
{
  Error("Not implemented");
}

void Socket::Accept(Status& status, Socket* connectionOut)
{
  Error("Not implemented");
}

void Socket::Connect(Status& status, const SocketAddress& remoteAddress)
{
  Error("Not implemented");
}

void Socket::Shutdown(Status& status, SocketIo::Enum io)
{
  Error("Not implemented");
}

void Socket::Close()
{
  Error("Not implemented");
}

void Socket::Close(Status& status)
{
  Error("Not implemented");
}

size_t Socket::Send(Status& status, const byte* data, size_t dataLength, SocketFlags::Enum flags)
{
  Error("Not implemented");
  return 0;
}

size_t Socket::SendTo(Status& status, const byte* data, size_t dataLength, const SocketAddress& to, SocketFlags::Enum flags)
{
  Error("Not implemented");
  return 0;
}

size_t Socket::Receive(Status& status, byte* dataOut, size_t dataLength, SocketFlags::Enum flags)
{
  Error("Not implemented");
  return 0;
}

size_t Socket::ReceiveFrom(Status& status, byte* dataOut, size_t dataLength, SocketAddress& from, SocketFlags::Enum flags)
{
  Error("Not implemented");
  return 0;
}

bool Socket::Select(Status& status, SocketSelect::Enum selectMode, float timeoutSeconds) const
{
  Error("Not implemented");
  return false;
}

void Socket::GetSocketOption(Status& status, SocketOption::Enum option, void* value, size_t* valueLength) const
{
  Error("Not implemented");
}

void Socket::GetSocketOption(Status& status, SocketIpv4Option::Enum option, void* value, size_t* valueLength) const
{
  Error("Not implemented");
}

void Socket::GetSocketOption(Status& status, SocketIpv6Option::Enum option, void* value, size_t* valueLength) const
{
  Error("Not implemented");
}

void Socket::GetSocketOption(Status& status, SocketTcpOption::Enum option, void* value, size_t* valueLength) const
{
  Error("Not implemented");
}

void Socket::GetSocketOption(Status& status, SocketUdpOption::Enum option, void* value, size_t* valueLength) const
{
  Error("Not implemented");
}

void Socket::SetSocketOption(Status& status, SocketOption::Enum option, const void* value, size_t valueLength)
{
  Error("Not implemented");
}

void Socket::SetSocketOption(Status& status, SocketIpv4Option::Enum option, const void* value, size_t valueLength)
{
  Error("Not implemented");
}

void Socket::SetSocketOption(Status& status, SocketIpv6Option::Enum option, const void* value, size_t valueLength)
{
  Error("Not implemented");
}

void Socket::SetSocketOption(Status& status, SocketTcpOption::Enum option, const void* value, size_t valueLength)
{
  Error("Not implemented");
}

void Socket::SetSocketOption(Status& status, SocketUdpOption::Enum option, const void* value, size_t valueLength)
{
  Error("Not implemented");
}

SocketAddress QueryLocalSocketAddress(Status& status, const Socket& socket)
{
  Error("Not implemented");
  return SocketAddress();
}

SocketAddress QueryRemoteSocketAddress(Status& status, const Socket& socket)
{
  Error("Not implemented");
  return SocketAddress();
}

} // namespace Zero
