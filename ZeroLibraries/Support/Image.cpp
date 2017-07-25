///////////////////////////////////////////////////////////////////////////////
///
/// \file Image.hpp
/// Image class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Image.hpp"
#include "Containers/Algorithm.hpp"
#include "Math/MathStandard.hpp"
#include "Math/ByteColor.hpp"

namespace Zero
{


//------------------------------------------------------------ Image

Image::Image()
{
  Width = 0;
  Height = 0;
  SizeInBytes = 0;
  UserIndex = -1;
  Data = nullptr;
}

Image::~Image()
{
  Deallocate();
}

void Image::Deallocate()
{
  if(Data)
  {
    zDeallocate(Data);
    Data = nullptr;
    Width = 0;
    Height = 0;
    SizeInBytes = 0;
    UserIndex = -1;
  }
}

// Take ownership over a image data
void Image::Set(ImagePixel* imageData, int width, int height)
{
  Deallocate();
  SizeInBytes = sizeof(ImagePixel) * width * height;
  Data = imageData;
  Height = height;
  Width = width;
}

// Allocate a image buffer
void Image::Allocate(int width, int height)
{
  Deallocate();
  SizeInBytes = sizeof(ImagePixel) * width * height;
  Data = (ImagePixel*)zAllocate(SizeInBytes);
  Height = height;
  Width = width;
}

void Image::Swap(Image* other)
{
  Math::Swap(this->Data, other->Data);
  Math::Swap(this->Width, other->Width);
  Math::Swap(this->Height, other->Height);
  Math::Swap(this->SizeInBytes, other->SizeInBytes);
}

void Image::Resize(int width, int height, uint color)
{
  Image newBuffer;
  newBuffer.Allocate(width, height);
  newBuffer.ClearColorTo(color);

  uint copyWidth = Math::Min(width, this->Width);
  uint copyHeight= Math::Min(height, this->Height);

  CopyImage(&newBuffer, this, 0, 0, 0, 0, copyWidth, copyHeight );

  this->Swap(&newBuffer);
}

void Image::ClearColorTo(uint color)
{
 PixelRange range(this);
 for(;!range.Empty();range.PopFront())
 {
   range.Front() = color;
 }
}


#define SetPixelIB(x, y, value) *( (destData) + (x) + (y) * destWidth) = value

void CopyImageExpand(Image* dest, Image* source)
{
  dest->Resize(source->Width, source->Height, 0x00000000);
  CopyImage(dest, source);
}

void CopyImage(Image* dest, Image* source)
{
  CopyImage(dest, source, 0, 0);
}

void CopyImage(Image* dest, Image* source, int destX, int destY)
{
  int sourceWidth = source->Width;
  int sourceHeight = source->Height;
  CopyImage(dest, source, destX, destY, 0, 0, sourceWidth, sourceHeight);
}

void CopyImage(Image* dest, Image* source, int startDestX, int startDestY, int sourceX, int sourceY, int sizeX, int sizeY)
{
  ImagePixel* destData = dest->Data;
  ImagePixel* sourceData = source->Data;
  int destWidth = dest->Width;
  int destHeight = dest->Height;

  int sourceStride = source->Width;

  // Clip width
  int copyWidth = Math::Min(startDestX+sizeX, destWidth);
  // Clip height
  int copyHeight = Math::Min(startDestY+sizeY, destHeight);

  startDestX = Math::Max(startDestX, 0);
  startDestY = Math::Max(startDestY, 0);

  int sx = sourceX;
  int sy = sourceY;
  for(int y = startDestY; y < copyHeight; ++y)
  {
    sx = sourceX;
    for(int x = startDestX; x < copyWidth; ++x)
    {
      ImagePixel pixel = *(sourceData + sx + sy * sourceStride);
      SetPixelIB(x, y, pixel);
      ++sx;
    }
    ++sy;
  }
}

const int alphaTol = 2;

struct FillRange
{
  FillRange() {}
  FillRange(int s, int e, int w, int h) : start(s), end(e), wRef(w), hRef(h) {}
  int start, end, wRef, hRef;
};

void FixAlphaHalo(Image* image)
{
  int width = image->Width;
  int height = image->Height;

  Array<FillRange> wFill;
  Array<FillRange> hFill;

  int hStart = 0;
  int hEnd = 0;

  for (int h = 0; h < height; ++h)
  {
    int wStart = 0;
    int wEnd = 0;

    for (int w = 0; w < width; ++w)
    {
      ImagePixel& pixel = image->GetPixel(w, h);
      uint alpha = pixel >> 24;

      if (alpha < alphaTol)
      {
        wEnd = w + 1;
        continue;
      }

      if (wEnd - wStart > 0)
      {
        if (wStart == 0)
          wFill.PushBack(FillRange(wStart, wEnd, w, h));
        else
        {
          int wMid = (wStart + wEnd) / 2;
          wFill.PushBack(FillRange(wStart, wMid, wStart - 1, h));
          wFill.PushBack(FillRange(wMid, wEnd, w, h));
        }
      }

      wStart = w + 1;
    }

    if (wStart == 0)
    {
      hEnd = h + 1;
      continue;
    }

    if (wEnd - wStart > 0)
      wFill.PushBack(FillRange(wStart, wEnd, wStart - 1, h));

    if (hEnd - hStart > 0)
    {
      if (hStart == 0)
        hFill.PushBack(FillRange(hStart, hEnd, -1, hEnd));
      else
      {
        int hMid = (hStart + hEnd) / 2;
        hFill.PushBack(FillRange(hStart, hMid, -1, hStart - 1));
        hFill.PushBack(FillRange(hMid, hEnd, -1, hEnd));
      }
    }

    hStart = h + 1;
  }

  if (hStart != 0 && hEnd - hStart > 0)
    hFill.PushBack(FillRange(hStart, hEnd, -1, hStart - 1));

  for (unsigned i = 0; i < wFill.Size(); ++i)
  {
    FillRange& range = wFill[i];
    for (int w = range.start; w < range.end; ++w)
      image->SetPixel(w, range.hRef, 0x00FFFFFF & image->GetPixel(range.wRef, range.hRef));
  }

  for (unsigned i = 0; i < hFill.Size(); ++i)
  {
    FillRange& range = hFill[i];
    for (int w = 0; w < width; ++w)
      image->SetPixel(w, range.start, 0x00FFFFFF & image->GetPixel(w, range.hRef));

    range.hRef = range.start++;
    for (int h = range.start; h < range.end; ++h)
      memcpy(&image->GetPixel(0, h), &image->GetPixel(0, range.hRef), width * sizeof(ImagePixel));
  }
}

void SetColorToAlpha(Image* buffer, ByteColor color)
{
  uint transparent = ByteColorRGBA(0,0,0,0);
  PixelRange pixelRange(buffer);
  for(;!pixelRange.Empty();pixelRange.PopFront())
  {
    ByteColor& pixel = pixelRange.Front();
    if( pixel == color )
      pixel = transparent;
  }
}

IntVec2 IntVec2Clamp(IntVec2 value, IntVec2 min, IntVec2 max)
{
  value.x = Math::Clamp(value.x, min.x, max.x);
  value.y = Math::Clamp(value.y, min.y, max.y);
  return value;
}

void FillPixelBorders(Image* image, IntVec2 topLeft, IntVec2 bottomRight, int borderWidth)
{
  IntVec2 borderTL = topLeft - IntVec2(borderWidth, borderWidth);
  IntVec2 borderBR = bottomRight + IntVec2(borderWidth, borderWidth);

  borderTL = IntVec2Clamp(borderTL, IntVec2(0, 0), IntVec2(image->Width - 1, image->Height - 1));
  borderBR = IntVec2Clamp(borderBR, IntVec2(0, 0), IntVec2(image->Width - 1, image->Height - 1));

  for (int y = borderTL.y; y <= borderBR.y; ++y)
  {
    for (int x = borderTL.x; x <= borderBR.x; ++x)
    {
      IntVec2 source = IntVec2Clamp(IntVec2(x, y), topLeft, bottomRight);
      image->SetPixel(x, y, image->GetPixel(source.x, source.y));
    }
  }
}

void AddPixelBorders(Image* input, Image* output, int frameWidth, int frameHeight, int borderWidth)
{
  int width = input->Width;
  int height = input->Height;
  int framesX = width / frameWidth;
  int framesY = height / frameHeight;

  int outputWidth = width + (framesX - 1) * borderWidth * 2;
  int outputHeight = height + (framesY - 1) * borderWidth * 2;

  output->Allocate(outputWidth, outputHeight);
  output->ClearColorTo(0);

  for (int y = 0; y < framesY; ++y)
  {
    int sourceY = y * frameHeight;
    int destY = y * (frameHeight + borderWidth * 2);

    for (int x = 0; x < framesX; ++x)
    {
      int sourceX = x * frameWidth;
      int destX = x * (frameWidth + borderWidth * 2);

      CopyImage(output, input, destX, destY, sourceX, sourceY, frameWidth, frameHeight);

      FillPixelBorders(output, IntVec2(destX, destY), IntVec2(destX + frameWidth - 1, destY + frameHeight - 1), borderWidth);
    }
  }
}

}
