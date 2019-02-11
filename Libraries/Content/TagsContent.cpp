// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "TagsContent.hpp"

namespace Zero
{

ZilchDefineType(ContentTags, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
}

void ContentTags::Serialize(Serializer& stream)
{
  SerializeName(mTags);
}

} // namespace Zero
