// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

#define ReturnStatusIf(condition, string)                                                                              \
  if ((condition))                                                                                                     \
  {                                                                                                                    \
    status.SetFailed((string));                                                                                        \
    return;                                                                                                            \
  }

namespace Zero
{

// float CubicHermite(float t, float f0, float f1, float f2, float f3)
//{
//  float a = 0.0f * f0 + 2.0f * f1 + 0.0f * f2 + 0.0f * f3;
//  float b =-1.0f * f0 + 0.0f * f1 + 1.0f * f2 + 0.0f * f3;
//  float c = 2.0f * f0 +-5.0f * f1 + 4.0f * f2 +-1.0f * f3;
//  float d =-1.0f * f0 + 3.0f * f1 +-3.0f * f2 + 1.0f * f3;
//
//  return 0.5f * (a + t * b + t*t * c + t*t*t * d);
//}

template <typename T>
T* GetPixelClamped(T* image, int width, int height, uint numChannels, int x, int y)
{
  x = Math::Clamp(x, 0, width - 1);
  y = Math::Clamp(y, 0, height - 1);
  return &image[(x + y * width) * numChannels];
}

template <typename T, unsigned Channels>
void ResizeImage(const byte* srcImage,
                 uint srcWidth,
                 uint srcHeight,
                 byte* dstImage,
                 uint dstWidth,
                 uint dstHeight,
                 void (*ClampFunc)(float&))
{
  T* src = (T*)srcImage;
  T* dst = (T*)dstImage;

  // Bilinear resampling
  for (uint y = 0; y < dstHeight; ++y)
  {
    float v = (y + 0.5f) / dstHeight * srcHeight;
    int srcY = (int)Math::Round(v);
    float yt = v - srcY + 0.5f;

    for (uint x = 0; x < dstWidth; ++x)
    {
      float u = (x + 0.5f) / dstWidth * srcWidth;
      int srcX = (int)Math::Round(u);
      float xt = u - srcX + 0.5f;

      T* r00 = GetPixelClamped(src, srcWidth, srcHeight, Channels, srcX - 1, srcY - 1);
      T* r01 = GetPixelClamped(src, srcWidth, srcHeight, Channels, srcX + 0, srcY - 1);
      T* r10 = GetPixelClamped(src, srcWidth, srcHeight, Channels, srcX - 1, srcY + 0);
      T* r11 = GetPixelClamped(src, srcWidth, srcHeight, Channels, srcX + 0, srcY + 0);

      for (uint c = 0; c < Channels; ++c)
      {
        float c0 = Math::Lerp((float)r00[c], (float)r01[c], xt);
        float c1 = Math::Lerp((float)r10[c], (float)r11[c], xt);

        float value = Math::Lerp(c0, c1, yt);
        ClampFunc(value);
        dst[(x + y * dstWidth) * Channels + c] = (T)value;
      }
    }
  }
}

void FloatClamp(float& value)
{
  // No op
}

void ByteClamp(float& value)
{
  value = Math::Clamp(value, 0.0f, 255.0f);
}

void ShortClamp(float& value)
{
  value = Math::Clamp(value, 0.0f, 65535.0f);
}

void ResizeImage(TextureFormat::Enum format,
                 const byte* srcImage,
                 uint srcWidth,
                 uint srcHeight,
                 byte* dstImage,
                 uint dstWidth,
                 uint dstHeight)
{
  if (format == TextureFormat::RGB32f)
    ResizeImage<float, 3>(srcImage, srcWidth, srcHeight, dstImage, dstWidth, dstHeight, FloatClamp);
  else if (format == TextureFormat::RGBA16)
    ResizeImage<u16, 4>(srcImage, srcWidth, srcHeight, dstImage, dstWidth, dstHeight, ShortClamp);
  else
    ResizeImage<byte, 4>(srcImage, srcWidth, srcHeight, dstImage, dstWidth, dstHeight, ByteClamp);
}

void MipmapTexture(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format, bool compressed)
{
  if (mipHeaders.Size() != 1 || imageData.Size() != 1)
    return;

  // If mips are going to be compressed we need the top level image to be a
  // power of two in order to guarantee that every mip size is a multiple of 4
  // Pixel padding caused by compressing sizes 2 or 1 works correctly if image
  // needs to be y inverted
  if (compressed)
  {
    uint width = mipHeaders[0].mWidth;
    uint height = mipHeaders[0].mHeight;
    uint newWidth = NextPowerOfTwo(width - 1);
    uint newHeight = NextPowerOfTwo(height - 1);

    // If not already power of two
    if (width != newWidth || height != newHeight)
    {
      uint newSize = newWidth * newHeight * GetPixelSize(format);
      byte* newImage = new byte[newSize];
      ResizeImage(format, imageData[0], width, height, newImage, newWidth, newHeight);

      delete[] imageData[0];
      imageData[0] = newImage;

      mipHeaders[0].mWidth = newWidth;
      mipHeaders[0].mHeight = newHeight;
      mipHeaders[0].mDataSize = newSize;
    }
  }

  uint width = mipHeaders[0].mWidth;
  uint height = mipHeaders[0].mHeight;
  uint pixelSize = GetPixelSize(format);

  byte* image = imageData[0];
  uint level = 1;
  uint dataOffset = mipHeaders[0].mDataSize;

  while (width > 1 || height > 1)
  {
    uint newWidth = Math::Max(1u, width / 2);
    uint newHeight = Math::Max(1u, height / 2);
    uint newSize = newWidth * newHeight * pixelSize;

    byte* newImage = new byte[newSize];
    ResizeImage(format, image, width, height, newImage, newWidth, newHeight);

    MipHeader header;
    header.mFace = TextureFace::None;
    header.mLevel = level;
    header.mWidth = newWidth;
    header.mHeight = newHeight;
    header.mDataOffset = dataOffset;
    header.mDataSize = newSize;

    mipHeaders.PushBack(header);
    imageData.PushBack(newImage);

    width = newWidth;
    height = newHeight;
    image = newImage;
    ++level;
    dataOffset += newSize;
  }
}

class CompressionOutput : public nvtt::OutputHandler
{
public:
  CompressionOutput()
  {
    mData = nullptr;
    mSize = 0;
    mCurrentLocation = 0;
  }

  void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
  {
    mSize = size;
    mData = new byte[mSize];
    mCurrentLocation = 0;
  }

  bool writeData(const void* data, int size) override
  {
    if (mData == nullptr)
    {
      // ignore header
      // mHeader = new byte[size];
      // memcpy(mHeader, data, size);
      // mHeaderSize = size;
      return true;
    }

    if (mCurrentLocation + size > mSize)
    {
      DebugPrint("Write error\n");
      return false;
    }

    memcpy(mData + mCurrentLocation, data, size);
    mCurrentLocation += size;
    return true;
  }

  void endImage() override
  {
  }

  byte* mHeader;
  uint mHeaderSize;
  byte* mData;
  uint mSize;
  uint mCurrentLocation;
};

class CompressedError : public nvtt::ErrorHandler
{
public:
  void error(nvtt::Error e) override
  {
    const char* message = nvtt::errorString(e);
    ZPrint("Nvtt Error: %s", message);
  }
};

void ToNvttSurface(nvtt::Surface& surface, uint width, uint height, TextureFormat::Enum format, const byte* image)
{
  byte* convertedImage = nullptr;

  // Nvtt requires input data to be BGRA8 or RGBA32f
  if (format == TextureFormat::RGB32f)
  {
    uint size = width * height * GetPixelSize(TextureFormat::RGBA32f);
    convertedImage = new byte[size];

    const float* srcImage = (const float*)image;
    float* destImage = (float*)convertedImage;

    for (uint i = 0; i < width * height; ++i)
    {
      destImage[i * 4 + 0] = srcImage[i * 3 + 0];
      destImage[i * 4 + 1] = srcImage[i * 3 + 1];
      destImage[i * 4 + 2] = srcImage[i * 3 + 2];
      destImage[i * 4 + 3] = 1.0f;
    }
  }
  else
  {
    uint pixelSize = GetPixelSize(format);
    uint size = width * height * pixelSize;
    convertedImage = new byte[size];
    memcpy(convertedImage, image, size);

    for (uint i = 0; i < size; i += pixelSize)
      Math::Swap(convertedImage[i], convertedImage[i + 2]);
  }

  nvtt::InputFormat inputFormat =
      format == TextureFormat::RGB32f ? nvtt::InputFormat_RGBA_32F : nvtt::InputFormat_BGRA_8UB;
  surface.setImage(inputFormat, width, height, 1, convertedImage);

  delete[] convertedImage;
}

void FromNvttSurface(const nvtt::Surface& surface, uint& width, uint& height, TextureFormat::Enum format, byte*& image)
{
  const float* srcImage = surface.data();
  width = surface.width();
  height = surface.height();

  uint pixelSize = GetPixelSize(format);
  uint size = width * height * pixelSize;
  image = new byte[size];

  if (format == TextureFormat::RGB32f)
  {
    float* destImage = (float*)image;
    for (uint i = 0; i < width * height; ++i)
    {
      destImage[i * 3 + 0] = srcImage[(width * height * 0) + i];
      destImage[i * 3 + 1] = srcImage[(width * height * 1) + i];
      destImage[i * 3 + 2] = srcImage[(width * height * 2) + i];
    }
  }
  else
  {
    for (uint i = 0; i < width * height; ++i)
    {
      image[i * 4 + 0] = (byte)(Math::Clamp(srcImage[(width * height * 0) + i] * 255.0f, 0.0f, 255.0f));
      image[i * 4 + 1] = (byte)(Math::Clamp(srcImage[(width * height * 1) + i] * 255.0f, 0.0f, 255.0f));
      image[i * 4 + 2] = (byte)(Math::Clamp(srcImage[(width * height * 2) + i] * 255.0f, 0.0f, 255.0f));
      image[i * 4 + 3] = (byte)(Math::Clamp(srcImage[(width * height * 3) + i] * 255.0f, 0.0f, 255.0f));
    }
  }
}

TextureImporter::TextureImporter(StringParam inputFile, StringParam outputFile, StringParam metaFile) :
    mInputFile(inputFile),
    mOutputFile(outputFile),
    mMetaFile(metaFile),
    mLoadFormat(TextureFormat::None),
    mImageContent(nullptr),
    mBuilder(nullptr),
    mMetaChanged(false)
{
  if (metaFile.Empty())
    mMetaFile = BuildString(inputFile, ".meta");
}

TextureImporter::~TextureImporter()
{
  for (size_t i = 0; i < mImageData.Size(); ++i)
    delete[] mImageData[i];

  for (size_t i = 0; i < mBackupImageData.Size(); ++i)
    delete[] mBackupImageData[i];
}

nvtt::Format NvttFormat(TextureCompression::Enum compression)
{
  switch (compression)
  {
  case TextureCompression::BC1:
    return nvtt::Format_BC1;
  case TextureCompression::BC2:
    return nvtt::Format_BC2;
  case TextureCompression::BC3:
    return nvtt::Format_BC3;
  case TextureCompression::BC4:
    return nvtt::Format_BC4;
  case TextureCompression::BC5:
    return nvtt::Format_BC5;
  case TextureCompression::BC6:
    return nvtt::Format_BC6;
  default:
    return (nvtt::Format)0;
  }
}

float GetCompressionRatio(TextureCompression::Enum compression)
{
  switch (compression)
  {
  case TextureCompression::BC1:
    return 1.0f / 8.0f;
  case TextureCompression::BC2:
    return 1.0f / 4.0f;
  case TextureCompression::BC3:
    return 1.0f / 4.0f;
  case TextureCompression::BC4:
    return 1.0f / 8.0f;
  case TextureCompression::BC5:
    return 1.0f / 4.0f;
  case TextureCompression::BC6:
    return 1.0f / 12.0f;
  default:
    return 0.0f;
  }
}

ImageProcessorCodes::Enum TextureImporter::ProcessTexture(Status& status)
{
  if (!FileExists(mInputFile))
  {
    ZPrint("Missing image file '%s'\n", mInputFile.c_str());
    return ImageProcessorCodes::Failed;
  }

  if (!FileExists(mMetaFile))
  {
    ZPrint("Missing meta file '%s'\n", mMetaFile.c_str());
    return ImageProcessorCodes::Failed;
  }

  mImageContent = new ImageContent();
  bool metaLoaded = LoadFromDataFile(*mImageContent, mMetaFile);
  mBuilder = mImageContent->has(TextureBuilder);

  if (metaLoaded == false || mBuilder == nullptr)
  {
    mBuilder = nullptr;
    delete mImageContent;
    status.SetFailed(String::Format("Failed to load meta file '%s'", mMetaFile.c_str()));
    return ImageProcessorCodes::Failed;
  }

  String extension = FilePath::GetExtension(mInputFile);

  LoadImageData(status, extension);

  if (status.Failed())
  {
    mBuilder = nullptr;
    delete mImageContent;
    return ImageProcessorCodes::Failed;
  }

  // Progressive downsample, done before any other processing
  if (mBuilder->mHalfScaleCount > 0)
  {
    for (int i = 0; i < mBuilder->mHalfScaleCount; ++i)
    {
      uint width = mMipHeaders[0].mWidth;
      uint height = mMipHeaders[0].mHeight;
      uint pixelSize = GetPixelSize(mLoadFormat);

      // Stop if image can't be down scaled anymore
      if (width > 1 || height > 1)
      {
        // Downscale by 1/2
        uint newWidth = Math::Max(width / 2, 1u);
        uint newHeight = Math::Max(height / 2, 1u);
        uint newSize = newWidth * newHeight * pixelSize;
        byte* newImageData = new byte[newSize];
        ResizeImage(mLoadFormat, mImageData[0], width, height, newImageData, newWidth, newHeight);

        mMipHeaders[0].mWidth = newWidth;
        mMipHeaders[0].mHeight = newHeight;
        mMipHeaders[0].mDataSize = newSize;
        delete mImageData[0];
        mImageData[0] = newImageData;
      }
      else
      {
        break;
      }
    }
  }

  // Convert to bytes for nvtt if image is going to be compressed
  if (mLoadFormat == TextureFormat::RGBA16 && mBuilder->mCompression != TextureCompression::None)
  {
    u16* imageData = (u16*)mImageData[0];

    mLoadFormat = TextureFormat::RGBA8;
    uint width = mMipHeaders[0].mWidth;
    uint height = mMipHeaders[0].mHeight;
    uint pixelSize = GetPixelSize(mLoadFormat);
    uint newSize = width * height * pixelSize;

    mMipHeaders[0].mDataSize = newSize;

    byte* newImageData = new byte[newSize];
    for (uint i = 0; i < newSize; ++i)
    {
      float normalized = imageData[i] / 65535.0f;
      newImageData[i] = (byte)(normalized * 255.0f);
    }

    delete[] mImageData[0];
    mImageData[0] = newImageData;
  }

  String fileType = extension;
  String loadFormat = TextureFormat::Names[mLoadFormat];

  if (mLoadFormat == TextureFormat::RGBA8)
  {
    byte* imageData = mImageData[0];
    uint pixelCount = mMipHeaders[0].mWidth * mMipHeaders[0].mHeight;
    uint pixelSize = GetPixelSize(mLoadFormat);

    if (mBuilder->mPremultipliedAlpha)
    {
      for (uint i = 0; i < pixelCount; ++i)
      {
        byte* pixel = (byte*)(imageData + i * pixelSize);

        float alpha = pixel[3] / 255.0f;
        pixel[0] = (byte)(pixel[0] * alpha);
        pixel[1] = (byte)(pixel[1] * alpha);
        pixel[2] = (byte)(pixel[2] * alpha);
      }
    }

    if (mBuilder->mGammaCorrection)
    {
      for (uint i = 0; i < pixelCount; ++i)
      {
        byte* pixel = (byte*)(imageData + i * pixelSize);

        pixel[0] = (byte)Math::Clamp(Math::Pow(pixel[0] / 255.0f, 2.2f) * 255.0f, 0.0f, 255.0f);
        pixel[1] = (byte)Math::Clamp(Math::Pow(pixel[1] / 255.0f, 2.2f) * 255.0f, 0.0f, 255.0f);
        pixel[2] = (byte)Math::Clamp(Math::Pow(pixel[2] / 255.0f, 2.2f) * 255.0f, 0.0f, 255.0f);
      }
    }
  }
  else if (mLoadFormat == TextureFormat::RGBA16)
  {
    byte* imageData = mImageData[0];
    uint pixelCount = mMipHeaders[0].mWidth * mMipHeaders[0].mHeight;
    uint pixelSize = GetPixelSize(mLoadFormat);

    if (mBuilder->mPremultipliedAlpha)
    {
      for (uint i = 0; i < pixelCount; ++i)
      {
        u16* pixel = (u16*)(imageData + i * pixelSize);

        float alpha = pixel[3] / 65535.0f;
        pixel[0] = (u16)(pixel[0] * alpha);
        pixel[1] = (u16)(pixel[1] * alpha);
        pixel[2] = (u16)(pixel[2] * alpha);
      }
    }

    if (mBuilder->mGammaCorrection)
    {
      for (uint i = 0; i < pixelCount; ++i)
      {
        u16* pixel = (u16*)(imageData + i * pixelSize);

        pixel[0] = (u16)Math::Clamp(Math::Pow(pixel[0] / 65535.0f, 2.2f) * 65535.0f, 0.0f, 65535.0f);
        pixel[1] = (u16)Math::Clamp(Math::Pow(pixel[1] / 65535.0f, 2.2f) * 65535.0f, 0.0f, 65535.0f);
        pixel[2] = (u16)Math::Clamp(Math::Pow(pixel[2] / 65535.0f, 2.2f) * 65535.0f, 0.0f, 65535.0f);
      }
    }
  }

  if (mBuilder->mType == TextureType::TextureCube)
  {
    ExtractCubemapFaces(status, mMipHeaders, mImageData, mLoadFormat);
    if (status.Failed())
    {
      // Resort to builing Texture2D so the resource at least builds
      mBuilder->mType = TextureType::Texture2D;
      mMetaChanged = true;
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      status.Reset();
    }
  }

  // Save a downsized backup of the texture for drivers that don't support
  // texture compression
  if (mBuilder->mCompression != TextureCompression::None)
  {
    mBackupMipHeaders.Resize(mMipHeaders.Size());
    mBackupImageData.Resize(mImageData.Size());

    uint pixelSize = GetPixelSize(mLoadFormat);
    uint dataOffset = 0;
    float dimensionScale = Math::Sqrt(GetCompressionRatio(mBuilder->mCompression));

    for (uint i = 0; i < mMipHeaders.Size(); ++i)
    {
      mBackupMipHeaders[i].mFace = mMipHeaders[i].mFace;
      mBackupMipHeaders[i].mLevel = mMipHeaders[i].mLevel;
      mBackupMipHeaders[i].mWidth = (uint)Math::Max(mMipHeaders[i].mWidth * dimensionScale, 1.0f);
      mBackupMipHeaders[i].mHeight = (uint)Math::Max(mMipHeaders[i].mHeight * dimensionScale, 1.0f);
      mBackupMipHeaders[i].mDataOffset = dataOffset;
      mBackupMipHeaders[i].mDataSize = mBackupMipHeaders[i].mWidth * mBackupMipHeaders[i].mHeight * pixelSize;
      dataOffset += mBackupMipHeaders[i].mDataSize;

      mBackupImageData[i] = new byte[mBackupMipHeaders[i].mDataSize];
      ResizeImage(mLoadFormat,
                  mImageData[i],
                  mMipHeaders[i].mWidth,
                  mMipHeaders[i].mHeight,
                  mBackupImageData[i],
                  mBackupMipHeaders[i].mWidth,
                  mBackupMipHeaders[i].mHeight);
    }
  }

  if (mBuilder->mMipMapping == TextureMipMapping::PreGenerated)
  {
    bool compressed = mBuilder->mCompression != TextureCompression::None;
    if (mBuilder->mType == TextureType::Texture2D)
      MipmapTexture(mMipHeaders, mImageData, mLoadFormat, compressed);
    else if (mBuilder->mType == TextureType::TextureCube)
      MipmapCubemap(mMipHeaders, mImageData, mLoadFormat, compressed);

    if (mBackupMipHeaders.Size())
    {
      if (mBuilder->mType == TextureType::Texture2D)
        MipmapTexture(mBackupMipHeaders, mBackupImageData, mLoadFormat, false);
      else if (mBuilder->mType == TextureType::TextureCube)
        MipmapCubemap(mBackupMipHeaders, mBackupImageData, mLoadFormat, false);
    }
  }

  if (mBuilder->mCompression != TextureCompression::None)
  {
    uint dataOffset = 0;
    for (uint i = 0; i < mMipHeaders.Size(); ++i)
    {
      uint width = mMipHeaders[i].mWidth;
      uint height = mMipHeaders[i].mHeight;

      // Compressed image must be a multiple of 4
      // Pixel padding caused by compressing sizes 2 or 1 works correctly if
      // image needs to be y inverted
      uint newWidth = width;
      uint newHeight = height;
      if (width >= 3 && width % 4 != 0)
        newWidth = width + 4 - width % 4;
      if (height >= 3 && height % 4 != 0)
        newHeight = height + 4 - height % 4;
      if (newWidth != width || newHeight != height)
      {
        uint newSize = newWidth * newHeight * GetPixelSize(mLoadFormat);
        byte* newImage = new byte[newSize];

        ResizeImage(mLoadFormat, mImageData[i], width, height, newImage, newWidth, newHeight);

        delete[] mImageData[i];
        mImageData[i] = newImage;

        width = newWidth;
        height = newHeight;
      }

      nvtt::Surface surface;
      ToNvttSurface(surface, width, height, mLoadFormat, mImageData[i]);

      nvtt::CompressionOptions compressionOptions;
      compressionOptions.setFormat(NvttFormat(mBuilder->mCompression));
      compressionOptions.setQuality(nvtt::Quality_Fastest);

      CompressionOutput compressionOutput;
      CompressedError compressedError;

      nvtt::OutputOptions outputOptions;
      outputOptions.setOutputHandler(&compressionOutput);
      outputOptions.setErrorHandler(&compressedError);
      outputOptions.setContainer(nvtt::Container_DDS10);

      nvtt::Context context;

      // NVidia texture tools uses threads (pthreads on Emscripten) and
      // since threads are disabled, this unfortunately just freezes in
      // browsers. For now, we actually support not having compressed textures.
      bool result = context.compress(surface, 0, 0, compressionOptions, outputOptions);

      if (!result)
      {
        mBuilder = nullptr;
        delete mImageContent;
        status.SetFailed("Compression failed");
        return ImageProcessorCodes::Failed;
      }

      mMipHeaders[i].mWidth = width;
      mMipHeaders[i].mHeight = height;
      mMipHeaders[i].mDataSize = compressionOutput.mSize;
      mMipHeaders[i].mDataOffset = dataOffset;
      dataOffset += mMipHeaders[i].mDataSize;

      delete[] mImageData[i];
      mImageData[i] = compressionOutput.mData;
    }
  }

  String dimensions = String::Format("%d x %d", mMipHeaders[0].mWidth, mMipHeaders[0].mHeight);

  MipHeader& mipHeader = mMipHeaders.Back();
  float dataSize = (mipHeader.mDataOffset + mipHeader.mDataSize) / (1024.0f * 1024.0f);
  // This keeps the number consistent accross platforms (sometimes printf behavior is different).
  float fixedSize = Math::Floor(dataSize * 1000.0f) / 1000.0f;
  String size = String::Format("%.3f MB", fixedSize);

  // Check for meta update
  TextureInfo* info = mImageContent->has(TextureInfo);
  if (info == nullptr)
  {
    info = new TextureInfo();
    mImageContent->AddComponent(info);
  }

  if (info->mFileType != fileType || info->mLoadFormat != loadFormat || info->mDimensions != dimensions ||
      info->mSize != size)
  {
    info->mFileType = fileType;
    info->mLoadFormat = loadFormat;
    info->mDimensions = dimensions;
    info->mSize = size;
    SaveToDataFile(*mImageContent, mMetaFile);
    mMetaChanged = true;
  }

  // Write output
  WriteTextureFile(status);

  mBuilder = nullptr;
  delete mImageContent;

  if (status.Failed())
    return ImageProcessorCodes::Failed;
  else if (mMetaChanged)
    return ImageProcessorCodes::Reload;
  else
    return ImageProcessorCodes::Success;
}

void TextureImporter::LoadImageData(Status& status, StringParam extension)
{
  File file;
  if (!file.Open(mInputFile.c_str(), FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status))
    return;

  FileStream stream(file);

  uint width, height;
  byte* imageData = nullptr;
  bool imageLoadAttempted = false;

#ifdef ZeroCustomPngSupport
  if (IsPng(&stream))
  {
    LoadPng(status, &stream, &imageData, &width, &height, &mLoadFormat);
    if (imageData != nullptr)
      AddImageData(imageData, width, height);
    return;
  }
#endif

#ifdef ZeroCustomHdrSupport
  if (IsHdr(&stream))
  {
    LoadHdr(status, &stream, &imageData, &width, &height, &mLoadFormat);
    if (imageData != nullptr)
      AddImageData(imageData, width, height);
    return;
  }
#endif

  LoadImage(status, &stream, &imageData, &width, &height, &mLoadFormat, TextureFormat::RGBA8);
  if (imageData != nullptr)
    AddImageData(imageData, width, height);
}

void TextureImporter::WriteTextureFile(Status& status)
{
  File file;
  file.Open(mOutputFile.c_str(), FileMode::Write, FileAccessPattern::Sequential);

  ReturnStatusIf(!file.IsOpen(), String::Format("Can not open output file '%s'", mOutputFile.c_str()));

  TextureHeader header;
  header.mFileId = TextureFileId;
  header.mFileVersion = TextureFileVersion;
  header.mType = mBuilder->mType;
  header.mFormat = mLoadFormat;
  header.mCompression = mBuilder->mCompression;
  header.mAddressingX = mBuilder->mAddressingX;
  header.mAddressingY = mBuilder->mAddressingY;
  header.mFiltering = mBuilder->mFiltering;
  header.mAnisotropy = mBuilder->mAnisotropy;
  header.mMipMapping = mBuilder->mMipMapping;

  uint totalDataSize = 0;
  for (size_t i = 0; i < mMipHeaders.Size(); ++i)
    totalDataSize += mMipHeaders[i].mDataSize;

  header.mMipCount = mMipHeaders.Size();
  header.mTotalDataSize = totalDataSize;

  file.Write((byte*)&header, sizeof(TextureHeader));

  file.Write((byte*)mMipHeaders.Data(), mMipHeaders.Size() * sizeof(MipHeader));

  for (size_t i = 0; i < mMipHeaders.Size(); ++i)
    file.Write(mImageData[i], mMipHeaders[i].mDataSize);

  if (mBackupMipHeaders.Size())
  {
    header.mCompression = TextureCompression::None;

    uint totalDataSize = 0;
    for (size_t i = 0; i < mBackupMipHeaders.Size(); ++i)
      totalDataSize += mBackupMipHeaders[i].mDataSize;

    header.mMipCount = mBackupMipHeaders.Size();
    header.mTotalDataSize = totalDataSize;

    file.Write((byte*)&header, sizeof(TextureHeader));

    file.Write((byte*)mBackupMipHeaders.Data(), mBackupMipHeaders.Size() * sizeof(MipHeader));

    for (size_t i = 0; i < mBackupMipHeaders.Size(); ++i)
      file.Write(mBackupImageData[i], mBackupMipHeaders[i].mDataSize);
  }
}

void TextureImporter::AddImageData(byte* imageData, uint width, uint height)
{
  MipHeader header;
  header.mFace = TextureFace::None;
  header.mLevel = 0;
  header.mWidth = width;
  header.mHeight = height;
  header.mDataOffset = 0;
  header.mDataSize = width * height * GetPixelSize(mLoadFormat);

  mMipHeaders.PushBack(header);
  mImageData.PushBack(imageData);
}

} // namespace Zero
