// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

const uint cMinFrameSize = 1;
const uint cMaxSpriteSize = 4096;

DeclareEnum2(SpriteSampling, Nearest, Linear);

DeclareEnum3(SpriteFill, Stretch, NineSlice, Tiled);

// Put u64 at the bottom so when this is in a structure with other members
// the structure padding is self contained
#define SpriteDataMembers()                                                                                                                                                                            \
  u32 FrameSizeX;                                                                                                                                                                                      \
  u32 FrameSizeY;                                                                                                                                                                                      \
  u32 FrameCount;                                                                                                                                                                                      \
  float FrameDelay;                                                                                                                                                                                    \
  float PixelsPerUnit;                                                                                                                                                                                 \
  float OriginX;                                                                                                                                                                                       \
  float OriginY;                                                                                                                                                                                       \
  SpriteSampling::Enum Sampling;                                                                                                                                                                       \
  bool Looping;                                                                                                                                                                                        \
  Vec4 Slices;                                                                                                                                                                                         \
  u32 Fill;                                                                                                                                                                                            \
  SpriteData& GetSpriteData()                                                                                                                                                                          \
  {                                                                                                                                                                                                    \
    return *(SpriteData*)&FrameSizeX;                                                                                                                                                                  \
  }

// Sprite Data is the extra 'Tag' data passed along with a sprite
class SpriteData
{
public:
  RaverieDeclareType(SpriteData, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);
  SpriteDataMembers();
};

// Sprite source builder outputs a png to be composited at runtime by the
// engine.
class SpriteSourceBuilder : public DirectBuilderComponent
{
public:
  RaverieDeclareType(SpriteSourceBuilder, TypeCopyMode::ReferenceType);
  SpriteDataMembers();

  void Serialize(Serializer& stream) override;

  SpriteSourceBuilder();

  SpriteFill::Enum GetFill();
  void SetFill(SpriteFill::Enum fill);

  int GetLeft();
  void SetLeft(int value);

  int GetRight();
  void SetRight(int value);

  int GetTop();
  void SetTop(int value);

  int GetBottom();
  void SetBottom(int value);

  // BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void BuildContent(BuildOptions& buildOptions) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
  void SetDefaults();
};

} // namespace Raverie
