///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleSocket.cpp
/// Implementation of the SimpleSocket class.
///
/// Authors: Joshua Claeys.
/// Copyright 2012, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SimpleSocket, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterProperty(Socket);
}

SimpleSocket::SimpleSocket() 
  : mSocket(Protocol::Chunks | Protocol::Events, "SimpleSocket")
{
}

TcpSocket* SimpleSocket::GetSocket()
{
  return &mSocket;
}

} // namespace Zero
