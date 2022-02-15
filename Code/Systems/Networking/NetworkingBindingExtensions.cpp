// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Meta Net Property
ZilchDefineType(MetaNetProperty, builder, type)
{
  ZilchBindField(mNetPropertyConfig)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(mNetChannelConfig)->AddAttribute(PropertyAttributes::cOptional);
}

} // namespace Zero
