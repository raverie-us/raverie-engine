// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "TagsContent.hpp"

namespace Raverie
{

RaverieDefineType(ContentTags, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);
}

void ContentTags::Serialize(Serializer& stream)
{
  SerializeName(mTags);
}

} // namespace Raverie
