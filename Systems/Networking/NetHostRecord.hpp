///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Reese Jones.
/// Copyright 2016, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// NetHostRecord.
/// A record that contains the basic information of a game server.
/// After a certain lifetime records expire on the master server.
class NetHostRecord : public SafeId32
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  NetHostRecord();
  NetHostRecord(NetHostRecord const& ref);

  /// Destructor.
  ~NetHostRecord();

  /// Comparison Operators (compares IP addresses).
  bool operator ==(const NetHostRecord& rhs) const;
  bool operator !=(const NetHostRecord& rhs) const;
  bool operator  <(const NetHostRecord& rhs) const;
  bool operator ==(const IpAddress& rhs) const;
  bool operator !=(const IpAddress& rhs) const;
  bool operator  <(const IpAddress& rhs) const;

  // Data
  float       mLifetime;      ///< How long has this record been alive in seconds?
  IpAddress   mIpAddress;     ///< The IpAddress associated with this record. This is who published it.
  EventBundle mBasicHostInfo; ///< The info published along with their record. Contains game server specific data.
  Guid        mProjectGuid;   ///< The project Guid that this host record belongs too.
};

/// Typedefs.
typedef UniquePointer<NetHostRecord>         NetHostRecordPtr;
typedef Array<NetHostRecordPtr>              HostRecordsArray;
typedef HashMap<IpAddress, NetHostRecord*>   HostRecordsMap;
typedef HashMap<MessageReceiptId, IpAddress> RecieptIpMap;

} // namespace Zero
