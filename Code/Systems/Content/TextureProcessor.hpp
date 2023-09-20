// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TextureProcessor
{
public:
  TextureProcessor(TextureContent* textureContent, String outputPath, String inputFile);

  void ExtractAndImportTextures(const aiScene* scene);
  void CreateTexture(aiTexture* texture, uint textureIndex, StringParam extension);

  TextureContent* mTextureContent;
  String mOutputPath;
  String mFilename;
};

} // namespace Raverie
