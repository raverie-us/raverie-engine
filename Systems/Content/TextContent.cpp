///////////////////////////////////////////////////////////////////////////////
///
/// \file TextContent.hpp
/// Implementation of TextContent classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(TextContent, builder, type)
{
}

TextContent::TextContent()
{
  EditMode = ContentEditMode::ResourceObject;
}

//------------------------------------------------------------ Factory

ContentItem* MakeTextContent(ContentInitializer& initializer)
{
  TextContent* content = new TextContent();

  content->Filename = initializer.Filename;
  
  DirectBuilderComponent* builder = nullptr;

  if(initializer.Extension == "txt")
    builder = new TextBuilder();

  TypeExtensionEntry* zilchTypeEntry = FileExtensionManager::GetZilchScriptTypeEntry();
  if(zilchTypeEntry->IsValidExtensionNoDot(initializer.Extension))
    builder = new ZilchScriptBuilder();

  TypeExtensionEntry* fragmentTypeEntry = FileExtensionManager::GetZilchFragmentTypeEntry();
  //at the moment the extension always comes through as
  //lower case so add both cases to cover any future changes
  if(fragmentTypeEntry->IsValidExtensionNoDot(initializer.Extension))
    builder = new ZilchFragmentBuilder();

  builder->Generate(initializer);

  content->AddComponent(builder);

  return content;
}

//------------------------------------------------------------Builders
ZilchDefineType(BaseTextBuilder, builder, type)
{
  ZeroBindDependency(TextContent);
}

void BaseTextBuilder::Generate(ContentInitializer& initializer)
{
  Name = initializer.Name;
  mResourceId = GenerateUniqueId64();
}

ZilchDefineType(TextBuilder, builder, type) {}
ZilchDefineType(ZilchScriptBuilder, builder, type) {}
ZilchDefineType(ZilchFragmentBuilder, builder, type) {}

void CreateScriptContent(ContentSystem* system)
{
  AddContent<TextContent>(system);
  AddContentComponent<TextBuilder>(system);
  AddContentComponent<ZilchScriptBuilder>(system);
  AddContentComponent<ZilchFragmentBuilder>(system);

  ContentTypeEntry text(ZilchTypeId(TextContent), MakeTextContent);
  system->CreatorsByExtension["txt"] = text;
  

  TypeExtensionEntry* zilchExtensions = FileExtensionManager::GetInstance()->GetZilchScriptTypeEntry();
  for(size_t i = 0; i < zilchExtensions->mExtensions.Size(); ++i)
    system->CreatorsByExtension[zilchExtensions->mExtensions[i]] = text;

  TypeExtensionEntry* fragmentExtensions = FileExtensionManager::GetInstance()->GetZilchFragmentTypeEntry();
  for(size_t i = 0; i < fragmentExtensions->mExtensions.Size(); ++i)
    system->CreatorsByExtension[fragmentExtensions->mExtensions[i]] = text;
}

}
