///////////////////////////////////////////////////////////////////////////////
///
/// \file LoadHdr.hpp
/// Load Hdr image format
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Rgbe Format is R, G, and B plus Exponent in bytes (R G B) * 2 ^ E
struct Rgbe
{
  byte r;
  byte g;
  byte b;
  byte e;
  inline byte& operator[](uint i){return ((byte*)this)[i];}
};

// Rgb32 is a Hdr 32 bit float per channel format
struct Rgb32
{
  float r;
  float g;
  float b;
};

void ConvertRgbeToRgb32(Rgbe* source, Rgb32* dest);

struct HdrImage
{
  uint Width;
  uint Height;
  Rgb32* Data;
};

// Load a file in the Hdr format from memory
void LoadHdrFromMemory(DataBlock block, HdrImage* image);

// Load a file in the Hdr format from a file.
void LoadHdrFile(StringParam name, HdrImage* image);

}
