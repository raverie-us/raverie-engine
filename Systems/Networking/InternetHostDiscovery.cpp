///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

InternetHostDiscovery::InternetHostDiscovery(NetPeer* netPeer)
  : NetDiscoveryInterface(netPeer),
    mCurrentMasterServerIndex(0),
    mMasterServerConnectionIp(),
    mInternetHostListTimer(0)
{
}

//
//  NetPeer Message Interface
//

bool InternetHostDiscovery::ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage)
{
  return NetDiscoveryInterface::ReceivePeerMessage(theirIpAddress, peerMessage);
}
bool InternetHostDiscovery::ReceiveLinkMessage(IpAddress const& theirIpAddress, Message& linkMessage)
{
  switch(linkMessage.GetType())
  {
    // Master server has finished sending us a net host record list.
    case NetPeerMessageType::NetHostRecordList:
    {
      ReceiveNetHostRecordList(theirIpAddress, linkMessage);
      return true;
    }
    break;

    default:
    return false;
    break;
  }
}

//
//  NetPeer connection interface
//

void InternetHostDiscovery::HandleNetPeerSentConnectResponse(NetPeerSentConnectResponse* event)
{
}

void InternetHostDiscovery::HandleNetPeerReceivedConnectResponse(NetPeerReceivedConnectResponse* event)
{
  //Clients and servers check to see if this is correspondence from a master server.
  if ( mNetPeer->IsSubscribedMasterServer(event->mTheirIpAddress) )
  {
    //if we handle this event internally, then we also terminate it so the user doesn't see it.
    event->Terminate();

    //are we actively trying to connect to a master server for a host list?
    if (mDiscoveryMode == NetDiscoveryMode::RefreshList)
    {
      if (event->mTheirConnectResponse == ConnectResponse::Accept)
      {
        //Accept the master server IP so we can retrieve the link.
        mMasterServerConnectionIp = event->mTheirIpAddress;
        //At this point, the client peer is waiting for a message on the peer link to move host discovery forward.
      }
      else // Link was denied for one reason or another, so Clear master server IP, and try the next one!
      {
        mMasterServerConnectionIp = IpAddress();
        TryMasterServerConnection();
      }
    }
    else //if not discovering or refreshing, then it must have been canceled.
    {
      //Clear the master server IP connection,  and disconnect our link to it (if we have it)
      mMasterServerConnectionIp = IpAddress();
      mNetPeer->DisconnectLink(event->mTheirIpAddress);
    }
  }
}

void InternetHostDiscovery::HandleNetPeerSentConnectRequest(NetPeerSentConnectRequest* event)
{
  //Clients and Servers will terminate connection events regarding master servers.
  TerminateInternalEvent(event->mTheirIpAddress, event);
}

void InternetHostDiscovery::HandleNetPeerReceivedConnectRequest(NetPeerReceivedConnectRequest* event)
{
}

void InternetHostDiscovery::HandleNetLinkConnected(NetLinkConnected* event)
{
  //Clients and Servers will terminate connection events to master server.
  if (mNetPeer->IsClientOrServer())
  {
    TerminateInternalEvent(event->mTheirIpAddress, event);
  }
}

void InternetHostDiscovery::HandleNetLinkDisconnected(NetLinkDisconnected* event)
{
  //TODO, do this only if it relates to us.
  if (mNetPeer->IsClientOrServer())
  {
    TerminateInternalEvent(event->mTheirIpAddress, event);

    //is this my master server connection???
    if (event->mTheirIpAddress == mMasterServerConnectionIp)
    {
      Assert(mDiscoveryMode == NetDiscoveryMode::RefreshList);

      //if the disconnect was not closed down purposely.
      if (event->mDisconnectReason != DisconnectReason::Request)
      {
        //presume something happened which caused the master server to fail to transmit its host list message.
        CancelRefreshes();
      }
      else
      {
        //if disconnected on a request, it means the master server should have finished sending the host list.
      }
    }
  }
}

//
//  NetDiscoveryInterface
//

void InternetHostDiscovery::RefreshAll(bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts)
{
  CancelIfNotIdle();

  mDiscoveryMode = NetDiscoveryMode::RefreshList;

  CreateMultiHostRequest(Network::Internet, allowDiscovery, removeStaleHosts, getExtraHostInfo);

  mCurrentMasterServerIndex = 0;
  mMasterServerConnectionIp = IpAddress();
  TryMasterServerConnection();

  //TODO: Start the timer!
  mInternetHostListTimer = 0.0f;

}

void InternetHostDiscovery::SingleHostRefresh(IpAddress const& thierIp, bool allowDiscovery, bool getExtraHostInfo, bool removeStaleHosts)
{
  CancelIfRefreshingList();

  mDiscoveryMode = NetDiscoveryMode::Refresh;

  SingleHostRequest* hostRequest = CreateSingleHostRequest(Network::Internet, allowDiscovery, thierIp, removeStaleHosts, getExtraHostInfo);


  // Can we possibly get a response with these parameters? (single refresh on not previously known host without discovery? no point...)
  if (hostRequest->mPreviouslyKnown == false && allowDiscovery == false)
  {
    // Instantly end the request and return.
    EndSingleRefresh( hostRequest );
    return;
  }

  //First create ping bundle
  EventBundle refreshHostRequestInfo;

  NetRequestHostRefreshData request;
  //Master server will want to know what host we want information on.
  request.mHostIp = thierIp;
  request.mProjectGuid = mNetPeer->GetOurProjectGuid();
  refreshHostRequestInfo.GetBitStream().Write(request);

  // Send off ping
  mPingManager.PingHost(Network::Internet, mNetPeer->mMasterServerSubscriptions, HostPingType::MasterServerRefreshHost, FloatSecondsToTimeMs(mNetPeer->mBasicHostInfoTimeout), refreshHostRequestInfo);
}

void InternetHostDiscovery::HandleCancelSingleHostRequest(SingleHostRequest& singleHostRequest)
{
}

void InternetHostDiscovery::HandleCancelMultiHostRequest(MultiHostRequest& multiHostRequest)
{
  // If we are connected to a master server, end that connection.
  mNetPeer->DisconnectLink(mMasterServerConnectionIp);
  mMasterServerConnectionIp = IpAddress();
  mCurrentMasterServerIndex = 0;
}

//handle different ping callbacks.
bool InternetHostDiscovery::HandlePing(IpAddress const& theirIpAddress, NetHostPingData& netHostPingData)
{
  return false;
}

void InternetHostDiscovery::HandlePingCancelled(PendingHostPing& pendingHostPing)
{
}

void InternetHostDiscovery::HandlePingTimeout(PendingHostPing& pendingHostPing)
{
  // Handle according to host ping type
  switch (pendingHostPing.mHostPingType)
  {
  case HostPingType::RefreshList: // multi host refresh finished. now get extra or flush.
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
      mRespondingHostData.Clear();
      mDiscoveryMode = NetDiscoveryMode::Idle; //set this back to idle.
    }
  }
  break;

  case HostPingType::Refresh: // single host refresh timed out. now get extra or flush.
  {
    SingleHostRequest* hostRequest = GetSingleHostRequest(pendingHostPing.mTheirIpAddresses[0]);
    Assert(hostRequest != nullptr);

    if (hostRequest->mAquireExtraHostInfo)
    {
      hostRequest->BeginExtraHostInfo();
    }
    else
    {
      EndSingleRefresh( hostRequest );
    }
  }
  break;

  case HostPingType::MasterServerRefreshHost: // Master server never responded. refresh failed to respond.
  {
    // First find the IP address we intended to get the information on.      
    // Read in the bitstream
    NetRequestHostRefreshData requestRefreshData;
    pendingHostPing.mPingBundle.GetBitStream().Read(requestRefreshData);
    IpAddress pingedHostIp = requestRefreshData.mHostIp;

    SingleHostRequest* hostRequest = GetSingleHostRequest(pingedHostIp);
    Assert(hostRequest != nullptr);

    if (hostRequest->mAquireExtraHostInfo)
    {
      hostRequest->BeginExtraHostInfo();
    }
    else
    {
      EndSingleRefresh(hostRequest);
    }
  }
  break;

  default: Assert(false); break;
  }
}

void InternetHostDiscovery::HandlePong(IpAddress const& theirIpAddress, NetHostPongData& netHostPongData, PendingHostPing& pendingHostPing)
{
  // if ping from a game server:
  // read out ping data save it. update level of refresh.
  // if its a refresh (individual refresh) then we should also remove pending ping.

  IpAddress pingedHost = PongHelper(theirIpAddress, netHostPongData, pendingHostPing);

  // if we didn't actually handle it or early outed, we don't do additional logic.
  if (pingedHost == IpAddress())
    return; 

  OpenHostRequest* hostRequest = nullptr;

  if (mDiscoveryMode == NetDiscoveryMode::RefreshList)
    hostRequest = GetMultiHostRequest();
  else
    hostRequest = GetSingleHostRequest(pingedHost);


  // check for first time response.
  bool isFirstResponse = hostRequest->GetIsFirstResponseFrom(pingedHost);
  // set it so that it is no longer a first time response.
  hostRequest->SetIsFirstResponseFrom(pingedHost, false);


  switch (pendingHostPing.mHostPingType)
  {
    case HostPingType::Refresh: //happens under single host refresh case
    {
      // this should be a non-first response to overall host discovery.
      Assert(!isFirstResponse);
      // we got direct info from single host refresh.
      if (hostRequest->mAquireExtraHostInfo)
      {
        // start process of getting extra host info.
        hostRequest->BeginExtraHostInfo();
      }
      else
      {
        // dispatch host immediately.
        EndSingleRefresh( reinterpret_cast<SingleHostRequest*>(hostRequest) );
      }
    }
    break;

    case HostPingType::RefreshList: // happens under multi host refresh case
    {
      // This should be a non-first response to overall host discovery.
      Assert(!isFirstResponse);
      // Do we NOT want extra host info?
      if (!hostRequest->mAquireExtraHostInfo)
      {
        hostRequest->FlushHost(*mNetPeer, *this, pingedHost);
        mRespondingHostData.EraseValue(pingedHost);
      }

    }
    break;

    case HostPingType::MasterServerRefreshHost: //happens under single host refresh case.
    {
      // We got indirect host info back from master server.
      if (isFirstResponse)
      {
        mPingManager.PingHost(Network::Internet, pingedHost, HostPingType::Refresh, FloatSecondsToTimeMs( mNetPeer->GetBasicHostInfoTimeout()), EventBundle());
      }
    }
    break;

    // Shouldn't ever do this.
    default: Assert(false);
  }
}

void InternetHostDiscovery::OnEngineUpdate(UpdateEvent * event)
{
  mInternetHostListTimer += event->Dt;

  // Are we trying to refresh an entire list? (on the internet)
  if (mDiscoveryMode == NetDiscoveryMode::RefreshList)
  {
    MultiHostRequest* hostRequest = GetMultiHostRequest();

    //Is this host request still in the unresponding stage? (this means it hasn't retrieved a host list yet)
    if (hostRequest->mDiscoveryStage == NetDiscoveryStage::Unresponding)
    {
      // has the time since the start of the host list request timed out? this including connection attempts to different master servers.
      if (mInternetHostListTimer > mNetPeer->GetInternetHostListTimeout())
      {
        // it has timed out in this special case. it means we need to cancel further refreshing and dispatch what we got.
        CancelRefreshes();
      }
    }
  }

}

void InternetHostDiscovery::CleanUp()
{
}

//
//  InternetDiscovery Specific Implementation
//

void InternetHostDiscovery::ReceiveNetHostRecordList(IpAddress const& theirIpAddress, Zero::Message const& message)
{
  Assert( mNetPeer ); //should have non-null net peer

  //Are we in the discover mode? If not, we likely canceled.
  if (mDiscoveryMode != NetDiscoveryMode::RefreshList)
  {
    // should not ever by in this mode here.
    Assert(mDiscoveryMode != NetDiscoveryMode::Refresh);

    //in the case that we canceled, just end the link and return. no further processing.
    mNetPeer->DisconnectLink(theirIpAddress);
    return;
  }

  // a multi host request likely triggered this function, so we get that request for further processing.
  MultiHostRequest* hostRequest = GetMultiHostRequest();
  // if we got this, then we should still currently be in the unresponding stage.
  Assert( hostRequest->mDiscoveryStage == NetDiscoveryStage::Unresponding );

  //update request discovery stage.
  hostRequest->mDiscoveryStage = NetDiscoveryStage::BasicHostInfo;

  // Read network user add response message
  NetHostRecordListData netHostRecordList;
  if (!message.GetData().Read(netHostRecordList)) // Unable?
  {
    // Failed to read host data. Cancel the multi host request.
    CancelRefreshes();
    DoNotifyWarning("Error Reading Net Message","Unable to read a NetHostRecordList from master server.");
    return;
  }

  // Get owner as game session
  GameSession* owner = static_cast<GameSession*>(mNetPeer->GetOwner());

  // Process All NetHostRecords into RespondingHostData.
  forRange(NetHostRecord& record, netHostRecordList.mNetHostRecords.All())
  {
    RespondingHostData* respondingHostData = mRespondingHostData.FindPointer(record.mIpAddress);

    if ( respondingHostData == nullptr )
    {
      if(hostRequest->mAllowDiscovery)
      {
        mRespondingHostData.Insert(record.mIpAddress, RespondingHostData());
        respondingHostData = mRespondingHostData.FindPointer(record.mIpAddress);
        Assert(respondingHostData != nullptr);
      }
      else
      {
        continue; // we do not allow discovery of new hosts, so skip this host record.
      }
    }
    // put them in the request as a responding host.
    hostRequest->mRespondingHosts.InsertOrAssign(record.mIpAddress);

    respondingHostData->mBasicHostInfo = ZeroMove(record.mBasicHostInfo.GetBitStream());
    respondingHostData->mRoundTripTime = 0;
    respondingHostData->mRefreshResult = NetRefreshResult::IndirectBasicHostInfo;
  }

  // TODO HANDLE PINGS
  // Ping hosts who responded.
  mPingManager.PingHost(Network::Internet, hostRequest->mRespondingHosts, HostPingType::RefreshList, FloatSecondsToTimeMs(mNetPeer->GetBasicHostInfoTimeout()), EventBundle() );
  // Now we wait till this ping expires. When it does we check if they want extra host info or not.
  // if they don't then the event is dispatched.

  //force disconnection from MasterServer on client end.
  mNetPeer->DisconnectLink(theirIpAddress);
}

bool InternetHostDiscovery::TryMasterServerConnection()
{
  Assert(mNetPeer->IsOpen());
  Assert(mNetPeer->IsClientOrServer());

  //if we are attempting a refresh or a host discovery.
  if (mDiscoveryMode == NetDiscoveryMode::RefreshList)
  {
    //if we have previously attempted a connection to the last master server
    if (mCurrentMasterServerIndex == mNetPeer->mMasterServerSubscriptions.Size())
    {
      //Failed to discover internet hosts. Attempted connections with All master servers, and none succeeded.
      CancelRefreshes();

      return false;
    }

    //Attempt to send a connection request!
    return mNetPeer->ConnectLink(mNetPeer->mMasterServerSubscriptions[mCurrentMasterServerIndex++]);
  }

  return false;
}

} // namespace Zero
