// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
class Image;

// All images assume 4 channels are filled out
struct ImageInfo
{
  uint Width;
  uint Height;
  TextureFormat::Enum Format;
};

// Supported formats are: R8, RGB8, RGBA8, R16, RGB16, RGBA16, R32f, RGB32f,
// RGBA32f

// Checks if the format is a valid load format (always returns true for None).
bool IsImageLoadFormat(TextureFormat::Enum format);

// Checks if the format is a valid save format (always returns false for None).
bool IsImageSaveFormat(TextureFormat::Enum format);

DeclareEnum4(ImageBitDepth, None, I8, I16, F32);

// Returns a valid image load format from the number of components and the bit
// depth. If the components or bit-depth (or combination) are unsupported, it
// will return TextureFormat::None.
TextureFormat::Enum ToImageFormat(int components, ImageBitDepth::Enum depth);

// Gets the number of components and image bit depth from a given format.
// If the format is not valid, then the components will be 0 and depth will be
// None.
void FromImageFormat(TextureFormat::Enum format, int* components, ImageBitDepth::Enum* depth);

// Returns a list of all the supported extensions, e.g. "png".
const Array<String>& GetSupportedImageLoadExtensions();

// Returns if the given extension is a valid iamge load extension.
bool IsSupportedImageLoadExtension(StringParam extension);

// When reading, file positions must be at the beginning (0) and may be left in
// any position after a call.

// We can load files Png, Bmp, Psd, Tga, Gif, Hdr, Pic, Jpg, and Pnm
bool ReadImageInfo(Stream* stream, ImageInfo& info);
bool ReadImageInfo(File& file, ImageInfo& info);
bool ReadImageInfo(StringParam filename, ImageInfo& info);
bool ReadImageInfo(byte* encoded, size_t size, ImageInfo& info);

// You may set 'requireFormat' to 'None' if you want to load the texture in
// whatever format it's stored in.
void LoadImage(Status& status, Stream* stream, byte** output, uint* width, uint* height, TextureFormat::Enum* format, TextureFormat::Enum requireFormat = TextureFormat::None);
void LoadImage(Status& status, File& file, byte** output, uint* width, uint* height, TextureFormat::Enum* format, TextureFormat::Enum requireFormat = TextureFormat::None);
void LoadImage(Status& status, StringParam filename, byte** output, uint* width, uint* height, TextureFormat::Enum* format, TextureFormat::Enum requireFormat = TextureFormat::None);
void LoadImage(Status& status, byte* encoded, size_t size, byte** output, uint* width, uint* height, TextureFormat::Enum* format, TextureFormat::Enum requireFormat = TextureFormat::None);
void LoadImage(Status& status, Stream* stream, Image* imageOut);
void LoadImage(Status& status, File& file, Image* imageOut);
void LoadImage(Status& status, StringParam filename, Image* imageOut);
void LoadImage(Status& status, byte* encoded, size_t size, Image* imageOut);

// These are the formats we can save
DeclareEnum5(ImageSaveFormat, Png, Bmp, Tga, Hdr, Jpg);

void SaveImage(Status& status, Stream* stream, const byte* image, uint width, uint height, TextureFormat::Enum format, ImageSaveFormat::Enum imageType = ImageSaveFormat::Png);
void SaveImage(Status& status, File& file, const byte* image, uint width, uint height, TextureFormat::Enum format, ImageSaveFormat::Enum imageType = ImageSaveFormat::Png);
void SaveImage(Status& status, StringParam filename, const byte* image, uint width, uint height, TextureFormat::Enum format, ImageSaveFormat::Enum imageType = ImageSaveFormat::Png);
void SaveImage(Status& status, Stream* stream, Image* image, ImageSaveFormat::Enum imageType = ImageSaveFormat::Png);
void SaveImage(Status& status, File& file, Image* image, ImageSaveFormat::Enum imageType = ImageSaveFormat::Png);
void SaveImage(Status& status, StringParam filename, Image* image, ImageSaveFormat::Enum imageType = ImageSaveFormat::Png);

// To save a file to memory, create your own ByteBufferMemoryStream,
// ArrayByteMemoryStream, or FixedMemoryStream with a computed size from
// ReadImageInfo.

} // namespace Raverie
