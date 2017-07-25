///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean, Trevor Sundberg
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#if defined(COMPILER_GCC) ||  defined(COMPILER_CLANG)
  // This is not at all correct for GCC, but we just want it to
  // compile on GCC for Windows (POSIX takes a different path anyways)
  typedef int socklen_t;
  int inet_pton(int af, const char* src, void* dst)
  {
    return 0;
  }
  const char* inet_ntop(int af, const void* src, char* dst, socklen_t size)
  {
    return nullptr;
  }
#endif

// Include Winsock (nearly POSIX-compliant sockets)
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// Platform Conversion Types and Macros
typedef SOCKET           SOCKET_TYPE;
typedef ADDRESS_FAMILY   SOCKET_ADDRESS_FAMILY;
typedef addrinfo         ADDRESS_INFO;
typedef sockaddr         SOCKET_ADDRESS_TYPE;
typedef sockaddr_in      SOCKET_ADDRESS_IPV4;
typedef sockaddr_in6     SOCKET_ADDRESS_IPV6;
typedef SOCKADDR_STORAGE SOCKET_ADDRESS_STORAGE;
#define TRANSLATE_TO_PLATFORM_ENUM(value)   ((void)0)
#define TRANSLATE_FROM_PLATFORM_ENUM(value) ((void)0)

#include "Platform/Socket.hpp"
#include "Utility/Atomic.hpp"

#define CAST_HANDLE_TO_SOCKET(value) (static_cast<SOCKET_TYPE>(reinterpret_cast<size_t>(value)))
#define CAST_SOCKET_TO_HANDLE(value) (reinterpret_cast<OsHandle>(value))

#define TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(value, whatToReturn) \
        TRANSLATE_TO_PLATFORM_ENUM(value);                                      \
        if(status.Failed())                                                     \
          return whatToReturn

#define TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(value)         \
        TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(value, )

#define TRANSLATE_FROM_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(value, whatToReturn) \
        TRANSLATE_FROM_PLATFORM_ENUM(value);                                      \
        if(status.Failed())                                                       \
          return whatToReturn

#define TRANSLATE_FROM_PLATFORM_ENUM_OR_RETURN_FAILURE(value)         \
        TRANSLATE_FROM_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(value, )

namespace Zero
{

/// Sets the status error code and optional error string
void FailOnError(Status& status, int errorCode, StringParam errorString)
{
  // Set status error code and string
  status.SetFailed(errorString, errorCode);
}
void FailOnError(Status& status, int errorCode)
{
  // Create error string
  wchar_t* errorString = NULL;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                NULL, errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&errorString, 0, NULL);

  // Use error string
  FailOnError(status, errorCode, Narrow(errorString));

  // Free error string
  LocalFree(errorString);
}

/// Sets the status with the last error code and optional error string
void FailOnLastError(Status& status)
{
  // Get last error code
  FailOnError(status, WSAGetLastError());
}

//---------------------------------------------------------------------------------//
//                                SocketLibrary                                    //
//---------------------------------------------------------------------------------//

/// Manages the platform's underlying socket library
class SocketLibrary
{
public:
  /// Constructor
  SocketLibrary()
    : mReferenceCount(0),
      mWinsockData()
  {
  }

  /// Returns true if the platform's underlying socket library is initialized (reference count greater than zero), else false
  bool IsInitialized()
  {
    return mReferenceCount > 0;
  }

  /// Initializes the platform's underlying socket library (acquiring the resources needed to provide socket functionality)
  /// Safe to call multiple times (even without matching Uninitialize calls), internally reference counted
  /// If already initialized (reference count greater than zero), only increments the reference count
  /// If not yet initialized (reference count of zero), initializes the socket library and if successful, increments the reference count
  void Initialize(Status& status)
  {
    // Already initialized?
    if(mReferenceCount > 0)
    {
      // Increment reference count
      ++mReferenceCount;
    }
    // Not initialized?
    else
    {
      Assert(mReferenceCount == 0);

      //
      // Initialize Socket Library
      //
      ZPrint("Initializing Socket Library...\n");
      mWinsockData = WSADATA();
      int result = WSAStartup(WINSOCK_VERSION, &mWinsockData);
      if(result != 0) // Unable?
      {
        FailOnError(status, result);
        Error("WSAStartup failed (%d : %s)\n", status.Context, status.Message.c_str());
        return;
      }

      // Increment reference count
      ++mReferenceCount;
    }

    // (Should be initialized)
    Assert(IsInitialized());
  }

  /// Uninitializes the platform's underlying socket library (releasing the resources needed to provide socket functionality)
  /// Safe to call multiple times (even without matching Initialize calls), internally reference counted
  /// If not initialized (reference count of zero), does nothing
  /// If initialized more than once (reference count greater than one), decrements the reference count
  /// If initialized once (reference count of one), decrements the reference count and uninitializes the socket library
  void Uninitialize(Status& status)
  {
    Zero::TimerBlock block("Uninitializing Socket Library.");

    // Not initialized?
    if(mReferenceCount == 0)
    {
      // (Should be uninitialized)
      Assert(!IsInitialized());
      return;
    }
    // Initialized more than once?
    else if(mReferenceCount > 1)
    {
      // Decrement reference count
      --mReferenceCount;
    }
    // Initialized once?
    else
    {
      Assert(mReferenceCount == 1);

      //
      // Uninitialize Socket Library
      //
      //ZPrint("Uninitializing Socket Library...\n");
      if(WSACleanup() == SOCKET_ERROR) // Unable?
      {
        FailOnLastError(status);
        Error("WSACleanup failed (%d : %s)\n", status.Context, status.Message.c_str());
      }
      mWinsockData = WSADATA();

      // Decrement reference count
      --mReferenceCount;

      // (Should be uninitialized)
      Assert(!IsInitialized());
    }
  }

  /// Initialization reference count
  Atomic<uint32> mReferenceCount;
  /// Winsock startup data
  WSADATA        mWinsockData;
};

// Socket library instance
static SocketLibrary gSocketLibrary;

u16 HostToNetworkShort(u16 hostShort)
{
  return htons(hostShort);
}
s16 HostToNetworkShort(s16 hostShort)
{
  return htons(hostShort);
}
u32 HostToNetworkLong(u32 hostLong)
{
  return htonl(hostLong);
}
s32 HostToNetworkLong(s32 hostLong)
{
  return htonl(hostLong);
}

u16 NetworkToHostShort(u16 networkShort)
{
  return ntohs(networkShort);
}
s16 NetworkToHostShort(s16 networkShort)
{
  return ntohs(networkShort);
}
u32 NetworkToHostLong(u32 networkLong)
{
  return ntohl(networkLong);
}
s32 NetworkToHostLong(s32 networkLong)
{
  return ntohl(networkLong);
}

//---------------------------------------------------------------------------------//
//                                SocketAddress                                    //
//---------------------------------------------------------------------------------//

SocketAddress::SocketAddress()
{
  // Clear address
  Clear();
}

SocketAddress::SocketAddress(const SocketAddress& rhs)
{
  // Copy address
  Clear();
  *this = rhs;
}

SocketAddress& SocketAddress::operator =(const SocketAddress& rhs)
{
  memcpy(this->mPrivateData, rhs.mPrivateData, sizeof(mPrivateData));
  return *this;
}

bool SocketAddress::operator ==(const SocketAddress& rhs) const
{
  return memcmp(this->mPrivateData, rhs.mPrivateData, sizeof(mPrivateData)) == 0;
}
bool SocketAddress::operator !=(const SocketAddress& rhs) const
{
  return !(*this == rhs);
}
bool SocketAddress::operator  <(const SocketAddress& rhs) const
{
  return memcmp(this->mPrivateData, rhs.mPrivateData, sizeof(mPrivateData)) < 0;
}

SocketAddress::operator bool(void) const
{
  return !IsEmpty();
}

bool SocketAddress::IsEmpty() const
{
  return *this == SocketAddress();
}

SocketAddressFamily::Enum SocketAddress::GetAddressFamily() const
{
  // Get socket address family
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)mPrivateData;
  SocketAddressFamily::Enum addressFamily = SocketAddressFamily::Enum(sockAddrStorage->ss_family);

  // Translate platform-specific enum as necessary
  Status status;
  TRANSLATE_FROM_PLATFORM_ENUM(addressFamily);
  StatusReturnIfFailed(status, SocketAddressFamily::Unspecified);

  // Success
  return addressFamily;
}

void SocketAddress::SetIpv4(Status& status, StringParam host, uint port)
{
  SetIpv4(status, host, port, SocketAddressResolutionFlags::None);
}
void SocketAddress::SetIpv4(Status& status, StringParam host, uint port, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  // Resolve IPv4 socket address
  *this = ResolveSocketAddress(status, host, PortToString(port), SocketAddressFamily::InternetworkV4, addressResolutionFlags);
}

void SocketAddress::SetIpv6(Status& status, StringParam host, uint port)
{
  SetIpv6(status, host, port, SocketAddressResolutionFlags::None);
}
void SocketAddress::SetIpv6(Status& status, StringParam host, uint port, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  // Resolve IPv6 socket address
  *this = ResolveSocketAddress(status, host, PortToString(port), SocketAddressFamily::InternetworkV6, addressResolutionFlags);
}

void SocketAddress::SetIpPort(Status& status, uint port)
{
  switch(GetAddressFamily())
  {
  // IPv4 socket address family?
  case SocketAddressFamily::InternetworkV4:
    ((SOCKET_ADDRESS_IPV4*)mPrivateData)->sin_port = HostToNetworkShort((ushort)port);
    return;
  // IPv6 socket address family?
  case SocketAddressFamily::InternetworkV6:
    ((SOCKET_ADDRESS_IPV6*)mPrivateData)->sin6_port = HostToNetworkShort((ushort)port);
    return;

  // Other socket address family?
  default:
    FailOnError(status, GetAddressFamily(), "Not an IPv4 or IPv6 socket address");
    return;
  }
}
uint SocketAddress::GetIpPort(Status& status) const
{
  switch(GetAddressFamily())
  {
  // IPv4 socket address family?
  case SocketAddressFamily::InternetworkV4:
    return NetworkToHostShort(((SOCKET_ADDRESS_IPV4*)mPrivateData)->sin_port);
  // IPv6 socket address family?
  case SocketAddressFamily::InternetworkV6:
    return NetworkToHostShort(((SOCKET_ADDRESS_IPV6*)mPrivateData)->sin6_port);

  // Other socket address family?
  default:
    FailOnError(status, GetAddressFamily(), "Not an IPv4 or IPv6 socket address");
    return 0;
  }
}

void SocketAddress::Clear()
{
  ZeroMemClearPrivateData(SOCKET_ADDRESS_STORAGE);
  Assert(GetAddressFamily() == SocketAddressFamily::Unspecified);
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, SocketAddress& socketAddress)
{
  // Note: Currently only supports IP address serialization (InternetworkV4 and InternetworkV6 socket addresses)

  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)socketAddress.mPrivateData;

    // Get internet protocol version
    InternetProtocol::Enum internetProtocol = InternetProtocol::Unspecified;
    switch(socketAddress.GetAddressFamily())
    {
    case SocketAddressFamily::InternetworkV4:
      internetProtocol = InternetProtocol::V4;
      break;
    case SocketAddressFamily::InternetworkV6:
      internetProtocol = InternetProtocol::V6;
      break;

    default:
      internetProtocol = InternetProtocol::Unspecified;
      break;
    }

    // Write internet protocol version
    bitStream.WriteQuantized(internetProtocol, InternetProtocolMin, InternetProtocolMax);

    // Write IP address according to it's IP version
    switch(internetProtocol)
    {
    case InternetProtocol::V4:
      {
        // Write network-order IPv4 host address
        bitStream.Write(((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_addr.s_addr);

        // Write network-order IP port
        bitStream.Write(((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_port);
      }
      break;

    case InternetProtocol::V6:
      {
        // Write network-order IPv6 host address
        bitStream.Write(((SOCKET_ADDRESS_IPV6*)sockAddrStorage)->sin6_addr.s6_addr);

        // Write network-order IP port
        bitStream.Write(((SOCKET_ADDRESS_IPV6*)sockAddrStorage)->sin6_port);
      }
      break;

    default:
      Assert(false);
    case InternetProtocol::Unspecified:
      // (Either the socket address is empty or not an IP address)
      // (So there's nothing more we will write)
      break;
    }

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)socketAddress.mPrivateData;

    // Read internet protocol version
    InternetProtocol::Enum internetProtocol = InternetProtocol::Unspecified;
    bitStream.ReadQuantized(internetProtocol, InternetProtocolMin, InternetProtocolMax);

    // Translate to socket address family
    SocketAddressFamily::Enum addressFamily = SocketAddressFamily::Unspecified;
    switch(internetProtocol)
    {
    case InternetProtocol::V4:
      addressFamily = SocketAddressFamily::InternetworkV4;
      break;
    case InternetProtocol::V6:
      addressFamily = SocketAddressFamily::InternetworkV6;
      break;

    default:
      addressFamily = SocketAddressFamily::Unspecified;
      break;
    }

    // Translate platform-specific enum as necessary
    Status status;
    TRANSLATE_TO_PLATFORM_ENUM(addressFamily);
    StatusReturnIfFailed(status, 0);

    // Read IP address according to it's IP version
    switch(internetProtocol)
    {
    case InternetProtocol::V4:
      {
        // Set IPv4 address family
        ((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_family = (SOCKET_ADDRESS_FAMILY)addressFamily;

        // Read network-order IPv4 host address
        ReturnIf(!bitStream.Read(((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_addr.s_addr), 0);

        // Read network-order IPv4 port
        ReturnIf(!bitStream.Read(((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_port), 0);
      }
      break;

    case InternetProtocol::V6:
      {
        // Set IPv6 address family
        ((SOCKET_ADDRESS_IPV6*)sockAddrStorage)->sin6_family = (SOCKET_ADDRESS_FAMILY)addressFamily;

        // Read network-order IPv6 host address
        ReturnIf(!bitStream.Read(((SOCKET_ADDRESS_IPV6*)sockAddrStorage)->sin6_addr.s6_addr), 0);

        // Read network-order IPv6 port
        ReturnIf(!bitStream.Read(((SOCKET_ADDRESS_IPV6*)sockAddrStorage)->sin6_port), 0);
      }
      break;

    default:
      Assert(false);
    case InternetProtocol::Unspecified:
      // (Either the socket address is empty or not an IP address)
      // (So there's nothing more we will read)
      break;
    }

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                   SocketProtocol::Enum protocol, SocketType::Enum type, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  SocketAddress address;

  // Choose the 'any' address?
  bool chooseAnyAddress = (addressResolutionFlags & SocketAddressResolutionFlags::AnyAddress);

  // Translate platform-specific enums as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(addressFamily, SocketAddress());
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(protocol, SocketAddress());
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(type, SocketAddress());
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(addressResolutionFlags, SocketAddress());

  // Initialize socket library (as needed)
  Status initStatus;
  Socket::InitializeSocketLibrary(initStatus);
  if(initStatus.Failed()) // Unable?
  {
    status = initStatus;
    return SocketAddress();
  }

  // Create socket type hints
  ADDRESS_INFO hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = addressFamily;
  hints.ai_protocol = protocol;
  hints.ai_socktype = type;
  hints.ai_flags    = addressResolutionFlags;

  // Resolve host list
  ADDRESS_INFO* hosts = nullptr;
  int result = getaddrinfo(chooseAnyAddress ? nullptr : host.c_str(),
                           service.c_str(),
                           &hints,
                           &hosts);
  if(result != 0) // Unable?
  {
    FailOnError(status, result);
    return SocketAddress();
  }

  // Set socket address to first host returned in the host list
  memcpy((SOCKET_ADDRESS_TYPE*)address.mPrivateData, hosts->ai_addr, hosts->ai_addrlen);

  // Free host list
  freeaddrinfo(hosts);

  // Uninitialize socket library (as needed)
  Status uninitStatus;
  Socket::UninitializeSocketLibrary(uninitStatus);

  // Success
  return address;
}
SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                   SocketProtocol::Enum protocol, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  return ResolveSocketAddress(status, host, service, addressFamily, protocol, SocketType::Unspecified, addressResolutionFlags);
}
SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                   SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  return ResolveSocketAddress(status, host, service, addressFamily, SocketProtocol::Unspecified, addressResolutionFlags);
}
SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  return ResolveSocketAddress(status, host, service, SocketAddressFamily::Unspecified, addressResolutionFlags);
}
SocketAddress ResolveSocketAddress(Status& status, StringParam host, StringParam service)
{
  return ResolveSocketAddress(status, host, service, SocketAddressResolutionFlags::None);
}

Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                               SocketProtocol::Enum protocol, SocketType::Enum type, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  Array<SocketAddress> addresses;

  // Choose the 'any' address?
  bool chooseAnyAddress = (addressResolutionFlags & SocketAddressResolutionFlags::AnyAddress);

  // Translate platform-specific enums as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(addressFamily, Array<SocketAddress>());
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(protocol, Array<SocketAddress>());
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(type, Array<SocketAddress>());
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(addressResolutionFlags, Array<SocketAddress>());

  // Initialize socket library (as needed)
  Status initStatus;
  Socket::InitializeSocketLibrary(initStatus);
  if(initStatus.Failed()) // Unable?
  {
    status = initStatus;
    return Array<SocketAddress>();
  }

  // Create socket type hints
  ADDRESS_INFO hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = addressFamily;
  hints.ai_protocol = protocol;
  hints.ai_socktype = type;
  hints.ai_flags    = addressResolutionFlags;

  // Resolve host list
  ADDRESS_INFO* hosts = nullptr;
  int result = getaddrinfo(chooseAnyAddress ? nullptr : host.c_str(),
                           service.c_str(),
                           &hints,
                           &hosts);
  if(result != 0) // Unable?
  {
    FailOnError(status, result);
    return Array<SocketAddress>();
  }
  
  // For every host returned in the host list
  for(ADDRESS_INFO* iter = hosts; iter != nullptr; iter = iter->ai_next)
  {
    // Set socket address to host
    SocketAddress address;
    memcpy((SOCKET_ADDRESS_TYPE*)address.mPrivateData, iter->ai_addr, iter->ai_addrlen);

    // Push back socket address
    addresses.PushBack(address);
  }

  // Free host list
  freeaddrinfo(hosts);

  // Uninitialize socket library (as needed)
  Status uninitStatus;
  Socket::UninitializeSocketLibrary(uninitStatus);

  // Success
  return addresses;
}
Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                               SocketProtocol::Enum protocol, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  return ResolveAllSocketAddresses(status, host, service, addressFamily, protocol, SocketType::Unspecified, addressResolutionFlags);
}
Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressFamily::Enum addressFamily,
                                               SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  return ResolveAllSocketAddresses(status, host, service, addressFamily, SocketProtocol::Unspecified, addressResolutionFlags);
}
Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service, SocketAddressResolutionFlags::Enum addressResolutionFlags)
{
  return ResolveAllSocketAddresses(status, host, service, SocketAddressFamily::Unspecified, addressResolutionFlags);
}
Array<SocketAddress> ResolveAllSocketAddresses(Status& status, StringParam host, StringParam service)
{
  return ResolveAllSocketAddresses(status, host, service, SocketAddressResolutionFlags::None);
}

Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address, SocketNameResolutionFlags::Enum nameResolutionFlags)
{
  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM(nameResolutionFlags);
  if(status.Failed())
    return Pair<String, String>();

  // Initialize socket library (as needed)
  Status initStatus;
  Socket::InitializeSocketLibrary(initStatus);
  if(initStatus.Failed()) // Unable?
  {
    status = initStatus;
    return Pair<String, String>();
  }

  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)address.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);

  // Resolve host and service names
  char host[MaxFullyQualifiedDomainNameStringLength] = {};
  char service[MaxServiceNameStringLength]           = {};
  int result = getnameinfo((SOCKET_ADDRESS_TYPE*)sockAddrStorage,
                           sockAddrLength,
                           host,
                           MaxFullyQualifiedDomainNameStringLength,
                           service,
                           MaxServiceNameStringLength,
                           nameResolutionFlags);
  if(result != 0) // Unable?
  {
    FailOnError(status, result);
    return Pair<String, String>();
  }

  // Uninitialize socket library (as needed)
  Status uninitStatus;
  Socket::UninitializeSocketLibrary(uninitStatus);

  // Success
  return Pair<String, String>(host, service);
}
Pair<String, String> ResolveHostAndServiceNames(Status& status, const SocketAddress& address)
{
  return ResolveHostAndServiceNames(status, address, SocketNameResolutionFlags::None);
}

String SocketAddressToString(Status& status, SocketAddressFamily::Enum addressFamily, const SocketAddress& address)
{
  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM(addressFamily);
  StatusReturnIfFailed(status, String());

  // Convert socket address to string
  char result[Ipv6StringLength] = {};
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)address.mPrivateData;
  if(!inet_ntop(addressFamily, &((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_addr, result, sizeof(result))) // Unable?
  {
    FailOnLastError(status);
    return String();
  }

  // Success
  return String(result);
}
String SocketAddressToString(SocketAddressFamily::Enum addressFamily, const SocketAddress& address)
{
  Status status;
  String result = SocketAddressToString(status, addressFamily, address);
  return result;
}

SocketAddress StringToSocketAddress(Status& status, SocketAddressFamily::Enum addressFamily, StringParam address)
{
  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM(addressFamily);
  StatusReturnIfFailed(status, SocketAddress());

  // Convert string to socket address
  SocketAddress result;
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)result.mPrivateData;
  if(inet_pton(addressFamily, address.c_str(), &((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_addr) != 1) // Unable?
  {
    FailOnLastError(status);
    return SocketAddress();
  }

  // Success
  return result;
}
SocketAddress StringToSocketAddress(SocketAddressFamily::Enum addressFamily, StringParam address)
{
  Status status;
  SocketAddress result = StringToSocketAddress(status, addressFamily, address);
  return result;
}

bool IsValidIpv4Address(const SocketAddress& address)
{
  return Ipv4AddressToString(address) != String();
}
bool IsValidIpv6Address(const SocketAddress& address)
{
  return Ipv6AddressToString(address) != String();
}

bool IsValidIpv4Address(StringParam address)
{
  return StringToIpv4Address(address) != SocketAddress();
}
bool IsValidIpv6Address(StringParam address)
{
  return StringToIpv6Address(address) != SocketAddress();
}

String Ipv4AddressToString(const SocketAddress& address)
{
  return SocketAddressToString(SocketAddressFamily::InternetworkV4, address);
}
String Ipv6AddressToString(const SocketAddress& address)
{
  return SocketAddressToString(SocketAddressFamily::InternetworkV6, address);
}

String PortToString(uint port)
{
  // Convert port to string
  return String::Format("%u", port);
}

String Ipv4AddressToStringWithPort(const SocketAddress& address)
{
  // Get host
  String hostString = Ipv4AddressToString(address);
  if(hostString == String()) // Unable?
    return String();

  // Get port
  Status status;
  uint port = address.GetIpPort(status);
  if(status.Failed()) // Unable?
    return String();
  String portString = PortToString(port);

  // Concatenate host:port string
  StringBuilder builder;
  builder.Append(hostString);
  builder.Append(":");
  builder.Append(portString);
  return builder.ToString();
}
String Ipv6AddressToStringWithPort(const SocketAddress& address)
{
  // Get host
  String hostString = Ipv6AddressToString(address);
  if(hostString == String()) // Unable?
    return String();

  // Get port
  Status status;
  uint port = address.GetIpPort(status);
  if(status.Failed()) // Unable?
    return String();
  String portString = PortToString(port);

  // Concatenate host:port string
  StringBuilder builder;
  builder.Append(hostString);
  builder.Append(":");
  builder.Append(portString);
  return builder.ToString();
}

SocketAddress StringToIpv4Address(StringParam address)
{
  return StringToSocketAddress(SocketAddressFamily::InternetworkV4, address);
}
SocketAddress StringToIpv4Address(StringParam address, ushort port)
{
  // Convert string to IPv4 address
  SocketAddress result = StringToIpv4Address(address);
  if(result != SocketAddress()) // Successful?
  {
    // Set port
    SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)result.mPrivateData;
    ((SOCKET_ADDRESS_IPV4*)sockAddrStorage)->sin_port = HostToNetworkShort(port);
  }
  return result;
}

SocketAddress StringToIpv6Address(StringParam address)
{
  return StringToSocketAddress(SocketAddressFamily::InternetworkV6, address);
}
SocketAddress StringToIpv6Address(StringParam address, ushort port)
{
  // Convert string to IPv6 address
  SocketAddress result = StringToIpv6Address(address);
  if(result != SocketAddress()) // Successful?
  {
    // Set port
    SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)result.mPrivateData;
    ((SOCKET_ADDRESS_IPV6*)sockAddrStorage)->sin6_port = HostToNetworkShort(port);
  }
  return result;
}

//---------------------------------------------------------------------------------//
//                                    Socket                                       //
//---------------------------------------------------------------------------------//

/// Clears the socket to it's default state
void Clear(Socket& socket)
{
  socket.mHandle        = CAST_SOCKET_TO_HANDLE(INVALID_SOCKET);
  socket.mAddressFamily = SocketAddressFamily::Unspecified;
  socket.mType          = SocketType::Unspecified;
  socket.mProtocol      = SocketProtocol::Unspecified;
  socket.mIsListening   = false;
  socket.mIsBlocking    = true;
}

/// Destroys the socket to it's default state
void Destroy(Socket& socket)
{
  // Socket still open?
  if(socket.IsOpen())
  {
    // Socket was connected?
    if(socket.HasConnectedRemoteAddress())
    {
      // Shut down connection
      Status status;
      socket.Shutdown(status, SocketIo::Both);
      if(status.Failed()) // Unable?
        ZPrint("Error shutting down socket connection (%d : %s)\n", status.Context, status.Message.c_str());
    }

    // Close socket
    Status status;
    socket.Close(status);
    if(status.Failed()) // Unable?
      ZPrint("Error closing socket (%d : %s)\n", status.Context, status.Message.c_str());
  }
}

//
// Static Member Functions
//

size_t Socket::GetMaxListenBacklog()
{
  return SOMAXCONN;
}

bool Socket::IsCommonReceiveError(int extendedErrorCode)
{
  switch(extendedErrorCode)
  {
  case WSAENETRESET:
  case WSAECONNABORTED:
  case WSAECONNRESET:
  case WSAEWOULDBLOCK:
    return true;

  default:
    return false;
  }
}

bool Socket::IsCommonAcceptError(int extendedErrorCode)
{
  switch(extendedErrorCode)
  {
  case WSAECONNRESET:
  case WSAEWOULDBLOCK:
    return true;

  default:
    return false;
  }
}

bool Socket::IsCommonConnectError(int extendedErrorCode)
{
  switch(extendedErrorCode)
  {
  case WSAEWOULDBLOCK:
  case WSAEINPROGRESS:
    return true;

  default:
    return false;
  }
}

bool Socket::IsSocketLibraryInitialized()
{
  return gSocketLibrary.IsInitialized();
}

void Socket::InitializeSocketLibrary(Status& status)
{
  return gSocketLibrary.Initialize(status);
}

void Socket::UninitializeSocketLibrary(Status& status)
{
  return gSocketLibrary.Uninitialize(status);
}

//
// Non-Static Member Functions
//

Socket::Socket()
{
  // Clear this socket
  Clear(*this);

  // Initialize socket library (as needed)
  Status status;
  InitializeSocketLibrary(status);
}

Socket::~Socket()
{
  // Destroy this socket
  Destroy(*this);

  // Uninitialize socket library (as needed)
  Status status;
  UninitializeSocketLibrary(status);
}

Socket::Socket(MoveReference<Socket> rhs)
{
  // Clear this socket
  Clear(*this);

  // Move data from rhs
  *this = ZeroMove(rhs);

  // Initialize socket library (as needed)
  Status status;
  InitializeSocketLibrary(status);
}

Socket& Socket::operator =(MoveReference<Socket> rhs)
{
  // Destroy this socket
  Destroy(*this);

  // Move data from rhs
  mHandle        = rhs->mHandle;
  mAddressFamily = rhs->mAddressFamily;
  mType          = rhs->mType;
  mProtocol      = rhs->mProtocol;
  mIsListening   = rhs->mIsListening;
  mIsBlocking    = rhs->mIsBlocking;

  // Clear rhs socket
  Clear(*rhs);
  return *this;
}

bool Socket::IsOpen() const
{
  return (mHandle != CAST_SOCKET_TO_HANDLE(INVALID_SOCKET));
}

SocketAddressFamily::Enum Socket::GetAddressFamily() const
{
  return mAddressFamily;
}

SocketType::Enum Socket::GetType() const
{
  return mType;
}

SocketProtocol::Enum Socket::GetProtocol() const
{
  return mProtocol;
}

bool Socket::IsBound() const
{
  return !GetBoundLocalAddress().IsEmpty();
}

SocketAddress Socket::GetBoundLocalAddress() const
{
  // Get the socket's local address
  Status status;
  return QueryLocalSocketAddress(status, *this);
}

bool Socket::IsListening() const
{
  return mIsListening;
}

bool Socket::IsBlocking() const
{
  return mIsBlocking;
}

bool Socket::HasConnectedRemoteAddress() const
{
  return !GetConnectedRemoteAddress().IsEmpty();
}

SocketAddress Socket::GetConnectedRemoteAddress() const
{
  // Get the socket's remote address
  Status status;
  return QueryRemoteSocketAddress(status, *this);
}

void Socket::Open(Status& status, SocketAddressFamily::Enum addressFamily, SocketType::Enum type, SocketProtocol::Enum protocol)
{
  // Translate platform-specific enums as necessary
  SocketAddressFamily::Enum addressFamily_ = addressFamily;
  SocketType::Enum          type_          = type;
  SocketProtocol::Enum      protocol_      = protocol;
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(addressFamily_);
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(type_);
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(protocol_);

  // Already open?
  if(IsOpen())
  {
    // Close socket
    Close(status);
    if(status.Failed()) // Unable?
      return;
  }

  // Create socket
  mHandle = CAST_SOCKET_TO_HANDLE(socket(addressFamily_, type_, protocol_));
  if(mHandle == CAST_SOCKET_TO_HANDLE(INVALID_SOCKET)) // Unable?
    return FailOnLastError(status);

  // Store values
  mAddressFamily = addressFamily;
  mType          = type;
  mProtocol      = protocol;
}

void Socket::Bind(Status& status, const SocketAddress& localAddress)
{
  // Bind socket to specified local address
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)localAddress.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  if(bind(CAST_HANDLE_TO_SOCKET(mHandle), (SOCKET_ADDRESS_TYPE*)sockAddrStorage, sockAddrLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}

void Socket::Listen(Status& status, uint backlog)
{
  // Set socket listening mode
  if(listen(CAST_HANDLE_TO_SOCKET(mHandle), backlog) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);

  // Store value
  mIsListening = true;
}

void Socket::SetBlocking(Status& status, bool blocking)
{
  // Set socket blocking mode
  ulong blockingMode = blocking ? 0 : 1;
  if(ioctlsocket(CAST_HANDLE_TO_SOCKET(mHandle), FIONBIO, &blockingMode) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);

  // Store value
  mIsBlocking = blocking;
}

void Socket::Accept(Status& status, Socket* connectionOut)
{
  // Output socket already open?
  if(connectionOut->IsOpen())
  {
    // Close output socket
    connectionOut->Close(status);
    if(status.Failed()) // Unable?
      return;
  }

  // Accept incoming connection as a new socket
  SocketAddress           remoteAddress;
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)remoteAddress.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  SOCKET_TYPE newSocket = accept(CAST_HANDLE_TO_SOCKET(mHandle), (SOCKET_ADDRESS_TYPE*)sockAddrStorage, &sockAddrLength);
  if(newSocket == INVALID_SOCKET) // Unable?
    return FailOnLastError(status);

  // Store values
  connectionOut->mHandle        = CAST_SOCKET_TO_HANDLE(newSocket);
  connectionOut->mAddressFamily = GetAddressFamily();
  connectionOut->mType          = GetType();
  connectionOut->mProtocol      = GetProtocol();
  Assert(connectionOut->GetConnectedRemoteAddress() == remoteAddress);
}

void Socket::Connect(Status& status, const SocketAddress& remoteAddress)
{
  // Connect socket to specified remote address
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)remoteAddress.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  if(connect(CAST_HANDLE_TO_SOCKET(mHandle), (SOCKET_ADDRESS_TYPE*)sockAddrStorage, sockAddrLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}

void Socket::Shutdown(Status& status, SocketIo::Enum io)
{
  // Shut down socket operation(s)
  if(shutdown(CAST_HANDLE_TO_SOCKET(mHandle), (int)io) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}

void Socket::Close(Status& status)
{
  int result = 0;

  // Close socket
  result = closesocket(CAST_HANDLE_TO_SOCKET(mHandle));

  if(result == SOCKET_ERROR) // Unable?
    FailOnLastError(status);

  // Clear values
  Clear(*this);
}

size_t Socket::Send(Status& status, const byte* data, size_t dataLength, SocketFlags::Enum flags)
{
  // Translate platform-specific enums as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(flags, 0);

  // Send data over socket to connected remote address
  int result = send(CAST_HANDLE_TO_SOCKET(mHandle), (const char*)data, (int)dataLength, (int)flags);
  if(result == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return 0;
  }

  // Success
  return result;
}

size_t Socket::SendTo(Status& status, const byte* data, size_t dataLength, const SocketAddress& to, SocketFlags::Enum flags)
{
  // Translate platform-specific enums as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(flags, 0);

  // Send data over socket to specified remote address
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)to.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  int result = sendto(CAST_HANDLE_TO_SOCKET(mHandle), (const char*)data, (int)dataLength, (int)flags, (SOCKET_ADDRESS_TYPE*)sockAddrStorage, sockAddrLength);
  if(result == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return 0;
  }

  // Success
  return result;
}

size_t Socket::Receive(Status& status, byte* dataOut, size_t dataLength, SocketFlags::Enum flags)
{
  // Translate platform-specific enums as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(flags, 0);

  // Receive data over socket from connected remote address
  int result = recv(CAST_HANDLE_TO_SOCKET(mHandle), (char*)dataOut, (int)dataLength, (int)flags);
  if(result == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return 0;
  }

  // Success
  return result;
}

size_t Socket::ReceiveFrom(Status& status, byte* dataOut, size_t dataLength, SocketAddress& from, SocketFlags::Enum flags)
{
  // Translate platform-specific enums as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE_VALUE(flags, 0);

  // Receive data over socket from any remote address
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)from.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  int result = recvfrom(CAST_HANDLE_TO_SOCKET(mHandle), (char*)dataOut, (int)dataLength, (int)flags, (SOCKET_ADDRESS_TYPE*)sockAddrStorage, &sockAddrLength);
  if(result == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return 0;
  }

  // Success
  return result;
}

bool Socket::Select(Status& status, SocketSelect::Enum selectMode, float timeoutSeconds) const
{
  // Configure select timeout
  timeval timeout = {};
  timeout.tv_sec  = (long)timeoutSeconds;
  timeout.tv_usec = (long)((timeoutSeconds - timeout.tv_sec) * 1000000L);

  // Configure select operation
  fd_set socketSet;
  FD_ZERO(&socketSet);
  FD_SET(CAST_HANDLE_TO_SOCKET(mHandle), &socketSet);

  // Query select for specified socket operability status
  int result = 0;
  switch(selectMode)
  {
  case SocketSelect::Read:
    result = select(0, &socketSet, NULL, NULL, &timeout);
    break;
  case SocketSelect::Write:
    result = select(0, NULL, &socketSet, NULL, &timeout);
    break;
  case SocketSelect::Error:
    result = select(0, NULL, NULL, &socketSet, &timeout);
    break;

  default:
    Error("Invalid switch value");
    break;
  }
  if(result == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return false;
  }

  // Success
  return (result != 0);
}

void Socket::GetSocketOption(Status& status, SocketOption::Enum option, void* value, size_t* valueLength) const
{
  Assert(valueLength && *valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Get socket option
  if(getsockopt(CAST_HANDLE_TO_SOCKET(mHandle), SOL_SOCKET, (int)option, (char*)value, (socklen_t*)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::GetSocketOption(Status& status, SocketIpv4Option::Enum option, void* value, size_t* valueLength) const
{
  Assert(valueLength && *valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket address family?
  if(GetAddressFamily() != SocketAddressFamily::InternetworkV4)
    return FailOnError(status, option, "Invalid socket option, not an IPv4 socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Get socket option
  if(getsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_IP, (int)option, (char*)value, (socklen_t*)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::GetSocketOption(Status& status, SocketIpv6Option::Enum option, void* value, size_t* valueLength) const
{
  Assert(valueLength && *valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket address family?
  if(GetAddressFamily() != SocketAddressFamily::InternetworkV6)
    return FailOnError(status, option, "Invalid socket option, not an IPv6 socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Get socket option
  if(getsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_IPV6, (int)option, (char*)value, (socklen_t*)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::GetSocketOption(Status& status, SocketTcpOption::Enum option, void* value, size_t* valueLength) const
{
  Assert(valueLength && *valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket protocol?
  if(GetProtocol() != SocketProtocol::Tcp)
    return FailOnError(status, option, "Invalid socket option, not a TCP socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Get socket option
  if(getsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_TCP, (int)option, (char*)value, (socklen_t*)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::GetSocketOption(Status& status, SocketUdpOption::Enum option, void* value, size_t* valueLength) const
{
  Assert(valueLength && *valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket protocol?
  if(GetProtocol() != SocketProtocol::Udp)
    return FailOnError(status, option, "Invalid socket option, not a UDP socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Get socket option
  if(getsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_UDP, (int)option, (char*)value, (socklen_t*)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}

void Socket::SetSocketOption(Status& status, SocketOption::Enum option, const void* value, size_t valueLength)
{
  Assert(valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Set socket option
  if(setsockopt(CAST_HANDLE_TO_SOCKET(mHandle), SOL_SOCKET, (int)option, (const char*)value, (int)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::SetSocketOption(Status& status, SocketIpv4Option::Enum option, const void* value, size_t valueLength)
{
  Assert(valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket address family?
  if(GetAddressFamily() != SocketAddressFamily::InternetworkV4)
    return FailOnError(status, option, "Invalid socket option, not an IPv4 socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Set socket option
  if(setsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_IP, (int)option, (const char*)value, (int)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::SetSocketOption(Status& status, SocketIpv6Option::Enum option, const void* value, size_t valueLength)
{
  Assert(valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket address family?
  if(GetAddressFamily() != SocketAddressFamily::InternetworkV6)
    return FailOnError(status, option, "Invalid socket option, not an IPv6 socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Set socket option
  if(setsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_IPV6, (int)option, (const char*)value, (int)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::SetSocketOption(Status& status, SocketTcpOption::Enum option, const void* value, size_t valueLength)
{
  Assert(valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket protocol?
  if(GetProtocol() != SocketProtocol::Tcp)
    return FailOnError(status, option, "Invalid socket option, not a TCP socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Set socket option
  if(setsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_TCP, (int)option, (const char*)value, (int)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}
void Socket::SetSocketOption(Status& status, SocketUdpOption::Enum option, const void* value, size_t valueLength)
{
  Assert(valueLength, "valueLength must contain the non-zero size of the value parameter");

  // Wrong socket protocol?
  if(GetProtocol() != SocketProtocol::Udp)
    return FailOnError(status, option, "Invalid socket option, not a UDP socket");

  // Translate platform-specific enum as necessary
  TRANSLATE_TO_PLATFORM_ENUM_OR_RETURN_FAILURE(option);

  // Set socket option
  if(setsockopt(CAST_HANDLE_TO_SOCKET(mHandle), IPPROTO_UDP, (int)option, (const char*)value, (int)valueLength) == SOCKET_ERROR) // Unable?
    return FailOnLastError(status);
}

SocketAddress QueryLocalSocketAddress(Status& status, const Socket& socket)
{
  // Get local socket address information
  SocketAddress           address;
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)address.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  if(getsockname(CAST_HANDLE_TO_SOCKET(socket.mHandle), (SOCKET_ADDRESS_TYPE*)sockAddrStorage, &sockAddrLength) == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return SocketAddress();
  }

  // Success
  return address;
}

SocketAddress QueryRemoteSocketAddress(Status& status, const Socket& socket)
{
  // Get remote socket address information
  SocketAddress           address;
  SOCKET_ADDRESS_STORAGE* sockAddrStorage = (SOCKET_ADDRESS_STORAGE*)address.mPrivateData;
  socklen_t               sockAddrLength  = sizeof(SOCKET_ADDRESS_STORAGE);
  if(getpeername(CAST_HANDLE_TO_SOCKET(socket.mHandle), (SOCKET_ADDRESS_TYPE*)sockAddrStorage, &sockAddrLength) == SOCKET_ERROR) // Unable?
  {
    FailOnLastError(status);
    return SocketAddress();
  }

  // Success
  return address;
}

} // namespace Zero
