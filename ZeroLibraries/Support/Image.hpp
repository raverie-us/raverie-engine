///////////////////////////////////////////////////////////////////////////////
///
/// \file Image.hpp
/// Image class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Containers/Array.hpp"
#include "Math/IntVector2.hpp"
#include "Support/Rect.hpp"

namespace Zero
{

using Math::IntVec2;

typedef unsigned int ImagePixel;
const uint BytesPerPixel = 4;

// Image object. Contains 2d array
// of 32 bit ABGR pixels.
class Image
{
public:
  int Width;
  int Height;
  int SizeInBytes;
  int UserIndex;
  ImagePixel* Data;

  Image();
  ~Image();

  void Deallocate();

  void Swap(Image* other);

  // Take ownership over a image data
  void Set(ImagePixel* imageData, int width, int height);

  // Allocate a image buffer
  void Allocate(int width, int height);

  //Resize clipping or adding bottom right
  void Resize(int width, int height, uint color);
  void ClearColorTo(uint color);

  ImagePixel& GetPixel(int x, int y)
  {
    return Data[Width * y + x];
  }

  void SetPixel(int x, int y, ImagePixel data)
  {
    Data[Width * y + x] = data;
  }
};

// Pixel range is used to edit a 2d rect
// of pixels.
class PixelRange
{
public:
  PixelRange(Image* buffer)
  {
    X = startX = 0;
    Y = startY = 0;
    endX = buffer->Width;
    endY = buffer->Height;
    pitch = buffer->Width;
    DataBuffer = buffer->Data;
  }

  PixelRange(Image* buffer, PixelRect area)
  {
    X = startX = area.X;
    Y = startY = area.Y;
    endX = area.X + area.SizeX;
    endY = area.Y + area.SizeY;
    pitch = area.SizeX;
    DataBuffer = buffer->Data;
  }

  int X;
  int Y;
  int startX;
  int startY;
  int endX;
  int endY;
  ImagePixel* DataBuffer;
  int pitch;

  void PopFront()
  {
    ++X;
    if(X >= endX)
    {
      ++Y;
      X = startX;
    }
  }

  bool Empty()
  {
    return Y >= endY;
  }

  ImagePixel& Front()
  {
    return DataBuffer[X + Y * pitch];
  }
};

void CopyImageExpand(Image* dest, Image* source);
void CopyImage(Image* dest, Image* source);
void CopyImage(Image* dest, Image* source, int startDestX, int startDestY);
void CopyImage(Image* dest, Image* source, int startDestX, int startDestY,
                                   int sourceX, int sourceY, int sizeX, int sizeY);

void FillPixelBorders(Image* image, IntVec2 topLeft, IntVec2 bottomRight, int borderWidth);
void AddPixelBorders(Image* input, Image* output, int frameWidth, int frameHeight, int borderWidth);

// Removed Alpha artifact created by edge transparent pixels when mip mapping.
void FixAlphaHalo(Image* image);

// Set all pixels that match color to transparent
void SetColorToAlpha(Image* buffer, uint color);

}
