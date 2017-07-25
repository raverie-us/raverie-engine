///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(AudioCueImport);
ZilchDefineEnum(AudioFileFormats);
ZilchDefineEnum(BasisType);
ZilchDefineEnum(ConflictAction);
ZilchDefineEnum(ImageImport);
ZilchDefineEnum(LoopingMode);
ZilchDefineEnum(MeshImport);
ZilchDefineEnum(PhysicsImport);
ZilchDefineEnum(PhysicsMeshType);
ZilchDefineEnum(ScaleConversion);
ZilchDefineEnum(SpriteFill);
ZilchDefineEnum(SpriteSampling);
ZilchDefineEnum(TrackType);
ZilchDefineEnum(TextureAddressing);
ZilchDefineEnum(TextureAnisotropy);
ZilchDefineEnum(TextureCompression);
ZilchDefineEnum(TextureFace);
ZilchDefineEnum(TextureFiltering);
ZilchDefineEnum(TextureFormat);
ZilchDefineEnum(TextureMipMapping);
ZilchDefineEnum(TextureType);

ZeroDefineArrayType(Array<AnimationClip>);

//**************************************************************************************************
ZilchDefineStaticLibrary(ContentMetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeEnum(AudioCueImport);
  ZilchInitializeEnum(AudioFileFormats);
  ZilchInitializeEnum(BasisType);
  ZilchInitializeEnum(ConflictAction);
  ZilchInitializeEnum(ImageImport);
  ZilchInitializeEnum(LoopingMode);
  ZilchInitializeEnum(MeshImport);
  ZilchInitializeEnum(PhysicsImport);
  ZilchInitializeEnum(PhysicsMeshType);
  ZilchInitializeEnum(ScaleConversion);
  ZilchInitializeEnum(SpriteFill);
  ZilchInitializeEnum(SpriteSampling);
  ZilchInitializeEnum(TrackType);
  ZilchInitializeEnum(TextureAddressing);
  ZilchInitializeEnum(TextureAnisotropy);
  ZilchInitializeEnum(TextureCompression);
  ZilchInitializeEnum(TextureFace);
  ZilchInitializeEnum(TextureFiltering);
  ZilchInitializeEnum(TextureFormat);
  ZilchInitializeEnum(TextureMipMapping);
  ZilchInitializeEnum(TextureType);

  ZeroInitializeArrayTypeAs(Array<AnimationClip>, "AnimationClips");

  // Meta Components
  ZilchInitializeType(ContentMetaComposition);

  // Events
  ZilchInitializeType(ContentSystemEvent);
  ZilchInitializeType(KeyFrameEvent);
  ZilchInitializeType(TrackEvent);

  ZilchInitializeType(ContentItem);
  ZilchInitializeType(ContentLibrary);
  ZilchInitializeType(ContentSystem);
  ZilchInitializeType(ContentComposition);
  ZilchInitializeType(ContentComponent);
  ZilchInitializeType(BuilderComponent);
  ZilchInitializeType(DataContent);
  ZilchInitializeType(DataBuilder);
  ZilchInitializeType(ContentTags);
  ZilchInitializeType(ZilchPluginBuilder);
  ZilchInitializeType(FontContent);
  ZilchInitializeType(FontBuilder);
  ZilchInitializeType(ImageContent);
  ZilchInitializeType(ImageOptions);
  ZilchInitializeType(ShowNormalGenerationOptionsFilter);
  ZilchInitializeType(GeometryOptions);
  ZilchInitializeType(AudioOptions);
  ZilchInitializeType(ConflictOptions);
  ZilchInitializeType(ImportOptions);
  ZilchInitializeType(ShowPremultipliedAlphaFilter);
  ZilchInitializeType(ShowGammaCorrectionFilter);
  ZilchInitializeType(TextureBuilder);
  ZilchInitializeType(SpriteData);
  ZilchInitializeType(SpriteSourceBuilder);
  ZilchInitializeType(TextContent);
  ZilchInitializeType(BaseTextBuilder);
  ZilchInitializeType(TextBuilder);
  ZilchInitializeType(ZilchScriptBuilder);
  ZilchInitializeType(ZilchFragmentBuilder);
  ZilchInitializeType(ContentCopyright);
  ZilchInitializeType(ContentHistory);
  ZilchInitializeType(ContentNotes);
  ZilchInitializeType(ResourceTemplate);
  ZilchInitializeType(RichAnimation);
  ZilchInitializeType(RichAnimationBuilder);
  ZilchInitializeType(TrackNode);
  ZilchInitializeType(GeometryImport);
  ZilchInitializeType(MeshEntry);
  ZilchInitializeType(MeshBuilder);
  ZilchInitializeType(PhysicsMeshBuilder);
  ZilchInitializeType(AnimationClip);
  ZilchInitializeType(AnimationBuilder);
  ZilchInitializeType(TextureContent);
  ZilchInitializeType(GeometryContent);
  ZilchInitializeType(AudioContent);
  ZilchInitializeType(TextureEntry);
  ZilchInitializeType(TextureInfo);
  ZilchInitializeType(SoundBuilder);
  ZilchInitializeType(BinaryContent);
  ZilchInitializeType(BinaryBuilder);
  ZilchInitializeType(GeneratedArchetype);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void ContentMetaLibrary::Initialize()
{
  ContentSystem::Initialize();
  Z::gContentSystem = ContentSystem::GetInstance();

  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

//**************************************************************************************************
void ContentMetaLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
  ContentSystem::Destroy();
}

}//namespace Zero

