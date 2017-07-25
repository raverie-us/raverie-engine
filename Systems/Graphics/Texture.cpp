////////////////////////////////////////////////////////////////////////////////
/// Authors: Nathan Carlson
/// Copyright 2016, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#define CheckProtected() if (mProtected) return DoNotifyException("Error", "Cannot modify non-runtime Textures.");

#define SetValue(member, value) \
  if (value == member)          \
    return;                     \
  member = value;               \
  mDirty = true;

namespace Zero
{

ZilchDefineType(Texture, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(CreateRuntime);

  ZilchBindFieldGetter(mType);
  ZilchBindFieldGetter(mCompression);
  ZilchBindFieldGetter(mWidth);
  ZilchBindFieldGetter(mHeight);
  ZilchBindGetterProperty(Size);
  ZilchBindGetterProperty(Format);
  ZilchBindGetterSetterProperty(AddressingX);
  ZilchBindGetterSetterProperty(AddressingY);
  ZilchBindGetterSetterProperty(Filtering);
  ZilchBindGetterSetterProperty(Anisotropy);
  ZilchBindGetterSetterProperty(MipMapping);
  ZilchBindGetterSetterProperty(CompareMode);
  ZilchBindGetterSetterProperty(CompareFunc);

  ZilchBindOverloadedMethod(Upload, ZilchInstanceOverload(void, TextureData&));
  ZilchBindOverloadedMethod(SubUpload, ZilchInstanceOverload(void, TextureData&, int, int));
}

HandleOf<Texture> Texture::CreateRuntime()
{
  Texture* texture = TextureManager::CreateRuntime();

  // Allow change of settings on runtime textures
  texture->mProtected = false;

  Z::gEngine->has(GraphicsEngine)->AddTexture(texture);
  return texture;
}

Texture::Texture()
  : mRenderData(nullptr)
{
  mType = TextureType::Texture2D;
  mCompression = TextureCompression::None;

  mWidth = 0;
  mHeight = 0;

  mFormat = TextureFormat::None;
  mAddressingX = TextureAddressing::Clamp;
  mAddressingY = TextureAddressing::Clamp;
  mFiltering = TextureFiltering::Nearest;
  mAnisotropy = TextureAnisotropy::x1;
  mMipMapping = TextureMipMapping::None;
  mCompareMode = TextureCompareMode::Disabled;
  mCompareFunc = TextureCompareFunc::Never;

  mMipCount = 0;
  mTotalDataSize = 0;
  mMipHeaders = nullptr;
  mImageData = nullptr;

  mProtected = true;
  mDirty = false;
}

IntVec2 Texture::GetSize()
{
  return IntVec2(mWidth, mHeight);
}

void Texture::SetSize(IntVec2 size)
{
  CheckProtected();

  size = Math::Clamp(size, IntVec2(1, 1), IntVec2(4096, 4096));
  if (size.x == mWidth && size.y == mHeight)
    return;

  mWidth = size.x;
  mHeight = size.y;
  mDirty = true;
}

TextureFormat::Enum Texture::GetFormat()
{
  return mFormat;
}

void Texture::SetFormat(TextureFormat::Enum format)
{
  CheckProtected();
  SetValue(mFormat, format);
}

TextureAddressing::Enum Texture::GetAddressingX()
{
  return mAddressingX;
}

void Texture::SetAddressingX(TextureAddressing::Enum addressingX)
{
  CheckProtected();
  SetValue(mAddressingX, addressingX);
}

TextureAddressing::Enum Texture::GetAddressingY()
{
  return mAddressingY;
}

void Texture::SetAddressingY(TextureAddressing::Enum addressingY)
{
  CheckProtected();
  SetValue(mAddressingY, addressingY);
}

TextureFiltering::Enum Texture::GetFiltering()
{
  return mFiltering;
}

void Texture::SetFiltering(TextureFiltering::Enum filtering)
{
  CheckProtected();
  SetValue(mFiltering, filtering);
}

TextureAnisotropy::Enum Texture::GetAnisotropy()
{
  return mAnisotropy;
}

void Texture::SetAnisotropy(TextureAnisotropy::Enum anisotropy)
{
  CheckProtected();
  SetValue(mAnisotropy, anisotropy);
}

TextureMipMapping::Enum Texture::GetMipMapping()
{
  return mMipMapping;
}

void Texture::SetMipMapping(TextureMipMapping::Enum mipMapping)
{
  CheckProtected();

  if (mipMapping == TextureMipMapping::PreGenerated)
    return DoNotifyException("Error", "Cannot pre-generate mipmaps for a runtime Texture.");

  SetValue(mMipMapping, mipMapping);
}

TextureCompareMode::Enum Texture::GetCompareMode()
{
  return mCompareMode;
}

void Texture::SetCompareMode(TextureCompareMode::Enum compareMode)
{
  CheckProtected();
  SetValue(mCompareMode, compareMode);
}

TextureCompareFunc::Enum Texture::GetCompareFunc()
{
  return mCompareFunc;
}

void Texture::SetCompareFunc(TextureCompareFunc::Enum compareFunc)
{
  CheckProtected();
  SetValue(mCompareFunc, compareFunc);
}

void Texture::Upload(TextureData& textureData)
{
  CheckProtected();

  if (textureData.mPixelCount == 0)
    return DoNotifyException("Error", "No data.");

  mWidth = textureData.mWidth;
  mHeight = textureData.mHeight;
  mFormat = textureData.mFormat;

  mMipCount = 1;
  mTotalDataSize = textureData.mDataSize;

  mImageData = new byte[mTotalDataSize];
  memcpy(mImageData, textureData.mData, mTotalDataSize);

  mMipHeaders = new MipHeader[1];
  mMipHeaders->mWidth = mWidth;
  mMipHeaders->mHeight = mHeight;
  mMipHeaders->mFace = TextureFace::None;
  mMipHeaders->mLevel = 0;
  mMipHeaders->mDataOffset = 0;
  mMipHeaders->mDataSize = mTotalDataSize;

  Z::gEngine->has(GraphicsEngine)->AddTexture(this);
}

void Texture::SubUpload(TextureData& textureData, int xOffset, int yOffset)
{
  CheckProtected();

  xOffset = Math::Max(xOffset, 0);
  yOffset = Math::Max(yOffset, 0);

  if (textureData.mPixelCount == 0)
    return DoNotifyException("Error", "No data.");

  if (textureData.mFormat != mFormat)
    return DoNotifyException("Error", "Not the same format.");

  if (xOffset + textureData.mWidth > mWidth || yOffset + textureData.mHeight > mHeight)
    return DoNotifyException("Error", "Sub image goes outside texture bounds.");

  mMipCount = 1;

  mImageData = new byte[textureData.mDataSize];
  memcpy(mImageData, textureData.mData, textureData.mDataSize);

  mMipHeaders = new MipHeader[1];
  mMipHeaders->mWidth = textureData.mWidth;
  mMipHeaders->mHeight = textureData.mHeight;
  mMipHeaders->mFace = TextureFace::None;
  mMipHeaders->mLevel = 0;
  mMipHeaders->mDataOffset = 0;
  mMipHeaders->mDataSize = mTotalDataSize;

  Z::gEngine->has(GraphicsEngine)->AddTexture(this, true, xOffset, yOffset);
}

void Texture::Upload(uint width, uint height, TextureFormat::Enum format, byte* data, uint size, bool copyData)
{
  mWidth = width;
  mHeight = height;
  mFormat = format;

  if (data != nullptr)
  {
    mMipCount = 1;
    mTotalDataSize = size;

    if (copyData)
    {
      mImageData = new byte[mTotalDataSize];
      memcpy(mImageData, data, mTotalDataSize);
    }
    else
    {
      mImageData = data;
    }

    mMipHeaders = new MipHeader[1];
    mMipHeaders->mWidth = mWidth;
    mMipHeaders->mHeight = mHeight;
    mMipHeaders->mFace = TextureFace::None;
    mMipHeaders->mLevel = 0;
    mMipHeaders->mDataOffset = 0;
    mMipHeaders->mDataSize = mTotalDataSize;
  }

  Z::gEngine->has(GraphicsEngine)->AddTexture(this);
}

void Texture::Upload(Image& image)
{
  mWidth = image.Width;
  mHeight = image.Height;
  mFormat = TextureFormat::RGBA8;

  mMipCount = 1;
  mTotalDataSize = image.SizeInBytes;

  mImageData = new byte[image.SizeInBytes];
  memcpy(mImageData, image.Data, image.SizeInBytes);

  mMipHeaders = new MipHeader[1];
  mMipHeaders->mWidth = mWidth;
  mMipHeaders->mHeight = mHeight;
  mMipHeaders->mFace = TextureFace::None;
  mMipHeaders->mLevel = 0;
  mMipHeaders->mDataOffset = 0;
  mMipHeaders->mDataSize = mTotalDataSize;

  Z::gEngine->has(GraphicsEngine)->AddTexture(this);
}

void Texture::SubUpload(Image& image, int xOffset, int yOffset)
{
  xOffset = Math::Max(xOffset, 0);
  yOffset = Math::Max(yOffset, 0);

  if (mFormat != TextureFormat::RGBA8)
    return DoNotifyException("Error", "Texture is not RGBA8 format.");

  if (xOffset + (uint)image.Width > mWidth || yOffset + (uint)image.Height > mHeight)
    return DoNotifyException("Error", "Sub image goes outside texture bounds.");

  mMipCount = 1;

  mImageData = new byte[image.SizeInBytes];
  memcpy(mImageData, image.Data, image.SizeInBytes);

  mMipHeaders = new MipHeader[1];
  mMipHeaders->mWidth = image.Width;
  mMipHeaders->mHeight = image.Height;
  mMipHeaders->mFace = TextureFace::None;
  mMipHeaders->mLevel = 0;
  mMipHeaders->mDataOffset = 0;
  mMipHeaders->mDataSize = mTotalDataSize;

  Z::gEngine->has(GraphicsEngine)->AddTexture(this, true, xOffset, yOffset);
}

ImplementResourceManager(TextureManager, Texture);

TextureManager::TextureManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader(ZTexLoader, new TextureLoader());

  DefaultResourceName = "Grey";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Images", "*.png;*.hdr"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.png"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.hdr"));
  mCategory = "Graphics";
  mCanReload = true;
  mPreview = true;
}

} // namespace Zero
