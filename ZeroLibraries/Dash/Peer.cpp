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
//                                    Peer                                         //
//---------------------------------------------------------------------------------//

void Peer::ResetSession()
{
  /// Operating Data
  mIpv4Address.Clear();
  mIpv6Address.Clear();
  mInternetProtocol  = InternetProtocol::Unspecified;
  mTransportProtocol = TransportProtocol::Unspecified;

  /// Thread Data
  mFatalError = false;

  /// Packet Data
  mIpv4RawPackets.Clear();
  mIpv6RawPackets.Clear();
  mSendBitStream.Clear(false);

  InitializeStats();
}

Peer::Peer(ProcessReceivedCustomPacketFn processReceivedCustomPacketFn, ProcessReceivedCustomMessageFn processReceivedCustomMessageFn)
  : BandwidthStats<true>(),

    /// Operating Data
    mGuid(0), // TODO
    mProcessReceivedCustomPacketFn(processReceivedCustomPacketFn),
    mProcessReceivedCustomMessageFn(processReceivedCustomMessageFn),
    mIpv4Address(),
    mIpv6Address(),
    mIpv4Socket(),
    mIpv6Socket(),
    mInternetProtocol(InternetProtocol::Unspecified),
    mTransportProtocol(TransportProtocol::Unspecified),
    mUserData(nullptr),

    /// Thread Data
    mFatalError(false),
    mIpv4ReceiveThread(),
    mExitIpv4ReceiveThread(false),
    mIpv6ReceiveThread(),
    mExitIpv6ReceiveThread(false),

    /// State Data
    mLocalTimer(),
    mSendTimer(),
    mReceiveTimer(),
    mLocalFrameId(0),

    /// Packet Data
    mIpv4RawPackets(),
    mIpv4RawPacketsLock(),
    mIpv6RawPackets(),
    mIpv6RawPacketsLock(),
    mSendBitStream(),
    mReceiveStatsLock(),
    mReleasedCustomPackets(),
    mReleasedCustomPacketsLock(),

    /// Link Data
    mCreatedLinks(),
    mLinks(),
    mDestroyedLinks(),

    /// Plugin Data
    mAddedPlugins(),
    mPlugins(),
    mRemovedPlugins()
{
  Assert(mProcessReceivedCustomPacketFn && mProcessReceivedCustomMessageFn);
  ResetConfig();
  InitializeStats();

  // Initialize protocol ID
  GetProtocolId();
}

Peer::~Peer()
{
  // Close peer if anything is left open
  Close();
}

bool Peer::operator ==(const Peer& rhs) const
{
  return mGuid == rhs.mGuid;
}
bool Peer::operator !=(const Peer& rhs) const
{
  return mGuid != rhs.mGuid;
}
bool Peer::operator  <(const Peer& rhs) const
{
  return mGuid < rhs.mGuid;
}
bool Peer::operator ==(Guid rhs) const
{
  return mGuid == rhs;
}
bool Peer::operator !=(Guid rhs) const
{
  return mGuid != rhs;
}
bool Peer::operator  <(Guid rhs) const
{
  return mGuid < rhs;
}

//
// Member Functions
//

ProtocolId Peer::GetProtocolId()
{
  static const ProtocolId protocolId(String("Dash Peer").Hash());
  return protocolId;
}

Guid Peer::GetGuid() const
{
  return mGuid;
}

const IpAddress& Peer::GetLocalIpv4Address() const
{
  return mIpv4Address;
}

const IpAddress& Peer::GetLocalIpv6Address() const
{
  return mIpv6Address;
}

bool Peer::IsOpen() const
{
  return mIpv4Socket.IsOpen()
      || mIpv6Socket.IsOpen()
      || !mIpv4ReceiveThread.IsCompleted()
      || !mIpv6ReceiveThread.IsCompleted();
}

InternetProtocol::Enum Peer::GetInternetProtocol() const
{
  return mInternetProtocol;
}

TransportProtocol::Enum Peer::GetTransportProtocol() const
{
  return mTransportProtocol;
}

void Peer::Open(Status& status, ushort port, InternetProtocol::Enum internetProtocol, TransportProtocol::Enum transportProtocol)
{
  // TODO: Support TCP option
  UnusedParameter(transportProtocol);

  // Close peer if anything is open
  Close();

  //
  // Open Sockets
  //

  // Open IPv4 socket?
  if(internetProtocol == InternetProtocol::V4
  || internetProtocol == InternetProtocol::Both)
  {
    // Open IPv4 socket
    mIpv4Socket.Open(status, SocketAddressFamily::InternetworkV4, SocketType::Datagram, SocketProtocol::Udp);
    if(status.Succeeded()) // Successful?
    {
      // Specify automatic IPv4 host and specified port
      IpAddress ipAddress;
      ipAddress.SetHost(String(), InternetProtocol::V4);
      ipAddress.SetPort(port);

      // Bind IPv4 socket
      mIpv4Socket.Bind(status, ipAddress);
      if(status.Succeeded()) // Successful?
      {
        // Set IPv4 socket to blocking mode
        mIpv4Socket.SetBlocking(status, true);

        // Enable socket broadcast capability
        bool canBroadcast = true;
        mIpv4Socket.SetSocketOption(status, SocketOption::CanBroadcast, canBroadcast);
      }
    }
  }
  if(status.Failed()) // Unable?
  {
    // Close IPv4 socket
    Status status;
    mIpv4Socket.Close(status);
    return;
  }

  // Open IPv6 socket?
  if(internetProtocol == InternetProtocol::V6
  || internetProtocol == InternetProtocol::Both)
  {
    // Open IPv6 socket
    mIpv6Socket.Open(status, SocketAddressFamily::InternetworkV6, SocketType::Datagram, SocketProtocol::Udp);
    if(status.Succeeded()) // Successful?
    {
      // Specify automatic IPv6 host and specified port
      IpAddress ipAddress;
      ipAddress.SetHost(String(), InternetProtocol::V6);
      ipAddress.SetPort(port);

      // Bind IPv6 socket
      mIpv6Socket.Bind(status, ipAddress);
      if(status.Succeeded()) // Successful?
      {
        // Set IPv6 socket to blocking mode
        mIpv6Socket.SetBlocking(status, true);
      }
    }
  }
  if(status.Failed()) // Unable?
  {
    // Close IPv6 socket
    Status status;
    mIpv6Socket.Close(status);
    return;
  }

  //
  // Store Session Information
  //
  mInternetProtocol = InternetProtocol::Unspecified;

  // Both IPv4 and IPv6 sockets opened?
  if(mIpv4Socket.IsOpen() && mIpv6Socket.IsOpen())
  {
    mInternetProtocol = InternetProtocol::Both;
    mIpv4Address = mIpv4Socket.GetBoundLocalAddress();
    mIpv6Address = mIpv6Socket.GetBoundLocalAddress();
  }
  // Only IPv4 socket opened?
  else if(mIpv4Socket.IsOpen())
  {
    mInternetProtocol = InternetProtocol::V4;
    mIpv4Address = mIpv4Socket.GetBoundLocalAddress();
  }
  // Only IPv6 socket opened?
  else if(mIpv6Socket.IsOpen())
  {
    mInternetProtocol = InternetProtocol::V6;
    mIpv6Address = mIpv6Socket.GetBoundLocalAddress();
  }
  // Unable to open a socket?
  else
  {
    Close();
    return;
  }

  //
  // Launch Threads
  //

  // Using IPv4 socket?
  if(mIpv4Socket.IsOpen())
  {
    // Launch IPv4 receive thread
    mExitIpv4ReceiveThread = false;
    bool result = mIpv4ReceiveThread.Initialize(Thread::ObjectEntryCreator<Peer, &Peer::Ipv4ReceiveThreadFn>, this, "PeerIpv4ReceiveThread");
    if(!result) // Unable?
    {
      status.SetFailed("Unable to launch peer IPv4 receive thread");
      Close();
      return;
    }
    mIpv4ReceiveThread.Resume();
  }

  // Using IPv6 socket?
  if(mIpv6Socket.IsOpen())
  {
    // Launch IPv6 receive thread
    mExitIpv6ReceiveThread = false;
    bool result = mIpv6ReceiveThread.Initialize(Thread::ObjectEntryCreator<Peer, &Peer::Ipv6ReceiveThreadFn>, this, "PeerIpv6ReceiveThread");
    if(!result) // Unable?
    {
      status.SetFailed("Unable to launch peer IPv6 receive thread");
      Close();
      return;
    }
    mIpv6ReceiveThread.Resume();
  }

  // Update once to initialize links and plugins
  Update();
}

void Peer::Close()
{
  //
  // Disconnect Links
  //

  // Gracefully disconnect all links
  forRange(PeerLink* link, mLinks.All())
    link->Disconnect();

  // Update once to send graceful disconnects
  Update();

  //
  // Clear Links and Plugins
  //

  // Destroy all links
  DestroyLinks();

  // Remove all peer plugins
  RemovePlugins();

  // Update once to uninitialize links and plugins
  Update();

  // (All links should be destroyed)
  //Assert(mCreatedLinks.Empty());
  Assert(mLinks.Empty());
  Assert(mDestroyedLinks.Empty());

  // (All peer plugins should be destroyed)
  //Assert(mAddedPlugins.Empty());
  Assert(mPlugins.Empty());
  Assert(mRemovedPlugins.Empty());

  //
  // Unblock Receive Threads
  //

  // IPv4 receive thread running?
  if(!mIpv4ReceiveThread.IsCompleted())
  {
    mExitIpv4ReceiveThread = true;
    Assert(mIpv4Address.IsValid());
    OutPacket packet(mIpv4Address);
    SendPacket(packet);
  }

  // IPv6 receive thread running?
  if(!mIpv6ReceiveThread.IsCompleted())
  {
    mExitIpv6ReceiveThread = true;
    Assert(mIpv6Address.IsValid());
    OutPacket packet(mIpv6Address);
    SendPacket(packet);
  }

  //
  // Close Sockets
  //

  // IPv4 socket open?
  if(mIpv4Socket.IsOpen())
  {
    Status status;
    mIpv4Socket.Close(status);
    Assert(!mIpv4Socket.IsOpen());
  }

  // IPv6 socket open?
  if(mIpv6Socket.IsOpen())
  {
    Status status;
    mIpv6Socket.Close(status);
    Assert(!mIpv6Socket.IsOpen());
  }

  //
  // Close Receive Threads
  //

  // IPv4 receive thread running?
  if(!mIpv4ReceiveThread.IsCompleted())
  {
    // Close IPv4 receive thread
    mExitIpv4ReceiveThread = true;
    mIpv4ReceiveThread.WaitForCompletion();
    mIpv4ReceiveThread.Close();
    Assert(mIpv4ReceiveThread.IsCompleted());
  }

  // IPv6 receive thread running?
  if(!mIpv6ReceiveThread.IsCompleted())
  {
    // Close IPv6 receive thread
    mExitIpv6ReceiveThread = true;
    mIpv6ReceiveThread.WaitForCompletion();
    mIpv6ReceiveThread.Close();
    Assert(mIpv6ReceiveThread.IsCompleted());
  }

  // Reset all peer session data
  ResetSession();
}

bool Peer::Send(const IpAddress& ipAddress, const Message& message)
{
  // Not open?
  if(!IsOpen())
    return false;

  // Create standalone outgoing packet
  OutPacket outPacket(ipAddress, true);

  // Copy message
  Message messageCopy(message);

  // Add message
  outPacket.mMessages.PushBack(OutMessage(ZeroMove(messageCopy)));

  // Send outgoing packet
  return SendPacket(outPacket);
}
bool Peer::Send(const IpAddress& ipAddress, const Array<Message>& messages)
{
  // Not open?
  if(!IsOpen())
    return false;

  // Create standalone outgoing packet
  OutPacket outPacket(ipAddress, true);

  // For all messages
  forRange(Message& message, messages.All())
  {
    // Copy message
    Message messageCopy(message);

    // Add message
    outPacket.mMessages.PushBack(OutMessage(ZeroMove(messageCopy)));
  }

  // Send outgoing packet
  return SendPacket(outPacket);
}

bool Peer::Update()
{
  // Fatal error occurred since last update call?
  if(mFatalError)
  {
    Close();
    return false;
  }

  // Peer not open?
  if(!IsOpen())
    return false;

  // Update current local time and frame ID
  UpdateAndGetLocalTime();
  ++mLocalFrameId;

  // Update peer state and process received custom packets
  UpdatePeerState();
  ProcessReceivedCustomPackets();

  // Success
  return true;
}

TimeMs Peer::GetLocalTime() const
{
  return mLocalTimer.TimeMilliseconds();
}
TimeMs Peer::GetLocalDeltaTime() const
{
  return mLocalTimer.TimeDeltaMilliseconds();
}
uint64 Peer::GetLocalFrameId() const
{
  return mLocalFrameId;
}

void Peer::SetUserData(void* userData)
{
  mUserData = userData;
}
void* Peer::GetUserData() const
{
  return mUserData;
}

//
// Peer Link Management
//

PeerLink* Peer::CreateLink(const IpAddress& ipAddress)
{
  // Link of that IP address is already active?
  if(mLinks.FindPointer(ipAddress))
    return nullptr;

  // Link of that IP address was just created?
  if(mCreatedLinks.FindPointer(ipAddress))
    return nullptr;

  // Push new link to created links
  // Will be added later on update
  PeerLink* link = new PeerLink(this, ipAddress, TransmissionDirection::Outgoing);
  PeerLinkSet::pointer_bool_pair result = mCreatedLinks.Insert(link);
  Assert(result.second); // (Insertion should have succeeded)

  // Success
  return *result.first;
}

PeerLink* Peer::GetLink(const IpAddress& ipAddress) const
{
  // Find active link
  PeerLinkSet::pointer result = mLinks.FindPointer(ipAddress);
  if(result) // Found?
    return *result;

  // Failure
  return nullptr;
}
PeerLinkSet Peer::GetLinks() const
{
  return mLinks;
}
uint Peer::GetLinkCount(LinkStatus::Enum linkStatus) const
{
  uint result = 0;

  forRange(PeerLink* link, mLinks.All())
    if(link->GetStatus() == linkStatus)
      ++result;

  return result;
}
uint Peer::GetLinkCount() const
{
  return mLinks.Size();
}

void Peer::DestroyLink(const IpAddress& ipAddress)
{
  // Find active link
  PeerLinkSet::iterator iter = mLinks.FindIterator(ipAddress);
  if(iter != mLinks.End()) // Found?
  {
    // Push link to destroyed links (if it hasn't already been added)
    // Will be deleted later on update
    PeerLinkSet::pointer_bool_pair result = mDestroyedLinks.Insert(*iter);
  }
}
void Peer::DestroyLinks()
{
  // Push link to destroyed links (if it hasn't already been added)
  // Will be deleted later on update
  forRange(PeerLink* link, mLinks.All())
    mDestroyedLinks.Insert(link);
}

//
// Peer Plugin Management
//

PeerPlugin* Peer::AddPlugin(PeerPlugin* plugin, StringParam name)
{
  // Plugin of that name is already active?
  if(mPlugins.FindPointer(name))
    return nullptr;

  // Plugin of that name was just added?
  if(mAddedPlugins.FindPointer(name))
    return nullptr;

  // Add new plugin to added plugins
  // Will be initialized later on update
  plugin->SetName(name);
  PeerPluginSet::pointer_bool_pair result = mAddedPlugins.Insert(plugin);
  Assert(result.second); // (Insertion should have succeeded)

  // Success
  return *result.first;
}

PeerPlugin* Peer::GetPlugin(StringParam name) const
{
  // Find active plugin
  PeerPluginSet::pointer result = mPlugins.FindPointer(name);
  if(result) // Found?
    return *result;

  // Failure
  return nullptr;
}
PeerPluginSet Peer::GetPlugins() const
{
  return mPlugins;
}
uint Peer::GetPluginCount() const
{
  return mPlugins.Size();
}

void Peer::RemovePlugin(StringParam name)
{
  // Find active plugin
  PeerPluginSet::iterator iter = mPlugins.FindIterator(name);
  if(iter != mPlugins.End()) // Found?
  {
    // Add plugin to removed plugins (if it hasn't already been added)
    // Will be deleted later on update
    PeerPluginSet::pointer_bool_pair result = mRemovedPlugins.Insert(*iter);
  }
}
void Peer::RemovePlugins()
{
  // Add plugin to removed plugins (if it hasn't already been added)
  // Will be deleted later on update
  forRange(PeerPlugin* plugin, mPlugins.All())
    mRemovedPlugins.Insert(plugin);
}

//
// Peer Configuration
//

void Peer::ResetConfig()
{
  SetLinkLimit();
  SetConnectionLimit();
  SetConnectResponseMode();
}

void Peer::SetLinkLimit(uint linkLimit)
{
  mLinkLimit = linkLimit;
}
uint Peer::GetLinkLimit() const
{
  return mLinkLimit;
}

void Peer::SetConnectionLimit(uint connectionLimit)
{
  mConnectionLimit = connectionLimit;
}
uint Peer::GetConnectionLimit() const
{
  return mConnectionLimit;
}

void Peer::SetConnectResponseMode(ConnectResponseMode::Enum connectResponseMode)
{
  mConnectResponseMode = uint32(connectResponseMode);
}
ConnectResponseMode::Enum Peer::GetConnectResponseMode() const
{
  return ConnectResponseMode::Enum(uint32(mConnectResponseMode));
}

Array< Pair<String, String> > Peer::GetConfigSummary() const
{
  // TODO
  return Array< Pair<String, String> >();
}
String Peer::GetConfigSummaryString() const
{
  // TODO
  return String();
}

//
// Peer Statistics
//

void Peer::ResetStats()
{
  BandwidthStats<true>::ResetStats();

  mLinksUpdated = false;
  mLinksMin     = 0;
  mLinksAvg     = 0;
  mLinksMax     = 0;

  mConnectionsUpdated = false;
  mConnectionsMin     = 0;
  mConnectionsAvg     = 0;
  mConnectionsMax     = 0;

  // For all links
  forRange(PeerLink* link, mLinks.All())
    link->ResetStats(); // Reset their stats
}

uint Peer::GetMinLinks() const
{
  return mLinksMin;
}
float Peer::GetAvgLinks() const
{
  return mLinksAvg;
}
uint Peer::GetMaxLinks() const
{
  return mLinksMax;
}

uint Peer::GetMinConnections() const
{
  return mConnectionsMin;
}
float Peer::GetAvgConnections() const
{
  return mConnectionsAvg;
}
uint Peer::GetMaxConnections() const
{
  return mConnectionsMax;
}

Array< Pair< String, Array<String> > > Peer::GetStatsSummary() const
{
  // TODO
  return Array< Pair< String, Array<String> > >();
}
String Peer::GetStatsSummaryString() const
{
  // TODO
  return String();
}

//
// Internal
//

void Peer::InitializeStats()
{
  ResetStats();
}

void Peer::UpdateLinks(uint32 sample)
{
  if(mLinksUpdated)
  {
    mLinksMin = std::min(uint32(mLinksMin), sample);
    mLinksAvg = Average(float(mLinksAvg), float(sample), 0.2);
    mLinksMax = std::max(uint32(mLinksMax), sample);
  }
  else
  {
    mLinksMin     = sample;
    mLinksAvg     = float(sample);
    mLinksMax     = sample;
    mLinksUpdated = true;
  }
}
void Peer::UpdateConnections(uint32 sample)
{
  if(mConnectionsUpdated)
  {
    mConnectionsMin = std::min(uint32(mConnectionsMin), sample);
    mConnectionsAvg = Average(float(mConnectionsAvg), float(sample), 0.2);
    mConnectionsMax = std::max(uint32(mConnectionsMax), sample);
  }
  else
  {
    mConnectionsMin     = sample;
    mConnectionsAvg     = float(sample);
    mConnectionsMax     = sample;
    mConnectionsUpdated = true;
  }
}

TimeMs Peer::UpdateAndGetLocalTime()
{
  return mLocalTimer.UpdateAndGetTimeMilliseconds();
}
TimeMs Peer::UpdateAndGetSendTime()
{
  return mSendTimer.UpdateAndGetTimeMilliseconds();
}
TimeMs Peer::UpdateAndGetReceiveTime()
{
  return mReceiveTimer.UpdateAndGetTimeMilliseconds();
}

bool Peer::SendPacket(OutPacket& outPacket)
{
  // [Peer Plugin Event] Stop?
  if(!PluginEventOnPacketSend(outPacket))
    return true;

  // Write packet to bitstream
  mSendBitStream.Write(outPacket);

  // Choose correct socket (IPv4 or IPv6)
  Socket& socket = outPacket.GetDestinationIpAddress().GetInternetProtocol() == InternetProtocol::V4
                 ? mIpv4Socket
                 : mIpv6Socket;

  // Send packet over socket
  Status status;
  Bytes result = socket.SendTo(status, mSendBitStream.GetData(), mSendBitStream.GetBytesWritten(), outPacket.GetDestinationIpAddress());
  if(result) // Successful?
  {
    Assert(status.Succeeded());
    Assert(result == mSendBitStream.GetBytesWritten());

    // Update stats
    UpdateSendStats(result);
  }

  // Clear for next send
  mSendBitStream.Clear(false);
  return (result != 0);
}

void Peer::UpdateSendStats(Bytes sentPacketBytes)
{
  // Update current send time
  TimeMs sendNow = UpdateAndGetSendTime();
  double sendDt  = mSendTimer.TimeDelta();

  // Update stats
  UpdatePacketsSent();
  UpdateOutgoingBandwidthUsage(double(BYTES_TO_BITS(sentPacketBytes)) / sendDt / double(1000) * double(cOneSecondTimeMs));
  UpdateSendRate(uint(cOneSecondTimeMs / sendDt));
  UpdateSentPacketBytes(sentPacketBytes);
}
void Peer::UpdateReceiveStats(Bytes receivedPacketBytes)
{
//<>-<>-<>-<>-< Receive Stats Locked >-<>-<>-<>-<>-
  Lock lock(mReceiveStatsLock);

  // Update current receive time
  TimeMs receiveNow = UpdateAndGetReceiveTime();
  double receiveDt  = mReceiveTimer.TimeDelta();

  // Update stats
  UpdatePacketsReceived();
  UpdateIncomingBandwidthUsage(double(BYTES_TO_BITS(receivedPacketBytes)) / receiveDt / double(1000) * double(cOneSecondTimeMs));
  UpdateReceiveRate(uint(cOneSecondTimeMs / receiveDt));
  UpdateReceivedPacketBytes(receivedPacketBytes);

//-<>-<>-<>-<>-< Receive Stats Unlocked >-<>-<>-<>-<>
}

bool Peer::IsValidRawPacket(RawPacket& rawPacket)
{
  rawPacket.mData.ClearBitsRead();

  // Packet is too small to have been written from our protocol?
  if(rawPacket.mData.GetBitsWritten() < MinPacketHeaderBits)
  {
    // Invalid packet
    return false;
  }

  // Read packet header's protocol ID
  ProtocolId protocolId = 0;
  if(!rawPacket.mData.Read(protocolId)) // Unable?
  {
    // Invalid packet
    Assert(false);
    rawPacket.mData.ClearBitsRead();
    return false;
  }
  rawPacket.mData.ClearBitsRead();

  // Packet's protocol ID does not match ours?
  if(protocolId != GetProtocolId())
  {
    // Invalid packet
    return false;
  }

  // Valid packet
  return true;
}

OsInt Peer::Ipv4ReceiveThreadFn()
{
try
{
  //
  // Receive Loop
  //
  RawPacket rawPacket;
  rawPacket.mData.Reserve(EthernetMtuBytes);
  while(!mExitIpv4ReceiveThread)
  {
    // Wait to receive a packet over socket
    Status status;
    SocketAddress sourceAddress;
    Bytes result = mIpv4Socket.ReceiveFrom(status, rawPacket.mData.GetDataExposed(), EthernetMtuBytes, sourceAddress);
    rawPacket.mData.SetBytesWritten(result);
    rawPacket.mIpAddress = sourceAddress;
    if(result && IsValidRawPacket(rawPacket)) // Successful?
    {
      Assert(rawPacket.mIpAddress.IsValid());

      { //<>-<>-<>-<>-< IPv4 Raw Packets Locked >-<>-<>-<>-<>-
        Lock lock(mIpv4RawPacketsLock);

        // Push raw packet copy
        mIpv4RawPackets.PushBack(rawPacket);

      } //-<>-<>-<>-<>-< IPv4 Raw Packets Unlocked >-<>-<>-<>-<>

      // Update stats
      UpdateReceiveStats(result);
    }

    // Clear for next receive
    rawPacket.mIpAddress.Clear();
    rawPacket.mData.Clear(false);
  }

  // Success
  return 0;
}
catch(const std::exception& error)
{
  // [Peer Event]
  PeerEventFatalError(error.what());
}
catch(...)
{
  // [Peer Event]
  PeerEventFatalError("Unknown IPv4 receive thread error");
}
  // Failure
  return 1;
}
OsInt Peer::Ipv6ReceiveThreadFn()
{
try
{
  //
  // Receive Loop
  //
  RawPacket rawPacket;
  rawPacket.mData.Reserve(EthernetMtuBytes);
  while(!mExitIpv6ReceiveThread)
  {
    // Wait to receive a packet over socket
    Status status;
    Bytes result = mIpv6Socket.ReceiveFrom(status, rawPacket.mData.GetDataExposed(), EthernetMtuBytes, rawPacket.mIpAddress);
    rawPacket.mData.SetBytesWritten(result);
    if(result && IsValidRawPacket(rawPacket)) // Successful?
    {
      Assert(rawPacket.mIpAddress.IsValid());

      { //<>-<>-<>-<>-< IPv6 Raw Packets Locked >-<>-<>-<>-<>-
        Lock lock(mIpv6RawPacketsLock);

        // Push raw packet copy
        mIpv6RawPackets.PushBack(rawPacket);

      } //-<>-<>-<>-<>-< IPv6 Raw Packets Unlocked >-<>-<>-<>-<>

      // Update stats
      UpdateReceiveStats(result);
    }

    // Clear for next receive
    rawPacket.mIpAddress.Clear();
    rawPacket.mData.Clear(false);
  }

  // Success
  return 0;
}
catch(const std::exception& error)
{
  // [Peer Event]
  PeerEventFatalError(error.what());
}
catch(...)
{
  // [Peer Event]
  PeerEventFatalError("Unknown IPv6 receive thread error");
}
  // Failure
  return 1;
}

void Peer::UpdatePeerState()
{
  //
  // Update Peer
  //
  Array<RawPacket> rawPackets;
  Array<InPacket>  inPackets;
  TimeMs           elapsedExitGraceDuration = 0;
  TimeMs           lastExitGraceTime        = 0;
  mSendBitStream.Reserve(EthernetMtuBytes);

  //
  // Update Plugin Set
  //
  {
    //
    // Remove Plugins
    //
    forRange(PeerPlugin* plugin, mRemovedPlugins.All())
    {
      // Uninitialize plugin
      plugin->Uninitialize();

      // Remove plugin from active plugins
      PeerPluginSet::pointer_bool_pair result = mPlugins.EraseValue(plugin);
      Assert(result.second); // (Erase should have succeeded)

      // Should this plugin be deleted?
      if(plugin->ShouldDeleteAfterRemoval())
      {
        // Delete plugin
        delete plugin;
      }
    }
    mRemovedPlugins.Clear();

    //
    // Add Plugins
    //
    forRange(PeerPlugin* plugin, mAddedPlugins.All())
    {
      // Initialize plugin
      if(plugin->Initialize(this)) // Continue?
      {
        // Add plugin to active plugins
        PeerPluginSet::pointer_bool_pair result = mPlugins.Insert(plugin);
        Assert(result.second);  // (Insertion should have succeeded)
      }
      // Stop?
      else
      {
        // Uninitialize plugin
        plugin->Uninitialize();

        // Should this plugin be deleted?
        if(plugin->ShouldDeleteAfterRemoval())
        {
          // Delete plugin
          delete plugin;
        }
      }
    }
    mAddedPlugins.Clear();
  }

  //
  // Update Link Set
  //
  {
    //
    // Remove Links
    //
    forRange(PeerLink* link, mDestroyedLinks.All())
    {
      // [Peer Plugin Event]
      PluginEventOnLinkRemove(link);

      // Remove link from active links
      PeerLinkSet::pointer_bool_pair result = mLinks.EraseValue(link);
      Assert(result.second); // (Erase should have succeeded)

      // Delete link
      delete link;
    }
    mDestroyedLinks.Clear();

    //
    // Add Links
    //
    forRange(PeerLink* link, mCreatedLinks.All())
    {
      // [Peer Plugin Event] Continue?
      if(PluginEventOnLinkAdd(link))
      {
        // Add link to active links
        PeerLinkSet::pointer_bool_pair result = mLinks.Insert(link);
        Assert(result.second);  // (Insertion should have succeeded)
      }
      // Stop?
      else
      {
        // [Peer Plugin Event]
        PluginEventOnLinkRemove(link);

        // Delete link
        delete link;
      }
    }
    mCreatedLinks.Clear();
  }

  //
  // Translate Raw IPv4 Packets
  //
  Assert(rawPackets.Empty());
  { //<>-<>-<>-<>-< IPv4 Raw Packets Locked >-<>-<>-<>-<>-
    Lock lock(mIpv4RawPacketsLock);

    // Get raw IPv4 packets
    rawPackets.Swap(mIpv4RawPackets);

  } //-<>-<>-<>-<>-< IPv4 Raw Packets Unlocked >-<>-<>-<>-<>

  // Translate raw IPv4 packets
  TranslateRawPackets(rawPackets, inPackets);

  //
  // Translate Raw IPv6 Packets
  //
  Assert(rawPackets.Empty());
  { //<>-<>-<>-<>-< IPv6 Raw Packets Locked >-<>-<>-<>-<>-
    Lock lock(mIpv6RawPacketsLock);

    // Get raw IPv6 packets
    rawPackets.Swap(mIpv6RawPackets);

  } //-<>-<>-<>-<>-< IPv6 Raw Packets Unlocked >-<>-<>-<>-<>

  // Translate raw IPv6 packets
  TranslateRawPackets(rawPackets, inPackets);

  //
  // Process Received Packets
  //
  forRange(InPacket& inPacket, inPackets.All())
  {
    // [Peer Plugin Event] Stop?
    if(!PluginEventOnPacketReceive(inPacket))
      continue;

    // Is a standalone packet?
    if(inPacket.IsStandalone())
    {
      // Let the user process the custom packet
      ProcessReceivedCustomPacket(inPacket);
      continue;
    }

    // Get the link representing this packet's remote peer
    PeerLink* link = mLinks.FindValue(inPacket.GetSourceIpAddress(), nullptr);
    if(!link) // Doesn't exist?
    {
      // Link limit already reached?
      if(mLinks.Size() >= GetLinkLimit())
        continue; // Ignore packet

      // Create new incoming link
      link = new PeerLink(this, inPacket.GetSourceIpAddress(), TransmissionDirection::Incoming);

      // [Peer Plugin Event] Continue?
      if(PluginEventOnLinkAdd(link))
      {
        // Add link to active links
        PeerLinkSet::pointer_bool_pair result = mLinks.Insert(link);
        Assert(result.second);  // (Insertion should have succeeded)
      }
      else
      {
        // [Peer Plugin Event]
        PluginEventOnLinkRemove(link);

        // Delete link
        delete link;

        // Ignore packet
        continue;
      }

      // [Peer Event]
      PeerEventIncomingLinkCreated(inPacket.GetSourceIpAddress());
    }

    // Push received packet into link to be processed later
    link->ReceivePacket(ZeroMove(inPacket));
  }
  inPackets.Clear();

  //
  // Update Links
  //
  forRange(PeerLink* link, mLinks.All())
  {
    // Update link state and process received custom messages
    link->UpdateLinkState();
    link->ProcessReceivedCustomMessages();
  }

  // Update stats
  UpdateLinks(mLinks.Size());
  UpdateConnections(GetLinkCount(LinkStatus::Connected));

  //
  // Update Plugins
  //
  forRange(PeerPlugin* plugin, mPlugins.All())
    plugin->OnUpdate();
}
void Peer::ProcessReceivedCustomPackets()
{
  // Array<RawPacket> customPackets;
  // Assert(customPackets.Empty());
  // { //<>-<>-<>-<>-< Released Raw Packets Locked >-<>-<>-<>-<>-
  //   Lock lock(mReleasedCustomPacketsLock);
  // 
  //   // Get custom packets
  //   customPackets.Swap(mReleasedCustomPackets);
  // 
  // } //-<>-<>-<>-<>-< Released Raw Packets Unlocked >-<>-<>-<>-<>
  // 
  // // For every custom packet
  // forRange(RawPacket& packet, customPackets.All())
  // {
  //   // Process custom packet
  //   ProcessReceivedCustomPacket(packet);
  // }
}
void Peer::ProcessReceivedCustomPacket(InPacket& packet)
{
  // Let user process the custom packet
  return mProcessReceivedCustomPacketFn(this, packet);
}

void Peer::TranslateRawPackets(Array<RawPacket>& rawPackets, Array<InPacket>& inPackets)
{
  // For all RawPackets
  forRange(RawPacket& rawPacket, rawPackets.All())
  {
    // Read as InPacket
    InPacket inPacket(rawPacket.mIpAddress);
    if(rawPacket.mData.Read(inPacket)) // Successful?
      inPackets.PushBack(ZeroMove(inPacket));
  }
  rawPackets.Clear();
}

bool Peer::PluginEventOnPacketSend(OutPacket& packet)
{
  // Ask all plugins if they wish to continue
  forRange(PeerPlugin* plugin, mPlugins.All())
    if(!plugin->OnPacketSend(packet)) // Stop?
      return false;

  // Continue
  return true;
}
bool Peer::PluginEventOnPacketReceive(InPacket& packet)
{
  // Ask all plugins if they wish to continue
  forRange(PeerPlugin* plugin, mPlugins.All())
    if(!plugin->OnPacketReceive(packet)) // Stop?
      return false;

  // Continue
  return true;
}

bool Peer::PluginEventOnLinkAdd(PeerLink* link)
{
  // Ask all plugins if they wish to continue
  forRange(PeerPlugin* plugin, mPlugins.All())
    if(!plugin->OnLinkAdd(link)) // Stop?
      return false;

  // Continue
  return true;
}
void Peer::PluginEventOnLinkRemove(PeerLink* link)
{
  // Notify all plugins
  forRange(PeerPlugin* plugin, mPlugins.All())
    plugin->OnLinkRemove(link);
}

void Peer::PeerEventIncomingLinkCreated(const IpAddress& ipAddress)
{
  // Create incoming link created event message
  Message incomingLinkCreatedMessage(PeerEventMessageType::IncomingLinkCreated);

  IncomingLinkCreatedData incomingLinkCreatedData;
  incomingLinkCreatedData.mIpAddress = ipAddress;

  incomingLinkCreatedMessage.GetData().Write(incomingLinkCreatedData);

  // Push incoming link created event message
  PushUserEventMessage(ZeroMove(incomingLinkCreatedMessage));
}
void Peer::PeerEventFatalError(const String& errorString)
{
  // Create fatal error event message
  Message fatalErrorMessage(PeerEventMessageType::FatalError);

  FatalErrorData fatalErrorData;
  fatalErrorData.mErrorString = errorString;

  fatalErrorMessage.GetData().Write(fatalErrorData);

  // Push fatal error event message
  PushUserEventMessage(ZeroMove(fatalErrorMessage));

  // Set fatal error flag (will close peer on the next user update call)
  mFatalError = true;
}

void Peer::PushUserEventMessage(MoveReference<Message> message)
{
// //<>-<>-<>-<>-< Released Raw Packets Locked >-<>-<>-<>-<>-
//   Lock lock(mReleasedCustomPacketsLock);
// 
//   // Create raw packet containing just the event message
//   RawPacket rawPacket;
//   rawPacket.mContainsEventMessage = true;
//   rawPacket.mData.Write(*message);
// 
//   // Release raw packet to the user
//   mReleasedCustomPackets.PushBack(ZeroMove(rawPacket));
// //-<>-<>-<>-<>-< Released Raw Packets Unlocked >-<>-<>-<>-<>
}

//---------------------------------------------------------------------------------//
//                                 PeerPlugin                                      //
//---------------------------------------------------------------------------------//

PeerPlugin::~PeerPlugin()
{
}

//
// Member Functions
//

bool PeerPlugin::operator ==(const PeerPlugin& rhs) const
{
  return mName == rhs.mName;
}
bool PeerPlugin::operator !=(const PeerPlugin& rhs) const
{
  return mName != rhs.mName;
}
bool PeerPlugin::operator  <(const PeerPlugin& rhs) const
{
  return mName < rhs.mName;
}
bool PeerPlugin::operator ==(const String& rhs) const
{
  return mName == rhs;
}
bool PeerPlugin::operator !=(const String& rhs) const
{
  return mName != rhs;
}
bool PeerPlugin::operator  <(const String& rhs) const
{
  return mName < rhs;
}

const String& PeerPlugin::GetName() const
{
  return mName;
}

bool PeerPlugin::IsInitialized() const
{
  return mPeer ? true : false;
}

Peer* PeerPlugin::GetPeer() const
{
  return mPeer;
}

PeerPlugin::PeerPlugin()
  : mName(),
    mPeer(nullptr)
{
}

//
// Internal
//

void PeerPlugin::SetName(const String& name)
{
  mName = name;
}

bool PeerPlugin::Initialize(Peer* peer)
{
  Assert(!IsInitialized());
  mPeer = peer;
  return OnInitialize();
}
void PeerPlugin::Uninitialize()
{
  Assert(IsInitialized());
  OnUninitialize();
  mPeer = nullptr;
}

} // namespace Zero
