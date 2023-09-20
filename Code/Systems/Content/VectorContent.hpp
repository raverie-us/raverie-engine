// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Font content file are true type font files. Loading into
/// vector or bitmap formats.
class FontContent : public ContentComposition
{
public:
  RaverieDeclareType(FontContent, TypeCopyMode::ReferenceType);

  FontContent();
};

// Normal font builder just copies the ttf for direct loading.
class FontBuilder : public DirectBuilderComponent
{
public:
  RaverieDeclareType(FontBuilder, TypeCopyMode::ReferenceType);
  FontBuilder();
  void Generate(ContentInitializer& initializer) override;
};

} // namespace Raverie
