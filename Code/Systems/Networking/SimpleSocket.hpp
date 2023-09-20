// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TcpSocket;

class SimpleSocket : public Component
{
public:
  RaverieDeclareType(SimpleSocket, TypeCopyMode::ReferenceType);

  /// Constructor.
  SimpleSocket();

  /// Returns the socket.
  TcpSocket* GetSocket();

private:
  /// Socket.
  TcpSocket mSocket;
};

} // namespace Raverie
