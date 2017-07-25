///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                    NetHost                                      //
//---------------------------------------------------------------------------------//

/// Describes a network host.
class NetHost : public SafeId32
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  NetHost();
  NetHost(const IpAddress& ipAddress);
  NetHost(const NetHost& rhs);

  /// Destructor.
  ~NetHost();

  /// Comparison Operators (compares IP addresses).
  bool operator ==(const NetHost& rhs) const;
  bool operator !=(const NetHost& rhs) const;
  bool operator  <(const NetHost& rhs) const;
  bool operator ==(const IpAddress& rhs) const;
  bool operator !=(const IpAddress& rhs) const;
  bool operator  <(const IpAddress& rhs) const;

  /// Returns the round-trip time (RTT) in milliseconds from our peer to this host.
  uint GetRoundTripTime() const;
  /// Returns the estimated latency ((RTT/2)) in milliseconds from our peer to this host.
  uint GetLatency() const;

  // Data
  Network::Enum mNetwork;       ///< Host's network residence.
  IpAddress     mIpAddress;     ///< Host's IP address.
  TimeMs        mRoundTripTime; ///< Round trip time (from our peer to this host).
  EventBundle   mBasicHostInfo; ///< Basic host info (limited to 480 bytes).
  EventBundle   mExtraHostInfo; ///< Extra host info.
};

/// Typedefs.
typedef UniquePointer<NetHost>                                NetHostPtr;
typedef ArraySet< NetHostPtr, PointerSortPolicy<NetHostPtr> > NetHostSet;
typedef ArrayMap<Network::Enum, NetHostSet>                   NetHostsMap;

//---------------------------------------------------------------------------------//
//                                 NetHostRange                                    //
//---------------------------------------------------------------------------------//

/// Network Host Range.
struct NetHostRange : public NetHostSet::range
{
  /// Typedefs.
  typedef NetHost* FrontResult;

  /// Constructors.
  NetHostRange();
  NetHostRange(const NetHostSet::range& rhs);

  // Data Access.
  void        PopFront()    { ++mBegin;                }
  FrontResult Front()       { return mBegin->mPointer; }
  bool        Empty() const { return mBegin == mEnd;   }
};

} // namespace Zero
