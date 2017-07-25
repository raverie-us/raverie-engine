///////////////////////////////////////////////////////////////////////////////
///
/// \file LoadHdr.cpp
///  Load Hdr image format
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "LoadHdr.hpp"
#include "Math/Reals.hpp"
#include "Platform/File.hpp"
#include <stdio.h>
#include "FileSupport.hpp"

namespace Zero
{

void ConvertRgbeToRgb32(Rgbe* source, Rgb32* dest)
{
  // Rebase exponent stored as (0 - 255) - 128;
  int exponent = source->e - 128;

  // Compute pixel power scalar
  float scalar = Math::Pow(2.0f, float(exponent));

  // Each byte is a mantissa so divide by 255 to map 0 to 1
  scalar *= (1.0f/255.0f);

  // Write full range values
  dest->r = float(source->r) * scalar;
  dest->g = float(source->g) * scalar;
  dest->b = float(source->b) * scalar;
}

void ConvertRowToRgb32(Rgbe* sourceRow, uint width, Rgb32* destRow)
{
  for(uint i=0;i<width;++i)
  {
    ConvertRgbeToRgb32(sourceRow, destRow);
    destRow++;
    sourceRow++;
  }
}

template<typename type>
type read(byte*& current)
{
  type& c = *(type*)current;
  current+=sizeof(type);
  return c;
}

void OldDecodeFormat(Rgbe* output, uint width, byte*& current)
{
  for(uint pixel=0;pixel<width;++pixel)
  {
    Rgbe start = read<Rgbe>(current);
    *output = start;
    ++output;
  }
}

inline u16 SwapBytes(u16 x)
{
  u16 hibyte = (x & 0xff00) >> 8;
  u16 lobyte = (x & 0x00ff) << 8;
  return lobyte | hibyte;
}

void DecodeRow(Rgbe* scanline, uint width, byte*& current)
{
  Rgbe start = read<Rgbe>(current);

  // The new decode format (described by Glassner in Graphics Gems II) starts with a pixel
  // with both r and g set to 2 and then a two byte scanline length 
  bool newDecodeFormat = start[0] == 2 && start[1] == 2;

  if(!newDecodeFormat)
  {
    // Unread bytes and decode the scanline using the old decode format
    current-=sizeof(Rgbe);
    return OldDecodeFormat(scanline, width, current);
  }

  uint length = SwapBytes( *(u16*)&start.b );
  ErrorIf(length!=width, "Bad format.");

  // In this decode format each pixel component (R,G,B,E) is stored separately
  // For each channel
  const int channelCount = 4;
  for(uint c = 0; c < channelCount; c++)
  {
    // For each pixel on the width of scanline
    uint p = 0;
    while(p < width)
    {
      // Read the control code

      // If the code is greater than 128 (high bit set)
      // this indicates a run of the same value
      byte count = read<byte>(current);
      if (count > 128)
      {
        // runLength is lower bits of code
        uint runLength = count & 127;
        byte val = read<byte>(current);
        for(uint r=0;r<runLength;++r)
          scanline[p++][c] = val;
      }
      else
      {
        // count number of separate values
        for(uint r=0;r<count;++r)
          scanline[p++][c] = read<byte>(current);
      }
    }
  }
}


// Read line in the header format and null terminate
// it in place so it can be used as a string.
byte* ReadHdrHeaderLine(byte*& d)
{
  byte* start = d;
  byte* c = d;
  while(*c!='\n')
    ++c;
  *c = '\0';
  ++c;
  d = c;
  return start;
}

void LoadHdrFile(StringParam name, HdrImage* image)
{
  DataBlock block = ReadFileIntoDataBlock(name.c_str());

  LoadHdrFromMemory(block, image);

  FreeBlock(block);
}

void LoadHdrFromMemory(DataBlock block, HdrImage* image)
{
  byte* data = block.Data;
  uint fileSize = block.Size;

  //Read all the header data
  cstr fileHeader = (cstr)ReadHdrHeaderLine(data);
  cstr madeBy = (cstr)ReadHdrHeaderLine(data);
  cstr format = (cstr)ReadHdrHeaderLine(data);
  cstr empty = (cstr)ReadHdrHeaderLine(data);
  cstr res = (cstr)ReadHdrHeaderLine(data);

  uint width = 0;
  uint height = 0;

  //Read the width and height
  if (!sscanf(res, "-Y %u +X %u", &height, &width))
    return;

  // Allocate final image data
  Rgb32* pixels = (Rgb32*)zAllocate(width * height * sizeof(Rgb32));

  // Allocate a scratch Rgbe scan line 
  Rgbe* scanLine = (Rgbe*)zAllocate(width * sizeof(Rgbe));

  // Decode a scan line and convert it to Rgb32
  // this prevents the need for another copy of the image in memory
  byte* current = data;
  Rgb32* destPixels = pixels;
  for(uint y=0;y<height;++y)
  {
    DecodeRow(scanLine, width, current);
    ConvertRowToRgb32(scanLine, width, destPixels);
    destPixels += width;
  }

  // Write out final data
  image->Data = pixels;
  image->Width = width;
  image->Height = height;
}


}
