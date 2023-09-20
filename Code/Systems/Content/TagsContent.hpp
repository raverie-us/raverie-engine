// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ContentTags : public ContentComponent
{
public:
  RaverieDeclareType(ContentTags, TypeCopyMode::ReferenceType);

  /// Constructor
  ContentTags()
  {
  }

  /// ContentComponent interface.
  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer)
  {
  }

  /// All unique tags
  HashSet<String> mTags;
};

} // namespace Raverie
