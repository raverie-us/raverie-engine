///////////////////////////////////////////////////////////////////////////////
///
/// \file Atlas.hpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Atlas : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Sprite sampling modes assume mip mapping
  // so this must be greater than 0
  // Sprite mipmapping currently not used
  static const int sMaxMipLevel = 0;
  // Border width must be 2^sMaxMipLevel in order to
  // leave a minimum of a 1 pixel border for bilinear sampling
  static const int sBorderWidth = 1 << sMaxMipLevel;

  Atlas();
  ~Atlas();

  void Serialize(Serializer& stream) override {}
  void Unload() override;
  void ClearTextures();
  void AddSpriteSource(SpriteSource* source);
  bool NeedsBuilding;

  InList<SpriteSource> Sources;
  Array< HandleOf<Texture> > Textures;
};

class AtlasManager : public ResourceManager
{
public:
  DeclareResourceManager(AtlasManager, Atlas);

  AtlasManager(BoundType* resourceType);
  ~AtlasManager();
};

}
