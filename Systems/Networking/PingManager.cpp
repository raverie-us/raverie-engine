///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#undef SetPort

namespace Zero
{

PingManager::PingManager(NetPeer* netPeer)
  : mNetPeer(netPeer),
    mPendingHostPings(),
    mManagerId(0),
    mNextRandomId(0),
    mPingTimeoutCallback(nullptr),
    mPingCancelledCallback(nullptr),
    mPingCallback(nullptr),
    mPongCallback(nullptr),
    mTimer(),
    mPingInterval(0.25f)
{
  // Now dependent on peer manager for id. Consider instead passing in argument for id
  // if you truly want to encapsulate the class.
  mManagerId = (netPeer->mNextManagerId += 1);
}

void PingManager::UpdatePendingPings()
{
  // Get current time
  TimeMs now = mTimer.UpdateAndGetTimeMilliseconds();

  // Handle pending host ping requests
  for (uint i = 0; i < mPendingHostPings.Size(); )
  {
    PendingHostPing& pendingHostPing = *mPendingHostPings[i];

    // Pending host ping request has timed out?
    if (pendingHostPing.HasTimedOut(now))
    {
      // Call callback for timed out ping!
      if(mPingTimeoutCallback)
      {
        (*mPingTimeoutCallback)(pendingHostPing);
      }

      // Erase and advance
      mPendingHostPings.EraseAt(i);
      continue;
    }
    // Pending host ping request has not timed out?
    else
    {
      // Time to send another host ping message?
      if (TimeMsToFloatSeconds(pendingHostPing.GetDurationSinceLastSendTime(now)) >= mPingInterval)
      {
        // Send another host ping message
        bool result = SendHostPing(pendingHostPing, now);
        Assert(result);
      }

      // Advance
      ++i;
      continue;
    }
  }
}

uint PingManager::AcquireNextRandomIncrementalId()
{
  // Increment next random ID by some small random value. We don't care if this wraps around.
  // (This helps prevent predictive message spam denial of service attacks)
  return (mNextRandomId += uint(Math::Random().IntRangeInIn(0, 1024)));
}

bool PingManager::PingHost(Network::Enum network, const IpAddress& theirIpAddress, HostPingType::Enum hostPingType, TimeMs timeout, const EventBundle& pingBundle)
{
  Array<IpAddress> ips;
  ips.PushBack(theirIpAddress);

  return PingHost(network, ips, hostPingType, timeout, pingBundle);
}

bool PingManager::PingHost(Network::Enum network, const Array<IpAddress>& theirIpAddresses, HostPingType::Enum hostPingType, TimeMs timeout, const EventBundle& pingBundle)
{
  // Get current time
  TimeMs now = mTimer.UpdateAndGetTimeMilliseconds();

  // Is there already a pending host ping request for the same IP address and ping type?
  for (uint i = 0; i < mPendingHostPings.Size(); ++i)
  {
    PendingHostPing& pendingHostPing = *mPendingHostPings[i];

    // Same IP addresses and host ping type?
    if (pendingHostPing.mTheirIpAddresses == theirIpAddresses
      && pendingHostPing.mHostPingType == hostPingType)
    {
      // Cancel previous pending host ping request
      if(mPingCancelledCallback)(*mPingCancelledCallback)(pendingHostPing);
        
      mPendingHostPings.EraseAt(i);
      break;
    }
  }

  // Get next unique ping ID
  uint pingId = AcquireNextRandomIncrementalId();

  PendingHostPing* newPing = new PendingHostPing(network, now, timeout, hostPingType, theirIpAddresses, pingId, pingBundle);

  // Add to pending host pings set
  PendingHostPingSet::pointer_bool_pair result = mPendingHostPings.Insert( UniquePointer<PendingHostPing>(newPing) );
  if (!result.second) // Unable?
  {
    Assert(false);
    return false;
  }

  // Send initial host ping message as configured
  return SendHostPing(**result.first, now);
}

bool PingManager::SendHostPing(PendingHostPing& pendingHostPing, TimeMs now)
{
  Assert(mNetPeer != nullptr);
  Assert(mNetPeer->IsOpen());

  // Get our project GUID
  Guid ourProjectGuid = mNetPeer->GetOurProjectGuid();

  // Start a new send attempt
  uint sendAttemptId = AcquireNextRandomIncrementalId();
  pendingHostPing.AddSendAttempt(sendAttemptId, now);

  // Create network host ping message
  Message netHostPingMessage(NetPeerMessageType::NetHostPing);

  NetHostPingData netHostPingData;
  netHostPingData.mProjectGuid = ourProjectGuid;
  netHostPingData.mPingId = pendingHostPing.mPingId;
  netHostPingData.mSendAttemptId = sendAttemptId;
  netHostPingData.mManagerId = mManagerId;
  netHostPingData.mEventBundleData = pendingHostPing.mPingBundle.GetBitStream();

  netHostPingMessage.GetData().Write(netHostPingData);

  // Single port specified?
  bool sendResult = true;

  forRange(IpAddress& theirIp, pendingHostPing.mTheirIpAddresses.All())
  {
    // Do they have a non zero port?
    if (theirIp.GetPort() != 0)
    {
      // Send net host ping directly to them.
      if (!mNetPeer->Peer::Send(theirIp, netHostPingMessage)) // Unable?
        sendResult = false;
    }
    // All ports (within our inclusive host range) specified?
    else
    {
      // For All ports within our inclusive host range
      IpAddress ipAddress(theirIp);
      for (uint i = mNetPeer->GetHostPortRangeStart(); i <= mNetPeer->GetHostPortRangeEnd(); ++i)
      {
        // Send net host ping
        ipAddress.SetPort(i);
        if (!mNetPeer->Peer::Send(ipAddress, netHostPingMessage)) // Unable?
          sendResult = false;
      }
    }
  }

  // Error sending host ping?
  if (!sendResult)
    DoNotifyWarning("Unable to ping hosts", "There was an error sending one or more host ping messages");

  // Complete
  return sendResult;
}

bool PingManager::ReceiveHostPing(const IpAddress& theirIpAddress, const Message& message)
{
  Assert(mNetPeer != nullptr);
  Assert(mNetPeer->IsOpen());

  // Save the state of the bitstream before it was read from in case we need to return it to its previous state.
  Bits bitsPrevRead = message.GetData().GetBitsRead();

  // Read network host ping message data
  NetHostPingData netHostPingData;
  if (!message.GetData().Read(netHostPingData)) // Unable?
  {
    Assert(false);
    return false;
  }

  bool handled = false;

  // Handle ping in callback
  if(mPingCallback)
  {
    // Potentially they decide they don't want to handle it
    handled = (*mPingCallback)(theirIpAddress, netHostPingData);
  }

  // Rewind message if we didn't handle it
  if (!handled)
  {
    message.GetData().SetBitsRead(bitsPrevRead);
  }

  // Success
  return handled;
}

bool PingManager::SendHostPong(const IpAddress& theirIpAddress, uint pingId, uint sendAttemptId, uint theirManagerId, BitStream& pongData)
{
  Assert(mNetPeer != nullptr);
  Assert(mNetPeer->IsOpen());

  // Get our project GUID
  Guid ourProjectGuid = mNetPeer->GetOurProjectGuid();

  // Create network host pong message
  Message netHostPongMessage(NetPeerMessageType::NetHostPong);

  NetHostPongData netHostPongData;
  netHostPongData.mProjectGuid = ourProjectGuid;
  netHostPongData.mPingId = pingId;
  netHostPongData.mSendAttemptId = sendAttemptId;
  netHostPongData.mManagerId = theirManagerId;
  netHostPongData.mEventBundleData = ZeroMove(pongData);

  netHostPongMessage.GetData().Write(netHostPongData);

  // Send net host pong
  bool sendResult = true;
  if (!mNetPeer->Peer::Send(theirIpAddress, netHostPongMessage))
  {
    DoNotifyWarning("Unable to pong host", "There was an error sending a host pong message");
    return false;
  }

  // Success
  return true;
}
bool PingManager::ReceiveHostPong(const IpAddress& theirIpAddress, const Message& message)
{
  Bits bitsPreviouslyRead = message.GetData().GetBitsRead();

  // Read network host pong message data
  NetHostPongData netHostPongData;
  if (!message.GetData().Read(netHostPongData)) // Unable?
  {
    Assert(false);
    return false;
  }

  // Check that this pong is for this PingManager. return false if its not.
  if(netHostPongData.mManagerId != mManagerId)
  {
    //Rewind the message.
    message.GetData().SetBitsRead(bitsPreviouslyRead);
    return false; //this ping is not for us.
  }

  // It was for us, but no pending pings?
  // No pending host ping requests?
  if (mPendingHostPings.Empty())
  {
    // Do nothing
    // (This likely means the pending host ping request has timed out or was canceled earlier)
    // (Or this is an erroneous/malicious response) in either case we return true as if it were handled.
    return true;
  }

  // Find pending host ping request via unique ping ID
  UniquePointer<PendingHostPing>* pendingHostPingPtr = mPendingHostPings.FindPointer(netHostPongData.mPingId);

  if (!pendingHostPingPtr) // Unable?
  {
    // Done
    // (This likely means the pending host ping request has timed out or was canceled earlier)
    return true;
  }

  PendingHostPing& pendingPingRef = **pendingHostPingPtr;

  // Let user handle receiving a host pong.
  if(mPongCallback)
  {
    (*mPongCallback)(theirIpAddress, netHostPongData, pendingPingRef);
  }

  // Add them to responding hosts.
  pendingPingRef.mRespondingHosts.InsertOrAssign(theirIpAddress);

  // Individual refreshes get auto removed. multi-host refreshes generally go to timeout.
  if (pendingPingRef.mHostPingType == HostPingType::Refresh || pendingPingRef.mHostPingType == HostPingType::MasterServerRefreshHost)
  {
    mPendingHostPings.EraseValue(pendingPingRef);
  }

  // Done
  return true;
}

bool PingManager::ReceivePeerMessage(IpAddress const& theirIpAddress, Message& peerMessage)
{
  switch (peerMessage.GetType())
  {
    case NetPeerMessageType::NetHostPing:
    {
      // Receive network host ping
      return ReceiveHostPing(theirIpAddress, peerMessage);
    }
    break;

    case NetPeerMessageType::NetHostPong:
    {
      // Receive network host pong
      return ReceiveHostPong(theirIpAddress, peerMessage);
    }
    break;

    default:
      return false; // We don't handle this message
  }
}

bool PingManager::ReceiveLinkMessage(IpAddress const& theirIpAddress, Message& linkMessage)
{
  return false; // Pings make no use of Link messages (that's why they are pings). So we don't handle this message.
}

void PingManager::SetPingTimeoutCallback(PendingPingCallback callback)
{
  mPingTimeoutCallback = callback;
}

void PingManager::SetPingCancelledCallback(PendingPingCallback callback)
{
  mPingCancelledCallback = callback;
}

void PingManager::SetPingCallback(PingReceivedCallback callback)
{
  mPingCallback = callback;
}


void PingManager::SetPongCallback(PongReceivedCallback callback)
{
  mPongCallback = callback;
}

} // namespace Zero
