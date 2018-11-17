///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  PeerLink                                       //
//---------------------------------------------------------------------------------//

void PeerLink::ResetSession()
{
  /// Operating Data
  mTheirGuid            = 0;
  mOurIpAddress         = IpAddress();
  LinkInbox defaultInbox(this);
  mInbox                = ZeroMove(defaultInbox);
  LinkOutbox defaultOutbox(this);
  mOutbox               = ZeroMove(defaultOutbox);

  /// State Data
  mConnectRequested           = false;
  mConnectRequestExtraData.Clear(true);
  mDisconnectRequested        = false;
  mDisconnectRequestExtraData.Clear(true);
  mConnectResponded           = UserConnectResponse::Pending;
  mConnectResponseExtraData.Clear(true);
  mStateACKId                 = 0;
  mSessionCompleted           = false;

  /// Frame Data
  mOutgoingBandwidth     = 0;
  mOutgoingFrameCapacity = 0;
  mOutgoingFrameSize     = 0;

  InitializeStats();
  Assert(mState == LinkState::Disconnected);
}

PeerLink::PeerLink(Peer* peer, const IpAddress& ipAddress, TransmissionDirection::Enum creationDirection)
  : BandwidthStats<false>(),

    /// Operating Data
    mPeer(peer),
    mTheirGuid(0),
    mTheirIpAddress(ipAddress),
    mOurIpAddress(),
    mInbox(this),
    mOutbox(this),
    mUserMessageTypeStart(CustomMessageTypeStart),
    mUserData(nullptr),

    /// State Data
    mLocalToRemoteTimeDifference(0),
    mCreationTime(GetLocalTime()),
    mCreationDirection(creationDirection),
    mConnectRequested(false),
    mConnectRequestExtraData(),
    mDisconnectRequested(false),
    mDisconnectRequestExtraData(),
    mConnectResponded(UserConnectResponse::Pending),
    mConnectResponseExtraData(),
    mState(LinkState::Disconnected),
    mStateTime(mCreationTime),
    mStateACKId(0),
    mSessionCompleted(false),

    /// Plugin Data
    mAddedPlugins(),
    mPlugins(),
    mRemovedPlugins(),

    /// Frame Data
    mOutgoingBandwidth(0),
    mOutgoingFrameCapacity(0),
    mOutgoingFrameSize(0)
{
  ResetConfig();
  InitializeStats();
  Assert(mTheirIpAddress.IsValid());
}

bool PeerLink::operator ==(const PeerLink& rhs) const
{
  return mTheirIpAddress == rhs.mTheirIpAddress;
}
bool PeerLink::operator !=(const PeerLink& rhs) const
{
  return mTheirIpAddress != rhs.mTheirIpAddress;
}
bool PeerLink::operator  <(const PeerLink& rhs) const
{
  return mTheirIpAddress < rhs.mTheirIpAddress;
}
bool PeerLink::operator ==(const IpAddress& rhs) const
{
  return mTheirIpAddress == rhs;
}
bool PeerLink::operator !=(const IpAddress& rhs) const
{
  return mTheirIpAddress != rhs;
}
bool PeerLink::operator  <(const IpAddress& rhs) const
{
  return mTheirIpAddress < rhs;
}

//
// Member Functions
//

Peer* PeerLink::GetPeer() const
{
  return mPeer;
}

TimeMs PeerLink::GetCreationDuration() const
{
  return GetDuration(mCreationTime, GetLocalTime());
}
TransmissionDirection::Enum PeerLink::GetCreationDirection() const
{
  return mCreationDirection;
}

LinkStatus::Enum PeerLink::GetStatus() const
{
  switch(mState)
  {
  // LinkStatus::Disconnected
  default:
    Assert(false);
  case LinkState::Disconnected:
  case LinkState::SentDisconnectNotice:
  case LinkState::ReceivedDisconnectNotice:
  case LinkState::SentNegativeConnectResponse:
  case LinkState::ReceivedNegativeConnectResponse:
    return LinkStatus::Disconnected;

  // LinkStatus::AttemptingConnection
  case LinkState::SentConnectRequest:
  case LinkState::ReceivedConnectRequest:
    return LinkStatus::AttemptingConnection;

  // LinkStatus::Connected
  case LinkState::Connected:
    return LinkStatus::Connected;
  }
}
LinkState::Enum PeerLink::GetState() const
{
  return mState;
}
TimeMs PeerLink::GetStateDuration() const
{
  return GetDuration(mStateTime, GetLocalTime());
}

Guid PeerLink::GetTheirGuid() const
{
  return mTheirGuid;
}

const IpAddress& PeerLink::GetTheirIpAddress() const
{
  return mTheirIpAddress;
}

const IpAddress& PeerLink::GetOurIpAddress() const
{
  return mOurIpAddress;
}

InternetProtocol::Enum PeerLink::GetInternetProtocol() const
{
  return mTheirIpAddress.GetInternetProtocol();
}

bool PeerLink::Connect(const BitStream& extraData)
{
  // Link not disconnected?
  if(GetState() != LinkState::Disconnected)
    return false;

  // Initiate connect request
  // Will be sent later on update
  mConnectRequested        = true;
  mConnectRequestExtraData = extraData;
  mConnectRequestExtraData.ClearBitsRead();

  // Success
  return true;
}

bool PeerLink::RespondToConnectRequest(bool accept, const BitStream& extraData)
{
  // Link not attempting connection?
  if(GetState() != LinkState::ReceivedConnectRequest)
    return false;

  // Initiate connect response
  // Will be sent later on update
  mConnectResponded         = accept ? UserConnectResponse::Accept : UserConnectResponse::Deny;
  mConnectResponseExtraData = extraData;
  mConnectResponseExtraData.ClearBitsRead();

  // Success
  return true;
}

bool PeerLink::Disconnect(const BitStream& extraData)
{
  // Link not connected and not attempting connection?
  LinkStatus::Enum linkStatus = GetStatus();
  if(linkStatus != LinkStatus::Connected
  && linkStatus != LinkStatus::AttemptingConnection)
    return false;

  // Initiate disconnect notice
  // Will be sent later on update
  mDisconnectRequested        = true;
  mDisconnectRequestExtraData = extraData;
  mDisconnectRequestExtraData.ClearBitsRead();

  // Success
  return true;
}

MessageReceiptId PeerLink::Send(Status& status, const Message& message,         bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime)
{
  // Relative message type outside of user range?
  if(message.GetType() >= GetUserMessageTypeCount())
  {
    // Failure
    status.SetFailed("User message type is out of range");
    return 0;
  }

  // Copy message
  Message messageCopy(message);

  // Convert relative message type to absolute message type
  messageCopy.mType = RelativeToAbsolute(messageCopy.mType);

  // Send message
  return SendInternal(status, ZeroMove(messageCopy), reliable, channelId, receipt, priority, lifetime, false);
}
MessageReceiptId PeerLink::Send(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime)
{
  // Relative message type outside of user range?
  if(message->GetType() >= GetUserMessageTypeCount())
  {
    // Failure
    status.SetFailed("User message type is out of range");
    return 0;
  }

  // Convert relative message type to absolute message type
  message->mType = RelativeToAbsolute(message->mType);

  // Send message
  return SendInternal(status, ZeroMove(message), reliable, channelId, receipt, priority, lifetime, false);
}

TimeMs PeerLink::GetLocalTime() const
{
  return GetPeer()->GetLocalTime();
}
TimeMs PeerLink::GetLocalDeltaTime() const
{
  return GetPeer()->GetLocalDeltaTime();
}
uint64 PeerLink::GetLocalFrameId() const
{
  return GetPeer()->GetLocalFrameId();
}

TimeMs PeerLink::GetRemoteTime() const
{
  return LocalToRemoteTime(GetLocalTime());
}

TimeMs PeerLink::LocalToRemoteTime(TimeMs localTime) const
{
  return localTime + GetLocalToRemoteTimeDifference();
}
TimeMs PeerLink::RemoteToLocalTime(TimeMs remoteTime) const
{
  return remoteTime - GetLocalToRemoteTimeDifference();
}

void PeerLink::SetLocalToRemoteTimeDifference(TimeMs localToRemoteTimeDifference)
{
  mLocalToRemoteTimeDifference = localToRemoteTimeDifference;
}
TimeMs PeerLink::GetLocalToRemoteTimeDifference() const
{
  return mLocalToRemoteTimeDifference;
}

void PeerLink::SetUserData(void* userData)
{
  mUserData = userData;
}
void* PeerLink::GetUserData() const
{
  return mUserData;
}

//
// Outgoing Message Channel Management
//

OutMessageChannel* PeerLink::OpenOutgoingChannel(TransferMode::Enum transferMode)
{
  return mOutbox.OpenOutgoingChannel(transferMode);
}

OutMessageChannel* PeerLink::GetOutgoingChannel(MessageChannelId channelId) const
{
  return mOutbox.GetOutgoingChannel(channelId);
}
ArraySet<OutMessageChannel>::range PeerLink::GetOutgoingChannels() const
{
  return mOutbox.GetOutgoingChannels();
}
uint PeerLink::GetOutgoingChannelCount() const
{
  return mOutbox.GetOutgoingChannelCount();
}

void PeerLink::CloseOutgoingChannel(MessageChannelId channelId)
{
  return mOutbox.CloseOutgoingChannel(channelId);
}
void PeerLink::CloseOutgoingChannels()
{
  return mOutbox.CloseOutgoingChannels();
}

//
// Incoming Message Channel Management
//

InMessageChannel* PeerLink::GetIncomingChannel(MessageChannelId channelId) const
{
  return mInbox.GetIncomingChannel(channelId);
}
ArraySet<InMessageChannel>::range PeerLink::GetIncomingChannels() const
{
  return mInbox.GetIncomingChannels();
}
uint PeerLink::GetIncomingChannelCount() const
{
  return mInbox.GetIncomingChannelCount();
}

//
// Link Plugin Management
//

LinkPlugin* PeerLink::AddPlugin(LinkPlugin* plugin, StringParam name)
{
  // Plugin of that name is already active?
  if(mPlugins.FindPointer(name))
    return nullptr;

  // Plugin of that name was just added?
  if(mAddedPlugins.FindPointer(name))
    return nullptr;

  // Plugin uses a custom message type range?
  MessageType count = plugin->GetMessageTypeCount();
  if(count)
  {
    // Not enough message types available to satisfy the plugin?
    MessageType available = GetUserMessageTypeCount();
    if(count > available)
    {
      Error("Unable to add plugin - Not enough message types available to satisfy the plugin");
      return nullptr;
    }

    // Set plugin message type start
    plugin->SetMessageTypeStart(mUserMessageTypeStart);

    // User message types exhausted?
    if(count == available)
      mUserMessageTypeStart = 0;
    else
      mUserMessageTypeStart += count;
  }

  // Add new plugin to added plugins
  // Will be initialized later on update
  plugin->SetName(name);
  LinkPluginSet::pointer_bool_pair result = mAddedPlugins.Insert(plugin);
  Assert(result.second); // (Insertion should have succeeded)
  return plugin;
}

LinkPlugin* PeerLink::GetPlugin(StringParam name) const
{
  // Find active plugin
  LinkPluginSet::pointer result = mPlugins.FindPointer(name);
  if(result) // Found?
    return *result;
  else
    return nullptr;
}
LinkPluginSet::range PeerLink::GetPlugins() const
{
  return mPlugins.All();
}
uint PeerLink::GetPluginCount() const
{
  return mPlugins.Size();
}

void PeerLink::RemovePlugin(StringParam name)
{
  // Find active plugin
  LinkPluginSet::iterator iter = mPlugins.FindIterator(name);
  if(iter != mPlugins.End()) // Found?
  {
    // Add plugin to removed plugins
    // Will be uninitialized later on update
    LinkPluginSet::pointer_bool_pair result = mRemovedPlugins.Insert(*iter);
    Assert(result.second);  // (Insertion should have succeeded)

    // Remove plugin from active plugins
    mPlugins.Erase(iter);
  }
}
void PeerLink::RemovePlugins()
{
  while(!mPlugins.Empty())
    RemovePlugin(mPlugins.Back()->GetName());
}

//
// Link Configuration
//

void PeerLink::ResetConfig()
{
  SetSendRate();
  SetPacketDataBytes();
  SetPreconnectionRoundTripTime();
  SetFloorRoundTripTime();
  SetPacketSequenceHistoryRangeFactor();
  SetPacketSequenceHistoryRateFactor();
  SetPacketNAKFactor();
  SetHeartbeatPacketRate();
  SetTimeoutFactor();
  SetLatencyLimit();
  SetConnectAttemptFactor();
  SetDisconnectAttemptFactor();
}

void PeerLink::SetSendRate(uint sendRate)
{
  mSendRate = std::max(uint(1), sendRate); //Clamp(sendRate, uint(5), uint(60)); //HACK

  // TODO: Update this after implementing AIMD
  // Separate this out into OutgoingBandwidth AND OutgoingDataBandwidth (for messages)
  // Update outgoing bandwidth
  SetOutgoingBandwidth((double(GetSendRate()) * double(BYTES_TO_BITS(GetPacketDataBytes()) + MaxPacketHeaderBits)) / double(1000));
}
uint PeerLink::GetSendRate() const
{
  return mSendRate;
}

void PeerLink::SetPacketDataBytes(Bytes packetDataBytes)
{
  mPacketDataBytes = Clamp(packetDataBytes, MinPacketDataBytes, MaxPacketDataBytes);

  // TODO: Update this after implementing AIMD
  // Separate this out into OutgoingBandwidth AND OutgoingDataBandwidth (for messages)
  // Update outgoing bandwidth
  SetOutgoingBandwidth((double(GetSendRate()) * double(BYTES_TO_BITS(GetPacketDataBytes()) + MaxPacketHeaderBits)) / double(1000));
}
Bytes PeerLink::GetPacketDataBytes() const
{
  return mPacketDataBytes;
}

void PeerLink::SetPreconnectionRoundTripTime(TimeMs preconnectionRoundTripTime)
{
  mPreconnectionRoundTripTime = preconnectionRoundTripTime;
}
TimeMs PeerLink::GetPreconnectionRoundTripTime() const
{
  return mPreconnectionRoundTripTime;
}

void PeerLink::SetFloorRoundTripTime(TimeMs floorRoundTripTime)
{
  mFloorRoundTripTime = floorRoundTripTime;
}
TimeMs PeerLink::GetFloorRoundTripTime() const
{
  return mFloorRoundTripTime;
}

void PeerLink::SetPacketSequenceHistoryRangeFactor(float packetSequenceHistoryRangeFactor)
{
  mPacketSequenceHistoryRangeFactor = packetSequenceHistoryRangeFactor;
}
float PeerLink::GetPacketSequenceHistoryRangeFactor() const
{
  return mPacketSequenceHistoryRangeFactor;
}

void PeerLink::SetPacketSequenceHistoryRateFactor(float packetSequenceHistoryRateFactor)
{
  mPacketSequenceHistoryRateFactor = packetSequenceHistoryRateFactor;
}
float PeerLink::GetPacketSequenceHistoryRateFactor() const
{
  return mPacketSequenceHistoryRateFactor;
}

void PeerLink::SetPacketNAKFactor(float packetNAKFactor)
{
  mPacketNAKFactor = packetNAKFactor;
}
float PeerLink::GetPacketNAKFactor() const
{
  return mPacketNAKFactor;
}

void PeerLink::SetHeartbeatPacketRate(uint heartbeatPacketRate)
{
  mHeartbeatPacketRate = heartbeatPacketRate;
}
uint PeerLink::GetHeartbeatPacketRate() const
{
  return mHeartbeatPacketRate;
}

void PeerLink::SetTimeoutFactor(float timeoutFactor)
{
  mTimeoutFactor = timeoutFactor;
}
float PeerLink::GetTimeoutFactor() const
{
  return mTimeoutFactor;
}

void PeerLink::SetLatencyLimit(TimeMs latencyLimit)
{
  mLatencyLimit = latencyLimit;
}
TimeMs PeerLink::GetLatencyLimit() const
{
  return mLatencyLimit;
}

void PeerLink::SetConnectAttemptFactor(float connectAttemptFactor)
{
  mConnectAttemptFactor = connectAttemptFactor;
}
float PeerLink::GetConnectAttemptFactor() const
{
  return mConnectAttemptFactor;
}

void PeerLink::SetDisconnectAttemptFactor(float disconnectAttemptFactor)
{
  mDisconnectAttemptFactor = disconnectAttemptFactor;
}
float PeerLink::GetDisconnectAttemptFactor() const
{
  return mDisconnectAttemptFactor;
}

Array< Pair<String, String> > PeerLink::GetConfigSummary() const
{
  // TODO
  return Array< Pair<String, String> >();
}
String PeerLink::GetConfigSummaryString() const
{
  // TODO
  return String();
}

//
// Link Statistics
//

TimeMs PeerLink::GetMinRoundTripTime() const
{
  return mRoundTripTimeMin;
}
TimeMs PeerLink::GetAvgRoundTripTime() const
{
  return mRoundTripTimeAvg;
}
TimeMs PeerLink::GetMaxRoundTripTime() const
{
  return mRoundTripTimeMax;
}

TimeMs PeerLink::GetMinInternalRoundTripTime() const
{
  return mInternalRoundTripTimeMin;
}
TimeMs PeerLink::GetAvgInternalRoundTripTime() const
{
  return mInternalRoundTripTimeAvg;
}
TimeMs PeerLink::GetMaxInternalRoundTripTime() const
{
  return mInternalRoundTripTimeMax;
}

Array< Pair< String, Array<String> > > PeerLink::GetStatsSummary() const
{
  // TODO
  return Array< Pair< String, Array<String> > >();
}
String PeerLink::GetStatsSummaryString() const
{
  // TODO
  return String();
}

//
// Link Frame Data
//

void PeerLink::SetOutgoingBandwidth(Kbps outgoingBandwidth)
{
  mOutgoingBandwidth = outgoingBandwidth;
}
Kbps PeerLink::GetOutgoingBandwidth() const
{
  return mOutgoingBandwidth;
}

void PeerLink::SetOutgoingFrameCapacity(Bits outgoingFrameCapacity)
{
  mOutgoingFrameCapacity = outgoingFrameCapacity;
}
Bits PeerLink::GetOutgoingFrameCapacity() const
{
  return mOutgoingFrameCapacity;
}

void PeerLink::SetOutgoingFrameSize(Bits outgoingFrameSize)
{
  mOutgoingFrameSize = outgoingFrameSize;
}
Bits PeerLink::GetOutgoingFrameSize() const
{
  return mOutgoingFrameSize;
}

float PeerLink::GetOutgoingFrameFill() const
{
  return float(mOutgoingFrameSize)
       / float(mOutgoingFrameCapacity);
}

//
// Internal
//

void PeerLink::ResetStats()
{
  BandwidthStats<false>::ResetStats();

  mRoundTripTimeUpdated = false;
  mRoundTripTimeMin     = 0;
  mRoundTripTimeAvg     = 0;
  mRoundTripTimeMax     = 0;

  // TODO: Update this after implementing AIMD
  // Separate this out into OutgoingBandwidth AND OutgoingDataBandwidth (for messages)
  // Update outgoing bandwidth
  SetOutgoingBandwidth((double(GetSendRate()) * double(BYTES_TO_BITS(GetPacketDataBytes()) + MaxPacketHeaderBits)) / double(1000));
}

void PeerLink::InitializeStats()
{
  ResetStats();

  // Default internal RTT to preconnection RTT
  mInternalRoundTripTimeMin = GetPreconnectionRoundTripTime();
  mInternalRoundTripTimeAvg = GetPreconnectionRoundTripTime();
  mInternalRoundTripTimeMax = GetPreconnectionRoundTripTime();
}

void PeerLink::UpdateRoundTripTime(TimeMs sample, TimeMs floor)
{
  if(mRoundTripTimeUpdated)
  {
    mRoundTripTimeMin = std::min(mRoundTripTimeMin, sample);
    mRoundTripTimeAvg = Average(mRoundTripTimeAvg, sample, 0.1);
    mRoundTripTimeMax = std::max(mRoundTripTimeMax, sample);
  }
  else
  {
    mRoundTripTimeMin     = sample;
    mRoundTripTimeAvg     = sample;
    mRoundTripTimeMax     = sample;
    mRoundTripTimeUpdated = true;
  }

  mInternalRoundTripTimeMin = std::max(std::min(mInternalRoundTripTimeMin, sample), floor);
  mInternalRoundTripTimeAvg = std::max(Average(mInternalRoundTripTimeAvg, sample, 0.1), floor);
  mInternalRoundTripTimeMax = std::max(std::max(mInternalRoundTripTimeMax, sample), floor);
}

MessageReceiptId PeerLink::SendInternal(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime, bool isProtocol)
{
  MessageReceiptId receiptId = mOutbox.PushMessage(status, ZeroMove(message), reliable, channelId, receipt, priority, lifetime, isProtocol);
  Assert(isProtocol ? status.Succeeded() : true); // (All protocol sends should succeed)
  Assert(receipt ? receiptId : true); // (All receipted messages should be given a non-zero receipt ID)
  return receiptId;
}

bool PeerLink::AbsoluteIsInRange(MessageType absoluteType) const
{
  MessageType count = GetUserMessageTypeCount();
  return count
       ? AbsoluteToRelative(absoluteType) <= (count - 1)
       : false;
}
MessageType PeerLink::RelativeToAbsolute(MessageType relativeType) const
{
  Assert(GetUserMessageTypeCount());
  return relativeType + mUserMessageTypeStart;
}
MessageType PeerLink::AbsoluteToRelative(MessageType absoluteType) const
{
  Assert(GetUserMessageTypeCount());
  Assert(absoluteType >= mUserMessageTypeStart);
  return absoluteType - mUserMessageTypeStart;
}

void PeerLink::SetState(LinkState::Enum state)
{
  LinkState::Enum  prevState  = mState;
  LinkStatus::Enum prevStatus = GetStatus();

  mState     = state;
  mStateTime = GetLocalTime();

  // State changed?
  if(prevState != mState)
  {
    // [Link Plugin Event]
    PluginEventOnStateChange(prevState);

    // [Link Event]
    LinkEventStateChange(mState);
  }

  // Status changed?
  LinkStatus::Enum curStatus = GetStatus();
  if(prevStatus != curStatus)
  {
    // [Link Plugin Event]
    PluginEventOnStatusChange(prevStatus);

    // [Link Event]
    LinkEventStatusChange(curStatus);
  }

  // Link just became disconnected?
  if(prevState != mState
  && mState == LinkState::Disconnected)
  {
    // This session is now complete
    // All link session data will be reset on the next user update call
    // This allows the link to be reused if so desired by the user
    mSessionCompleted = true;
  }
}

void PeerLink::ReceivePacket(MoveReference<InPacket> inPacket)
{
  // [Link Plugin Event] Stop?
  if(!PluginEventOnPacketReceive(*inPacket))
    return;

  // Update Stats
  UpdatePacketsReceived();
  TimeMs lastReceiveDuration = std::max(mInbox.GetLastReceiveDuration(), TimeMs(1));
  UpdateIncomingBandwidthUsage((double(inPacket->GetTotalBits()) / double(1000)) * (double(cOneSecondTimeMs) / double(lastReceiveDuration)));
  UpdateReceiveRate(uint(cOneSecondTimeMs / lastReceiveDuration));
  UpdateReceivedPacketBytes(BITS_TO_BYTES(inPacket->GetTotalBits()));

  // Receive packet to be processed later
  mInbox.ReceivePacket(ZeroMove(inPacket));
}

void PeerLink::SendPacket(OutPacket& outPacket)
{
  // [Link Plugin Event] Stop?
  if(!PluginEventOnPacketSend(outPacket))
    return;

  // Get packet size (in bits)
  Bits outPacketBits = outPacket.GetTotalBits();

  // Send packet now
  GetPeer()->SendPacket(outPacket);

  // Update Stats
  UpdatePacketsSent();
  TimeMs lastSendDuration = std::max(mOutbox.GetLastSendDuration(), TimeMs(1));
  UpdateOutgoingBandwidthUsage((double(outPacketBits) / double(1000)) * (double(cOneSecondTimeMs) / double(lastSendDuration)));
  UpdateSendRate(uint(cOneSecondTimeMs / lastSendDuration));
  UpdateSentPacketBytes(BITS_TO_BYTES(outPacketBits));

  // Add sent packet size to our outstanding frame size
  Bits currentFrameSize = GetOutgoingFrameSize();
  Bits newFrameSize = currentFrameSize + outPacketBits;
  SetOutgoingFrameSize(newFrameSize);
}

void PeerLink::UpdateLinkState()
{
  // Session is complete?
  if(mSessionCompleted)
  {
    // Reset all link session data so the link may be reused
    ResetSession();
  }

  // Get current time and delta time
  TimeMs now = GetLocalTime();
  float  dt  = TimeMsToFloatSeconds(GetLocalDeltaTime());

  // Set the amount of outgoing bandwidth available since our last update
  Bits newFrameCapacity = Bits(GetOutgoingBandwidth() * double(1000) * dt);
  SetOutgoingFrameCapacity(newFrameCapacity);

  // Deduct 'earned' frame capacity from our outstanding frame size
  Bits currentFrameSize = GetOutgoingFrameSize();
  Bits newFrameSize = currentFrameSize - Math::Min(currentFrameSize, newFrameCapacity);
  SetOutgoingFrameSize(newFrameSize);

  //
  // Remove Plugins
  //
  forRange(LinkPlugin* plugin, mRemovedPlugins.All())
  {
    // Uninitialize plugin
    plugin->Uninitialize();

    // Delete plugin
    delete plugin;
  }
  mRemovedPlugins.Clear();

  //
  // Add Plugins
  //
  forRange(LinkPlugin* plugin, mAddedPlugins.All())
  {
    // Initialize plugin
    if(plugin->Initialize(this)) // Continue?
    {
      // Add plugin to active plugins
      LinkPluginSet::pointer_bool_pair result = mPlugins.Insert(plugin);
      Assert(result.second);  // (Insertion should have succeeded)
    }
    // Stop?
    else
    {
      // Uninitialize plugin
      plugin->Uninitialize();

      // Delete plugin
      delete plugin;
    }
  }
  mAddedPlugins.Clear();

  //
  // Process Incoming Packets
  //
  ACKArray remoteACKs;
  NAKArray remoteNAKs;
  mInbox.Update(remoteACKs, remoteNAKs);

  //
  // Update Link State
  //

  // Get received protocol messages
  Array<Message>& protocolMessages = mInbox.mReleasedProtocolMessages;

  // Update link state
UpdateLinkState:
  switch(mState)
  {
  default:
    Assert(false);
    break;

  //--------------------------------------//
  //       LinkStatus::Disconnected       //
  //--------------------------------------//
  case LinkState::Disconnected:
    {
      //
      // Check for pending connect requests
      //

      // Received connect request message?
      forRange(Message& message, protocolMessages.All())
        if(message.GetType() == ProtocolMessageType::ConnectRequest)
        {
          // Connection limit has not been reached?
          if((GetPeer()->GetLinkCount(LinkStatus::Connected)
            + GetPeer()->GetLinkCount(LinkStatus::AttemptingConnection)) < GetPeer()->GetConnectionLimit())
          {
            // Read connect request message
            ConnectRequestData connectRequestData;
            if(message.GetData().Read(connectRequestData)) // Successful?
            {
              // Clear bits read
              message.GetData().ClearBitsRead();

              // Set our IP address (as seen from their perspective)
              mOurIpAddress = connectRequestData.mIpAddress;

              // Set difference between our local time values and their remote time values
              TimeMs localNow  = now;
              TimeMs remoteNow = connectRequestData.mTimestamp;
              mLocalToRemoteTimeDifference = (remoteNow - localNow);

              // [Link Event]
              LinkEventConnectRequested(TransmissionDirection::Incoming, connectRequestData);

              // Change link state
              SetState(LinkState::ReceivedConnectRequest);

              // [Link Plugin Event]
              PluginEventOnConnectRequestReceive(message);

              // Update link state again
              goto UpdateLinkState;
            }
          }
          // Connection limit has been reached?
          else
          {
            // Create connect response message
            Message connectResponseMessage(ProtocolMessageType::ConnectResponse);

            ConnectResponseData connectResponseData;
            connectResponseData.mIpAddress       = GetTheirIpAddress();
            connectResponseData.mTimestamp       = now;
            connectResponseData.mConnectResponse = ConnectResponse::DenyFull;

            connectResponseMessage.GetData().Write(connectResponseData);

            // Store connect response message copy
            Message connectResponseMessageCopy(connectResponseMessage);

            // Send connect response message
            Status status;
            mStateACKId = SendInternal(status, ZeroMove(connectResponseMessage), true, 0, true);

            // [Link Event]
            LinkEventConnectResponded(TransmissionDirection::Outgoing, connectResponseData);

            // Change link state
            SetState(LinkState::SentNegativeConnectResponse);

            // [Link Plugin Event]
            PluginEventOnConnectResponseSend(connectResponseMessageCopy);

            // Update link state again
            goto UpdateLinkState;

            break;
          }
        }

      // Outgoing connect requested?
      if(mConnectRequested)
      {
        // Create connect request message
        Message connectRequestMessage(ProtocolMessageType::ConnectRequest);

        ConnectRequestData connectRequestData;
        connectRequestData.mIpAddress = GetTheirIpAddress();
        connectRequestData.mTimestamp = now;
        connectRequestData.mExtraData = mConnectRequestExtraData;

        connectRequestMessage.GetData().Write(connectRequestData);

        // Store connect request message copy
        Message connectRequestMessageCopy(connectRequestMessage);

        // Send connect request message
        Status status;
        SendInternal(status, ZeroMove(connectRequestMessage), true);

        // [Link Event]
        LinkEventConnectRequested(TransmissionDirection::Outgoing, connectRequestData);

        // Change link state
        SetState(LinkState::SentConnectRequest);

        // [Link Plugin Event]
        PluginEventOnConnectRequestSend(connectRequestMessageCopy);
      }
    }
    break;

  case LinkState::SentDisconnectNotice:
    {
      //
      // Determine if our disconnect notice was ACKd before proceeding to the fully disconnected state
      // This lets our user ensure their disconnects are graceful, if they don't mind waiting for a reply
      //
      Assert(mStateACKId);

      // Received ACK receipt message for our previously sent disconnect notice?
      bool ACKd = false;
      forRange(Message& message, protocolMessages.All())
        if(message.GetType() == LinkEventMessageType::Receipt)
        {
          // Read receipt message
          ReceiptData receiptData;
          if(message.GetData().Read(receiptData)) // Successful?
          {
            // Is ACK receipt for our previously sent disconnect notice?
            if(receiptData.mReceiptId == mStateACKId
            && receiptData.mReceipt == Receipt::ACK)
            {
              ACKd = true;
              break;
            }
          }
        }

      // ACKd or (RTT/2) * DisconnectAttemptFactor has elapsed?
      if(ACKd
      || GetStateDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetDisconnectAttemptFactor()))
      {
        // Change link state
        SetState(LinkState::Disconnected);
      }
    }
    break;

  case LinkState::ReceivedDisconnectNotice:
    {
      //
      // Give our peer time to send out our ACK to their disconnect notice message
      // So the remote peer can proceed to the fully disconnected state gracefully
      //

      // (RTT/2) * DisconnectAttemptFactor has elapsed?
      if(GetStateDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetDisconnectAttemptFactor()))
      {
        // Change link state
        SetState(LinkState::Disconnected);
      }
    }
    break;

  case LinkState::SentNegativeConnectResponse:
    {
      //
      // Determine if our negative connect response was ACKd before proceeding to the fully disconnected state
      // This lets our user ensure their negative connect responses are graceful, if they don't mind waiting for a reply
      //
      Assert(mStateACKId);

      // Received ACK receipt message for our previously sent negative connect response?
      bool ACKd = false;
      forRange(Message& message, protocolMessages.All())
        if(message.GetType() == LinkEventMessageType::Receipt)
        {
          // Read receipt message
          ReceiptData receiptData;
          if(message.GetData().Read(receiptData)) // Successful?
          {
            // Is ACK receipt for our previously sent negative connect response?
            if(receiptData.mReceiptId == mStateACKId
            && receiptData.mReceipt == Receipt::ACK)
            {
              ACKd = true;
              break;
            }
          }
        }

      // ACKd or (RTT/2) * ConnectAttemptFactor has elapsed?
      if(ACKd
      || GetStateDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetConnectAttemptFactor()))
      {
        // Change link state
        SetState(LinkState::Disconnected);
      }
    }
    break;

  case LinkState::ReceivedNegativeConnectResponse:
    {
      //
      // Give our peer time to send out our ACK to their negative connect response message
      // So the remote peer can proceed to the fully disconnected state gracefully
      //

      // (RTT/2) * ConnectAttemptFactor has elapsed?
      if(GetStateDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetConnectAttemptFactor()))
      {
        // Change link state
        SetState(LinkState::Disconnected);
      }
    }
    break;

  //--------------------------------------//
  //   LinkStatus::AttemptingConnection   //
  //--------------------------------------//
  case LinkState::SentConnectRequest:
    {
      //
      // Check for pending connect responses or timeout
      //

      // Received connect response message?
      forRange(Message& message, protocolMessages.All())
        if(message.GetType() == ProtocolMessageType::ConnectResponse)
        {
          // Read connect response message
          ConnectResponseData connectResponseData;
          if(message.GetData().Read(connectResponseData)) // Successful?
          {
            // Clear bits read
            message.GetData().ClearBitsRead();

            // Set our IP address (as seen from their perspective)
            mOurIpAddress = connectResponseData.mIpAddress;

            // Set difference between our local time values and their remote time values
            TimeMs localNow  = now;
            TimeMs remoteNow = connectResponseData.mTimestamp;
            mLocalToRemoteTimeDifference = (remoteNow - localNow);

            // [Link Event]
            LinkEventConnectResponded(TransmissionDirection::Incoming, connectResponseData);

            // Handle connect response
            switch(connectResponseData.mConnectResponse)
            {
            // Positive connect response?
            case ConnectResponse::Accept:
              // Change link state
              SetState(LinkState::Connected);
              break;

            // Negative connect response?
            default:
              Assert(false);
            case ConnectResponse::Deny:
            case ConnectResponse::DenyFull:
              // Change link state
              SetState(LinkState::ReceivedNegativeConnectResponse);
              break;
            }

            // [Link Plugin Event]
            PluginEventOnConnectResponseReceive(message);
            break;
          }
        }

      // Else, Disconnect requested or (RTT/2) * ConnectAttemptFactor has elapsed?
      if(mState == LinkState::SentConnectRequest)
      {
        if(mDisconnectRequested)
        {
          // Change link state
          SetState(LinkState::Disconnected);
        }
#if PEER_LINK_ENABLE_FAIL_CONNECT_VIA_TIMEOUT
        else if(GetStateDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetConnectAttemptFactor()))
        {
          // Create connect response message
          Message connectResponseMessage(ProtocolMessageType::ConnectResponse);

          ConnectResponseData connectResponseData;
          connectResponseData.mIpAddress       = GetInternetProtocol() == InternetProtocol::V4 ? GetPeer()->GetLocalIpv4Address() : GetPeer()->GetLocalIpv6Address();
          connectResponseData.mTimestamp       = now;
          connectResponseData.mConnectResponse = ConnectResponse::DenyTimeout;

          connectResponseMessage.GetData().Write(connectResponseData);

          // [Link Event]
          LinkEventConnectResponded(TransmissionDirection::Incoming, connectResponseData);

          // Change link state
          SetState(LinkState::ReceivedNegativeConnectResponse);

          // [Link Plugin Event]
          PluginEventOnConnectResponseReceive(connectResponseMessage);
        }
#endif
      }
    }
    break;

  case LinkState::ReceivedConnectRequest:
    {
      //
      // Reply to connect request or timeout
      //

      // Determine the appropriate connect response with respect to our peer's desired connect response mode
      UserConnectResponse::Enum response = UserConnectResponse::Pending;
      BitStream                 extraData;
      switch(GetPeer()->GetConnectResponseMode())
      {
      case ConnectResponseMode::Accept:
        response = UserConnectResponse::Accept;
        break;

      case ConnectResponseMode::Deny:
        response = UserConnectResponse::Deny;
        break;

      default:
        Assert(false);
      case ConnectResponseMode::Custom:
        response  = mConnectResponded;
        extraData = mConnectResponseExtraData;
        break;
      }

      // Connect response determined?
      if(response != UserConnectResponse::Pending)
      {
        // Create connect response message
        Message connectResponseMessage(ProtocolMessageType::ConnectResponse);

        ConnectResponseData connectResponseData;
        connectResponseData.mIpAddress       = GetTheirIpAddress();
        connectResponseData.mTimestamp       = now;
        connectResponseData.mConnectResponse = (response == UserConnectResponse::Accept) ? ConnectResponse::Accept : ConnectResponse::Deny;
        connectResponseData.mExtraData       = extraData;

        connectResponseMessage.GetData().Write(connectResponseData);

        // Store connect response message copy
        Message connectResponseMessageCopy(connectResponseMessage);

        // Send connect response message
        Status status;
        MessageReceiptId receiptId = SendInternal(status, ZeroMove(connectResponseMessage), true, 0, true);

        // [Link Event]
        LinkEventConnectResponded(TransmissionDirection::Outgoing, connectResponseData);

        // Positive connect response?
        if(connectResponseData.mConnectResponse == ConnectResponse::Accept)
        {
          // Change link state
          SetState(LinkState::Connected);
        }
        // Negative connect response?
        else
        {
          mStateACKId = receiptId;

          // Change link state
          SetState(LinkState::SentNegativeConnectResponse);
        }

        // [Link Plugin Event]
        PluginEventOnConnectResponseSend(connectResponseMessageCopy);
      }

      // Else, Disconnect requested or (RTT/2) * ConnectAttemptFactor has elapsed?
      if(mState == LinkState::ReceivedConnectRequest)
      {
        if(mDisconnectRequested
        || GetStateDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetConnectAttemptFactor()))
        {
          // Change link state
          SetState(LinkState::Disconnected);
        }
      }
    }
    break;

  //--------------------------------------//
  //        LinkStatus::Connected         //
  //--------------------------------------//
  case LinkState::Connected:
    {
      //
      // Check for disconnection via request, timeout, or latency
      //

      // Received disconnect notice message?
      forRange(Message& message, protocolMessages.All())
        if(message.GetType() == ProtocolMessageType::DisconnectNotice)
        {
          // Read disconnect notice message
          DisconnectNoticeData disconnectNoticeData;
          if(message.GetData().Read(disconnectNoticeData)) // Successful?
          {
            // Clear bits read
            message.GetData().ClearBitsRead();

            // [Link Event]
            LinkEventDisconnectNoticed(TransmissionDirection::Incoming, disconnectNoticeData);

            // Change link state
            SetState(LinkState::ReceivedDisconnectNotice);

            // [Link Plugin Event]
            PluginEventOnDisconnectNoticeReceive(message);
            break;
          }
        }

      // Else
      if(mState == LinkState::Connected)
      {
        // Determine if we should disconnect
        bool                   shouldDisconnect = false;
        DisconnectReason::Enum disconnectReason = DisconnectReason::Request;

        // Outgoing disconnect requested?
        if(mDisconnectRequested)
        {
          shouldDisconnect = true;
          disconnectReason = DisconnectReason::Request;
        }
#if PEER_LINK_ENABLE_DISCONNECT_VIA_LATENCY
        // RTT exceeds LatencyLimit?
        else if(GetAvgInternalRoundTripTime() > GetLatencyLimit())
        {
          shouldDisconnect = true;
          disconnectReason = DisconnectReason::Latency;
        }
#endif
#if PEER_LINK_ENABLE_DISCONNECT_VIA_TIMEOUT
        // (RTT/2) * TimeoutFactor elapsed without receiving any Packets?
        else if(mInbox.GetLastReceiveDuration() > ((GetAvgInternalRoundTripTime() / 2) * GetTimeoutFactor()))
        {
          shouldDisconnect = true;
          disconnectReason = DisconnectReason::Timeout;
        }
#endif

        // Should we disconnect?
        if(shouldDisconnect)
        {
          // Create disconnect notice message
          Message disconnectNoticeMessage(ProtocolMessageType::DisconnectNotice);

          DisconnectNoticeData disconnectNoticeData;
          disconnectNoticeData.mDisconnectReason = disconnectReason;
          disconnectNoticeData.mExtraData        = mDisconnectRequested ? mDisconnectRequestExtraData : BitStream();

          disconnectNoticeMessage.GetData().Write(disconnectNoticeData);

          // Store disconnect notice message copy
          Message disconnectNoticeMessageCopy(disconnectNoticeMessage);

          // Send disconnect notice message
          Status status;
          mStateACKId = SendInternal(status, ZeroMove(disconnectNoticeMessage), true, 0, true);

          // [Link Event]
          LinkEventDisconnectNoticed(TransmissionDirection::Outgoing, disconnectNoticeData);

          // Change link state
          SetState(LinkState::SentDisconnectNotice);

          // [Link Plugin Event]
          PluginEventOnDisconnectNoticeSend(disconnectNoticeMessageCopy);
        }
      }
    }
    break;
  } // (Update Link State)
  protocolMessages.Clear();

  //
  // Send Outgoing Packets
  //
  mOutbox.Update(remoteACKs, remoteNAKs);

  //
  // Update Plugins
  //
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnUpdate();
}
void PeerLink::ProcessReceivedCustomMessages()
{
  // Get received custom messages
  Array<Message>& customMessages = mInbox.mReleasedCustomMessages;
  if(customMessages.Empty()) // None?
    return;

  // For all custom messages
  Array<Message>::iterator iter = customMessages.Begin();
  for( ; iter != customMessages.End(); ++iter)
  {
    // Process custom message
    if(!ProcessReceivedCustomMessage(*iter, false)) // Stop processing?
    {
      ++iter; // (Advance to include this message in the erase below)
      break;
    }
  }

  // Erase processed messages
  customMessages.Erase(Array<Message>::range(customMessages.Begin(), iter));
}
bool PeerLink::ProcessReceivedCustomMessage(Message& message, bool isEvent)
{
  // Is an event message?
  if(isEvent)
  {
    // Let user process the custom message
    return GetPeer()->mProcessReceivedCustomMessageFn(this, message);
  }

  // Attempt to receive the custom message as a plugin message
  bool continueProcessingCustomMessages = true;
  bool belongsToPlugin = AttemptPluginMessageReceive(ZeroMove(message), continueProcessingCustomMessages);
  if(!belongsToPlugin) // Not a plugin message?
  {
    // Should be a user type
    if(AbsoluteIsInRange(message.GetType()))
    {
      // Convert absolute message type to relative message type
      message.mType = AbsoluteToRelative(message.mType);

      // Let user process the custom message
      return GetPeer()->mProcessReceivedCustomMessageFn(this, message);
    }
    else
      Assert(false); // (What type is this?)
  }

  // Continue to process next custom message if specified
  return continueProcessingCustomMessages;
}

void PeerLink::PluginEventOnConnectRequestSend(Message& message)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnConnectRequestSend(message);
}
void PeerLink::PluginEventOnConnectRequestReceive(Message& message)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnConnectRequestReceive(message);
}

void PeerLink::PluginEventOnConnectResponseSend(Message& message)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnConnectResponseSend(message);
}
void PeerLink::PluginEventOnConnectResponseReceive(Message& message)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnConnectResponseReceive(message);
}

void PeerLink::PluginEventOnDisconnectNoticeSend(Message& message)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnDisconnectNoticeSend(message);
}
void PeerLink::PluginEventOnDisconnectNoticeReceive(Message& message)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnDisconnectNoticeReceive(message);
}

void PeerLink::PluginEventOnStateChange(LinkState::Enum prevState)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnStateChange(prevState);
}
void PeerLink::PluginEventOnStatusChange(LinkStatus::Enum prevStatus)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnStatusChange(prevStatus);
}

bool PeerLink::PluginEventOnPacketSend(OutPacket& packet)
{
  // Ask all plugins if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(!plugin->OnPacketSend(packet)) // Stop?
      return false;

  // Continue
  return true;
}
bool PeerLink::PluginEventOnPacketReceive(InPacket& packet)
{
  // Ask all plugins if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(!plugin->OnPacketReceive(packet)) // Stop?
      return false;

  // Continue
  return true;
}

bool PeerLink::PluginEventOnMessageSend(OutMessage& message)
{
  // Ask all plugins if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(!plugin->OnMessageSend(message)) // Stop?
      return false;

  // Continue
  return true;
}
bool PeerLink::PluginEventOnMessageReceipt(OutMessage& message, Receipt::Enum receipt)
{
  // Ask all plugins if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(!plugin->OnMessageReceipt(message, receipt)) // Stop?
      return false;

  // Continue
  return true;
}
bool PeerLink::PluginEventOnMessageReceive(Message& message)
{
  // Ask all plugins if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(!plugin->OnMessageReceive(message)) // Stop?
      return false;

  // Continue
  return true;
}

bool PeerLink::PluginEventOnPluginMessageSend(OutMessage& message)
{
  // Ask all plugins if this message type is intended for them and if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(plugin->AbsoluteIsInRange(message.GetType())) // Belongs to this plugin?
    {
      // Convert absolute message type to relative message type
      message.mType = plugin->AbsoluteToRelative(message.mType);

      // Stop?
      bool result = plugin->OnPluginMessageSend(message);

      // Convert relative message type back to absolute message type
      message.mType = plugin->RelativeToAbsolute(message.mType);

      return result;
    }

  // Continue
  return true;
}

bool PeerLink::PluginEventOnIncomingChannelOpen(MessageChannelId channelId)
{
  // Ask all plugins if they wish to continue
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(!plugin->OnIncomingChannelOpen(channelId)) // Stop?
      return false;

  // Continue
  return true;
}
void PeerLink::PluginEventOnIncomingChannelClose(MessageChannelId channelId)
{
  // Notify all plugins
  forRange(LinkPlugin* plugin, mPlugins.All())
    plugin->OnIncomingChannelClose(channelId);
}

bool PeerLink::AttemptPluginMessageReceipt(MoveReference<OutMessage> message, Receipt::Enum receipt)
{
  // Ask all plugins if this message type is intended for them
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(plugin->AbsoluteIsInRange(message->GetType())) // Belongs to this plugin?
    {
      // Convert absolute message type to relative message type
      message->mType = plugin->AbsoluteToRelative(message->mType);

      // [Link Plugin Event]
      plugin->OnPluginMessageReceipt(ZeroMove(message), receipt);
      return true;
    }

  // Is user message
  return false;
}
bool PeerLink::AttemptPluginMessageReceive(MoveReference<Message> message, bool& continueProcessingCustomMessages)
{
  // Ask all plugins if this message type is intended for them
  forRange(LinkPlugin* plugin, mPlugins.All())
    if(plugin->AbsoluteIsInRange(message->GetType())) // Belongs to this plugin?
    {
      // Convert absolute message type to relative message type
      message->mType = plugin->AbsoluteToRelative(message->mType);

      // [Link Plugin Event]
      plugin->OnPluginMessageReceive(ZeroMove(message), continueProcessingCustomMessages);
      return true;
    }

  // Is user message
  return false;
}

void PeerLink::LinkEventConnectRequested(TransmissionDirection::Enum direction, const ConnectRequestData& connectRequestData)
{
  // Create connect requested event message
  Message connectRequestedMessage(LinkEventMessageType::ConnectRequested);

  ConnectRequestedData connectRequestedData;
  connectRequestedData.mDirection          = direction;
  connectRequestedData.mConnectRequestData = connectRequestData;

  connectRequestedMessage.GetData().Write(connectRequestedData);

  // Push connect requested event message
  PushUserEventMessage(ZeroMove(connectRequestedMessage));
}
void PeerLink::LinkEventConnectResponded(TransmissionDirection::Enum direction, const ConnectResponseData& connectResponseData)
{
  // Create connect responded event message
  Message connectRespondedMessage(LinkEventMessageType::ConnectResponded);

  ConnectRespondedData connectRespondedData;
  connectRespondedData.mDirection           = direction;
  connectRespondedData.mConnectResponseData = connectResponseData;

  connectRespondedMessage.GetData().Write(connectRespondedData);

  // Push connect responded event message
  PushUserEventMessage(ZeroMove(connectRespondedMessage));
}
void PeerLink::LinkEventDisconnectNoticed(TransmissionDirection::Enum direction, const DisconnectNoticeData& disconnectNoticeData)
{
  // Create disconnect noticed event message
  Message disconnectNoticedMessage(LinkEventMessageType::DisconnectNoticed);

  DisconnectNoticedData disconnectNoticedData;
  disconnectNoticedData.mDirection            = direction;
  disconnectNoticedData.mDisconnectNoticeData = disconnectNoticeData;

  disconnectNoticedMessage.GetData().Write(disconnectNoticedData);

  // Push disconnect noticed event message
  PushUserEventMessage(ZeroMove(disconnectNoticedMessage));
}
void PeerLink::LinkEventIncomingChannelOpened(MessageChannelId channelId, TransferMode::Enum transferMode)
{
  // Create incoming channel opened event message
  Message incomingChannelOpenedMessage(LinkEventMessageType::IncomingChannelOpened);

  IncomingChannelOpenedData incomingChannelOpenedData;
  incomingChannelOpenedData.mChannelId    = channelId;
  incomingChannelOpenedData.mTransferMode = transferMode;

  incomingChannelOpenedMessage.GetData().Write(incomingChannelOpenedData);

  // Push incoming channel opened event message
  PushUserEventMessage(ZeroMove(incomingChannelOpenedMessage));
}
void PeerLink::LinkEventIncomingChannelClosed(MessageChannelId channelId)
{
  // Create incoming channel closed event message
  Message incomingChannelClosedMessage(LinkEventMessageType::IncomingChannelClosed);

  IncomingChannelClosedData incomingChannelClosedData;
  incomingChannelClosedData.mChannelId = channelId;

  incomingChannelClosedMessage.GetData().Write(incomingChannelClosedData);

  // Push incoming channel closed event message
  PushUserEventMessage(ZeroMove(incomingChannelClosedMessage));
}
void PeerLink::LinkEventStateChange(LinkState::Enum newState)
{
  // Create state change event message
  Message stateChangeMessage(LinkEventMessageType::StateChange);

  StateChangeData stateChangeData;
  stateChangeData.mNewState = newState;

  stateChangeMessage.GetData().Write(stateChangeData);

  // Push state change event message
  PushUserEventMessage(ZeroMove(stateChangeMessage));
}
void PeerLink::LinkEventStatusChange(LinkStatus::Enum newStatus)
{
  // Create status change event message
  Message statusChangeMessage(LinkEventMessageType::StatusChange);

  StatusChangeData statusChangeData;
  statusChangeData.mNewStatus = newStatus;

  statusChangeMessage.GetData().Write(statusChangeData);

  // Push status change event message
  PushUserEventMessage(ZeroMove(statusChangeMessage));
}
void PeerLink::LinkEventReceipt(MessageReceiptId receiptId, Receipt::Enum receipt, bool forUser)
{
  // Create receipt event message
  Message receiptMessage(LinkEventMessageType::Receipt);

  ReceiptData receiptData;
  receiptData.mReceiptId = receiptId;
  receiptData.mReceipt   = receipt;

  receiptMessage.GetData().Write(receiptData);

  // Push receipt event message
  if(forUser)
    PushUserEventMessage(ZeroMove(receiptMessage));
  else
    PushProtocolEventMessage(ZeroMove(receiptMessage));
}

void PeerLink::PushUserEventMessage(MoveReference<Message> message)
{
  mInbox.PushUserEventMessage(ZeroMove(message));
}
void PeerLink::PushProtocolEventMessage(MoveReference<Message> message)
{
  mInbox.PushProtocolEventMessage(ZeroMove(message));
}

//---------------------------------------------------------------------------------//
//                                 LinkPlugin                                      //
//---------------------------------------------------------------------------------//

LinkPlugin::~LinkPlugin()
{
}

//
// Member Functions
//

bool LinkPlugin::operator ==(const LinkPlugin& rhs) const
{
  return mName == rhs.mName;
}
bool LinkPlugin::operator !=(const LinkPlugin& rhs) const
{
  return mName != rhs.mName;
}
bool LinkPlugin::operator  <(const LinkPlugin& rhs) const
{
  return mName < rhs.mName;
}
bool LinkPlugin::operator ==(const String& rhs) const
{
  return mName == rhs;
}
bool LinkPlugin::operator !=(const String& rhs) const
{
  return mName != rhs;
}
bool LinkPlugin::operator  <(const String& rhs) const
{
  return mName < rhs;
}

const String& LinkPlugin::GetName() const
{
  return mName;
}

bool LinkPlugin::IsInitialized() const
{
  return mLink ? true : false;
}

PeerLink* LinkPlugin::GetLink() const
{
  return mLink;
}

MessageReceiptId LinkPlugin::Send(Status& status, const Message& message,         bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime)
{
  // Plugin not initialized?
  if(!IsInitialized())
  {
    // Failure
    status.SetFailed("Plugin is not initialized");
    return 0;
  }

  // Relative message type outside of plugin range?
  if(message.GetType() >= GetMessageTypeCount())
  {
    // Failure
    status.SetFailed("Plugin message type is out of range");
    return 0;
  }

  // Copy message
  Message messageCopy(message);

  // Convert relative message type to absolute message type
  messageCopy.mType = RelativeToAbsolute(messageCopy.mType);

  // Send message
  return mLink->SendInternal(status, ZeroMove(messageCopy), reliable, channelId, receipt, priority, lifetime, false);
}
MessageReceiptId LinkPlugin::Send(Status& status, MoveReference<Message> message, bool reliable, MessageChannelId channelId, bool receipt, MessagePriority priority, TimeMs lifetime)
{
  // Plugin not initialized?
  if(!IsInitialized())
  {
    // Failure
    status.SetFailed("Plugin is not initialized");
    return 0;
  }

  // Relative message type outside of plugin range?
  if(message->GetType() >= GetMessageTypeCount())
  {
    // Failure
    status.SetFailed("Plugin message type is out of range");
    return 0;
  }

  // Convert relative message type to absolute message type
  message->mType = RelativeToAbsolute(message->mType);

  // Send message
  return mLink->SendInternal(status, ZeroMove(message), reliable, channelId, receipt, priority, lifetime, false);
}

LinkPlugin::LinkPlugin(size_t messageTypeCount)
  : mName(),
    mLink(nullptr),
    mMessageTypeStart(0),
    mMessageTypeCount(MessageType(messageTypeCount))
{
}

//
// Internal
//

void LinkPlugin::SetName(const String& name)
{
  Assert(mName.Empty());
  mName = name;
}

bool LinkPlugin::Initialize(PeerLink* link)
{
  Assert(!IsInitialized());
  mLink = link;
  return OnInitialize();
}
void LinkPlugin::Uninitialize()
{
  Assert(IsInitialized());
  OnUninitialize();
  mLink = nullptr;
}

bool LinkPlugin::AbsoluteIsInRange(MessageType absoluteType) const
{
  if(!mMessageTypeCount)
    return false;

  if(absoluteType < mMessageTypeStart)
    return false;

  return AbsoluteToRelative(absoluteType) <= (mMessageTypeCount - 1);
}
MessageType LinkPlugin::RelativeToAbsolute(MessageType relativeType) const
{
  Assert(mMessageTypeCount);
  return relativeType + mMessageTypeStart;
}
MessageType LinkPlugin::AbsoluteToRelative(MessageType absoluteType) const
{
  Assert(mMessageTypeCount);
  Assert(absoluteType >= mMessageTypeStart);
  return absoluteType - mMessageTypeStart;
}

MessageType LinkPlugin::GetMessageTypeCount() const
{
  return mMessageTypeCount;
}
void LinkPlugin::SetMessageTypeStart(MessageType messageTypeStart)
{
  mMessageTypeStart = messageTypeStart;
}

} // namespace Zero
