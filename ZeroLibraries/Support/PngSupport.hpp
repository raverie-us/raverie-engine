///////////////////////////////////////////////////////////////////////////////
///
/// \file PngSuppport.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Utility/Status.hpp"

namespace Zero
{
class Image;

struct PngInfo
{
  uint Width;
  uint Height;
  uint BitDepth;
};

bool ReadPngInfo(StringParam filename, PngInfo& info);

void LoadFromPng(Status& status, byte** output, uint* width, uint* height, uint* bitDepth, const byte* data, uint size, bool stripBitDepth = false);
void LoadFromPng(Status& status, Image* image, const byte* data, uint size);
void LoadFromPng(Status& status, Image* image, StringParam filename);

void SaveToPng(Status& status, Image* image, StringParam filename);
void SaveToPng(Status& status, byte* image, uint width, uint height, uint bitDepth, StringParam filename);

}
