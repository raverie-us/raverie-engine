// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Meta Net Property
RaverieDefineType(MetaNetProperty, builder, type)
{
  RaverieBindField(mNetPropertyConfig)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindField(mNetChannelConfig)->AddAttribute(PropertyAttributes::cOptional);
}

} // namespace Raverie
