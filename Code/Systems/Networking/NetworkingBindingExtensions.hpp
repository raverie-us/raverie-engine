// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Meta Net Property
class MetaNetProperty : public MetaAttribute
{
public:
  RaverieDeclareType(MetaNetProperty, TypeCopyMode::ReferenceType);

  /// The net property type name.
  String mNetPropertyConfig;

  /// Desired net channel name.
  String mNetChannelConfig;
};

} // namespace Raverie
