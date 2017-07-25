///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Image.hpp"
#include "Utility/Status.hpp"
#include "PngSupport.hpp"
#include "Platform/File.hpp"

#include "png.h"
#ifdef ZeroDebug
#pragma comment(lib, "libpngd.lib")
#else
#pragma comment(lib, "libpng.lib")
#endif

namespace Zero
{

const uint PngHeaderSize = 8;

// Read data from any source
template<typename readerType>
void CustomReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
  Status status;
  png_voidp ioPtr = png_get_io_ptr(pngPtr);
  readerType* reader = (readerType*)ioPtr;
  reader->Read(status, data, length);
}

// Write data from any source
template<typename readerType>
void CustomWriteData(png_structp pngPtr, png_bytep data, png_size_t length)
{
  png_voidp ioPtr = png_get_io_ptr(pngPtr);
  readerType* reader = (readerType*)ioPtr;
  reader->Write(data, length);
}

void CustomFlush(png_structp pngPtr)
{
  // Do nothing
}

bool ReadPngInfo(StringParam filename, PngInfo& info)
{
  Status status;
  File file;
  file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential);

  if (!file.IsOpen())
    return false;

  // Read the png header
  png_byte pngHeader[PngHeaderSize];
  file.Read(status, pngHeader, PngHeaderSize);

  // Is this a valid png file?
  int isPng = png_check_sig(pngHeader, PngHeaderSize);
  if (!isPng)
    return false;

  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop infoPtr = png_create_info_struct(pngPtr);

  if (setjmp(png_jmpbuf(pngPtr)))
  {
    png_destroy_read_struct(&pngPtr, &infoPtr, png_infopp_NULL);
    return false;
  }

  // Set up custom read to read from the file object
  png_set_read_fn(pngPtr, (png_voidp)&file, CustomReadData<File>);

  // Move paste the png header that was read above
  png_set_sig_bytes(pngPtr, PngHeaderSize);

  // Read the png info data
  png_read_info(pngPtr, infoPtr);

  // Load the data into the structure
  info.Width =  png_get_image_width(pngPtr, infoPtr);
  info.Height = png_get_image_height(pngPtr, infoPtr);
  info.BitDepth = png_get_bit_depth(pngPtr, infoPtr);
  png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

  png_destroy_read_struct(&pngPtr, &infoPtr, png_infopp_NULL);

  return true;
}

void LoadFromPng(Status& status, byte** output, uint* width, uint* height, uint* bitDepth, const byte* data, uint size, bool stripBitDepth)
{
  // Init pointer value so memory can be cleaned up if an error occurs after allocation
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
    png_destroy_read_struct(&pngPtr, png_infopp_NULL, png_infopp_NULL);
    status.SetFailed("Can not read png file");
    return;
  }

  // Set error handling if you are using the setjmp/longjmp method (this is
  // the normal method of doing things with libpng).  REQUIRED unless you
  // set up your own error handlers in the png_create_read_struct() earlier.
  if (setjmp(png_jmpbuf(pngPtr)))
  {
    // Clean up memory
    zDeallocate(imageData);
    // Free all of the memory associated with the pngPtr and infoPtr
    png_destroy_read_struct(&pngPtr, &infoPtr, png_infopp_NULL);
    // If we get here, we had a problem reading the file
    status.SetFailed("Can not read png file");
    return;
  }

  // If you are using replacement read functions, instead of calling png_init_io()
  ByteBufferBlock bufferBlock(const_cast<byte*>(data), size, false);
  png_set_read_fn(pngPtr, (png_voidp)&bufferBlock, CustomReadData<ByteBufferBlock>);

  // The call to png_read_info() gives us all of the information from the
  // PNG file before the first IDAT (image data chunk).  REQUIRED
  png_read_info(pngPtr, infoPtr);

  png_uint_32 readWidth, readHeight;
  int readDepth, colorType, interlaceType;
  png_get_IHDR(pngPtr, infoPtr, &readWidth, &readHeight, &readDepth, &colorType, &interlaceType, nullptr, nullptr);

  // Strip 16 bits/color files down to 8 bits/color.
  if (stripBitDepth)
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
    png_set_gray_1_2_4_to_8(pngPtr);

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

  // Set up row pointers to point directly into allocated image
  png_bytep* rowPointers = (png_bytep*)alloca(readHeight * sizeof(byte*));
  for (uint i = 0; i < readHeight; ++i)
    rowPointers[i] = (png_bytep)(imageData + rowSize * i);

  // Now read the image.
  png_read_image(pngPtr, rowPointers);

  // Read rest of file, and get additional chunks in infoPtr - REQUIRED
  png_read_end(pngPtr, infoPtr);

  // Final clean up
  png_destroy_read_struct(&pngPtr, &infoPtr, png_infopp_NULL);

  *output = imageData;
  *width = readWidth;
  *height = readHeight;
  *bitDepth = readDepth;
}

void LoadFromPng(Status& status, Image* image, const byte* data, uint size)
{
  byte* output = nullptr;
  uint width, height, bitDepth;
  LoadFromPng(status, &output, &width, &height, &bitDepth, data, size, true);
  if (output && status.Succeeded())
    image->Set((ImagePixel*)output, width, height);
}

void LoadFromPng(Status& status, Image* buffer, StringParam filename)
{
  File file;
  file.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential);

  if (!file.IsOpen())
  {
    status.SetFailed("Can not open png file");
    return;
  }

  uint fileSize = file.Size();
  byte* fileData = new byte[fileSize];
  uint bytesRead = file.Read(status, fileData, fileSize);

  if (status.Failed())
  {
    delete[] fileData;
    status.SetFailed("Can not read png file");
    return;
  }

  LoadFromPng(status, buffer, fileData, fileSize);

  delete[] fileData;
}

void SaveToPng(Status& status, Image* image, StringParam filename)
{
  SaveToPng(status, (byte*)image->Data, image->Width, image->Height, 8, filename);
}

void SaveToPng(Status& status, byte* image, uint width, uint height, uint bitDepth, StringParam filename)
{
  if (image == nullptr || width == 0 || height == 0)
  {
    status.SetFailed("Empty Image");
    return;
  }

  if (bitDepth != 8 && bitDepth != 16)
  {
    status.SetFailed("Invalid bit depth");
    return;
  }

  File file;
  file.Open(filename.c_str(), FileMode::Write, FileAccessPattern::Sequential, FileShare::Write);

  if (!file.IsOpen())
  {
    status.SetFailed(String::Format("Can not open png file '%s' for writing", filename.c_str()));
    return;
  }

  // Initialize the write struct
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  // Initialize the info struct.
  png_infop pngInfo = png_create_info_struct(png_ptr);

  // Set up error handling.
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_write_struct(&png_ptr, &pngInfo);
    status.SetFailed("Internal Png Error");
    return;
  }

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

  png_set_write_fn(png_ptr, (png_voidp)&file, CustomWriteData<File>, CustomFlush);

  // Initialize rows of PNG.
  uint pixelSize = bitDepth / 2;
  uint bytesPerRow = width * pixelSize;

  // Allocated Rows
  png_byte** rows = (png_byte**)alloca(height * sizeof(byte*));
  for (uint y = 0; y < height; ++y)
    rows[y] = (png_byte *)(byte*)(image + bytesPerRow * y);

  // Write the image data to the file
  png_set_rows(png_ptr, pngInfo, rows);
  png_write_png(png_ptr, pngInfo, PNG_TRANSFORM_IDENTITY, nullptr);

  // Finish writing.
  png_destroy_write_struct(&png_ptr, &pngInfo);
}

}
