// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

//                                NetHostRecord //

RaverieDefineType(NetHostRecord, builder, type)
{
  // Bind documentation
  RaverieBindDocumented();

  // Bind member properties
  RaverieBindFieldGetterProperty(mIpAddress);
  RaverieBindFieldGetterProperty(mBasicHostInfo);
  RaverieBindFieldGetterProperty(mLifetime);
  // RaverieBindFieldGetterProperty(mProjectGuid); //This type is not yet bound.
  // (int64 I think)
}

NetHostRecord::NetHostRecord() : mLifetime(0.0f), mIpAddress(), mBasicHostInfo(), mProjectGuid()
{
}
NetHostRecord::NetHostRecord(NetHostRecord const& ref) : mLifetime(ref.mLifetime), mIpAddress(ref.mIpAddress), mBasicHostInfo(ref.mBasicHostInfo), mProjectGuid(ref.mProjectGuid)
{
}

NetHostRecord::~NetHostRecord()
{
}

bool NetHostRecord::operator==(const NetHostRecord& rhs) const
{
  return mIpAddress == rhs.mIpAddress;
}
bool NetHostRecord::operator!=(const NetHostRecord& rhs) const
{
  return mIpAddress != rhs.mIpAddress;
}
bool NetHostRecord::operator<(const NetHostRecord& rhs) const
{
  return mIpAddress < rhs.mIpAddress;
}
bool NetHostRecord::operator==(const IpAddress& rhs) const
{
  return mIpAddress == rhs;
}
bool NetHostRecord::operator!=(const IpAddress& rhs) const
{
  return mIpAddress != rhs;
}
bool NetHostRecord::operator<(const IpAddress& rhs) const
{
  return mIpAddress < rhs;
}

} // namespace Raverie
