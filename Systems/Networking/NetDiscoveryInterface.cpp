///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//
// Responding host data constructors
//
  RespondingHostData::RespondingHostData()
    : mRoundTripTime(0),
      mBasicHostInfo(),
      mExtraHostInfo(),
      mRefreshResult()
  {
  }

  RespondingHostData::RespondingHostData(RespondingHostData const& rhs)
    : mRoundTripTime(rhs.mRoundTripTime),
      mBasicHostInfo(rhs.mBasicHostInfo),
      mExtraHostInfo(rhs.mExtraHostInfo),
      mRefreshResult(rhs.mRefreshResult)
  {
  }

  RespondingHostData::RespondingHostData(MoveReference<RespondingHostData> rhs)
    : mRoundTripTime(rhs->mRoundTripTime),
      mBasicHostInfo(ZeroMove(rhs->mBasicHostInfo)),
      mExtraHostInfo(ZeroMove(rhs->mExtraHostInfo)),
      mRefreshResult(rhs->mRefreshResult)
  {
  }

  void RespondingHostData::UpdateToBasic(bool isIndirect)
  {
    // Is this an indirect basic update or direct?
    if (isIndirect)
    {
      // Update the discovery stage of this host
      if (mRefreshResult < NetRefreshResult::IndirectBasicHostInfo)
      {
        mRefreshResult = NetRefreshResult::IndirectBasicHostInfo;
      }
    }
    else
    {
      // Update the discovery stage of this host
      if (mRefreshResult < NetRefreshResult::DirectBasicHostInfo)
      {
        mRefreshResult = NetRefreshResult::DirectBasicHostInfo;
      }
    }
  }

  //
  //  Net Discovery Interface
  //

  NetDiscoveryInterface::NetDiscoveryInterface(NetPeer* netPeer)
  : NetPeerConnectionInterface(netPeer),
    mDiscoveryMode(NetDiscoveryMode::Idle),
    mPingManager(netPeer)
  {
  }

  void NetDiscoveryInterface::Update(UpdateEvent * event)
  {
    // Update pings
    mPingManager.UpdatePendingPings();

    // Do custom logic that Specific implementations of net discovery might have
    OnEngineUpdate(event);
  }

  void NetDiscoveryInterface::CancelRefreshes()
  {
    if(mOpenHostRequests.Size() > 0)
    {
      // We can make some assumptions about the type of refreshes going on based on our discovery mode
      if(mDiscoveryMode == NetDiscoveryMode::Refresh)
      {
        // Go through each request and cancel it
        for (unsigned i = 0; i < mOpenHostRequests.Size(); i += 1)
        {
          OpenHostRequest& request = *mOpenHostRequests[i];
          HandleCancelSingleHostRequest( reinterpret_cast<SingleHostRequest&>(request) );
          request.FlushHostRequest( *mNetPeer, *this );
        }
      }
      else if (mDiscoveryMode == NetDiscoveryMode::RefreshList)
      {
        MultiHostRequest& multiHostRequest = reinterpret_cast<MultiHostRequest&>(*mOpenHostRequests[0]);
        HandleCancelMultiHostRequest(multiHostRequest);
        multiHostRequest.FlushHostRequest(*mNetPeer, *this);
      }
    }

    // Reset discovery mode to idle (we canceled refreshes)
    mDiscoveryMode = NetDiscoveryMode::Idle;
    // Remove All requests.
    mOpenHostRequests.Clear();
    // Remove All responding host data.
    mRespondingHostData.Clear();

    //if any there still any pings going on, we clear them.
    mPingManager.mPendingHostPings.Clear();
  }

  void NetDiscoveryInterface::CancelIfNotIdle()
  {
    if(mDiscoveryMode != NetDiscoveryMode::Idle) CancelRefreshes();
  }

  void NetDiscoveryInterface::CancelIfRefreshingList()
  {
    if (mDiscoveryMode != NetDiscoveryMode::RefreshList) CancelRefreshes();
  }

  bool NetDiscoveryInterface::ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage)
  {
    if(mPingManager.ReceivePeerMessage(theirIpAddress, peerMessage)) return true;

    return false;
  }

  void NetDiscoveryInterface::SetPingManagerCallbacks()
  {
  //these two commented out lines were experiments on what it could figure out without filling out template.
    //auto thing = CreateCallback(&NetDiscoveryInterface::HandlePing, this);
    //mPingManager.mPingCallback = CreateCallback<void, NetDiscoveryInterface, IpAddress const&, NetHostPingData&>( &NetDiscoveryInterface::HandlePing, this);

    mPingManager.SetPingCallback( CreateCallback(&NetDiscoveryInterface::HandlePing, this) );
    mPingManager.SetPingCancelledCallback( CreateCallback(&NetDiscoveryInterface::HandlePingCancelled, this) );
    mPingManager.SetPingTimeoutCallback( CreateCallback(&NetDiscoveryInterface::HandlePingTimeout, this) );
    mPingManager.SetPongCallback( CreateCallback(&NetDiscoveryInterface::HandlePong, this) );

    mPingManager.mPingInterval = mNetPeer->mHostPingInterval;
  }

  void NetDiscoveryInterface::Initialize()
  {
    InitializeEventConnections(mNetPeer);
    SetPingManagerCallbacks();
  }

  SingleHostRequest* NetDiscoveryInterface::CreateSingleHostRequest(Network::Enum network, bool allowDiscovery, IpAddress const & theirIpAddress, bool removeStaleHosts, bool extraHostInfo)
  {
    //TODO: Avoid duplicate single host requests. Cancel the duplicate refresh, start again.
    
    // Create Request
    SingleHostRequest* newRequest = new SingleHostRequest();
    newRequest->mNetwork = network;
    newRequest->mAllowDiscovery = allowDiscovery;
    newRequest->mIpAddress = theirIpAddress;
    newRequest->mDiscoveryStage = NetDiscoveryStage::Unresponding;
    newRequest->mPreviouslyKnown = mNetPeer->GetHostByAddress(network, theirIpAddress) != nullptr; // did we have this host before we refreshed it?
    newRequest->mRemoveStaleHosts = removeStaleHosts;
    newRequest->mAquireExtraHostInfo = extraHostInfo;

    // add initial response for this host.
    mRespondingHostData.Insert( theirIpAddress, RespondingHostData() );

    // Save Request.
    mOpenHostRequests.PushBack(OpenHostRequestPtr(newRequest));

    //put into single host refresh list.
    mSingleHostRequests[theirIpAddress] = newRequest;

    return newRequest;
  }

  MultiHostRequest* NetDiscoveryInterface::CreateMultiHostRequest(Network::Enum network, bool allowDiscovery, bool removeStaleHosts, bool extraHostInfo)
  {
    // Create Request
    MultiHostRequest* newRequest = new MultiHostRequest();
    newRequest->mNetwork = network;
    newRequest->mAllowDiscovery = allowDiscovery;
    GetExpectedHosts(network, newRequest->mExpectedHosts ); // get the hosts the multi host request is expecting. (if allow discovery is true, anyone can respond)
    newRequest->mDiscoveryStage = NetDiscoveryStage::Unresponding;
    newRequest->mRemoveStaleHosts = removeStaleHosts;
    newRequest->mAquireExtraHostInfo = extraHostInfo;

    //add initial response for this host.
    forRange(IpAddress& hostIp, newRequest->mExpectedHosts.All())
    {
      mRespondingHostData.Insert(hostIp, RespondingHostData());
    }

    //Save Request.
    mOpenHostRequests.PushBack(OpenHostRequestPtr(newRequest));

    return newRequest;
  }

  void NetDiscoveryInterface::GetExpectedHosts(Network::Enum network, ArraySet<IpAddress>& outIpAddressList)
  {
    //Clear expect host list and get it anew from the netpeer.
    Assert(mNetPeer);
    outIpAddressList.Clear();
    outIpAddressList.Reserve(mNetPeer->GetHostList(network).Size());

    //Copy the IP addresses of the hosts that the netpeer has in the host list.
    forRange(NetHost* host, mNetPeer->GetHostList(network))
    {
      outIpAddressList.Insert(host->mIpAddress);
    }

  }

  void NetDiscoveryInterface::DispatchHost(IpAddress const & hostIp, OpenHostRequest& hostRequest)
  {
    NetHostUpdate event;
    RespondingHostData defaultHostData;
    RespondingHostData* hostData = mRespondingHostData.FindPointer(hostIp);
    Assert(hostData != nullptr);
    if(hostData == nullptr)
    {
    // (I don't think this should ever happen. newly discovered hosts should create this. Already known get one by default)
    Assert(false);
      hostData = &defaultHostData;
    }

    event.mNetwork = hostRequest.mNetwork;
    event.mResponseTime = TimeMsToFloatSeconds( hostData->mRoundTripTime );
    event.mRefreshResult = hostData->mRefreshResult;
    
    String eventId;

    bool isNewHost = hostRequest.IsNewHost(hostIp);

    //
    // select event id
    //
    if (isNewHost)
    {
      eventId = Events::NetHostDiscovered;
    }
    else
    {
      eventId = Events::NetHostRefreshed;
    }

    //
    // find NetHost. (do we make it? Does it get put into host list?)
    //
    NetHost* netHost = nullptr;
    NetHostPtr tempNetHost = nullptr;

    if (isNewHost) //not in list, goes in list.
    {
      netHost = mNetPeer->AddOrFindHost(hostRequest.mNetwork, hostIp);
    }
  else if(hostData->mRefreshResult == NetRefreshResult::NoResponse) //not in list, doesn't go in list.
    {
      netHost = new NetHost();
      netHost->mIpAddress = hostIp;
      tempNetHost = netHost; //deletes it when it goes out of scope...
    }
    else // already in list.
    {
      //should already exist.
      netHost = mNetPeer->GetHostByAddress(hostRequest.mNetwork, hostIp);
      Assert(netHost != nullptr);
    }

    //
    // Update nethost data
    //

    netHost->mRoundTripTime = hostData->mRoundTripTime;

    // Get owner as game session
    GameSession* owner = static_cast<GameSession*>(mNetPeer->GetOwner());

  //we do all these if checks so that if they fail to get new host info, we don't clear out the old info
  //on the NetHost. (eg. so if they fail to get the server name or something other, it doesn't clear out the server name any more, or clear the last screenshot or sent)
    if(hostData->mRefreshResult != NetRefreshResult::NoResponse)
    {
      if (!hostData->mBasicHostInfo.IsEmpty())
      {
        netHost->mBasicHostInfo = ZeroMove( hostData->mBasicHostInfo );
      }
      if (!hostData->mExtraHostInfo.IsEmpty())
      {
        netHost->mExtraHostInfo = ZeroMove( hostData->mExtraHostInfo );
      }
    }

    netHost->mBasicHostInfo.SetGameSession(owner);
    netHost->mExtraHostInfo.SetGameSession(owner);
    
    //lastly put nethost into the event.
    event.mHost = netHost;

    //dispatch it!
    mNetPeer->DispatchEvent(eventId, &event);

  }

  MultiHostRequest* NetDiscoveryInterface::GetMultiHostRequest()
  {
    Assert(mDiscoveryMode == NetDiscoveryMode::RefreshList); //should be in refresh list discovery mode.
    Assert(mOpenHostRequests.Size() == 1);  //should have exactly one open host request (a multi host request.)

    return reinterpret_cast<MultiHostRequest*>(mOpenHostRequests[0].mPointer);
  }

  SingleHostRequest * NetDiscoveryInterface::GetSingleHostRequest(IpAddress const & thierIpAddress)
  {
    if (mSingleHostRequests.ContainsKey(thierIpAddress))
    {
      return mSingleHostRequests[thierIpAddress];
    }
    else
    {
      return nullptr;
    }
  }

  void NetDiscoveryInterface::TerminateInternalEvent(IpAddress const& eventIp, Event* event)
  {
    //If an event Contains correspondence from a master server, then a lot of the time
    //we want to terminate the event so the client does not receive unwanted events.
    if ( mNetPeer->IsSubscribedMasterServer(eventIp) )
    {
      event->Terminate();
    }
    //might put other stuff here.
  }

  IpAddress NetDiscoveryInterface::PongHelper(IpAddress const & theirIpAddress, NetHostPongData & netHostPongData, PendingHostPing & pendingHostPing)
  {
  if ( mOpenHostRequests.Size() < 0 )
    return IpAddress(); //we have no open host requests. so ignore pongs.

  if ( !PongIsForThisProject(netHostPongData, pendingHostPing) )
    return IpAddress(); //pong is not from our project.

    TimeMs now = mPingManager.mTimer.UpdateAndGetTimeMilliseconds();

    //Create EventBundle and set its game session so it can deserialize properly.
    EventBundle pongBundle(static_cast<GameSession*>(mNetPeer->GetOwner()));
    pongBundle = ZeroMove(netHostPongData.mEventBundleData);

    //a temporary which talks about the host which we intended to ping (like for example, sometimes we ping the master server, but we want a host which it may contain)
    IpAddress pingedHostIp = theirIpAddress;
    //a variable which indicates if this pong was from a master server as opposed to a host.
    
    bool fromMasterServer = false;
    OpenHostRequest* hostRequest = nullptr;
    if (mDiscoveryMode == NetDiscoveryMode::RefreshList)
    {
      MultiHostRequest* multiHostRequest = GetMultiHostRequest();
      hostRequest = multiHostRequest;
    }
    else if (mDiscoveryMode == NetDiscoveryMode::Refresh)
    {
      //Handle special case where ping came from master server.
      if (pendingHostPing.mHostPingType == HostPingType::MasterServerRefreshHost)
      {
        //we got a ping back from a master server?
        //get the refresh data out of the packet.
        NetHostRefreshData refreshData;
        unsigned int bits_read = pongBundle.GetBitStream().Read(refreshData);
        Assert(bits_read > 0);
        pingedHostIp = refreshData.mHostIp;
      //read out the IP, then here we can just move the info back into the pong.
        pongBundle = ZeroMove(refreshData.mBasicHostInfo);
        //this ping did come from a master server.
        fromMasterServer = true;
      }

    //find the request associated with this IP.
      SingleHostRequest* singleHostRequest = GetSingleHostRequest(pingedHostIp);
    if (singleHostRequest == nullptr)
      return IpAddress(); // pong was fabricated, canceled or expired.
      hostRequest = singleHostRequest;

      hostRequest->mDiscoveryStage = fromMasterServer ? NetDiscoveryStage::BasicHostInfo : NetDiscoveryStage::BasicHostInfoDirect;
    }
    //now actually store the data they delivered!
    RespondingHostData* respondingHostData = mRespondingHostData.FindPointer(pingedHostIp);

    bool isFirstResponse = hostRequest->GetIsFirstResponseFrom(pingedHostIp);

    // check to see if we have some data, or if it needs to be created (and if not, do nothing so discovery does not happen)
    if (respondingHostData == nullptr)
    {
      // Is discovery allowed? And have we already seen another response from this host?
      // If this is not the first response, allow discovery is true, and the respondingHostData is missing, this host
      // may have already been dispatched as refreshed or discovered host.
      if (hostRequest->mAllowDiscovery && isFirstResponse)
      {
        mRespondingHostData.Insert(pingedHostIp, RespondingHostData());
        respondingHostData = mRespondingHostData.FindPointer(pingedHostIp);
        Assert(respondingHostData);
      }
      else
      {
        // we are not allowing discovery of new hosts, so ignore it.
        return IpAddress();
      }
    }
    
    //save the basic host info.
    respondingHostData->mBasicHostInfo = ZeroMove(pongBundle.GetBitStream());
    respondingHostData->UpdateToBasic(fromMasterServer);

    //set round trip time.
    respondingHostData->mRoundTripTime = pendingHostPing.GetDurationSinceSendAttempt(netHostPongData.mSendAttemptId, now);

    //At this point we have updated or added responding host data from the pong.
    return pingedHostIp;
  }

  bool NetDiscoveryInterface::PongIsForThisProject(NetHostPongData const & netHostPongData, PendingHostPing const& pendingHostPing)
  {
  return pendingHostPing.mHostPingType == HostPingType::MasterServerRefreshHost
      || mNetPeer->GetOurProjectGuid() == netHostPongData.mProjectGuid;
  }

  void NetDiscoveryInterface::EndSingleRefresh(SingleHostRequest* hostRequest)
  {
    hostRequest->FlushHostRequest(*mNetPeer, *this);         // Dispatches event. Creates net hosts.
  mSingleHostRequests.Erase(hostRequest->mIpAddress);      // Removes map of IP to single host request.
    mOpenHostRequests.EraseValue(hostRequest);               // Cleans up individual host refresh request.
    mRespondingHostData.EraseValue(hostRequest->mIpAddress); // Clean up responding host data.
  }

  //
  // OpenHostRequest Implementation
  //

  void OpenHostRequest::FlushHost(NetPeer& netPeer, NetDiscoveryInterface& netDiscoveryInstance, IpAddress const& ipAddress)
  {
    //dispatch single host events as needed.
    netDiscoveryInstance.DispatchHost(ipAddress, *this);

    RespondingHostData* data = netDiscoveryInstance.mRespondingHostData.FindPointer(ipAddress);
  Assert(data); // data should not ever be null. the host data should have been created immediately after the host request was created.
                  //if assert fails, IpAddress could be wrong possibly?

    //Remove stale host 
    if ( mRemoveStaleHosts && IsStaleHost(ipAddress) )
    {
      netPeer.RemoveHost(mNetwork, ipAddress);
    }

    //TODO
    //Future Todo: Cancel extra host info connections if needed.
  }

  //
  //  Single Host Request Implementation
  //

  void SingleHostRequest::FlushHostRequest(NetPeer& netPeer, NetDiscoveryInterface& netDiscoveryInstance)
  {
    FlushHost(netPeer, netDiscoveryInstance, mIpAddress);
  }

  bool SingleHostRequest::IsNewHost(IpAddress const & hostIpAddress)
  {
    //TODO: Verify what would be needed to be certain this is a new host for a single host request.
    //if we were not previously known, and now we are discovered.
  return mPreviouslyKnown == false
      && mAllowDiscovery
      && mDiscoveryStage != NetDiscoveryStage::Unresponding;
  }

  bool SingleHostRequest::IsStaleHost(IpAddress const& hostIpAddress)
  {
  return mPreviouslyKnown
      && mDiscoveryStage == NetDiscoveryStage::Unresponding;
  }

  void SingleHostRequest::BeginExtraHostInfo()
  {
    //start the connections required in order to transfer extra host info
    Assert(mDiscoveryStage == NetDiscoveryStage::BasicHostInfo || mDiscoveryStage == NetDiscoveryStage::BasicHostInfoDirect);
    mDiscoveryStage = NetDiscoveryStage::ExtraHostInfo;
  }

  bool SingleHostRequest::GetIsFirstResponseFrom(IpAddress const& pingedHostIp)
  {
    return !mReceivedFirstResponse;
  }

  void SingleHostRequest::SetIsFirstResponseFrom(IpAddress const & pingedHostIp, bool isFirstResponse)
  {
    mReceivedFirstResponse = !isFirstResponse;
  }

  //
  //  Multi Host Request Implementation
  //

  void MultiHostRequest::FlushHostRequest(NetPeer& netPeer, NetDiscoveryInterface& netDiscoveryInstance)
  {
    // Alias name so its easier to access.
    ArrayMap<IpAddress, RespondingHostData>& respondingHostData = netDiscoveryInstance.mRespondingHostData;

    //Operate on all responding host data in it.
    typedef Zero::Pair< Zero::IpAddress, Zero::RespondingHostData > HostDataPair;
    forRange(HostDataPair &ipDataPair, respondingHostData.All() )
    {
      FlushHost(netPeer, netDiscoveryInstance, ipDataPair.first);
    }

    //also dispatch host list found event.
    NetHostListUpdate event;
    event.mNetwork = mNetwork;

    Zero::String eventId = mAllowDiscovery ? Events::NetHostListDiscovered : Events::NetHostListRefreshed;

    netPeer.DispatchEvent(eventId, &event);
  }

  bool MultiHostRequest::IsNewHost(IpAddress const & hostIpAddress)
  {
    return !mExpectedHosts.Contains(hostIpAddress) && mAllowDiscovery;
  }

  bool MultiHostRequest::IsStaleHost(IpAddress const& hostIpAddress)
  {
    //if we expected it, but did not get a response from it.
    return mExpectedHosts.Contains(hostIpAddress) && mAllowDiscovery && !mRespondingHosts.Contains(hostIpAddress);
  }

  void MultiHostRequest::BeginExtraHostInfo()
  {
    Assert(mDiscoveryStage == NetDiscoveryStage::BasicHostInfo || mDiscoveryStage == NetDiscoveryStage::BasicHostInfoDirect);
    mDiscoveryStage = NetDiscoveryStage::ExtraHostInfo;
    //start the connections required in order to transfer extra host info
  }

  bool MultiHostRequest::GetIsFirstResponseFrom(IpAddress const& pingedHostIp)
  {
    return !mRespondingHosts.Contains(pingedHostIp);
  }

  void MultiHostRequest::SetIsFirstResponseFrom(IpAddress const & pingedHostIp, bool isFirstResponse)
  {
    if (isFirstResponse)
    {
      mRespondingHosts.EraseValue(pingedHostIp);
    }
    else
    {
      mRespondingHosts.InsertOrAssign(pingedHostIp);
    }
  }

} // namespace Zero
