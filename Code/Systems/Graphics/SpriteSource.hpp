// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

const float cMaxFrameRate = 100000.0f;
const float cMinFrameRate = 0.001f;

class SpriteSource : public Resource
{
public:
  RaverieDeclareType(SpriteSource, TypeCopyMode::ReferenceType);

  SpriteDataMembers();
  void Unload() override;

  Vec2 GetSize();
  Vec2 GetOrigin();

  float GetFrameRate();

  // Get the texture uv rect for the given frame.
  UvRect GetUvRect(uint currentFrame);

  // Loads content image file into memory.
  void LoadSourceImage(Status& status, Image* image);

  HandleOf<Texture> GetAtlasTexture();
  TextureRenderData* GetAtlasTextureRenderData();

  // Below members are set by AtlasManager.
  // Texture atlas where sprite images are placed.
  HandleOf<Atlas> mAtlas;
  // UV's in texture where image of all frames was placed, including pixel
  // padding.
  UvRect mAtlasUvRect;
  // UV's of sprite frame 0, the top left frame.
  UvRect mBaseFrameUv;
  // UV translation per x/y frame count.
  Vec2 mPerFrameUvOffset;
  // Number of frames in one row of the whole sprite image.
  uint mFramesPerRow;
};

class SpriteSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(SpriteSourceManager, SpriteSource);

  SpriteSourceManager(BoundType* resourceType);
};

} // namespace Raverie
