#include "Precompiled.hpp"

namespace Zero
{

void SetPixelData(byte* data, uint index, Vec4 value, TextureFormat::Enum format)
{
  switch (format)
  {
    case TextureFormat::R8:      SetPixelDataByte(data, index, value, 1);      break;
    case TextureFormat::RG8:     SetPixelDataByte(data, index, value, 2);      break;
    case TextureFormat::RGB8:    SetPixelDataByte(data, index, value, 3);      break;
    case TextureFormat::RGBA8:   SetPixelDataByte(data, index, value, 4);      break;

    case TextureFormat::R16:     SetPixelDataShort(data, index, value, 1);     break;
    case TextureFormat::RG16:    SetPixelDataShort(data, index, value, 2);     break;
    case TextureFormat::RGB16:   SetPixelDataShort(data, index, value, 3);     break;
    case TextureFormat::RGBA16:  SetPixelDataShort(data, index, value, 4);     break;

    case TextureFormat::R16f:    SetPixelDataHalfFloat(data, index, value, 1); break;
    case TextureFormat::RG16f:   SetPixelDataHalfFloat(data, index, value, 2); break;
    case TextureFormat::RGB16f:  SetPixelDataHalfFloat(data, index, value, 3); break;
    case TextureFormat::RGBA16f: SetPixelDataHalfFloat(data, index, value, 4); break;

    case TextureFormat::R32f:    SetPixelDataFloat(data, index, value, 1);     break;
    case TextureFormat::RG32f:   SetPixelDataFloat(data, index, value, 2);     break;
    case TextureFormat::RGB32f:  SetPixelDataFloat(data, index, value, 3);     break;
    case TextureFormat::RGBA32f: SetPixelDataFloat(data, index, value, 4);     break;

    case TextureFormat::SRGB8:   SetPixelDataGamma(data, index, value, 3);     break;
    case TextureFormat::SRGB8A8: SetPixelDataGamma(data, index, value, 4);     break;
  }
}

void ReadPixelData(byte* data, uint index, Vec4& value, TextureFormat::Enum format)
{
  switch (format)
  {
    case TextureFormat::R8:      ReadPixelDataByte(data, index, value, 1);      break;
    case TextureFormat::RG8:     ReadPixelDataByte(data, index, value, 2);      break;
    case TextureFormat::RGB8:    ReadPixelDataByte(data, index, value, 3);      break;
    case TextureFormat::RGBA8:   ReadPixelDataByte(data, index, value, 4);      break;

    case TextureFormat::R16:     ReadPixelDataShort(data, index, value, 1);     break;
    case TextureFormat::RG16:    ReadPixelDataShort(data, index, value, 2);     break;
    case TextureFormat::RGB16:   ReadPixelDataShort(data, index, value, 3);     break;
    case TextureFormat::RGBA16:  ReadPixelDataShort(data, index, value, 4);     break;

    case TextureFormat::R16f:    ReadPixelDataHalfFloat(data, index, value, 1); break;
    case TextureFormat::RG16f:   ReadPixelDataHalfFloat(data, index, value, 2); break;
    case TextureFormat::RGB16f:  ReadPixelDataHalfFloat(data, index, value, 3); break;
    case TextureFormat::RGBA16f: ReadPixelDataHalfFloat(data, index, value, 4); break;

    case TextureFormat::R32f:    ReadPixelDataFloat(data, index, value, 1);     break;
    case TextureFormat::RG32f:   ReadPixelDataFloat(data, index, value, 2);     break;
    case TextureFormat::RGB32f:  ReadPixelDataFloat(data, index, value, 3);     break;
    case TextureFormat::RGBA32f: ReadPixelDataFloat(data, index, value, 4);     break;

    case TextureFormat::SRGB8:   ReadPixelDataGamma(data, index, value, 3);     break;
    case TextureFormat::SRGB8A8: ReadPixelDataGamma(data, index, value, 4);     break;
  }
}

void SetPixelDataByte(byte* data, uint index, Vec4 value, uint elementCount)
{
  const uint maxByte = 0xFF;
  const float normFactor = 0x100;
  switch (elementCount)
  {
    case 4: ((byte*)(data + index))[3] = (byte)Math::Min(maxByte, (uint)(Math::Max(value.w, 0.0f) * normFactor));
    case 3: ((byte*)(data + index))[2] = (byte)Math::Min(maxByte, (uint)(Math::Max(value.z, 0.0f) * normFactor));
    case 2: ((byte*)(data + index))[1] = (byte)Math::Min(maxByte, (uint)(Math::Max(value.y, 0.0f) * normFactor));
    case 1: ((byte*)(data + index))[0] = (byte)Math::Min(maxByte, (uint)(Math::Max(value.x, 0.0f) * normFactor));
  }
}

void SetPixelDataShort(byte* data, uint index, Vec4 value, uint elementCount)
{
  const uint maxShort = 0xFFFF;
  const float normFactor = 0x10000;
  switch (elementCount)
  {
    case 4: ((u16*)(data + index))[3] = (u16)Math::Min(maxShort, (uint)(Math::Max(value.w, 0.0f) * normFactor));
    case 3: ((u16*)(data + index))[2] = (u16)Math::Min(maxShort, (uint)(Math::Max(value.z, 0.0f) * normFactor));
    case 2: ((u16*)(data + index))[1] = (u16)Math::Min(maxShort, (uint)(Math::Max(value.y, 0.0f) * normFactor));
    case 1: ((u16*)(data + index))[0] = (u16)Math::Min(maxShort, (uint)(Math::Max(value.x, 0.0f) * normFactor));
  }
}

void SetPixelDataHalfFloat(byte* data, uint index, Vec4 value, uint elementCount)
{
  switch (elementCount)
  {
    case 4: ((u16*)(data + index))[3] = HalfFloatConverter::ToHalfFloat(value.w);
    case 3: ((u16*)(data + index))[2] = HalfFloatConverter::ToHalfFloat(value.z);
    case 2: ((u16*)(data + index))[1] = HalfFloatConverter::ToHalfFloat(value.y);
    case 1: ((u16*)(data + index))[0] = HalfFloatConverter::ToHalfFloat(value.x);
  }
}

void SetPixelDataFloat(byte* data, uint index, Vec4 value, uint elementCount)
{
  switch (elementCount)
  {
    case 4: ((float*)(data + index))[3] = value.w;
    case 3: ((float*)(data + index))[2] = value.z;
    case 2: ((float*)(data + index))[1] = value.y;
    case 1: ((float*)(data + index))[0] = value.x;
  }
}

void SetPixelDataGamma(byte* data, uint index, Vec4 value, uint elementCount)
{
  const uint maxByte = 0xFF;
  const float normFactor = 0x100;
  const float gammaPower = 1.0f / 2.2f;
  switch (elementCount)
  {
    case 4: ((byte*)(data + index))[3] = (byte)Math::Min(maxByte, (uint)(Math::Max(value.w, 0.0f) * normFactor));
    case 3: ((byte*)(data + index))[2] = (byte)Math::Min(maxByte, (uint)(Math::Pow(Math::Max(value.z, 0.0f), gammaPower) * normFactor));
    case 2: ((byte*)(data + index))[1] = (byte)Math::Min(maxByte, (uint)(Math::Pow(Math::Max(value.y, 0.0f), gammaPower) * normFactor));
    case 1: ((byte*)(data + index))[0] = (byte)Math::Min(maxByte, (uint)(Math::Pow(Math::Max(value.x, 0.0f), gammaPower) * normFactor));
  }
}

void ReadPixelDataByte(byte* data, uint index, Vec4& value, uint elementCount)
{
  const uint maxByte = 0xFF;
  const float normFactor = 1.0f / 0x100;
  switch (elementCount)
  {
    uint isOne;
    case 4: isOne = ((byte*)(data + index))[3] / maxByte; value.w = ((byte*)(data + index))[3] * normFactor * (1 - isOne) + isOne;
    case 3: isOne = ((byte*)(data + index))[2] / maxByte; value.z = ((byte*)(data + index))[2] * normFactor * (1 - isOne) + isOne;
    case 2: isOne = ((byte*)(data + index))[1] / maxByte; value.y = ((byte*)(data + index))[1] * normFactor * (1 - isOne) + isOne;
    case 1: isOne = ((byte*)(data + index))[0] / maxByte; value.x = ((byte*)(data + index))[0] * normFactor * (1 - isOne) + isOne;
  }
}

void ReadPixelDataShort(byte* data, uint index, Vec4& value, uint elementCount)
{
  const uint maxShort = 0xFFFF;
  const float normFactor = 1.0f / 0x10000;
  switch (elementCount)
  {
    uint isOne;
    case 4: isOne = ((u16*)(data + index))[3] / maxShort; value.w = ((u16*)(data + index))[3] * normFactor * (1 - isOne) + isOne;
    case 3: isOne = ((u16*)(data + index))[2] / maxShort; value.z = ((u16*)(data + index))[2] * normFactor * (1 - isOne) + isOne;
    case 2: isOne = ((u16*)(data + index))[1] / maxShort; value.y = ((u16*)(data + index))[1] * normFactor * (1 - isOne) + isOne;
    case 1: isOne = ((u16*)(data + index))[0] / maxShort; value.x = ((u16*)(data + index))[0] * normFactor * (1 - isOne) + isOne;
  }
}

void ReadPixelDataHalfFloat(byte* data, uint index, Vec4& value, uint elementCount)
{
  switch (elementCount)
  {
    case 4: value.w = HalfFloatConverter::ToFloat(((u16*)(data + index))[3]);
    case 3: value.z = HalfFloatConverter::ToFloat(((u16*)(data + index))[2]);
    case 2: value.y = HalfFloatConverter::ToFloat(((u16*)(data + index))[1]);
    case 1: value.x = HalfFloatConverter::ToFloat(((u16*)(data + index))[0]);
  }
}

void ReadPixelDataFloat(byte* data, uint index, Vec4& value, uint elementCount)
{
  switch (elementCount)
  {
    case 4: value.w = ((float*)(data + index))[3];
    case 3: value.z = ((float*)(data + index))[2];
    case 2: value.y = ((float*)(data + index))[1];
    case 1: value.x = ((float*)(data + index))[0];
  }
}

void ReadPixelDataGamma(byte* data, uint index, Vec4& value, uint elementCount)
{
  const uint maxByte = 0xFF;
  const float normFactor = 1.0f / 0x100;
  const float gammaPower = 2.2f;
  switch (elementCount)
  {
    uint isOne;
    case 4: isOne = ((byte*)(data + index))[3] / maxByte; value.w = ((byte*)(data + index))[3] * normFactor * (1 - isOne) + isOne;
    case 3: isOne = ((byte*)(data + index))[2] / maxByte; value.z = Math::Pow(((byte*)(data + index))[2] * normFactor, gammaPower) * (1 - isOne) + isOne;
    case 2: isOne = ((byte*)(data + index))[1] / maxByte; value.y = Math::Pow(((byte*)(data + index))[1] * normFactor, gammaPower) * (1 - isOne) + isOne;
    case 1: isOne = ((byte*)(data + index))[0] / maxByte; value.x = Math::Pow(((byte*)(data + index))[0] * normFactor, gammaPower) * (1 - isOne) + isOne;
  }
}

bool IsColorFormat(TextureFormat::Enum format)
{
  return (format > TextureFormat::None && format < TextureFormat::Depth16);
}

bool IsShortColorFormat(TextureFormat::Enum format)
{
  return (format >= TextureFormat::R16 && format <= TextureFormat::RGBA16);
}

bool IsFloatColorFormat(TextureFormat::Enum format)
{
  return (format >= TextureFormat::R16f && format <= TextureFormat::RGBA32f);
}

bool IsDepthFormat(TextureFormat::Enum format)
{
  return (format >= TextureFormat::Depth16);
}

bool IsDepthStencilFormat(TextureFormat::Enum format)
{
  return (format >= TextureFormat::Depth24Stencil8);
}

// Compression utilities
// BC6 documentation: https://msdn.microsoft.com/en-us/library/windows/desktop/hh308952(v=vs.85).aspx
class PartitionPattern
{
public:
  u32 mIndexBits0_7;
  u32 mIndexBits8_15;
  byte mInvertsTo;
  bool mSwapEndpoints01;
  bool mImplicit0Top;
  bool mImplicit0Bottom;
};

const PartitionPattern cPartitionPatterns[32] =
{
  {0xfc0fc0, 0xfc0fc0,  0, false, false, false}, // 0
  {0xe00e00, 0xe00e00,  1, false, false, false}, // 1
  {0xff8ff8, 0xff8ff8,  2, false, false, false}, // 2
  {0xfc0e00, 0xff8fc0, 23, false, false, false}, // 3
  {0xe00000, 0xfc0e00, 24, false, false, false}, // 4
  {0xff8fc0, 0xfffff8, 25,  true, false, false}, // 5
  {0xfc0e00, 0xfffff8, 21,  true, false, false}, // 6
  {0xe00000, 0xff8fc0, 19, false, false, false}, // 7
  {0x000000, 0xfc0e00, 20, false, false, false}, // 8
  {0xff8fc0, 0xffffff, 22,  true, false, false}, // 9
  {0xe00000, 0xfffff8, 16,  true, false, false}, // 10
  {0x000000, 0xff8e00, 17, false, false, false}, // 11
  {0xff8e00, 0xffffff, 18,  true, false, false}, // 12
  {0x000000, 0xffffff, 13,  true, false, false}, // 13
  {0xfff000, 0xffffff, 15,  true, false, false}, // 14
  {0x000000, 0xfff000, 14,  true, false, false}, // 15
  {0x007000, 0xfff1ff, 10,  true, false, false}, // 16
  {0xe00ff8, 0x000000, 11, false,  true, false}, // 17
  {0x000000, 0x1ff007, 12,  true, false,  true}, // 18
  {0xfc0ff8, 0x000e00,  7, false,  true, false}, // 19
  {0xe00fc0, 0x000000,  8, false,  true, false}, // 20
  {0x007000, 0x1ff03f,  6,  true, false,  true}, // 21
  {0x000000, 0x03f007,  9,  true, false,  true}, // 22
  {0xfc0ff8, 0xe00fc0,  3, false, false, false}, // 23
  {0xc00fc0, 0x000c00,  4, false,  true, false}, // 24
  {0x007000, 0x03f007,  5,  true, false,  true}, // 25
  {0x1f81f8, 0x1f81f8, 26, false,  true, false}, // 26
  {0x1f8fc0, 0x03f1f8, 31,  true,  true, false}, // 27
  {0xff8e00, 0x0071ff, 30,  true, false,  true}, // 28
  {0xfff000, 0x000fff, 29, false, false,  true}, // 29
  {0xe00ff8, 0x1ff007, 28,  true,  true, false}, // 30
  {0xe07fc0, 0x03fe07, 27,  true,  true, false}, // 31
};

uint GetBlockSize(TextureCompression::Enum compression)
{
  switch (compression)
  {
    case TextureCompression::BC1: return 8;
    case TextureCompression::BC2: return 16;
    case TextureCompression::BC3: return 16;
    case TextureCompression::BC4: return 8;
    case TextureCompression::BC5: return 16;
    case TextureCompression::BC6: return 16;
    //case TextureCompression::BC7: return 16;
    default: return 0;
  }
}

void YInvertBC1Block(byte* block)
{
  Math::Swap(block[4], block[7]);
  Math::Swap(block[5], block[6]);
}

void YInvertBC3Block(byte* block)
{
  u32 indices0_7 = block[2] + 256 * (block[3] + 256 * block[4]);
  u32 indices8_15 = block[5] + 256 * (block[6] + 256 * block[7]);

  indices0_7 = ((indices0_7 & 0x00fff000) >> 12) | ((indices0_7 & 0x00000fff) << 12);
  indices8_15 = ((indices8_15 & 0x00fff000) >> 12) | ((indices8_15 & 0x00000fff) << 12);

  block[2] = (indices8_15 & 0x000000ff);
  block[3] = (indices8_15 & 0x0000ff00) >> 8;
  block[4] = (indices8_15 & 0x00ff0000) >> 16;

  block[5] = (indices0_7 & 0x000000ff);
  block[6] = (indices0_7 & 0x0000ff00) >> 8;
  block[7] = (indices0_7 & 0x00ff0000) >> 16;
}

void YInvertBC6Mode10Block(byte* block)
{
  u32 indices0_7 = block[10] + 256 * (block[11] + 256 * block[12]);
  u32 indices8_15 = block[13] + 256 * (block[14] + 256 * block[15]);

  // Second implicit 0 is in second half for most cases
  indices0_7 = ((indices8_15 & 0x00000001) << 23) | (indices0_7 >> 1);
  indices8_15 >>= 1;

  // Fill implicit 0 from index0
  indices0_7 = (indices0_7 & 0x00fffff8) | ((indices0_7 & 0x00000006) >> 1);

  // Partition value before flipping
  byte partition = (block[9] >> 5) | ((block[10] & 0x03) << 3);

  // Fix implicit index bits if needed
  if (cPartitionPatterns[partition].mImplicit0Top)
  {
    indices8_15 = (indices8_15 << 1) | ((indices0_7 & 0x00800000) >> 23);
    indices0_7 = ((indices0_7 & 0x007fff00) << 1) | (indices0_7 & 0x000000ff);
  }
  else if (cPartitionPatterns[partition].mImplicit0Bottom)
  {
    indices8_15 = ((indices8_15 & 0x007ffffc) << 1) | (indices8_15 & 0x00000003);
  }

  // Swap indices
  indices0_7 = ((indices0_7 & 0x00fff000) >> 12) | ((indices0_7 & 0x00000fff) << 12);
  indices8_15 = ((indices8_15 & 0x00fff000) >> 12) | ((indices8_15 & 0x00000fff) << 12);
  Math::Swap(indices0_7, indices8_15);

  // Determine which endpoints need to be swapped from partition flip and required implicit 0's
  byte newPartition = cPartitionPatterns[partition].mInvertsTo;
  bool swapEndpoints01 = cPartitionPatterns[partition].mSwapEndpoints01;
  bool swapEndpoints0AB = indices0_7 & 0x00000004;
  bool swapEndpoints1AB = false;

  if (cPartitionPatterns[newPartition].mImplicit0Top)
    swapEndpoints1AB = indices0_7  & 0x00000100;
  else if (cPartitionPatterns[newPartition].mImplicit0Bottom)
    swapEndpoints1AB = indices8_15 & 0x00000004;
  else
    swapEndpoints1AB = indices8_15 & 0x00800000;

  // Invert index values to get implicit 0's in the required locations
  if (swapEndpoints0AB)
  {
    indices0_7 = (~indices0_7 & ~cPartitionPatterns[newPartition].mIndexBits0_7) | (indices0_7 & cPartitionPatterns[newPartition].mIndexBits0_7);
    indices8_15 = (~indices8_15 & ~cPartitionPatterns[newPartition].mIndexBits8_15) | (indices8_15 & cPartitionPatterns[newPartition].mIndexBits8_15);
  }

  if (swapEndpoints1AB)
  {
    indices0_7 = (~indices0_7 & cPartitionPatterns[newPartition].mIndexBits0_7) | (indices0_7 & ~cPartitionPatterns[newPartition].mIndexBits0_7);
    indices8_15 = (~indices8_15 & cPartitionPatterns[newPartition].mIndexBits8_15) | (indices8_15 & ~cPartitionPatterns[newPartition].mIndexBits8_15);
  }

  // Remove implicit 0's
  indices0_7 = (indices0_7 & 0x00fffff8) | (indices0_7 & 0x00000003) << 1;
  if (cPartitionPatterns[newPartition].mImplicit0Top)
  {
    indices0_7 = (indices0_7 & 0x00fffe00) | (indices0_7 & 0x000000ff) << 1;
  }
  else if (cPartitionPatterns[newPartition].mImplicit0Bottom)
  {
    indices8_15 = (indices8_15 & 0x00fffff8) | (indices8_15 & 0x00000003) << 1 | (indices0_7 & 0x00800000) >> 23;
    indices0_7 = (indices0_7 & 0x007fffff) << 1;
  }
  else
  {
    indices8_15 = indices8_15 << 1 | (indices0_7 & 0x00800000) >> 23;
    indices0_7 = (indices0_7 & 0x007fffff) << 1;
  }

  // Only pull out endpoint data if any swaps are required
  if (swapEndpoints01 || swapEndpoints0AB || swapEndpoints1AB)
  {
    // Bit locations for all endpoint values
    // a. byte number
    // b. bit visualization
    // c. identifier and bit numbers
    // a. |        9|         8|         7|           6|           5|                   4|               3|                   2|                     1|        0|
    // b.  xxx 00000 0 000000 0 000  00000 0 0000   000 000 0000   0 00000 0    0    0    0    000000 0    0    0    0    00000 0 0    00     0    000 000 xxxxx
    // c.     |r1B5:0 |r1A5:0|b1A3:0|b0B5:0 |g1B3:0|g0B5:0 |g1A3:0|r0B5:0 |b1B4|b1B5|b1B3|g1B5|b0A5:0|g1A4|b1B2|b1A5|g1A5|g0A5:0 |b1A4|b1B1:0|g1B4|r0A5:0 |

    // Read all endpoints
    byte r0A = (block[1] & 0x07) << 3 | (block[0] & 0xe0) >> 5;
    byte r0B = (block[5] & 0x01) << 5 | (block[4] & 0xf8) >> 3;
    byte r1A = (block[8] & 0x7e) >> 1;
    byte r1B = (block[9] & 0x1f) << 1 | (block[8] & 0x80) >> 7;

    byte g0A = (block[2] & 0x1f) << 1 | (block[1] & 0x80) >> 7;
    byte g0B = (block[6] & 0x07) << 3 | (block[5] & 0xe0) >> 5;
    byte g1A = (block[2] & 0x20) | (block[3] & 0x01) << 4 | (block[5] & 0x1e) >> 1;
    byte g1B = (block[3] & 0x80) >> 2 | (block[1] & 0x08) << 1 | (block[6] & 0x78) >> 3;

    byte b0A = (block[3] & 0x7e) >> 1;
    byte b0B = (block[7] & 0x1f) << 1 | (block[6] & 0x80) >> 7;
    byte b1A = (block[2] & 0x40) >> 1 | (block[1] & 0x40) >> 2 | (block[8] & 0x01) << 3 | (block[7] & 0xe0) >> 5;
    byte b1B = (block[4] & 0x02) << 4 | (block[4] & 0x04) << 2 | (block[4] & 0x01) << 3 | (block[2] & 0x80) >> 5 | (block[1] & 0x30) >> 4;

    // Other swaps assume this swap will happen first if it's needed
    if (swapEndpoints01)
    {
      Math::Swap(r0A, r1A);
      Math::Swap(r0B, r1B);
      Math::Swap(g0A, g1A);
      Math::Swap(g0B, g1B);
      Math::Swap(b0A, b1A);
      Math::Swap(b0B, b1B);
    }

    if (swapEndpoints0AB)
    {
      Math::Swap(r0A, r0B);
      Math::Swap(g0A, g0B);
      Math::Swap(b0A, b0B);
    }

    if (swapEndpoints1AB)
    {
      Math::Swap(r1A, r1B);
      Math::Swap(g1A, g1B);
      Math::Swap(b1A, b1B);
    }

    // Write new endpoints
    block[0] = (r0A & 0x07) << 5 | (block[0] & 0x1f);
    block[1] = (g0A & 0x01) << 7 | (b1A & 0x10) << 2 | (b1B & 0x03) << 4 | (g1B & 0x10) >> 1 | (r0A & 0x38) >> 3;
    block[2] = (b1B & 0x04) << 5 | (b1A & 0x20) << 1 | (g1A & 0x20) | (g0A & 0x3e) >> 1;
    block[3] = (g1B & 0x20) << 2 | (b0A & 0x3f) << 1 | (g1A & 0x10) >> 4;
    block[4] = (r0B & 0x1f) << 3 | (b1B & 0x10) >> 2 | (b1B & 0x20) >> 4 | (b1B & 0x08) >> 3;
    block[5] = (g0B & 0x07) << 5 | (g1A & 0x0f) << 1 | (r0B & 0x20) >> 5;
    block[6] = (b0B & 0x01) << 7 | (g1B & 0x0f) << 3 | (g0B & 0x38) >> 3;
    block[7] = (b1A & 0x07) << 5 | (b0B & 0x3e) >> 1;
    block[8] = (r1B & 0x01) << 7 | (r1A & 0x3f) << 1 | (b1A & 0x08) >> 3;
    block[9] = (block[9] & 0xe0) | (r1B & 0x3e) >> 1;
  }

  // Write new partition
  block[9] = (newPartition & 0x07) << 5 | (block[9] & 0x1f);
  block[10] = (newPartition & 0x18) >> 3;

  // Write new indices
  block[10] = (indices0_7 & 0x000000fc) | (block[10] & 0x03);
  block[11] = (indices0_7 & 0x0000ff00) >> 8;
  block[12] = (indices0_7 & 0x00ff0000) >> 16;
  block[13] = (indices8_15 & 0x000000ff);
  block[14] = (indices8_15 & 0x0000ff00) >> 8;
  block[15] = (indices8_15 & 0x00ff0000) >> 16;
}

void YInvertBC6Mode11Block(byte* block)
{
  // Swap indices
  u32 indices0_7 = block[14] + 256 * (block[15] + 256 * (block[12] + 256 * block[13]));
  u32 indices8_15 = block[10] + 256 * (block[11] + 256 * (block[8] + 256 * block[9]));

  // Fill implicit 0 from index0
  indices8_15 = (indices8_15 & 0xfff0ffff) | ((indices8_15 & 0x000e0000) >> 1);

  // Check for implicit 0 for new index0
  // If not a 0, have to invert indices to make it a 0
  // which requires swapping endpoints so that indices still map the same colors
  if (indices0_7 & 0x00000008)
  {
    // Invert index values
    indices0_7 = ~indices0_7;
    indices8_15 = ~indices8_15;

    // Swap endpoints
    u32 endpointA = block[5] + 256 * (block[6] + 256 * (block[7] + 256 * (block[8] & 0x01)));
    endpointA = (endpointA << 5) | (block[4] >> 3);

    u32 endpointB = block[1] + 256 * (block[2] + 256 * (block[3] + 256 * (block[4] & 0x07)));
    endpointB = (endpointB << 3) | (block[0] >> 5);

    // Write new endpoints
    block[0] = (endpointA << 5) | (block[0] & 0x1f);
    block[1] = (endpointA >> 3);
    block[2] = (endpointA >> 11);
    block[3] = (endpointA >> 19);
    block[4] = (endpointA >> 27) | (endpointB << 3);
    block[5] = (endpointB >> 5);
    block[6] = (endpointB >> 13);
    block[7] = (endpointB >> 21);
    block[8] = (endpointB >> 29) | (block[8] & 0xfe);
  }

  // Write new indices
  block[8]  = (indices0_7  & 0x000000f0) | ((indices0_7 & 0x00000007) << 1) | (block[8] & 0x01);
  block[9]  = (indices0_7  & 0x0000ff00) >> 8;
  block[10] = (indices0_7  & 0x00ff0000) >> 16;
  block[11] = (indices0_7  & 0xff000000) >> 24;
  block[12] = (indices8_15 & 0x000000ff);
  block[13] = (indices8_15 & 0x0000ff00) >> 8;
  block[14] = (indices8_15 & 0x00ff0000) >> 16;
  block[15] = (indices8_15 & 0xff000000) >> 24;
}

void YInvertNonCompressed(byte* imageData, uint width, uint height, uint pixelSize)
{
  uint byteWidth = width * pixelSize;

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

void YInvertBlockCompressed(byte* imageData, uint width, uint height, uint dataSize, TextureCompression::Enum compression)
{
  ReturnIf((width >= 3 && width % 4 != 0) || (height >= 3 && height % 4 != 0), , "Block compressed image dimensions must be multiple of 4");

  width = Math::Max(width, 4u);
  height = Math::Max(height, 4u);

  uint numRows = height / 4;
  uint bytesPerRow = dataSize / numRows;

  uint blockSize = GetBlockSize(compression);
  ReturnIf(width / 4 * blockSize != bytesPerRow, , "Invalid image dimensions for compressed size");

  // Invert rows
  for (uint i = 0; i < numRows / 2; ++i)
  {
    byte* rowA = imageData + (i * bytesPerRow);
    byte* rowB = imageData + ((numRows - 1 - i) * bytesPerRow);
    for (uint b = 0; b < bytesPerRow; ++b)
      Math::Swap(rowA[b], rowB[b]);
  }

  // Invert blocks
  if (compression == TextureCompression::BC1)
  {
    for (uint i = 0; i < dataSize; i += blockSize)
    {
      byte* block = imageData + i;
      // Color bytes
      YInvertBC1Block(block);
    }
  }
  else if (compression == TextureCompression::BC2)
  {
    for (uint i = 0; i < dataSize; i += blockSize)
    {
      byte* block = imageData + i;
      // Alpha bytes
      Math::Swap(block[0], block[6]);
      Math::Swap(block[1], block[7]);
      Math::Swap(block[2], block[4]);
      Math::Swap(block[3], block[5]);
      // Color bytes
      YInvertBC1Block(block + 8);
    }
  }
  else if (compression == TextureCompression::BC3)
  {
    for (uint i = 0; i < dataSize; i += blockSize)
    {
      byte* block = imageData + i;
      // Alpha bytes
      YInvertBC3Block(block);
      // Color bytes
      YInvertBC1Block(block + 8);
    }
  }
  else if (compression == TextureCompression::BC4)
  {
    for (uint i = 0; i < dataSize; i += blockSize)
    {
      byte* block = imageData + i;
      YInvertBC3Block(block);
    }
  }
  else if (compression == TextureCompression::BC5)
  {
    for (uint i = 0; i < dataSize; i += blockSize)
    {
      byte* block = imageData + i;
      YInvertBC3Block(block);
      YInvertBC3Block(block + 8);
    }
  }
  else if (compression == TextureCompression::BC6)
  {
    for (uint i = 0; i < dataSize; i += blockSize)
    {
      byte* block = imageData + i;

      // (2 bits)
      // Mode 1  = 0x00
      // Mode 2  = 0x01
      // (5 bits)
      // Mode 3  = 0x02
      // Mode 4  = 0x06
      // Mode 5  = 0x0a
      // Mode 6  = 0x0e
      // Mode 7  = 0x12
      // Mode 8  = 0x16
      // Mode 9  = 0x1a
      // Mode 10 = 0x1e
      // Mode 11 = 0x03
      // Mode 12 = 0x07
      // Mode 13 = 0x0b
      // Mode 14 = 0x0f
      byte mode = block[0] & 0x1f;

      if (mode == 0x1e)
        YInvertBC6Mode10Block(block);
      else if (mode == 0x03)
        YInvertBC6Mode11Block(block);
      else
        ErrorIf(true, "Only supporting BC6 block modes 10 & 11. Nvtt libraries must be built to only output these formats.");
    }
  }
}

} // namespace Zero
