// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(SpriteSourceBuilder, builder, type)
{
  RaverieBindDependency(ImageContent);

  RaverieBindFieldProperty(OriginX);
  RaverieBindFieldProperty(OriginY);

  RaverieBindFieldProperty(Looping);

  RaverieBindFieldProperty(Sampling);
  RaverieBindFieldProperty(PixelsPerUnit);

  RaverieBindGetterSetterProperty(Fill);
  RaverieBindGetterSetterProperty(Left);
  RaverieBindGetterSetterProperty(Right);
  RaverieBindGetterSetterProperty(Top);
  RaverieBindGetterSetterProperty(Bottom);
}

SpriteSourceBuilder::SpriteSourceBuilder() : DirectBuilderComponent(6, ".png", "SpriteSource")
{
}

SpriteFill::Enum SpriteSourceBuilder::GetFill()
{
  return (SpriteFill::Enum)Fill;
}

void SpriteSourceBuilder::SetFill(SpriteFill::Enum fill)
{
  Fill = fill;
}

int SpriteSourceBuilder::GetLeft()
{
  return (int)Slices[NineSlices::Left];
}

void SpriteSourceBuilder::SetLeft(int value)
{
  Slices[NineSlices::Left] = (float)value;
}

int SpriteSourceBuilder::GetRight()
{
  return (int)Slices[NineSlices::Right];
}

void SpriteSourceBuilder::SetRight(int value)
{
  Slices[NineSlices::Right] = (float)value;
}

int SpriteSourceBuilder::GetTop()
{
  return (int)Slices[NineSlices::Top];
}

void SpriteSourceBuilder::SetTop(int value)
{
  Slices[NineSlices::Top] = (float)value;
}

int SpriteSourceBuilder::GetBottom()
{
  return (int)Slices[NineSlices::Bottom];
}

void SpriteSourceBuilder::SetBottom(int value)
{
  Slices[NineSlices::Bottom] = (float)value;
}

RaverieDefineType(SpriteData, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool SpriteSourceBuilder::NeedsBuilding(BuildOptions& options)
{
  mOwner->EditMode = ContentEditMode::ResourceObject;

  return DirectBuilderComponent::NeedsBuilding(options);
}

void SpriteSourceBuilder::BuildListing(ResourceListing& listing)
{
  DirectBuilderComponent::BuildListing(listing);
}

void SpriteSourceBuilder::BuildContent(BuildOptions& buildOptions)
{
  DirectBuilderComponent::BuildContent(buildOptions);
  String destFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());

  // Append SpriteData to the end of the png
  File file;
  file.Open(destFile.c_str(), FileMode::Append, FileAccessPattern::Sequential);
  file.Write((byte*)&GetSpriteData(), sizeof(SpriteData));
  file.Close();
}

void SpriteSourceBuilder::Serialize(Serializer& stream)
{
  DirectBuilderComponent::Serialize(stream);
  GetSpriteData().Serialize(stream);
}

void SpriteSourceBuilder::Generate(ContentInitializer& initializer)
{
  SetDefaults();
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;
}

void SpriteSourceBuilder::SetDefaults()
{
  FrameSizeX = 0;
  FrameSizeY = 0;
  FrameCount = 1;
  FrameDelay = 1.0f / 12.0f;
  PixelsPerUnit = 64.0f;
  OriginX = 0;
  OriginY = 0;
  Sampling = SpriteSampling::Linear;
  Looping = true;
  Slices = Vec4(0, 0, 0, 0);
  Fill = SpriteFill::Stretch;
}

void SpriteData::Serialize(Serializer& stream)
{
  SerializeNameDefault(FrameSizeX, uint(0));
  SerializeNameDefault(FrameSizeY, uint(0));
  SerializeNameDefault(FrameCount, uint(0));
  SerializeNameDefault(FrameDelay, float(0));
  SerializeNameDefault(OriginX, float(0));
  SerializeNameDefault(OriginY, float(0));
  SerializeNameDefault(PixelsPerUnit, float(64.0f));
  SerializeEnumNameDefault(SpriteSampling, Sampling, SpriteSampling::Linear) SerializeNameDefault(Looping, true);
  SerializeNameDefault(Slices, Vec4(0, 0, 0, 0));
  SerializeEnumName(SpriteFill, Fill);
}

} // namespace Raverie
