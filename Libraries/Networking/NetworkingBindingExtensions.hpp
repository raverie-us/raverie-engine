// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Meta Net Property
class MetaNetProperty : public MetaAttribute
{
public:
  ZilchDeclareType(MetaNetProperty, TypeCopyMode::ReferenceType);

  /// The net property type name.
  String mNetPropertyConfig;

  /// Desired net channel name.
  String mNetChannelConfig;
};

} // namespace Zero
