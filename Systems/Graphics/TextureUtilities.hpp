#pragma once

namespace Zero
{

void SetPixelData(byte* data, uint index, Vec4 value, TextureFormat::Enum format);
void ReadPixelData(byte* data, uint index, Vec4& value, TextureFormat::Enum format);

void SetPixelDataByte(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataShort(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataHalfFloat(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataFloat(byte* data, uint index, Vec4 value, uint elementCount);
void SetPixelDataGamma(byte* data, uint index, Vec4 value, uint elementCount);

void ReadPixelDataByte(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataShort(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataHalfFloat(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataFloat(byte* data, uint index, Vec4& value, uint elementCount);
void ReadPixelDataGamma(byte* data, uint index, Vec4& value, uint elementCount);

bool IsColorFormat(TextureFormat::Enum format);
bool IsShortColorFormat(TextureFormat::Enum format);
bool IsFloatColorFormat(TextureFormat::Enum format);
bool IsDepthFormat(TextureFormat::Enum format);
bool IsDepthStencilFormat(TextureFormat::Enum format);

void YInvertNonCompressed(byte* imageData, uint width, uint height, uint pixelSize);
void YInvertBlockCompressed(byte* imageData, uint width, uint height, uint dataSize, TextureCompression::Enum compression);

} // namespace Zero
