// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SimpleSocket, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterProperty(Socket);
}

SimpleSocket::SimpleSocket() : mSocket(Protocol::Chunks | Protocol::Events, "SimpleSocket")
{
}

TcpSocket* SimpleSocket::GetSocket()
{
  return &mSocket;
}

} // namespace Zero
