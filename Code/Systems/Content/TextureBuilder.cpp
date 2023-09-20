// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "ContentEnumerations.hpp"

namespace Raverie
{

RaverieDefineType(TextureInfo, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);
  RaverieBindDependency(ImageContent);
  RaverieBindExpanded();

  RaverieBindGetterProperty(FileType);
  RaverieBindGetterProperty(LoadFormat);
  RaverieBindGetterProperty(Dimensions);
  RaverieBindGetterProperty(Size);
}

void TextureInfo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mFileType, String(""));
  SerializeNameDefault(mLoadFormat, String(""));
  SerializeNameDefault(mDimensions, String(""));
  SerializeNameDefault(mSize, String(""));
}

void TextureInfo::Generate(ContentInitializer& initializer)
{
}

String TextureInfo::GetFileType()
{
  return mFileType;
}

String TextureInfo::GetLoadFormat()
{
  return mLoadFormat;
}

String TextureInfo::GetDimensions()
{
  return mDimensions;
}

String TextureInfo::GetSize()
{
  return mSize;
}

RaverieDefineType(ShowPremultipliedAlphaFilter, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool ShowPremultipliedAlphaFilter::Filter(Member* prop, HandleParam instance)
{
  TextureBuilder* builder = instance.Get<TextureBuilder*>();
  ImageContent* content = (ImageContent*)builder->mOwner;
  TextureInfo* info = content->has(TextureInfo);
  if (info == nullptr)
    return true;

  return info->mLoadFormat == "RGBA8" || info->mLoadFormat == "RGBA16" || info->mLoadFormat == "SRGB8A8";
}

RaverieDefineType(ShowGammaCorrectionFilter, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool ShowGammaCorrectionFilter::Filter(Member* prop, HandleParam instance)
{
  TextureBuilder* builder = instance.Get<TextureBuilder*>();
  ImageContent* content = (ImageContent*)builder->mOwner;
  TextureInfo* info = content->has(TextureInfo);
  if (info == nullptr)
    return true;

  return info->mLoadFormat != "RGB32f";
}

RaverieDefineType(TextureBuilder, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDependency(ImageContent);
  RaverieBindExpanded();

  RaverieBindFieldProperty(Name);
  RaverieBindFieldProperty(mType);
  RaverieBindFieldProperty(mCompression);
  RaverieBindFieldProperty(mAddressingX);
  RaverieBindFieldProperty(mAddressingY);
  RaverieBindFieldProperty(mFiltering);
  RaverieBindFieldProperty(mAnisotropy);
  RaverieBindFieldProperty(mMipMapping);
  RaverieBindGetterSetterProperty(HalfScaleCount);
  RaverieBindFieldProperty(mPremultipliedAlpha)->Add(new ShowPremultipliedAlphaFilter());
  RaverieBindFieldProperty(mGammaCorrection)->Add(new ShowGammaCorrectionFilter());
}

void TextureBuilder::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);

  SerializeEnumNameDefault(TextureType, mType, TextureType::Texture2D);
  SerializeEnumNameDefault(TextureCompression, mCompression, TextureCompression::None);
  SerializeEnumNameDefault(TextureAddressing, mAddressingX, TextureAddressing::Repeat);
  SerializeEnumNameDefault(TextureAddressing, mAddressingY, TextureAddressing::Repeat);
  SerializeEnumNameDefault(TextureFiltering, mFiltering, TextureFiltering::Trilinear);
  SerializeEnumNameDefault(TextureAnisotropy, mAnisotropy, TextureAnisotropy::x16);
  SerializeEnumNameDefault(TextureMipMapping, mMipMapping, TextureMipMapping::PreGenerated);
  SerializeNameDefault(mHalfScaleCount, 0);
  SerializeNameDefault(mPremultipliedAlpha, false);
  SerializeNameDefault(mGammaCorrection, false);
}

void TextureBuilder::Initialize(ContentComposition* item)
{
  BuilderComponent::Initialize(item);
}

void TextureBuilder::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;

  mType = TextureType::Texture2D;
  mCompression = TextureCompression::None;
  mAddressingX = TextureAddressing::Repeat;
  mAddressingY = TextureAddressing::Repeat;
  mFiltering = TextureFiltering::Trilinear;
  mAnisotropy = TextureAnisotropy::x16;
  mMipMapping = TextureMipMapping::PreGenerated;

  mPremultipliedAlpha = false;
  mGammaCorrection = false;

  String filename = initializer.Filename.ToLower();
  if (initializer.Extension.ToLower() == "hdr")
  {
    mType = TextureType::TextureCube;
    mAddressingX = TextureAddressing::Clamp;
    mAddressingY = TextureAddressing::Clamp;
  }
  else if (filename.Contains("albedo"))
  {
    mCompression = TextureCompression::BC1;
    mGammaCorrection = true;
  }
  else if (filename.Contains("normal"))
    mCompression = TextureCompression::BC5;
  else if ((filename.Contains("metallic") || filename.Contains("metalness")) && filename.Contains("roughness"))
    mCompression = TextureCompression::BC5;
  else if (filename.Contains("metallic") || filename.Contains("metalness"))
    mCompression = TextureCompression::BC4;
  else if (filename.Contains("roughness"))
    mCompression = TextureCompression::BC4;
}

bool TextureBuilder::NeedsBuilding(BuildOptions& options)
{
  String destFile = FilePath::Combine(options.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileAndMeta(options, sourceFile, destFile);
}

void TextureBuilder::BuildListing(ResourceListing& listing)
{
  String loader = ZTexLoader;
  String destFile = GetOutputFile();
  listing.PushBack(ResourceEntry(0, loader, Name, destFile, mResourceId, this->mOwner, this));
}

void TextureBuilder::BuildContent(BuildOptions& buildOptions)
{
  String inputFile = FilePath::Combine(buildOptions.SourcePath, mOwner->Filename);
  String outputFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());

  TextureImporter importer(inputFile, outputFile, String());

  Status status;
  ImageProcessorCodes::Enum result = importer.ProcessTexture(status);

  switch (result)
  {
  case ImageProcessorCodes::Success:
    break;
  case ImageProcessorCodes::Failed:
    buildOptions.Failure = true;
    buildOptions.Message = "Failed to process texture.";
    break;
  case ImageProcessorCodes::Reload:
    ((ImageContent*)mOwner)->mReload = true;
    break;
  }
}

void TextureBuilder::Rename(StringParam newName)
{
  Name = newName;
}

int TextureBuilder::GetHalfScaleCount()
{
  return mHalfScaleCount;
}

void TextureBuilder::SetHalfScaleCount(int halfScaleCount)
{
  mHalfScaleCount = Math::Max(halfScaleCount, 0);
}

String TextureBuilder::GetOutputFile()
{
  return BuildString(Name, ".ztex");
}

// ZeroDefineType(HeightToNormalBuilder);
//
// void HeightToNormalBuilder::Generate(ContentInitializer& initializer)
//{
//  mNormalSource = NormalGeneration::AverageRGB;
//  mNormalFilter = NormalFilter::n3x3;
//  mStoreHeightInAlpha = false;
//  mBumpiness = 1.0f;
//}
//
// void HeightToNormalBuilder::Serialize(Serializer& stream)
//{
//  SerializeEnumName(NormalGeneration, mNormalSource);
//  SerializeEnumName(NormalFilter, mNormalFilter);
//  SerializeName(mBumpiness);
//  SerializeName(mStoreHeightInAlpha);
//}
//
// void HeightToNormalBuilder::InitializeMeta(MetaType* meta)
//{
//  RaverieBindDependency(ImageContent);
//  RaverieBindDependency(TextureBuilder);
//}

} // namespace Raverie
