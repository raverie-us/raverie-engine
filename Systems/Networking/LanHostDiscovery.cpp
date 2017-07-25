///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

LanHostDiscovery::LanHostDiscovery(NetPeer* netPeer)
  : NetDiscoveryInterface(netPeer)
{
}

//
//  NetPeer Message Interface
//

bool LanHostDiscovery::ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage)
{
  return NetDiscoveryInterface::ReceivePeerMessage(theirIpAddress, peerMessage);
}
bool LanHostDiscovery::ReceiveLinkMessage(IpAddress const& theirIpAddress, Message& linkMessage)
{
  return false;
}

//
//  NetPeer connection interface
//

void LanHostDiscovery::HandleNetPeerSentConnectResponse(NetPeerSentConnectResponse* event)
{
  return; // LAN discovery currently not using connections.
}

void LanHostDiscovery::HandleNetPeerReceivedConnectResponse(NetPeerReceivedConnectResponse* event)
{
  return; // LAN discovery currently not using connections.  
}

void LanHostDiscovery::HandleNetPeerSentConnectRequest(NetPeerSentConnectRequest* event)
{
  return; //LAN discovery currently does not deal with connections.

  //TODO: Only cancel if we caused.
  //Clients and Servers will terminate connection events to master server.
  //TerminateInternalEvent(event->mTheirIpAddress, event);
}

void LanHostDiscovery::HandleNetPeerReceivedConnectRequest(NetPeerReceivedConnectRequest* event)
{
  return; //LAN discovery does not currently handle LAN connections.
}

void LanHostDiscovery::HandleNetLinkConnected(NetLinkConnected* event)
{
  return; // LanHostDiscovery does not currently use connections.
  //TODO: only cancel if we caused.
  //Clients and Servers will terminate connection events to master server.
  //TerminateInternalEvent(event->mTheirIpAddress, event);
}

void LanHostDiscovery::HandleNetLinkDisconnected(NetLinkDisconnected* event)
{
  return; // LAN Host Discovery does not currently deal with connections.
  /*

  //TODO, do this only if it relates to us. terminate, only if it relates to use.
  if (mNetPeer->IsClientOrServer())
  {
    TerminateInternalEvent(event->mTheirIpAddress, event);

    //is this my master server connection???
    if (event->mTheirIpAddress == mMasterServerConnectionIp)
    {
      //if the disconnect was not closed down purposely.
      if (event->mDisconnectReason != DisconnectReason::Request)
      {
        //presume something happened which caused the master server to fail to transmit its message (host list).
        HandleFailedInternetHostList();
      }
    }

  }

  */
}

//
//  NetDiscoveryInterface
//

void LanHostDiscovery::RefreshAll(bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts)
{
  CancelIfNotIdle();

  mDiscoveryMode = NetDiscoveryMode::RefreshList;

  MultiHostRequest* hostRequest = CreateMultiHostRequest(Network::LAN, allowDiscovery, removeStaleHosts, getExtraHostInfo);

  TimeMs timeout = FloatSecondsToTimeMs(mNetPeer->GetBasicHostInfoTimeout());
  // Create a ping which is sent to everyone on the network.
  if(allowDiscovery)
  {
    mPingManager.PingHost(Network::LAN, IpAddress("255.255.255.255", 0), HostPingType::DiscoverList, timeout, EventBundle());
  }
  else // create a ping which is sent only to people we expect responses from.
  {
    mPingManager.PingHost(Network::LAN, hostRequest->mExpectedHosts, HostPingType::RefreshList, timeout, EventBundle());
  }
}

void LanHostDiscovery::SingleHostRefresh(IpAddress const & theirIp, bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts)
{
  CancelIfRefreshingList();

  mDiscoveryMode = NetDiscoveryMode::Refresh;

  SingleHostRequest* hostRequest = CreateSingleHostRequest(Network::LAN, allowDiscovery, theirIp, removeStaleHosts, getExtraHostInfo);

  TimeMs timeout = FloatSecondsToTimeMs(mNetPeer->GetBasicHostInfoTimeout());

  mPingManager.PingHost(Network::LAN, theirIp, HostPingType::Refresh, timeout, EventBundle());
}

void LanHostDiscovery::HandleCancelSingleHostRequest(SingleHostRequest & singleHostRequest)
{
  // not sure this will do anything ever.
}

void LanHostDiscovery::HandleCancelMultiHostRequest(MultiHostRequest & multiHostRequest)
{
  // not sure this will do anything ever.
  // pre-handle a multi host request before it is canceled. What did the discovery interface do that it might need to clean up?
}

bool LanHostDiscovery::HandlePing(IpAddress const& theirIpAddress, NetHostPingData& netHostPingData)
{
  return false; //Discovery doesn't actually handle the pings. Only the pongs.
}

void LanHostDiscovery::HandlePingCancelled(PendingHostPing& pendingHostPing)
{
  //This never actually happens unless ping is called multiple times in a row. 
  Assert(false);
}

void LanHostDiscovery::HandlePingTimeout(PendingHostPing& pendingHostPing)
{
  // Handle according to host ping type
  switch (pendingHostPing.mHostPingType)
  {
    //allow no more responses.
    case HostPingType::DiscoverList:
    case HostPingType::RefreshList:
    {
      MultiHostRequest* hostRequest = GetMultiHostRequest();
      if (hostRequest->mAquireExtraHostInfo)
      {
        hostRequest->BeginExtraHostInfo();
      }
      else
      {
        hostRequest->FlushHostRequest(*mNetPeer, *this);  // dispatch events, create net hosts, clean up stale hosts. dispatch host list.
        mOpenHostRequests.Clear();        // Clear out hosts requests. (it is finished)
        mRespondingHostData.Clear();      // Clear out responding host data (we are done with it)
        Assert(mSingleHostRequests.Size() == 0); //shouldn't need to clean this, because it should be empty.
      }
    }
    break;

    //no response 
    case HostPingType::Refresh:
    {
      SingleHostRequest* hostRequest = GetSingleHostRequest(pendingHostPing.mTheirIpAddresses[0]);
      Assert(hostRequest != nullptr);

      if (hostRequest->mAquireExtraHostInfo)
      {
        hostRequest->BeginExtraHostInfo();
      }
      else
      {
        hostRequest->FlushHostRequest(*mNetPeer, *this);    //dispatches event. Creates net hosts.
        mSingleHostRequests.Erase(hostRequest->mIpAddress); //removes map of IP address to single host request.
        mOpenHostRequests.EraseValue(hostRequest);         //cleans up individual host refresh request.
      }

    }
    break;

    default:
    case HostPingType::MasterServerRefreshHost: //shouldn't be receiving master server host refreshes on LAN...
    Assert(false);
    break;
  }
}

void LanHostDiscovery::HandlePong(IpAddress const& theirIpAddress, NetHostPongData& netHostPongData, PendingHostPing& pendingHostPing)
{
  // if ping from a game server:
  // read out ping data save it. update level of refresh.
  // if its a refresh (individual refresh) then we should also remove pending ping.

  // PongHelper returns IP address of pinged host. It also processes and updates responding host data.
  IpAddress pingedHost = PongHelper(theirIpAddress, netHostPongData, pendingHostPing);

  // Was PongHelper early outed?
  if(pingedHost == IpAddress()) return; 

  OpenHostRequest* hostRequest = nullptr;

  // Grab the host request this IP address is related to.
  if(mDiscoveryMode == NetDiscoveryMode::RefreshList)
    hostRequest = GetMultiHostRequest();
  else
    hostRequest = GetSingleHostRequest(pingedHost);

  // check for first time response.
  bool isFirstResponse = hostRequest->GetIsFirstResponseFrom(pingedHost);
  // set it so that it is no longer a first time response.
  hostRequest->SetIsFirstResponseFrom(pingedHost, false);

  // first time response?
  if(!isFirstResponse) return; //we only want to handle events and additional actions on the first response.

  // check if extra host info was requested.
  if (hostRequest->mAquireExtraHostInfo)
  {
    // do extraHostInfo
    if (mDiscoveryMode == NetDiscoveryMode::Refresh)
    {
      hostRequest->BeginExtraHostInfo();
    }
  }
  else // no further information is required. dispatch event!
  {
    if (mDiscoveryMode == NetDiscoveryMode::Refresh) //refresh events are finished at this point.
    {
      hostRequest->FlushHostRequest(*mNetPeer, *this);          // dispatches event. Creates net hosts.
      mSingleHostRequests.Erase(theirIpAddress);                // removes map of IP address to single host request.
      mOpenHostRequests.EraseValue(hostRequest);                // cleans up individual host refresh request.
        
      //if there are no more requests, then we are idle again.
      if (mOpenHostRequests.Empty())
      {
        mDiscoveryMode = NetDiscoveryMode::Idle;
      }

    }
    else // mDiscoveryMode == NetDiscoveryMode::RefreshList
    {
      // Here we dispatch a single host from the from a multi host request. then we have to remove the responding host data struct so that it does not get dispatched again.
      hostRequest->FlushHost(*mNetPeer, *this, theirIpAddress); //dispatches the host in the host request. Must remove the RespondingHostData.
    }

    // remove them from responding hosts since they are finished and dispatched. (multi host requests will then not dispatch them later when it iterates through on timeout)
    mRespondingHostData.EraseValue(theirIpAddress);
  }
}

void LanHostDiscovery::OnEngineUpdate(UpdateEvent * event)
{
}

void LanHostDiscovery::CleanUp()
{
}

} // namespace Zero
