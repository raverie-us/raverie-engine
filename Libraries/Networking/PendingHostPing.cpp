///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               PendingHostPing                                   //
//---------------------------------------------------------------------------------//

PendingHostPing::PendingHostPing()
  : mNetwork(Network::LAN),
  mCreationTime(0),
  mLastSendTime(0),
  mSendAttempts(),
  mTimeout(0),
  mHostPingType(HostPingType::DiscoverList),
  mTheirIpAddresses(),
  mPingId(0),
  mPingBundle(),
  mRespondingHosts()
{
}

PendingHostPing::PendingHostPing(Network::Enum network, TimeMs creationTime, TimeMs timeout, HostPingType::Enum hostPingType, const IpAddress& theirIpAddress, uint pingId, const EventBundle& pingBundle)
  : mNetwork(network),
  mCreationTime(creationTime),
  mLastSendTime(0),
  mSendAttempts(),
  mTimeout(timeout),
  mHostPingType(hostPingType),
  mTheirIpAddresses(),
  mPingId(pingId),
  mPingBundle(pingBundle),
  mRespondingHosts()
{
  mTheirIpAddresses.PushBack(theirIpAddress);
}

PendingHostPing::PendingHostPing(Network::Enum network, TimeMs creationTime, TimeMs timeout, HostPingType::Enum hostPingType, const Array<IpAddress>& theirIpAddresses, uint pingId, const EventBundle& pingBundle)
  : mNetwork(network),
  mCreationTime(creationTime),
  mLastSendTime(0),
  mSendAttempts(),
  mTimeout(timeout),
  mHostPingType(hostPingType),
  mTheirIpAddresses(theirIpAddresses),
  mPingId(pingId),
  mPingBundle(pingBundle),
  mRespondingHosts()
{
}

bool PendingHostPing::operator ==(const PendingHostPing& rhs) const
{
  return mPingId == rhs.mPingId;
}
bool PendingHostPing::operator !=(const PendingHostPing& rhs) const
{
  return mPingId != rhs.mPingId;
}
bool PendingHostPing::operator  <(const PendingHostPing& rhs) const
{
  return mPingId < rhs.mPingId;
}
bool PendingHostPing::operator ==(const uint& rhs) const
{
  return mPingId == rhs;
}
bool PendingHostPing::operator !=(const uint& rhs) const
{
  return mPingId != rhs;
}
bool PendingHostPing::operator  <(const uint& rhs) const
{
  return mPingId < rhs;
}

bool PendingHostPing::HasTimedOut(TimeMs now)
{
  return GetDurationSinceCreationTime(now) >= mTimeout;
}
TimeMs PendingHostPing::GetDurationSinceCreationTime(TimeMs now)
{
  return GetDuration(mCreationTime, now);
}
TimeMs PendingHostPing::GetDurationSinceLastSendTime(TimeMs now)
{
  return GetDuration(mLastSendTime, now);
}
TimeMs PendingHostPing::GetDurationSinceSendAttempt(uint sendAttemptId, TimeMs now)
{
  TimeMs sendTime = mSendAttempts.FindValue(sendAttemptId, mCreationTime);
  return GetDuration(sendTime, now);
}

void PendingHostPing::AddSendAttempt(uint sendAttemptId, TimeMs now)
{
  mLastSendTime = now;
  mSendAttempts.InsertOrAssign(sendAttemptId, now);
}

} // namespace Zero
