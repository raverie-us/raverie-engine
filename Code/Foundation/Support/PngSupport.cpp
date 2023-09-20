// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "png.h"

namespace Raverie
{

const uint PngSignatureSize = 8;

// Read data from any Stream
static void StreamReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
  png_voidp ioPtr = png_get_io_ptr(pngPtr);
  Stream* stream = (Stream*)ioPtr;
  stream->Read(data, length);
}

// Write data to any Stream
static void StreamWriteData(png_structp pngPtr, png_bytep data, png_size_t length)
{
  png_voidp ioPtr = png_get_io_ptr(pngPtr);
  Stream* stream = (Stream*)ioPtr;
  stream->Write(data, length);
}

static void CustomFlush(png_structp pngPtr)
{
  png_voidp ioPtr = png_get_io_ptr(pngPtr);
  Stream* stream = (Stream*)ioPtr;
  stream->Flush();
}

bool IsPng(Stream* stream)
{
  // Read the png header
  png_byte pngSignature[PngSignatureSize];
  stream->Read(pngSignature, PngSignatureSize);
  stream->Seek(0);
  // Is this a valid png file?
  return ((png_check_sig(pngSignature, PngSignatureSize)) != 0);
}

bool ReadPngInfo(Stream* stream, ImageInfo& info)
{
  // Read the png signature (this moves the stream forward!)
  if (!IsPng(stream))
    return false;

  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop infoPtr = png_create_info_struct(pngPtr);

  //TODO(trevor): Handle setjmp
  //if (setjmp(png_jmpbuf(pngPtr)))
  //{
  //  png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
  //  stream->Seek(0);
  //  return false;
  //}

  // Set up custom read to read from the file object
  png_set_read_fn(pngPtr, (png_voidp)stream, StreamReadData);

  // Move paste the png header that was read above
  png_set_sig_bytes(pngPtr, PngSignatureSize);

  // Read the png info data
  png_read_info(pngPtr, infoPtr);

  // Load the data into the structure
  info.Width = png_get_image_width(pngPtr, infoPtr);
  info.Height = png_get_image_height(pngPtr, infoPtr);
  int bitDepth = png_get_bit_depth(pngPtr, infoPtr);

  if (bitDepth == 16)
    info.Format = TextureFormat::RGBA16;
  else
    info.Format = TextureFormat::RGBA8;

  png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

  png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
  stream->Seek(0);
  return true;
}

bool IsPngLoadFormat(TextureFormat::Enum format)
{
  return format == TextureFormat::None || IsPngSaveFormat(format);
}

bool IsPngSaveFormat(TextureFormat::Enum format)
{
  return format == TextureFormat::RGBA8 || format == TextureFormat::RGBA16;
}

void LoadPng(Status& status,
             Stream* stream,
             byte** output,
             uint* width,
             uint* height,
             TextureFormat::Enum* format,
             TextureFormat::Enum requireFormat)
{
  if (!IsPngLoadFormat(requireFormat))
  {
    status.SetFailed("Png only supports the formats RGBA8 and RGBA16");
    return;
  }

  // Init pointer value so memory can be cleaned up if an error occurs after
  // allocation
  byte* imageData = nullptr;

  // Create and initialize the png_struct with the desired error handler
  // functions.  If you want to use the default stderr and longjump method,
  // you can supply null for the last three parameters.  We also supply the
  // the compiler header file version, so that we know if the application
  // was compiled with a compatible version of the library.  REQUIRED
  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (pngPtr == nullptr)
  {
    status.SetFailed("Can not read png file");
    return;
  }

  // Allocate/initialize the memory for image information.  REQUIRED.
  png_infop infoPtr = png_create_info_struct(pngPtr);
  if (infoPtr == nullptr)
  {
    png_destroy_read_struct(&pngPtr, nullptr, nullptr);
    status.SetFailed("Can not read png file");
    return;
  }

  //TODO(trevor): Handle setjmp
  // Set error handling if you are using the setjmp/longjmp method (this is
  // the normal method of doing things with libpng).  REQUIRED unless you
  // set up your own error handlers in the png_create_read_struct() earlier.
  //if (setjmp(png_jmpbuf(pngPtr)))
  //{
  //  // Clean up memory
  //  zDeallocate(imageData);
  //  // Free all of the memory associated with the pngPtr and infoPtr
  //  png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
  //  // If we get here, we had a problem reading the file
  //  status.SetFailed("Can not read png file");
  //  return;
  //}

  // If you are using replacement read functions, instead of calling
  // png_init_io()
  png_set_read_fn(pngPtr, (png_voidp)stream, StreamReadData);

  // The call to png_read_info() gives us all of the information from the
  // PNG file before the first IDAT (image data chunk).  REQUIRED
  // This reads the header, and if it fails it will jump back to the setjmp
  // point.
  png_read_info(pngPtr, infoPtr);

  png_uint_32 readWidth, readHeight;
  int readDepth, colorType, interlaceType;
  png_get_IHDR(pngPtr, infoPtr, &readWidth, &readHeight, &readDepth, &colorType, &interlaceType, nullptr, nullptr);

  // Strip 16 bits/color files down to 8 bits/color.
  if (requireFormat == TextureFormat::RGBA8)
  {
    png_set_strip_16(pngPtr);
    readDepth = 8;
  }

  // Swap endianness to little endian
  if (readDepth == 16)
    png_set_swap(pngPtr);

  // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
  // byte into separate bytes (for paletted and grayscale images).
  png_set_packing(pngPtr);

  // Expand paletted colors into true RGB triplets
  if (colorType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(pngPtr);

  // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
  if (colorType == PNG_COLOR_TYPE_GRAY && readDepth < 8)
    png_set_expand_gray_1_2_4_to_8(pngPtr);

  // Expand paletted or RGB images with transparency to full alpha channels
  // so the data will be available as RGBA quartets.
  if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pngPtr);

  // Expand grayscale images to RGB/RGBA
  if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(pngPtr);

  // Add filler (or alpha) byte (before/after each RGB triplet)
  png_set_filler(pngPtr, 0xff, PNG_FILLER_AFTER);

  uint pixelSize = readDepth / 2;
  uint imageSize = readWidth * readHeight * pixelSize;
  uint rowSize = readWidth * pixelSize;

  // Allocate space for image
  imageData = (byte*)zAllocate(imageSize);

  if (!imageData)
  {
    status.SetFailed("Failed to allocate memory for the Png output image");
    return;
  }

  // Set up row pointers to point directly into allocated image
  png_bytep* rowPointers = (png_bytep*)alloca(readHeight * sizeof(byte*));
  for (uint i = 0; i < readHeight; ++i)
    rowPointers[i] = (png_bytep)(imageData + rowSize * i);

  // Now read the image.
  png_read_image(pngPtr, rowPointers);

  // Read rest of file, and get additional chunks in infoPtr - REQUIRED
  png_read_end(pngPtr, infoPtr);

  // Final clean up
  png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);

  *output = imageData;
  *width = readWidth;
  *height = readHeight;

  if (readDepth == 16)
    *format = TextureFormat::RGBA16;
  else
    *format = TextureFormat::RGBA8;
}

void SavePng(Status& status, Stream* stream, const byte* image, uint width, uint height, TextureFormat::Enum format)
{
  if (!IsPngSaveFormat(format))
  {
    status.SetFailed("Png only supports the formats RGBA8 and RGBA16");
    return;
  }

  if (image == nullptr || width == 0 || height == 0)
  {
    status.SetFailed("Empty Image");
    return;
  }

  // Initialize the write struct
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // Initialize the info struct.
  png_infop pngInfo = png_create_info_struct(png_ptr);

  // TODO(trevor): Handle setjmp removal
  // Set up error handling.
  //if (setjmp(png_jmpbuf(png_ptr)))
  //{
  //  png_destroy_write_struct(&png_ptr, &pngInfo);
  //  status.SetFailed("Internal Png Error");
  //  return;
  //}

  int bitDepth = 8;
  if (format == TextureFormat::RGBA16)
    bitDepth = 16;

  // Set image attributes
  png_set_IHDR(png_ptr,
               pngInfo,
               width,
               height,
               bitDepth,
               PNG_COLOR_TYPE_RGB_ALPHA,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_set_write_fn(png_ptr, (png_voidp)stream, StreamWriteData, CustomFlush);

  // Initialize rows of PNG.
  uint pixelSize = bitDepth / 2;
  uint bytesPerRow = width * pixelSize;

  // Allocated Rows
  png_byte** rows = (png_byte**)alloca(height * sizeof(byte*));
  for (uint y = 0; y < height; ++y)
    rows[y] = (png_byte*)(byte*)(image + bytesPerRow * y);

  // Write the image data to the file
  png_set_rows(png_ptr, pngInfo, rows);
  png_write_png(png_ptr, pngInfo, PNG_TRANSFORM_IDENTITY, nullptr);

  // Finish writing.
  png_destroy_write_struct(&png_ptr, &pngInfo);
}

} // namespace Raverie
