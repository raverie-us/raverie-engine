// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(Atlas, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

HandleOf<Atlas> Atlas::CreateRuntime()
{
  Atlas* atlas = AtlasManager::CreateRuntime();
  return atlas;
}

Atlas::Atlas()
{
  mTexture = Texture::CreateRuntime();
  uint size = cAtlasSize * cAtlasSize * GetPixelSize(TextureFormat::RGBA8);
  byte* image = new byte[size];
  if (image != nullptr)
    memset(image, 0, size);
  mTexture->Upload(cAtlasSize, cAtlasSize, TextureFormat::RGBA8, image, size, false);
}

bool Atlas::AddSpriteSource(SpriteSource* source, Image* image)
{
  TextureFiltering::Enum filtering = source->Sampling == SpriteSampling::Nearest ? TextureFiltering::Nearest : TextureFiltering::Trilinear;

  if (mAabbTreeProxies.Empty())
  {
    mTexture->mFiltering = filtering;
    mTexture->mMipMapping = filtering == TextureFiltering::Nearest ? TextureMipMapping::None : TextureMipMapping::GpuGenerated;
    mTexture->mMaxMipOverride = Atlas::sMaxMipLevel;
    mTexture->mAddressingX = TextureAddressing::Clamp;
    mTexture->mAddressingY = TextureAddressing::Clamp;

    // Place large images on their own texture
    if (image->Width > (int)mTexture->mWidth || image->Height > (int)mTexture->mHeight)
    {
      mTexture->Upload(*image);
      source->mAtlas = this;
      source->mAtlasUvRect.TopLeft = Vec2(0.0f);
      source->mAtlasUvRect.BotRight = Vec2(1.0f);

      Aabb aabb;
      aabb.mMin = Vec3(0.0f);
      aabb.mMax = aabb.mMin + Vec3((float)image->Width, (float)image->Height, 1.0f);

      BroadPhaseProxy proxy;
      BaseBroadPhaseData<Aabb> data;
      data.mAabb = aabb;
      data.mClientData = aabb;
      mPlacedAabbs.CreateProxy(proxy, data);
      mAabbTreeProxies.Insert(source, proxy);

      return true;
    }
  }

  if (filtering != mTexture->mFiltering)
    return false;

  IntVec2 pos(0, 0);
  IntVec2 size(image->Width, image->Height);

  int width = mTexture->mWidth;
  int height = mTexture->mHeight;
  int minLineHeight = height;

  while (pos.y + size.y <= height)
  {
    while (pos.x + size.x <= width)
    {
      Aabb aabb;
      aabb.mMin = Vec3(Math::ToVec2(pos), 0.0f);
      aabb.mMax = aabb.mMin + Vec3(Math::ToVec2(size), 1.0f);

      Aabb testAabb = aabb;
      testAabb.SetHalfExtents(testAabb.GetHalfExtents() - Vec3(0.1f));

      bool overlap = false;
      Aabb overlapAabb;
      forRangeBroadphaseTree(AvlDynamicAabbTree<Aabb>, mPlacedAabbs, Aabb, testAabb)
      {
        overlapAabb = range.Front();
        if (overlapAabb.Overlap(testAabb))
        {
          overlap = true;
          break;
        }
      }

      if (!overlap)
      {
        BroadPhaseProxy proxy;
        BaseBroadPhaseData<Aabb> data;
        data.mAabb = aabb;
        data.mClientData = aabb;
        mPlacedAabbs.CreateProxy(proxy, data);
        mAabbTreeProxies.Insert(source, proxy);

        source->mAtlas = this;
        source->mAtlasUvRect.TopLeft = Vec2(aabb.mMin.x / width, aabb.mMin.y / height);
        source->mAtlasUvRect.BotRight = Vec2(aabb.mMax.x / width, aabb.mMax.y / height);

        mTexture->SubUpload(*image, pos.x, pos.y);
        return true;
      }

      pos.x = (int)overlapAabb.mMax.x;
      minLineHeight = Math::Min(minLineHeight, (int)overlapAabb.mMax.y);
    }

    pos.x = 0;
    pos.y = minLineHeight;
    minLineHeight = height;
  }

  return false;
}

void Atlas::RemoveSpriteSource(SpriteSource* source)
{
  ErrorIf(mAabbTreeProxies.ContainsKey(source) == false, "Atlas is missing an entry for a source that references it.");

  BroadPhaseProxy proxy = mAabbTreeProxies.FindValue(source, BroadPhaseProxy());

  Aabb aabb = mPlacedAabbs.GetClientData(proxy);
  Image image;
  image.Allocate((int)aabb.GetExtents().x, (int)aabb.GetExtents().y);
  source->mAtlas->mTexture->SubUpload(image, (int)aabb.mMin.x, (int)aabb.mMin.y);

  mPlacedAabbs.RemoveProxy(proxy);
  mAabbTreeProxies.Erase(source);
}

ImplementResourceManager(AtlasManager, Atlas);

AtlasManager::AtlasManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  mNoFallbackNeeded = true;
}

void AtlasManager::AddSpriteSource(SpriteSource* source, Image* image)
{
  ErrorIf(source->mAtlas.IsNotNull(), "SpriteSource is already on an Atlas.");

  // Get frame count before adding pixel borders (if applicable). Must not be 0.
  source->mFramesPerRow = Math::Max(image->Width / source->FrameSizeX, 1u);

  int borderWidth = (source->Sampling == SpriteSampling::Linear) ? Atlas::sBorderWidth : 1;

  uint paddedSizeX = image->Width + (image->Width / source->FrameSizeX) * borderWidth * 2;
  uint paddedSizeY = image->Height + (image->Height / source->FrameSizeY) * borderWidth * 2;
  if (paddedSizeX > cMaxSpriteSize || paddedSizeY > cMaxSpriteSize)
  {
    borderWidth = 0;
    DoNotifyWarning("Warning",
                    String::Format("Cannot pad Sprite '%s', resulting image is too large. "
                                   "Sampling at sprite edge may be wrong.",
                                   source->Name.c_str()));
  }
  else
  {
    AddPixelBorders(image, source->FrameSizeX, source->FrameSizeY, borderWidth);
  }

  bool added = false;
  forRange (Resource* resource, AllResources())
  {
    Atlas* atlas = (Atlas*)resource;
    if (atlas->AddSpriteSource(source, image))
    {
      added = true;
      break;
    }
  }

  if (!added)
  {
    HandleOf<Atlas> atlas = Atlas::CreateRuntime();
    atlas->AddSpriteSource(source, image);
  }

  Texture* atlasTexture = source->GetAtlasTexture();
  ErrorIf(atlasTexture == nullptr, "Failed to place SpriteSource into an Atlas.");

  // Calculate frame uv's.
  Vec2 padUv = Vec2((float)borderWidth, (float)borderWidth) / Math::ToVec2(atlasTexture->GetSize());
  Vec2 frameUvSize = Vec2((float)source->FrameSizeX, (float)source->FrameSizeY) / Math::ToVec2(atlasTexture->GetSize());

  source->mBaseFrameUv.TopLeft = source->mAtlasUvRect.TopLeft + padUv;
  source->mBaseFrameUv.BotRight = source->mBaseFrameUv.TopLeft + frameUvSize;

  source->mPerFrameUvOffset = frameUvSize + padUv * 2.0f;
}

void AtlasManager::RemoveSpriteSource(SpriteSource* source)
{
  ErrorIf(source->mAtlas.IsNull(), "SpriteSource is not on an Atlas.");

  source->mAtlas->RemoveSpriteSource(source);
  source->mAtlas = nullptr;
}

} // namespace Raverie
