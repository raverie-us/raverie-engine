// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

#include "Systems/Engine/ThreadDispatch.hpp"
#include "Systems/Engine/JobSystem.hpp"

namespace Raverie
{

// Aspect ratios of valid cubemap layouts: 1/6, 6/1, 2/3, 3/2, 3/4, 4/3, 4/2
// Cubemap faces are processed from the image as shown in the diagrams below
// Face locations are world space axis and images are expected to be viewed from
// the origin
// +/-Y faces are expected to be oriented on the top/bottom of the +Z face for
// all layouts

// 1/6
//  ___
// |+X |
// |___|
// |-X |
// |___|
// |+Y |
// |___|
// |-Y |
// |___|
// |+Z |
// |___|
// |-Z |
// |___|

// 6/1
//  _______________________
// |+X |-X |+Y |-Y |+Z |-Z |
// |___|___|___|___|___|___|

// 2/3
//  _______
// |+X |-X |
// |___|___|
// |+Y |-Y |
// |___|___|
// |+Z |-Z |
// |___|___|

// 3/2
//  ___________
// |+X |+Y |+Z |
// |___|___|___|
// |-X |-Y |-Z |
// |___|___|___|

// 3/4
//  ___________
// |   |+Y |   |
// |___|___|___|
// |+X |+Z |-X |
// |___|___|___|
// |   |-Y |   |
// |___|___|___|
// |   | Z-|   |
// |___|___|___|

// 4/3
//  _______________
// |   |+Y |   |   |
// |___|___|___|___|
// |+X |+Z |-X |-Z |
// |___|___|___|___|
// |   |-Y |   |   |
// |___|___|___|___|

// 4/2 is spherically mapped
// diagram does not accurately represent the distortion
// but rather the fundamental layout
//  ____________________________________
// |                 +Y                 |
// |___   ______   ______   ______   ___|
// |    Y        Y        Y        Y    |
// |Z   |   +X   |   +Z   |   -X   |   -|
// |    |        |        |        |    |
// |___ ^ ______ ^ ______ ^ ______ ^ ___|
// |                 -Y                 |
// |____________________________________|

DeclareEnum8(CubemapLayout, Invalid, Ratio1_6, Ratio6_1, Ratio2_3, Ratio3_2, Ratio3_4, Ratio4_3, Ratio4_2);

CubemapLayout::Enum GetCubemapLayout(uint width, uint height)
{
  if (width * 6 == height)
    return CubemapLayout::Ratio1_6;
  else if (width == height * 6)
    return CubemapLayout::Ratio6_1;
  else if (width % 2 == 0 && (width / 2) * 3 == height)
    return CubemapLayout::Ratio2_3;
  else if (width % 3 == 0 && (width / 3) * 2 == height)
    return CubemapLayout::Ratio3_2;
  else if (width % 3 == 0 && (width / 3) * 4 == height)
    return CubemapLayout::Ratio3_4;
  else if (width % 4 == 0 && (width / 4) * 3 == height)
    return CubemapLayout::Ratio4_3;
  else if (width % 4 == 0 && (width / 4) * 2 == height)
    return CubemapLayout::Ratio4_2;
  else
    return CubemapLayout::Invalid;
}

void CopyFaces(const byte* image, uint offsets[6], uint dim, uint pixelSize, uint stride, byte* faces[6])
{
  for (uint f = 0; f < 6; ++f)
  {
    const byte* imagePtr = image + offsets[f];
    byte* facePtr = faces[f];

    for (uint y = 0; y < dim; ++y)
    {
      for (uint x = 0; x < dim * pixelSize; ++x)
      {
        uint imageIndex = x + y * stride;
        uint faceIndex = x + y * dim * pixelSize;
        facePtr[faceIndex] = imagePtr[imageIndex];
      }
    }
  }
}

void SetToFaceData(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, uint width, uint height, uint size, byte* faces[6])
{
  TextureFace::Enum faceEnums[] = {TextureFace::PositiveX, TextureFace::PositiveY, TextureFace::PositiveZ, TextureFace::NegativeX, TextureFace::NegativeY, TextureFace::NegativeZ};

  for (uint i = 0; i < 6; ++i)
  {
    MipHeader header;
    header.mFace = faceEnums[i];
    header.mLevel = 0;
    header.mWidth = width;
    header.mHeight = height;
    header.mDataOffset = size * i;
    header.mDataSize = size;

    mipHeaders.PushBack(header);
    imageData.PushBack(faces[i]);
  }

  delete[] imageData[0];
  mipHeaders.EraseAt(0);
  imageData.EraseAt(0);
}

void RotateFace(byte* imageData, uint width, uint height, uint pixelSize)
{
  uint byteWidth = width * pixelSize;

  // Flip X
  for (uint y = 0; y < height; ++y)
  {
    for (uint x = 0; x < byteWidth / 2; x += pixelSize)
    {
      uint i0 = x + y * byteWidth;
      uint i1 = (byteWidth - x - pixelSize) + y * byteWidth;
      for (uint b = 0; b < pixelSize; ++b)
        Math::Swap(imageData[i0 + b], imageData[i1 + b]);
    }
  }

  // Flip Y
  for (uint y = 0; y < height / 2; ++y)
  {
    for (uint x = 0; x < byteWidth; ++x)
    {
      uint i0 = x + y * byteWidth;
      uint i1 = x + (height - y - 1) * byteWidth;
      Math::Swap(imageData[i0], imageData[i1]);
    }
  }
}

void ProcessCubemap1_6(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth;
  uint faceHeight = mipHeaders[0].mHeight / 6;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  uint heigtOffset = faceSize;
  uint offsets[6] = {0, heigtOffset * 2, heigtOffset * 4, heigtOffset, heigtOffset * 3, heigtOffset * 5};

  CopyFaces(imageData[0], offsets, faceWidth, pixelSize, mipHeaders[0].mWidth * pixelSize, faces);
  SetToFaceData(mipHeaders, imageData, faceWidth, faceHeight, faceSize, faces);
}

void ProcessCubemap6_1(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth / 6;
  uint faceHeight = mipHeaders[0].mHeight;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  uint widthOffset = faceWidth * pixelSize;
  uint offsets[6] = {0, widthOffset * 2, widthOffset * 4, widthOffset, widthOffset * 3, widthOffset * 5};

  CopyFaces(imageData[0], offsets, faceWidth, pixelSize, mipHeaders[0].mWidth * pixelSize, faces);
  SetToFaceData(mipHeaders, imageData, faceWidth, faceHeight, faceSize, faces);
}

void ProcessCubemap2_3(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth / 2;
  uint faceHeight = mipHeaders[0].mHeight / 3;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  uint widthOffset = faceWidth * pixelSize;
  uint heightOffset = faceSize * 2;
  uint offsets[6] = {0, heightOffset, heightOffset * 2, widthOffset, widthOffset + heightOffset, widthOffset + heightOffset * 2};

  CopyFaces(imageData[0], offsets, faceWidth, pixelSize, mipHeaders[0].mWidth * pixelSize, faces);
  SetToFaceData(mipHeaders, imageData, faceWidth, faceHeight, faceSize, faces);
}

void ProcessCubemap3_2(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth / 3;
  uint faceHeight = mipHeaders[0].mHeight / 2;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  uint widthOffset = faceWidth * pixelSize;
  uint heightOffset = faceSize * 3;
  uint offsets[6] = {0, widthOffset, widthOffset * 2, heightOffset, heightOffset + widthOffset, heightOffset + widthOffset * 2};

  CopyFaces(imageData[0], offsets, faceWidth, pixelSize, mipHeaders[0].mWidth * pixelSize, faces);
  SetToFaceData(mipHeaders, imageData, faceWidth, faceHeight, faceSize, faces);
}

void ProcessCubemap3_4(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth / 3;
  uint faceHeight = mipHeaders[0].mHeight / 4;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  uint widthOffset = faceWidth * pixelSize;
  uint heightOffset = faceSize * 3;
  uint offsets[6] = {heightOffset, widthOffset, widthOffset + heightOffset, widthOffset * 2 + heightOffset, widthOffset + heightOffset * 2, widthOffset + heightOffset * 3};

  CopyFaces(imageData[0], offsets, faceWidth, pixelSize, mipHeaders[0].mWidth * pixelSize, faces);
  SetToFaceData(mipHeaders, imageData, faceWidth, faceHeight, faceSize, faces);

  RotateFace(imageData[5], faceWidth, faceHeight, pixelSize);
}

void ProcessCubemap4_3(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth / 4;
  uint faceHeight = mipHeaders[0].mHeight / 3;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  uint widthOffset = faceWidth * pixelSize;
  uint heightOffset = faceSize * 4;
  uint offsets[6] = {heightOffset, widthOffset, widthOffset + heightOffset, widthOffset * 2 + heightOffset, widthOffset + heightOffset * 2, widthOffset * 3 + heightOffset};

  CopyFaces(imageData[0], offsets, faceWidth, pixelSize, mipHeaders[0].mWidth * pixelSize, faces);
  SetToFaceData(mipHeaders, imageData, faceWidth, faceHeight, faceSize, faces);
}

// Helper functions for sphere mapping
Vec3 GetFaceDirection(TextureFace::Enum face, float x, float y, float halfWidth)
{
  switch (face)
  {
  case Raverie::TextureFace::PositiveX:
    return Vec3(halfWidth, halfWidth - y, x - halfWidth);
  case Raverie::TextureFace::PositiveY:
    return Vec3(halfWidth - x, halfWidth, y - halfWidth);
  case Raverie::TextureFace::PositiveZ:
    return Vec3(halfWidth - x, halfWidth - y, halfWidth);
  case Raverie::TextureFace::NegativeX:
    return Vec3(-halfWidth, halfWidth - y, halfWidth - x);
  case Raverie::TextureFace::NegativeY:
    return Vec3(halfWidth - x, -halfWidth, halfWidth - y);
  case Raverie::TextureFace::NegativeZ:
    return Vec3(x - halfWidth, halfWidth - y, -halfWidth);
  case Raverie::TextureFace::None:
    break;
  }

  return Vec3::cZero;
}

Vec2 DirectionToUv(Vec3 dir)
{
  dir.AttemptNormalize();

  float theta = Math::ArcTan2(-dir.x, dir.z);
  float phi = Math::ArcCos(dir.y);

  if (Math::Abs(dir.x) < 0.001f && Math::Abs(dir.z) < 0.001f)
    theta = 0.0f;

  Vec2 uv = Vec2(theta / Math::cPi * 0.5f + 0.5f, phi / Math::cPi);
  return uv;
}

int WrapCoord(int value, int size)
{
  if (value < 0)
    value = size - 1;
  else if (value >= size)
    value = 0;
  return value;
}

void ProcessCubemap4_2(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  uint pixelSize = GetPixelSize(format);
  uint faceWidth = mipHeaders[0].mWidth / 4;
  uint faceHeight = mipHeaders[0].mHeight / 2;
  uint faceSize = faceWidth * faceHeight * pixelSize;

  byte* faces[6];
  for (uint i = 0; i < 6; ++i)
    faces[i] = new byte[faceSize];

  byte* image = imageData[0];
  uint width = mipHeaders[0].mWidth;
  uint height = mipHeaders[0].mHeight;

  float halfWidth = faceWidth * 0.5f;

  TextureFace::Enum faceEnums[] = {TextureFace::PositiveX, TextureFace::PositiveY, TextureFace::PositiveZ, TextureFace::NegativeX, TextureFace::NegativeY, TextureFace::NegativeZ};

  byte* sample = new byte[pixelSize];

  for (uint f = 0; f < 6; ++f)
  {
    for (uint y = 0; y < faceHeight; ++y)
    {
      for (uint x = 0; x < faceWidth; ++x)
      {
        Vec3 dir = GetFaceDirection(faceEnums[f], x + 0.5f, y + 0.5f, halfWidth);
        Vec2 uv = DirectionToUv(dir);

        Vec2 sampleCoord;
        sampleCoord.x = uv.x * width - 0.5f;
        sampleCoord.y = uv.y * height - 0.5f;

        int x0 = (int)Math::Floor(sampleCoord.x);
        int x1 = x0 + 1;
        int y0 = (int)Math::Floor(sampleCoord.y);
        int y1 = y0 + 1;

        float xT = sampleCoord.x - x0;
        float yT = sampleCoord.y - y0;

        x0 = WrapCoord(x0, (int)width);
        x1 = WrapCoord(x1, (int)width);
        y0 = Math::Clamp(y0, 0, (int)height - 1);
        y1 = Math::Clamp(y1, 0, (int)height - 1);

        byte* sample00 = &image[(x0 + y0 * width) * pixelSize];
        byte* sample10 = &image[(x1 + y0 * width) * pixelSize];
        byte* sample01 = &image[(x0 + y1 * width) * pixelSize];
        byte* sample11 = &image[(x1 + y1 * width) * pixelSize];

        if (format == TextureFormat::RGB32f)
        {
          float* samplePtr = (float*)sample;
          for (uint i = 0; i < 3; ++i)
            samplePtr[i] = Math::Lerp(Math::Lerp(((float*)sample00)[i], ((float*)sample10)[i], xT), Math::Lerp(((float*)sample01)[i], ((float*)sample11)[i], xT), yT);
        }
        else if (format == TextureFormat::RGBA8)
        {
          for (uint i = 0; i < 3; ++i)
            sample[i] = Math::Lerp(Math::Lerp(sample00[i], sample10[i], xT), Math::Lerp(sample01[i], sample11[i], xT), yT);
        }
        else if (format == TextureFormat::SRGB8A8)
        {
          for (uint i = 0; i < 3; ++i)
            sample[i] = (byte)Math::Pow(Math::Lerp(Math::Lerp(Math::Pow((float)sample00[i], 2.2), Math::Pow((float)sample10[i], 2.2), xT),
                                                   Math::Lerp(Math::Pow((float)sample01[i], 2.2), Math::Pow((float)sample11[i], 2.2), xT),
                                                   yT),
                                        1.0 / 2.2);
        }

        uint faceIndex = (x + y * faceWidth) * pixelSize;
        for (uint i = 0; i < pixelSize; ++i)
          faces[f][faceIndex + i] = sample[i];
      }
    }
  }

  delete[] sample;

  uint dataOffset = 0;

  for (uint i = 0; i < 6; ++i)
  {
    MipHeader header;
    header.mFace = faceEnums[i];
    header.mLevel = 0;
    header.mWidth = faceWidth;
    header.mHeight = faceHeight;
    header.mDataOffset = dataOffset;
    header.mDataSize = faceSize;

    dataOffset += faceSize;

    mipHeaders.PushBack(header);
    imageData.PushBack(faces[i]);
  }

  delete[] imageData[0];
  mipHeaders.EraseAt(0);
  imageData.EraseAt(0);
}

void ExtractCubemapFaces(Status& status, Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format)
{
  if (mipHeaders.Empty() || imageData.Empty())
  {
    status.SetFailed("No image data");
    return;
  }

  uint width = mipHeaders[0].mWidth;
  uint height = mipHeaders[0].mHeight;

  CubemapLayout::Enum layout = GetCubemapLayout(width, height);
  switch (layout)
  {
  case CubemapLayout::Invalid:
    status.SetFailed("Invalid image size for a cubemap layout");
    break;
  case CubemapLayout::Ratio1_6:
    ProcessCubemap1_6(mipHeaders, imageData, format);
    break;
  case CubemapLayout::Ratio6_1:
    ProcessCubemap6_1(mipHeaders, imageData, format);
    break;
  case CubemapLayout::Ratio2_3:
    ProcessCubemap2_3(mipHeaders, imageData, format);
    break;
  case CubemapLayout::Ratio3_2:
    ProcessCubemap3_2(mipHeaders, imageData, format);
    break;
  case CubemapLayout::Ratio3_4:
    ProcessCubemap3_4(mipHeaders, imageData, format);
    break;
  case CubemapLayout::Ratio4_3:
    ProcessCubemap4_3(mipHeaders, imageData, format);
    break;
  case CubemapLayout::Ratio4_2:
    ProcessCubemap4_2(mipHeaders, imageData, format);
    break;
  }
}

TextureFace::Enum FaceIndexToEnum(uint index)
{
  return (TextureFace::Enum)(index + 1);
}

uint FaceEnumToIndex(TextureFace::Enum face)
{
  return (face - 1);
}

void FaceToWorldDir(TextureFace::Enum face, Vec2 uv, Vec3& worldDir)
{
  uv -= Vec2(0.5f);
  switch (face)
  {
  case TextureFace::PositiveX:
    worldDir = Vec3(0.5f, -uv.y, uv.x);
    break;
  case TextureFace::PositiveY:
    worldDir = Vec3(-uv.x, 0.5f, uv.y);
    break;
  case TextureFace::PositiveZ:
    worldDir = Vec3(-uv.x, -uv.y, 0.5f);
    break;
  case TextureFace::NegativeX:
    worldDir = Vec3(-0.5f, -uv.y, -uv.x);
    break;
  case TextureFace::NegativeY:
    worldDir = Vec3(-uv.x, -0.5f, -uv.y);
    break;
  case TextureFace::NegativeZ:
    worldDir = Vec3(uv.x, -uv.y, -0.5f);
    break;
  case TextureFace::None:
    break;
  }

  Math::Normalize(worldDir);
}

void WorldDirToFace(Vec3 worldDir, TextureFace::Enum& face, Vec2& uv)
{
  float absX = Math::Abs(worldDir.x);
  float absY = Math::Abs(worldDir.y);
  float absZ = Math::Abs(worldDir.z);

  if (absX > absY && absX > absZ)
  {
    face = (worldDir.x < 0.0f) ? TextureFace::NegativeX : TextureFace::PositiveX;
    worldDir *= 0.5f / absX;
  }
  else if (absY > absZ)
  {
    face = (worldDir.y < 0.0f) ? TextureFace::NegativeY : TextureFace::PositiveY;
    worldDir *= 0.5f / absY;
  }
  else
  {
    face = (worldDir.z < 0.0f) ? TextureFace::NegativeZ : TextureFace::PositiveZ;
    worldDir *= 0.5f / absZ;
  }

  switch (face)
  {
  case TextureFace::PositiveX:
    uv = Vec2(worldDir.z, -worldDir.y);
    break;
  case TextureFace::PositiveY:
    uv = Vec2(-worldDir.x, worldDir.z);
    break;
  case TextureFace::PositiveZ:
    uv = Vec2(-worldDir.x, -worldDir.y);
    break;
  case TextureFace::NegativeX:
    uv = Vec2(-worldDir.z, -worldDir.y);
    break;
  case TextureFace::NegativeY:
    uv = Vec2(-worldDir.x, -worldDir.z);
    break;
  case TextureFace::NegativeZ:
    uv = Vec2(worldDir.x, -worldDir.y);
    break;
  case TextureFace::None:
    break;
  }

  uv += Vec2(0.5f);
  uv = Math::Clamp(uv, Vec2(0.0f), Vec2(1.0f));
}

// These are pretty rad!
const float gRadicalInverses[1024] = {
    0.000000, 0.500000, 0.250000, 0.750000, 0.125000, 0.625000, 0.375000, 0.875000, 0.062500, 0.562500, 0.312500, 0.812500, 0.187500, 0.687500, 0.437500, 0.937500, 0.031250, 0.531250, 0.281250,
    0.781250, 0.156250, 0.656250, 0.406250, 0.906250, 0.093750, 0.593750, 0.343750, 0.843750, 0.218750, 0.718750, 0.468750, 0.968750, 0.015625, 0.515625, 0.265625, 0.765625, 0.140625, 0.640625,
    0.390625, 0.890625, 0.078125, 0.578125, 0.328125, 0.828125, 0.203125, 0.703125, 0.453125, 0.953125, 0.046875, 0.546875, 0.296875, 0.796875, 0.171875, 0.671875, 0.421875, 0.921875, 0.109375,
    0.609375, 0.359375, 0.859375, 0.234375, 0.734375, 0.484375, 0.984375, 0.007813, 0.507813, 0.257813, 0.757813, 0.132813, 0.632813, 0.382813, 0.882813, 0.070313, 0.570313, 0.320313, 0.820313,
    0.195313, 0.695313, 0.445313, 0.945313, 0.039063, 0.539063, 0.289063, 0.789063, 0.164063, 0.664063, 0.414063, 0.914063, 0.101563, 0.601563, 0.351563, 0.851563, 0.226563, 0.726563, 0.476563,
    0.976563, 0.023438, 0.523438, 0.273438, 0.773438, 0.148438, 0.648438, 0.398438, 0.898438, 0.085938, 0.585938, 0.335938, 0.835938, 0.210938, 0.710938, 0.460938, 0.960938, 0.054688, 0.554688,
    0.304688, 0.804688, 0.179688, 0.679688, 0.429688, 0.929688, 0.117188, 0.617188, 0.367188, 0.867188, 0.242188, 0.742188, 0.492188, 0.992188, 0.003906, 0.503906, 0.253906, 0.753906, 0.128906,
    0.628906, 0.378906, 0.878906, 0.066406, 0.566406, 0.316406, 0.816406, 0.191406, 0.691406, 0.441406, 0.941406, 0.035156, 0.535156, 0.285156, 0.785156, 0.160156, 0.660156, 0.410156, 0.910156,
    0.097656, 0.597656, 0.347656, 0.847656, 0.222656, 0.722656, 0.472656, 0.972656, 0.019531, 0.519531, 0.269531, 0.769531, 0.144531, 0.644531, 0.394531, 0.894531, 0.082031, 0.582031, 0.332031,
    0.832031, 0.207031, 0.707031, 0.457031, 0.957031, 0.050781, 0.550781, 0.300781, 0.800781, 0.175781, 0.675781, 0.425781, 0.925781, 0.113281, 0.613281, 0.363281, 0.863281, 0.238281, 0.738281,
    0.488281, 0.988281, 0.011719, 0.511719, 0.261719, 0.761719, 0.136719, 0.636719, 0.386719, 0.886719, 0.074219, 0.574219, 0.324219, 0.824219, 0.199219, 0.699219, 0.449219, 0.949219, 0.042969,
    0.542969, 0.292969, 0.792969, 0.167969, 0.667969, 0.417969, 0.917969, 0.105469, 0.605469, 0.355469, 0.855469, 0.230469, 0.730469, 0.480469, 0.980469, 0.027344, 0.527344, 0.277344, 0.777344,
    0.152344, 0.652344, 0.402344, 0.902344, 0.089844, 0.589844, 0.339844, 0.839844, 0.214844, 0.714844, 0.464844, 0.964844, 0.058594, 0.558594, 0.308594, 0.808594, 0.183594, 0.683594, 0.433594,
    0.933594, 0.121094, 0.621094, 0.371094, 0.871094, 0.246094, 0.746094, 0.496094, 0.996094, 0.001953, 0.501953, 0.251953, 0.751953, 0.126953, 0.626953, 0.376953, 0.876953, 0.064453, 0.564453,
    0.314453, 0.814453, 0.189453, 0.689453, 0.439453, 0.939453, 0.033203, 0.533203, 0.283203, 0.783203, 0.158203, 0.658203, 0.408203, 0.908203, 0.095703, 0.595703, 0.345703, 0.845703, 0.220703,
    0.720703, 0.470703, 0.970703, 0.017578, 0.517578, 0.267578, 0.767578, 0.142578, 0.642578, 0.392578, 0.892578, 0.080078, 0.580078, 0.330078, 0.830078, 0.205078, 0.705078, 0.455078, 0.955078,
    0.048828, 0.548828, 0.298828, 0.798828, 0.173828, 0.673828, 0.423828, 0.923828, 0.111328, 0.611328, 0.361328, 0.861328, 0.236328, 0.736328, 0.486328, 0.986328, 0.009766, 0.509766, 0.259766,
    0.759766, 0.134766, 0.634766, 0.384766, 0.884766, 0.072266, 0.572266, 0.322266, 0.822266, 0.197266, 0.697266, 0.447266, 0.947266, 0.041016, 0.541016, 0.291016, 0.791016, 0.166016, 0.666016,
    0.416016, 0.916016, 0.103516, 0.603516, 0.353516, 0.853516, 0.228516, 0.728516, 0.478516, 0.978516, 0.025391, 0.525391, 0.275391, 0.775391, 0.150391, 0.650391, 0.400391, 0.900391, 0.087891,
    0.587891, 0.337891, 0.837891, 0.212891, 0.712891, 0.462891, 0.962891, 0.056641, 0.556641, 0.306641, 0.806641, 0.181641, 0.681641, 0.431641, 0.931641, 0.119141, 0.619141, 0.369141, 0.869141,
    0.244141, 0.744141, 0.494141, 0.994141, 0.005859, 0.505859, 0.255859, 0.755859, 0.130859, 0.630859, 0.380859, 0.880859, 0.068359, 0.568359, 0.318359, 0.818359, 0.193359, 0.693359, 0.443359,
    0.943359, 0.037109, 0.537109, 0.287109, 0.787109, 0.162109, 0.662109, 0.412109, 0.912109, 0.099609, 0.599609, 0.349609, 0.849609, 0.224609, 0.724609, 0.474609, 0.974609, 0.021484, 0.521484,
    0.271484, 0.771484, 0.146484, 0.646484, 0.396484, 0.896484, 0.083984, 0.583984, 0.333984, 0.833984, 0.208984, 0.708984, 0.458984, 0.958984, 0.052734, 0.552734, 0.302734, 0.802734, 0.177734,
    0.677734, 0.427734, 0.927734, 0.115234, 0.615234, 0.365234, 0.865234, 0.240234, 0.740234, 0.490234, 0.990234, 0.013672, 0.513672, 0.263672, 0.763672, 0.138672, 0.638672, 0.388672, 0.888672,
    0.076172, 0.576172, 0.326172, 0.826172, 0.201172, 0.701172, 0.451172, 0.951172, 0.044922, 0.544922, 0.294922, 0.794922, 0.169922, 0.669922, 0.419922, 0.919922, 0.107422, 0.607422, 0.357422,
    0.857422, 0.232422, 0.732422, 0.482422, 0.982422, 0.029297, 0.529297, 0.279297, 0.779297, 0.154297, 0.654297, 0.404297, 0.904297, 0.091797, 0.591797, 0.341797, 0.841797, 0.216797, 0.716797,
    0.466797, 0.966797, 0.060547, 0.560547, 0.310547, 0.810547, 0.185547, 0.685547, 0.435547, 0.935547, 0.123047, 0.623047, 0.373047, 0.873047, 0.248047, 0.748047, 0.498047, 0.998047, 0.000977,
    0.500977, 0.250977, 0.750977, 0.125977, 0.625977, 0.375977, 0.875977, 0.063477, 0.563477, 0.313477, 0.813477, 0.188477, 0.688477, 0.438477, 0.938477, 0.032227, 0.532227, 0.282227, 0.782227,
    0.157227, 0.657227, 0.407227, 0.907227, 0.094727, 0.594727, 0.344727, 0.844727, 0.219727, 0.719727, 0.469727, 0.969727, 0.016602, 0.516602, 0.266602, 0.766602, 0.141602, 0.641602, 0.391602,
    0.891602, 0.079102, 0.579102, 0.329102, 0.829102, 0.204102, 0.704102, 0.454102, 0.954102, 0.047852, 0.547852, 0.297852, 0.797852, 0.172852, 0.672852, 0.422852, 0.922852, 0.110352, 0.610352,
    0.360352, 0.860352, 0.235352, 0.735352, 0.485352, 0.985352, 0.008789, 0.508789, 0.258789, 0.758789, 0.133789, 0.633789, 0.383789, 0.883789, 0.071289, 0.571289, 0.321289, 0.821289, 0.196289,
    0.696289, 0.446289, 0.946289, 0.040039, 0.540039, 0.290039, 0.790039, 0.165039, 0.665039, 0.415039, 0.915039, 0.102539, 0.602539, 0.352539, 0.852539, 0.227539, 0.727539, 0.477539, 0.977539,
    0.024414, 0.524414, 0.274414, 0.774414, 0.149414, 0.649414, 0.399414, 0.899414, 0.086914, 0.586914, 0.336914, 0.836914, 0.211914, 0.711914, 0.461914, 0.961914, 0.055664, 0.555664, 0.305664,
    0.805664, 0.180664, 0.680664, 0.430664, 0.930664, 0.118164, 0.618164, 0.368164, 0.868164, 0.243164, 0.743164, 0.493164, 0.993164, 0.004883, 0.504883, 0.254883, 0.754883, 0.129883, 0.629883,
    0.379883, 0.879883, 0.067383, 0.567383, 0.317383, 0.817383, 0.192383, 0.692383, 0.442383, 0.942383, 0.036133, 0.536133, 0.286133, 0.786133, 0.161133, 0.661133, 0.411133, 0.911133, 0.098633,
    0.598633, 0.348633, 0.848633, 0.223633, 0.723633, 0.473633, 0.973633, 0.020508, 0.520508, 0.270508, 0.770508, 0.145508, 0.645508, 0.395508, 0.895508, 0.083008, 0.583008, 0.333008, 0.833008,
    0.208008, 0.708008, 0.458008, 0.958008, 0.051758, 0.551758, 0.301758, 0.801758, 0.176758, 0.676758, 0.426758, 0.926758, 0.114258, 0.614258, 0.364258, 0.864258, 0.239258, 0.739258, 0.489258,
    0.989258, 0.012695, 0.512695, 0.262695, 0.762695, 0.137695, 0.637695, 0.387695, 0.887695, 0.075195, 0.575195, 0.325195, 0.825195, 0.200195, 0.700195, 0.450195, 0.950195, 0.043945, 0.543945,
    0.293945, 0.793945, 0.168945, 0.668945, 0.418945, 0.918945, 0.106445, 0.606445, 0.356445, 0.856445, 0.231445, 0.731445, 0.481445, 0.981445, 0.028320, 0.528320, 0.278320, 0.778320, 0.153320,
    0.653320, 0.403320, 0.903320, 0.090820, 0.590820, 0.340820, 0.840820, 0.215820, 0.715820, 0.465820, 0.965820, 0.059570, 0.559570, 0.309570, 0.809570, 0.184570, 0.684570, 0.434570, 0.934570,
    0.122070, 0.622070, 0.372070, 0.872070, 0.247070, 0.747070, 0.497070, 0.997070, 0.002930, 0.502930, 0.252930, 0.752930, 0.127930, 0.627930, 0.377930, 0.877930, 0.065430, 0.565430, 0.315430,
    0.815430, 0.190430, 0.690430, 0.440430, 0.940430, 0.034180, 0.534180, 0.284180, 0.784180, 0.159180, 0.659180, 0.409180, 0.909180, 0.096680, 0.596680, 0.346680, 0.846680, 0.221680, 0.721680,
    0.471680, 0.971680, 0.018555, 0.518555, 0.268555, 0.768555, 0.143555, 0.643555, 0.393555, 0.893555, 0.081055, 0.581055, 0.331055, 0.831055, 0.206055, 0.706055, 0.456055, 0.956055, 0.049805,
    0.549805, 0.299805, 0.799805, 0.174805, 0.674805, 0.424805, 0.924805, 0.112305, 0.612305, 0.362305, 0.862305, 0.237305, 0.737305, 0.487305, 0.987305, 0.010742, 0.510742, 0.260742, 0.760742,
    0.135742, 0.635742, 0.385742, 0.885742, 0.073242, 0.573242, 0.323242, 0.823242, 0.198242, 0.698242, 0.448242, 0.948242, 0.041992, 0.541992, 0.291992, 0.791992, 0.166992, 0.666992, 0.416992,
    0.916992, 0.104492, 0.604492, 0.354492, 0.854492, 0.229492, 0.729492, 0.479492, 0.979492, 0.026367, 0.526367, 0.276367, 0.776367, 0.151367, 0.651367, 0.401367, 0.901367, 0.088867, 0.588867,
    0.338867, 0.838867, 0.213867, 0.713867, 0.463867, 0.963867, 0.057617, 0.557617, 0.307617, 0.807617, 0.182617, 0.682617, 0.432617, 0.932617, 0.120117, 0.620117, 0.370117, 0.870117, 0.245117,
    0.745117, 0.495117, 0.995117, 0.006836, 0.506836, 0.256836, 0.756836, 0.131836, 0.631836, 0.381836, 0.881836, 0.069336, 0.569336, 0.319336, 0.819336, 0.194336, 0.694336, 0.444336, 0.944336,
    0.038086, 0.538086, 0.288086, 0.788086, 0.163086, 0.663086, 0.413086, 0.913086, 0.100586, 0.600586, 0.350586, 0.850586, 0.225586, 0.725586, 0.475586, 0.975586, 0.022461, 0.522461, 0.272461,
    0.772461, 0.147461, 0.647461, 0.397461, 0.897461, 0.084961, 0.584961, 0.334961, 0.834961, 0.209961, 0.709961, 0.459961, 0.959961, 0.053711, 0.553711, 0.303711, 0.803711, 0.178711, 0.678711,
    0.428711, 0.928711, 0.116211, 0.616211, 0.366211, 0.866211, 0.241211, 0.741211, 0.491211, 0.991211, 0.014648, 0.514648, 0.264648, 0.764648, 0.139648, 0.639648, 0.389648, 0.889648, 0.077148,
    0.577148, 0.327148, 0.827148, 0.202148, 0.702148, 0.452148, 0.952148, 0.045898, 0.545898, 0.295898, 0.795898, 0.170898, 0.670898, 0.420898, 0.920898, 0.108398, 0.608398, 0.358398, 0.858398,
    0.233398, 0.733398, 0.483398, 0.983398, 0.030273, 0.530273, 0.280273, 0.780273, 0.155273, 0.655273, 0.405273, 0.905273, 0.092773, 0.592773, 0.342773, 0.842773, 0.217773, 0.717773, 0.467773,
    0.967773, 0.061523, 0.561523, 0.311523, 0.811523, 0.186523, 0.686523, 0.436523, 0.936523, 0.124023, 0.624023, 0.374023, 0.874023, 0.249023, 0.749023, 0.499023, 0.999023,
};

float RadicalInverse(uint i)
{
  return gRadicalInverses[i];
}

Vec2 Hammersley(uint i, uint count)
{
  return Vec2(i / (float)count, RadicalInverse(i));
}

Vec3 ImportanceSampleGgx(Vec2 xi, float alpha, float random)
{
  // Random rotation on angle phi so sample pattern does not cause
  // banding/ghosting
  float phi = (xi.x + random) * Math::cTwoPi;
  float cosTheta = Math::Sqrt((1.0f - xi.y) / (1.0f + (alpha * alpha - 1.0f) * xi.y));
  float sinTheta = Math::Sqrt(1.0f - cosTheta * cosTheta);

  Vec3 direction;
  direction.x = sinTheta * Math::Cos(phi);
  direction.y = sinTheta * Math::Sin(phi);
  direction.z = cosTheta;
  return direction;
}

float Random(Vec2 uv)
{
  // http://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
  return Math::Fractional(Math::Sin(uv.x * 12.9898f + uv.y * 78.233f) * 43758.5453f);
}

Vec3 SampleEnvMap(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, uint pixelSize, uint index, Vec3 dir)
{
  TextureFace::Enum face;
  Vec2 uv;
  WorldDirToFace(dir, face, uv);

  uint faceIndex = FaceEnumToIndex(face);

  uint width = mipHeaders[index + faceIndex].mWidth;
  uint x = (uint)Math::Max(uv.x * width - 0.5f, 0.0f);
  uint y = (uint)Math::Max(uv.y * width - 0.5f, 0.0f);

  float* pixel = (float*)(imageData[index + faceIndex] + (x + y * width) * pixelSize);
  Vec3 sample(pixel[0], pixel[1], pixel[2]);
  return sample;
}

Vec3 FilterEnvMap(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, uint pixelSize, uint index, Vec3 n, float alpha, float random)
{
  Vec3 v = n;

  // change of basis for sample directions
  Vec3 upVector = Math::Lerp(Vec3::cXAxis, Vec3::cZAxis, (float)(Math::Abs(n.z) < 0.999f));
  Vec3 tangentX = Math::Normalized(Math::Cross(upVector, n));
  Vec3 tangentY = Math::Cross(n, tangentX);

  Vec3 filteredColor = Vec3(0.0f);
  float totalWeight = 0.0f;

  // RadicalInverse values are precomputed, max samples is 1024
  const uint numSamples = 128;
  for (uint i = 0; i < numSamples; ++i)
  {
    Vec2 xi = Hammersley(i, numSamples);
    Vec3 dir = ImportanceSampleGgx(xi, alpha, random);
    Vec3 h = tangentX * dir.x + tangentY * dir.y + n * dir.z;
    Vec3 l = v.ReflectAcrossVector(h);
    float dotnl = Math::Clamp(Math::Dot(n, l), 0.0f, 1.0f);

    // Luminance weighting in combination with the cosine weight to prevent
    // very bright contrasting pixel artifacts
    // Likely not as physically accurate but it looks significantly better
    // when using as few samples as possible to save processing time
    Vec3 sample = SampleEnvMap(mipHeaders, imageData, pixelSize, index, l);
    float weight = dotnl / (1.0f + Luminance(sample));
    filteredColor += sample * weight;
    totalWeight += weight;
  }

  filteredColor /= totalWeight;
  return filteredColor;
}

class FilterJob : public Job
{
public:
  void Execute() override
  {
    uint width = (*mMipHeaders)[mTargetIndex].mWidth;
    float alpha = mRoughness * mRoughness;

    // mFaceIndex assumed to be 0 when doing all 6 faces
    for (uint i = 0; i < mFaceCount; ++i)
    {
      for (uint y = 0; y < width; ++y)
      {
        for (uint x = 0; x < width; ++x)
        {
          Vec2 uv = Vec2(x + 0.5f, y + 0.5f) / (float)width;
          Vec3 worldDir;
          FaceToWorldDir(FaceIndexToEnum(mFaceIndex + i), uv, worldDir);

          Vec3 filteredSample = FilterEnvMap(*mMipHeaders, *mImageData, mPixelSize, mSourceIndex, worldDir, alpha, Random(uv));

          float* pixel = (float*)((*mImageData)[mTargetIndex + i] + (x + y * width) * mPixelSize);
          pixel[0] = filteredSample.x;
          pixel[1] = filteredSample.y;
          pixel[2] = filteredSample.z;
        }
      }
    }

    mCountdownEvent->DecrementCount();
  }

  Array<MipHeader>* mMipHeaders;
  Array<byte*>* mImageData;
  uint mPixelSize;
  uint mSourceIndex;
  uint mTargetIndex;
  uint mFaceIndex;
  uint mFaceCount;
  float mRoughness;
  CountdownEvent* mCountdownEvent;
};

void MipmapCubemap(Array<MipHeader>& mipHeaders, Array<byte*>& imageData, TextureFormat::Enum format, bool compressed)
{
  if (mipHeaders.Size() != 6 || imageData.Size() != 6)
    return;

  // Only incoming formats are RGBA8, SRGB8A8, and RGB32f
  if (format != TextureFormat::RGBA8 && format != TextureFormat::SRGB8 && format != TextureFormat::RGB32f)
    return;

  // If mips are going to be compressed we need the top level faces to be a
  // power of two in order to guarantee that every mip size is a multiple of 4
  // Pixel padding caused by compressing sizes 2 or 1 works correctly if image
  // needs to be y inverted
  if (compressed)
  {
    uint width = mipHeaders[0].mWidth;
    uint newWidth = NextPowerOfTwo(width - 1);

    // If not already power of two
    if (width != newWidth)
    {
      uint dataOffset = 0;
      for (uint i = 0; i < mipHeaders.Size(); ++i)
      {
        uint newSize = newWidth * newWidth * GetPixelSize(format);
        byte* newImage = new byte[newSize];
        ResizeImage(format, imageData[i], width, width, newImage, newWidth, newWidth);

        delete[] imageData[i];
        imageData[i] = newImage;

        mipHeaders[i].mWidth = newWidth;
        mipHeaders[i].mHeight = newWidth;
        mipHeaders[i].mDataSize = newSize;

        mipHeaders[i].mDataOffset = dataOffset;
        dataOffset += mipHeaders[i].mDataSize;
      }
    }
  }

  // Only mipmap in RGB32f format, byte formats will get an alpha of 255
  if (format != TextureFormat::RGB32f)
  {
    uint targetPixelSize = GetPixelSize(TextureFormat::RGB32f);
    uint dataOffset = 0;

    for (uint i = 0; i < mipHeaders.Size(); ++i)
    {
      uint width = mipHeaders[i].mWidth;
      uint pixelCount = width * width;
      uint faceSize = pixelCount * targetPixelSize;

      float* targetFace = (float*)(new byte[faceSize]);
      byte* sourceFace = imageData[i];
      for (uint p = 0; p < pixelCount; ++p)
      {
        targetFace[p * 3 + 0] = sourceFace[p * 4 + 0] / 255.0f;
        targetFace[p * 3 + 1] = sourceFace[p * 4 + 1] / 255.0f;
        targetFace[p * 3 + 2] = sourceFace[p * 4 + 2] / 255.0f;
      }

      delete[] imageData[i];
      imageData[i] = (byte*)targetFace;

      mipHeaders[i].mDataSize = faceSize;
      mipHeaders[i].mDataOffset = dataOffset;
      dataOffset += faceSize;
    }
  }

  uint pixelSize = GetPixelSize(TextureFormat::RGB32f);
  uint dataOffset = mipHeaders.Back().mDataOffset + mipHeaders.Back().mDataSize;

  uint firstMipWidth = mipHeaders[0].mWidth / 2;
  uint currentMip = 1;

  // Allocate all mip levels
  for (uint mipWidth = firstMipWidth; mipWidth >= 1; mipWidth /= 2)
  {
    uint size = mipWidth * mipWidth * pixelSize;

    for (uint i = 0; i < 6; ++i)
    {
      MipHeader header;
      header.mFace = FaceIndexToEnum(i);
      header.mLevel = currentMip;
      header.mWidth = mipWidth;
      header.mHeight = mipWidth;
      header.mDataOffset = dataOffset;
      header.mDataSize = size;

      dataOffset += size;

      mipHeaders.PushBack(header);
      imageData.PushBack(new byte[size]);
    }

    ++currentMip;
  }

  // Timer timer;
  CountdownEvent countdownEvent;

  uint maxMip = (uint)Math::Floor(Math::Log2((real)mipHeaders[0].mWidth));
  currentMip = 1;

  uint sourceIndex = 0;
  uint targetIndex = 6;

  for (uint mipWidth = firstMipWidth; mipWidth >= 1; mipWidth /= 2)
  {
    float roughness = currentMip / (float)maxMip;
    ++currentMip;

    if (mipWidth <= 8)
    {
      countdownEvent.IncrementCount();

      FilterJob* job = new FilterJob();
      job->mMipHeaders = &mipHeaders;
      job->mImageData = &imageData;
      job->mPixelSize = pixelSize;
      job->mSourceIndex = sourceIndex;
      job->mTargetIndex = targetIndex;
      job->mFaceIndex = 0;
      job->mFaceCount = 6;
      job->mRoughness = roughness;
      job->mCountdownEvent = &countdownEvent;
      job->mRunImmediateWhenThreadingDisabled = true;
      Z::gJobs->AddJob(job);
    }
    else
    {
      for (uint i = 0; i < 6; ++i)
      {
        countdownEvent.IncrementCount();

        FilterJob* job = new FilterJob();
        job->mMipHeaders = &mipHeaders;
        job->mImageData = &imageData;
        job->mPixelSize = pixelSize;
        job->mSourceIndex = sourceIndex;
        job->mTargetIndex = targetIndex + i;
        job->mFaceIndex = i;
        job->mFaceCount = 1;
        job->mRoughness = roughness;
        job->mCountdownEvent = &countdownEvent;
        job->mRunImmediateWhenThreadingDisabled = true;
        Z::gJobs->AddJob(job);
      }

      countdownEvent.Wait();

      sourceIndex += 6;
    }

    targetIndex += 6;
  }

  countdownEvent.Wait();
  // timer.Update();
  // double time = timer.Time();
  // ZPrint("Time: %f\n", time);

  // Convert back to byte format if needed
  if (format != TextureFormat::RGB32f)
  {
    uint targetPixelSize = GetPixelSize(TextureFormat::RGBA8);
    uint offset = 0;

    for (uint i = 0; i < mipHeaders.Size(); ++i)
    {
      uint width = mipHeaders[i].mWidth;
      uint pixelCount = width * width;
      uint faceSize = pixelCount * targetPixelSize;

      byte* targetFace = new byte[faceSize];
      float* sourceFace = (float*)imageData[i];
      for (uint p = 0; p < pixelCount; ++p)
      {
        targetFace[p * 4 + 0] = (byte)Math::Clamp(sourceFace[p * 3 + 0] * 255.0f, 0.0f, 255.0f);
        targetFace[p * 4 + 1] = (byte)Math::Clamp(sourceFace[p * 3 + 1] * 255.0f, 0.0f, 255.0f);
        targetFace[p * 4 + 2] = (byte)Math::Clamp(sourceFace[p * 3 + 2] * 255.0f, 0.0f, 255.0f);
        targetFace[p * 4 + 3] = (byte)255;
      }

      delete[] imageData[i];
      imageData[i] = targetFace;

      mipHeaders[i].mDataSize = faceSize;
      mipHeaders[i].mDataOffset = offset;
      offset += faceSize;
    }
  }
}

} // namespace Raverie
