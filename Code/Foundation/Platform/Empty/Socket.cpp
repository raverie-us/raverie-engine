// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

u16 HostToNetworkShort(u16 hostShort)
{
  return hostShort;
}

s16 HostToNetworkShort(s16 hostShort)
{
  return hostShort;
}

u32 HostToNetworkLong(u32 hostLong)
{
  return hostLong;
}

s32 HostToNetworkLong(s32 hostLong)
{
  return hostLong;
}

u16 NetworkToHostShort(u16 networkShort)
{
  return networkShort;
}

s16 NetworkToHostShort(s16 networkShort)
{
  return networkShort;
}

u32 NetworkToHostLong(u32 networkLong)
{
  return networkLong;
}

s32 NetworkToHostLong(s32 networkLong)
{
  return networkLong;
}

//                                SocketAddress //
SocketAddress::SocketAddress()
{
}

SocketAddress::SocketAddress(const SocketAddress& rhs)
{
}

SocketAddress& SocketAddress::operator=(const SocketAddress& rhs)
{
  return *this;
}

bool SocketAddress::operator==(const SocketAddress& rhs) const
{
  return false;
}
bool SocketAddress::operator!=(const SocketAddress& rhs) const
{
  return false;
}
bool SocketAddress::operator<(const SocketAddress& rhs) const
{
  return false;
}

SocketAddress::operator bool(void) const
{
  return false;
}

bool SocketAddress::IsEmpty() const
{
  return true;
}

SocketAddressFamily::Enum SocketAddress::GetAddressFamily() const
{
  return SocketAddressFamily::Unspecified;
}

void SocketAddress::SetIpv4(Status& status, StringParam host, uint port)
{
  status.SetFailed("Socket not implemented");
}

void SocketAddress::SetIpv4(Status& status,
                            StringParam host,
                            uint port,
                            SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
}

void SocketAddress::SetIpv6(Status& status, StringParam host, uint port)
{
  status.SetFailed("Socket not implemented");
}

void SocketAddress::SetIpv6(Status& status,
                            StringParam host,
                            uint port,
                            SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
}

void SocketAddress::SetIpPort(Status& status, uint port)
{
  status.SetFailed("Socket not implemented");
}

uint SocketAddress::GetIpPort(Status& status) const
{
  status.SetFailed("Socket not implemented");
  return 0;
}

void SocketAddress::Clear()
{
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, SocketAddress& socketAddress)
{
  return 0;
}

SocketAddress ResolveSocketAddress(Status& status,
                                   StringParam host,
                                   StringParam service,
                                   SocketAddressFamily::Enum addressFamily,
                                   SocketProtocol::Enum protocol,
                                   SocketType::Enum type,
                                   SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status,
                                   StringParam host,
                                   StringParam service,
                                   SocketAddressFamily::Enum addressFamily,
                                   SocketProtocol::Enum protocol,
                                   SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status,
                                   StringParam host,
                                   StringParam service,
                                   SocketAddressFamily::Enum addressFamily,
                                   SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status,
                                   StringParam host,
                                   StringParam service,
                                   SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status,
                                               StringParam host,
                                               StringParam service,
                                               SocketAddressFamily::Enum addressFamily,
                                               SocketProtocol::Enum protocol,
                                               SocketType::Enum type,
                                               SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status,
                                               StringParam host,
                                               StringParam service,
                                               SocketAddressFamily::Enum addressFamily,
                                               SocketProtocol::Enum protocol,
                                               SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status,
                                               StringParam host,
                                               StringParam service,
                                               SocketAddressFamily::Enum addressFamily,
                                               SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status,
                                               StringParam host,
                                               StringParam service,
                                               SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service)
{
  status.SetFailed("Socket not implemented");
  Array<SocketAddress> addresses;
  return addresses;
}

Pair<String, String> ResolveHostAndServiceNames(Status& status,
                                                const SocketAddress& address,
                                                SocketNameResolutionFlags::Enum nameResolutionFlags)
{
  status.SetFailed("Socket not implemented");
  return Pair<String, String>();
}

Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address)
{
  status.SetFailed("Socket not implemented");
  return Pair<String, String>();
}

String SocketAddressToString(Status& status, SocketAddressFamily::Enum addressFamily, const SocketAddress& address)
{
  status.SetFailed("Socket not implemented");
  return String();
}

String SocketAddressToString(SocketAddressFamily::Enum addressFamily, const SocketAddress& address)
{
  return String();
}

SocketAddress StringToSocketAddress(Status& status, SocketAddressFamily::Enum addressFamily, StringParam address)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

SocketAddress StringToSocketAddress(SocketAddressFamily::Enum addressFamily, StringParam address)
{
  return SocketAddress();
}

bool IsValidIpv4Address(const SocketAddress& address)
{
  return false;
}

bool IsValidIpv6Address(const SocketAddress& address)
{
  return false;
}

bool IsValidIpv4Address(StringParam address)
{
  return false;
}

bool IsValidIpv6Address(StringParam address)
{
  return false;
}

String Ipv4AddressToString(const SocketAddress& address)
{
  return String();
}

String Ipv6AddressToString(const SocketAddress& address)
{
  return String();
}

String PortToString(uint port)
{
  return ToString(port);
}

String Ipv4AddressToStringWithPort(const SocketAddress& address)
{
  return String();
}

String Ipv6AddressToStringWithPort(const SocketAddress& address)
{
  return String();
}

SocketAddress StringToIpv4Address(StringParam address)
{
  return SocketAddress();
}

SocketAddress StringToIpv4Address(StringParam address, ushort port)
{
  return SocketAddress();
}

SocketAddress StringToIpv6Address(StringParam address)
{
  return SocketAddress();
}

SocketAddress StringToIpv6Address(StringParam address, ushort port)
{
  return SocketAddress();
}

//                                    Socket //

//
// Static Member Functions
//

size_t Socket::GetMaxListenBacklog()
{
  return 0;
}

bool Socket::IsCommonReceiveError(int extendedErrorCode)
{
  return false;
}

bool Socket::IsCommonAcceptError(int extendedErrorCode)
{
  return false;
}

bool Socket::IsCommonConnectError(int extendedErrorCode)
{
  return false;
}

bool Socket::IsSocketLibraryInitialized()
{
  return false;
}

void Socket::InitializeSocketLibrary(Status& status)
{
  status.SetFailed("Socket not implemented");
}

void Socket::UninitializeSocketLibrary(Status& status)
{
  status.SetFailed("Socket not implemented");
}

//
// Non-Static Member Functions
//

Socket::Socket()
{
}

Socket::~Socket()
{
}

Socket::Socket(MoveReference<Socket> rhs)
{
}

Socket& Socket::operator=(MoveReference<Socket> rhs)
{
  return *this;
}

bool Socket::IsOpen() const
{
  return false;
}

SocketAddressFamily::Enum Socket::GetAddressFamily() const
{
  return SocketAddressFamily::Unspecified;
}

SocketType::Enum Socket::GetType() const
{
  return SocketType::Unspecified;
}

SocketProtocol::Enum Socket::GetProtocol() const
{
  return SocketProtocol::Unspecified;
}

bool Socket::IsBound() const
{
  return false;
}

SocketAddress Socket::GetBoundLocalAddress() const
{
  return SocketAddress();
}

bool Socket::IsListening() const
{
  return false;
}

bool Socket::IsBlocking() const
{
  return false;
}

bool Socket::HasConnectedRemoteAddress() const
{
  return false;
}

SocketAddress Socket::GetConnectedRemoteAddress() const
{
  return SocketAddress();
}

void Socket::Open(Status& status,
                  SocketAddressFamily::Enum addressFamily,
                  SocketType::Enum type,
                  SocketProtocol::Enum protocol)
{
  status.SetFailed("Socket not implemented");
}

void Socket::Bind(Status& status, const SocketAddress& localAddress)
{
  status.SetFailed("Socket not implemented");
}

void Socket::Listen(Status& status, uint backlog)
{
  status.SetFailed("Socket not implemented");
}

void Socket::SetBlocking(Status& status, bool blocking)
{
  status.SetFailed("Socket not implemented");
}

void Socket::Accept(Status& status, Socket* connectionOut)
{
  status.SetFailed("Socket not implemented");
}

void Socket::Connect(Status& status, const SocketAddress& remoteAddress)
{
  status.SetFailed("Socket not implemented");
}

void Socket::Shutdown(Status& status, SocketIo::Enum io)
{
  status.SetFailed("Socket not implemented");
}

void Socket::Close(Status& status)
{
  status.SetFailed("Socket not implemented");
}

size_t Socket::Send(Status& status, const byte* data, size_t dataLength, SocketFlags::Enum flags)
{
  status.SetFailed("Socket not implemented");
  return 0;
}

size_t
Socket::SendTo(Status& status, const byte* data, size_t dataLength, const SocketAddress& to, SocketFlags::Enum flags)
{
  status.SetFailed("Socket not implemented");
  return 0;
}

size_t Socket::Receive(Status& status, byte* dataOut, size_t dataLength, SocketFlags::Enum flags)
{
  status.SetFailed("Socket not implemented");
  return 0;
}

size_t
Socket::ReceiveFrom(Status& status, byte* dataOut, size_t dataLength, SocketAddress& from, SocketFlags::Enum flags)
{
  status.SetFailed("Socket not implemented");
  return 0;
}

bool Socket::Select(Status& status, SocketSelect::Enum selectMode, float timeoutSeconds) const
{
  status.SetFailed("Socket not implemented");
  return false;
}

void Socket::GetSocketOption(Status& status, SocketOption::Enum option, void* value, size_t* valueLength) const
{
  status.SetFailed("Socket not implemented");
}

void Socket::GetSocketOption(Status& status, SocketIpv4Option::Enum option, void* value, size_t* valueLength) const
{
  status.SetFailed("Socket not implemented");
}

void Socket::GetSocketOption(Status& status, SocketIpv6Option::Enum option, void* value, size_t* valueLength) const
{
  status.SetFailed("Socket not implemented");
}

void Socket::GetSocketOption(Status& status, SocketTcpOption::Enum option, void* value, size_t* valueLength) const
{
  status.SetFailed("Socket not implemented");
}

void Socket::GetSocketOption(Status& status, SocketUdpOption::Enum option, void* value, size_t* valueLength) const
{
  status.SetFailed("Socket not implemented");
}

void Socket::SetSocketOption(Status& status, SocketOption::Enum option, const void* value, size_t valueLength)
{
  status.SetFailed("Socket not implemented");
}

void Socket::SetSocketOption(Status& status, SocketIpv4Option::Enum option, const void* value, size_t valueLength)
{
  status.SetFailed("Socket not implemented");
}

void Socket::SetSocketOption(Status& status, SocketIpv6Option::Enum option, const void* value, size_t valueLength)
{
  status.SetFailed("Socket not implemented");
}

void Socket::SetSocketOption(Status& status, SocketTcpOption::Enum option, const void* value, size_t valueLength)
{
  status.SetFailed("Socket not implemented");
}

void Socket::SetSocketOption(Status& status, SocketUdpOption::Enum option, const void* value, size_t valueLength)
{
  status.SetFailed("Socket not implemented");
}

SocketAddress QueryLocalSocketAddress(Status& status, const Socket& socket)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

SocketAddress QueryRemoteSocketAddress(Status& status, const Socket& socket)
{
  status.SetFailed("Socket not implemented");
  return SocketAddress();
}

} // namespace Zero
