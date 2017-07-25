///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  IpAddress                                      //
//---------------------------------------------------------------------------------//

/// Updates an IP address' numeric "host:port" string
void UpdateHostPortString(Status& status, IpAddress& ipAddress)
{
  ErrorIf(!ipAddress.IsValid());

  // Get host string
  String host;
  switch(ipAddress.GetInternetProtocol())
  {
  case InternetProtocol::V4:
    host = Ipv4AddressToString(ipAddress);
    break;

  case InternetProtocol::V6:
    host = Ipv6AddressToString(ipAddress);
    break;

  default:
  case InternetProtocol::Unspecified:
    break;
  }
  if(host == String()) // Unable?
  {
    status.SetFailed("Unable to convert IP address host to string");
    return;
  }

  // Get port string
  String port = PortToString(ipAddress.GetPort());
  if(port == String()) // Unable?
  {
    status.SetFailed("Unable to convert IP address host to string");
    return;
  }

  // Concatenate host and port strings
  StringBuilder builder;
  builder.Append(host);
  builder.Append(":");
  builder.Append(port);

  // Update IP address string
  ipAddress.mHostPortString = builder.ToString();
}

IpAddress::IpAddress()
  : SocketAddress(),
    mHostPortString()
{
}

IpAddress::IpAddress(Status& status, StringParam host, uint port, InternetProtocol::Enum internetProtocol)
  : SocketAddress(),
    mHostPortString()
{
  SetHost(status, host, internetProtocol);
  if(status.Succeeded())
    SetPort(status, port);
}
IpAddress::IpAddress(Status& status, StringParam host, uint port)
  : SocketAddress(),
    mHostPortString()
{
  SetHost(status, host, InternetProtocol::Unspecified);
  if(status.Succeeded())
    SetPort(status, port);
}
IpAddress::IpAddress(StringParam host, uint port, InternetProtocol::Enum internetProtocol)
  : SocketAddress(),
    mHostPortString()
{
  Status status;
  SetHost(status, host, internetProtocol);
  if(status.Succeeded())
    SetPort(status, port);
  ErrorIf(status.Failed());
}
IpAddress::IpAddress(StringParam host, uint port)
  : SocketAddress(),
    mHostPortString()
{
  Status status;
  SetHost(status, host, InternetProtocol::Unspecified);
  if(status.Succeeded())
    SetPort(status, port);
  ErrorIf(status.Failed());
}

IpAddress::IpAddress(const IpAddress& rhs)
  : SocketAddress(rhs),
    mHostPortString(rhs.mHostPortString)
{
}
IpAddress::IpAddress(const SocketAddress& rhs)
  : SocketAddress(),
    mHostPortString()
{
  *this = rhs;
}

IpAddress::IpAddress(MoveReference<IpAddress> rhs)
  : SocketAddress(*rhs),
    mHostPortString(rhs->mHostPortString)
{
}

IpAddress& IpAddress::operator =(const IpAddress& rhs)
{
  SocketAddress::operator=(rhs);
  mHostPortString        = rhs.mHostPortString;

  return *this;
}
IpAddress& IpAddress::operator =(const SocketAddress& rhs)
{
  SocketAddress::operator=(rhs);

  if(IsValid())
  {
    Status status;
    UpdateHostPortString(status, *this);
    ErrorIf(status.Failed());
  }
  else if(!SocketAddress::IsEmpty())
  {
    Error("Not an IPv4 or IPv6 socket address");
    Clear();
  }

  return *this;
}

bool IpAddress::IsValid() const
{
  return !SocketAddress::IsEmpty()
  && (SocketAddress::GetAddressFamily() == SocketAddressFamily::InternetworkV4
   || SocketAddress::GetAddressFamily() == SocketAddressFamily::InternetworkV6);
}

InternetProtocol::Enum IpAddress::GetInternetProtocol() const
{
  switch(SocketAddress::GetAddressFamily())
  {
  case SocketAddressFamily::InternetworkV4:
    return InternetProtocol::V4;
  case SocketAddressFamily::InternetworkV6:
    return InternetProtocol::V6;
  default:
    return InternetProtocol::Unspecified;
  }
}

const String& IpAddress::GetString() const
{
  return mHostPortString;
}
size_t IpAddress::Hash() const
{
  return mHostPortString.Hash();
}

void IpAddress::SetHost(Status& status, StringParam host, InternetProtocol::Enum internetProtocol)
{
  uint port = GetPort();

  switch(internetProtocol)
  {
  case InternetProtocol::V4:
    SocketAddress::SetIpv4(status, host, port);
    break;

  case InternetProtocol::V6:
    SocketAddress::SetIpv6(status, host, port);
    break;

  default:
  case InternetProtocol::Unspecified:
    SocketAddress::SetIpv4(status, host, port);
    if(status.Failed())
    {
      status = Status();
      SocketAddress::SetIpv6(status, host, port);
    }
    break;
  }

  if(IsValid())
    UpdateHostPortString(status, *this);
}
void IpAddress::SetHost(Status& status, StringParam host)
{
  SetHost(status, host, InternetProtocol::Unspecified);
}
void IpAddress::SetHost(StringParam host, InternetProtocol::Enum internetProtocol)
{
  Status status;
  SetHost(status, host, internetProtocol);
  ErrorIf(status.Failed());
}
void IpAddress::SetHost(StringParam host)
{
  Status status;
  SetHost(status, host, InternetProtocol::Unspecified);
  ErrorIf(status.Failed());
}
String IpAddress::GetHost() const
{
  if(!IsValid())
    return String();


  //mHostPortString.FindLastOf
  StringRange found = mHostPortString.FindLastOf(":");
  return mHostPortString.SubString(mHostPortString.Begin(), found.Begin());
}

void IpAddress::SetPort(Status& status, uint port)
{
  SocketAddress::SetIpPort(status, port);

  if(IsValid())
    UpdateHostPortString(status, *this);
}
void IpAddress::SetPort(uint port)
{
  Status status;
  SetPort(status, port);
  ErrorIf(status.Failed());
}
uint IpAddress::GetPort() const
{
  if(!IsValid())
    return 0;

  Status status;
  uint result = SocketAddress::GetIpPort(status);
  ErrorIf(status.Failed());
  return result;
}
String IpAddress::GetPortString() const
{
  if(!IsValid())
    return String();

  StringRange separator = mHostPortString.FindLastOf(":");
  return mHostPortString.SubString(separator.End(), mHostPortString.End());
}

void IpAddress::Clear()
{
  SocketAddress::Clear();
  mHostPortString.Clear();
}

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, IpAddress& ipAddress)
{
  // Serialize socket address
  Bits result = Serialize(direction, bitStream, static_cast<SocketAddress&>(ipAddress));

  // Read operation?
  if(direction == SerializeDirection::Read)
  {
    // Update internal host port string
    if(ipAddress.IsValid())
    {
      Status status;
      UpdateHostPortString(status, ipAddress);
      if(status.Failed()) // Unable?
      {
        ipAddress.Clear();
        return 0;
      }
    }
    else
    {
      ipAddress.Clear();
      return 0;
    }
  }

  // Success
  return result;
}

} // namespace Zero
