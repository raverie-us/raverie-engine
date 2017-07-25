///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteBuilder.hpp
/// 
/// Authors: Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

const uint MaxSpriteSize = 4096;

//------------------------------------------------------------------ Sprite Data 

DeclareEnum2(SpriteSampling, Nearest, Linear);

DeclareEnum3(SpriteFill, Stretch, NineSlice, Tiled);

// Put u64 at the bottom so when this is in a structure with other members
// the structure padding is self contained
#define SpriteDataMembers()     \
  u32 FrameSizeX;               \
  u32 FrameSizeY;               \
  u32 FrameCount;               \
  float FrameDelay;             \
  float PixelsPerUnit;          \
  float OriginX;                \
  float OriginY;                \
  SpriteSampling::Enum Sampling;\
  bool Looping;                 \
  Vec4 Slices;                  \
  u32 Fill;                     \
  Guid AtlasId;                 \
  SpriteData& GetSpriteData() { return *(SpriteData*)&FrameSizeX; }

//Sprite Data is the extra 'Tag' data passed along with a sprite
class SpriteData
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
 
  void Serialize(Serializer& stream);
  SpriteDataMembers();
};

//-------------------------------------------------------- Sprite Source Builder
//Sprite source builder outputs a png to be composited at runtime by the engine.
class SpriteSourceBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  SpriteDataMembers();

  void Serialize(Serializer& stream) override;

  SpriteSourceBuilder();

  //BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void BuildContent(BuildOptions& buildOptions) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
  void SetDefaults();
};

}
