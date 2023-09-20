// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

TextureProcessor::TextureProcessor(TextureContent* textureContent, String outputPath, String inputFile) :
    mTextureContent(textureContent),
    mOutputPath(outputPath),
    mFilename(FilePath::GetFileNameWithoutExtension(inputFile))
{
}

void TextureProcessor::ExtractAndImportTextures(const aiScene* scene)
{
  aiTexture** textures = scene->mTextures;
  size_t numTextures = scene->mNumTextures;

  for (size_t i = 0; i < numTextures; ++i)
  {
    aiTexture* texture = textures[i];

    // A height of 0 means the file is stored in memory as its compressed
    // format, e.g. a full png file in memory.
    if (texture->mHeight != 0)
    {
      ZPrint("Geometry Processor: File contains uncompressed texture data "
             "(currently unsupported)\n");
    }
    else
    {
      String extension = texture->achFormatHint;
      if (IsSupportedImageLoadExtension(extension))
      {
        CreateTexture(texture, i, extension);
      }
      else
      {
        ZPrint("Geometry Processor: File contains unsupported texture format: "
               "%s\n",
               texture->achFormatHint);
      }
    }
  }
}

void TextureProcessor::CreateTexture(aiTexture* texture, uint textureIndex, StringParam extension)
{
  Status status;
  String filename = BuildString(mFilename, ToString(textureIndex));

  // The member pcData holds the entire file data in memory, where mWidth is the
  // full length in bytes of pcData.
  String filePath = FilePath::CombineWithExtension(mOutputPath, filename, BuildString(".", extension));
  WriteToFile(filePath.c_str(), (byte*)texture->pcData, texture->mWidth);
}

} // namespace Raverie
