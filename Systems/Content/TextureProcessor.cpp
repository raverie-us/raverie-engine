//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

TextureProcessor::TextureProcessor(TextureContent* textureContent, String outputPath, String inputFile)
  : mTextureContent(textureContent),
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
    // compressed texture have 0 height, we support pngs only atm
    if (texture->mHeight == 0 && texture->CheckFormat("png"))
      CreatePngTexture(texture, i);
    else
      ZPrint("Geometry Processor: File contains unsupported non-png format texture: format %s\n", texture->achFormatHint);
  }
}

void TextureProcessor::CreatePngTexture(aiTexture* texture, uint textureIndex)
{
  // create the .png file
  Image pngTexture;
  Status status;
  String filename = BuildString(mFilename, ToString(textureIndex));

  // our texture is a png, mWidth is the size of the image in bytes
  // and the pcData is just a buffer of that png data
  // save the png
  String fullPathToPngFile = FilePath::CombineWithExtension(mOutputPath, filename, ".png");
  WriteToFile(fullPathToPngFile.c_str(), (byte*)texture->pcData, texture->mWidth);
}

}// namespace Zero