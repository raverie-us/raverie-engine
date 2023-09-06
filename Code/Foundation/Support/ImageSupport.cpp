// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// We privately include this file because it includes the full implementation
// (as if it were a .c file)
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_WRITE_NO_STDIO
#include "stb_image.h"
#include "stb_image_write.h"

namespace Zero
{

// Fill 'data' with 'size' bytes and return number of bytes actually read
static int StbRead(void* userData, char* data, int size)
{
  Stream* stream = (Stream*)userData;
  return stream->Read((byte*)data, size);
}

static void StbSkip(void* userData, int bytes)
{
  Stream* stream = (Stream*)userData;
  stream->Seek((FilePosition)bytes, SeekOrigin::Current);
}

static int StbEof(void* userData)
{
  Stream* stream = (Stream*)userData;
  return stream->IsEof();
}

static void StbWrite(void* userData, void* data, int size)
{
  Stream* stream = (Stream*)userData;
  size_t amountWritten = stream->Write((byte*)data, (size_t)size);
  ErrorIf(amountWritten != (size_t)size,
          "Not all bytes were written in call to StbWrite "
          "(stb does not support write calls that don't write all bytes)");
}

static stbi_io_callbacks StbStreamCallbacks()
{
  stbi_io_callbacks callbacks;
  callbacks.read = &StbRead;
  callbacks.skip = &StbSkip;
  callbacks.eof = &StbEof;
  return callbacks;
}

bool IsImageLoadFormat(TextureFormat::Enum format)
{
  return format == TextureFormat::None || IsImageSaveFormat(format);
}

bool IsImageSaveFormat(TextureFormat::Enum format)
{
  return format == TextureFormat::R8 || format == TextureFormat::RG8 || format == TextureFormat::RGB8 ||
         format == TextureFormat::RGBA8 || format == TextureFormat::SRGB8 || format == TextureFormat::SRGB8A8 ||
         format == TextureFormat::R16 || format == TextureFormat::RG16 || format == TextureFormat::RGB16 ||
         format == TextureFormat::RGBA16 || format == TextureFormat::R32f || format == TextureFormat::RG32f ||
         format == TextureFormat::RGB32f || format == TextureFormat::RGBA32f;
}

TextureFormat::Enum ToImageFormat(int components, ImageBitDepth::Enum depth)
{
  switch (depth)
  {
  case ImageBitDepth::I8:
    switch (components)
    {
    case 1:
      return TextureFormat::R8;
    case 2:
      return TextureFormat::RG8;
    case 3:
      return TextureFormat::RGB8;
    case 4:
      return TextureFormat::RGBA8;
    }
    break;
  case ImageBitDepth::I16:
    switch (components)
    {
    case 1:
      return TextureFormat::R16;
    case 2:
      return TextureFormat::RG16;
    case 3:
      return TextureFormat::RGB16;
    case 4:
      return TextureFormat::RGBA16;
    }
    break;
  case ImageBitDepth::F32:
    switch (components)
    {
    case 1:
      return TextureFormat::R32f;
    case 2:
      return TextureFormat::RG32f;
    case 3:
      return TextureFormat::RGB32f;
    case 4:
      return TextureFormat::RGBA32f;
    }
    break;
  default:
    break;
  }
  return TextureFormat::None;
}

void FromImageFormat(TextureFormat::Enum format, int* components, ImageBitDepth::Enum* depth)
{
  switch (format)
  {
  case TextureFormat::R8:
    *components = 1;
    *depth = ImageBitDepth::I8;
    return;
  case TextureFormat::RG8:
    *components = 2;
    *depth = ImageBitDepth::I8;
    return;
  case TextureFormat::RGB8:
    *components = 3;
    *depth = ImageBitDepth::I8;
    return;
  case TextureFormat::RGBA8:
    *components = 4;
    *depth = ImageBitDepth::I8;
    return;
  case TextureFormat::SRGB8:
    *components = 3;
    *depth = ImageBitDepth::I8;
    return;
  case TextureFormat::SRGB8A8:
    *components = 4;
    *depth = ImageBitDepth::I8;
    return;
  case TextureFormat::R16:
    *components = 1;
    *depth = ImageBitDepth::I16;
    return;
  case TextureFormat::RG16:
    *components = 2;
    *depth = ImageBitDepth::I16;
    return;
  case TextureFormat::RGB16:
    *components = 3;
    *depth = ImageBitDepth::I16;
    return;
  case TextureFormat::RGBA16:
    *components = 4;
    *depth = ImageBitDepth::I16;
    return;
  case TextureFormat::R32f:
    *components = 1;
    *depth = ImageBitDepth::F32;
    return;
  case TextureFormat::RG32f:
    *components = 2;
    *depth = ImageBitDepth::F32;
    return;
  case TextureFormat::RGB32f:
    *components = 3;
    *depth = ImageBitDepth::F32;
    return;
  case TextureFormat::RGBA32f:
    *components = 4;
    *depth = ImageBitDepth::F32;
    return;
  default:
    *components = 0;
    *depth = ImageBitDepth::None;
    return;
  }
}

const Array<String>& GetSupportedImageLoadExtensions()
{
  static Array<String> extensions;
  if (extensions.Empty())
  {
    extensions.PushBack("png");
    extensions.PushBack("bmp");
    extensions.PushBack("psd");
    extensions.PushBack("tga");
    extensions.PushBack("gif");
    extensions.PushBack("hdr");
    extensions.PushBack("pic");
    extensions.PushBack("jpg");
    extensions.PushBack("jpeg");
    extensions.PushBack("jpe");
    extensions.PushBack("jfif");
    extensions.PushBack("pgm");
    extensions.PushBack("ppm");
    extensions.PushBack("pbm");
  }
  return extensions;
}

bool IsSupportedImageLoadExtension(StringParam extension)
{
  const Array<String>& extensions = GetSupportedImageLoadExtensions();
  return extensions.FindIndex(extension) != Array<String>::InvalidIndex;
}

bool ReadImageInfo(Stream* stream, ImageInfo& info)
{
  stream->Seek(0);
  if (ReadPngInfo(stream, info))
    return true;

  stream->Seek(0);
  if (ReadHdrInfo(stream, info))
    return true;

  stbi_io_callbacks callbacks = StbStreamCallbacks();

  ImageBitDepth::Enum depth;
  if (stream->Seek(0) && stbi_is_hdr_from_callbacks(&callbacks, stream))
    depth = ImageBitDepth::F32;
  else if (stream->Seek(0) && stbi_is_16_bit_from_callbacks(&callbacks, stream))
    depth = ImageBitDepth::I16;
  else
    depth = ImageBitDepth::I8;

  stream->Seek(0);

  // We don't actually care about how many components because we always
  // up-convert to 4
  int components = 0;
  bool result = stbi_info_from_callbacks(&callbacks, stream, (int*)&info.Width, (int*)&info.Height, &components) != 0;

  info.Format = ToImageFormat(components, depth);
  return result;
}

bool ReadImageInfo(File& file, ImageInfo& info)
{
  FileStream stream(file);
  return ReadImageInfo(&stream, info);
}

bool ReadImageInfo(StringParam filename, ImageInfo& info)
{
  File file;
  if (!file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential))
    return false;

  return ReadImageInfo(file, info);
}

bool ReadImageInfo(byte* encoded, size_t size, ImageInfo& info)
{
  FixedMemoryStream stream(encoded, size);
  return ReadImageInfo(&stream, info);
}

void LoadImage(Status& status,
               Stream* stream,
               byte** output,
               uint* width,
               uint* height,
               TextureFormat::Enum* format,
               TextureFormat::Enum requireFormat)
{
#ifdef ZeroCustomPngSupport
  if (IsPngLoadFormat(requireFormat) && IsPng(stream))
    return LoadPng(status, stream, output, width, height, format, requireFormat);
#endif

#ifdef ZeroCustomHdrSupport
  if (IsHdrLoadFormat(requireFormat) && IsHdr(stream))
    return LoadHdr(status, stream, output, width, height, format, requireFormat);
#endif

  if (!IsImageLoadFormat(requireFormat))
  {
    status.SetFailed("Image only supports the formats R8, RGB8, RGBA8, R16, "
                     "RGB16, RGBA16, R32f, RGB32f, and RGBA32f");
    return;
  }

  stbi_io_callbacks callbacks = StbStreamCallbacks();

  int requireComponents = 0;
  ImageBitDepth::Enum requireDepth = ImageBitDepth::None;
  FromImageFormat(requireFormat, &requireComponents, &requireDepth);

  // If the user didn't specify a depth, then detect it.
  if (requireDepth == ImageBitDepth::None)
  {
    if (stream->Seek(0) && stbi_is_hdr_from_callbacks(&callbacks, stream))
      requireDepth = ImageBitDepth::F32;
    else if (stream->Seek(0) && stbi_is_16_bit_from_callbacks(&callbacks, stream))
      requireDepth = ImageBitDepth::I16;
    else
      requireDepth = ImageBitDepth::I8;

    stream->Seek(0);
  }

  // We don't know how many components there are until we actually load the
  // image.
  int componentsOut = 0;

  switch (requireDepth)
  {
  case ImageBitDepth::F32:
    *output = (byte*)stbi_loadf_from_callbacks(
        &callbacks, stream, (int*)width, (int*)height, &componentsOut, requireComponents);
    break;
  case ImageBitDepth::I16:
    *output = (byte*)stbi_load_16_from_callbacks(
        &callbacks, stream, (int*)width, (int*)height, &componentsOut, requireComponents);
    break;
  case ImageBitDepth::I8:
    *output = (byte*)stbi_load_from_callbacks(
        &callbacks, stream, (int*)width, (int*)height, &componentsOut, requireComponents);
    break;
  case ImageBitDepth::None:
    break;
  }

  // If we have required components, then the original number of components in
  // the file isn't useful.
  if (requireComponents != 0)
    componentsOut = requireComponents;

  // Check to see if the file might be a gif file (unfortunately STB does not
  // treat gifs the same)
  if (!*output)
  {
    // Check if this is a valid GIF before even attempting to read it,
    // because the STB gif implementation requries it to be all read into
    // memory.
    stream->Seek(0);
    static const String cGifHeader("GIF");
    static const size_t cGifBytes = 3;
    byte gifHeader[cGifBytes];
    if (stream->Peek(gifHeader, cGifBytes) == cGifBytes && memcmp(gifHeader, cGifHeader.Data(), cGifBytes) == 0)
    {
      // Read the entire stream into memory
      ByteBufferBlock block;
      stream->ReadMemoryBlock(status, block, (size_t)stream->Size());

      if (status.Failed())
        return;

      // Note that when a gif loads, it will be much larger in memory because of
      // the extra frames.
      int frames = 0;
      stbi_uc* allFrames = stbi_load_gif_from_memory(block.GetBegin(),
                                                     block.Size(),
                                                     nullptr,
                                                     (int*)width,
                                                     (int*)height,
                                                     &frames,
                                                     &componentsOut,
                                                     requireComponents);

      // If we have required components, then the original number of components
      // in the file isn't useful.
      if (requireComponents != 0)
        componentsOut = requireComponents;

      if (allFrames != nullptr && frames > 0)
      {
        // Copy a single frame to the output (we don't care about multiple
        // frames)
        size_t singleFrameSize = *width * *height * componentsOut;
        *output = (byte*)zAllocate(singleFrameSize);
        memcpy(*output, allFrames, singleFrameSize);
      }

      free(allFrames);
    }
  }

  // Warning: This is not thread safe! At least we're guaranteed this will
  // fail, but may return a random message if this is called by different
  // threads.
  if (!*output)
  {
    status.SetFailed(stbi_failure_reason());
    return;
  }

  *format = ToImageFormat(componentsOut, requireDepth);
}

void LoadImage(Status& status,
               File& file,
               byte** output,
               uint* width,
               uint* height,
               TextureFormat::Enum* format,
               TextureFormat::Enum requireFormat)
{
  FileStream stream(file);
  LoadImage(status, &stream, output, width, height, format, requireFormat);
}

void LoadImage(Status& status,
               StringParam filename,
               byte** output,
               uint* width,
               uint* height,
               TextureFormat::Enum* format,
               TextureFormat::Enum requireFormat)
{
  File file;
  if (!file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential, FileShare::Unspecified, &status))
    return;

  return LoadImage(status, file, output, width, height, format, requireFormat);
}

void LoadImage(Status& status,
               byte* encoded,
               size_t size,
               byte** output,
               uint* width,
               uint* height,
               TextureFormat::Enum* format,
               TextureFormat::Enum requireFormat)
{
  FixedMemoryStream stream(encoded, size);
  LoadImage(status, &stream, output, width, height, format, requireFormat);
}

void LoadImage(Status& status, Stream* stream, Image* imageOut)
{
  // Images only support 8 bit depth, so force it if we can
  byte* output = nullptr;
  uint width = 0;
  uint height = 0;
  TextureFormat::Enum format = TextureFormat::None;
  LoadImage(status, stream, &output, &width, &height, &format, TextureFormat::RGBA8);

  // Note that Image::Set steals the data
  if (output && status.Succeeded())
  {
    ErrorIf(format != TextureFormat::RGBA8, "Got back an invalid format from LoadImage");
    imageOut->Set((ImagePixel*)output, width, height);
  }
}

void LoadImage(Status& status, File& file, Image* imageOut)
{
  FileStream stream(file);
  LoadImage(status, &stream, imageOut);
}

void LoadImage(Status& status, StringParam filename, Image* imageOut)
{
  File file;
  if (!file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential, FileShare::Unspecified, &status))
    return;

  return LoadImage(status, file, imageOut);
}

void LoadImage(Status& status, byte* encoded, size_t size, Image* imageOut)
{
  FixedMemoryStream stream(encoded, size);
  LoadImage(status, &stream, imageOut);
}

void SaveImage(Status& status,
               Stream* stream,
               const byte* image,
               uint width,
               uint height,
               TextureFormat::Enum format,
               ImageSaveFormat::Enum imageType)
{
  if (image == nullptr || width == 0 || height == 0)
  {
    status.SetFailed("Empty Image");
    return;
  }

#ifdef ZeroCustomPngSupport
  if (imageType == ImageSaveFormat::Png && IsPngSaveFormat(format))
    return SavePng(status, stream, image, width, height, format);
#endif

#ifdef ZeroCustomHdrSupport
  if (imageType == ImageSaveFormat::Hdr && IsHdrSaveFormat(format))
    return SaveHdr(status, stream, image, width, height, format);
#endif

  if (!IsImageSaveFormat(format))
  {
    status.SetFailed("Image only supports the formats R8, RG8, RGB8, RGBA8, R16, RG16, "
                     "RGB16, RGBA16, R32f, RG32f, RGB32f, and RGBA32f");
    return;
  }

  int components = 0;
  ImageBitDepth::Enum depth = ImageBitDepth::None;
  FromImageFormat(format, &components, &depth);

  // In the future we should remove the F32 and I8 restrictions below by
  // automatically converting to the expected format.
  if (imageType == ImageSaveFormat::Hdr && depth != ImageBitDepth::F32)
  {
    status.SetFailed("Writing HDR images requires a 32-bit floating point "
                     "format (R32f, RG32f, RGB32f, or RGBA32f)");
    return;
  }

  if (imageType != ImageSaveFormat::Hdr && depth != ImageBitDepth::I8)
  {
    status.SetFailed("All image types except HDR require 8-bit depth");
    return;
  }

  int result = 0;

  switch (imageType)
  {
  case ImageSaveFormat::Png:
    result = stbi_write_png_to_func(&StbWrite, stream, (int)width, (int)height, components, image, 0);
    break;
  case ImageSaveFormat::Bmp:
    result = stbi_write_bmp_to_func(&StbWrite, stream, (int)width, (int)height, components, image);
    break;
  case ImageSaveFormat::Tga:
    result = stbi_write_tga_to_func(&StbWrite, stream, (int)width, (int)height, components, image);
    break;
  case ImageSaveFormat::Hdr:
    result = stbi_write_hdr_to_func(&StbWrite, stream, (int)width, (int)height, components, (float*)image);
    break;
  case ImageSaveFormat::Jpg:
    result = stbi_write_jpg_to_func(&StbWrite, stream, (int)width, (int)height, components, image, 100);
    break;
  }

  if (result == 0)
    status.SetFailed("Failed to write image");
}

void SaveImage(Status& status,
               File& file,
               const byte* image,
               uint width,
               uint height,
               TextureFormat::Enum format,
               ImageSaveFormat::Enum imageType)
{
  FileStream stream(file);
  return SaveImage(status, &stream, image, width, height, format, imageType);
}

void SaveImage(Status& status,
               StringParam filename,
               const byte* image,
               uint width,
               uint height,
               TextureFormat::Enum format,
               ImageSaveFormat::Enum imageType)
{
  File file;
  if (!file.Open(filename.c_str(), FileMode::Write, FileAccessPattern::Sequential, FileShare::Unspecified, &status))
    return;
  return SaveImage(status, file, image, width, height, format, imageType);
}

void SaveImage(Status& status, Stream* stream, Image* image, ImageSaveFormat::Enum imageType)
{
  return SaveImage(status, stream, (byte*)image->Data, image->Width, image->Height, TextureFormat::RGBA8, imageType);
}

void SaveImage(Status& status, File& file, Image* image, ImageSaveFormat::Enum imageType)
{
  FileStream stream(file);
  return SaveImage(status, &stream, image, imageType);
}

void SaveImage(Status& status, StringParam filename, Image* image, ImageSaveFormat::Enum imageType)
{
  File file;
  if (!file.Open(filename.c_str(), FileMode::Write, FileAccessPattern::Sequential, FileShare::Unspecified, &status))
    return;
  return SaveImage(status, file, image, imageType);
}

} // namespace Zero
