// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(NetPeerConnectionInterface, builder, type)
{
}

NetPeerConnectionInterface::NetPeerConnectionInterface(NetPeer* netPeer) : mNetPeer(netPeer), mReceiver()
{
}

void NetPeerConnectionInterface::InitializeEventConnections(NetPeer* netPeer)
{
  Assert(netPeer != nullptr);
  mNetPeer = netPeer;

  ConnectThisTo(netPeer, Events::NetPeerSentConnectResponse, HandleNetPeerSentConnectResponse);
  ConnectThisTo(netPeer, Events::NetPeerReceivedConnectResponse, HandleNetPeerReceivedConnectResponse);
  ConnectThisTo(netPeer, Events::NetPeerSentConnectRequest, HandleNetPeerSentConnectRequest);
  ConnectThisTo(netPeer, Events::NetPeerReceivedConnectRequest, HandleNetPeerReceivedConnectRequest);
  ConnectThisTo(netPeer, Events::NetLinkConnected, HandleNetLinkConnected);
  ConnectThisTo(netPeer, Events::NetLinkDisconnected, HandleNetLinkDisconnected);
}

EventReceiver* NetPeerConnectionInterface::GetReceiver()
{
  return &mReceiver;
}

} // namespace Raverie
