///////////////////////////////////////////////////////////////////////////////
///
/// \file Sprite.hpp
/// Declaration of the Sprite component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

const int cMinFrameSize = 4;
const float cMaxFrameRate = 100000.0f;
const float cMinFrameRate = 0.001f;

DeclareEnum4(NineSlices, Left, Top, Right, Bottom);

//---------------------------------------------------------------- Sprite Source
//class SpriteSource : public TextureRegion
class SpriteSource : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  SpriteSource();
  ~SpriteSource();

  SpriteDataMembers();
  void Save(StringParam filename) override;
  void SetAtlas(Atlas* atlas);

  //TextureRegionInfo GetData() override;

  Vec2 GetSize();
  Vec2 GetOrigin();

  float GetFrameRate();
  void SetFrameRate(float newFrameRate);

  /// Shared texture from sprite Group
  Atlas* mAtlas;
  Texture* mTexture;
  UvRect mUvRect;

  Image SourceImage;

  /// Local Frame Data
  uint FramesX;
  uint FramesY;
  Vec2 PixelSize;
  Vec2 FrameTexSize;
  void FrameSetup();

  //ByteColor Sample(SoftwareSampleMode::Enum mode, real mip, uint currFrame, Vec2Param uv);

  /// Get the texture rect for the given frame.
  UvRect GetUvRect(uint currentFrame);

  SpriteSourceBuilder* mBuilder;
  Link<SpriteSource> link;
  Array<UvRect> mUvRects;
};

//-------------------------------------------------------- Sprite Source Manager
class SpriteSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(SpriteSourceManager, SpriteSource);

  SpriteSourceManager(BoundType* resourceType);
  ~SpriteSourceManager();

  void OnResourcesLoaded(ResourceEvent* event);
  void RebuildSpriteSheets();
};

struct PlacedSprite
{
  Image* Source;
  int OutputSheet;
  int Index;
  PixelRect Rect;
};

// Outputs is generated sprite sheets with all images placed on sprite
// Input is image buffer to be placed on sprite sheets
// PlacedSprites is location of each sprite
// Generated is image buffer that was created (needed to be cleaned up)
void GenerateSpriteSheets(Array<Image*>& Outputs, Array<Image*>& Inputs, Array<PlacedSprite>& placed, Array<Image*>& Generated, int outputSheetOffset);

}
