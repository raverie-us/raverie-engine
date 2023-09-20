// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

const String ZTexLoader = "TextureZTex";

const uint TextureFileId = 'ztex';
const uint TextureFileVersion = 1;

class TextureHeader
{
public:
  uint mFileId;
  uint mFileVersion;
  uint mType;
  uint mFormat;
  uint mMipCount;
  uint mTotalDataSize;
  uint mCompression;
  uint mAddressingX;
  uint mAddressingY;
  uint mFiltering;
  uint mAnisotropy;
  uint mMipMapping;
};

/// Information about a processed image.
class TextureInfo : public ContentComponent
{
public:
  RaverieDeclareType(TextureInfo, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Generate(ContentInitializer& initializer) override;

  /// File type extension of the source image.
  String GetFileType();
  String mFileType;

  /// Decompressed pixel format used to process image.
  String GetLoadFormat();
  String mLoadFormat;

  /// Width and height of the image, or of each face if used as a cubemap.
  String GetDimensions();
  String mDimensions;

  /// Total data size on hardware accounting for compression and pre-generated
  /// mips if applicable.
  String GetSize();
  String mSize;
};

class ShowPremultipliedAlphaFilter : public MetaPropertyFilter
{
public:
  RaverieDeclareType(ShowPremultipliedAlphaFilter, TypeCopyMode::ReferenceType);
  bool Filter(Member* prop, HandleParam instance) override;
};

class ShowGammaCorrectionFilter : public MetaPropertyFilter
{
public:
  RaverieDeclareType(ShowGammaCorrectionFilter, TypeCopyMode::ReferenceType);
  bool Filter(Member* prop, HandleParam instance) override;
};

/// Configuration for how an image file should be processed for use as a Texture
/// resource.
class TextureBuilder : public BuilderComponent
{
public:
  RaverieDeclareType(TextureBuilder, TypeCopyMode::ReferenceType);

  // BuilderComponent Interface

  void Serialize(Serializer& stream) override;
  void Initialize(ContentComposition* item) override;
  void Generate(ContentInitializer& initializer) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;
  void BuildContent(BuildOptions& buildOptions) override;
  void Rename(StringParam newName) override;

  // Properties

  /// Name for the Texture resource.
  String Name;
  /// Type of Texture that the image will be used for.
  TextureType::Enum mType;
  /// Block compression method to use if hardware supports it.
  TextureCompression::Enum mCompression;
  /// How to treat uv coordinates outside of [0, 1] along the Texture's width.
  TextureAddressing::Enum mAddressingX;
  /// How to treat uv coordinates outside of [0, 1] along the Texture's height.
  TextureAddressing::Enum mAddressingY;
  /// How samples should be blended under minification/magnification.
  TextureFiltering::Enum mFiltering;
  /// Max ratio of anisotropy that filtering will account for at oblique viewing
  /// angles.
  TextureAnisotropy::Enum mAnisotropy;
  /// If downsampled versions of the texture (mip maps) should be generated.
  TextureMipMapping::Enum mMipMapping;
  /// If color data should be stored pre-multiplied by alpha, applied before
  /// other operations.
  bool mPremultipliedAlpha;
  /// If color data should be stored in linear color space instead of sRGB color
  /// space. Important for albedo values used in lighting.
  bool mGammaCorrection;
  /// Progressively downsamples the image by half the size the specified number
  /// of times.
  int GetHalfScaleCount();
  void SetHalfScaleCount(int halfScaleCount);
  int mHalfScaleCount;

  // Internal

  String GetOutputFile();

  ResourceId mResourceId;
};

// DeclareEnum2(NormalGeneration, AverageRGB, Alpha);
// DeclareEnum3(NormalFilter, n3x3, n5x5, dudv);
//
// class HeightToNormalBuilder : public ContentComponent
//{
// public:
//  ZeroDeclareType(HeightToNormalBuilder);
//  static void InitializeMeta(MetaType* meta);
//  float mBumpiness;
//  NormalGeneration::Type mNormalSource;
//  NormalFilter::Type mNormalFilter;
//  bool mStoreHeightInAlpha;
//  //BuilderComponent Interface
//  void Generate(ContentInitializer& initializer) override;
//  void Serialize(Serializer& stream) override;
//};

} // namespace Raverie
