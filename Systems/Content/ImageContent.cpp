///////////////////////////////////////////////////////////////////////////////
///
/// \file ImageContent.cpp
/// Implementation of the Image content classes.
/// 
/// Authors: Chris Peters
/// Copyright 2011-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ImageContent, builder, type)
{
}

ImageContent::ImageContent()
{
  EditMode = ContentEditMode::ContentItem;
  mReload = false;
}

void ImageContent::BuildContent(BuildOptions& options)
{
  forRange (BuilderComponent* bc, Builders.All())
  {
    if (bc->NeedsBuilding(options))
      bc->BuildContent(options);
  }

  if (mReload)
  {
    ClearComponents();
    LoadFromDataFile(*this, GetMetaFilePath());
    this->OnInitialize();

    // Queue for editor processing
    options.EditorProcessing.PushBack(this);
    mReload = false;
  }
}

ContentItem* MakeImageContentItem(ContentInitializer& initializer)
{
  ImageContent* content = new ImageContent();

  String fullPath = FilePath::Combine(initializer.Library->SourcePath, initializer.Filename);

  content->Filename = initializer.Filename;

  String extension = FilePath::GetExtension(initializer.Filename);

  bool isSprite = initializer.BuilderType == "SpriteSource" || initializer.Options && initializer.Options->mImageOptions->mImportImages == ImageImport::Sprites;

  if (isSprite)
  {
    PngInfo pngInfo;

    bool readPng = ReadPngInfo(fullPath, pngInfo);
    if(!readPng)
    {
      initializer.Success = false;
      initializer.Message = "Invalid png file.";
      return content;
    }

    if(pngInfo.Width > MaxSpriteSize ||  pngInfo.Height > MaxSpriteSize)
    {
      initializer.Success = false;
      initializer.Message = "Sprite is too large must be less than 4096";
      return content;
    }

    SpriteSourceBuilder* builder = new SpriteSourceBuilder();
    builder->Generate(initializer);

    // Use width and height info
    // to default to center
    builder->OriginX = float(pngInfo.Width / 2);
    builder->OriginY = float(pngInfo.Height / 2);

    content->AddComponent(builder);
  }
  else
  {
    TextureInfo* info = new TextureInfo();
    content->AddComponent(info);

    TextureBuilder* builder = new TextureBuilder();
    builder->Generate(initializer);
    content->AddComponent(builder);
  }

  return content;
}

//---------------------------------------------------------------- Registration

void CreateImageContent(ContentSystem* system)
{
  AddContentComponent<TextureInfo>(system);
  AddContentComponent<TextureBuilder>(system);
  //AddContentComponent<HeightToNormalBuilder>(system);
  AddContentComponent<SpriteSourceBuilder>(system);

  AddContent<ImageContent>(system);

  ContentTypeEntry imageContent(ZilchTypeId(ImageContent), MakeImageContentItem);

  system->CreatorsByExtension["png"] = imageContent;
  system->CreatorsByExtension["hdr"] = imageContent;
  //system->CreatorsByExtension["tga"] = imageContent;
  //system->CreatorsByExtension["tif"] = imageContent;
  //system->CreatorsByExtension["psd"] = imageContent;
  //system->CreatorsByExtension["jpg"] = imageContent;
}

}//namespace Zero
