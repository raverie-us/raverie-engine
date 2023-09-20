// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ImageContent, builder, type)
{
}

ImageContent::ImageContent()
{
  EditMode = ContentEditMode::ContentItem;
  mReload = false;
}

void ImageContent::BuildContentItem(BuildOptions& options)
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
    mNeedsEditorProcessing = true;
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
    ImageInfo imageInfo;

    bool reaImage = ReadImageInfo(fullPath, imageInfo);
    if (!reaImage)
    {
      initializer.Success = false;
      initializer.Message = "Invalid image file.";
      return content;
    }

    if (imageInfo.Width > cMaxSpriteSize || imageInfo.Height > cMaxSpriteSize)
    {
      initializer.Success = false;
      initializer.Message = "Sprite is too large, must be within 4096x4096.";
      return content;
    }

    SpriteSourceBuilder* builder = new SpriteSourceBuilder();
    builder->Generate(initializer);

    // Default frame size to image size
    builder->FrameSizeX = imageInfo.Width;
    builder->FrameSizeY = imageInfo.Height;
    // Default origin to center
    builder->OriginX = float(imageInfo.Width / 2);
    builder->OriginY = float(imageInfo.Height / 2);

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

void CreateImageContent(ContentSystem* system)
{
  AddContentComponent<TextureInfo>(system);
  AddContentComponent<TextureBuilder>(system);
  // AddContentComponent<HeightToNormalBuilder>(system);
  AddContentComponent<SpriteSourceBuilder>(system);

  AddContent<ImageContent>(system);

  ContentTypeEntry imageContent(RaverieTypeId(ImageContent), MakeImageContentItem);

  // The extensions returned are always without the '.', e.g. "png"
  forRange (StringParam extension, GetSupportedImageLoadExtensions())
    system->CreatorsByExtension[extension] = imageContent;
}

void BuildImageFileDialogFilters(Array<FileDialogFilter>& filters)
{
  StringBuilder builder;
  forRange (StringParam extension, GetSupportedImageLoadExtensions())
  {
    builder.Append("*.");
    builder.Append(extension);
    builder.Append(';');
  }

  filters.PushBack(FileDialogFilter("All Images", builder.ToString()));

  forRange (StringParam extension, GetSupportedImageLoadExtensions())
    filters.PushBack(FileDialogFilter(BuildString("*.", extension)));
}

} // namespace Raverie
