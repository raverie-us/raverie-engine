// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(SimpleSocket, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);

  RaverieBindGetterProperty(Socket);
}

SimpleSocket::SimpleSocket() : mSocket(Protocol::Chunks | Protocol::Events, "SimpleSocket")
{
}

TcpSocket* SimpleSocket::GetSocket()
{
  return &mSocket;
}

} // namespace Raverie
