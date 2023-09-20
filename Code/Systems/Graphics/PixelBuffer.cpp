// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
PixelBuffer::PixelBuffer() : Width(0), Height(0), Total(0), Data(nullptr), MaxMipLevel(-1), Image(Texture::CreateRuntime())
{
}

PixelBuffer::PixelBuffer(MoveReference<PixelBuffer> rhs) : Width(rhs->Width), Height(rhs->Height), Total(rhs->Total), Data(rhs->Data), MaxMipLevel(rhs->MaxMipLevel), Image(rhs->Image)
{
  rhs->Width = 0;
  rhs->Height = 0;
  rhs->Total = 0;
  rhs->Data = nullptr;
  rhs->Image = nullptr;
  rhs->MaxMipLevel = -1;
}

PixelBuffer::PixelBuffer(ByteColor clearColor, uint width, uint height) :
    Width(width), Height(height), Total(width * height), Data(new ByteColor[Total]), MaxMipLevel(-1), Image(Texture::CreateRuntime())
{
  Clear(clearColor);

  Image->Upload(width, height, TextureFormat::RGBA8, (byte*)Data, Total * sizeof(ByteColor));
}

PixelBuffer::~PixelBuffer()
{
  delete[] Data;
  Data = nullptr;
}

void PixelBuffer::Resize(uint width, uint height, bool copyOldContents, bool clearNewContents, ByteColor clearColor, bool Upload)
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

  if (Upload)
  {
    uint dataSize = width * height * sizeof(ByteColor);
    Image->Upload(width, height, TextureFormat::RGBA8, (byte*)newData, dataSize);
  }
}

IntVec2 PixelBuffer::GetSize()
{
  return Image->GetSize();
}

// Clear the pixel buffer
void PixelBuffer::Clear(ByteColor color)
{
  for (uint i = 0; i < Total; ++i)
  {
    Data[i] = color;
  }
}

// Clear the pixel buffer with a gray scale color (much faster)
void PixelBuffer::Clear(byte grayScaleColor)
{
  memset(Data, grayScaleColor, Total * sizeof(ByteColor));
}

// Set a color on an area of pixels in the buffer.
void PixelBuffer::FillRect(Vec2Param topLeft, Vec2Param bottomRight, ByteColor color)
{
  uint index = Width * (uint)topLeft.y + (uint)topLeft.x;
  ReturnIf(index >= Total, , "TopLeft coordinate out of PixelBuffer bounds.");

  index = Width * (uint)bottomRight.y + (uint)bottomRight.x;
  ReturnIf(index >= Total, , "BottomRight coordinate out of PixelBuffer bounds.");

  for (int y = (int)topLeft.y; y <= bottomRight.y; ++y)
  {
    for (int x = (int)topLeft.x; x <= bottomRight.x; ++x)
    {
      index = Width * y + x;
      Data[index] = color;
    }
  }
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
  Image->Upload(Width, Height, TextureFormat::RGBA8, (byte*)Data, Total * sizeof(ByteColor));
}

void PixelBuffer::SetAll(byte* data)
{
  memcpy(Data, data, Total * sizeof(ByteColor));
  Upload();
}

} // namespace Raverie
