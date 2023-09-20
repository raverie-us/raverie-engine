// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
RaverieDefineType(FontContent, builder, type)
{
}

FontContent::FontContent()
{
  EditMode = ContentEditMode::ContentItem;
}

ContentItem* MakeFontContent(ContentInitializer& initializer)
{
  FontContent* content = new FontContent();

  content->Filename = initializer.Filename;

  FontBuilder* builder = new FontBuilder();
  builder->Extension = BuildString(".", initializer.Extension);
  builder->Generate(initializer);
  content->AddComponent(builder);

  return content;
}

RaverieDefineType(FontBuilder, builder, type)
{
  RaverieBindDependency(FontContent);
}

// Note: Since we added more font builders, and we cannot pass the extension
// into the constructor (builders require a default constructor) then we cannot
// properly pass the extension in. All the base class does is store it, so above
// when we create the builder we just immediately set the Extension afterward
// (this should be refactored later)
FontBuilder::FontBuilder() : DirectBuilderComponent(0, ".ttf", "Font")
{
}

void FontBuilder::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;
}

void CreateVectorContent(ContentSystem* system)
{
  AddContent<FontContent>(system);
  AddContentComponent<FontBuilder>(system);
  system->CreatorsByExtension["ttf"] = ContentTypeEntry(RaverieTypeId(FontContent), MakeFontContent);
  system->CreatorsByExtension["ttc"] = ContentTypeEntry(RaverieTypeId(FontContent), MakeFontContent);
  system->CreatorsByExtension["otf"] = ContentTypeEntry(RaverieTypeId(FontContent), MakeFontContent);
}

} // namespace Raverie
