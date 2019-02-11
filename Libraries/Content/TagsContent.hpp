// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ContentTags : public ContentComponent
{
public:
  ZilchDeclareType(ContentTags, TypeCopyMode::ReferenceType);

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

} // namespace Zero
