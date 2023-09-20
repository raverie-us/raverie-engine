// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
// This define controls whether we use our own Png loading/saving
#define RaverieCustomPngSupport

// When reading, file positions must be at the beginning (0).

// Queries only if the file is valid (puts the stream back to 0).
bool IsPng(Stream* stream);

// Queries if the file is a valid Png and if so, what its dimensions are (puts
// the stream back to 0).
bool ReadPngInfo(Stream* stream, ImageInfo& info);

// Checks if the format is a valid load format (always returns true for None).
bool IsPngLoadFormat(TextureFormat::Enum format);

// Checks if the format is a valid save format (always returns false for None).
bool IsPngSaveFormat(TextureFormat::Enum format);

// Both Load/Save will leave the stream at the end, or wherever they failed.
// Supported texture formats: RGBA8, RGBA16
void LoadPng(Status& status, Stream* stream, byte** output, uint* width, uint* height, TextureFormat::Enum* format, TextureFormat::Enum requireFormat = TextureFormat::None);
void SavePng(Status& status, Stream* stream, const byte* image, uint width, uint height, TextureFormat::Enum format);

} // namespace Raverie
