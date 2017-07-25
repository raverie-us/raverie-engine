#pragma once

namespace Zero
{

/// Data that represents a texture in the way that is intended to be used by graphics hardware.
class Texture : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Makes an anonymous Texture resource that can be defined by script and uploaded to the gpu.
  static HandleOf<Texture> CreateRuntime();

  Texture();

  // Properties

  /// The type of texture data being represented.
  TextureType::Enum mType;

  /// Block compression method being used. Requires pre-processing, cannot be set for runtime Textures.
  TextureCompression::Enum mCompression;

  // Setters are for modifying runtime textures and setting dirty flag.
  // Engine does not go through these methods for non-runtime.

  /// Width of the Texture in pixels. Set on Upload() for runtime Textures.
  uint mWidth;

  /// Height of the Texture in pixels. Set on Upload() for runtime Textures.
  uint mHeight;

  /// Width and height (x, y) of the Texture in pixels. Set on Upload() for runtime Textures.
  IntVec2 GetSize();
  void SetSize(IntVec2 size);

  /// Memory format of the stored pixel data. Set on Upload() for runtime Textures.
  TextureFormat::Enum GetFormat();
  void SetFormat(TextureFormat::Enum format);
  TextureFormat::Enum mFormat;

  /// How to treat uv coordinates outside of [0, 1] along the Texture's width.
  TextureAddressing::Enum GetAddressingX();
  void SetAddressingX(TextureAddressing::Enum addressingX);
  TextureAddressing::Enum mAddressingX;

  /// How to treat uv coordinates outside of [0, 1] along the Texture's height.
  TextureAddressing::Enum GetAddressingY();
  void SetAddressingY(TextureAddressing::Enum addressingY);
  TextureAddressing::Enum mAddressingY;

  /// How samples should be blended under minification/magnification.
  TextureFiltering::Enum GetFiltering();
  void SetFiltering(TextureFiltering::Enum filtering);
  TextureFiltering::Enum mFiltering;

  /// Max ratio of anisotropy that filtering will account for at oblique viewing angles.
  TextureAnisotropy::Enum GetAnisotropy();
  void SetAnisotropy(TextureAnisotropy::Enum anisotropy);
  TextureAnisotropy::Enum mAnisotropy;

  /// If downsampled versions of the texture (mip maps) should be generated. PreGenerated is not valid for runtime Textures.
  TextureMipMapping::Enum GetMipMapping();
  void SetMipMapping(TextureMipMapping::Enum mipMapping);
  TextureMipMapping::Enum mMipMapping;

  /// If sampling in hardware should perform comparison instead of fetching. Requires using SamplerShadow2d in the shader.
  TextureCompareMode::Enum GetCompareMode();
  void SetCompareMode(TextureCompareMode::Enum compareMode);
  TextureCompareMode::Enum mCompareMode;

  /// Which method of comparison should be used if CompareMode is set to Enable.
  TextureCompareFunc::Enum GetCompareFunc();
  void SetCompareFunc(TextureCompareFunc::Enum compareFunc);
  TextureCompareFunc::Enum mCompareFunc;

  /// Uploads the given texture data to the gpu, configured with the current settings of this Texture.
  void Upload(TextureData& textureData);

  /// Uploads the given texture data, overwriting a sub region of the texture data that is already on the gpu.
  void SubUpload(TextureData& textureData, int xOffset, int yOffset);

  // Used by graphics engine to upload data directly.
  void Upload(uint width, uint height, TextureFormat::Enum format, byte* data, uint size, bool copyData = true);

  // Internal

  void Upload(Image& image);
  void SubUpload(Image& image, int xOffset, int yOffset);

  TextureRenderData* mRenderData;

  uint mMipCount;
  uint mTotalDataSize;
  MipHeader* mMipHeaders;
  byte* mImageData;

  bool mProtected;
  bool mDirty;
};

/// Resource Manager for Textures.
class TextureManager : public ResourceManager
{
public:
  DeclareResourceManager(TextureManager, Texture);
  TextureManager(BoundType* resourceType);
};

} //namespace Zero
