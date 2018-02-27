// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

const uint cAtlasSize = 4096;

class Atlas : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  static const int sMaxMipLevel = 2;
  // Border width must be 2^sMaxMipLevel in order to
  // leave a minimum of a 1 pixel border for bilinear sampling
  static const int sBorderWidth = 1 << sMaxMipLevel;

  static HandleOf<Atlas> CreateRuntime();

  Atlas();

  bool AddSpriteSource(SpriteSource* source, Image* image);
  void RemoveSpriteSource(SpriteSource* source);
  
  HandleOf<Texture> mTexture;

  AvlDynamicAabbTree<Aabb> mPlacedAabbs;
  HashMap<SpriteSource*, BroadPhaseProxy> mAabbTreeProxies;
};

class AtlasManager : public ResourceManager
{
public:
  DeclareResourceManager(AtlasManager, Atlas);

  AtlasManager(BoundType* resourceType);

  void AddSpriteSource(SpriteSource* source, Image* image);
  void RemoveSpriteSource(SpriteSource* source);
};

} // namespace Zero
