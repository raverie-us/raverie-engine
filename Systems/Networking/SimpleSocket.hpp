///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleSocket.hpp
/// Declaration of the SimpleSocket class.
///
/// Authors: Joshua Claeys.
/// Copyright 2012, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TcpSocket;

class SimpleSocket : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  SimpleSocket();

  /// Returns the socket.
  TcpSocket* GetSocket();

private:

  /// Socket.
  TcpSocket mSocket;
};

}//namespace Zero
