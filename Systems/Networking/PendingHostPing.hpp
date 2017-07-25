///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               PendingHostPing                                   //
//---------------------------------------------------------------------------------//

/// Network host ping type.
DeclareEnum4(HostPingType,
  DiscoverList,
  Refresh,
  RefreshList,
  MasterServerRefreshHost);

/// Pending host ping request.
class PendingHostPing
{
public:
  /// Constructors.
  PendingHostPing();
  PendingHostPing(Network::Enum network, TimeMs creationTime, TimeMs timeout, HostPingType::Enum hostPingType, const IpAddress& theirIpAddress, uint pingId, const EventBundle& pingBundle);
  PendingHostPing(Network::Enum network, TimeMs creationTime, TimeMs timeout, HostPingType::Enum hostPingType, const Array<IpAddress>& theirIpAddresses, uint pingId, const EventBundle& pingBundle);

  /// Comparison Operators (compares ping IDs).
  bool operator ==(const PendingHostPing& rhs) const;
  bool operator !=(const PendingHostPing& rhs) const;
  bool operator  <(const PendingHostPing& rhs) const;
  bool operator ==(const uint& rhs) const;
  bool operator !=(const uint& rhs) const;
  bool operator  <(const uint& rhs) const;

  /// Returns true if the pending host ping request has timed out, else false.
  bool HasTimedOut(TimeMs now);
  /// Returns the duration since the pending host ping request was created.
  TimeMs GetDurationSinceCreationTime(TimeMs now);
  /// Returns the duration since the last host ping message was sent.
  TimeMs GetDurationSinceLastSendTime(TimeMs now);
  /// Returns the duration since the specified send attempt.
  TimeMs GetDurationSinceSendAttempt(uint sendAttemptId, TimeMs now);

  /// Adds a send attempt.
  void AddSendAttempt(uint sendAttemptId, TimeMs now);

  // Data
  Network::Enum          mNetwork;          ///< Operating network.
  TimeMs                 mCreationTime;     ///< Creation time.
  TimeMs                 mLastSendTime;     ///< Last send attempt time.
  ArrayMap<uint, TimeMs> mSendAttempts;     ///< Send attempts (send attempt ID, send time).
  TimeMs                 mTimeout;          ///< Request time out.
  HostPingType::Enum     mHostPingType;     ///< Host ping type.
  Array<IpAddress>       mTheirIpAddresses; ///< Their IP addresses (as seen from our perspective).
  uint                   mPingId;           ///< Unique ping request identifier.
  EventBundle            mPingBundle;       ///< Our bundled ping request event data.
  ArraySet<IpAddress>    mRespondingHosts;  ///< Hosts that responded to our ping (sorted by memory address).
};

/// Typedefs.
typedef ArraySet< UniquePointer<PendingHostPing>, PointerSortPolicy<UniquePointer<PendingHostPing> > > PendingHostPingSet;

} // namespace Zero
