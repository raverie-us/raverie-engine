//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////

namespace Zero
{

class TextureProcessor
{
public:
  TextureProcessor(TextureContent* textureContent, String outputPath, String inputFile);

  void ExtractAndImportTextures(const aiScene* scene);
  void CreatePngTexture(aiTexture* texture, uint textureIndex);

  TextureContent* mTextureContent;
  String mOutputPath;
  String mFilename;
};

}// namespace Zero