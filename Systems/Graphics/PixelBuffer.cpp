///////////////////////////////////////////////////////////////////////////////
///
/// \file PixelBuffer.cpp
///  Implementation of the PixelBuffer class.
///
/// Authors: Benjamin Strukus, Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
PixelBuffer::PixelBuffer() :
  Width(0),
  Height(0),
  Total(0),
  Data(nullptr),
  Image(Texture::CreateRuntime()),
  MaxMipLevel(-1)
{
}

PixelBuffer::PixelBuffer(ByteColor clearColor, uint width, uint height) :
  Width(width),
  Height(height),
  Total(width * height),
  Data(new ByteColor[Total]),
  Image(Texture::CreateRuntime()),
  MaxMipLevel(-1)
{
  Clear(clearColor);

  Image->Upload(width, height, TextureFormat::RGBA8, (byte*)Data, Total * sizeof(ByteColor));
}

PixelBuffer::~PixelBuffer()
{
  delete [] Data;
  Data = nullptr;
}

void PixelBuffer::Resize(uint width, uint height, bool copyOldContents, bool clearNewContents, ByteColor clearColor)
{
  Total = width * height;
  ByteColor* oldData = Data;
  ByteColor* newData = new ByteColor[Total];

  int oldX = Width;
  int oldY = Height;

  int newX = width;
  int newY = height;

  Width = width;
  Height = height;
  Data = newData;

  int sameX = Math::Min(oldX, newX);
  int sameY = Math::Min(oldY, newY);

  // The old texture
  //  ___________
  // |OOOOOOOOOOO|
  // |OOOOOOOOOOO|

  // The new texture
  //  _______
  // |NNNNNNN|
  // |NNNNNNN|
  // |NNNNNNN|
  // |NNNNNNN|

  // Where they mean in the middle (S for same)
  //  ___________
  // |SSSSSSS|OOO|
  // |SSSSSSS|OOO|
  // |NNNNNNN|
  // |NNNNNNN|

  if (copyOldContents)
  {
    for (int y = 0; y < sameY; ++y)
    {
      for (int x = 0; x < sameX; ++x)
      {
        newData[x + y * newX] = oldData[x + y * oldX];
      }
    }
  }

  if (clearNewContents)
  {
    // Everything below (only to the 'same' right)
    for (int y = sameY; y < newY; ++y)
    {
      for (int x = 0; x < sameX; ++x)
      {
        newData[x + y * newX] = clearColor;
      }
    }

    // Everything to the right (top to the entire bottom)
    for (int y = 0; y < newY; ++y)
    {
      for (int x = sameX; x < newX; ++x)
      {
        newData[x + y * newX] = clearColor;
      }
    }
  }

  delete[] oldData;

  uint dataSize = width * height * sizeof(ByteColor);
  Image->Upload(width, height, TextureFormat::RGBA8, (byte*)newData, dataSize);
}

// Clear the pixel buffer
void PixelBuffer::Clear(ByteColor color)
{
  for(uint i = 0; i < Total; ++i)
  {
    Data[i] = color;
  }
}

// Clear the pixel buffer with a gray scale color (much faster)
void PixelBuffer::Clear(byte grayScaleColor)
{
  memset(Data, grayScaleColor, Total * sizeof(ByteColor));
}

// Set a pixel in the buffer.
void PixelBuffer::SetPixel(uint x, uint y, ByteColor color)
{
  uint index = Width * y + x;
  ReturnIf(index >= Total, , "Pixel coordinate out of bounds.");
  Data[index] = color;
}

/// Adds the given color to the current color taking into account the
/// transparency of the given color.
void PixelBuffer::AddToPixel(uint x, uint y, ByteColor color)
{
  // Get the index of the color
  uint index = Width * y + x;
  ReturnIf(index >= Total, , "Pixel coordinate out of bounds.");

  // Get the current color
  Vec4 currColor = ToFloatColor(Data[index]);
  Vec4 additiveColor = ToFloatColor(color);
  float alpha = additiveColor.w;

  Vec4 finalColor = (currColor * (1.0f - alpha)) + (additiveColor * alpha);
  Data[index] = ToByteColor(finalColor);
}

/// Returns a pixel in the buffer.
ByteColor PixelBuffer::GetPixel(uint x, uint y)
{
  uint index = Width * y + x;
  ReturnIf(index >= Total, ByteColor(), "Pixel coordinate out of bounds.");
  return Data[index];
}

/// Returns whether or not the given coordinate is valid.
bool PixelBuffer::IsValid(uint x, uint y)
{
  return x < Width && y < Height;
}

// Get the coordinates of an index (including sample scale)
void PixelBuffer::GetCoordinates(uint index, uint* x, uint* y)
{
  index %= Total;

  *x = (index % Width);
  *y = (index / Width);
}

// Upload the image data
void PixelBuffer::Upload()
{
  Image->Upload(Image->mWidth, Image->mHeight, TextureFormat::RGBA8, (byte*)Data, Total * sizeof(ByteColor));
}

void PixelBuffer::SetAll(byte* data)
{
  memcpy(Data, data, Total * sizeof(ByteColor));
  Upload();
}

}//namespace Zero
