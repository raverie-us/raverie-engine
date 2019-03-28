// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class TcpSocket;

class SimpleSocket : public Component
{
public:
  ZilchDeclareType(SimpleSocket, TypeCopyMode::ReferenceType);

  /// Constructor.
  SimpleSocket();

  /// Returns the socket.
  TcpSocket* GetSocket();

private:
  /// Socket.
  TcpSocket mSocket;
};

} // namespace Zero
