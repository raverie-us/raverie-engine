// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

class SpriteSourceLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    SpriteSource* source = new SpriteSource();
    source->Name = entry.Name;
    DataBlock dataBlock = ReadFileIntoDataBlock(entry.FullPath.c_str());
    LoadSprite(source, entry, dataBlock);
    SpriteSourceManager::GetInstance()->AddResource(entry, source);
    FreeBlock(dataBlock);
    return source;
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    SpriteSource* source = new SpriteSource();
    source->Name = entry.Name;
    DataBlock dataBlock = entry.Block;
    LoadSprite(source, entry, dataBlock);
    SpriteSourceManager::GetInstance()->AddResource(entry, source);
    return source;
  }

  void ReloadFromFile(Resource* resource, ResourceEntry& entry)
  {
    SpriteSource* source = (SpriteSource*)resource;
    source->Unload();
    DataBlock dataBlock = ReadFileIntoDataBlock(entry.FullPath.c_str());
    LoadSprite(source, entry, dataBlock);
    FreeBlock(dataBlock);
  }

  void LoadSprite(SpriteSource* source, ResourceEntry& entry, DataBlock block)
  {
    Status status;
    Image image;
    LoadImage(status, block.Data, block.Size, &image);

    if (image.Data == nullptr)
    {
      String message = String::Format("Failed to load sprite %s", entry.Name.c_str());
      DoNotifyWarning("Sprite Error", message);

      // Create a dummy source
      image.Allocate(2, 2);
      image.ClearColorTo(0xFFFFFFFF);
    }

    // If there is a builder the sprite is being load in the editor
    SpriteSourceBuilder* builder = (SpriteSourceBuilder*)entry.mBuilder;

    if (builder)
    {
      // Load sprite frame data directly from builder
      source->GetSpriteData() = builder->GetSpriteData();
    }
    else
    {
      // Get extra data on the end of the sprite file
      SpriteData* spriteData = (SpriteData*)(block.Data + block.Size - sizeof(SpriteData));
      memcpy(&source->GetSpriteData(), spriteData, sizeof(SpriteData));
    }

    // Sprite importing from group import used to output 0.
    if (source->FrameSizeX == 0 || source->FrameSizeY == 0)
    {
      source->FrameSizeX = image.Width;
      source->FrameSizeY = image.Height;
    }
    // Make sure frame size is valid
    source->FrameSizeX = Math::Clamp(source->FrameSizeX, cMinFrameSize, (uint)image.Width);
    source->FrameSizeY = Math::Clamp(source->FrameSizeY, cMinFrameSize, (uint)image.Height);

    FixAlphaHalo(&image);

    AtlasManager::GetInstance()->AddSpriteSource(source, &image);
  }
};

RaverieDefineType(SpriteSource, builder, type)
{
  RaverieBindDocumented();

  RaverieBindField(FrameCount);

  RaverieBindMethod(GetSize);
  RaverieBindMethod(GetOrigin);
}

void SpriteSource::Unload()
{
  AtlasManager::GetInstance()->RemoveSpriteSource(this);
}

Vec2 SpriteSource::GetSize()
{
  return Vec2((float)FrameSizeX, (float)FrameSizeY);
}

Vec2 SpriteSource::GetOrigin()
{
  return Vec2(OriginX, OriginY);
}

float SpriteSource::GetFrameRate()
{
  return 1.0f / FrameDelay;
}

UvRect SpriteSource::GetUvRect(uint currentFrame)
{
  if (mFramesPerRow == 0)
    return mBaseFrameUv;

  uint frameX = currentFrame % mFramesPerRow;
  uint frameY = currentFrame / mFramesPerRow;
  UvRect frameUv = mBaseFrameUv;
  Vec2 offset = mPerFrameUvOffset * Vec2((float)frameX, (float)frameY);
  frameUv.TopLeft += offset;
  frameUv.BotRight += offset;
  return frameUv;
}

void SpriteSource::LoadSourceImage(Status& status, Image* image)
{
  String fullPath = mContentItem->GetFullPath();
  if (!FileExists(fullPath))
  {
    String msg = String::Format("File '%s' didn't exist.", fullPath.c_str());
    status.SetFailed(msg);
    return;
  }

  DataBlock block = ReadFileIntoDataBlock(fullPath.c_str());

  LoadImage(status, block.Data, block.Size, image);

  FreeBlock(block);
}

HandleOf<Texture> SpriteSource::GetAtlasTexture()
{
  if (Atlas* atlas = mAtlas)
    return mAtlas->mTexture;

  return nullptr;
}

TextureRenderData* SpriteSource::GetAtlasTextureRenderData()
{
  if (Atlas* atlas = mAtlas)
    if (Texture* texture = atlas->mTexture)
      return texture->mRenderData;

  return nullptr;
}

ImplementResourceManager(SpriteSourceManager, SpriteSource);

SpriteSourceManager::SpriteSourceManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("SpriteSource", new SpriteSourceLoader());
  mCategory = "Graphics";
  mCanAddFile = true;
  BuildImageFileDialogFilters(mOpenFileFilters);
  DefaultResourceName = "SquareBordered";
  mPreview = true;
  mCanReload = true;
  mExtension = "png";
}

} // namespace Raverie
