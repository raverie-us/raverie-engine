// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

enum CoordinateMode
{
  cPixels,
  cRelative,
  cScreen
};

enum eImageMode
{
  cFill,
  cCenter,
  cStretch,
  cTile,
  cSlice,
};

/// Definition object for SlicedImage;
class SlicedDefinition : public BaseDefinition
{
public:
  RaverieDeclareType(SlicedDefinition, TypeCopyMode::ReferenceType);

  // BaseDefinition Interface
  void Initialize() override;
  void Serialize(Serializer& stream) override;
  void Unload() override;

  HandleOf<Texture> TexturePtr;
  String mTexture;
  Vec2 Uv0;
  Vec2 Uv1;
  Vec4 Slices;
  Vec4 Borders;
  Vec2 DefaultSize;
  int ImageMode;
  bool Sliced;
};

/// Definition object for SubImage.
class ImageDefinition : public SlicedDefinition
{
public:
  RaverieDeclareType(ImageDefinition, TypeCopyMode::ReferenceType);

  // BaseDefinition Interface
  void Initialize() override;
  void Serialize(Serializer& stream) override;
};

} // namespace Raverie
