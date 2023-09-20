// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
// This define controls whether we use our own Hdr loading/saving
#define RaverieCustomHdrSupport

// When reading, file positions must be at the beginning (0).

// Queries only if the file is valid (puts the stream back to 0).
bool IsHdr(Stream* stream);

// Queries if the file is a valid Png and if so, what its dimensions are (puts
// the stream back to 0).
bool ReadHdrInfo(Stream* stream, ImageInfo& info);

// Checks if the format is a valid load format (always returns true for None).
bool IsHdrLoadFormat(TextureFormat::Enum format);

// Checks if the format is a valid save format (always returns false for None).
bool IsHdrSaveFormat(TextureFormat::Enum format);

// Both Load/Save will leave the stream at the end, or wherever they failed.
// Supported texture formats: RGB32f
void LoadHdr(Status& status,
             Stream* stream,
             byte** output,
             uint* width,
             uint* height,
             TextureFormat::Enum* format,
             TextureFormat::Enum requireFormat = TextureFormat::None);
void SaveHdr(Status& status, Stream* stream, const byte* image, uint width, uint height, TextureFormat::Enum format);

void RgbeToRgb32f(byte* rgbe, float* rgb32f);
void Rgb32fToRgbe(float* rgb32f, byte* rgbe);

} // namespace Raverie
