///////////////////////////////////////////////////////////////////////////////
///
/// \file PixelBuffer.hpp
///  Declaration of the PixelBuffer class.
///
/// Authors: Benjamin Strukus, Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Pixel buffer is a simple interface for dynamically drawing to textures.
class PixelBuffer
{
public:

  /// Constructor.
  PixelBuffer();
  PixelBuffer(ByteColor clearColor, uint width, uint height);

  /// Destructor.
  ~PixelBuffer();

  /// Resizes the pixel buffer (specifying whether we want to copy the old data)
  void Resize(uint width, uint height, bool copyOldContents = true, bool clearNewContents = true, ByteColor clearColor = Color::White);

  /// Clear the pixel buffer.
  void Clear(ByteColor color);

  /// Clear the pixel buffer with a gray scale color (much faster).
  void Clear(byte grayScaleColor);

  /// Set a pixel in the buffer.
  void SetPixel(uint x, uint y, ByteColor color);

  /// Adds the given color to the current color taking into account the
  /// transparency of the given color.
  void AddToPixel(uint x, uint y, ByteColor color);
  
  /// Returns a pixel in the buffer.
  ByteColor GetPixel(uint x, uint y);

  /// Returns whether or not the given coordinate is valid.
  bool IsValid(uint x, uint y);

  /// Get the coordinates of an index.
  void GetCoordinates(uint index, uint* x, uint* y);

  /// Upload the image data.
  void Upload();

  void SetAll(byte* data);
public:

  /// Store the width and height of the texture.
  uint Width;
  uint Height;

  /// Store the total dimensions.
  uint Total;

  /// The byte data that we store for the pixel buffer.
  ByteColor* Data;

  int MaxMipLevel;

  /// The final texture that we spit out.
  HandleOf<Texture> Image;
};

}//namespace Zero
