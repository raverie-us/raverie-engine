// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
RaverieDefineType(TextContent, builder, type)
{
}

TextContent::TextContent()
{
  EditMode = ContentEditMode::ResourceObject;
}

ContentItem* MakeTextContent(ContentInitializer& initializer)
{
  TextContent* content = new TextContent();

  content->Filename = initializer.Filename;

  DirectBuilderComponent* builder = nullptr;

  if (initializer.Extension == "txt")
    builder = new TextBuilder();

  TypeExtensionEntry* raverieTypeEntry = FileExtensionManager::GetRaverieScriptTypeEntry();
  if (raverieTypeEntry->IsValidExtensionNoDot(initializer.Extension))
    builder = new RaverieScriptBuilder();

  TypeExtensionEntry* fragmentTypeEntry = FileExtensionManager::GetRaverieFragmentTypeEntry();
  // at the moment the extension always comes through as
  // lower case so add both cases to cover any future changes
  if (fragmentTypeEntry->IsValidExtensionNoDot(initializer.Extension))
    builder = new RaverieFragmentBuilder();

  builder->Generate(initializer);

  content->AddComponent(builder);

  return content;
}

RaverieDefineType(BaseTextBuilder, builder, type)
{
  RaverieBindDependency(TextContent);
}

void BaseTextBuilder::Generate(ContentInitializer& initializer)
{
  Name = initializer.Name;
  mResourceId = GenerateUniqueId64();
}

RaverieDefineType(TextBuilder, builder, type)
{
}
RaverieDefineType(RaverieScriptBuilder, builder, type)
{
}
RaverieDefineType(RaverieFragmentBuilder, builder, type)
{
}

void CreateScriptContent(ContentSystem* system)
{
  AddContent<TextContent>(system);
  AddContentComponent<TextBuilder>(system);
  AddContentComponent<RaverieScriptBuilder>(system);
  AddContentComponent<RaverieFragmentBuilder>(system);

  ContentTypeEntry text(RaverieTypeId(TextContent), MakeTextContent);
  system->CreatorsByExtension["txt"] = text;

  TypeExtensionEntry* raverieExtensions = FileExtensionManager::GetInstance()->GetRaverieScriptTypeEntry();
  for (size_t i = 0; i < raverieExtensions->mExtensions.Size(); ++i)
    system->CreatorsByExtension[raverieExtensions->mExtensions[i]] = text;

  TypeExtensionEntry* fragmentExtensions = FileExtensionManager::GetInstance()->GetRaverieFragmentTypeEntry();
  for (size_t i = 0; i < fragmentExtensions->mExtensions.Size(); ++i)
    system->CreatorsByExtension[fragmentExtensions->mExtensions[i]] = text;
}

} // namespace Raverie
