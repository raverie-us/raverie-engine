///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteBuilder.cpp
/// 
/// Authors: Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{


//-------------------------------------------------------- Sprite Source Builder
ZilchDefineType(SpriteSourceBuilder, builder, type)
{
  ZeroBindDependency(ImageContent);
}

SpriteSourceBuilder::SpriteSourceBuilder()
  : DirectBuilderComponent(6, ".png", "SpriteSource")
{
}

ZilchDefineType(SpriteData, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool SpriteSourceBuilder::NeedsBuilding(BuildOptions& options)
{
  mOwner->EditMode = ContentEditMode::ResourceObject;

  if(DirectBuilderComponent::NeedsBuilding(options))
    return true;

  return DirectBuilderComponent::NeedsBuildingTool(options, "SpriteVersion.txt");
}

void SpriteSourceBuilder::BuildListing(ResourceListing& listing)
{
  DirectBuilderComponent::BuildListing(listing);
}

void SpriteSourceBuilder::BuildContent(BuildOptions& buildOptions)
{
  DirectBuilderComponent::BuildContent(buildOptions);
  String destFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());

  //Append SpriteData to the end of the png
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
  FrameDelay = 1.0f/12.0f;
  PixelsPerUnit = 64.0f;
  OriginX = 0;
  OriginY = 0;
  Sampling = SpriteSampling::Linear;
  Looping = true;
  Slices = Vec4(0,0,0,0);
  AtlasId = 0x5274cf95e4a661fdULL;
  Fill = SpriteFill::Stretch;
}

//------------------------------------------------------------------ Sprite Data
void SpriteData::Serialize(Serializer& stream)
{
  SerializeNameDefault(AtlasId, Guid(0));
  SerializeNameDefault(FrameSizeX, uint(0));
  SerializeNameDefault(FrameSizeY, uint(0));
  SerializeNameDefault(FrameCount, uint(0));
  SerializeNameDefault(FrameDelay, float(0));
  SerializeNameDefault(OriginX, float(0));
  SerializeNameDefault(OriginY, float(0));
  SerializeNameDefault(PixelsPerUnit, float(64.0f));
  SerializeEnumNameDefault(SpriteSampling, Sampling, SpriteSampling::Linear)
  SerializeNameDefault(Looping, true);
  SerializeNameDefault(Slices, Vec4(0,0,0,0));
  SerializeEnumName(SpriteFill, Fill);
}

}
