#include "Precompiled.hpp"

namespace Zero
{

void LoadTexture(StringParam filename, Texture* texture)
{
  texture->mFormat = TextureFormat::None;
  texture->mMipHeaders = nullptr;
  texture->mImageData = nullptr;
  texture->mTotalDataSize = 0;

  File file;
  file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential);

  if (!file.IsOpen())
    return;

  TextureHeader header;
  header.mFileId = 0;
  Read(file, header);

  if (header.mFileId != TextureFileId)
    return;

  // Check for compression support and fallback to downsized texture if needed
  if (header.mCompression != TextureCompression::None && Z::gRenderer->mDriverSupport.mTextureCompression == false)
  {
    // If a texture is compressed, the data file will have an uncompressed version of the texture
    // after the compressed data, including a separate file header
    uint dataSizeToSkip = header.mMipCount * sizeof(MipHeader) + header.mTotalDataSize;
    file.Seek(dataSizeToSkip, FileOrigin::Current);

    // Read new header
    header.mFileId = 0;
    Read(file, header);

    if (header.mFileId != TextureFileId)
      return;
  }

  MipHeader* mipHeaders = new MipHeader[header.mMipCount];
  byte* imageData = new byte[header.mTotalDataSize];

  Status status;
  file.Read(status, (byte*)mipHeaders, header.mMipCount * sizeof(MipHeader));
  file.Read(status, imageData, header.mTotalDataSize);

  if (status.Failed())
  {
    delete[] mipHeaders;
    delete[] imageData;
    return;
  }

  // Pull size off of top level
  texture->mWidth = mipHeaders->mWidth;
  texture->mHeight = mipHeaders->mHeight;

  texture->mMipCount = header.mMipCount;
  texture->mTotalDataSize = header.mTotalDataSize;
  texture->mMipHeaders = mipHeaders;
  texture->mImageData = imageData;

  texture->mType = (TextureType::Enum)header.mType;
  texture->mFormat = (TextureFormat::Enum)header.mFormat;
  texture->mCompression = (TextureCompression::Enum)header.mCompression;
  texture->mAddressingX = (TextureAddressing::Enum)header.mAddressingX;
  texture->mAddressingY = (TextureAddressing::Enum)header.mAddressingY;
  texture->mFiltering = (TextureFiltering::Enum)header.mFiltering;
  texture->mAnisotropy = (TextureAnisotropy::Enum)header.mAnisotropy;
  texture->mMipMapping = (TextureMipMapping::Enum)header.mMipMapping;
}

HandleOf<Resource> TextureLoader::LoadFromFile(ResourceEntry& entry)
{
  Texture* texture = new Texture();
  LoadTexture(entry.FullPath, texture);
  TextureManager::GetInstance()->AddResource(entry, texture);
  return texture;
}

void TextureLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  Texture* texture = (Texture*)resource;
  LoadTexture(entry.FullPath, texture);
  texture->SendModified();
}

HandleOf<Resource> TextureLoader::LoadFromBlock(ResourceEntry& entry)
{
  return nullptr;
}

} // namespace Zero
