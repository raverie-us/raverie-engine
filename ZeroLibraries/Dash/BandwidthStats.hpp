///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Manages network bandwidth statistics
template<bool UseAtomicStats>
class BandwidthStats
{
public:
  /// Default Constructor
  BandwidthStats() { InitializeStats(); }

  //
  // Bandwidth Statistics
  //

  /// Returns the minimum outgoing bandwidth usage
  Kbps GetMinOutgoingBandwidthUsage() const { return mOutgoingBandwidthUsageMin; }
  /// Returns the average outgoing bandwidth usage
  Kbps GetAvgOutgoingBandwidthUsage() const { return mOutgoingBandwidthUsageAvg; }
  /// Returns the maximum outgoing bandwidth usage
  Kbps GetMaxOutgoingBandwidthUsage() const { return mOutgoingBandwidthUsageMax; }

  /// Returns the minimum incoming bandwidth usage
  Kbps GetMinIncomingBandwidthUsage() const { return mIncomingBandwidthUsageMin; }
  /// Returns the average incoming bandwidth usage
  Kbps GetAvgIncomingBandwidthUsage() const { return mIncomingBandwidthUsageAvg; }
  /// Returns the maximum incoming bandwidth usage
  Kbps GetMaxIncomingBandwidthUsage() const { return mIncomingBandwidthUsageMax; }

  /// Returns the minimum total bandwidth usage (outgoing and incoming)
  Kbps GetMinTotalBandwidthUsage() const { return mTotalBandwidthUsageMin; }
  /// Returns the average total bandwidth usage (outgoing and incoming)
  Kbps GetAvgTotalBandwidthUsage() const { return mTotalBandwidthUsageAvg; }
  /// Returns the maximum total bandwidth usage (outgoing and incoming)
  Kbps GetMaxTotalBandwidthUsage() const { return mTotalBandwidthUsageMax; }

  /// Returns the minimum packet send rate
  uint GetMinSendRate() const   { return mSendRateMin; }
  /// Returns the average packet send rate
  double GetAvgSendRate() const { return mSendRateAvg; }
  /// Returns the maximum packet send rate
  uint GetMaxSendRate() const   { return mSendRateMax; }

  /// Returns the minimum packet receive rate
  uint GetMinReceiveRate() const   { return mReceiveRateMin; }
  /// Returns the average packet receive rate
  double GetAvgReceiveRate() const { return mReceiveRateAvg; }
  /// Returns the maximum packet receive rate
  uint GetMaxReceiveRate() const   { return mReceiveRateMax; }

  /// Returns the minimum sent packet byte size
  Bytes GetMinSentPacketBytes() const  { return mSentPacketBytesMin; }
  /// Returns the average sent packet byte size
  double GetAvgSentPacketBytes() const { return mSentPacketBytesAvg; }
  /// Returns the maximum sent packet byte size
  Bytes GetMaxSentPacketBytes() const  { return mSentPacketBytesMax; }

  /// Returns the minimum received packet byte size
  Bytes GetMinReceivedPacketBytes() const  { return mReceivedPacketBytesMin; }
  /// Returns the average received packet byte size
  double GetAvgReceivedPacketBytes() const { return mReceivedPacketBytesAvg; }
  /// Returns the maximum received packet byte size
  Bytes GetMaxReceivedPacketBytes() const  { return mReceivedPacketBytesMax; }

  /// Returns the total number of packets ever sent (unaffected by ResetStats)
  uintmax GetTotalPacketsSent() const     { return mTotalPacketsSent; }
  /// Returns the total number of packets ever received (unaffected by ResetStats)
  uintmax GetTotalPacketsReceived() const { return mTotalPacketsReceived; }

  /// Returns a summary of all peer statistics as an array of pairs containing the property name and array of minimum, average, and maximum values
  Array< Pair< String, Array<String> > > GetStatsSummary() const
  {
    // TODO
    return Array< Pair< String, Array<String> > ();
  }
  /// Returns a summary of all peer statistics as a single multi-line string (intended for debugging convenience)
  String GetStatsSummaryString() const
  {
    // TODO
    return String();
  }

protected:
  /// Resets all applicable bandwidth statistics to start over relative to a new statistics period
  void ResetStats()
  {
    mOutgoingBandwidthUsageUpdated = false;
    mOutgoingBandwidthUsageMin     = 0;
    mOutgoingBandwidthUsageAvg     = 0;
    mOutgoingBandwidthUsageMax     = 0;

    mIncomingBandwidthUsageUpdated = false;
    mIncomingBandwidthUsageMin     = 0;
    mIncomingBandwidthUsageAvg     = 0;
    mIncomingBandwidthUsageMax     = 0;

    mSendRateUpdated = false;
    mSendRateMin     = 0;
    mSendRateAvg     = 0;
    mSendRateMax     = 0;

    mReceiveRateUpdated = false;
    mReceiveRateMin     = 0;
    mReceiveRateAvg     = 0;
    mReceiveRateMax     = 0;

    mSentPacketBytesUpdated = false;
    mSentPacketBytesMin     = 0;
    mSentPacketBytesAvg     = 0;
    mSentPacketBytesMax     = 0;

    mReceivedPacketBytesUpdated = false;
    mReceivedPacketBytesMin     = 0;
    mReceivedPacketBytesAvg     = 0;
    mReceivedPacketBytesMax     = 0;
  }

  /// Initializes all bandwidth statistics
  void InitializeStats()
  {
    ResetStats();

    mPacketsSent     = 0;
    mPacketsReceived = 0;
  }

  /// Updates the outgoing bandwidth usage statistics
  void UpdateOutgoingBandwidthUsage(Kbps sample)
  {
    if(mOutgoingBandwidthUsageUpdated)
    {
      mOutgoingBandwidthUsageMin = std::min(Kbps(mOutgoingBandwidthUsageMin), sample);
      mOutgoingBandwidthUsageAvg = Average(Kbps(mOutgoingBandwidthUsageAvg), sample, 0.1);
      mOutgoingBandwidthUsageMax = std::max(Kbps(mOutgoingBandwidthUsageMax), sample);
    }
    else
    {
      mOutgoingBandwidthUsageMin     = sample;
      mOutgoingBandwidthUsageAvg     = sample;
      mOutgoingBandwidthUsageMax     = sample;
      mOutgoingBandwidthUsageUpdated = true;
    }
  }
  /// Updates the incoming bandwidth usage statistics
  void UpdateIncomingBandwidthUsage(Kbps sample)
  {
    if(mIncomingBandwidthUsageUpdated)
    {
      mIncomingBandwidthUsageMin = std::min(Kbps(mIncomingBandwidthUsageMin), sample);
      mIncomingBandwidthUsageAvg = Average(Kbps(mIncomingBandwidthUsageAvg), sample, 0.1);
      mIncomingBandwidthUsageMax = std::max(Kbps(mIncomingBandwidthUsageMax), sample);
    }
    else
    {
      mIncomingBandwidthUsageMin     = sample;
      mIncomingBandwidthUsageAvg     = sample;
      mIncomingBandwidthUsageMax     = sample;
      mIncomingBandwidthUsageUpdated = true;
    }
  }
  /// Updates the send rate statistics
  void UpdateSendRate(uint32 sample)
  {
    if(mSendRateUpdated)
    {
      mSendRateMin = std::min(uint32(mSendRateMin), sample);
      mSendRateAvg = Average(double(mSendRateAvg), double(sample), 0.15);
      mSendRateMax = std::max(uint32(mSendRateMax), sample);
    }
    else
    {
      mSendRateMin     = sample;
      mSendRateAvg     = sample;
      mSendRateMax     = sample;
      mSendRateUpdated = true;
    }
  }
  /// Updates the receive rate statistics
  void UpdateReceiveRate(uint32 sample)
  {
    if(mReceiveRateUpdated)
    {
      mReceiveRateMin = std::min(uint32(mReceiveRateMin), sample);
      mReceiveRateAvg = Average(double(mReceiveRateAvg), double(sample), 0.15);
      mReceiveRateMax = std::max(uint32(mReceiveRateMax), sample);
    }
    else
    {
      mReceiveRateMin     = sample;
      mReceiveRateAvg     = sample;
      mReceiveRateMax     = sample;
      mReceiveRateUpdated = true;
    }
  }
  /// Updates the sent packet bytes statistics
  void UpdateSentPacketBytes(Bytes sample)
  {
    if(mSentPacketBytesUpdated)
    {
      mSentPacketBytesMin = std::min(Bytes(mSentPacketBytesMin), sample);
      mSentPacketBytesAvg = Average(double(mSentPacketBytesAvg), double(sample), 0.1);
      mSentPacketBytesMax = std::max(Bytes(mSentPacketBytesMax), sample);
    }
    else
    {
      mSentPacketBytesMin     = sample;
      mSentPacketBytesAvg     = sample;
      mSentPacketBytesMax     = sample;
      mSentPacketBytesUpdated = true;
    }
  }
  /// Updates the received packet bytes statistics
  void UpdateReceivedPacketBytes(Bytes sample)
  {
    if(mReceivedPacketBytesUpdated)
    {
      mReceivedPacketBytesMin = std::min(Bytes(mReceivedPacketBytesMin), sample);
      mReceivedPacketBytesAvg = Average(double(mReceivedPacketBytesAvg), double(sample), 0.1);
      mReceivedPacketBytesMax = std::max(Bytes(mReceivedPacketBytesMax), sample);
    }
    else
    {
      mReceivedPacketBytesMin     = sample;
      mReceivedPacketBytesAvg     = sample;
      mReceivedPacketBytesMax     = sample;
      mReceivedPacketBytesUpdated = true;
    }
  }
  /// Updates the packets sent statistics
  void UpdatePacketsSent()
  {
    ++mPacketsSent;
  }
  /// Updates the packets received statistics
  void UpdatePacketsReceived()
  {
    ++mPacketsReceived;
  }

private:
  /// Typedefs
  typedef typename conditional<UseAtomicStats, Atomic<bool>,   bool>::type      bool_type;
  typedef typename conditional<UseAtomicStats, Atomic<double>, double>::type    double_type;
  typedef typename conditional<UseAtomicStats, Atomic<Kbps>,   Kbps>::type      Kbps_type;
  typedef typename conditional<UseAtomicStats, Atomic<uint32>, uint32>::type    uint32_type;
  typedef typename conditional<UseAtomicStats, Atomic<Bytes>,  Bytes>::type     Bytes_type;
  typedef typename conditional<UseAtomicStats, Atomic<uintmax>,  uintmax>::type uintmax_type;

  /// Statistics
  bool_type    mOutgoingBandwidthUsageUpdated; /// Outgoing bandwidth usage updated?
  Kbps_type    mOutgoingBandwidthUsageMin;     /// Minimum outgoing bandwidth usage
  Kbps_type    mOutgoingBandwidthUsageAvg;     /// Average outgoing bandwidth usage
  Kbps_type    mOutgoingBandwidthUsageMax;     /// Maximum outgoing bandwidth usage

  bool_type    mIncomingBandwidthUsageUpdated; /// Incoming bandwidth usage updated?
  Kbps_type    mIncomingBandwidthUsageMin;     /// Minimum incoming bandwidth usage
  Kbps_type    mIncomingBandwidthUsageAvg;     /// Average incoming bandwidth usage
  Kbps_type    mIncomingBandwidthUsageMax;     /// Maximum incoming bandwidth usage

  bool_type    mSendRateUpdated;               /// Packet send rate updated?
  uint32_type  mSendRateMin;                   /// Minimum packet send rate
  double_type  mSendRateAvg;                   /// Average packet send rate
  uint32_type  mSendRateMax;                   /// Maximum packet send rate

  bool_type    mReceiveRateUpdated;            /// Packet receive rate updated?
  uint32_type  mReceiveRateMin;                /// Minimum packet receive rate
  double_type  mReceiveRateAvg;                /// Average packet receive rate
  uint32_type  mReceiveRateMax;                /// Maximum packet receive rate

  bool_type    mSentPacketBytesUpdated;        /// Sent packet bytes updated?
  Bytes_type   mSentPacketBytesMin;            /// Minimum sent packet bytes
  double_type  mSentPacketBytesAvg;            /// Average sent packet bytes
  Bytes_type   mSentPacketBytesMax;            /// Maximum sent packet bytes

  bool_type    mReceivedPacketBytesUpdated;    /// Received packet bytes updated?
  Bytes_type   mReceivedPacketBytesMin;        /// Minimum received packet bytes
  double_type  mReceivedPacketBytesAvg;        /// Average received packet bytes
  Bytes_type   mReceivedPacketBytesMax;        /// Maximum received packet bytes

  uintmax_type mPacketsSent;                   /// Packets sent
  uintmax_type mPacketsReceived;               /// Packets received
};

} // namespace Zero
